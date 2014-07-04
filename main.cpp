#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <spi.h>
#include <uart.h>
#include <io.h>

typedef ACLK_T<> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_PIN_T<1, 0, OUTPUT, false> LED_RED;
typedef GPIO_PIN_T<1, 1, OUTPUT, false, false, RISING, 3> RX;
typedef GPIO_PIN_T<1, 2, INPUT, false, false, RISING, 3> TX;
typedef GPIO_PIN_T<1, 4, OUTPUT, true> CS;
typedef GPIO_PIN_T<1, 5, OUTPUT, false, false, RISING, 3> SCK;
typedef GPIO_PIN_T<1, 6, INPUT, false, false, RISING, 3> MISO;
typedef GPIO_PIN_T<1, 7, OUTPUT, false, false, RISING, 3> MOSI;
typedef GPIO_PORT_T<1, LED_RED, RX, TX, CS, SCK, MISO, MOSI> PORT2;

typedef WDT_T<ACLK, WDT_TIMER> WDT;
typedef SPI_T<USCI_B, 0, SMCLK> SPI;
typedef UART_T<USCI_A, 0, SMCLK> UART;

typedef TIMEOUT_T<WDT> TIMEOUT;

int main(void)
{
	ACLK::init();
	SMCLK::init();
	WDT::init();
	PORT2::init();
	SPI::init();
	UART::init();
	WDT::enable_irq();
	while (1) {
		LED_RED::toggle();

		CS::set_low();
		SPI::transfer((uint8_t *) "abc", 3);
		CS::set_high();

		TIMEOUT::set(1000);
		printf<UART>("WDT frequency: %d Alarm: %d\n", WDT::frequency, TIMEOUT::timeout);
		enter_idle<WDT>();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

void usci_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}

