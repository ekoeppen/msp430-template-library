#ifndef __USCI_I2C_H
#define __USCI_I2C_H

#include <stdint.h>
#include <clocks.h>
#include <tasks.h>
#include <usci.h>

struct USCI_I2C_FLAGS {
	bool restart: 1;
	bool busy: 1;
};

template<const USCI_MODULE MODULE,
	const int INSTANCE,
	typename CLOCK,
	const bool MASTER = true,
	const long FREQUENCY = 100000>
struct USCI_I2C_T {
	typedef USCI_T<MODULE, INSTANCE> USCI;

	volatile static int rx_tx_count;
	static uint8_t *rx_tx_buffer;
	static USCI_I2C_FLAGS flags;

	static void init(void) {
		*USCI::CTL1 |= UCSWRST;
		*USCI::CTL0 = (MASTER ? UCMST : 0) | UCMODE_3 | UCSYNC;
		*USCI::BR0 = (CLOCK::frequency / FREQUENCY) & 0xff;
		*USCI::BR1 = (CLOCK::frequency / FREQUENCY) >> 8;
		if (CLOCK::type == CLOCK_TYPE_ACLK) {
			*USCI::CTL1 = UCSSEL_1;
		} else {
			*USCI::CTL1 = UCSSEL_2;
		}
		rx_tx_count = 0;
		USCI::disable_rx_tx_irq();
	}

	static void enable(void) {
	}

	static void disable(void) {
	}

	static void set_slave_addr(const uint8_t slave_addr) {
		*USCI::I2CSA = slave_addr;
	}

	static void stop(void) {
		*USCI::CTL1 |= UCTXSTP;
		while (*USCI::CTL1 & UCTXSTP);
	}

