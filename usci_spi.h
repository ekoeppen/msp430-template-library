#ifndef __SPI_H
#define __SPI_H

#include <stdint.h>
#include <clocks.h>
#include <usci.h>

template<const USCI_MODULE MODULE,
	const int INSTANCE,
	typename CLOCK,
	const bool MASTER = true,
	const int MODE = 3,
	const long FREQUENCY = 1000000,
	const int DATA_LENGTH = 8,
	const bool LSB = true>
struct USCI_SPI_T {
	static constexpr volatile unsigned char *CTL0 = USCI_REGISTER(MODULE, INSTANCE, 0);
	static constexpr volatile unsigned char *CTL1 = USCI_REGISTER(MODULE, INSTANCE, 1);
	static constexpr volatile unsigned char *BR0 = USCI_REGISTER(MODULE, INSTANCE, 2);
	static constexpr volatile unsigned char *BR1 = USCI_REGISTER(MODULE, INSTANCE, 3);
	static constexpr volatile unsigned char *MCTL = USCI_REGISTER(MODULE, INSTANCE, 4);
	static constexpr volatile unsigned char *STAT = USCI_REGISTER(MODULE, INSTANCE, 5);
	static constexpr volatile unsigned char *RXBUF = USCI_REGISTER(MODULE, INSTANCE, 6);
	static constexpr volatile unsigned char *TXBUF = USCI_REGISTER(MODULE, INSTANCE, 7);

	static constexpr unsigned char idle_mode = LPM0_bits;

	static int tx_count;
	static int rx_count;
	static uint8_t *rx_buffer;
	static uint8_t *tx_buffer;

	static void init(void) {
		*CTL1 |= UCSWRST;
		if (MCTL != 0) *MCTL = 0x00;
		*CTL0 = UCCKPH | UCMSB | (MASTER ? UCMST : 0) | UCMODE_0 | UCSYNC;  // SPI mode 0, master
		*BR0 = (CLOCK::frequency / FREQUENCY) & 0xff;
		*BR1 = (CLOCK::frequency / FREQUENCY) >> 8;
		if (CLOCK::type == CLOCK_TYPE_ACLK) {
			*CTL1 = UCSSEL_1;
		} else {
			*CTL1 = UCSSEL_2;
		}
	}

	static void disable(void) {
	}

	static int ready(void) {
	}

	static void enable_rx_irq(void) {
		IE2 |= (MODULE == USCI_A ? UCA0RXIE : UCB0RXIE);
	}

	static uint8_t transfer(uint8_t data) {
		IE2 |= (MODULE == USCI_A ? UCA0RXIE : UCB0RXIE);
		*TXBUF = data;
		tx_count = 0;
		rx_buffer = 0;
		do {
			enter_idle(idle_mode);
		} while (*STAT & UCBUSY);
		return *RXBUF;
	}

	static void transfer(uint8_t *tx_data, int count, uint8_t *rx_data = 0) {
		enable_rx_irq();
		tx_buffer = tx_data + 1;
		tx_count = count - 1;
		rx_buffer = rx_data;
		rx_count = 0;
		*TXBUF = *tx_data;
		enter_idle(idle_mode);
	}

	static bool handle_irq(void) {
		bool resume = false;

		if (IFG2 & (MODULE == USCI_A ? UCA0RXIFG : UCB0RXIFG)) {
			clear_rx_irq();
			if (tx_count > 0) {
				enable_rx_irq();
				if (rx_buffer) {
					*rx_buffer = *RXBUF;
					rx_buffer++;
					rx_count++;
				} else {
					clear_rx_irq();
				}
				*TXBUF = *tx_buffer;
				tx_buffer++;
				tx_count--;
			} else {
				resume = true;
			}
		}
		return resume;
	}

	static void clear_rx_irq(void) {
		IFG2 &= ~(MODULE == USCI_A ? UCA0RXIFG : UCB0RXIFG);
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
int USCI_SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
int USCI_SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *USCI_SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::rx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long FREQUENCY, const int DATA_LENGTH, const bool LSB>
uint8_t *USCI_SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, FREQUENCY, DATA_LENGTH, LSB>::tx_buffer;

#endif
