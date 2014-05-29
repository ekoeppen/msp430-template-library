#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <tlv.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_uart.h>
#else
#include <timer.h>
#include <soft_uart.h>
#endif

typedef ACLK_T<ACLK_SOURCE_VLOCLK> ACLK;
typedef SMCLK_T<CLK_SOURCE_DCOCLK, 1000000> SMCLK;

#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef TIMER_T<TIMER_A, 0, SMCLK, TASSEL_2 + MC_2> TIMER;
typedef SOFT_UART_T<TIMER, TX, RX> UART;
#endif
typedef GPIO_PORT_T<1, RX, TX> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

typedef TLV_T<SMCLK, WDT, &__infoa> CALIBRATION_DATA;
typedef TLV_T<SMCLK, WDT, &__infod> SETTINGS;

struct ITERATOR {
	static void handle_tag(uint8_t tag, uint8_t length, void *value) {
		printf<UART>("Tag %02x length %d address %04x\n",
				tag, length, value);
	}
};

int main(void)
{
	unsigned settings_value = 0x6633;

	WDT::disable();
	ACLK::init();
	SMCLK::init();
	PORT1::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	UART::init();
	printf<UART>("Calibration data:\n");
	CALIBRATION_DATA::iterate<ITERATOR>();
	if (!SETTINGS::verify_checksum()) {
		unsigned values[2] = {0x02fd, 0x1234};
		printf<UART>("Clearing settings (checksum was %04x)\n", SETTINGS::checksum());
		SETTINGS::write(values, values + 2);
	}
	SETTINGS::set_tag(0xfd, &settings_value);
	printf<UART>("Settings:\n");
	SETTINGS::iterate<ITERATOR>();
	printf<UART>("Done.\n");
	while (1) {
		enter_idle();
	}
}
