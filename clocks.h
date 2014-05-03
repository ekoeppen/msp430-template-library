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

template<const ACLK_SOURCE SOURCE = ACLK_SOURCE_LFXT1CLK,
	const int DIVIDER = 0>
struct ACLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_ACLK;
	static constexpr long speed = (SOURCE == ACLK_SOURCE_VLOCLK ? 12000 : 32768);

	static void init(void) {
		if (SOURCE == ACLK_SOURCE_VLOCLK) BCSCTL3 |= LFXT1S_2;
	};
};

template<const CLK_SOURCE SOURCE = CLK_SOURCE_DCOCLK,
	const long SPEED = 1000000>
struct SMCLK_T {
	static constexpr CLOCK_TYPE type = CLOCK_TYPE_SMCLK;
	static constexpr long speed = SPEED;

	static void init(void) {
		switch (speed) {
			case 1000000:
			default:
				BCSCTL1 = CALBC1_1MHZ;
				DCOCTL = CALDCO_1MHZ;
				break;
		}
	};
};

#endif
