#ifndef __CLOCKS_H
#define __CLOCKS_H

enum CLOCK_TYPE {
	CLOCK_TYPE_ACLK,
	CLOCK_TYPE_SCLK,
	CLOCK_TYPE_SMCLK
};

enum ACLK_SOURCE {
	ACLK_SOURCE_VLOCLK,
	ACLK_SOURCE_LFXT1CLK
};

enum CLK_SOURCE {
	CLK_SOURCE_VLOCLK,
	CLK_SOURCE_LFXT1CLK,
	CLK_SOURCE_DCOCLK
};

struct DEFAULT_IDLER {
	static constexpr unsigned char idle_mode = LPM4_bits;
};

template<typename PERIPH0 = DEFAULT_IDLER,
	typename PERIPH1 = DEFAULT_IDLER,
	typename PERIPH2 = DEFAULT_IDLER,
	typename PERIPH3 = DEFAULT_IDLER>
inline void enter_idle(const unsigned char idle_mode = LPM4_bits) {
	__bis_SR_register((idle_mode & PERIPH0::idle_mode & PERIPH1::idle_mode & PERIPH2::idle_mode & PERIPH3::idle_mode) + GIE);
}

inline void exit_idle(void) {
	__bic_SR_register_on_exit(LPM4_bits);
}

template<const ACLK_SOURCE SOURCE = ACLK_SOURCE_VLOCLK,
	const int DIVIDER = 0>
struct ACLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_ACLK;
	static constexpr long frequency = (SOURCE == ACLK_SOURCE_VLOCLK ? 12000 : 32768);
	static constexpr unsigned char idle_mode = LPM3_bits;

	static void init(void) {
		if (SOURCE == ACLK_SOURCE_VLOCLK) BCSCTL3 |= LFXT1S_2;
	};
};

template<const CLK_SOURCE SOURCE = CLK_SOURCE_DCOCLK,
	const long FREQUENCY = 1000000>
struct SMCLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_SMCLK;
	static constexpr long frequency = FREQUENCY;
	static constexpr unsigned char idle_mode = LPM0_bits;

	static void init(void) {
		switch (frequency) {
			case 1000000:
			default:
				BCSCTL1 = CALBC1_1MHZ;
				DCOCTL = CALDCO_1MHZ;
				break;
		}
	};
};

template<typename CLOCK>
struct TIMEOUT_T {
	static unsigned long timeout;
	static constexpr unsigned char idle_mode = CLOCK::idle_mode;

	static void set_timeout(const unsigned long milliseconds) {
		timeout = milliseconds * CLOCK::frequency / 1000;
	};

	static inline bool timeout_triggered(void) {
		return (timeout && (--timeout == 0));
	};
};

template<typename CLOCK>
unsigned long TIMEOUT_T<CLOCK>::timeout;

#endif
