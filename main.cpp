#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <spi.h>
#include <uart.h>

typedef ACLK_T<> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_PIN_T<1, 0, OUTPUT> LED_RED;
typedef GPIO_PIN_T<1, 1, OUTPUT, false, RISING, 3> RX;
typedef GPIO_PIN_T<1, 2, INPUT, false, RISING, 3> TX;
typedef GPIO_PIN_T<1, 4, OUTPUT> CS;
typedef GPIO_PIN_T<1, 5, OUTPUT, false, RISING, 3> SCK;
typedef GPIO_PIN_T<1, 6, INPUT, false, RISING, 3> MISO;
typedef GPIO_PIN_T<1, 7, OUTPUT, false, RISING, 3> MOSI;
typedef GPIO_PORT_T<1, LED_RED, RX, TX, CS, SCK, MISO, MOSI> PORT2;

typedef WDT_T<ACLK, WDT_TIMER> WDT;
typedef SPI_T<USCI_B, 0, SMCLK> SPI;
typedef UART_T<USCI_A, 0, SMCLK> UART;

int main(void)
{
	ACLK::init();
	SMCLK::init();
	WDT::init();
	LED_RED::init();
	PORT2::init();
	CS::set_high();
	SPI::init();
	UART::init();
	WDT::enable_irq();
	while (1) {
		LED_RED::toggle();
		CS::set_low();
		SPI::transfer((uint8_t *) "abc", 3);
		for (uint8_t i = 32; i < 128; i++) UART::putc(i);
		CS::set_high();
		WDT::enter_idle();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	WDT::resume_irq();
}

void usci_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_irq(void)
{
	if (SPI::handle_irq()) SPI::resume_irq();
}

