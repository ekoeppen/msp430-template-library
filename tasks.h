#ifndef __TASKS_H
#define __TASKS_H

#include <clocks.h>

void enter_idle(void) {
	__bis_SR_register(GIE + CPUOFF +
			(VLOCLK_usage_count == 0 && LFXT1CLK_usage_count == 0 ? OSCOFF : 0) +
			(DCOCLK_usage_count == 0 ? SCG0 + SCG1 : 0));
}

inline void exit_idle(void) {
	__bic_SR_register_on_exit(LPM4_bits);
}

template<typename CLOCK>
struct TIMEOUT_T {
	static volatile unsigned long timeout;

	static void set(const unsigned long milliseconds) {
		__bic_SR_register(GIE);
		timeout = milliseconds * CLOCK::frequency / 1000;
		__bis_SR_register(GIE);
	};

	static inline bool count_down(void) {
		return (!timeout || (--timeout == 0));
	};

	static inline bool triggered(void) {
		return timeout == 0;
	};

	static inline unsigned long get() {
		return timeout;
	};

	static void set_and_wait(const unsigned long milliseconds) {
		set(milliseconds);
		while (!triggered()) {
			enter_idle();
		}
	}
};

struct TIMEOUT_NEVER {
	static void set(const unsigned long milliseconds) { }
	static inline bool count_down(void) { return false; }
	static inline bool triggered(void) { return false; }
	static inline unsigned long get() { return 1; }
};

struct TIMEOUT_IMMEDIATELY {
	static void set(const unsigned long milliseconds) { }
	static inline bool count_down(void) { return true; }
	static inline bool triggered(void) { return true; }
	static inline unsigned long get() { return 0; }
};

template<typename CLOCK>
volatile unsigned long TIMEOUT_T<CLOCK>::timeout;

#endif
