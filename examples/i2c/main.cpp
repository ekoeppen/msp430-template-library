//#define USE_SOFT_SPI

#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#if defined(USE_SOFT_SPI)
#include <soft_i2c.h>
#elif defined(__MSP430_HAS_USCI__)
#include <usci_i2c.h>
#else
#include <usi_i2c.h>
#endif
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <timer.h>
#include <soft_uart.h>
#endif

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<12000000> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
#if defined(USE_SOFT_SPI)
typedef GPIO_INPUT_T<1, 6> SCL;
typedef GPIO_OUTPUT_T<1, 7> SDA;
typedef SOFT_SPI_T<SCL, SDA> I2C;
#elif defined(__MSP430_HAS_USCI__)
typedef USCI_I2C_T<USCI_B, 0, SMCLK> I2C;
typedef GPIO_MODULE_T<1, 6, 3> SCL;
typedef GPIO_MODULE_T<1, 7, 3> SDA;
#else
typedef USI_I2C_T<SMCLK> I2C;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCL;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SDA;
#endif
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
typedef GPIO_MODULE_T<1, 0, 1> SMCLK_OUT;
typedef GPIO_OUTPUT_T<1, 0> LED;
typedef GPIO_MODULE_T<1, 4, 1> ACLK_OUT;
typedef GPIO_OUTPUT_T<1, 3, LOW> CS;

typedef GPIO_PORT_T<1, LED, ACLK_OUT, RX, TX, CS, SCL, SDA> PORT1;

typedef TIMEOUT_T<WDT> TIMEOUT;

uint8_t buffer[1];

int main(void)
{
	DCO::init();
	ACLK::init();
	SMCLK::init();
	WDT::init();
	P1DIR = BIT6;
	while (!(P1IN & BIT7)) {
		P1OUT ^= BIT6;
	}
	PORT1::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	//UART::init();
	I2C::init();
	WDT::enable_irq();
	P2OUT = 0;
	P2DIR = BIT0 + BIT1 + BIT2 + BIT3 + BIT4;
	//UART::puts("I2C test starting...\n");
	while (1) {
		LED::toggle();
		CS::set_high();
		I2C::set_slave_addr(0b1110111);
		//I2C::send((const uint8_t *) "\xd0\x55", 2, true);
		//I2C::send((const uint8_t *) "\xd0", 1, false);
		//I2C::receive(buffer, sizeof(buffer));
		//I2C::transfer((const uint8_t *) "\xd0\x55", 2, true);
		I2C::transfer((const uint8_t *) "\xd0", 1, true);
		I2C::transfer(buffer, sizeof(buffer), false);
		CS::set_low();
		TIMEOUT::set_and_wait(1000);
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

#if !defined(USE_SOFT_SPI)
#if defined( __MSP430_HAS_USCI__)
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (I2C::handle_tx_irq()) exit_idle();
	//if (UART::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	//if (UART::handle_rx_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (I2C::handle_irq()) exit_idle();
}
#endif
#endif
