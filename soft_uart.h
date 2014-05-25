#ifndef __UART_H
#define __UART_H

#include <stdint.h>
#include <clocks.h>

struct SOFT_UART_STATUS {
	unsigned int bit_count: 4;
	bool is_receiving: 1;
	bool has_received: 1;
};

template<typename TIMER,
	typename TX,
	typename RX,
	const long SPEED = 9600>
struct SOFT_UART_T {
	static constexpr unsigned char idle_mode = TIMER::idle_mode;
	static constexpr unsigned int bit_time = 1000000 / SPEED - 16;
	static constexpr unsigned int half_bit_time = bit_time / 2;
	static const uint16_t bit_offsets[9];

	static SOFT_UART_STATUS status;
	static char tx_byte;
	static char rx_byte;

	static void init(void) {
		// static_assert(TIMER::frequency == 1000000, "Timer must run at 1MHz");
	}

	static void disable(void) {
	}

	static int ready(void) {
		return 0;
	}

	static void transfer(uint8_t *tx_data, int count) {
	}

	static void putc(char data) {
		unsigned st;
		int i;

		TX::set_high();
		st = TIMER::counter();
		TX::set_low();
		while (TIMER::counter() - st < bit_offsets[0]);
		for (i = 1; i < 9; i++) {
			TX::set(data & 0x80 == 0x80);
			while (TIMER::counter() - st < bit_offsets[i]);
			data >>= 1;
		}
		TX::set_high();
		while (TIMER::counter() - st < bit_offsets[i]);
#if 0
		while (status.is_receiving); 					// Wait for RX completion
		tx_byte = data;

		CCTL0 = OUT; 							// TXD Idle as Mark

		status.bit_count = 10; 						// Load Bit counter, 8 bits + ST/SP
		CCR0 = TAR + bit_time; 						// Set time till first bit
		tx_byte |= 0x100; 						// Add stop bit to TXByte (which is logical 1)
		tx_byte = tx_byte << 1; 					// Add start bit (which is logical 0)

		CCTL0 = CCIS_0 + OUTMOD_0 + CCIE + OUT; // Set signal, intial value, enable interrupts

		while ( CCTL0 & CCIE ); 				// Wait for previous TX completion
#endif
	}

	static void puts(char *data) {
		while (*data) {
			putc(*data++);
		};
	}

	static char getc(void) {
		status.has_received = false;
		return rx_byte;
	}
};

template<typename TIMER, typename TX, typename RX, const long SPEED>
SOFT_UART_STATUS SOFT_UART_T<TIMER, TX, RX, SPEED>::status;
template<typename TIMER, typename TX, typename RX, const long SPEED>
char SOFT_UART_T<TIMER, TX, RX, SPEED>::tx_byte;
template<typename TIMER, typename TX, typename RX, const long SPEED>
char SOFT_UART_T<TIMER, TX, RX, SPEED>::rx_byte;
template<typename TIMER, typename TX, typename RX, const long SPEED>
const uint16_t SOFT_UART_T<TIMER, TX, RX, SPEED>::bit_offsets[9] = {
	1000000 / SPEED,
	1000000 / SPEED * 2,
	1000000 / SPEED * 3,
	1000000 / SPEED * 4,
	1000000 / SPEED * 5,
	1000000 / SPEED * 6,
	1000000 / SPEED * 7,
	1000000 / SPEED * 8,
	1000000 / SPEED * 9
};

#endif
