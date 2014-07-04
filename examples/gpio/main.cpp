#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <soft_uart.h>
#endif

typedef ACLK_T<> ACLK;
typedef SMCLK_T<> SMCLK;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<1, 1, HIGH> P1_1;
typedef GPIO_OUTPUT_T<1, 2, LOW> P1_2;
typedef GPIO_INPUT_T<1, 3, RESISTOR_ENABLED, PULL_UP, INTERRUPT_ENABLED, TRIGGER_FALLING> BUTTON;
typedef GPIO_OUTPUT_T<1, 4, HIGH> P1_4;
typedef GPIO_OUTPUT_T<1, 5, LOW> P1_5;
typedef GPIO_OUTPUT_T<1, 6, HIGH> LED_GREEN;
typedef GPIO_OUTPUT_T<2, 0, HIGH> P2_0;
typedef GPIO_OUTPUT_T<2, 1, LOW> P2_1;
typedef GPIO_OUTPUT_T<2, 2, HIGH> P2_2;
struct PORT1: public GPIO_PORT_T<1, LED_RED, LED_GREEN, P1_1, P1_2, P1_4, P1_5, BUTTON> {
	static constexpr unsigned char idle_mode = LPM4_bits;
};
typedef GPIO_PORT_T<2, P2_0, P2_1, P2_2> PORT2;

void port1_irq (void) __attribute__((interrupt(PORT1_VECTOR)));
void port1_irq(void)
{
	LED_RED::toggle();
	PORT1::clear_irq();
	exit_idle();
}

void test_polling(void)
{
	while (1) {
		if (BUTTON::get()) LED_RED::set_high();
		else LED_RED::set_low();
	}
}

void test_irq(void)
{
	enter_idle<PORT1>();
}

int main(void)
{
	ACLK::init();
	SMCLK::init();
	WDT::init();
	PORT1::init();
	PORT2::init();
	test_irq();
	LED_GREEN::set_low();
	while (1) {
		enter_idle(LPM4_bits);
	}
}
