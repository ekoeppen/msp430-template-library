#ifndef __UTILS_H
#define __UTILS_H

#include <io.h>

extern "C" {

char *itoa_ext(int value, unsigned int radix, unsigned int uppercase, int zero_pad);

}

template<typename OUTPUT>
void hex_dump_bytes(uint8_t *start, int count, const char *title = 0)
{
	if (title) printf<OUTPUT>("%s:\n", title);
	while (count > 0) {
		printf<OUTPUT>("%04x: %02x %02x %02x %02x\n",
			       start, start[0], start[1], start[2], start[3]);
		start += 4;
		count -= 4;
	}
}

template<typename OUTPUT>
void hex_dump_short(uint16_t *start, int count, const char *title = 0)
{
	if (title) printf<OUTPUT>("%s:\n", title);
	while (count > 0) {
		printf<OUTPUT>("%04x: %04x %04x %04x %04x\n",
			       start, start[0], start[1], start[2], start[3]);
		start += 8;
		count -= 8;
	}
}

#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))
#define ABS_DIFF(a, b) (a > b ? a - b : b - a)
#define hex_dump_periph(OUTPUT, PERIPH) hex_dump_words<OUTPUT>((uint16_t *) PERIPH, sizeof(*PERIPH), #PERIPH)
#define FROM_HEX(c) (c >= '0' && c <= '9' ? c - '0' : (c >= 'A' && c <= 'F' ? c - 'A' + 10 : (c >= 'a' && c <= 'f' ? c - 'a' + 10 : 0)))

#endif
