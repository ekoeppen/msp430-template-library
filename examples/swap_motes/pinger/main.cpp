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
typedef GPIO_PORT_T<1, LED_RED, SCLK, MISO, MOSI ,RX, TX> PORT1;
typedef GPIO_PORT_T<2, IRQ, CSN, CE> PORT2;

typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

#ifdef __MSP430_HAS_USCI__
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
typedef USCI_SPI_T<USCI_B, 0, SMCLK, true, 0> SPI;
#else
typedef SOFT_UART_T<TIMER, TX, RX> UART;
typedef USI_SPI_T<SMCLK, true, 0> SPI;
#endif

typedef TIMEOUT_T<WDT> TIMEOUT;
typedef NRF24_T<SPI, CSN, CE, IRQ, MCLK> NRF24;
typedef SWAP_MOTE_T<1, 1, 1, 1, NRF24, 70> MOTE;

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
	MOTE::tx_packet.src = MOTE::config.address[0];
	MOTE::tx_packet.dest = 1;
	MOTE::tx_packet.function = COMMAND;
	MOTE::tx_packet.nonce = 0;
	MOTE::tx_packet.hop_secu = 0;
	MOTE::tx_packet.reg_addr = 1;
	MOTE::tx_packet.len = 7 + 1;
	MOTE::tx_packet.reg_id = 11;
	MOTE::tx_packet.reg_value[0] = 0x55;
	while (1) {
		printf<UART>("Sending command\n");
		NRF24::start_tx();
		NRF24::set_tx_addr(MOTE::BROADCAST_ADDR);
		NRF24::tx_buffer((uint8_t *) &MOTE::tx_packet, MOTE::tx_packet.len);
		MOTE::tx_packet.reg_value[0] ^= 0xff;
		NRF24::power_down();
		printf<UART>("Sleeping\n");
		TIMEOUT::set(5000);
		MOTE::sleep<TIMEOUT>();
		TIMEOUT::disable();
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
	if (SPI::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (UART::handle_rx_irq()) exit_idle();
	if (SPI::handle_rx_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#endif
