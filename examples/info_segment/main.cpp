#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <spi.h>
#include <uart.h>
#include <io.h>
#include <tlv.h>

typedef ACLK_T<ACLK_SOURCE_VLOCLK> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef GPIO_PORT_T<1, RX, TX> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
typedef UART_T<USCI_A, 0, SMCLK> UART;

typedef TLV_T<&__infoa> CALIBRATION_DATA;

struct ITERATOR {
	static void handle_tag(uint8_t tag, uint8_t length, void *value) {
		printf<UART>("Tag %02x length %d address %04x\n",
				tag, length, value);
	}
};

int main(void)
{
	WDT::disable();
	ACLK::init();
	SMCLK::init();
	PORT1::init();
	UART::init();
	CALIBRATION_DATA::iterate<ITERATOR>();
	while (1) {
		enter_idle();
	}
}
