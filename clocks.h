#ifndef __CLOCKS_H
#define __CLOCKS_H

enum CLOCK_TYPE {
	CLOCK_TYPE_ACLK,
	CLOCK_TYPE_MCLK,
	CLOCK_TYPE_SMCLK
};

enum CLOCK_SOURCE_TYPE {
	CLOCK_SOURCE_TYPE_VLO,
	CLOCK_SOURCE_TYPE_LFXT1,
	CLOCK_SOURCE_TYPE_DCO
};

uint8_t VLOCLK_usage_count;
uint8_t LFXT1CLK_usage_count;
uint8_t DCOCLK_usage_count;

template<const uint16_t FREQUENCY = 12000>
struct VLOCLK_T {
	static constexpr CLOCK_SOURCE_TYPE type = CLOCK_SOURCE_TYPE_VLO;
	static constexpr uint16_t frequency = FREQUENCY;
	static void claim(void) { VLOCLK_usage_count++; }
	static void release(void) { VLOCLK_usage_count--; }
};


template<const uint16_t FREQUENCY = 32768>
struct LFXT1CLK_T {
	static constexpr CLOCK_SOURCE_TYPE type = CLOCK_SOURCE_TYPE_LFXT1;
	static constexpr uint16_t frequency = FREQUENCY;
	static void claim(void) { LFXT1CLK_usage_count++; }
	static void release(void) { LFXT1CLK_usage_count--; }
};


template<const uint32_t FREQUENCY = 1000000>
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
	static void release(void) { DCOCLK_usage_count--; }
};

template<typename SOURCE,
	const int DIVIDER = 1>
struct ACLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_ACLK;
	static constexpr long frequency = SOURCE::frequency / DIVIDER;
	static constexpr uint8_t idle_mode(void) { return LPM3_bits; }

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
	static constexpr uint8_t idle_mode(void) { return LPM0_bits; }

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
};

template<typename SOURCE,
	const int DIVIDER = 1>
struct MCLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_MCLK;
	static constexpr long frequency = SOURCE::frequency / DIVIDER;
	static constexpr uint8_t idle_mode(void) { return 0; }

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

	static void claim(void) { SOURCE::claim(); }
	static void release(void) { SOURCE::release(); }
};

#endif
