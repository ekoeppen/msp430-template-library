#ifndef __CLOCKS_H
#define __CLOCKS_H

enum CLOCK_TYPE {
	CLOCK_TYPE_ACLK,
	CLOCK_TYPE_MCLK,
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
	const int FREQUENCY = 0,
	const int DIVIDER = 0>
struct ACLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_ACLK;
	static constexpr long frequency = (FREQUENCY == 0 ? (SOURCE == ACLK_SOURCE_VLOCLK ? 12000 : 32768) : FREQUENCY);
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
		if (SOURCE == CLK_SOURCE_DCOCLK) {
			BCSCTL2 &= ~SELS;
			switch (frequency) {
			case 8000000:
				BCSCTL1 = CALBC1_8MHZ;
				DCOCTL = CALDCO_8MHZ;
				break;
			case 12000000:
				BCSCTL1 = CALBC1_12MHZ;
				DCOCTL = CALDCO_12MHZ;
				break;
			case 16000000:
				BCSCTL1 = CALBC1_16MHZ;
				DCOCTL = CALDCO_16MHZ;
			case 1000000:
				BCSCTL1 = CALBC1_1MHZ;
				DCOCTL = CALDCO_1MHZ;
				break;
			default:
				BCSCTL1 = CALBC1_1MHZ;
				DCOCTL = CALDCO_1MHZ;
				break;
			}
		} else {
			BCSCTL2 |= SELS;
		}
	};
};

template<const CLK_SOURCE SOURCE = CLK_SOURCE_DCOCLK,
	const long FREQUENCY = 1000000>
struct MCLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_MCLK;
	static constexpr long frequency = FREQUENCY;
	static constexpr unsigned char idle_mode = LPM0_bits;

	static void init(void) {
		switch (frequency) {
			case 8000000:
				BCSCTL1 = CALBC1_8MHZ;
				DCOCTL = CALDCO_8MHZ;
				break;
			case 12000000:
				BCSCTL1 = CALBC1_12MHZ;
				DCOCTL = CALDCO_12MHZ;
				break;
			case 16000000:
				BCSCTL1 = CALBC1_16MHZ;
				DCOCTL = CALDCO_16MHZ;
			case 1000000:
				BCSCTL1 = CALBC1_1MHZ;
				DCOCTL = CALDCO_1MHZ;
				break;
			default:
				BCSCTL1 = CALBC1_1MHZ;
				DCOCTL = CALDCO_1MHZ;
				break;
		}
	};
};

template<typename CLOCK>
struct TIMEOUT_T {
	static volatile unsigned long timeout;
	static constexpr unsigned char idle_mode = CLOCK::idle_mode;

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
};

struct TIMEOUT_NEVER {
	static constexpr unsigned char idle_mode = LPM0_bits;

	static void set(const unsigned long milliseconds) { }
	static inline bool count_down(void) { return false; }
	static inline bool triggered(void) { return false; }
	static inline unsigned long get() { return 1; }
};

struct TIMEOUT_IMMEDIATELY {
	static constexpr unsigned char idle_mode = LPM0_bits;

	static void set(const unsigned long milliseconds) { }
	static inline bool count_down(void) { return true; }
	static inline bool triggered(void) { return true; }
	static inline unsigned long get() { return 0; }
};

template<typename CLOCK>
volatile unsigned long TIMEOUT_T<CLOCK>::timeout;

#endif
