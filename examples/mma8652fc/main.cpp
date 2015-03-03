#include <msp430.h>
#include <stdint.h>
#include <string.h>
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
#include <drivers/mma8652fc.h>

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
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2, HIGH> TX;
typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
#endif
typedef GPIO_MODULE_T<1, 0, 1> SMCLK_OUT;
typedef GPIO_MODULE_T<1, 4, 1> ACLK_OUT;

typedef GPIO_OUTPUT_T<2, 3, LOW> LED;
typedef GPIO_PORT_T<1, RX, TX, SCL, SDA> PORT1;
typedef GPIO_PORT_T<2, LED> PORT2;

typedef TIMEOUT_T<WDT> TIMEOUT;
typedef MMA8652FC::T<I2C, MMA8652FC::FAST_READ, MMA8652FC::RANGE_2G, MMA8652FC::RATE_800HZ, MMA8652FC::NORMAL> ACCELEROMETER;

int16_t z;
int16_t y;
int16_t x;

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
	PORT2::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	I2C::init();
	WDT::enable_irq();
	ACCELEROMETER::init();
	ACCELEROMETER::wakeup();
	while (1) {
		ACCELEROMETER::read_regs();
		ACCELEROMETER::sample();
		x = ACCELEROMETER::x;
		y = ACCELEROMETER::y;
		z = ACCELEROMETER::z;
		LED::set(z > 0 ? 1 : 0);
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

#if defined( __MSP430_HAS_USCI__)
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (I2C::handle_tx_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (I2C::handle_irq()) exit_idle();
}
#endif
