#include <gpio.h>
#include <clocks.h>
#include <tasks.h>
#include <wdt.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <timer.h>
#include <soft_uart.h>
#endif
#include <io.h>
#include <adc.h>

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

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
typedef GPIO_ANALOG_T<1, 3> ADC_INPUT;
typedef GPIO_ANALOG_T<1, 4> ADC_REFOUT;
typedef GPIO_PORT_T<1, RX, TX, ADC_INPUT, ADC_REFOUT> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

typedef ADC10_T<ADC10OSC, SREF_1 + ADC10SHT_3 + REFON + REFOUT + ADC10ON + REF2_5V, 0, ADC_INPUT> ADC_CHANNEL_3;

typedef TIMEOUT_T<WDT> TIMEOUT;

int adc_read_ref(int channel, int ref)
{
	int r;

	ADC10CTL0 &= ~ENC;
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + REFOUT + ADC10ON + ref;
	ADC10CTL1 = ADC10SSEL_3 + channel;
	__delay_cycles (128);
	ADC10CTL0 |= ENC + ADC10SC;
        while( ADC10CTL1 & ADC10BUSY ) ;

	r = ADC10MEM;
	ADC10CTL0 &= ~ENC;

	return r;
}

int adc_read_single(int channel)
{
	int r;

	ADC10CTL0 &= ~ENC;
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON;
	ADC10CTL1 = ADC10SSEL_3 + channel;
	ADC10CTL0 |= ENC + ADC10SC;
        while( ADC10CTL1 & ADC10BUSY ) ;

	r = ADC10MEM;
	ADC10CTL0 &= ~ENC;

	return r;
}

unsigned read_voltage(void)
{
	unsigned adc, voltage;

	ADC10CTL1 = INCH_11 | ADC10DIV_3 | ADC10SSEL_3;
	ADC10CTL0 = ADC10SHT_3 | ADC10ON | ENC | ADC10SC | REFON | REFBURST | SREF_1;
	while (ADC10CTL1 & ADC10BUSY) ;
	adc = ADC10MEM;
	ADC10CTL0 &= ~ENC;
	if (adc >= 0x380) {
		ADC10CTL0 |= ENC | REF2_5V | ADC10SC;
		while (ADC10CTL1 & ADC10BUSY) ;
		adc = ADC10MEM;
		ADC10CTL0 &= ~ENC;
		voltage = ((unsigned long) adc * 5L * 125L / 128L);
	} else {
		voltage = ((unsigned long) adc * 3L * 125L / 128L);
	}

	return voltage;
}

int main(void)
{
	unsigned int vcc_milli, adc, adc_ref, adc_norm, r;

	DCO::init();

	SMCLK::init();
	ACLK::init();
	MCLK::init();
	WDT::init();
	PORT1::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	UART::init();
	WDT::enable_irq();
	printf<UART>("ADC example:\n");
	while (1) {
		ADC_CHANNEL_3::init();
		vcc_milli = read_voltage(); // (unsigned long) (adc_read_ref(INCH_11, REF2_5V) * 5L * 125L / 128L);
		adc = ADC_CHANNEL_3::sample_once(); // adc_read_single(INCH_3);
		adc = adc_read_single(INCH_3);
		adc_ref = adc_read_ref(INCH_3, REF2_5V);
		adc_norm = ((unsigned long) adc * (unsigned long) vcc_milli) >> 10;
		r = 10000L * (unsigned long) adc_norm / (unsigned long) (vcc_milli - adc_norm);
		printf<UART>("\rADC: no ref: %d ref 2.5: %d norm: %d r: %d V: %d\033[K",
			     adc, adc_ref, adc_norm, r, vcc_milli);
		ADC_CHANNEL_3::disable();
		TIMEOUT::set(1000);
		enter_idle();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}


void adc_irq(void) __attribute__((interrupt(ADC10_VECTOR)));
void adc_irq(void)
{
	exit_idle();
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
