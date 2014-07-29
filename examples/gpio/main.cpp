#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <soft_uart.h>
#endif

typedef VLOCLK_T<> VLO;
typedef ACLK_T<VLO> ACLK;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;

typedef GPIO_MODULE_T<1, 0, 1> ACLK_OUT;
typedef GPIO_INPUT_T<1, 1, RESISTOR_ENABLED, PULL_UP> P1_1;
typedef GPIO_INPUT_T<1, 2, RESISTOR_ENABLED, PULL_UP> P1_2;
typedef GPIO_INPUT_T<1, 3, RESISTOR_ENABLED, PULL_UP, INTERRUPT_ENABLED, TRIGGER_FALLING> BUTTON;
typedef GPIO_MODULE_T<1, 4, 1> SMCLK_OUT;
typedef GPIO_INPUT_T<1, 5, RESISTOR_ENABLED, PULL_UP> P1_5;
typedef GPIO_OUTPUT_T<1, 6, LOW> LED_GREEN;
typedef GPIO_INPUT_T<1, 7, RESISTOR_ENABLED, PULL_UP> P1_7;
typedef GPIO_INPUT_T<2, 0, RESISTOR_ENABLED, PULL_UP> P2_0;
typedef GPIO_INPUT_T<2, 1, RESISTOR_ENABLED, PULL_UP> P2_1;
typedef GPIO_INPUT_T<2, 2, RESISTOR_ENABLED, PULL_UP> P2_2;
typedef GPIO_INPUT_T<2, 3, RESISTOR_ENABLED, PULL_UP> P2_3;
typedef GPIO_INPUT_T<2, 4, RESISTOR_ENABLED, PULL_UP> P2_4;
typedef GPIO_INPUT_T<2, 5, RESISTOR_ENABLED, PULL_UP> P2_5;
typedef GPIO_INPUT_T<2, 6, RESISTOR_ENABLED, PULL_UP> P2_6;
typedef GPIO_INPUT_T<2, 7, RESISTOR_ENABLED, PULL_UP> P2_7;
typedef GPIO_PORT_T<1, LED_GREEN, SMCLK_OUT, ACLK_OUT, BUTTON> PORT1;

typedef TIMEOUT_T<WDT> TIMEOUT;

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

void port1_irq(void) __attribute__((interrupt(PORT1_VECTOR)));
void port1_irq(void)
{
	if (PORT1::handle_irq()) {
		exit_idle();
	}
}

int main(void)
{
	ACLK::init();
	PORT1::init();
	WDT::init();
	WDT::enable_irq();
	while (1) {
		BUTTON::wait_for_irq();
		LED_GREEN::toggle();
		TIMEOUT::set_and_wait(1000);
		LED_GREEN::toggle();
	}
}
