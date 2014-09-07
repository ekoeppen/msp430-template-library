#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <timer.h>
#include <dht22.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <soft_uart.h>
#endif

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
typedef TIMEOUT_T<WDT> TIMEOUT;

typedef GPIO_OUTPUT_T<1, 0> LED_RED;
#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef SOFT_UART_T<TIMER, TX, RX> UART;
#endif
typedef GPIO_PIN_T<2, 3, OUTPUT, LOW> DHT_POWER;
typedef GPIO_PIN_T<2, 5, OUTPUT, HIGH> DHT_DATA;
typedef GPIO_PORT_T<1, RX, TX, LED_RED> PORT1;
typedef GPIO_PORT_T<2, DHT_POWER, DHT_DATA> PORT2;

int main(void)
{
	unsigned char buffer[6];
	int r;
	unsigned int humidity;
	int temperature;

	DCO::init();
	ACLK::init();
	SMCLK::init();
	WDT::init();
	TIMER::init();
	PORT1::init();
	PORT2::init();
	UART::init();
	WDT::enable_irq();
	printf<UART>("DHT22 example\n");
	while (1) {
		LED_RED::toggle();
		r = read_dht<TIMER, TIMEOUT, DHT_DATA, DHT_POWER>(&temperature, &humidity);
		printf<UART>("%d %d (%d)\n", temperature, humidity, r);
		TIMEOUT::set_and_wait(1000);
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
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
