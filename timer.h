#ifndef __TIMER_H
#define __TIMER_H

enum TIMER_MODULE {
	TIMER_A = 0,
#ifdef TB0CTL_
	TIMER_B = 1
#endif
};

static constexpr unsigned int timer_regs[][3][10] = {
	{
		{TA0CTL_, TA0R_, TA0CCTL0_, TA0CCTL1_, TA0CCTL2_, 0, 0, 0, 0, TA0IV_},
#ifdef TA1CTL_
		{TA1CTL_, TA1R_, TA1CCTL0_, TA1CCTL1_, TA1CCTL2_, 0, 0, 0, 0, TA1IV_},
#ifdef TA2CTL_
		{TA2CTL_, TA2R_, TA2CCTL0_, TA2CCTL1_, TA2CCTL2_, 0, 0, 0, 0, TA2IV_}
#else
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#endif
#else
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#endif
	},
#ifdef TB0CTL_
	{
		{TB0CTL_, TB0R_, TB0CCTL0_, TB0CCTL1_, TB0CCTL2_, 0, 0, 0, 0, TB0IV_}
	}
#endif
};

template<const TIMER_MODULE MODULE,
	const unsigned char INSTANCE,
	typename CLOCK_SOURCE,
	const unsigned int CTL_INIT,
	const unsigned int IV_INIT = 0,
	const unsigned int CCTL0_INIT = 0,
	const unsigned int CCTL1_INIT = 0,
	const unsigned int CCTL2_INIT = 0,
	const unsigned int CCTL3_INIT = 0,
	const unsigned int CCTL4_INIT = 0,
	const unsigned int CCTL5_INIT = 0,
	const unsigned int CCTL6_INIT = 0>
struct TIMER_T {
	static constexpr volatile unsigned int *CTL = (unsigned int *) timer_regs[MODULE][INSTANCE][0];
	static constexpr volatile unsigned int *CTR = (unsigned int *) timer_regs[MODULE][INSTANCE][1];
	static constexpr volatile unsigned int *CCTL0 = (unsigned int *) timer_regs[MODULE][INSTANCE][2];
	static constexpr volatile unsigned int *CCTL1 = (unsigned int *) timer_regs[MODULE][INSTANCE][3];
	static constexpr volatile unsigned int *CCTL2 = (unsigned int *) timer_regs[MODULE][INSTANCE][4];
	static constexpr volatile unsigned int *CCTL3 = (unsigned int *) timer_regs[MODULE][INSTANCE][5];
	static constexpr volatile unsigned int *CCTL4 = (unsigned int *) timer_regs[MODULE][INSTANCE][6];
	static constexpr volatile unsigned int *CCTL5 = (unsigned int *) timer_regs[MODULE][INSTANCE][7];
	static constexpr volatile unsigned int *CCTL6 = (unsigned int *) timer_regs[MODULE][INSTANCE][8];
	static constexpr volatile unsigned int *IV = (unsigned int *) timer_regs[MODULE][INSTANCE][9];

	static constexpr uint8_t idle_mode(void) { return CLOCK_SOURCE::idle_mode };
	static constexpr unsigned long frequency = CLOCK_SOURCE::frequency;

	static void init(void) {
		static_assert(CCTL0_INIT == 0 || CCTL0 != 0, "Timer CCTL0 register does not exist");
		static_assert(CCTL1_INIT == 0 || CCTL1 != 0, "Timer CCTL1 register does not exist");
		static_assert(CCTL2_INIT == 0 || CCTL2 != 0, "Timer CCTL2 register does not exist");
		static_assert(CCTL3_INIT == 0 || CCTL3 != 0, "Timer CCTL3 register does not exist");
		static_assert(CCTL4_INIT == 0 || CCTL4 != 0, "Timer CCTL4 register does not exist");
		static_assert(CCTL5_INIT == 0 || CCTL5 != 0, "Timer CCTL5 register does not exist");
		static_assert(CCTL6_INIT == 0 || CCTL6 != 0, "Timer CCTL6 register does not exist");
		*CTL = CTL_INIT;
		*IV = IV_INIT;
		if (CCTL0_INIT) *CCTL0 = CCTL0_INIT;
		if (CCTL1_INIT) *CCTL1 = CCTL1_INIT;
		if (CCTL2_INIT) *CCTL2 = CCTL2_INIT;
		if (CCTL3_INIT) *CCTL3 = CCTL3_INIT;
		if (CCTL4_INIT) *CCTL4 = CCTL4_INIT;
		if (CCTL5_INIT) *CCTL5 = CCTL5_INIT;
		if (CCTL6_INIT) *CCTL6 = CCTL6_INIT;
	};

	static unsigned int counter(void) {
		return *CTR;
	};
};

#endif
