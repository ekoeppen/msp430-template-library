#ifndef __FLASH_H
#define __FLASH_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern unsigned int __infoa;
extern unsigned int __infob;
extern unsigned int __infoc;
extern unsigned int __infod;

template<typename CLOCK, typename WDT,
	unsigned int *BEGIN>
struct FLASH_T {
	static constexpr bool ftg_ok(const uint8_t divider) {
		return CLOCK::frequency / (unsigned long) divider > 257000L && CLOCK::frequency / (unsigned long) divider < 476000L;
	}

	static constexpr uint8_t calculate_divider(const uint8_t divider) {
		return divider <= 64 ? (ftg_ok(divider) ? divider : calculate_divider(divider + 1)) : 255;
	}

	static void write(const void *data = NULL, unsigned size = 64) {
		constexpr uint8_t fn = calculate_divider(1);
		volatile unsigned *flash = BEGIN;
		const unsigned *p = reinterpret_cast<const unsigned *>(data);
		bool wdt_enabled = WDT::enabled();
		bool wdt_irq_enabled = WDT::irq_enabled();
		bool gie_enabled = __read_status_register() & GIE;

		static_assert(fn <= 64, "Can not calculate flash clock");
		if (gie_enabled) __bic_SR_register(GIE);
		if (wdt_irq_enabled) WDT::disable_irq();
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
		size = (size + 1) / 2;
		while (size-- > 0) {
			*flash++ = *p++;
		}
		FCTL3 = FWKEY + LOCK;

		if (wdt_enabled) WDT::init();
		if (wdt_irq_enabled) WDT::enable_irq();
		if (gie_enabled) __bis_SR_register(GIE);
	}

};

#endif
