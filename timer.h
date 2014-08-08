#ifndef __TIMER_H
#define __TIMER_H

enum TIMER_MODULE {
	TIMER_A = 0,
#ifdef TB0CTL_
	TIMER_B = 1
#endif
};

enum TIMER_MODE {
	TIMER_MODE_STOP = MC_0,
	TIMER_MODE_UP = MC_1,
	TIMER_MODE_CONTINUOUS = MC_2,
	TIMER_MODE_UP_DOWN = MC_3
};

static constexpr uint16_t timer_regs[][3][17] = {
	{
		{TA0CTL_, TA0R_, TA0CCTL0_, TA0CCR0_, TA0CCTL1_, TA0CCR1_, TA0CCTL2_, TA0CCR2_, 0, 0, 0, 0, 0, 0, 0, 0, TA0IV_},
#ifdef TA1CTL_
		{TA1CTL_, TA1R_, TA1CCTL0_, TA1CCR0_, TA1CCTL1_, TA1CCR1_, TA1CCTL2_, TA1CCR2_, 0, 0, 0, 0, 0, 0, 0, 0, TA1IV_},
#ifdef TA2CTL_
		{TA2CTL_, TA2R_, TA2CCTL0_, TA2CCR0_, TA2CCTL1_, TA2CCR1_, TA2CCTL2_, TA2CCR2_, 0, 0, 0, 0, 0, 0, 0, 0, TA2IV_},
#else
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#endif
#else
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#endif
	},
#ifdef TB0CTL_
	{
		{TB0CTL_, TB0R_, TB0CCTL0_, TB0CCR0_, TB0CCTL1_, TB0CCR1_, TB0CCTL2_, TB0CCR2_, 0, 0, 0, 0, 0, 0, 0, 0, TB0IV_},
	}
#endif
};

template<const TIMER_MODULE MODULE,
	const unsigned char INSTANCE,
	typename CLOCK,
	const TIMER_MODE MODE,
	const uint8_t DIVIDER = 1,
	const uint16_t CCTL0_INIT = 0,
	const uint16_t CCR0_INIT = 0,
	const uint16_t CCTL1_INIT = 0,
	const uint16_t CCR1_INIT = 0,
	const uint16_t CCTL2_INIT = 0,
	const uint16_t CCR2_INIT = 0,
	const uint16_t CCTL3_INIT = 0,
	const uint16_t CCR3_INIT = 0,
	const uint16_t CCTL4_INIT = 0,
	const uint16_t CCR4_INIT = 0,
	const uint16_t CCTL5_INIT = 0,
	const uint16_t CCR5_INIT = 0,
	const uint16_t CCTL6_INIT = 0,
	const uint16_t CCR6_INIT = 0>
struct TIMER_T {
	static constexpr volatile uint16_t *CTL = (uint16_t *) timer_regs[MODULE][INSTANCE][0];
	static constexpr volatile uint16_t *CTR = (uint16_t *) timer_regs[MODULE][INSTANCE][1];
	static constexpr volatile uint16_t *CCTL0 = (uint16_t *) timer_regs[MODULE][INSTANCE][2];
	static constexpr volatile uint16_t *CCR0 = (uint16_t *) timer_regs[MODULE][INSTANCE][3];
	static constexpr volatile uint16_t *CCTL1 = (uint16_t *) timer_regs[MODULE][INSTANCE][4];
	static constexpr volatile uint16_t *CCR1 = (uint16_t *) timer_regs[MODULE][INSTANCE][5];
	static constexpr volatile uint16_t *CCTL2 = (uint16_t *) timer_regs[MODULE][INSTANCE][6];
	static constexpr volatile uint16_t *CCR2 = (uint16_t *) timer_regs[MODULE][INSTANCE][7];
	static constexpr volatile uint16_t *CCTL3 = (uint16_t *) timer_regs[MODULE][INSTANCE][8];
	static constexpr volatile uint16_t *CCR3 = (uint16_t *) timer_regs[MODULE][INSTANCE][9];
	static constexpr volatile uint16_t *CCTL4 = (uint16_t *) timer_regs[MODULE][INSTANCE][10];
	static constexpr volatile uint16_t *CCR4 = (uint16_t *) timer_regs[MODULE][INSTANCE][11];
	static constexpr volatile uint16_t *CCTL5 = (uint16_t *) timer_regs[MODULE][INSTANCE][12];
	static constexpr volatile uint16_t *CCR5 = (uint16_t *) timer_regs[MODULE][INSTANCE][13];
	static constexpr volatile uint16_t *CCTL6 = (uint16_t *) timer_regs[MODULE][INSTANCE][14];
	static constexpr volatile uint16_t *CCR6 = (uint16_t *) timer_regs[MODULE][INSTANCE][15];
	static constexpr volatile uint16_t *IV = (uint16_t *) timer_regs[MODULE][INSTANCE][16];

	static constexpr uint32_t frequency = CLOCK::frequency /
		DIVIDER /
		(MODE == TIMER_MODE_UP || MODE == TIMER_MODE_UP_DOWN ? CCR0_INIT : 1);

	static void init(void) {
		static_assert(CCTL0_INIT == 0 || CCTL0 != 0, "Timer CCTL0 register does not exist");
		static_assert(CCTL1_INIT == 0 || CCTL1 != 0, "Timer CCTL1 register does not exist");
		static_assert(CCTL2_INIT == 0 || CCTL2 != 0, "Timer CCTL2 register does not exist");
		static_assert(CCTL3_INIT == 0 || CCTL3 != 0, "Timer CCTL3 register does not exist");
		static_assert(CCTL4_INIT == 0 || CCTL4 != 0, "Timer CCTL4 register does not exist");
		static_assert(CCTL5_INIT == 0 || CCTL5 != 0, "Timer CCTL5 register does not exist");
		static_assert(CCTL6_INIT == 0 || CCTL6 != 0, "Timer CCTL6 register does not exist");
		*CTL = MODE +
			(CLOCK::type == CLOCK_TYPE_ACLK ? TASSEL_1 : TASSEL_2) +
			(DIVIDER == 2 ? ID_1 : (DIVIDER == 4 ? ID_2 : (DIVIDER == 8 ? ID_3 : ID_0)));
		if (CCTL0_INIT) *CCTL0 = CCTL0_INIT;
		if (CCR0_INIT) *CCR0 = CCR0_INIT;
		if (CCTL1_INIT) *CCTL1 = CCTL1_INIT;
		if (CCR1_INIT) *CCR1 = CCR1_INIT;
		if (CCTL2_INIT) *CCTL2 = CCTL2_INIT;
		if (CCR2_INIT) *CCR2 = CCR2_INIT;
		if (CCTL3_INIT) *CCTL3 = CCTL3_INIT;
		if (CCR3_INIT) *CCR3 = CCR3_INIT;
		if (CCTL4_INIT) *CCTL4 = CCTL4_INIT;
		if (CCR4_INIT) *CCR4 = CCR4_INIT;
		if (CCTL5_INIT) *CCTL5 = CCTL5_INIT;
		if (CCR5_INIT) *CCR5 = CCR5_INIT;
		if (CCTL6_INIT) *CCTL6 = CCTL6_INIT;
		if (CCR6_INIT) *CCR6 = CCR6_INIT;
	};

	static void enable(void) {
		*CTL |= MODE;
	};

	static void disable(void) {
		*CTL &= ~MC_3;
	};

	static void claim(void) {
		CLOCK::claim();
	}

	static void release(void) {
		CLOCK::release();
	};

	static uint16_t counter(void) {
		return *CTR;
	};
};

#endif
