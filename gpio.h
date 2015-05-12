#ifndef __GPIO_H
#define __GPIO_H

#include <stdint.h>
#include <msp430.h>
#include <tasks.h>

enum DIRECTION {
	INPUT = 0,
	OUTPUT = 1
};

enum TRIGGER_EDGE {
	TRIGGER_RISING = 0,
	TRIGGER_FALLING = 1
};

enum LEVEL {
	LOW = false,
	PULL_DOWN = false,
	HIGH = true,
	PULL_UP = true
};

enum INTERRUPT_ENABLE {
	INTERRUPT_DISABLED = false,
	INTERRUPT_ENABLED = true
};

enum RESISTOR_ENABLE {
	RESISTOR_DISABLED = false,
	RESISTOR_ENABLED = true
};

enum PIN_FUNCTION {
	TIMER_OUTPUT,
	TIMER_COMPARE,
	ACLK_OUTPUT,
	CAPACITIVE_SENSE,
	ADCCLK_OUTPUT,
	COMPARATOR_OUTPUT,
	SMCLK_OUTUT,
	XIN,
	XOUT,
	USCI
};

uint8_t P1IFGS;
uint8_t P2IFGS;

static constexpr uint16_t ports[][10] = {
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{P1IN_, P1OUT_, P1DIR_, P1IFG_, P1IES_, P1IE_, P1SEL_, P1SEL2_, P1REN_, (uint16_t) &P1IFGS},
#ifdef P2IN_
	{P2IN_, P2OUT_, P2DIR_, P2IFG_, P2IES_, P2IE_, P2SEL_, P2SEL2_, P2REN_, (uint16_t) &P2IFGS},
#ifdef P3IN_
	{P3IN_, P3OUT_, P3DIR_, 0, 0, 0, P3SEL_, P3SEL2_, P3REN_, 0},
#ifdef P4IN_
	{P4IN_, P4OUT_, P4DIR_, 0, 0, 0, P4SEL_, P4SEL2_, P4REN_, 0},
#ifdef P5IN_
	{P5IN_, P5OUT_, P5DIR_, 0, 0, 0, P5SEL_, P5SEL2_, P5REN_, 0},
#ifdef P6IN_
	{P6IN_, P6OUT_, P6DIR_, 0, 0, 0, P6SEL_, P6SEL2_, P6REN_, 0},
#ifdef P7IN_
	{P7IN_, P7OUT_, P7DIR_, 0, 0, 0, P7SEL_, P7SEL2_, P7REN_, 0},
#ifdef P8IN_
	{P8IN_, P8OUT_, P8DIR_, 0, 0, 0, P8SEL_, P8SEL2_, P8REN_, 0}
#endif
#endif
#endif
#endif
#endif
#endif
#endif
};

template<const char PORT, const char PIN,
	const DIRECTION PIN_DIRECTION = OUTPUT,
	const LEVEL INITIAL_LEVEL = LOW,
	const INTERRUPT_ENABLE INTERRUPT = INTERRUPT_DISABLED,
	const TRIGGER_EDGE EDGE = TRIGGER_RISING,
	const char FUNCTION_SELECT = 0,
	const RESISTOR_ENABLE RESISTOR = RESISTOR_DISABLED>
struct GPIO_PIN_T {
	static constexpr volatile uint8_t *PxIN = (uint8_t *) ports[PORT][0];
	static constexpr volatile uint8_t *PxOUT = (uint8_t *) ports[PORT][1];
	static constexpr volatile uint8_t *PxDIR = (uint8_t *) ports[PORT][2];
	static constexpr volatile uint8_t *PxIFG = (uint8_t *) ports[PORT][3];
	static constexpr volatile uint8_t *PxIES = (uint8_t *) ports[PORT][4];
	static constexpr volatile uint8_t *PxIE = (uint8_t *) ports[PORT][5];
	static constexpr volatile uint8_t *PxSEL = (uint8_t *) ports[PORT][6];
	static constexpr volatile uint8_t *PxSEL2 = (uint8_t *) ports[PORT][7];
	static constexpr volatile uint8_t *PxREN = (uint8_t *) ports[PORT][8];
	static constexpr volatile uint8_t *PxIFGS = (uint8_t *) ports[PORT][9];

	static constexpr uint8_t pin = PIN;
	static constexpr uint8_t bit_value = 1 << PIN;

