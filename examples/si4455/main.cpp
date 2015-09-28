#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <utils.h>
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

#include "radio_config_Si4355.h"

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<12000000> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_INPUT_T<1, 3, RESISTOR_ENABLED, PULL_UP, INTERRUPT_ENABLED, TRIGGER_FALLING> BUTTON;
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

typedef GPIO_PORT_T<1, BUTTON, LED_RED, SCLK, MISO, MOSI, RX, TX> PORT1;
typedef GPIO_PORT_T<2, IRQ, CSN, SDN, CTS, GPIO0> PORT2;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

typedef TIMEOUT_T<WDT> TIMEOUT;

static uint8_t const radio_config[] = RADIO_CONFIGURATION_DATA_ARRAY;
static uint8_t response_buffer[64];

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

void radio_command(uint8_t *command, uint8_t length, uint8_t *response, uint8_t response_length)
{
	uint8_t r, done;

	while (CTS::is_low());
	CSN::set_low();
	while (length--) {
		SPI::transfer(*command++);
	}
	CSN::set_high();
	done = false;
	while (!done) {
		while (CTS::is_low());
		CSN::set_low();
		SPI::transfer(0x44);
		r = SPI::transfer(0xff);
		if (r == 0xff) {
			while (response_length--) {
				*response++ = SPI::transfer(0xff);
			}
			done = true;
		}
		CSN::set_high();
	}
}

void radio_command(uint8_t command, uint8_t *response, uint8_t response_length)
{
	radio_command(&command, 1, response, response_length);
}

void read_rx_fifo(uint8_t *response, uint8_t response_length)
{
	CSN::set_low();
	SPI::transfer(0x77);
	while (response_length--) {
		*response++ = SPI::transfer(0xff);
	}
	CSN::set_high();
}

void read_rx_fifo(uint8_t length)
{
	read_rx_fifo(response_buffer, length); hex_dump_bytes<UART>(response_buffer, length, "RX FIFO");
}

void start_rx(void)
{
	radio_command((uint8_t *) "\x32\x00\x00\x00\x07\x00\x01\x00", 8, 0, 0);
}

void rx_fifo_info(void)
{
	radio_command((uint8_t *) "\x15\x00", 2, response_buffer, 1); hex_dump_bytes<UART>(response_buffer, 16, "FIFO Status");
}

void reset_rx_fifo(void)
{
	radio_command((uint8_t *) "\x15\x02", 2, response_buffer, 1); hex_dump_bytes<UART>(response_buffer, 16, "FIFO Status (reset)");
}

void device_status(void)
{
	radio_command(0x33, response_buffer, 2); hex_dump_bytes<UART>(response_buffer, 16, "Device status");
}

void clear_irq(void)
{
	radio_command((uint8_t *) "\x20\x00\x00\x00", 4, response_buffer, 8); hex_dump_bytes<UART>(response_buffer, 16, "IRQ Status");
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
	while (1) {
		clear_irq();
		start_rx();
		device_status();
		UART::puts("-- Wait for IRQ ------------------------------------------\n");
		do {
			IRQ::clear_irq();
			IRQ::wait_for_irq();
			clear_irq();
		} while (!(response_buffer[0] & 0x01));
		LED_RED::set_high();
		rx_fifo_info();
		read_rx_fifo(response_buffer[0]);
		reset_rx_fifo();
		UART::puts("----------------------------------------------------------\n");
		LED_RED::set_low();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

void port1_irq(void) __attribute__((interrupt(PORT1_VECTOR)));
void port1_irq(void)
{
	if (PORT1::handle_irq()) {
		exit_idle();
	}
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
