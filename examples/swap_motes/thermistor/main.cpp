// #define DEBUG

#include <msp430.h>
#include <stdint.h>
#include <string.h>

#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <timer.h>
#include <nrf24.h>
#include <flash.h>
#include <adc.h>
#include <utils.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_spi.h>
#include <usci_uart.h>
#else
#include <usi_spi.h>
#include <soft_uart.h>
#endif

#include <swap_mote.h>

extern constexpr uint8_t rx_addr[5] = {
	0xff, 0xf0, 0xf0, 0xf0, 0xf0
};

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

#ifdef __MSP430_HAS_USCI__
#ifdef DEBUG
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
#else
typedef GPIO_OUTPUT_T<1, 1, LOW> RX;
typedef GPIO_OUTPUT_T<1, 2, LOW> TX;
#endif
typedef GPIO_MODULE_T<1, 5, 3> SCLK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;
#else
#ifdef DEBUG
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
#else
typedef GPIO_OUTPUT_T<1, 1, LOW> RX;
typedef GPIO_OUTPUT_T<1, 2, LOW> TX;
#endif
typedef GPIO_PIN_T<1, 5, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCLK;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MOSI;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MISO;
#endif
typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_ANALOG_T<1, 3> ADC_INPUT;
typedef GPIO_OUTPUT_T<1, 4> VREF_OUT;
typedef GPIO_OUTPUT_T<2, 0, LOW> CE;
typedef GPIO_OUTPUT_T<2, 1, HIGH> CSN;
typedef GPIO_INPUT_T<2, 2> IRQ;
typedef GPIO_OUTPUT_T<2, 3, LOW> P2_3_UNUSED;
typedef GPIO_OUTPUT_T<2, 4, LOW> P2_4_UNUSED;
typedef GPIO_OUTPUT_T<2, 5, LOW> P2_5_UNUSED;
typedef GPIO_OUTPUT_T<2, 6, LOW> P2_6_UNUSED;
typedef GPIO_OUTPUT_T<2, 7, LOW> P2_7_UNUSED;
typedef GPIO_PORT_T<1, LED_RED, ADC_INPUT, VREF_OUT, SCLK, MISO, MOSI, RX, TX> PORT1;
typedef GPIO_PORT_T<2, IRQ, CSN, CE, P2_3_UNUSED, P2_4_UNUSED, P2_5_UNUSED, P2_6_UNUSED, P2_7_UNUSED> PORT2;

typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

#ifdef __MSP430_HAS_USCI__
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
typedef USCI_SPI_T<USCI_B, 0, SMCLK, true, 0> SPI;
#else
#ifdef DEBUG
typedef SOFT_UART_T<TIMER, TX, RX> UART;
#else
typedef DISABLED_UART UART;
#endif
typedef USI_SPI_T<SMCLK, true, 0> SPI;
#endif

typedef TIMEOUT_T<WDT> TIMEOUT;
typedef ADC10_T<ADC10OSC, SREF_1 + ADC10SHT_3 + REFON + REFOUT + ADC10ON + REF2_5V, 0, ADC_INPUT> ADC_CHANNEL_3;

template<const uint32_t ID>
struct THERMISTOR_REGISTER_T {
	static const uint16_t t_table[][2];
	static uint16_t value;
	static constexpr uint8_t reg_id = ID;
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = ID;
		packet.reg_value[0] = value >> 8;
		packet.reg_value[1] = value & 0xff;
		packet.len = 7 + 2;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) { return false; }
	static bool handle_query(SWAP_PACKET& packet) { return false; }

	static void update(void) {
		uint16_t adc, r;
		int i;

		ADC10AE0 = BIT3 | BIT4;
		ADC10CTL1 = ADC10SSEL_3 + INCH_3;
		ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + REFOUT + ADC10ON;
		ADC10CTL0 |= ENC + ADC10SC;
		while (ADC10CTL1 & ADC10BUSY) ;
		adc = ADC10MEM;
		ADC10CTL0 &= ~ENC;
		ADC10CTL0 &= ~(ADC10ON | REFON);

		r = 10000L * (uint32_t) (0x3ff - adc) / (uint32_t) (adc);

		for (i = 1; i < ARRAY_COUNT(t_table); i++) {
			if (r > t_table[i][0]) break;
		}
		value = t_table[i - 1][1] +
			((uint32_t) (t_table[i - 1][0]) - r) * 50L / (uint32_t) (t_table[i - 1][0] - t_table[i][0]);
	};
};

