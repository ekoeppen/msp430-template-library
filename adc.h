#ifndef __ADC_H
#define __ADC_H

#include <clocks.h>
#include <gpio.h>

struct ADC10OSC {
	static constexpr int type = CLOCK_TYPE_ADCOSC;
	static void claim(void) { }
	static void release(void) { }
};

template<typename CLOCK,
	const unsigned int CTL0,
	const unsigned int CTL1,
	typename INPUT0,
	typename INPUT1 = PIN_UNUSED,
	typename INPUT2 = PIN_UNUSED,
	typename INPUT3 = PIN_UNUSED,
	typename INPUT4 = PIN_UNUSED,
	typename INPUT5 = PIN_UNUSED,
	typename INPUT6 = PIN_UNUSED,
	typename INPUT7 = PIN_UNUSED>
struct ADC10_T {
	static void init(void) {
		uint16_t clock_type;
		uint16_t n;

		switch (CLOCK::type) {
			case CLOCK_TYPE_ADCOSC: clock_type = ADC10SSEL_0; break;
			case CLOCK_TYPE_ACLK: clock_type = ADC10SSEL_1; break;
			case CLOCK_TYPE_MCLK: clock_type = ADC10SSEL_2; break;
			case CLOCK_TYPE_SMCLK: clock_type = ADC10SSEL_3; break;
		}
		ADC10CTL0 = CTL0 | ADC10IE;
		ADC10CTL1 = CTL1 | clock_type |
			INPUT0::adc_input | INPUT1::adc_input | INPUT2::adc_input | INPUT3::adc_input |
			INPUT4::adc_input | INPUT5::adc_input | INPUT6::adc_input | INPUT7::adc_input;
		ADC10AE0 =
			INPUT0::bit_value | INPUT1::bit_value | INPUT2::bit_value | INPUT3::bit_value |
			INPUT4::bit_value | INPUT5::bit_value | INPUT6::bit_value | INPUT7::bit_value;
		__delay_cycles (128);
	};

	static void enable(void) {
		ADC10CTL0 |= ENC + ADC10SC;
	};

	static unsigned int sample_once(void) {
		unsigned int r;

		CLOCK::claim();
		enable();
		while (ADC10CTL1 & ADC10BUSY) {
			enter_idle();
		}
		r = ADC10MEM;
		disable();
		CLOCK::release();
		return r;
	};

	static void disable(void) {
		ADC10CTL0 &= ~ENC;
	};
};

#endif
