#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#if defined(__MSP430_HAS_USCI__)
#include <usci_spi.h>
#else
#include <usi_spi.h>
#endif
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <timer.h>
#include <soft_uart.h>
#endif
#include <utils.h>
#include <drivers/fm25.h>

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<1000000> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
#if defined(__MSP430_HAS_USCI__)
typedef USCI_SPI_T<USCI_B, 0, SMCLK, true, 0> SPI;
typedef GPIO_MODULE_T<1, 5, 3> SCLK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;
#else
typedef USI_SPI_T<SMCLK, true, 0> SPI;
typedef GPIO_PIN_T<1, 5, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCLK;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MOSI;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MISO;
#endif
#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2, HIGH> TX;
typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef SOFT_UART_T<TIMER, TX, RX> UART;
#endif
typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<1, 4, HIGH> CS;

typedef GPIO_OUTPUT_T<2, 0, HIGH> WP;
typedef GPIO_OUTPUT_T<2, 1, HIGH> HOLD;

typedef GPIO_PORT_T<1, LED_RED, RX, TX, CS, SCLK, MISO, MOSI> PORT1;
typedef GPIO_PORT_T<2, WP, HOLD> PORT2;

typedef FM25_T<SPI, CS, uint16_t> FRAM;
typedef TIMEOUT_T<WDT> TIMEOUT;

uint8_t test_data[64];

int main(void)
{

	DCO::init();
	ACLK::init();
	SMCLK::init();
	WDT::init();
	PORT1::init();
	PORT2::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	UART::init();
	SPI::init();
	WDT::enable_irq();
	UART::puts("FM25 FRAM test starting...\n");
	FRAM::init();
	FRAM::enable_write();
	test_data[0] = test_data[1] = 0;
	for (int i = 2; i < sizeof(test_data); i++) {
		test_data[i] = i % 255;
	}
	FRAM::write(0, test_data, sizeof(test_data));
	for (int i = 0; i < sizeof(test_data); i++) {
		test_data[i] = 0;
	}
	FRAM::read(0, test_data, sizeof(test_data));
	hex_dump_bytes<UART>(test_data, sizeof(test_data));
	__bis_SR_register(LPM4_bits);
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

#if defined( __MSP430_HAS_USCI__)
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (SPI::handle_tx_irq()) exit_idle();
	if (UART::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (SPI::handle_rx_irq()) exit_idle();
	if (UART::handle_rx_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#endif
