#ifndef __CLOCKS_H
#define __CLOCKS_H

enum CLOCK_TYPE {
	CLOCK_TYPE_ACLK,
	CLOCK_TYPE_MCLK,
	CLOCK_TYPE_SMCLK,
	CLOCK_TYPE_ADCOSC
};

enum CLOCK_SOURCE_TYPE {
	CLOCK_SOURCE_TYPE_VLO,
	CLOCK_SOURCE_TYPE_LFXT1,
	CLOCK_SOURCE_TYPE_DCO,
};

uint8_t VLOCLK_usage_count;
uint8_t LFXT1CLK_usage_count;
uint8_t DCOCLK_usage_count;

inline void enter_idle(void) {
	__bis_SR_register(GIE + CPUOFF +
		(VLOCLK_usage_count == 0 && LFXT1CLK_usage_count == 0 && DCOCLK_usage_count == 0 ? OSCOFF : 0) +
		(DCOCLK_usage_count == 0 ? SCG0 + SCG1 : 0));
}

inline void exit_idle(void) {
	__bic_SR_register_on_exit(LPM4_bits);
}

void delay_ms(uint8_t milliseconds) {
	uint8_t m;
	switch (BCSCTL1 & 0b00001111) {
		case 6: m = 1; break;
		case 13: m = 8; break;
		case 14: m = 12; break;
		default: m = 16; break;
	}
	while (milliseconds--) {
		for (int i = 0; i < m; i++) __delay_cycles(1000);
	}
}

void delay_us(int16_t microseconds) {
	switch (BCSCTL1 & 0b00001111) {
		case 6: while (microseconds--) __delay_cycles(1); break;
		case 13: while (microseconds--) __delay_cycles(8); break;
		case 14: while (microseconds--) __delay_cycles(12); break;
		default: while (microseconds--) __delay_cycles(16); break;
	}
}

template<const uint16_t FREQUENCY = 12000, const bool DEBUG_RELEASE = false>
struct VLOCLK_T {
	static constexpr CLOCK_SOURCE_TYPE type = CLOCK_SOURCE_TYPE_VLO;
	static constexpr uint16_t frequency = FREQUENCY;
	static void claim(void) { VLOCLK_usage_count++; }
	static void release(void) {
		if (!DEBUG_RELEASE || VLOCLK_usage_count > 0) {
			VLOCLK_usage_count--;
		} else {
			while (1);
		}
	}
};


template<const uint8_t CAPS, const uint16_t FREQUENCY = 32768, const bool DEBUG_RELEASE = false>
struct LFXT1CLK_T {
	static constexpr CLOCK_SOURCE_TYPE type = CLOCK_SOURCE_TYPE_LFXT1;
	static constexpr uint16_t frequency = FREQUENCY;

	static void init(void) {
		__bis_SR_register(OSCOFF);
		BCSCTL3 |= CAPS;
		__bic_SR_register(OSCOFF);
		while (IFG1 & OFIFG) IFG1 &= ~OFIFG;
	}

	static void claim(void) { LFXT1CLK_usage_count++; }
	static void release(void) {
		if (!DEBUG_RELEASE || LFXT1CLK_usage_count > 0) {
			LFXT1CLK_usage_count--;
		} else {
			while (1);
		}
	}
};


template<const uint32_t FREQUENCY = 1000000, const bool DEBUG_RELEASE = false>
struct DCOCLK_T {
	static constexpr CLOCK_SOURCE_TYPE type = CLOCK_SOURCE_TYPE_DCO;
	static constexpr uint32_t frequency = FREQUENCY;
	static void init(void) {
		switch (FREQUENCY) {
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
	}
	static void claim(void) { DCOCLK_usage_count++; }
	static void release(void) {
		if (!DEBUG_RELEASE || DCOCLK_usage_count > 0) {
			DCOCLK_usage_count--;
		} else {
			while (1);
		}
	}
};

template<typename SOURCE,
	const int DIVIDER = 1>
struct ACLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_ACLK;
	static constexpr long frequency = SOURCE::frequency / DIVIDER;

	static void init(void) {
		if (SOURCE::type == CLOCK_SOURCE_TYPE_VLO) BCSCTL3 |= LFXT1S_2;
	};

	static void claim(void) { SOURCE::claim(); }
	static void release(void) { SOURCE::release(); }
};

template<typename SOURCE,
	const int DIVIDER = 1>
struct SMCLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_SMCLK;
	static constexpr long frequency = SOURCE::frequency / DIVIDER;

	static void init(void) {
		if (DIVIDER > 1) {
			BCSCTL2 &= DIVS_3;
			BCSCTL2 |= (DIVIDER == 2 ? DIVS_1 : (DIVIDER == 4 ? DIVS_2 : DIVS_3));
		}
		if (SOURCE::type != CLOCK_SOURCE_TYPE_DCO) {
			BCSCTL2 |= SELS;
		}
	};

	static void claim(void) { SOURCE::claim(); }
	static void release(void) { SOURCE::release(); }

	static void set_and_wait_us(uint8_t microseconds) {
		while (microseconds--) {
			__delay_cycles(frequency / 1000000);
		}
	}

	static void set_and_wait(uint8_t milliseconds) {
		while (milliseconds--) {
			__delay_cycles(frequency / 1000);
		}
	}

};

template<typename SOURCE,
	const int DIVIDER = 1>
struct MCLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_MCLK;
	static constexpr long frequency = SOURCE::frequency / DIVIDER;

	static void init(void) {
		switch (SOURCE::type) {
			case CLOCK_SOURCE_TYPE_DCO:
				break;
			case CLOCK_SOURCE_TYPE_VLO:
			case CLOCK_SOURCE_TYPE_LFXT1:
				BCSCTL2 |= SELM_3;
				break;
		}
	};

	static void set_and_wait_us(uint8_t microseconds) {
		while (microseconds--) {
			__delay_cycles(frequency / 1000000);
		}
	}

	static void set_and_wait(uint8_t milliseconds) {
		while (milliseconds--) {
			__delay_cycles(frequency / 1000);
		}
	}

	static void claim(void) { SOURCE::claim(); }
	static void release(void) { SOURCE::release(); }
};

#endif
