#ifndef __SWAP_MOTE_H
#define __SWAP_MOTE_H

#define SWAP_CONFIG_DATA 1

enum PACKET_TYPE {
	STATUS = 0,
	QUERY = 1,
	COMMAND = 2
};

enum REGISTER_NUMBER {
	PRODUCT_CODE_REG = 0,
	HW_VERSION_REG = 1,
	FW_VERSION_REG = 2,
	SYSTEM_STATE_REG = 3,
	CHANNEL_REG = 4,
	SECURITY_REG = 5,
	PASSWORD_REG = 6,
	NONCE_REG = 7,
	NETWORK_ID_REG = 8,
	DEVICE_ADDR_REG = 9,
	TX_INTERVAL_REG = 10,
	UNUSED_REG = 255
};

enum SYSTEM_STATE {
	RESTART = 0,
	RXON = 1,
	RXOFF = 2,
	SYNC = 3,
	LOWBAT = 4
};

struct SWAP_PACKET {
	uint8_t dest;
	uint8_t src;
	uint8_t hop_secu;
	uint8_t nonce;
	uint8_t function;
	uint8_t reg_addr;
	uint8_t reg_id;
	uint8_t reg_value[16];
	uint8_t len;
};

struct CONFIGURATION_DATA {
	uint8_t tag;
	uint8_t length;
	uint8_t address[5];
	uint8_t channel;
	uint16_t tx_interval;

	void update(uint8_t ch, uint8_t addr, uint16_t tx_int) {
		tag = SWAP_CONFIG_DATA;
		length = sizeof(CONFIGURATION_DATA) - 2;
		address[1] = address[2] = address[3] = address[4] = 0xf0;
		address[0] = addr;
		channel = ch;
		tx_interval = tx_int;
	}
};

struct CONFIG_STORAGE_UNUSED {
	static void write(const void *data, unsigned size) { }
};

struct UNUSED_REGISTER {
	static bool write(SWAP_PACKET& packet) { return false; }
	static bool handle_command(SWAP_PACKET& packet) { return false; }
	static bool handle_query(SWAP_PACKET& packet) { return false; }
	static void update(void) { }
};

template<const uint32_t ID, const uint16_t VALUE>
struct REGISTER_T {
	static constexpr uint8_t reg_id = ID;
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = ID;
		packet.reg_value[0] = VALUE >> 8;
		packet.reg_value[1] = VALUE & 0xff;
		packet.len = 7 + 2;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) { return false; }
	static bool handle_query(SWAP_PACKET& packet) { return false; }
	static void update(void) { };
};

template<const uint32_t MANUFACTURER_ID, const uint32_t PRODUCT_ID>
struct PRODUCT_CODE_REGISTER_T {
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = PRODUCT_CODE_REG;
		packet.reg_value[0] = (MANUFACTURER_ID >> 24) & 0xff; packet.reg_value[1] = (MANUFACTURER_ID >> 16) & 0xff;
		packet.reg_value[2] = (MANUFACTURER_ID >>  8) & 0xff; packet.reg_value[3] =  MANUFACTURER_ID        & 0xff;
		packet.reg_value[4] = (PRODUCT_ID >> 24) & 0xff; packet.reg_value[5] = (PRODUCT_ID >> 16) & 0xff;
		packet.reg_value[6] = (PRODUCT_ID >>  8) & 0xff; packet.reg_value[7] =  PRODUCT_ID        & 0xff;
		packet.len = 7 + 8;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) { return false; }
	static bool handle_query(SWAP_PACKET& packet) { return true; }
	static void update(void) { };
};

template<uint8_t *STATE>
struct SYSTEM_STATE_REGISTER_T {
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = SYSTEM_STATE_REG;
		packet.reg_value[0] = *STATE;
		packet.len = 7 + 1;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) { return false; }
	static bool handle_query(SWAP_PACKET& packet) { return true; }
	static void update(void) { };
};

template<CONFIGURATION_DATA *config, typename CONFIG_STORAGE>
struct CHANNEL_REGISTER_T {
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = CHANNEL_REG;
		packet.reg_value[0] = config->channel;
		packet.len = 7 + 1;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) {
		CONFIGURATION_DATA c;
		c.update(packet.reg_value[0], config->address[0], config->tx_interval);
		CONFIG_STORAGE::write(&c, sizeof(c));
	}
	static bool handle_query(SWAP_PACKET& packet) {
		return true;
	}
	static void update(void) { };
};

template<CONFIGURATION_DATA *config, typename CONFIG_STORAGE>
struct DEVICE_ADDR_REGISTER_T {
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = DEVICE_ADDR_REG;
		packet.reg_value[0] = config->address[0];
		packet.len = 7 + 1;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) {
		CONFIGURATION_DATA c;
		c.update(config->channel, packet.reg_value[0], config->tx_interval);
		CONFIG_STORAGE::write(&c, sizeof(c));
	}
	static bool handle_query(SWAP_PACKET& packet) {
		return true;
	}
	static void update(void) { };
};

