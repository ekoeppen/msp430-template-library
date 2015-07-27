#include "common.h"

uint8_t packet[64];

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
	LED_RED::set_high();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	UART::init();
	printf<UART>("CC110X example start.\n");

	SPI::init();
	CC1101::reset();
	CC1101::init(cc110x_default_init_values, ARRAY_COUNT(cc110x_default_init_values));
	CC1101::start_rx();
	LED_RED::set_low();
	while (1) {
		CC1101::rx_buffer<TIMEOUT_NEVER>(packet, sizeof(packet));
		LED_RED::set_high();
		TIMEOUT::set_and_wait(50);
		LED_RED::set_low();
	}
}
