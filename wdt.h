#ifndef __WDT_H
#define __WDT_H

#include <clocks.h>

enum WDT_MODE {
	WDT_TIMER,
	WDT_WATCHDOG
};

enum WDT_SOURCE {
	WDT_SMCLK,
	WDT_ACLK,
};

enum WDT_INTERVAL {
	WDT_INTERVAL_32768 = 0,
	WDT_INTERVAL_8192 = 1,
	WDT_INTERVAL_512 = 2,
	WDT_INTERVAL_64 = 3
};

template<typename CLOCK_SOURCE,
	const WDT_MODE MODE = WDT_WATCHDOG,
	const WDT_INTERVAL INTERVAL = WDT_INTERVAL_32768>
struct WDT_T {
	static void init(void) {
		WDTCTL = WDTPW + WDTCNTCL +
			(MODE == WDT_TIMER ? WDTTMSEL : 0) +
			(CLOCK_SOURCE::type == CLOCK_TYPE_ACLK ? WDTSSEL : 0) +
			INTERVAL;
	};

	static void enable_irq(void) {
		IE1 |= WDTIE;
	};

	static void disable_irq(void) {
		IE1 &= ~WDTIE;
	};

	static void enter_idle(void) {
		__bis_SR_register(LPM3_bits + GIE);
	};

	static inline void resume_irq(void) {
		__bic_SR_register_on_exit(LPM3_bits);
	};

	static void enable(void) {
		WDTCTL = WDTPW + (WDTCTL & (0x00ff & ~WDTHOLD)) + WDTPW;
	};

	static void disable(void) {
		WDTCTL = WDTPW + WDTHOLD;
	}
};

#endif
