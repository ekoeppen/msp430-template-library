#define USE_SOFT_SPI
#define SENDING

#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <nrf24.h>
#include <timer.h>
#if defined(USE_SOFT_SPI)
#include <soft_spi.h>
#elif defined __MSP430_HAS_USCI__
#include <usci_spi.h>
#else
#include <usi_spi.h>
#endif

#if defined __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <soft_uart.h>
#endif

extern constexpr uint8_t rx_addr[5] = {
	0xff, 0xf0, 0xf0, 0xf0, 0xf0
};

const uint8_t BROADCAST_ADDR[] = {0x00, 0xf0, 0xf0, 0xf0, 0xf0};

typedef ACLK_T<ACLK_SOURCE_VLOCLK> ACLK;
typedef SMCLK_T<CLK_SOURCE_DCOCLK, 12000000> SMCLK;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef TIMER_T<TIMER_A, 0, SMCLK, TASSEL_2 + MC_2> TIMER;
typedef SOFT_UART_T<TIMER, TX, RX> UART;
#endif

#if defined(USE_SOFT_SPI)
typedef GPIO_OUTPUT_T<1, 5> SCLK;
typedef GPIO_INPUT_T<1, 6> MISO;
typedef GPIO_OUTPUT_T<1, 7> MOSI;
typedef SOFT_SPI_T<SMCLK, SCLK, MOSI, MISO, true, 0> SPI;
#elif defined(__MSP430_HAS_USCI__)
typedef GPIO_MODULE_T<1, 5, 3> SCLK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;
typedef USCI_SPI_T<USCI_B, 0, SMCLK> SPI;
#else
typedef GPIO_PIN_T<1, 5, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCLK;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MOSI;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MISO;
typedef USI_SPI_T<SMCLK, true, 0> SPI;
#endif
typedef GPIO_OUTPUT_T<2, 0, LOW> CE;
typedef GPIO_OUTPUT_T<2, 1, HIGH> CSN;
typedef GPIO_INPUT_T<2, 2> IRQ;

typedef GPIO_PORT_T<1, LED_RED, SCLK, MISO, MOSI, RX, TX> PORT1;
typedef GPIO_PORT_T<2, SCLK, CSN, CE> PORT2;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;

typedef TIMEOUT_T<WDT> TIMEOUT;

typedef NRF24_T<TIMEOUT, SPI, CSN, CE, IRQ> NRF24;

int main(void)
{
	uint8_t regs[64];
	uint8_t packet[24];

	ACLK::init();
	SMCLK::init();
	WDT::init();
	WDT::enable_irq();

	PORT1::init();
	PORT2::init();
	LED_RED::set_low();

#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	UART::init();
	printf<UART>("NRF24 example start.\n");

	SPI::init();

	for (int i = 0; i < sizeof(regs); i++) regs[i] = 0;
	for (int i = 0; i < sizeof(packet); i++) packet[i] = 0;
	NRF24::init();
	NRF24::set_rx_addr(rx_addr);
	NRF24::set_channel(70);
	NRF24::read_regs(regs);
	hex_dump_bytes<UART>(regs, sizeof(regs));
#if (defined SENDING)
	while (1) {
		NRF24::start_tx();
		NRF24::tx_buffer(BROADCAST_ADDR, packet, sizeof(packet), false);
		LED_RED::toggle();
		TIMEOUT::set(1000);
		enter_idle<WDT>();
	}
#elif (defined SCANNING)
	NRF24::start_rx();
	while (1) {
		for (int i = 0; i < 127; i++) {
			NRF24::set_channel(i);
			printf<UART>("%03d: ", i);
			LED_RED::toggle();
			for (int j = 0; j < 5; j++) {
				if (NRF24::rw_reg(RF24_RPD, RF24_NOP)) {
					UART::putc('*');
				}
				TIMEOUT::set(100);
				enter_idle<WDT>();
			}
			UART::putc('\n');
		}
	}
#elif (defined RECEIVING)
	NRF24::start_rx();
	while (1) {
		uint8_t pipe, n;
		n = NRF24::rx_buffer(packet, sizeof(packet), &pipe, 60000L);
		UART::puts("------------------\n");
		hex_dump_bytes<UART>(packet, n);
	}
#endif
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

#ifndef USE_SOFT_SPI
#ifdef __MSP430_HAS_USCI__
void usci_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#endif
#endif
