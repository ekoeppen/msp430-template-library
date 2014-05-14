#ifndef __ADC_H
#define __ADC_H

#include <clocks.h>

struct ADC10OSC {
	static constexpr unsigned char idle_mode = LPM4_bits;
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
	};

	static void disable(void) {
		ADC10CTL0 &= ~ENC;
		ADC10CTL0 &= ~(ADC10IFG | ADC10ON | REFON);
	};
};

#endif
