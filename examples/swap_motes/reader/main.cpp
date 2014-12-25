#include <msp430.h>
#include <stdint.h>
#include <string.h>

#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <timer.h>
#include <nrf24.h>
#include <flash.h>
#include <adc.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_spi.h>
#include <usci_uart.h>
#else
#include <usi_spi.h>
#include <soft_uart.h>
#endif

#include <swap_mote.h>
#include <soft_rtc.h>

typedef LFXT1CLK_T<XCAP_3> LFXT1;
typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<12000000> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef GPIO_MODULE_T<1, 5, 3> SCLK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef GPIO_PIN_T<1, 5, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCLK;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MOSI;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MISO;
#endif
typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_MODULE_T<1, 4, 1> SMCLK_OUT;
typedef GPIO_OUTPUT_T<2, 0, LOW> CE;
typedef GPIO_OUTPUT_T<2, 1, HIGH> CSN;
typedef GPIO_INPUT_T<2, 2, RESISTOR_ENABLED, PULL_UP, INTERRUPT_ENABLED, TRIGGER_FALLING> IRQ;
typedef GPIO_PORT_T<1, LED_RED, SCLK, MISO, MOSI, RX, TX, SMCLK_OUT> PORT1;
typedef GPIO_PORT_T<2, IRQ, CSN, CE> PORT2;

typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
typedef SOFT_RTC_T<WDT> RTC;
typedef FLASH_T<SMCLK, WDT, &__infod> CONFIGURATION;

#ifdef __MSP430_HAS_USCI__
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
typedef USCI_SPI_T<USCI_B, 0, SMCLK, true, 0> SPI;
#else
typedef SOFT_UART_T<TIMER, TX, RX> UART;
typedef USI_SPI_T<SMCLK, true, 0> SPI;
#endif

typedef TIMEOUT_T<WDT> TIMEOUT;
typedef NRF24_T<SPI, CSN, CE, IRQ, MCLK> NRF24;
typedef SWAP_MOTE_T<1, 1, 1, 1, NRF24, 70, CONFIG_STORAGE_UNUSED> MOTE;

DATE_TIME date_time;

void set_rtc(uint8_t* raw_data)
{
	date_time.year = raw_data[0] + (raw_data[1] << 8);
	date_time.month = raw_data[2];
	date_time.day = raw_data[3];
	date_time.hours = raw_data[4];
	date_time.minutes = raw_data[5];
	date_time.seconds = raw_data[6];
	RTC::set(date_time);
}

void print_rtc(void)
{
	RTC::get(date_time);
	printf<UART>("RTC: %04d-%02d-%02d %02d:%02d:%02d\n", date_time.year, date_time.month, date_time.day, date_time.hours, date_time.minutes, date_time.seconds);
}

void handle_status(void)
{
	if (MOTE::rx_packet.len > 0) {
		hex_dump_bytes<UART>((uint8_t *) &MOTE::rx_packet, MOTE::rx_packet.len);
		printf<UART>("Packet from %d, register %d\n", MOTE::rx_packet.src, MOTE::rx_packet.reg_id);
		if (MOTE::rx_packet.reg_id == 129) {
			printf<UART>("Got RTC update\n");
			set_rtc(MOTE::rx_packet.reg_value);
		}
	}
}

int main(void)
{
	//LFXT1::init();
	static uint8_t regs[64];
	DCO::init();
	ACLK::init();
	SMCLK::init();
	TIMER::init();

	WDT::init();
	WDT::enable_irq();

	RTC::init();

	PORT1::init();
	PORT2::init();
	UART::init();
	printf<UART>("Mote example start\n");
	SPI::init();
	NRF24::init();
	MOTE::init();
	//TIMEOUT::set(5000);
	//MOTE::announce<TIMEOUT>();
	while (1) {
		LED_RED::set_high();
#if 0
		printf<UART>("VLOCLK usage: %d\n", VLOCLK_usage_count);
		print_rtc();
		MOTE::start_tx();
		printf<UART>("Sending query, %d\n", VLOCLK_usage_count);
		MOTE::tx_packet.reg_id = 129;
		MOTE::tx_packet.len = 7;
		MOTE::send_query_packet(1, 1);
		MOTE::start_rx();
		printf<UART>("VLOCLK usage: %d\n", VLOCLK_usage_count);
		printf<UART>("Awaiting response\n");
		TIMEOUT::set(5000);
		while (!TIMEOUT::triggered()) {
			MOTE::handle_radio<TIMEOUT, handle_status>();
		}
		TIMEOUT::disable();
		printf<UART>("VLOCLK usage: %d\n", VLOCLK_usage_count);
		NRF24::read_regs(regs);
		hex_dump_bytes<UART>(regs, sizeof(regs));
#endif
		UART::puts("Entering sync state\n");
		TIMEOUT::set(30000);
		MOTE::sync<TIMEOUT>();
		NRF24::read_regs(regs);
		hex_dump_bytes<UART>(regs, sizeof(regs));
		MOTE::rx_off();
		LED_RED::set_low();

		NRF24::power_down();
		printf<UART>("Sleeping\n");
		MOTE::sleep<TIMEOUT>();
	}
	return 0;
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	RTC::update();
	if (TIMEOUT::count_down()) exit_idle();
}

void port2_irq(void) __attribute__((interrupt(PORT2_VECTOR)));
void port2_irq(void)
{
	if (PORT2::handle_irq()) {
		exit_idle();
	}
}

#ifdef __MSP430_HAS_USCI__
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (UART::handle_tx_irq()) exit_idle();
	if (SPI::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (UART::handle_rx_irq()) exit_idle();
	if (SPI::handle_rx_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#endif
