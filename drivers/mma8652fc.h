#ifndef MMA8652FC_H
#define MMA8652FC_H

namespace MMA8652FC {

enum DATA_RATE {
	RATE_800HZ = 0,
	RATE_400HZ = 1,
	RATE_200HZ = 2,
	RATE_100HZ = 3,
	RATE_50HZ = 4,
	RATE_12HZ = 5,
	RATE_6HZ = 6,
	RATE_1HZ = 7
};

enum OVERSAMPLING_MODE {
	NORMAL = 0,
	LOW_NOISE_LOW_POWER = 1,
	HIGH_RESOLUTION = 2,
	LOW_POWER = 3
};

enum READ_MODE {
	NORMAL_READ,
	FAST_READ
};

enum RANGE {
	RANGE_2G,
	RANGE_4G,
	RANGE_8G
};

template<typename I2C,
	const READ_MODE MODE = NORMAL_READ,
	const RANGE SAMPLE_RANGE = RANGE_2G,
	const DATA_RATE RATE = RATE_800HZ,
	const OVERSAMPLING_MODE OVR = NORMAL>
struct T {
	static constexpr uint8_t address = 0x1d;
	static constexpr uint8_t ctrl_reg1 = RATE << 3 | (MODE == FAST_READ ? 2 : 0);
	static constexpr uint8_t ctrl_reg2 = OVR;

	static int16_t x;
	static int16_t y;
	static int16_t z;

	static void init(void) {
		uint8_t buffer[2];
		I2C::set_slave_addr(address);
		I2C::write_reg(0x2a, ctrl_reg1);
		I2C::write_reg(0x2b, ctrl_reg2);
	};

	static void sample(void) {
		static int8_t buffer[7];
		memset(buffer, 0, sizeof(buffer));
		I2C::set_slave_addr(address);
		do {
			I2C::read_reg(0x00, (uint8_t *) buffer, sizeof(buffer) - (MODE == FAST_READ ? 3 : 0));
		} while (!(buffer[0] & 0x03));
		if (MODE == NORMAL_READ) {
			x = (buffer[1] << 8) + (buffer[2] >> 4);
			y = (buffer[3] << 8) + (buffer[4] >> 4);
			z = (buffer[5] << 8) + (buffer[6] >> 4);
		} else {
			x = buffer[1];
			y = buffer[2];
			z = buffer[3];
		}
	}

	static void read_regs(void) {
		static uint8_t regs[40];
		memset(regs, 0, sizeof(regs));
		I2C::set_slave_addr(address);
		I2C::read_reg(0x09, regs, sizeof(regs));
	}

	static void wakeup(void) {
		I2C::write_reg(0x2a, ctrl_reg1 | 0x01);
	}

	static void sleep(void) {
		I2C::write_reg(0x2a, ctrl_reg1);
	}
};

template<typename I2C, const READ_MODE MODE, const RANGE VALUE_RANGE, const DATA_RATE RATE, const OVERSAMPLING_MODE OVR>
int16_t T<I2C, MODE, VALUE_RANGE, RATE, OVR>::x;

template<typename I2C, const READ_MODE MODE, const RANGE VALUE_RANGE, const DATA_RATE RATE, const OVERSAMPLING_MODE OVR>
int16_t T<I2C, MODE, VALUE_RANGE, RATE, OVR>::y;

template<typename I2C, const READ_MODE MODE, const RANGE VALUE_RANGE, const DATA_RATE RATE, const OVERSAMPLING_MODE OVR>
int16_t T<I2C, MODE, VALUE_RANGE, RATE, OVR>::z;

};

#endif
