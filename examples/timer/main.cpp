#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <timer.h>

typedef ACLK_T<ACLK_SOURCE_VLOCLK> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_OUTPUT_T<1, 0> LED_RED;
typedef GPIO_OUTPUT_T<1, 6> LED_GREEN;
typedef GPIO_PORT_T<1, LED_RED, LED_GREEN> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;
typedef TIMER_T<TIMER_A, 0, SMCLK, TASSEL_2 + MC_2> TIMER;
typedef TIMEOUT_T<WDT> TIMEOUT;

int main(void)
{
	ACLK::init();
	SMCLK::init();
	WDT::init();
	TIMER::init();
	PORT1::init();
	WDT::enable_irq();
	while (1) {
		LED_RED::toggle();
		LED_GREEN::set_high();
		for (int i = 0; i < 10000; i++) {
			unsigned int end = TIMER::counter() + 50000;
			while (TIMER::counter() < end) ;
		}
		LED_GREEN::set_low();
		TIMEOUT::set_timeout(1000);
		enter_idle<TIMEOUT>();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::timeout_triggered()) {
		exit_idle();
	}
}
