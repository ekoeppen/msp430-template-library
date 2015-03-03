//#define USE_SOFT_SPI

//#define I2C_POLLING

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
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2, HIGH> TX;
typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
#endif
typedef GPIO_MODULE_T<1, 0, 1> SMCLK_OUT;
typedef GPIO_OUTPUT_T<1, 0> LED;
typedef GPIO_MODULE_T<1, 4, 1> ACLK_OUT;
typedef GPIO_OUTPUT_T<1, 3, LOW> CS;

typedef GPIO_PORT_T<1, LED, ACLK_OUT, RX, TX, CS, SCL, SDA> PORT1;

typedef TIMEOUT_T<WDT> TIMEOUT;

uint8_t chip_id[1];
uint8_t ctrl_meas[1];
uint8_t out[3];

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
	I2C::init();
	I2C::set_slave_addr(0b1110111);
	WDT::enable_irq();
	while (1) {
		LED::toggle();

		I2C::write((const uint8_t *) "\xf4\xc0", 2, false);

		I2C::write((const uint8_t *) "\xd0", 1, true);
		I2C::read(chip_id, sizeof(chip_id));

		I2C::write((const uint8_t *) "\xf4", 1, true);
		I2C::read(ctrl_meas, sizeof(ctrl_meas));

		I2C::write((const uint8_t *) "\xf6", 1, true);
		I2C::read(out, sizeof(out));

		I2C::write_reg(0xf4, 0xc0);
		chip_id[0] = I2C::read_reg(0xd0);
		ctrl_meas[0] = I2C::read_reg(0xf4);
		I2C::read_reg(0xf6, out, sizeof(out));

		TIMEOUT::set_and_wait(1000);
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

#if !defined(USE_SOFT_SPI) && !defined(I2C_POLLING)
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