	static constexpr uint8_t direction = (PIN_DIRECTION == OUTPUT ? bit_value : 0);
	static constexpr uint8_t interrupt_enable = (INTERRUPT ? bit_value : 0);
	static constexpr uint8_t interrupt_edge = (EDGE == TRIGGER_FALLING ? bit_value : 0);
	static constexpr uint8_t function_select = (FUNCTION_SELECT & 0b01 ? bit_value : 0);
	static constexpr uint8_t function_select2 = (FUNCTION_SELECT & 0b10 ? bit_value : 0);
	static constexpr uint8_t resistor_enable = (RESISTOR ? bit_value : 0);
	static constexpr uint8_t initial_level = (INITIAL_LEVEL ? bit_value : 0);
	static constexpr uint16_t adc_input = (PORT == 1 ? PIN << 12 : 0);
	static constexpr bool is_unused(void) { return false; }

	static void init(void) {
		if (INITIAL_LEVEL) *PxOUT |= bit_value;
		if (PIN_DIRECTION == OUTPUT) *PxDIR |= bit_value;
		if (INTERRUPT) {
			static_assert(PxIE != 0, "Port not interrupt capable");
			*PxIE |= bit_value;
		}
		if (EDGE == TRIGGER_FALLING) {
			static_assert(PxIES != 0, "Port not interrupt capable");
			*PxIES |= bit_value;
		}
		if (FUNCTION_SELECT & 0b01) {
			static_assert(PxSEL != 0, "Port does not have alternate functions");
			*PxSEL |= bit_value;
		}
		if (FUNCTION_SELECT & 0b10) {
			static_assert(PxSEL2 != 0, "Port does not have alternate functions");
			*PxSEL2 |= bit_value;
		}
		if (RESISTOR) *PxREN |= bit_value;
	};

	static void set_high(void) {
		*PxOUT |= bit_value;
	};

	static void set_low(void) {
		*PxOUT &= ~bit_value;
	}

	static void set(bool value) {
		if (value) set_high();
		else set_low();
	}

	static bool get(void) {
		return *PxIN & bit_value;
	}

	static bool is_high(void) {
		return *PxIN & bit_value;
	}

	static bool is_low(void) {
		return !is_high();
	}

	static void toggle(void) {
		*PxOUT ^= bit_value;
	}

	static void clear_irq(void) {
		*PxIFG &= ~bit_value;
		*PxIFGS &= ~bit_value;
	}

	static bool irq_raised(void) {
		return *PxIFGS & bit_value;
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static bool wait_for_irq(void) {
		while (!TIMEOUT::triggered() && !(*PxIFGS & bit_value)) {
			enter_idle();
		}
		return irq_raised();
	}

	static void set_output(void) {
		*PxDIR |= bit_value;
	}

	static void set_input(void) {
		*PxDIR &= ~bit_value;
	}

	static void enable_resistor(void) {
		*PxREN |= bit_value;
	}

	static void disable_resistor(void) {
		*PxREN &= ~bit_value;
	}

	static void pull_up(void) {
		set_high();
	}

	static void pull_down(void) {
		set_low();
	}
};

template<const char PORT, const char PIN,
	const RESISTOR_ENABLE RESISTOR = RESISTOR_DISABLED,
	const LEVEL PULL = PULL_DOWN,
	const INTERRUPT_ENABLE INTERRUPT = INTERRUPT_DISABLED,
	const TRIGGER_EDGE EDGE = TRIGGER_RISING>
struct GPIO_INPUT_T: public GPIO_PIN_T<PORT, PIN, INPUT, PULL, INTERRUPT, EDGE, 0, RESISTOR> {
};

template<const char PORT, const char PIN,
	const LEVEL INITIAL_LEVEL = LOW>
struct GPIO_OUTPUT_T: public GPIO_PIN_T<PORT, PIN, OUTPUT, INITIAL_LEVEL, INTERRUPT_DISABLED, TRIGGER_RISING, 0, RESISTOR_DISABLED> {
};

template<const char PORT, const char PIN,
	const char FUNCTION_SELECT = 0,
	const DIRECTION PIN_DIRECTION = OUTPUT>
struct GPIO_MODULE_T: public GPIO_PIN_T<PORT, PIN, PIN_DIRECTION, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, FUNCTION_SELECT, RESISTOR_DISABLED> {
};

template<const char PORT, const char PIN>
struct GPIO_ANALOG_T {
	static constexpr uint8_t direction = 0;
	static constexpr uint8_t interrupt_enable = 0;
	static constexpr uint8_t interrupt_edge = 0;
	static constexpr uint8_t function_select = 0;
	static constexpr uint8_t function_select2 = 0;
	static constexpr uint8_t resistor_enable = 0;
	static constexpr uint8_t initial_level = 0;
	static constexpr uint8_t pin = PIN;
	static constexpr uint8_t bit_value = 1 << PIN;
	static constexpr uint16_t adc_input = (PORT == 1 ? PIN << 12 : 0);
};

struct PIN_UNUSED {
	static constexpr uint8_t direction = 0;
	static constexpr uint8_t interrupt_enable = 0;
	static constexpr uint8_t interrupt_edge = 0;
	static constexpr uint8_t function_select = 0;
	static constexpr uint8_t function_select2 = 0;
	static constexpr uint8_t resistor_enable = 0;
	static constexpr uint8_t initial_level = 0;
	static constexpr uint16_t adc_input = 0;
	static constexpr uint8_t pin = 0;
	static constexpr uint8_t bit_value = 0;
	static constexpr bool is_unused(void) { return true; }
	static constexpr bool is_high(void) { return false; }
	static constexpr bool is_low(void) { return true; }
	static void toggle(void) { }
	static void set_high(void) { }
	static void set_low(void) { }
	static void clear_irq(void) { }
	static bool wait_for_irq(void) { return true; }
	static constexpr bool irq_raised(void) { return false; }
};

template<const int PORT,
	typename PIN0 = PIN_UNUSED,
	typename PIN1 = PIN_UNUSED,
	typename PIN2 = PIN_UNUSED,
	typename PIN3 = PIN_UNUSED,
	typename PIN4 = PIN_UNUSED,
	typename PIN5 = PIN_UNUSED,
	typename PIN6 = PIN_UNUSED,
	typename PIN7 = PIN_UNUSED>
struct GPIO_PORT_T {
	static constexpr volatile uint8_t *PxIN = (uint8_t *) ports[PORT][0];
	static constexpr volatile uint8_t *PxOUT = (uint8_t *) ports[PORT][1];
	static constexpr volatile uint8_t *PxDIR = (uint8_t *) ports[PORT][2];
	static constexpr volatile uint8_t *PxIFG = (uint8_t *) ports[PORT][3];
	static constexpr volatile uint8_t *PxIES = (uint8_t *) ports[PORT][4];
	static constexpr volatile uint8_t *PxIE = (uint8_t *) ports[PORT][5];
	static constexpr volatile uint8_t *PxSEL = (uint8_t *) ports[PORT][6];
	static constexpr volatile uint8_t *PxSEL2 = (uint8_t *) ports[PORT][7];
	static constexpr volatile uint8_t *PxREN = (uint8_t *) ports[PORT][8];
	static constexpr volatile uint8_t *PxIFGS = (uint8_t *) ports[PORT][9];

