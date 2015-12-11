//#define I2C_POLLING
#define USE_SOFT_I2C

#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#if defined(USE_SOFT_I2C)
#include <soft_i2c.h>
#elif defined(__MSP430_HAS_USCI__)
#include <usci_i2c.h>
#else
#include <usi_i2c.h>
#endif
#include <drivers/si70xx.h>

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<12000000> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
#if defined(USE_SOFT_I2C)
typedef GPIO_INPUT_T<1, 1> SCL;
typedef GPIO_INPUT_T<1, 2> SDA;
typedef SOFT_I2C_T<MCLK, SCL, SDA> I2C;
#elif defined(__MSP430_HAS_USCI__)
typedef USCI_I2C_T<USCI_B, 0, SMCLK> I2C;
typedef GPIO_MODULE_T<1, 6, 3> SCL;
typedef GPIO_MODULE_T<1, 7, 3> SDA;
#else
typedef USI_I2C_T<SMCLK> I2C;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCL;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SDA;
#endif
typedef GPIO_OUTPUT_T<1, 0> LED;
typedef GPIO_OUTPUT_T<1, 1> PIN1;
typedef GPIO_OUTPUT_T<1, 2> PIN2;
typedef GPIO_OUTPUT_T<1, 3> PIN3;
typedef GPIO_OUTPUT_T<1, 4> PIN4;

typedef GPIO_PORT_T<1, LED, PIN1, PIN2, PIN3, PIN4, SCL, SDA> PORT1;
typedef SI70XX::T<I2C> SENSOR;

typedef TIMEOUT_T<WDT> TIMEOUT;

static int16_t temperature;
static uint16_t humidity;

int main(void)
{
	DCO::init();
	ACLK::init();
	SMCLK::init();
	WDT::init();
	/*
	P1DIR = BIT6;
	while (!(P1IN & BIT7)) {
		P1OUT ^= BIT6;
	}
	*/
	PORT1::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	I2C::init();
	SENSOR::init();
	WDT::enable_irq();
	while (1) {
		temperature = SENSOR::temperature();
		humidity = SENSOR::humidity();
		LED::toggle();
		TIMEOUT::set_and_wait(2000);
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

#if !defined(USE_SOFT_I2C) && !defined(I2C_POLLING)
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
#endif
