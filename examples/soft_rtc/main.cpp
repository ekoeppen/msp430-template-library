#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <soft_uart.h>
#endif
#include <drivers/soft_rtc.h>

typedef LFXT1CLK_T<XCAP_0> LFXT1;
typedef ACLK_T<LFXT1> ACLK;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_PORT_T<1, LED_RED> PORT1;
typedef SOFT_RTC_T<WDT> RTC;

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	RTC::update();
}

int main(void)
{
	LFXT1::init();
	ACLK::init();
	RTC::init();
	PORT1::init();
	WDT::init();
	WDT::enable_irq();
	while (1) {
		enter_idle();
	}
}
