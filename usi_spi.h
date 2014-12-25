#ifndef __USI_SPI_H
#define __USI_SPI_H

#include <stdint.h>
#include <clocks.h>
#include <tasks.h>

template<typename CLOCK,
	const bool MASTER = true,
	const int MODE = 3,
	const long FREQUENCY = 1000000,
	const int DATA_LENGTH = 8,
	const bool MSB = true>
struct USI_SPI_T {
	volatile static int tx_count;
	volatile static int rx_count;
	static uint8_t *rx_buffer;
	static uint8_t *tx_buffer;

	static constexpr uint8_t calculate_divider(uint8_t divider) {
		return divider < 8 ? (CLOCK::frequency / (1 << divider) <= FREQUENCY ? divider : calculate_divider(divider + 1)) : 7;
	}

	static void enable(void) {
		constexpr uint8_t divider = calculate_divider(0);
		USICTL0 |= USISWRST;
		if (MODE == 0 || MODE == 1) USICTL1 = USICKPH;
		USICKCTL = USISSEL_2 | (divider << 5) | (MODE == 1 || MODE == 3 ? USICKPL : 0);
		USICTL0 = USIPE7 | USIPE6 | USIPE5 | USIMST | USIOE;
		USISRL = 0;
		USICNT = 8;
		while (!(USICTL1 & USIIFG));
	}

	static void init(void) {
		enable();
		enable_irq();
	}

	static void disable(void) {
		USICTL0 = USISWRST;
		USICTL1 = 0;
	}

	static int ready(void) {
	}

	static void enable_irq(void) {
		USICTL1 |= USIIE;
	}

	static void disable_irq(void) {
		USICTL1 &= ~USIIE;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static uint8_t transfer(uint8_t data) {
		CLOCK::claim();
		disable_irq();
		USISRL = data;
		USICNT = 8;
		while (!(USICTL1 & USIIFG));
		clear_irq();
		CLOCK::release();
		return USISRL;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(uint8_t *tx_data, int count, uint8_t *rx_data = 0) {
		CLOCK::claim();
		tx_buffer = tx_data;
		tx_count = count;
		rx_buffer = rx_data;
		rx_count = -1;
		enable_irq();
		USICTL1 |= USIIFG;
		while (!TIMEOUT::triggered() && tx_count > 0) {
			enter_idle();
		}
		CLOCK::release();
	}

	static bool handle_irq(void) {
		bool resume = false;
		uint8_t rx_data;

		if (USICTL1 & USIIFG) {
			rx_data = USISRL;
			if (tx_count > 0) {
				if (tx_buffer) {
					USISRL = *tx_buffer++;
				} else {
					USISRL = 0xff;
				}
				tx_count--;
				USICNT = DATA_LENGTH;
			} else {
				clear_irq();
				resume = true;
			}
			if (rx_buffer) {
				if (rx_count >= 0) {
					*rx_buffer++ = rx_data;
				}
				rx_count++;
			}
		}
		return resume;
	}

	static void clear_irq(void) {
		USICTL1 &= ~USIIFG;
	}

	static void putc(char data) {
		transfer(data);
	}

	static void puts(char *data) {
		while (*data) {
			putc(*data++);
		};
	}
};

template<typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
volatile int USI_SPI_T<CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_count;

template<typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
volatile int USI_SPI_T<CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_count;

template<typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *USI_SPI_T<CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_buffer;

template<typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *USI_SPI_T<CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_buffer;

#endif
