#ifndef __NRF24_H
#define __NRF24_H

#include <stdint.h>
#include <utils.h>

#define RF24_ADDR_WIDTH     5
#define TX_PLOAD_WIDTH     32
#define RX_PLOAD_WIDTH     32

/* Register Map */
#define RF24_CONFIG      0x00
#define RF24_EN_AA       0x01
#define RF24_EN_RXADDR   0x02
#define RF24_SETUP_AW    0x03
#define RF24_SETUP_RETR  0x04
#define RF24_RF_CH       0x05
#define RF24_RF_SETUP    0x06
#define RF24_STATUS      0x07
#define RF24_OBSERVE_TX  0x08
#define RF24_CD          0x09
#define RF24_RPD         0x09
#define RF24_RX_ADDR_P0  0x0A
#define RF24_RX_ADDR_P1  0x0B
#define RF24_RX_ADDR_P2  0x0C
#define RF24_RX_ADDR_P3  0x0D
#define RF24_RX_ADDR_P4  0x0E
#define RF24_RX_ADDR_P5  0x0F
#define RF24_TX_ADDR     0x10
#define RF24_RX_PW_P0    0x11
#define RF24_RX_PW_P1    0x12
#define RF24_RX_PW_P2    0x13
#define RF24_RX_PW_P3    0x14
#define RF24_RX_PW_P4    0x15
#define RF24_RX_PW_P5    0x16
#define RF24_FIFO_STATUS 0x17
#define RF24_DYNPD       0x1C
#define RF24_FEATURE     0x1D

/* Register Bits */
#define RF24_MASK_RX_DR  BIT6
#define RF24_MASK_TX_DS  BIT5
#define RF24_MASK_MAX_RT BIT4
#define RF24_IRQ_MASK    (BIT4 + BIT5 + BIT6)
#define RF24_EN_CRC      BIT3
#define RF24_CRCO        BIT2
#define RF24_PWR_UP      BIT1
#define RF24_PRIM_RX     BIT0
#define RF24_ENAA_P5     BIT5
#define RF24_ENAA_P4     BIT4
#define RF24_ENAA_P3     BIT3
#define RF24_ENAA_P2     BIT2
#define RF24_ENAA_P1     BIT1
#define RF24_ENAA_P0     BIT0
#define RF24_DPL_P5      BIT5
#define RF24_DPL_P4      BIT4
#define RF24_DPL_P3      BIT3
#define RF24_DPL_P2      BIT2
#define RF24_DPL_P1      BIT1
#define RF24_DPL_P0      BIT0
#define RF24_ERX_P5      BIT5
#define RF24_ERX_P4      BIT4
#define RF24_ERX_P3      BIT3
#define RF24_ERX_P2      BIT2
#define RF24_ERX_P1      BIT1
#define RF24_ERX_P0      BIT0
#define RF24_AW          BIT0
#define RF24_ARD         BIT4
#define RF24_ARC         BIT0
#define RF24_PLL_LOCK    BIT4
#define RF24_CONT_WAVE   BIT7
#define RF24_RF_DR       BIT3
#define RF24_RF_DR_LOW   BIT5
#define RF24_RF_DR_HIGH  BIT3
#define RF24_RF_PWR      BIT1
#define RF24_LNA_HCURR   BIT0
#define RF24_RX_DR       BIT6
#define RF24_TX_DS       BIT5
#define RF24_MAX_RT      BIT4
#define RF24_RX_P_NO     BIT1
#define RF24_TX_FULL     BIT0
#define RF24_PLOS_CNT    BIT4
#define RF24_ARC_CNT     BIT0
#define RF24_TX_REUSE    BIT6
#define RF24_FIFO_FULL   BIT5
#define RF24_TX_EMPTY    BIT4
#define RF24_RX_FULL     BIT1
#define RF24_RX_EMPTY    BIT0
#define RF24_EN_DPL      BIT2
#define RF24_EN_ACK_PAY  BIT1
#define RF24_EN_DYN_ACK  BIT0

