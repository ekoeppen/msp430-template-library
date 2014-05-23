#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <soft_uart.h>
#endif
#include <io.h>

typedef ACLK_T<ACLK_SOURCE_VLOCLK> ACLK;
typedef SMCLK_T<> SMCLK;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef SOFT_UART_T<TX, RX, SMCLK> UART;
#endif
typedef GPIO_PORT_T<1, RX, TX> PORT1;

int main(void)
{
	char c;

	WDT::disable();
	ACLK::init();
	SMCLK::init();
	PORT1::init();
	UART::init();
	while (1) {
		printf<UART>("Press any key...\n");
		c = UART::getc();
		printf<UART>("Key pressed = %c (code %d, hex %x)\n", c, c, c);
	}
}
