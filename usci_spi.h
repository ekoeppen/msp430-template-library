#ifndef __USCI_SPI_H
#define __USCI_SPI_H

#include <stdint.h>
#include <clocks.h>
#include <tasks.h>
#include <usci.h>

template<const USCI_MODULE MODULE,
	const int INSTANCE,
	typename CLOCK,
	const bool MASTER = true,
	const int MODE = 3,
	const long FREQUENCY = 1000000,
	const int DATA_LENGTH = 8,
	const bool MSB = true>
struct USCI_SPI_T {
	typedef USCI_T<MODULE, INSTANCE> USCI;

	volatile static int tx_count;
	volatile static int rx_count;
	static uint8_t *rx_buffer;
	static uint8_t *tx_buffer;

	static void init(void) {
		*USCI::CTL1 |= UCSWRST;
		if (USCI::MCTL != 0) *USCI::MCTL = 0x00;
		*USCI::CTL0 = (MODE == 0 || MODE == 2 ? UCCKPH : 0) |
			(MODE == 2 || MODE == 3 ? UCCKPL : 0) | (MSB ? UCMSB : 0) | (MASTER ? UCMST : 0) | UCMODE_0 | UCSYNC;
		*USCI::BR0 = (CLOCK::frequency / FREQUENCY) & 0xff;
		*USCI::BR1 = (CLOCK::frequency / FREQUENCY) >> 8;
		if (CLOCK::type == CLOCK_TYPE_ACLK) {
			*USCI::CTL1 = UCSSEL_1;
		} else {
			*USCI::CTL1 = UCSSEL_2;
		}
		rx_buffer = 0;
		tx_count = 0;
		USCI::enable_rx_irq();
	}

	static void enable(void) {
		*USCI::CTL1 &= ~UCSWRST;
	}

	static void disable(void) {
		*USCI::CTL1 |= UCSWRST;
	}

	static int ready(void) {
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static uint8_t transfer(uint8_t data) {
		CLOCK::claim();
		tx_buffer = &data;
		tx_count = 1;
		rx_buffer = 0;
		USCI::enable_tx_irq();
		while (!TIMEOUT::triggered() && tx_count > 0) {
			enter_idle();
		}
		CLOCK::release();
		return *USCI::RXBUF;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(uint8_t *tx_data, int count, uint8_t *rx_data = 0) {
		CLOCK::claim();
		tx_buffer = tx_data;
		tx_count = count;
		rx_buffer = rx_data;
		rx_count = 0;
		USCI::enable_tx_irq();
		while (!TIMEOUT::triggered() && tx_count > 0) {
			enter_idle();
		}
		CLOCK::release();
	}

	static bool handle_tx_irq(void) {
		bool resume = false;

		if (USCI::tx_irq_pending()) {
			if (tx_count > 0) {
				if (tx_buffer) {
					*USCI::TXBUF = *tx_buffer;
					tx_buffer++;
				} else {
					*USCI::TXBUF = 0xff;
				}
				tx_count--;
			} else {
				USCI::disable_tx_irq();
				if (rx_buffer == 0) {
					resume = true;
				}
			}
		}
		return resume;
	}

	static bool handle_rx_irq(void) {
		bool resume = false;

		if (USCI::rx_irq_pending()) {
			if (rx_buffer) {
				*rx_buffer = *USCI::RXBUF;
				rx_buffer++;
				rx_count++;
			}
			USCI::clear_rx_irq();
			if (tx_count == 0) {
				resume = true;
			}
		}
		return resume;
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

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
volatile int USCI_SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
volatile int USCI_SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *USCI_SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *USCI_SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_buffer;

#endif
