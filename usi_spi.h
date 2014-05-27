#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>
#include <clocks.h>

template<typename CLOCK,
	const bool MASTER = true,
	const int MODE = 3,
	const long FREQUENCY = 1000000,
	const int DATA_LENGTH = 8,
	const bool LSB = true>
struct USI_SPI_T {
	static constexpr unsigned char idle_mode = CLOCK::idle_mode;

	static int tx_count;
	static int rx_count;
	static uint8_t *rx_buffer;
	static uint8_t *tx_buffer;

	static void init(void) {
		USICTL0 |= USISWRST;
		if (MODE == 0 || MODE == 1) USICTL1 = USICKPH;
		USICKCTL = USISSEL_2 | USIDIV_0 | (MODE == 1 || MODE == 3 ? USICKPL : 0);
		USICTL0 = USIPE7 | USIPE6 | USIPE5 | USIMST | USIOE;
	}

	static void disable(void) {
	}

	static int ready(void) {
	}

	static void enable_irq(void) {
		USICTL1 |= USIIE;
	}

	static uint8_t transfer(uint8_t data) {
		enable_irq();
		USISRL = data;
		USICNT = DATA_LENGTH;
		tx_count = 0;
		rx_buffer = 0;
		do {
			enter_idle(idle_mode);
		} while (USICTL1 & USIIFG);
		return USISRL;
	}

	static void transfer(uint8_t *tx_data, int count, uint8_t *rx_data = 0) {
		enable_irq();
		tx_buffer = tx_data + 1;
		tx_count = count - 1;
		rx_buffer = rx_data;
		rx_count = 0;
		USISRL = *tx_data;
		USICNT = DATA_LENGTH;
		enter_idle(idle_mode);
	}

	static bool handle_irq(void) {
		bool resume = false;

		if (USICTL1 & USIIFG) {
			if (tx_count > 0) {
				if (rx_buffer) {
					*rx_buffer = USISRL;
					rx_buffer++;
					rx_count++;
				}
				USISRL = *tx_buffer;
				USICNT = DATA_LENGTH;
				tx_buffer++;
				tx_count--;
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
int USI_SPI_T<CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_count;

template<typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
int USI_SPI_T<CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_count;

template<typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *USI_SPI_T<CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_buffer;

template<typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *USI_SPI_T<CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_buffer;

#endif
