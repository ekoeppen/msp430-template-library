#ifndef __PRINTF_H
#define __PRINTF_H

#include <stdarg.h>

extern "C" char *itoa_ext(int value, unsigned int radix, unsigned int uppercase, int zero_pad);

template <typename OUTPUT>
void vprintf(const char *fmt, va_list va)
{
	char ch;

	while ((ch = *fmt++) != 0) {
		if (ch != '%') {
			OUTPUT::putc(ch);
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
					OUTPUT::puts(itoa_ext(va_arg(va, unsigned int),
								ch == 'u' || ch == 'd' ? 10 : 16,
								ch == 'X', zero_pad));
					break;
				case 'c' :
					OUTPUT::putc((char)(va_arg(va, int)));
					break;
				case 's' :
					OUTPUT::puts(va_arg(va, char*));
					break;
				default:
					OUTPUT::putc(ch);
					break;
			}
		}
	}
}


template <typename OUTPUT>
void printf(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vprintf<OUTPUT>(fmt, va);
	va_end(va);
}

template <typename INPUT>
void gets(char *buffer, int count)
{
	while (count > 0) {
		*buffer = INPUT::getc();
		buffer++;
		count--;
	}
}

#endif
