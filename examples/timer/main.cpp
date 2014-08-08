#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <timer.h>

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

typedef GPIO_OUTPUT_T<1, 0> LED_RED;
typedef GPIO_OUTPUT_T<1, 6> LED_GREEN;
typedef GPIO_PORT_T<1, LED_RED, LED_GREEN> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;
typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER_SMCLK;
typedef TIMER_T<TIMER_A, 0, ACLK, TIMER_MODE_UP, 1, CCIE, ACLK::frequency> TIMER_ACLK;
typedef TIMEOUT_T<TIMER_ACLK> TIMEOUT;

int main(void)
{
	DCO::init();
	ACLK::init();
	SMCLK::init();
	WDT::init();
	PORT1::init();
	//WDT::enable_irq();
	TIMER_ACLK::init();
	while (1) {
		TIMEOUT::set_and_wait(1000);
		LED_RED::toggle();
#if 0
		LED_GREEN::set_high();
		for (int i = 0; i < 10000; i++) {
			unsigned int end = TIMER::counter() + 50000;
			while (TIMER::counter() < end) ;
		}
		LED_GREEN::set_low();
		TIMEOUT::set_and_wait(1000);
#endif
	}
}

void timer_irq(void) __attribute__((interrupt(TIMER0_A0_VECTOR)));
void timer_irq(void)
{
	LED_GREEN::toggle();
	if (TIMEOUT::count_down()) {
		exit_idle();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) {
		exit_idle();
	}
}
