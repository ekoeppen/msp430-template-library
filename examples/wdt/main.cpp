#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <timer.h>
#include <soft_uart.h>
#endif

typedef LFXT1CLK_T<> LFXT1;
typedef DCOCLK_T<> DCO;
typedef ACLK_T<LFXT1> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<1, 6, LOW> LED_GREEN;
typedef GPIO_MODULE_T<1, 4, 1> SMCLK_OUT;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;
#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef TIMER_T<TIMER_A, 0, SMCLK, TASSEL_2 + MC_2> TIMER;
typedef SOFT_UART_T<TIMER, TX, RX> UART;
#endif

typedef GPIO_PORT_T<1, LED_RED, LED_GREEN, RX, TX, SMCLK_OUT> PORT1;

typedef TIMEOUT_T<WDT> TIMEOUT;

int main(void)
{
	DCO::init();

	MCLK::init();
	SMCLK::init();
	ACLK::init();

	WDT::init();
	PORT1::init();

	WDT::enable_irq();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	UART::init();
	UART::puts("WDT example start\n");
	while (1) {
		LED_RED::toggle();
		UART::disable();
		TIMEOUT::set_and_wait(1000);
		UART::enable();
		printf<UART>("Timeout\n");
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	LED_GREEN::toggle();
	if (TIMEOUT::count_down()) exit_idle();
}

#ifdef __MSP430_HAS_USCI__
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (UART::handle_tx_irq()) {
		exit_idle();
	}
}
#endif
