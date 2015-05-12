#include "common.h"

uint8_t packet[64];
uint8_t hex_packet[128];

#define TO_HEX_NIBBLE(n) ((n) + ((n) > 9 ? 'a' - 10 : '0'))

void to_hex(uint8_t *in, uint8_t *out, int count)
{
	while (count) {
		*out = TO_HEX_NIBBLE(*in >> 4); out++;
		*out = TO_HEX_NIBBLE(*in & 0x0f); out++;
		in++;
		count--;
	}
}

int main(void)
{
	int n;

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
	LED_RED::set_low();
	while (1) {
		n = CC1101::rx_buffer(packet, sizeof(packet));
		if (n > 0) {
			to_hex(packet, hex_packet, n);
			UART::putc('(');
			UART::transfer(hex_packet + (n - 2) * 2, 4);
			UART::putc(')');
			UART::transfer(hex_packet + 2, (n - 3) * 2);
			UART::putc('\n');
		}
		LED_RED::set_high();
		TIMEOUT::set_and_wait(50);
		LED_RED::set_low();
	}
}
