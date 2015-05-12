#ifndef __BMP180_H
#define __BMP180_h

namespace BMP180 {

enum PRESSURE_ACCURACY {
	ULTRA_LOW_POWER,
	STANDARD,
	HIGH_RESOLUTION,
	ULTRA_HIGH_RESOLUTION
};

enum REGISTERS
{
	CAL_AC1            = 0xaa,
	CAL_AC2            = 0xac,
	CAL_AC3            = 0xae,
	CAL_AC4            = 0xb0,
	CAL_AC5            = 0xb2,
	CAL_AC6            = 0xb4,
	CAL_B1             = 0xb6,
	CAL_B2             = 0xb8,
	CAL_MB             = 0xba,
	CAL_MC             = 0xbc,
	CAL_MD             = 0xbe,
	CHIPID             = 0xd0,
	VERSION            = 0xd1,
	SOFTRESET          = 0xe0,
	CONTROL            = 0xf4,
	TEMPDATA           = 0xf6,
	PRESSUREDATA       = 0xf6,
	READTEMPCMD        = 0x2e,
	READPRESSURECMD    = 0x34
};

template<typename I2C,
	typename TIMEOUT>
struct T {
	static void init(void) {
	}

	static void disable(void) {
	}

	static int16_t temperature(void) {
		uint8_t r[2];
		I2C::write_reg(CONTROL, READTEMPCMD);
		TIMEOUT::set_and_wait(5);
		I2C::read_reg(TEMPDATA, r, sizeof(r));
		return r[1] + (r[0] << 8);
	}

	static uint16_t pressure(PRESSURE_ACCURACY accuracy) {
	}
};

}

#endif
