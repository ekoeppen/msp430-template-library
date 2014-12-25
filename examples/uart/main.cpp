#define SENDING
// #define RECEIVING

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

typedef VLOCLK_T<12500> VLO;
typedef DCOCLK_T<1000000> DCO;
typedef LFXT1CLK_T<XCAP_3> LFXT1;

typedef ACLK_T<LFXT1> ACLK;
typedef SMCLK_T<DCO> SMCLK;
typedef MCLK_T<DCO> MCLK;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2, HIGH> TX;
typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef SOFT_UART_T<TIMER, TX, RX> UART;
#endif
typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<1, 6, LOW> LED_GREEN;
typedef GPIO_MODULE_T<1, 4, 1> SMCLK_OUT;
typedef GPIO_PORT_T<1, RX, TX, LED_GREEN, LED_RED, SMCLK_OUT> PORT1;

typedef TIMEOUT_T<WDT> TIMEOUT;

int main(void)
{
	char c;

	DCO::init();
	LFXT1::init();

	MCLK::init();
	SMCLK::init();
	ACLK::init();

#ifdef RECEIVING
	WDT::hold();
#else
	WDT::init();
	WDT::enable_irq();
#endif
	PORT1::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	UART::init();
	while (1) {
		LED_GREEN::toggle();
#ifdef RECEIVING
		UART::puts("Press any key...\n");
		c = UART::getc();
		printf<UART>("Key pressed = %c (code %d, hex %x, error %d)\n", c, c, c, UART::status.framing_error);
#else
		LED_RED::set_high();
		UART::transfer<TIMEOUT_IMMEDIATELY>((uint8_t *) "0123456789\n", 11);
		LED_RED::set_low();
		UART::disable();
		TIMEOUT::set_and_wait(2000);
		UART::enable();
#endif
	}
}

#ifdef __MSP430_HAS_USCI__
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (UART::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (UART::handle_rx_irq()) exit_idle();
}
#endif

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}
