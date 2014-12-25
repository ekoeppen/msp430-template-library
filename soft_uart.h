#ifndef __SOFT_UART_H
#define __SOFT_UART_H

#include <stdint.h>
#include <clocks.h>

struct SOFT_UART_STATUS {
	bool framing_error: 1;
	bool rx_available: 1;
};

template<typename TIMER,
	typename TX,
	typename RX,
	const long SPEED = 9600>
struct SOFT_UART_T {
	static constexpr unsigned int bit_time = TIMER::frequency / SPEED;

	static SOFT_UART_STATUS status;

	static void init(void) {
		TIMER::claim();
	}

	static constexpr bool enabled(void) { return true; }

	static void enable(void) {
		TIMER::claim();
	}

	static void disable(void) {
		TIMER::release();
	}

	static int ready(void) {
		return 0;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void putc(char data) {
		unsigned st;

		TX::set_high();
		st = TIMER::counter();
		TX::set_low();
		while (TIMER::counter() - st < bit_time);
		for (int i = 0; i < 8; i++) {
			st += bit_time;
			TX::set(data & 0x80 == 0x80);
			while (TIMER::counter() - st < bit_time);
			data >>= 1;
		}
		st += bit_time;
		TX::set_high();
		while (TIMER::counter() - st < bit_time);
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(uint8_t *tx_data, int count) {
		while (count--) {
			putc(*tx_data++);
		}
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void puts(const char *data) {
		while (*data) {
			putc(*data++);
		};
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static char getc(void) {
		unsigned char c = 0;
		unsigned st;

		status.framing_error = false;
		status.rx_available = false;
		while (RX::is_high() && !TIMEOUT::triggered());
		if (RX::is_low()) {
			st = TIMER::counter();
			while (TIMER::counter() - st < bit_time);
			for (unsigned char i = 1; i != 0; i <<= 1) {
				st += bit_time;
				if (RX::is_high()) c |= i;
				while (TIMER::counter() - st < bit_time);
			}
			st += bit_time;
			while (TIMER::counter() - st < bit_time);
			if (RX::is_high()) {
				status.framing_error = false;
				status.rx_available = true;
			} else {
				status.framing_error = true;
			}
		}
		return c;
	}
};

template<typename TIMER, typename TX, typename RX, const long SPEED>
SOFT_UART_STATUS SOFT_UART_T<TIMER, TX, RX, SPEED>::status;

#endif
