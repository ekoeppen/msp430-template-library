#define I2C_POLLING

#include <msp430.h>
#include <stdint.h>
#include <string.h>
#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <utils.h>
#include <usci_i2c.h>
#include <usci_uart.h>

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<12000000> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

typedef GPIO_MODULE_T<1, 6, 3> SCL;
typedef GPIO_MODULE_T<1, 7, 3> SDA;
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef GPIO_OUTPUT_T<1, 0, LOW> LED;
typedef GPIO_OUTPUT_T<2, 0, LOW> SDN;
typedef GPIO_INPUT_T<2, 2, RESISTOR_ENABLED, PULL_UP, INTERRUPT_ENABLED, TRIGGER_FALLING> IRQ;
typedef GPIO_PORT_T<1, LED, RX, TX, SCL, SDA> PORT1;
typedef GPIO_PORT_T<2, SDN, IRQ> PORT2;

typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
typedef USCI_I2C_T<USCI_B, 0, SMCLK> I2C;

#define MAX_PACKET_LEN 32
#define CRC16_POLY 0x8005
#define CRC_INIT 0xFFFF

#if 1
uint16_t crc_update(uint8_t crcData, uint16_t crcReg) {
	for (int i = 0; i < 8; i++) {
		if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
			crcReg = (crcReg << 1) ^ CRC16_POLY;
		else
			crcReg = (crcReg << 1);
		crcData <<= 1;
	}
	return crcReg;
}
#else
uint16_t crc_update(uint8_t data, uint16_t crc) {
	uint16_t crc_new = (uint8_t)(crc >> 8) | (crc << 8);
	crc_new ^= data;
	crc_new ^= (uint8_t)(crc_new & 0xff) >> 4;
	crc_new ^= crc_new << 12;
	crc_new ^= (crc_new & 0xff) << 5;
	return crc_new;
}
#endif

typedef TIMEOUT_T<WDT> TIMEOUT;
struct SI4012 {
	static void init(void) {
		SDN::set_low();
		while (IRQ::is_high());
	}

	static void get_int_status(uint8_t *data) {
		uint8_t r;
		r = 0x64;
		I2C::transfer(&r, 1, true);
		I2C::transfer(data, 2, false);
	}

	static uint8_t init_fifo(void) {
		uint8_t r;
		r = 0x65;
		I2C::transfer(&r, 1, true);
		I2C::transfer(&r, 1, false);
		return r;
	}

	static void get_rev(uint8_t *data) {
		uint8_t r;
		r = 0x10;
		I2C::transfer(&r, 1, true);
		I2C::transfer(data, 11, false);
	}

	static void get_state(uint8_t *data) {
		uint8_t r;
		r = 0x61;
		I2C::transfer(&r, 1, true);
		I2C::transfer(data, 6, false);
	}

	static uint8_t change_state(void) {
		uint8_t r;
		I2C::transfer((uint8_t *) "\x60\x00\x00", 3, true);
		I2C::transfer(&r, 1, false);
		return r;
	}

	static uint8_t set_fifo(uint8_t *data, uint8_t length) {
		uint8_t r;
		uint8_t buffer[MAX_PACKET_LEN];
		int n = 0;
		uint16_t crc = CRC_INIT;

		buffer[n++] = 0x66;
		for (int i = 0; i < 8; i++) buffer[n++] = 0xaa;
		buffer[n++] = 0xd3; buffer[n++] = 0x91;
		//buffer[n++] = 0xd3; buffer[n++] = 0x91;
		for (int i = 0; i < length; i++) {
			buffer[n++] = *data;
			crc = crc_update(*data, crc);
			data++;
		}
		buffer[n++] = crc >> 8;
		buffer[n++] = crc & 0xff;
		I2C::transfer(buffer, n, true);
		I2C::transfer(&r, 1, false);
		return r;
	}

	static uint8_t led_ctrl(bool on) {
		uint8_t buffer[2];
		uint8_t r;
		buffer[0] = 0x13; buffer[1] = on ? 1 : 0;
		I2C::transfer(buffer, 2, true);
		I2C::transfer(&r, 1, false);
		return r;
	}

	static void tx_start(uint8_t tx_length, uint8_t *data, uint8_t length) {
		uint8_t buffer[6];
		buffer[0] = 0x62; buffer[1] = 0; buffer[2] = tx_length; buffer[3] = 0; buffer[4] = 2; buffer[5] = 0;
		I2C::transfer(buffer, 6, true);
		I2C::transfer(data, length, false);
	}

	static uint8_t set_property(uint8_t *data, uint8_t length) {
		static uint8_t buffer[8];
		uint8_t r;
		buffer[0] = 0x11;
		for (int i = 0; i < length; i++) buffer[i + 1] = *data++;
		I2C::transfer(buffer, length + 1, true);
		I2C::transfer(&r, 1, false);
		return r;
	}
};

static uint8_t buffer[32];

static uint8_t pa_config[] = {0x60, 0x00, 0x01, 0x00, 0x19, 0x7d, 0xff};
static uint8_t tx_freq[] = {0x40, 0x19, 0xde, 0x7f, 0x60};
static uint8_t modulation_fskdev[] = {0x20, 0x01, 0x0b};
static uint8_t bitrate_config[] = {0x31, 0x01, 0x80, 0x04};
static uint8_t chip_config[] = {0x10, 0x00};
static uint8_t packet[] = {0x04, 0x12, 0x34, 0x56, 0x78};

int main(void)
{
	uint8_t r;
	bool on;
	DCO::init();
	ACLK::init();
	MCLK::init();
	SMCLK::init();
	WDT::init();
	WDT::enable_irq();
	PORT1::init();
	PORT2::init();
	UART::init();
	UART::puts("Si4012 example start\n");

	LED::set_high();
	SI4012::init();
	I2C::init();
	I2C::set_slave_addr(0b1110000);
	SI4012::get_int_status(buffer); hex_dump_bytes<UART>(buffer, 2, "Interrupt status");
	r = SI4012::change_state(); printf<UART>("State response: %02x\n", r);
	SI4012::get_rev(buffer); hex_dump_bytes<UART>(buffer, 11, "Revision");
	SI4012::get_state(buffer); hex_dump_bytes<UART>(buffer, 6, "State");
	SI4012::set_property((uint8_t *) "\x11\x03", 2);
	SI4012::set_property(pa_config, sizeof(pa_config));
	SI4012::set_property(tx_freq, sizeof(tx_freq));
	SI4012::set_property(modulation_fskdev, sizeof(modulation_fskdev));
	SI4012::set_property(bitrate_config, sizeof(bitrate_config));
	SI4012::set_property(chip_config, sizeof(chip_config));

	while (1) {
		LED::toggle();
		SI4012::led_ctrl(on);
		SI4012::init_fifo();
		SI4012::set_fifo(packet, sizeof(packet));
		SI4012::tx_start(sizeof(packet) + 8 + 2 * 2 + 2, buffer, 2); hex_dump_bytes<UART>(buffer, 2, "TX start");
		on = !on;
		TIMEOUT::set_and_wait(1000);
	}
}

void port2_irq(void) __attribute__((interrupt(PORT2_VECTOR)));
void port2_irq(void)
{
	if (PORT2::handle_irq()) {
		exit_idle();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
#ifndef I2C_POLLING
	if (I2C::handle_tx_irq()) exit_idle();
#endif
	if (UART::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (UART::handle_rx_irq()) exit_idle();
}