template<CONFIGURATION_DATA *config, typename CONFIG_STORAGE>
struct TX_INTERVAL_REGISTER_T {
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = TX_INTERVAL_REG;
		packet.reg_value[0] = config->tx_interval >> 8;
		packet.reg_value[1] = config->tx_interval & 0xff;
		packet.len = 7 + 2;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) {
		CONFIGURATION_DATA c;
		c.update(config->channel, config->address[0], (packet.reg_value[0] << 8) + packet.reg_value[1]);
		CONFIG_STORAGE::write(&c, sizeof(c));
	}
	static bool handle_query(SWAP_PACKET& packet) {
		return true;
	}
	static void update(void) { };
};

template<const uint32_t MANUFACTURER_ID,
	const uint32_t PRODUCT_ID,
	const uint32_t HARDWARE_VERSION,
	const uint32_t FIRMWARE_VERSION,
	typename RADIO,
	const uint8_t DEFAULT_CHANNEL,
	typename TIMEOUT,
	typename CONFIG_STORAGE = CONFIG_STORAGE_UNUSED,
	typename REGISTER1 = UNUSED_REGISTER,
	typename REGISTER2 = UNUSED_REGISTER,
	typename REGISTER3 = UNUSED_REGISTER,
	typename REGISTER4 = UNUSED_REGISTER,
	typename REGISTER5 = UNUSED_REGISTER,
	typename REGISTER6 = UNUSED_REGISTER,
	typename REGISTER7 = UNUSED_REGISTER,
	typename REGISTER8 = UNUSED_REGISTER>
struct SWAP_MOTE_T {
	static SWAP_PACKET rx_packet;
	static SWAP_PACKET tx_packet;
	static uint8_t state;
	static CONFIGURATION_DATA config;
	static const uint8_t BROADCAST_ADDR[5];

	typedef PRODUCT_CODE_REGISTER_T<MANUFACTURER_ID, PRODUCT_ID> PRODUCT_CODE_REGISTER;
	typedef SYSTEM_STATE_REGISTER_T<&state> SYSTEM_STATE_REGISTER;
	typedef CHANNEL_REGISTER_T<&config, CONFIG_STORAGE> CHANNEL_REGISTER;
	typedef DEVICE_ADDR_REGISTER_T<&config, CONFIG_STORAGE> DEVICE_ADDR_REGISTER;
	typedef TX_INTERVAL_REGISTER_T<&config, CONFIG_STORAGE> TX_INTERVAL_REGISTER;

	static bool address_match(void) {
		return rx_packet.reg_addr == 0 || rx_packet.reg_addr == config.address[0];
	};

	static void send_tx_packet(void) {
		tx_packet.src = config.address[0];
		tx_packet.dest = 0;
		tx_packet.function = STATUS;
		tx_packet.nonce = 0;
		tx_packet.hop_secu = 0;
		tx_packet.reg_addr = config.address[0];
		RADIO::tx_buffer(BROADCAST_ADDR, (uint8_t *) &tx_packet, tx_packet.len, false);
	}

	static void init(void) {
		RADIO::set_channel(config.channel);
		RADIO::set_rx_addr(config.address);
	}

	static bool handle_radio(uint16_t timeout) {
		uint8_t pipe;
		bool got_packet = false;

		if (RADIO::rx_buffer((uint8_t *) &rx_packet, sizeof(rx_packet) - sizeof(rx_packet.len), &pipe, timeout) > 0) {
			got_packet = true;
			if (address_match()) {
				switch (rx_packet.function) {
					case STATUS: handle_status(); break;
					case QUERY: handle_query(); break;
					case COMMAND: handle_command(); break;
				}
			}
		}
		return got_packet;
	}

	static void announce(void) {
		RADIO::start_tx();
		PRODUCT_CODE_REGISTER::write(tx_packet); send_tx_packet();
		state = SYNC;
		SYSTEM_STATE_REGISTER::write(tx_packet); send_tx_packet();
		RADIO::start_rx();
		handle_radio(10000);
		state = RXOFF;
		RADIO::start_tx();
		SYSTEM_STATE_REGISTER::write(tx_packet); send_tx_packet();
		RADIO::power_down();
	};

	static void handle_status(void) {
	};

