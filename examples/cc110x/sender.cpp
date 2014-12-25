#include "common.h"

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
	printf<UART>("CC110X example start.\n");

	SPI::init();
	CC1101::init();
	while (1) {
		LED_RED::set_high();
		CC1101::tx_buffer((uint8_t *) "\x55\xaa\x55\xaa", 4);
		LED_RED::set_low();
		TIMEOUT::set_and_wait(1000);
	}
}
