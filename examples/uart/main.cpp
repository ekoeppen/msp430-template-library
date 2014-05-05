#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <spi.h>
#include <uart.h>
#include <io.h>

typedef ACLK_T<ACLK_SOURCE_VLOCLK> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef GPIO_PORT_T<1, RX, TX> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
typedef UART_T<USCI_A, 0, SMCLK> UART;

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
