#ifndef __USCI_UART_H
#define __USCI_UART_H

#include <stdint.h>
#include <clocks.h>
#include <usci.h>

struct USCI_UART_STATUS {
	bool framing_error: 1;
	bool rx_available: 1;
};

template<const USCI_MODULE MODULE,
	const int INSTANCE,
	typename CLOCK,
	const long SPEED = 9600>
struct USCI_UART_T {
	typedef USCI_T<MODULE, INSTANCE> USCI;

	static constexpr unsigned char idle_mode = LPM0_bits;
	static USCI_UART_STATUS status;
	static int tx_count;
	static int rx_count;
	static uint8_t *rx_buffer;
	static uint8_t *tx_buffer;

	static constexpr uint16_t USCI_DIV_INT = CLOCK::frequency / SPEED;
	static constexpr uint16_t USCI_BR0_VAL = USCI_DIV_INT & 0x00FF;
	static constexpr uint16_t USCI_BR1_VAL = (USCI_DIV_INT >> 8) & 0xFF;

	static constexpr uint16_t USCI_DIV_FRAC_NUMERATOR = CLOCK::frequency - (USCI_DIV_INT * SPEED);
	static constexpr uint16_t USCI_DIV_FRAC_NUM_X_8 = USCI_DIV_FRAC_NUMERATOR * 8;
	static constexpr uint16_t USCI_DIV_FRAC_X_8 = USCI_DIV_FRAC_NUM_X_8 / SPEED;

	static constexpr uint16_t USCI_BRS_VAL =
		((USCI_DIV_FRAC_NUM_X_8 - (USCI_DIV_FRAC_X_8 * SPEED)) * 10) / SPEED < 5 ?
			USCI_DIV_FRAC_X_8 << 1 :
			(USCI_DIV_FRAC_X_8+1) << 1;

	static void init(void) {
		*USCI::CTL1 |= UCSWRST;
		*USCI::BR0 = USCI_BR0_VAL;
		*USCI::BR1 = USCI_BR1_VAL;
		*USCI::MCTL = USCI_BRS_VAL;
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

	static void puts(const char *data) {
		while (*data) {
			putc(*data++);
		};
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
		static char getc() {
			status.rx_available = false;
			while (!(IFG2 & (MODULE == USCI_A ? UCA0RXIFG : UCB0RXIFG)) && !TIMEOUT::triggered());
			if (!TIMEOUT::triggered()) status.rx_available = true;
			return *USCI::RXBUF;
		}
};

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
int USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::tx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
int USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::rx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
uint8_t *USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::rx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
uint8_t *USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::tx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
USCI_UART_STATUS USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::status;

#endif