template<const uint32_t ID>
struct VOLTAGE_REGISTER_T {
	static uint16_t value;
	static constexpr uint8_t reg_id = ID;
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = ID;
		packet.reg_value[0] = value >> 8;
		packet.reg_value[1] = value & 0xff;
		packet.len = 7 + 2;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) { return false; }
	static bool handle_query(SWAP_PACKET& packet) { return false; }

	static void update(void) {
		uint16_t adc;

		ADC10CTL1 = INCH_11 + ADC10SSEL_3;
		ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON;
		ADC10CTL0 |= ENC | ADC10SC;
		while (ADC10CTL1 & ADC10BUSY) ;
		adc = ADC10MEM;
		ADC10CTL0 &= ~ENC;

		if (adc >= 0x380) {
			ADC10CTL0 |= ENC | REF2_5V | ADC10SC;
			while (ADC10CTL1 & ADC10BUSY) ;
			adc = ADC10MEM;
			ADC10CTL0 &= ~ENC;
			value = ((unsigned long) adc * 5L * 125L / 128L);
		} else {
			value = ((unsigned long) adc * 3L * 125L / 128L);
		}
		ADC10CTL0 &= ~(ADC10ON | REFON);
	};
};

template<const uint32_t ID>
const uint16_t THERMISTOR_REGISTER_T<ID>::t_table[][2] = {
/*
	{332094, -40 + 50},
	{239900, -35 + 50},
	{175200, -30 + 50},
	{129287, -25 + 50},
	{96358, -20 + 50},
	{72500, -15 + 50},
*/
	{65535, -130 + 500},
	{55046, -100 + 500},
	{42157, -50 + 500},
	{32554, 0 + 500},
	{25339, 50 + 500},
	{19872, 100 + 500},
	{15698, 150 + 500},
	{12488, 200 + 500},
	{10000, 250 + 500},
	{8059, 300 + 500},
	{6535, 350 + 500},
	{5330, 400 + 500},
	{4372, 450 + 500},
	{3605, 500 + 500},
	{2989, 550 + 500},
	{2490, 600 + 500},
	{2084, 650 + 500},
	{1753, 700 + 500},
	{1481, 750 + 500},
	{1256, 800 + 500},
	{1070, 850 + 500},
	{915, 900 + 500},
	{786, 950 + 500},
	{677, 1000 + 500},
	{586, 1050 + 500},
	{508, 1100 + 500},
	{443, 1150 + 500},
	{387, 1200 + 500},
	{339, 1250 + 500},
	{298, 1300 + 500},
	{262, 1350 + 500},
	{232, 1400 + 500},
	{206, 1450 + 500}
};


template<const uint32_t ID>
uint16_t VOLTAGE_REGISTER_T<ID>::value = 0;
template<const uint32_t ID>
uint16_t THERMISTOR_REGISTER_T<ID>::value = 0;

typedef VOLTAGE_REGISTER_T<11> VOLTAGE;
typedef THERMISTOR_REGISTER_T<12> TEMPERATURE;
typedef NRF24_T<SPI, CSN, CE, IRQ, MCLK> NRF24;
typedef SWAP_MOTE_T<1, 4, 1, 1, NRF24, 70, CONFIG_STORAGE_UNUSED, VOLTAGE, TEMPERATURE> MOTE;

void dump_regs(void)
{
	uint8_t regs[64];
	NRF24::read_regs(regs);
	hex_dump_bytes<UART>(regs, sizeof(regs));
}


int main(void)
{
	DCO::init();
	ACLK::init();
	SMCLK::init();
#ifndef __MSP430_HAS_USCI__
#ifdef DEBUG
	TIMER::init();
#endif
#endif
	WDT::init();
	WDT::enable_irq();

	PORT1::init();
	PORT2::init();
	UART::init();
	printf<UART>("Mote example start\n");
	SPI::init();
	NRF24::init();
	MOTE::init();
	printf<UART>("Announcing mote\n");
	TIMEOUT::set(5000);
	MOTE::announce<TIMEOUT>();
	TIMEOUT::disable();
	while (1) {
		printf<UART>("Updating registers\n");
		MOTE::update_registers();
		printf<UART>("Transmitting data\n");
		MOTE::transmit_data();
		printf<UART>("Sleeping\n");
		UART::disable();
		TIMEOUT::set(5000);
		MOTE::sleep<TIMEOUT>();
		TIMEOUT::disable();
		UART::enable();
	}
	return 0;
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

#ifdef __MSP430_HAS_USCI__
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (SPI::handle_tx_irq() || UART::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (SPI::handle_rx_irq() || UART::handle_rx_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#endif
