#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <cc110x.h>
#include <timer.h>
#if defined(USE_SOFT_SPI)
#include <soft_spi.h>
#elif defined __MSP430_HAS_USCI__
#include <usci_spi.h>
#else
#include <usi_spi.h>
#endif

#if defined __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <soft_uart.h>
#endif

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<12000000> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<1, 3, LOW> WDT_ACTIVE;
typedef GPIO_MODULE_T<1, 4, 1> SMCLK_OUT;
#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef SOFT_UART_T<TIMER, TX, RX> UART;
#endif

#if defined(USE_SOFT_SPI)
typedef GPIO_OUTPUT_T<1, 5> SCLK;
typedef GPIO_INPUT_T<1, 7> MISO;
typedef GPIO_OUTPUT_T<1, 6> MOSI;
typedef SOFT_SPI_T<SCLK, MOSI, MISO, true, 0> SPI;
#elif defined(__MSP430_HAS_USCI__)
typedef GPIO_MODULE_T<1, 5, 3> SCLK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;
typedef USCI_SPI_T<USCI_B, 0, SMCLK, true, 0> SPI;
#else
typedef GPIO_PIN_T<1, 5, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCLK;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MOSI;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MISO;
typedef USI_SPI_T<SMCLK, true, 0> SPI;
#endif
typedef GPIO_INPUT_T<2, 0, RESISTOR_DISABLED, PULL_DOWN, INTERRUPT_ENABLED, TRIGGER_FALLING> IRQ;
typedef GPIO_OUTPUT_T<2, 1, HIGH> CSN;

typedef GPIO_PORT_T<1, LED_RED, SCLK, MISO, MOSI, RX, TX, SMCLK_OUT, WDT_ACTIVE> PORT1;
typedef GPIO_PORT_T<2, IRQ, CSN> PORT2;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

typedef TIMEOUT_T<WDT> TIMEOUT;

typedef CC110X_T<SPI, CSN, MISO, IRQ, MCLK, 0> CC1101;

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

void port2_irq(void) __attribute__((interrupt(PORT2_VECTOR)));
void port2_irq(void)
{
	if (PORT2::handle_irq()) {
		exit_idle();
	}
}

#ifndef USE_SOFT_SPI
#ifdef __MSP430_HAS_USCI__
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (SPI::handle_tx_irq()) exit_idle();
	if (UART::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (SPI::handle_rx_irq()) exit_idle();
	if (UART::handle_rx_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#endif
#else
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
#endif