/* Instructions */
#define RF24_R_REGISTER    0x00
#define RF24_W_REGISTER    0x20
#define RF24_REGISTER_MASK 0x1F
#define RF24_R_RX_PAYLOAD  0x61
#define RF24_W_TX_PAYLOAD  0xA0
#define RF24_FLUSH_TX      0xE1
#define RF24_FLUSH_RX      0xE2
#define RF24_REUSE_TX_PL   0xE3
#define RF24_R_RX_PL_WID   0x60
#define RF24_W_ACK_PAYLOAD 0xA8
#define RF24_W_TX_PAYLOAD_NOACK 0xB0
#define RF24_NOP           0xFF

static constexpr uint8_t nrf24_init_values[][2] = {
	{RF24_W_REGISTER + RF24_CONFIG, 0x00},
	{RF24_W_REGISTER + RF24_EN_AA, RF24_ENAA_P0 + RF24_ENAA_P1 + RF24_ENAA_P2 + RF24_ENAA_P3 + RF24_ENAA_P4},
	{RF24_W_REGISTER + RF24_SETUP_AW, 0x03},
	{RF24_W_REGISTER + RF24_SETUP_RETR, 0x3},
        {RF24_W_REGISTER + RF24_STATUS, RF24_IRQ_MASK},
	{RF24_W_REGISTER + RF24_RX_PW_P0, 0},
	{RF24_W_REGISTER + RF24_RX_PW_P1, 0},
	{RF24_W_REGISTER + RF24_RX_PW_P2, 0},
	{RF24_W_REGISTER + RF24_RX_PW_P3, 0},
	{RF24_W_REGISTER + RF24_RX_PW_P4, 0},
	{RF24_W_REGISTER + RF24_RF_SETUP, RF24_RF_DR_LOW},
	{RF24_W_REGISTER + RF24_DYNPD, RF24_DPL_P0 + RF24_DPL_P1 + RF24_DPL_P2 + RF24_DPL_P3 + RF24_DPL_P4},
	{RF24_W_REGISTER + RF24_FEATURE, RF24_EN_DPL + RF24_EN_ACK_PAY + RF24_EN_DYN_ACK},
	{RF24_W_REGISTER + RF24_CONFIG, RF24_EN_CRC + RF24_PWR_UP}
};

template<typename SPI,
	typename SCK,
	typename CSN,
	typename CE,
	typename IRQ,
	const uint8_t CHANNEL,
	const uint8_t *RX_ADDR,
	const uint8_t RX_ADDR2 = 0,
	const uint8_t RX_ADDR3 = 0,
	const uint8_t RX_ADDR4 = 0>
struct NRF24_t {
	static void init(void) {
		int i;
		uint8_t pipe_enable = RF24_ERX_P0 + RF24_ERX_P1;

		for (i = 0; i < ARRAY_COUNT(nrf24_init_values); i++) {
			rw_reg(nrf24_init_values[i][0], nrf24_init_values[i][1]);
		}
		write_reg(RF24_W_REGISTER + RF24_RX_ADDR_P1, RX_ADDR, RF24_ADDR_WIDTH);
		if (RX_ADDR2 >= 0) {
			pipe_enable += RF24_ERX_P2;
			rw_reg(RF24_W_REGISTER + RF24_RX_ADDR_P2, RX_ADDR2);
		}
		if (RX_ADDR3 >= 0) {
			pipe_enable += RF24_ERX_P2;
			rw_reg(RF24_W_REGISTER + RF24_RX_ADDR_P3, RX_ADDR3);
		}
		if (RX_ADDR4 >= 0) {
			pipe_enable += RF24_ERX_P2;
			rw_reg(RF24_W_REGISTER + RF24_RX_ADDR_P4, RX_ADDR4);
		}
		rw_reg(RF24_W_REGISTER + RF24_RF_CH, CHANNEL);
		rw_reg(RF24_W_REGISTER + RF24_EN_RXADDR, pipe_enable);
	}

