#ifndef __TLV_H
#define __TLV_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern unsigned int __infoa;
extern unsigned int __infob;
extern unsigned int __infoc;
extern unsigned int __infod;

template<typename CLOCK, typename WDT,
	unsigned int *BEGIN>
struct TLV_T {
	static unsigned checksum(const unsigned *p = BEGIN + 1, const unsigned *end = BEGIN + 32) {
		unsigned chk = 0;
		while (p < end) chk ^= *p++;
		return chk;
	}

	static bool verify_checksum(void) {
		return checksum() + *BEGIN == 0;
	}

	static unsigned *find_tag(const uint8_t tag, unsigned *p = BEGIN + 1, const unsigned *end = BEGIN + 32) {
		do {
			if ((*p & 0xFF) == tag) return p;
			p += (*p >> 9) + 1;
		} while (p < end);
		return 0;
	}

	template<typename ITERATOR>
	static void iterate(void) {
		unsigned *p = BEGIN + 1;
		do {
			unsigned d = *p++;
			ITERATOR::handle_tag(d & 0xff, d >> 8, reinterpret_cast<void *>(p));
			p += (d >> 9);
		} while (p < BEGIN + 32);
	}

	static constexpr bool ftg_ok(const uint8_t divider) {
		return CLOCK::frequency / (unsigned long) divider > 257000L && CLOCK::frequency / (unsigned long) divider < 476000L;
	}

	static constexpr uint8_t calculate_divider(const uint8_t divider) {
		return divider <= 64 ? (ftg_ok(divider) ? divider : calculate_divider(divider + 1)) : 255;
	}

	static void write(const unsigned *p = NULL, const unsigned *end = NULL) {
		constexpr uint8_t fn = calculate_divider(1);
		volatile unsigned *flash = BEGIN + 1;
		bool wdt_enabled = WDT::enabled();
		bool gie_enabled = __read_status_register() & GIE;

		static_assert(fn <= 64, "Can not calculate flash clock");
		if (gie_enabled) __bic_SR_register(GIE);
		if (wdt_enabled) WDT::disable();

		while (FCTL3 & BUSY);
		FCTL2 = FWKEY + FSSEL_2 + (fn - 1);
		FCTL3 = FWKEY;
		FCTL1 = FWKEY + ERASE;
		BEGIN[0] = 0;
		FCTL3 = FWKEY + LOCK;

		__delay_cycles(1000);

		while (FCTL3 & BUSY);
		FCTL3 = FWKEY;
		FCTL1 = FWKEY + WRT;
		while (p < end) {
			*flash++ = *p++;
		}
		*flash = 0xfe | (((BEGIN + 32 - flash - 1) * 2) << 8);
		flash = BEGIN;
		*flash = ((checksum() ^ 0xffff) + 1);
		FCTL3 = FWKEY + LOCK;

		if (wdt_enabled) WDT::enable();
		if (gie_enabled) __bis_SR_register(GIE);
	}

	static void set_tag(const uint8_t tag, const void *value) {
		unsigned info[31];
		unsigned *p;

		for (int i = 0; i < 31; i++) {
			info[i] = BEGIN[i + 1];
		}
		p = find_tag(tag, info, info + 31);
		if (p != NULL) {
			uint8_t len = *p >> 8;
			memcpy(p + 1, value, len);
			write(info, info + 31);
		}
	}
};

#endif