	static bool slave_present(const uint8_t slave_address){
		uint8_t ie2_bak, slaveadr_bak, ucb0i2cie;
		bool r;
		ucb0i2cie = *USCI::I2CIE;
		ie2_bak = IE2;
		*USCI::I2CIE &= ~UCNACKIE;
		*USCI::I2CSA = slave_address;
		IE2 &= ~(UCB0TXIE + UCB0RXIE);
		__disable_interrupt();
		*USCI::CTL1 |= UCTR + UCTXSTT + UCTXSTP;
		while (*USCI::CTL1 & UCTXSTP);

		r = !(*USCI::STAT & UCNACKIFG);
		__enable_interrupt();
		IE2 = ie2_bak;
		*USCI::I2CIE = ucb0i2cie;
		return r;
	}


#ifdef I2C_POLLING
	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void write(const uint8_t *data, int length, bool restart = false) {
		CLOCK::claim();
		*USCI::CTL1 |= UCTR | UCTXSTT;
		while (length--) {
			while (!USCI::tx_irq_pending());
			*USCI::TXBUF = *data++;
		}
		while (!USCI::tx_irq_pending());
		if (!restart) {
			*USCI::CTL1 |= UCTXSTP;
		}
		CLOCK::release();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void read(uint8_t *data, uint16_t length) {
		CLOCK::claim();
		*USCI::CTL1 &= ~UCTR;
		*USCI::CTL1 |= UCTXSTT;
		while (!TIMEOUT::triggered() && (*USCI::CTL1 & UCTXSTT));
		while (length--) {
			if (length == 0) {
				*USCI::CTL1 |= UCTXSTP;
			}
			while (!USCI::rx_irq_pending());
			*data++ = *USCI::RXBUF;
		}
		while (*USCI::STAT & UCBBUSY);
		CLOCK::release();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void write_reg(const uint8_t reg, const uint8_t value) {
		CLOCK::claim();
		*USCI::CTL1 |= UCTR | UCTXSTT;
		while (!USCI::tx_irq_pending());
		*USCI::TXBUF = reg;
		while (*USCI::CTL1 & UCTXSTT);
		while (!USCI::tx_irq_pending());
		*USCI::TXBUF = value;
		while (!USCI::tx_irq_pending());
		*USCI::CTL1 |= UCTXSTP;
		while (*USCI::STAT & UCBBUSY);
		CLOCK::release();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static uint8_t read_reg(const uint8_t reg) {
		uint8_t r;
		CLOCK::claim();
		*USCI::CTL1 |= UCTR | UCTXSTT;
		while (!USCI::tx_irq_pending());
		*USCI::TXBUF = reg;
		while (*USCI::CTL1 & UCTXSTT);
		*USCI::CTL1 &= ~UCTR;
		*USCI::CTL1 |= UCTXSTT;
		while (*USCI::CTL1 & UCTXSTT);
		*USCI::CTL1 |= UCTXSTP;
		while (!USCI::rx_irq_pending());
		r = *USCI::RXBUF;
		while (*USCI::STAT & UCBBUSY);
		CLOCK::release();
		return r;
	}

	static void read_reg(const uint8_t reg, uint8_t *data, uint16_t length) {
		CLOCK::claim();
		*USCI::CTL1 |= UCTR | UCTXSTT;
		while (!USCI::tx_irq_pending());
		*USCI::TXBUF = reg;
		while (*USCI::CTL1 & UCTXSTT);
		read(data, length);
		CLOCK::release();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(const uint8_t *data, int length, bool direction, bool restart = false) {
		if (direction) write(data, length, restart);
		else read(const_cast<uint8_t *>(data), length);
	}
#else

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(const uint8_t *data, int length, bool write, bool restart = false) {
		CLOCK::claim();
		flags.restart = restart;
		rx_tx_buffer = const_cast<uint8_t *>(data);
		rx_tx_count = length;
		if (write) {
			*USCI::CTL1 |= UCTR | UCTXSTT;
			while (!USCI::tx_irq_pending());
			*USCI::TXBUF = *rx_tx_buffer++;
			rx_tx_count--;
			while (!TIMEOUT::triggered() && (*USCI::CTL1 & UCTXSTT));
		} else {
			*USCI::CTL1 &= ~UCTR;
			*USCI::CTL1 |= UCTXSTT;
			if (length == 1) {
				while (!TIMEOUT::triggered() && (*USCI::CTL1 & UCTXSTT));
				*USCI::CTL1 |= UCTXSTP;
			}
		}

		USCI::enable_rx_tx_irq();
		while (!TIMEOUT::triggered() && rx_tx_count > 0) {
			enter_idle();
		}
		USCI::disable_rx_tx_irq();
		CLOCK::release();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void write_reg(const uint8_t reg, const uint8_t value) {
		uint8_t buffer[2];
		buffer[0] = reg; buffer[1] = value;
		transfer(buffer, sizeof(buffer), true, false);
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static uint8_t read_reg(const uint8_t reg) {
		uint8_t buffer;
		buffer = reg;
		transfer(&buffer, 1, true, true);
		transfer(&buffer, 1, false);
		return buffer;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void write(const uint8_t *data, int length, bool restart = false) {
		transfer(data, length, true, restart);
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void read(uint8_t *data, uint16_t length) {
		transfer(data, length, false, false);
	}

	static void read_reg(const uint8_t reg, uint8_t *data, uint16_t length) {
		transfer(&reg, 1, true, true);
		transfer(data, length, false);
	}

	static bool handle_tx_irq(void) {
		bool resume = false;
		if (USCI::rx_irq_pending()) {
			uint8_t rx_data = *USCI::RXBUF;
			if (rx_tx_count) {
				if (rx_tx_count <= 2) {
					*USCI::CTL1 |= UCTXSTP;
					resume = true;
				}
				*rx_tx_buffer++ = rx_data;
				rx_tx_count--;
			}
		}
		if (USCI::tx_irq_pending()) {
			if (rx_tx_count) {
				*USCI::TXBUF = *rx_tx_buffer++;
				rx_tx_count--;
			} else {
				if (!flags.restart) {
					*USCI::CTL1 |= UCTXSTP;
				}
				USCI::clear_tx_irq();
				resume = true;
			}
		}
		return resume;
	}

	static void handle_rx_irq(void) {
		if (*USCI::STAT & UCNACKIFG) {
			*USCI::CTL1 |= UCTXSTT;
			*USCI::STAT &= ~UCNACKIFG;
		}
	}
#endif
};

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const long FREQUENCY>
volatile int USCI_I2C_T<MODULE, INSTANCE, CLOCK, MASTER, FREQUENCY>::rx_tx_count;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const long FREQUENCY>
uint8_t *USCI_I2C_T<MODULE, INSTANCE, CLOCK, MASTER, FREQUENCY>::rx_tx_buffer;

template<const USCI_MODULE MODULE, const int INSTANCE, typename CLOCK, const bool MASTER, const long FREQUENCY>
USCI_I2C_FLAGS USCI_I2C_T<MODULE, INSTANCE, CLOCK, MASTER, FREQUENCY>::flags;

#endif