	static void tx_buffer(const uint8_t tx_addr[5], const uint8_t *data, const uint8_t len, bool auto_ack) {
		CE::set_low();
		write_reg(RF24_W_REGISTER + RF24_TX_ADDR, tx_addr, RF24_ADDR_WIDTH);
		write_reg(RF24_W_REGISTER + RF24_RX_ADDR_P0, tx_addr, RF24_ADDR_WIDTH);
		write_reg(RF24_FLUSH_TX, 0, 0);
		write_reg(auto_ack ? RF24_W_TX_PAYLOAD : RF24_W_TX_PAYLOAD_NOACK, data, len);
		CE::set_high();
		__delay_cycles(10000);
		CE::set_low();
		while (IRQ::is_high());
		rw_reg(RF24_W_REGISTER + RF24_STATUS, RF24_TX_DS | RF24_MAX_RT);
	}

	static int rx_buffer(uint8_t *data, int max_len, uint8_t *pipe, unsigned int timeout) {
		int n = 0;

		/*
		if (timeout) {
			timeout = timeout / 42 + systick;
		}
		*/
		rw_reg(RF24_W_REGISTER + RF24_CONFIG, RF24_EN_CRC + RF24_PWR_UP + RF24_PRIM_RX);
		__delay_cycles(10000);
		CE::set_high();
		do {
			if (!(rw_reg(RF24_R_REGISTER + RF24_FIFO_STATUS, RF24_NOP) & RF24_RX_EMPTY)) {
				n = rw_reg(RF24_R_RX_PL_WID, RF24_NOP);
				rw_reg(RF24_W_REGISTER + RF24_STATUS, RF24_RX_DR);
			} else {
				while (IRQ::is_high()/* && (timeout ? systick < timeout : 1)*/);
			}
		} while (n == 0 /*&& (timeout ? systick < timeout : 1)*/);
		if (n > 0) {
			if (n > max_len) n = max_len;
			if (pipe != 0) {
				*pipe = (rw_reg(RF24_W_REGISTER + RF24_STATUS, RF24_NOP) >> 1) & 0x03;
			}
			read_reg(RF24_R_RX_PAYLOAD, data, n);
		}
		CE::set_low();
		rw_reg(RF24_W_REGISTER + RF24_CONFIG, RF24_EN_CRC + RF24_PWR_UP);
		return n;
	}

	static void start_tx(void) {
	}

	static void start_rx(void) {
	}

	static uint8_t rw_reg(uint8_t reg, uint8_t value)
	{
		uint8_t r;

		CSN::set_low();
		SPI::transfer(reg);
		r = SPI::transfer(value);
		CSN::set_high();
		return r;
	}

	static void spi_transfer_buffer(uint8_t reg, int dir, uint8_t *data, int len)
	{
		CSN::set_low();
		SPI::transfer(reg);
		while (len-- > 0) {
			if (dir) {
				SPI::transfer(*data);
			} else {
				*data = SPI::transfer(RF24_NOP);
			}
			data++;
		}
		CSN::set_high();
	}

	static inline void read_reg(uint8_t reg, uint8_t *data, int len)
	{
		spi_transfer_buffer(reg, 0, data, len);
	}

	static inline void write_reg(uint8_t reg, const uint8_t *data, int len)
	{
		spi_transfer_buffer(reg, 1, (uint8_t *) data, len);
	}

	static void read_regs(uint8_t *p)
	{
		int i;

		for (i = RF24_CONFIG; i <= RF24_RPD; i++) {
			*p++ = rw_reg(i, RF24_NOP);
		}
		read_reg(RF24_RX_ADDR_P0, p, RF24_ADDR_WIDTH); p += RF24_ADDR_WIDTH;
		read_reg(RF24_RX_ADDR_P1, p, RF24_ADDR_WIDTH); p += RF24_ADDR_WIDTH;
		for (i = RF24_RX_ADDR_P2; i <= RF24_RX_ADDR_P5; i++) {
			*p++ = rw_reg(i, RF24_NOP);
		}
		read_reg(RF24_TX_ADDR, p, RF24_ADDR_WIDTH); p += RF24_ADDR_WIDTH;
		for (i = RF24_RX_PW_P0; i <= RF24_FEATURE; i++) {
			*p++ = rw_reg(i, RF24_NOP);
		}
	}
};

#endif
