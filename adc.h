#ifndef __ADC_H
#define __ADC_H

#include <clocks.h>

struct ADC10OSC {
	static constexpr unsigned char idle_mode = LPM0_bits;
};

template<typename CLOCK,
	const unsigned int CTL0,
	const unsigned int CTL1,
	const unsigned int ANALOG_INPUT_ENABLE = 0>
struct ADC10_T {
	static constexpr unsigned int idle_mode = CLOCK::idle_mode;

	static void init(void) {
		ADC10CTL0 = CTL0;
		ADC10CTL1 = CTL1;
		ADC10AE0 = ANALOG_INPUT_ENABLE & 0xff;
#ifdef ADC10AE1_
		ADC10AE1 = ANALOG_INPUT_ENABLE >> 8;
#endif
	};

	static unsigned int sample_once(void) {
		ADC10CTL0 |= ENC + ADC10SC;
		while (ADC10CTL1 & ADC10BUSY) {
			enter_idle<CLOCK>();
		}
		ADC10CTL &= ~ENC;
		return ADC10MEM;
	};

	static void disable(void) {
		ADC10CTL0 &= ~ENC;
		ADC10CTL0 &= ~(ADC10IFG | ADC10ON | REFON);
	};
};

#endif
