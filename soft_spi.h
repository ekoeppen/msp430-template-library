#ifndef __SOFT_SPI_H
#define __SOFT_SPI_H

#include <stdint.h>
#include <clocks.h>

template<typename SCLK,
	typename MOSI,
	typename MISO,
	const bool MASTER = true,
	const int MODE = 3,
	const long FREQUENCY = 1000000,
	const int DATA_LENGTH = 8,
	const bool MSB = true>
struct SOFT_SPI_T {
	static int tx_count;
	static int rx_count;
	static uint8_t *rx_buffer;
	static uint8_t *tx_buffer;

	static void init(void) {
		if (MODE == 2 || MODE == 3) {
			SCLK::set_high();
			MOSI::set_high();
		} else {
			SCLK::set_low();
			MOSI::set_low();
		}
	}

	static void disable(void) {
	}

	static int ready(void) {
	}

	static uint8_t transfer(uint8_t data) {
		for (int i = 0; i < 8; i++) {
			if (MODE == 0 || MODE == 2) {
				MOSI::set(data & 0x80);
				data <<= 1;
			}
			if (MODE == 1 || MODE == 3) {
				if (MISO::is_high()) data |= 1;
			}
			SCLK::toggle();
			if (MODE == 1 || MODE == 3) {
				MOSI::set(data & 0x80);
				data <<= 1;
			}
			if (MODE == 0 || MODE == 2) {
				if (MISO::is_high()) data |= 1;
			}
			SCLK::toggle();
		}
		if (MODE == 0 || MODE == 1) {
			MOSI::set_low();
		}
		return data;
	}

	static void transfer(uint8_t *tx_data, int count, uint8_t *rx_data = 0) {
		uint8_t rx;

		while (count-- > 0) {
			rx = transfer(*tx_data++);
			if (rx_data) *rx_data++ = rx;
		}
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

template<typename SCLK, typename MOSI, typename MISO, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
int SOFT_SPI_T<SCLK, MOSI, MISO, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_count;

template<typename SCLK, typename MOSI, typename MISO, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
int SOFT_SPI_T<SCLK, MOSI, MISO, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_count;

template<typename SCLK, typename MOSI, typename MISO, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *SOFT_SPI_T<SCLK, MOSI, MISO, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_buffer;

template<typename SCLK, typename MOSI, typename MISO, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *SOFT_SPI_T<SCLK, MOSI, MISO, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_buffer;

#endif

