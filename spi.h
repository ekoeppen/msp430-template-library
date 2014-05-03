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
	const long SPEED = 1000000,
	const int DATA_LENGTH = 8,
	const bool LSB = true>
struct SPI_T {
	static constexpr volatile unsigned char *CTL0 = USCI_REGISTER(MODULE, INSTANCE, 0);
	static constexpr volatile unsigned char *CTL1 = USCI_REGISTER(MODULE, INSTANCE, 1);
	static constexpr volatile unsigned char *BR0 = USCI_REGISTER(MODULE, INSTANCE, 2);
	static constexpr volatile unsigned char *BR1 = USCI_REGISTER(MODULE, INSTANCE, 3);
	static constexpr volatile unsigned char *MCTL = USCI_REGISTER(MODULE, INSTANCE, 4);
	static constexpr volatile unsigned char *STAT = USCI_REGISTER(MODULE, INSTANCE, 5);
	static constexpr volatile unsigned char *RXBUF = USCI_REGISTER(MODULE, INSTANCE, 6);
	static constexpr volatile unsigned char *TXBUF = USCI_REGISTER(MODULE, INSTANCE, 7);

	static int tx_count;
	static int rx_count;
	static uint8_t *rx_buffer;
	static uint8_t *tx_buffer;

	static void init(void) {
		*CTL1 |= UCSWRST;
		if (MCTL != 0) *MCTL = 0x00;
		*CTL0 = UCCKPH | UCMSB | (MASTER ? UCMST : 0) | UCMODE_0 | UCSYNC;  // SPI mode 0, master
		*BR0 = (CLOCK::speed / SPEED) & 0xff;
		*BR1 = (CLOCK::speed / SPEED) >> 8;
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
		do {
			enter_idle();
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
		enter_idle();
	}

	static inline void enter_idle(void) {
		__bis_SR_register(LPM0_bits + GIE);
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

	static inline void resume_irq(void) {
		__bic_SR_register_on_exit(LPM0_bits);
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

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long SPEED, const int DATA_LENGTH, const bool LSB>
int SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, SPEED, DATA_LENGTH, LSB>::tx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long SPEED, const int DATA_LENGTH, const bool LSB>
int SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, SPEED, DATA_LENGTH, LSB>::rx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long SPEED, const int DATA_LENGTH, const bool LSB>
uint8_t *SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, SPEED, DATA_LENGTH, LSB>::rx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const int MODE, const long SPEED, const int DATA_LENGTH, const bool LSB>
uint8_t *SPI_T<MODULE, INSTANCE, CLOCK, MASTER, MODE, SPEED, DATA_LENGTH, LSB>::tx_buffer;

#endif
