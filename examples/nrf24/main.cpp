#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <nrf24.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#include <usci_spi.h>
#else
#include <soft_uart.h>
#include <usi_spi.h>
#endif

extern constexpr uint8_t rx_addr[5] = {
	0x03, 0xf0, 0xf0, 0xf0, 0xf0
};

const uint8_t BROADCAST_ADDR[] = {0x00, 0xf0, 0xf0, 0xf0, 0xf0};

typedef ACLK_T<ACLK_SOURCE_VLOCLK> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
typedef USCI_SPI_T<USCI_B, 0, SMCLK> SPI;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef SOFT_UART_T<TX, RX, SMCLK> UART;
typedef USI_SPI_T<SMCLK> SPI;
#endif
typedef GPIO_MODULE_T<1, 5, 3> SCK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;

typedef GPIO_OUTPUT_T<2, 0, LOW> CE;
typedef GPIO_OUTPUT_T<2, 1, HIGH> CSN;
typedef GPIO_INPUT_T<2, 2> IRQ;

typedef GPIO_PORT_T<1, LED_RED, SCK, MISO, MOSI, RX, TX> PORT1;
typedef GPIO_PORT_T<2, SCK, CSN, CE> PORT2;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_8192> WDT;

typedef TIMEOUT_T<WDT> TIMEOUT;

typedef NRF24_T<TIMEOUT, SPI, SCK, CSN, CE, IRQ> NRF24;

int main(void)
{
	uint8_t regs[64];
	uint8_t packet[24];

	ACLK::init();
	SMCLK::init();
	WDT::init();
	WDT::enable_irq();

	PORT1::init();
	PORT2::init();

	UART::init();
	printf<UART>("NRF24 example start.\n");

	SPI::init();

	for (int i = 0; i < sizeof(regs); i++) regs[i] = 0;
	for (int i = 0; i < sizeof(packet); i++) packet[i] = 0;
	NRF24::init();
	NRF24::set_rx_addr(rx_addr);
	NRF24::set_channel(70);
	NRF24::read_regs(regs);
	hex_dump_bytes<UART>(regs, sizeof(regs));
	while (1) {
		NRF24::tx_buffer(BROADCAST_ADDR, packet, sizeof(packet), false);
		LED_RED::toggle();
		TIMEOUT::set_timeout(1000);
		enter_idle<WDT>();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::timeout_triggered()) exit_idle();
}

#ifdef __MSP430_HAS_USCI__
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