	static void init(void) {
		uint8_t reg;

		reg =
			PIN0::direction | PIN1::direction | PIN2::direction | PIN3::direction |
			PIN4::direction | PIN5::direction | PIN6::direction | PIN7::direction;
		if (reg) *PxDIR = reg;

		reg =
			PIN0::interrupt_enable | PIN1::interrupt_enable | PIN2::interrupt_enable | PIN3::interrupt_enable |
			PIN4::interrupt_enable | PIN5::interrupt_enable | PIN6::interrupt_enable | PIN7::interrupt_enable;
		if (reg) *PxIE = reg;

		reg =
			PIN0::interrupt_edge | PIN1::interrupt_edge | PIN2::interrupt_edge | PIN3::interrupt_edge |
			PIN4::interrupt_edge | PIN5::interrupt_edge | PIN6::interrupt_edge | PIN7::interrupt_edge;
		if (reg) *PxIES = reg;

		reg =
			PIN0::function_select | PIN1::function_select | PIN2::function_select | PIN3::function_select |
			PIN4::function_select | PIN5::function_select | PIN6::function_select | PIN7::function_select;
		if (reg) *PxSEL = reg;

		reg =
			PIN0::function_select2 | PIN1::function_select2 | PIN2::function_select2 | PIN3::function_select2 |
			PIN4::function_select2 | PIN5::function_select2 | PIN6::function_select2 | PIN7::function_select2;
		if (reg) *PxSEL2 = reg;

		reg =
			PIN0::resistor_enable | PIN1::resistor_enable | PIN2::resistor_enable | PIN3::resistor_enable |
			PIN4::resistor_enable | PIN5::resistor_enable | PIN6::resistor_enable | PIN7::resistor_enable;
		if (reg) *PxREN = reg;

		*PxOUT =
			PIN0::initial_level | PIN1::initial_level | PIN2::initial_level | PIN3::initial_level |
			PIN4::initial_level | PIN5::initial_level | PIN6::initial_level | PIN7::initial_level;
	}

	static void clear_irq(void) {
		*PxIFG = 0;
	};

	static bool handle_irq(void) {
		bool resume = false;

		if (*PxIFG) {
			*PxIFGS = *PxIFG;
			clear_irq();
			resume = true;
		}
		return resume;
	}
};

#endif
