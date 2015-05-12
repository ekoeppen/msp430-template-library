#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
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

#include "radio_config_Si4455.h"

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<12000000> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<1, 3, LOW> WDT_ACTIVE;
typedef GPIO_MODULE_T<1, 4, 1> SMCLK_OUT;
#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef SOFT_UART_T<TIMER, TX, RX> UART;
#endif

#if defined(USE_SOFT_SPI)
typedef GPIO_OUTPUT_T<1, 5> SCLK;
typedef GPIO_INPUT_T<1, 7> MISO;
typedef GPIO_OUTPUT_T<1, 6> MOSI;
typedef SOFT_SPI_T<SCLK, MOSI, MISO, true, 0> SPI;
#elif defined(__MSP430_HAS_USCI__)
typedef GPIO_MODULE_T<1, 5, 3> SCLK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;
typedef USCI_SPI_T<USCI_B, 0, SMCLK, true, 0> SPI;
#else
typedef GPIO_PIN_T<1, 5, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCLK;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MOSI;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MISO;
typedef USI_SPI_T<SMCLK, true, 0> SPI;
#endif
typedef GPIO_OUTPUT_T<2, 1, HIGH> CSN;
typedef GPIO_OUTPUT_T<2, 0, HIGH> SDN;
typedef GPIO_INPUT_T<2, 2, RESISTOR_DISABLED, PULL_DOWN, INTERRUPT_ENABLED, TRIGGER_FALLING> IRQ;
typedef GPIO_INPUT_T<2, 3> CTS;
typedef GPIO_INPUT_T<2, 4> GPIO0;

typedef GPIO_PORT_T<1, LED_RED, SCLK, MISO, MOSI, RX, TX> PORT1;
typedef GPIO_PORT_T<2, IRQ, CSN, SDN, CTS, GPIO0> PORT2;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

typedef TIMEOUT_T<WDT> TIMEOUT;

static uint8_t const radio_config[] = RADIO_CONFIGURATION_DATA_ARRAY;

void poll_cts(void) {
	uint8_t response;
	do {
		SPI::transfer(0x44);
		response = SPI::transfer(0xff);
	} while (response == 0x00);
	response = SPI::transfer(0xff);
}

void radio_init(void)
{
	const uint8_t *data = radio_config;
	uint8_t len, response;
	while ((len = *data++) > 0) {
		while (CTS::is_low());
		CSN::set_low();
		SPI::transfer((uint8_t *) data, len);
		CSN::set_high();
		if (*data != 0x66) {
			while (CTS::is_low());
			do {
				CSN::set_low();
				SPI::transfer(0x44);
				response = SPI::transfer(0xff);
				CSN::set_high();
			} while (response != 0xff);
		}
		data += len;
	}
}

void radio_command(uint8_t command, uint8_t length)
{
	uint8_t response, done;
	while (CTS::is_low());
	CSN::set_low();
	SPI::transfer(command);
	CSN::set_high();
	done = false;
	while (!done) {
		while (CTS::is_low());
		CSN::set_low();
		SPI::transfer(0x44);
		response = SPI::transfer(0xff);
		if (response = 0xff) {
			while (length--) {
				response = SPI::transfer(0xff);
			}
			done = true;
		}
		CSN::set_high();
	}
}

int main(void)
{
	DCO::init();
	ACLK::init();
	SMCLK::init();
	MCLK::init();
	WDT::init();
	WDT::enable_irq();

	PORT1::init();
	PORT2::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	UART::init();
	printf<UART>("Si4455 example start.\n");

	SPI::init();
	SDN::set_high();
	MCLK::set_and_wait(1);
	SDN::set_low();
	MCLK::set_and_wait(5);
	radio_init();
	radio_command(0x01, 8);
	radio_command(0x10, 7);
	radio_command(0x33, 3);
	radio_command(0x01, 8);
	while (1) {
		LED_RED::toggle();
		TIMEOUT::set_and_wait(500);
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

void port2_irq(void) __attribute__((interrupt(PORT2_VECTOR)));
void port2_irq(void)
{
	if (PORT2::handle_irq()) {
		exit_idle();
	}
}

#ifndef USE_SOFT_SPI
#ifdef __MSP430_HAS_USCI__
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
#else
#ifdef __MSP430_HAS_USCI__
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (UART::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (UART::handle_rx_irq()) exit_idle();
}
#endif
#endif
