#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <spi.h>
#include <uart.h>
#include <io.h>

typedef ACLK_T<> ACLK;
typedef SMCLK_T<> SMCLK;
typedef WDT_T<ACLK, WDT_TIMER> WDT;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<1, 6, HIGH> LED_GREEN;
typedef GPIO_INPUT_T<1, 3, RESISTOR_ENABLED, PULL_UP, INTERRUPT_ENABLED, TRIGGER_FALLING> BUTTON;
struct PORT1: public GPIO_PORT_T<1, LED_RED, LED_GREEN, BUTTON> {
	static constexpr unsigned char idle_mode = LPM4_bits;
};

void port1_irq (void) __attribute__((interrupt(PORT1_VECTOR)));
void port1_irq(void)
{
	LED_RED::toggle();
	PORT1::clear_irq();
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
	test_irq();
	LED_GREEN::set_low();
	while (1) {
		enter_idle(LPM4_bits);
	}
}
