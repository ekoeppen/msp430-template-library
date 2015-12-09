#pragma once

#include <msp430.h>

template<typename CLOCK, typename SCL, typename SDA>
struct SOFT_I2C_T {
	static uint8_t slave_addr;
	static bool started;
	static bool ack;

	static void init(void) {
		SCL::set_input();
		SDA::set_input();
		SCL::set_low();
		SDA::set_low();
	}

	static void enable(void) {
	}

	static void disable(void) {
	}

	static void set_slave_addr(const uint8_t addr) {
		slave_addr = addr;
	}

	static bool slave_present(const uint8_t slave_address) {
		bool r;
		return r;
	}


	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void write(const uint8_t *data, int length, bool restart = false) {
		start();
		tx_byte(slave_addr << 1);
		for (int i = 0; i < length; i++) {
			tx_byte(data[i]);
		}
		if (!restart) {
			stop();
		}
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void read(uint8_t *data, uint16_t length) {
		start();
		tx_byte((slave_addr << 1) | BIT0);
		for (int i = 0; i < length; i++) {
			data[i] = rx_byte(!(i == length - 1));
		}
		stop();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void write_reg(const uint8_t reg, const uint8_t value) {
		start();
		tx_byte(slave_addr << 1);
		tx_byte(reg);
		stop();
		start();
		tx_byte(slave_addr << 1);
		tx_byte(value);
		stop();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static uint8_t read_reg(const uint8_t reg) {
		uint8_t r;
		start();
		tx_byte(slave_addr << 1);
		tx_byte(reg);
		stop();
		start();
		tx_byte((slave_addr << 1) | BIT0);
		r = rx_byte(false);
		stop();
		return r;
	}

	static void read_reg(const uint8_t reg, uint8_t *data, uint16_t length) {
		start();
		tx_byte(slave_addr << 1);
		tx_byte(reg);
		stop();
		start();
		tx_byte((slave_addr << 1) | BIT0);
		for (int i = 0; i < length; i++) {
			data[i] = rx_byte(!(i == length - 1));
		}
		stop();
	}

	template<typename TIMEOUT = TIMEOUT_NEVER>
	static void transfer(const uint8_t *data, int length, bool direction, bool restart = false) {
		if (direction) write(data, length, restart);
		else read(const_cast<uint8_t *>(data), length);
	}

	template<typename PIN>
	static void clear(void) {
		PIN::set_output();
	}

	template<typename PIN>
	static bool read(void) {
		PIN::set_input();
		return PIN::get();
	}

	template<typename PIN>
	static void set(void) {
		PIN::set_input();
	}

	template<typename PIN>
	static void release(void) {
		PIN::set_input();
	}

	static void delay(void) {
		CLOCK::set_and_wait_us(5);
	}

	static void start(void) {
		if (started) {
			set<SDA>();
			delay();
			while (read<SCL>() == 0);
			delay();
		}

		clear<SDA>();
		delay();
		clear<SCL>();
		started = true;
	}

	static void stop(void) {
		clear<SDA>();
		delay();
		while (read<SCL>() == 0);
		delay();
		set<SDA>();
		delay();
		started = false;

	}

	static void write_bit(bool bit) {
		if (bit) {
			set<SDA>();
		} else {
			clear<SDA>();
		}
		delay();
		while (read<SCL>() == 0);
		clear<SCL>();
	}

	static bool read_bit(void) {
		bool bit;
		release<SDA>();
		delay();
		while (read<SCL>() == 0);
		bit = read<SDA>();
		delay();
		clear<SCL>();
		return bit;
	}

	static bool tx_byte(uint8_t byte) {
		for (int bit = 0; bit < 8; bit++) {
			write_bit((byte & 0x80) != 0);
			byte <<= 1;
		}
		ack = !read_bit();
		return ack;
	}

	static uint8_t rx_byte(bool ack) {
		uint8_t byte = 0;
		for (int bit = 0; bit < 8; bit++) {
			byte = (byte << 1) | read_bit();
		}
		write_bit(!ack);
		return byte;
	}

};

template<typename CLOCK, typename SCL, typename SDA>
uint8_t SOFT_I2C_T<CLOCK, SCL, SDA>::slave_addr = 0;

template<typename CLOCK, typename SCL, typename SDA>
bool SOFT_I2C_T<CLOCK, SCL, SDA>::started;

template<typename CLOCK, typename SCL, typename SDA>
bool SOFT_I2C_T<CLOCK, SCL, SDA>::ack;

