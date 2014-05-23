#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include <clocks.h>
#include <usci.h>

template<const USCI_MODULE MODULE,
	const int INSTANCE,
	typename CLOCK,
	const long SPEED = 9600>
struct UART_T {
	typedef USCI_T<MODULE, INSTANCE> USCI;

	static constexpr unsigned char idle_mode = LPM0_bits;

	static int tx_count;
	static int rx_count;
	static uint8_t *rx_buffer;
	static uint8_t *tx_buffer;

	static void init(void) {
		*USCI::CTL1 |= UCSWRST;
		*USCI::BR0 = 0x68;
		*USCI::BR1 = 0x00;
		*USCI::MCTL = UCBRS2 + UCBRS0;
		if (CLOCK::type == CLOCK_TYPE_ACLK) {
			*USCI::CTL1 = UCSSEL_1;
		} else {
			*USCI::CTL1 = UCSSEL_2;
		}
		*USCI::CTL1 &= ~UCSWRST;
	}

	static void disable(void) {
	}

	static int ready(void) {
	}

	static void transfer(uint8_t *tx_data, int count) {
		USCI::enable_rx_irq();
		tx_buffer = tx_data + 1;
		tx_count = count - 1;
		*USCI::TXBUF = *tx_data;
		USCI::enter_idle();
	}

	static bool handle_irq(void) {
		bool resume = false;

		return resume;
	}

	static void putc(char data) {
		while (!(IFG2 & (MODULE == USCI_A ? UCA0TXIFG : UCB0TXIFG))) ;
		*USCI::TXBUF = data;
	}

	static void puts(char *data) {
		while (*data) {
			putc(*data++);
		};
	}

	static char getc() {
		while (!(IFG2 & (MODULE == USCI_A ? UCA0RXIFG : UCB0RXIFG))) ;
		return *USCI::RXBUF;
	}
};

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
int UART_T<MODULE, INSTANCE, CLOCK, SPEED>::tx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
int UART_T<MODULE, INSTANCE, CLOCK, SPEED>::rx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
uint8_t *UART_T<MODULE, INSTANCE, CLOCK, SPEED>::rx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
uint8_t *UART_T<MODULE, INSTANCE, CLOCK, SPEED>::tx_buffer;

#endif
