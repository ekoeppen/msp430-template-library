#define USE_SOFT_SPI

#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#if defined(USE_SOFT_SPI)
#include <soft_spi.h>
#elif defined(__MSP430_HAS_USCI__)
#include <usci_spi.h>
#else
#include <usi_spi.h>
#endif

typedef ACLK_T<ACLK_SOURCE_LFXT1CLK> ACLK;
typedef SMCLK_T<CLK_SOURCE_DCOCLK, 1000000> SMCLK;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_64> WDT;
#if defined(USE_SOFT_SPI)
typedef GPIO_OUTPUT_T<1, 5> SCLK;
typedef GPIO_INPUT_T<1, 6> MISO;
typedef GPIO_OUTPUT_T<1, 7> MOSI;
typedef SOFT_SPI_T<SMCLK, SCLK, MOSI, MISO, true, 0> SPI;
#elif defined(__MSP430_HAS_USCI__)
typedef USCI_SPI_T<USCI_B, 0, SMCLK> SPI;
typedef GPIO_MODULE_T<1, 5, 3> SCLK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;
#else
typedef USI_SPI_T<SMCLK, true, 0> SPI;
typedef GPIO_PIN_T<1, 5, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCLK;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MOSI;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MISO;
#endif
typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<1, 4, HIGH> CS;

typedef GPIO_PORT_T<1, LED_RED, CS, SCLK, MISO, MOSI> PORT1;

typedef TIMEOUT_T<WDT> TIMEOUT;

int main(void)
{
	ACLK::init();
	SMCLK::init();
	WDT::init();
	PORT1::init();
	SPI::init();
	WDT::enable_irq();
	while (1) {
		LED_RED::toggle();
		CS::set_low();
		SPI::transfer((uint8_t *) "abcd", 4);
		CS::set_high();

		TIMEOUT::set_timeout(500);
		enter_idle<WDT>();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::timeout_triggered()) exit_idle();
}

#if !defined(USE_SOFT_SPI)
#if defined( __MSP430_HAS_USCI__)
void usci_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#endif
#endif
