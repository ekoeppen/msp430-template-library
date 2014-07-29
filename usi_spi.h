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

	static void init(void) {
		USICTL0 |= USISWRST;
		if (MODE == 0 || MODE == 1) USICTL1 = USICKPH;
		USICKCTL = USISSEL_2 | USIDIV_0 | (MODE == 1 || MODE == 3 ? USICKPL : 0);
		USICTL0 = USIPE7 | USIPE6 | USIPE5 | USIMST | USIOE;
		USICNT = 8;
		USISRL = 0;
		enable_irq();
		CLOCK::claim();
	}

	static void disable(void) {
		CLOCK::release();
	}

	static int ready(void) {
	}

	static void enable_irq(void) {
		USICTL1 |= USIIE;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static uint8_t transfer(uint8_t data) {
		USISRL = data;
		USICNT = DATA_LENGTH;
		tx_count = 1;
		rx_buffer = 0;
		do {
			enter_idle();
		} while (!TIMEOUT::triggered() && tx_count > 0);
		return USISRL;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(uint8_t *tx_data, int count, uint8_t *rx_data = 0) {
		tx_buffer = tx_data + 1;
		tx_count = count;
		rx_buffer = rx_data;
		rx_count = 0;
		USISRL = *tx_data;
		USICNT = DATA_LENGTH;
		do {
			enter_idle();
		} while (!TIMEOUT::triggered() && tx_count > 0);
	}

	static bool handle_irq(void) {
		bool resume = false;

		if (USICTL1 & USIIFG) {
			if (rx_buffer) {
				*rx_buffer++ = USISRL;
				rx_count++;
			}
			tx_count--;
			if (tx_count > 0) {
				USISRL = *tx_buffer++;
				USICNT = DATA_LENGTH;
			} else {
				clear_irq();
				resume = true;
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
