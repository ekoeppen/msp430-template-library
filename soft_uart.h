#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include <clocks.h>

template<typename TX,
	typename RX,
	typename CLOCK,
	const long SPEED = 9600>
struct SOFT_UART_T {
	static constexpr unsigned char idle_mode = CLOCK::idle_mode;

	static void init(void) {
	}

	static void disable(void) {
	}

	static int ready(void) {
		return 0;
	}

	static void transfer(uint8_t *tx_data, int count) {
	}

	static void putc(char data) {
	}

	static void puts(char *data) {
		while (*data) {
			putc(*data++);
		};
	}

	static char getc() {
		return 0;
	}
};

#endif