	static void handle_query(void) {
		if (address_match()) {
			if (SYSTEM_STATE_REGISTER::handle_query(rx_packet) &&
					SYSTEM_STATE_REGISTER::write(tx_packet)) send_tx_packet();
			else if (CHANNEL_REGISTER::handle_query(rx_packet) &&
					CHANNEL_REGISTER::write(tx_packet)) send_tx_packet();
			else if (DEVICE_ADDR_REGISTER::handle_query(rx_packet) &&
					DEVICE_ADDR_REGISTER::write(tx_packet)) send_tx_packet();
			else if (TX_INTERVAL_REGISTER::handle_query(rx_packet) &&
					TX_INTERVAL_REGISTER::write(tx_packet)) send_tx_packet();
			else if (REGISTER1::handle_query(rx_packet) && REGISTER1::write(tx_packet)) send_tx_packet();
			else if (REGISTER2::handle_query(rx_packet) && REGISTER2::write(tx_packet)) send_tx_packet();
			else if (REGISTER3::handle_query(rx_packet) && REGISTER3::write(tx_packet)) send_tx_packet();
			else if (REGISTER4::handle_query(rx_packet) && REGISTER4::write(tx_packet)) send_tx_packet();
			else if (REGISTER5::handle_query(rx_packet) && REGISTER5::write(tx_packet)) send_tx_packet();
			else if (REGISTER6::handle_query(rx_packet) && REGISTER6::write(tx_packet)) send_tx_packet();
			else if (REGISTER7::handle_query(rx_packet) && REGISTER7::write(tx_packet)) send_tx_packet();
			else if (REGISTER8::handle_query(rx_packet) && REGISTER8::write(tx_packet)) send_tx_packet();
		}
	};

	static bool handle_command(void) {
		if (address_match()) {
			if (SYSTEM_STATE_REGISTER::handle_command(rx_packet) &&
					SYSTEM_STATE_REGISTER::write(tx_packet)) send_tx_packet();
			else if (CHANNEL_REGISTER::handle_command(rx_packet) &&
					CHANNEL_REGISTER::write(tx_packet)) send_tx_packet();
			else if (DEVICE_ADDR_REGISTER::handle_command(rx_packet) &&
					DEVICE_ADDR_REGISTER::write(tx_packet)) send_tx_packet();
			else if (TX_INTERVAL_REGISTER::handle_command(rx_packet) &&
					TX_INTERVAL_REGISTER::write(tx_packet)) send_tx_packet();
			else if (REGISTER1::handle_command(rx_packet) && REGISTER1::write(tx_packet)) send_tx_packet();
			else if (REGISTER2::handle_command(rx_packet) && REGISTER2::write(tx_packet)) send_tx_packet();
			else if (REGISTER3::handle_command(rx_packet) && REGISTER3::write(tx_packet)) send_tx_packet();
			else if (REGISTER4::handle_command(rx_packet) && REGISTER4::write(tx_packet)) send_tx_packet();
			else if (REGISTER5::handle_command(rx_packet) && REGISTER5::write(tx_packet)) send_tx_packet();
			else if (REGISTER6::handle_command(rx_packet) && REGISTER6::write(tx_packet)) send_tx_packet();
			else if (REGISTER7::handle_command(rx_packet) && REGISTER7::write(tx_packet)) send_tx_packet();
			else if (REGISTER8::handle_command(rx_packet) && REGISTER8::write(tx_packet)) send_tx_packet();
		}
	};

	static void update_registers(void) {
		REGISTER1::update();
		REGISTER2::update();
		REGISTER3::update();
		REGISTER4::update();
		REGISTER5::update();
		REGISTER6::update();
		REGISTER7::update();
		REGISTER8::update();
	};

	static void transmit_data(void) {
		RADIO::start_tx();
		if (REGISTER1::write(tx_packet)) send_tx_packet();
		if (REGISTER2::write(tx_packet)) send_tx_packet();
		if (REGISTER3::write(tx_packet)) send_tx_packet();
		if (REGISTER4::write(tx_packet)) send_tx_packet();
		if (REGISTER5::write(tx_packet)) send_tx_packet();
		if (REGISTER6::write(tx_packet)) send_tx_packet();
		if (REGISTER7::write(tx_packet)) send_tx_packet();
		if (REGISTER8::write(tx_packet)) send_tx_packet();
		RADIO::power_down();
	};

	static void sleep(void) {
		TIMEOUT::set_and_wait((uint32_t) config.tx_interval * (uint32_t) 1000);
	};

	static void run(void) {
		announce();
		while (1) {
			update_registers();
			transmit_data();
			sleep();
		}
	}
};

template<const uint32_t MANUFACTURER_ID, const uint32_t PRODUCT_ID, const uint32_t HARDWARE_VERSION, const uint32_t FIRMWARE_VERSION,
	typename RADIO, const uint8_t DEFAULT_CHANNEL, typename TIMEOUT,
	typename CONFIG_STORAGE,
	typename REGISTER1, typename REGISTER2, typename REGISTER3, typename REGISTER4,
	typename REGISTER5, typename REGISTER6, typename REGISTER7, typename REGISTER8>
