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

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_INPUT_T<1, 1, RESISTOR_ENABLED, PULL_UP> P1_1;
typedef GPIO_INPUT_T<1, 2, RESISTOR_ENABLED, PULL_UP> P1_2;
typedef GPIO_INPUT_T<1, 3, RESISTOR_ENABLED, PULL_UP, INTERRUPT_ENABLED, TRIGGER_FALLING> BUTTON;
typedef GPIO_INPUT_T<1, 4, RESISTOR_ENABLED, PULL_UP> P1_4;
typedef GPIO_INPUT_T<1, 5, RESISTOR_ENABLED, PULL_UP> P1_5;
typedef GPIO_OUTPUT_T<1, 6, LOW> P1_6;
typedef GPIO_INPUT_T<1, 7, RESISTOR_ENABLED, PULL_UP> P1_7;
typedef GPIO_INPUT_T<2, 0, RESISTOR_ENABLED, PULL_UP> P2_0;
typedef GPIO_INPUT_T<2, 1, RESISTOR_ENABLED, PULL_UP> P2_1;
typedef GPIO_INPUT_T<2, 2, RESISTOR_ENABLED, PULL_UP> P2_2;
typedef GPIO_INPUT_T<2, 3, RESISTOR_ENABLED, PULL_UP> P2_3;
typedef GPIO_INPUT_T<2, 4, RESISTOR_ENABLED, PULL_UP> P2_4;
typedef GPIO_INPUT_T<2, 5, RESISTOR_ENABLED, PULL_UP> P2_5;
typedef GPIO_INPUT_T<2, 6, RESISTOR_ENABLED, PULL_UP> P2_6;
typedef GPIO_INPUT_T<2, 7, RESISTOR_ENABLED, PULL_UP> P2_7;
typedef GPIO_PORT_T<1, LED_RED, BUTTON, P1_1, P1_2, P1_4, P1_5, P1_6, P1_7> PORT1;
typedef GPIO_PORT_T<2, P2_0, P2_1, P2_2, P2_3, P2_4, P2_5, P2_6, P2_7> PORT2;

void port1_irq(void) __attribute__((interrupt(PORT1_VECTOR)));
void port1_irq(void)
{
	LED_RED::toggle();
	PORT1::clear_irq();
}

int main(void)
{
	WDT::hold();
	PORT1::init();
	PORT2::init();
	while (1) {
		enter_idle();
	}
}
