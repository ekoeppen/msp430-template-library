#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <spi.h>
#include <uart.h>
#include <io.h>

typedef ACLK_T<ACLK_SOURCE_LFXT1CLK> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<1, 6, LOW> LED_GREEN;
typedef GPIO_PORT_T<1, LED_RED, LED_GREEN> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;
typedef UART_T<USCI_A, 0, SMCLK> UART;

typedef ALARM_T<WDT> ALARM;

int main(void)
{
	ACLK::init();
	SMCLK::init();
	WDT::init();
	PORT1::init();
	WDT::enable_irq();
	while (1) {
		LED_RED::toggle();
		ALARM::set_alarm(1000);
		enter_idle<WDT>();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	LED_GREEN::toggle();
	if (ALARM::alarm_triggered()) exit_idle();
}