SWAP_PACKET SWAP_MOTE_T<MANUFACTURER_ID, PRODUCT_ID, HARDWARE_VERSION, FIRMWARE_VERSION, RADIO, DEFAULT_CHANNEL, TIMEOUT, CONFIG_STORAGE,
	    REGISTER1, REGISTER2, REGISTER3, REGISTER4, REGISTER5, REGISTER6, REGISTER7, REGISTER8>::tx_packet;

template<const uint32_t MANUFACTURER_ID, const uint32_t PRODUCT_ID, const uint32_t HARDWARE_VERSION, const uint32_t FIRMWARE_VERSION,
	typename RADIO, const uint8_t DEFAULT_CHANNEL, typename TIMEOUT,
	typename CONFIG_STORAGE,
	typename REGISTER1, typename REGISTER2, typename REGISTER3, typename REGISTER4,
	typename REGISTER5, typename REGISTER6, typename REGISTER7, typename REGISTER8>
SWAP_PACKET SWAP_MOTE_T<MANUFACTURER_ID, PRODUCT_ID, HARDWARE_VERSION, FIRMWARE_VERSION, RADIO, DEFAULT_CHANNEL, TIMEOUT, CONFIG_STORAGE,
	    REGISTER1, REGISTER2, REGISTER3, REGISTER4, REGISTER5, REGISTER6, REGISTER7, REGISTER8>::rx_packet;

template<const uint32_t MANUFACTURER_ID, const uint32_t PRODUCT_ID, const uint32_t HARDWARE_VERSION, const uint32_t FIRMWARE_VERSION,
	typename RADIO, const uint8_t DEFAULT_CHANNEL, typename TIMEOUT,
	typename CONFIG_STORAGE,
	typename REGISTER1, typename REGISTER2, typename REGISTER3, typename REGISTER4,
	typename REGISTER5, typename REGISTER6, typename REGISTER7, typename REGISTER8>
CONFIGURATION_DATA SWAP_MOTE_T<MANUFACTURER_ID, PRODUCT_ID, HARDWARE_VERSION, FIRMWARE_VERSION, RADIO, DEFAULT_CHANNEL, TIMEOUT, CONFIG_STORAGE,
	    REGISTER1, REGISTER2, REGISTER3, REGISTER4,
	    REGISTER5, REGISTER6, REGISTER7, REGISTER8>::config __attribute__((section(".infod"))) = {
	SWAP_CONFIG_DATA,
	sizeof(CONFIGURATION_DATA) - 2,
	0xff, 0xf0, 0xf0, 0xf0, 0xf0,
	DEFAULT_CHANNEL,
	15
};

template<const uint32_t MANUFACTURER_ID, const uint32_t PRODUCT_ID, const uint32_t HARDWARE_VERSION, const uint32_t FIRMWARE_VERSION,
	typename RADIO, const uint8_t DEFAULT_CHANNEL, typename TIMEOUT,
	typename CONFIG_STORAGE,
	typename REGISTER1, typename REGISTER2, typename REGISTER3, typename REGISTER4,
	typename REGISTER5, typename REGISTER6, typename REGISTER7, typename REGISTER8>
const uint8_t SWAP_MOTE_T<MANUFACTURER_ID, PRODUCT_ID, HARDWARE_VERSION, FIRMWARE_VERSION, RADIO, DEFAULT_CHANNEL, TIMEOUT, CONFIG_STORAGE,
	    REGISTER1, REGISTER2, REGISTER3, REGISTER4,
	    REGISTER5, REGISTER6, REGISTER7, REGISTER8>::BROADCAST_ADDR[5] = {0x00, 0xf0, 0xf0, 0xf0, 0xf0};

template<const uint32_t MANUFACTURER_ID, const uint32_t PRODUCT_ID, const uint32_t HARDWARE_VERSION, const uint32_t FIRMWARE_VERSION,
	typename RADIO, const uint8_t DEFAULT_CHANNEL, typename TIMEOUT,
	typename CONFIG_STORAGE,
	typename REGISTER1, typename REGISTER2, typename REGISTER3, typename REGISTER4,
	typename REGISTER5, typename REGISTER6, typename REGISTER7, typename REGISTER8>
uint8_t SWAP_MOTE_T<MANUFACTURER_ID, PRODUCT_ID, HARDWARE_VERSION, FIRMWARE_VERSION, RADIO, DEFAULT_CHANNEL, TIMEOUT, CONFIG_STORAGE,
	    REGISTER1, REGISTER2, REGISTER3, REGISTER4, REGISTER5, REGISTER6, REGISTER7, REGISTER8>::state;

#endif
