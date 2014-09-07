#define DEBUG

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
#include <dht22.h>
#include <utils.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_spi.h>
#include <usci_uart.h>
#else
#include <usi_spi.h>
#include <soft_uart.h>
#endif

#include <swap_mote.h>

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef GPIO_MODULE_T<1, 5, 3> SCLK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef GPIO_PIN_T<1, 5, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCLK;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MOSI;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MISO;
#endif
typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<2, 0, LOW> CE;
typedef GPIO_OUTPUT_T<2, 1, HIGH> CSN;
typedef GPIO_INPUT_T<2, 2> IRQ;
typedef GPIO_PIN_T<2, 3, OUTPUT, LOW> DHT_POWER;
typedef GPIO_PIN_T<2, 5, OUTPUT, HIGH> DHT_DATA;
typedef GPIO_PORT_T<1, LED_RED, SCLK, MISO, MOSI, RX, TX> PORT1;
typedef GPIO_PORT_T<2, IRQ, CSN, CE, DHT_POWER, DHT_DATA> PORT2;

typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

#ifdef __MSP430_HAS_USCI__
#ifdef DEBUG
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef DISABLED_UART UART;
#endif
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
typedef FLASH_T<SMCLK, WDT, &__infod> CONFIGURATION;

template<const uint32_t ID, typename TIMER, typename TIMEOUT, typename DATA, typename POWER>
struct TEMP_HUM_REGISTER_T {
	static int16_t temperature;
	static uint16_t humidity;
	static constexpr uint8_t reg_id = ID;
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = ID;
		packet.reg_value[0] = temperature >> 8;
		packet.reg_value[1] = temperature & 0xff;
		packet.reg_value[2] = humidity >> 8;
		packet.reg_value[3] = humidity & 0xff;
		packet.len = 7 + 4;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) { return false; }
	static bool handle_query(SWAP_PACKET& packet) { return false; }

	static void update(void) {
		int r;
		temperature = humidity = 0;
		r = read_dht<TIMER, TIMEOUT, DATA, POWER>(&temperature, &humidity);
		printf<UART>("Temperature: %d Humidity: %d (%d)\n", temperature, humidity, r);
		temperature += 500;
	};
};

template<const uint32_t ID, typename TIMER, typename TIMEOUT, typename DATA, typename POWER>
int16_t TEMP_HUM_REGISTER_T<ID, TIMER, TIMEOUT, DATA, POWER>::temperature = 0;
template<const uint32_t ID, typename TIMER, typename TIMEOUT, typename DATA, typename POWER>
uint16_t TEMP_HUM_REGISTER_T<ID, TIMER, TIMEOUT, DATA, POWER>::humidity = 0;

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
		printf<UART>("Voltage: %d\n", value);
		ADC10CTL0 &= ~(ADC10ON | REFON);
	};
};

template<const uint32_t ID>
uint16_t VOLTAGE_REGISTER_T<ID>::value = 0;

typedef VOLTAGE_REGISTER_T<11> VOLTAGE;
typedef TEMP_HUM_REGISTER_T<12, TIMER, TIMEOUT, DHT_DATA, DHT_POWER> TEMPERATURE_HUMIDITY;
typedef NRF24_T<TIMEOUT, SPI, CSN, CE, IRQ> NRF24;
typedef SWAP_MOTE_T<1, 1, 1, 1, NRF24, 70, TIMEOUT, CONFIGURATION, VOLTAGE, TEMPERATURE_HUMIDITY> MOTE;

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
	TIMER::init();

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
	NRF24::start_rx();
	while (1) {
		LED_RED::set_high();
		MOTE::handle_radio(10000);
		LED_RED::set_low();
		while (1) __bis_SR_register(LPM3_bits);
	}
	MOTE::announce();
	while (1) {
		printf<UART>("Updating registers\n");
		MOTE::update_registers();
		printf<UART>("Transmitting data\n");
		MOTE::transmit_data();
		printf<UART>("Sleeping\n");
		UART::disable();
		MOTE::sleep();
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
	if (UART::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (SPI::handle_irq() || UART::handle_rx_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#endif
