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
	static constexpr unsigned char idle_mode = LPM3_bits;
	static constexpr int frequency = CLOCK_SOURCE::frequency /
		(INTERVAL == WDT_INTERVAL_32768 ? 32768 :
		(INTERVAL == WDT_INTERVAL_8192 ? 8192 :
		(INTERVAL == WDT_INTERVAL_512 ? 512 : 64)));

	static void init(void) {
		static_assert(frequency > 0, "WDT frequency can't be zero");
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

	static void enable(void) {
		WDTCTL = WDTPW + (WDTCTL & (0x00ff & ~WDTHOLD)) + WDTPW;
	};

	static void disable(void) {
		WDTCTL = WDTPW + WDTHOLD;
	}
};

#endif
