#ifndef __PRINTF_H
#define __PRINTF_H

#include <stdarg.h>
#include <tasks.h>

extern "C" char *itoa_ext(int value, unsigned int radix, unsigned int uppercase, unsigned int unsigned_value, int zero_pad);

template<class T, class U>
struct is_same {
	enum { value = 0 };
};

template<class T>
struct is_same<T, T> {
	enum { value = 1 };
};

struct DISABLED_UART {
	template<typename TIMEOUT = TIMEOUT_NEVER> static void init(void) { }
	template<typename TIMEOUT = TIMEOUT_NEVER> static void putc(char data) { }
	template<typename TIMEOUT = TIMEOUT_NEVER> static void puts(const char *data) { }
	template<typename TIMEOUT = TIMEOUT_NEVER> static char getc() { }
	static bool handle_rx_irq(void) { }
	static bool handle_tx_irq(void) { }
	static constexpr bool enabled(void) { return false; }
	static void enable(void) { }
	static void disable(void) { }
};

template<typename OUTPUT, typename TIMEOUT = TIMEOUT_NEVER>
void vprintf(const char *fmt, va_list va)
{
	char ch;

	while ((ch = *fmt++) != 0) {
		if (ch != '%') {
			OUTPUT::template putc<TIMEOUT>(ch);
		} else {
			char zero_pad = 0;

			ch = *fmt++;

			/* Zero padding requested */
			if (ch == '0') {
				ch = *fmt++;
				if (ch >= '0' && ch <= '9')
					zero_pad = ch - '0';
				ch = *fmt++;
			}

			switch (ch) {
				case 'u':
				case 'd':
				case 'x':
				case 'X':
					OUTPUT::template puts<TIMEOUT>(itoa_ext(va_arg(va, unsigned int),
								ch == 'u' || ch == 'd' ? 10 : 16,
								ch == 'X', ch == 'u', zero_pad));
					break;
				case 'c' :
					OUTPUT::template putc<TIMEOUT>((char)(va_arg(va, int)));
					break;
				case 's' :
					OUTPUT::template puts<TIMEOUT>(va_arg(va, char*));
					break;
				default:
					OUTPUT::template putc<TIMEOUT>(ch);
					break;
			}
		}
	}
}


template<typename OUTPUT, typename TIMEOUT = TIMEOUT_NEVER>
void printf(const char *fmt, ...)
{
	if (OUTPUT::enabled()) {
		va_list va;

		va_start(va, fmt);
		vprintf<OUTPUT, TIMEOUT>(fmt, va);
		va_end(va);
	}
}

template<typename INPUT, typename TIMEOUT = TIMEOUT_NEVER>
void gets(char *buffer, int count)
{
	while (count > 0) {
		*buffer = INPUT::template getc<TIMEOUT>();
		buffer++;
		count--;
	}
}

#endif
