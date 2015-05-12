#include "common.h"

static constexpr uint8_t cc110x_test_init_values[][2] = {
  {CC1101_IOCFG2,      0x0B},
  {CC1101_IOCFG0,      0x0C},
  {CC1101_FIFOTHR,     0x47},
  {CC1101_PKTCTRL0,    0x22},
  {CC1101_FSCTRL1,     0x06},
  {CC1101_FREQ2,       0x10},
  {CC1101_FREQ1,       0xb1},
  {CC1101_FREQ0,       0x3b},
  {CC1101_MDMCFG4,     0xF5},
  {CC1101_MDMCFG3,     0x83},
  {CC1101_MDMCFG2,     0x30},
  {CC1101_DEVIATN,     0x15},
  {CC1101_MCSM0,       0x18},
  {CC1101_FOCCFG,      0x16},
  {CC1101_WORCTRL,     0xFB},
  {CC1101_FREND0,      0x11},
  {CC1101_FSCAL3,      0xE9},
  {CC1101_FSCAL2,      0x2A},
  {CC1101_FSCAL1,      0x00},
  {CC1101_FSCAL0,      0x1F},
  {CC1101_TEST2,       0x81},
  {CC1101_TEST1,       0x35},
  {CC1101_TEST0,       0x09},
};

int main(void)
{
	DCO::init();
	ACLK::init();
	SMCLK::init();
	MCLK::init();
	WDT::init();
	WDT::enable_irq();

	PORT1::init();
	PORT2::init();
#ifndef __MSP430_HAS_USCI__
	TIMER::init();
#endif
	SPI::init();
	CC1101::reset();
	while (1) {
		CC1101::init(cc110x_default_init_values, ARRAY_COUNT(cc110x_default_init_values));
		CC1101::read_reg(CC1101_STATUS_REGISTER | CC1101_MARCSTATE);
		CC1101::read_reg(CC1101_CONFIG_REGISTER | CC1101_RXFIFO);
		CC1101::read_reg(CC1101_STATUS_REGISTER | CC1101_VERSION);
		TIMEOUT::set_and_wait(500);
		LED_RED::toggle();
	}
/*
	LED_RED::set_high();
	CC1101::strobe(CC1101_STX);
	while (1) {
		enter_idle();
	}
*/
}
