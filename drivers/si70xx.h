#pragma once

namespace SI70XX {

static constexpr uint8_t RESET = 0xfe;
static constexpr uint8_t RH_HOLD = 0xe5;
static constexpr uint8_t RH_NOHOLD = 0xf5;
static constexpr uint8_t TEMP_HOLD = 0xe3;
static constexpr uint8_t TEMP_NOHOLD = 0xf3;

template<typename I2C, typename TIMEOUT = TIMEOUT_NEVER>
struct T {
	static void init(void) {
	}

	static int16_t temperature(void) {
		uint8_t buffer[3];
		uint16_t temperature_raw;

		buffer[0] = TEMP_HOLD;
		I2C::set_slave_addr(0b1000000);
		I2C::write(buffer, 1);
		I2C::read(buffer, 3);
		temperature_raw = buffer[1] + (buffer[0] << 8);
		return (((uint32_t) 1757 * temperature_raw) >> 16) - 468;
	}

	static uint16_t humidity(void) {
		uint8_t buffer[3];
		uint16_t humidity_raw;
		uint16_t r;

		buffer[0] = RH_HOLD;
		I2C::set_slave_addr(0b1000000);
		I2C::write(buffer, 1);
		I2C::read(buffer, 3);
		humidity_raw = buffer[1] + (buffer[0] << 8);
		r = (((uint32_t) 1250 * humidity_raw) >> 16);
		if (r < 60) return 0;
		else if (r > 1060) return 1000;
		return r - 60;
	}
};

}
