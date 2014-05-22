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
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef GPIO_PORT_T<1, LED_RED, LED_GREEN, RX, TX> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;
typedef UART_T<USCI_A, 0, SMCLK> UART;

typedef TIMEOUT_T<WDT> TIMEOUT;

int main(void)
{
	ACLK::init();
	SMCLK::init();
	WDT::init();
	PORT1::init();
	WDT::enable_irq();
	UART::init();
	printf<UART>("WDT example start\n");
	while (1) {
		LED_RED::toggle();
		TIMEOUT::set_timeout(60000 * 30);
		enter_idle<WDT>();
		printf<UART>("Timeout\n");
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	LED_GREEN::toggle();
	if (TIMEOUT::timeout_triggered()) exit_idle();
}
