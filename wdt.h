#ifndef __WDT_H
#define __WDT_H

#include <clocks.h>

enum WDT_MODE {
	WDT_TIMER,
	WDT_WATCHDOG
};

enum WDT_INTERVAL {
	WDT_INTERVAL_32768 = 0,
	WDT_INTERVAL_8192 = 1,
	WDT_INTERVAL_512 = 2,
	WDT_INTERVAL_64 = 3
};

template<typename CLOCK,
	const WDT_MODE MODE = WDT_WATCHDOG,
	const WDT_INTERVAL INTERVAL = WDT_INTERVAL_32768>
struct WDT_T {
	static constexpr uint8_t idle_mode(void) { return LPM3_bits; }
	static constexpr int frequency = CLOCK::frequency /
		(INTERVAL == WDT_INTERVAL_32768 ? 32768 :
		(INTERVAL == WDT_INTERVAL_8192 ? 8192 :
		(INTERVAL == WDT_INTERVAL_512 ? 512 : 64)));

	static void init(void) {
		static_assert(frequency > 0, "WDT frequency can't be zero");
		WDTCTL = WDTPW + WDTCNTCL +
			(MODE == WDT_TIMER ? WDTTMSEL : 0) +
			(CLOCK::type == CLOCK_TYPE_ACLK ? WDTSSEL : 0) +
			INTERVAL;
		CLOCK::claim();
	};

	static void enable_irq(void) {
		IE1 |= WDTIE;
	};

	static void disable_irq(void) {
		IE1 &= ~WDTIE;
	};

	static bool irq_enabled(void) {
		return IE1 & WDTIE;
	}

	static void enable(void) {
		WDTCTL = WDTPW + (WDTCTL & (0x00ff & ~WDTHOLD)) + WDTPW;
		CLOCK::claim();
	};

	static void hold(void) {
		WDTCTL = WDTPW + WDTHOLD;
	}

	static bool enabled(void) {
		return !(WDTCTL & WDTHOLD);
	}

	static void disable(void) {
		hold();
		CLOCK::release();
	}
};

#endif
