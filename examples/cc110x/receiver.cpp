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
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	UART::init();
	printf<UART>("CC110X example start.\n");

	SPI::init();
	CC1101::init();
	while (1) {
		CC1101::rx_buffer(packet, sizeof(packet));
		LED_RED::set_high();
		TIMEOUT::set_and_wait(50);
		LED_RED::set_low();
	}
}
