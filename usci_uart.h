#ifndef __USCI_UART_H
#define __USCI_UART_H

#include <stdint.h>
#include <clocks.h>
#include <tasks.h>
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

	static USCI_UART_STATUS status;
	static volatile int tx_count;
	static volatile uint8_t rx_count;
	static volatile uint8_t rx_max;
	static uint8_t* rx_buffer;
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
		rx_buffer = 0;
		USCI::enable_rx_irq();
	}

	static void enable(void) {
	}

	static constexpr bool enabled(void) { return true; }

	static void disable(void) {
	}

	static inline void enter_idle(void) {
		__bis_SR_register(CLOCK::idle_mode + GIE);
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(uint8_t *tx_data, int count) {
		CLOCK::claim();
		tx_buffer = tx_data;
		tx_count = count;
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
				*USCI::TXBUF = *tx_buffer;
				tx_buffer++;
				tx_count--;
			} else {
				USCI::disable_tx_irq();
				resume = true;
			}
		}
		return resume;
	}

	static bool handle_rx_irq(void) {
		bool resume = false;

		if (USCI::rx_irq_pending()) {
			if (rx_buffer && rx_count < rx_max) {
				*rx_buffer = *USCI::RXBUF;
				rx_buffer++;
				rx_count++;
			}
			USCI::clear_rx_irq();
			resume = true;
		}
		return resume;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void putc(char data) {
		CLOCK::claim();
		tx_buffer = (uint8_t *) &data;
		tx_count = 1;
		USCI::enable_tx_irq();
		while (!TIMEOUT::triggered() && tx_count > 0) {
			enter_idle();
		}
		CLOCK::release();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void puts(const char *data) {
		unsigned n;

		for (n = 0; data[n] != '\0'; n++) ;
		transfer<TIMEOUT>((uint8_t *) data, n);
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static char getc() {
		char c;
		status.rx_available = false;
		rx_count = 0;
		rx_max = 1;
		rx_buffer = (uint8_t *) &c;
		while (!TIMEOUT::triggered() && rx_count == 0) {
			enter_idle();
		}
		if (!TIMEOUT::triggered()) status.rx_available = true;
		return c;
	}
};

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
volatile int USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::tx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
volatile uint8_t USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::rx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
volatile uint8_t USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::rx_max;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
uint8_t *USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::rx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
uint8_t *USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::tx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const long SPEED>
USCI_UART_STATUS USCI_UART_T<MODULE, INSTANCE, CLOCK, SPEED>::status;

#endif
