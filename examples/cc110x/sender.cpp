#include "common.h"

int main(void)
{
	bool active = true;

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
	CC1101::reset();
	CC1101::init(cc110x_default_init_values, ARRAY_COUNT(cc110x_default_init_values));
	while (1) {
		if (active) {
			LED_RED::set_high();
			TIMEOUT::set_and_wait(50);
			CC1101::tx_buffer((uint8_t *) "\x55\xaa\x55\xaa", 4);
			LED_RED::set_low();
			TIMEOUT::set_and_wait(1000);
			if (BUTTON::irq_raised()) {
				active = false;
			}
		} else {
			LED_RED::set_high();
			BUTTON::wait_for_irq();
			BUTTON::clear_irq();
			active = true;
		}
	}
}
