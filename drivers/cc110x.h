#ifndef __CC110X_H
#define __CC110X_H

#include <utils.h>

/**
 * Type of transfers
 */
#define WRITE_BURST              0x40
#define READ_SINGLE              0x80
#define READ_BURST               0xC0

/**
 * Type of register
 */
#define CC1101_CONFIG_REGISTER   READ_SINGLE
#define CC1101_STATUS_REGISTER   READ_BURST

/**
 * PATABLE & FIFO's
 */
#define CC1101_PATABLE           0x3E        // PATABLE address
#define CC1101_TXFIFO            0x3F        // TX FIFO address
#define CC1101_RXFIFO            0x3F        // RX FIFO address

/**
 * Command strobes
 */
#define CC1101_SRES              0x30        // Reset CC1101 chip
#define CC1101_SFSTXON           0x31        // Enable and calibrate frequency synthesizer (if MCSM0.FS_AUTOCAL=1). If in RX (with CCA):
                                             // Go to a wait state where only the synthesizer is running (for quick RX / TX turnaround).
#define CC1101_SXOFF             0x32        // Turn off crystal oscillator
#define CC1101_SCAL              0x33        // Calibrate frequency synthesizer and turn it off. SCAL can be strobed from IDLE mode without
                                             // setting manual calibration mode (MCSM0.FS_AUTOCAL=0)
#define CC1101_SRX               0x34        // Enable RX. Perform calibration first if coming from IDLE and MCSM0.FS_AUTOCAL=1
#define CC1101_STX               0x35        // In IDLE state: Enable TX. Perform calibration first if MCSM0.FS_AUTOCAL=1.
                                             // If in RX state and CCA is enabled: Only go to TX if channel is clear
#define CC1101_SIDLE             0x36        // Exit RX / TX, turn off frequency synthesizer and exit Wake-On-Radio mode if applicable
#define CC1101_SWOR              0x38        // Start automatic RX polling sequence (Wake-on-Radio) as described in Section 19.5 if
                                             // WORCTRL.RC_PD=0
#define CC1101_SPWD              0x39        // Enter power down mode when CSn goes high
#define CC1101_SFRX              0x3A        // Flush the RX FIFO buffer. Only issue SFRX in IDLE or RXFIFO_OVERFLOW states
#define CC1101_SFTX              0x3B        // Flush the TX FIFO buffer. Only issue SFTX in IDLE or TXFIFO_UNDERFLOW states
#define CC1101_SWORRST           0x3C        // Reset real time clock to Event1 value
#define CC1101_SNOP              0x3D        // No operation. May be used to get access to the chip status uint8_t

/**
 * CC1101 configuration registers
 */
#define CC1101_IOCFG2            0x00        // GDO2 Output Pin Configuration
#define CC1101_IOCFG1            0x01        // GDO1 Output Pin Configuration
#define CC1101_IOCFG0            0x02        // GDO0 Output Pin Configuration
#define CC1101_FIFOTHR           0x03        // RX FIFO and TX FIFO Thresholds
#define CC1101_SYNC1             0x04        // Sync Word, High Byte
#define CC1101_SYNC0             0x05        // Sync Word, Low Byte
#define CC1101_PKTLEN            0x06        // Packet Length
#define CC1101_PKTCTRL1          0x07        // Packet Automation Control
#define CC1101_PKTCTRL0          0x08        // Packet Automation Control
#define CC1101_ADDR              0x09        // Device Address
#define CC1101_CHANNR            0x0A        // Channel Number
#define CC1101_FSCTRL1           0x0B        // Frequency Synthesizer Control
#define CC1101_FSCTRL0           0x0C        // Frequency Synthesizer Control
#define CC1101_FREQ2             0x0D        // Frequency Control Word, High Byte
#define CC1101_FREQ1             0x0E        // Frequency Control Word, Middle Byte
#define CC1101_FREQ0             0x0F        // Frequency Control Word, Low Byte
#define CC1101_MDMCFG4           0x10        // Modem Configuration
#define CC1101_MDMCFG3           0x11        // Modem Configuration
#define CC1101_MDMCFG2           0x12        // Modem Configuration
#define CC1101_MDMCFG1           0x13        // Modem Configuration
#define CC1101_MDMCFG0           0x14        // Modem Configuration
#define CC1101_DEVIATN           0x15        // Modem Deviation Setting
#define CC1101_MCSM2             0x16        // Main Radio Control State Machine Configuration
#define CC1101_MCSM1             0x17        // Main Radio Control State Machine Configuration
#define CC1101_MCSM0             0x18        // Main Radio Control State Machine Configuration
#define CC1101_FOCCFG            0x19        // Frequency Offset Compensation Configuration
#define CC1101_BSCFG             0x1A        // Bit Synchronization Configuration
#define CC1101_AGCCTRL2          0x1B        // AGC Control
#define CC1101_AGCCTRL1          0x1C        // AGC Control
#define CC1101_AGCCTRL0          0x1D        // AGC Control
#define CC1101_WOREVT1           0x1E        // High Byte Event0 Timeout
#define CC1101_WOREVT0           0x1F        // Low Byte Event0 Timeout
#define CC1101_WORCTRL           0x20        // Wake On Radio Control
#define CC1101_FREND1            0x21        // Front End RX Configuration
#define CC1101_FREND0            0x22        // Front End TX Configuration
#define CC1101_FSCAL3            0x23        // Frequency Synthesizer Calibration
#define CC1101_FSCAL2            0x24        // Frequency Synthesizer Calibration
#define CC1101_FSCAL1            0x25        // Frequency Synthesizer Calibration
#define CC1101_FSCAL0            0x26        // Frequency Synthesizer Calibration
#define CC1101_RCCTRL1           0x27        // RC Oscillator Configuration
#define CC1101_RCCTRL0           0x28        // RC Oscillator Configuration
#define CC1101_FSTEST            0x29        // Frequency Synthesizer Calibration Control
#define CC1101_PTEST             0x2A        // Production Test
#define CC1101_AGCTEST           0x2B        // AGC Test
#define CC1101_TEST2             0x2C        // Various Test Settings
#define CC1101_TEST1             0x2D        // Various Test Settings
#define CC1101_TEST0             0x2E        // Various Test Settings

/**
 * Status registers
 */
#define CC1101_PARTNUM           0x30        // Chip ID
#define CC1101_VERSION           0x31        // Chip ID
#define CC1101_FREQEST           0x32        // Frequency Offset Estimate from Demodulator
#define CC1101_LQI               0x33        // Demodulator Estimate for Link Quality
#define CC1101_RSSI              0x34        // Received Signal Strength Indication
#define CC1101_MARCSTATE         0x35        // Main Radio Control State Machine State
#define CC1101_WORTIME1          0x36        // High Byte of WOR Time
#define CC1101_WORTIME0          0x37        // Low Byte of WOR Time
#define CC1101_PKTSTATUS         0x38        // Current GDOx Status and Packet Status
#define CC1101_VCO_VC_DAC        0x39        // Current Setting from PLL Calibration Module
#define CC1101_TXBYTES           0x3A        // Underflow and Number of Bytes
#define CC1101_RXBYTES           0x3B        // Overflow and Number of Bytes
#define CC1101_RCCTRL1_STATUS    0x3C        // Last RC Oscillator Calibration Result
#define CC1101_RCCTRL0_STATUS    0x3D        // Last RC Oscillator Calibration Result

static constexpr uint8_t cc110x_init_values[][2] = {
#if 0
	{CC1101_IOCFG0, 0x06},
	{CC1101_PKTCTRL1, 0x04},
	{CC1101_PKTCTRL0, 0x41},
	{CC1101_MCSM0, 0x38},
	{CC1101_FREQ2, 0x10},
	{CC1101_FREQ1, 0xB1},
	{CC1101_FREQ0, 0x3B},
#endif
#if 1
  {CC1101_IOCFG0,      0x06},
  {CC1101_FIFOTHR,     0x47},
  {CC1101_PKTCTRL0,    0x05},
  {CC1101_FSCTRL1,     0x06},
  {CC1101_FREQ2,       0x10},
  {CC1101_FREQ1,       0xB1},
  {CC1101_FREQ0,       0x3B},
  {CC1101_MDMCFG4,     0xFA},
  {CC1101_MDMCFG3,     0x83},
  {CC1101_MDMCFG2,     0x0B},
  {CC1101_DEVIATN,     0x15},
  {CC1101_MCSM0,       0x18},
  {CC1101_FOCCFG,      0x16},
  {CC1101_WORCTRL,     0xFB},
  {CC1101_FSCAL3,      0xE9},
  {CC1101_FSCAL2,      0x2A},
  {CC1101_FSCAL1,      0x00},
  {CC1101_FSCAL0,      0x1F},
  {CC1101_TEST2,       0x81},
  {CC1101_TEST1,       0x35},
  {CC1101_TEST0,       0x09},
#endif
#if 0
	{0x00,   0x2E},               // GDO2 output pin configuration.
	{0x01,   0x2E},               // GDO1 output pin configuration. (default)
	{0x02,   0x06},               // GDO0 output pin configuration.
	{0x03,   0x07},               // RXFIFO and TXFIFO thresholds. (default)
	{0x04,   0xD3},               // Sync word, high byte (default)
	{0x05,   0x91},               // Sync word, low byte (default)
	{0x06,   0xFF},               // Packet length. (default)
	{0x07,   0x0C},               // Packet automation control. CRC autoflush, append status, no address check
	{0x08,   0x05},               // Packet automation control. No whitening, CRC enabled, variable packet length
	{0x09,   0x00},               // Device address. (default)
	{0x0a,   0x00},               // Channel number. (default)
	{0x0b,   0x0A},               // Frequency synthesizer control. IF = 253kHz
	{0x0c,   0x00},               // Frequency synthesizer control. (default)
	{0x0d,   0x10},               // Frequency control word, high byte.
	{0x0e,   0xA7},               // Frequency control word, middle byte.
	{0x0f,   0x62},               // Frequency control word, low byte.
#ifdef FRQ_900
	{0x0d,   0x22},               // Frequency control word, high byte.
	{0x0e,   0xB6},               // Frequency control word, middle byte.
	{0x0f,   0x27},               // Frequency control word, low byte.
#endif
	{0x10,   0x35},               // Modem configuration. 464kHz channel bandwidth
	{0x11,   0x83},               // Modem configuration. 1.2kbps data rate
	{0x12,   0x03},               // Modem configuration. 2-FSK, no Manchester, 30/32 sync words needed, no carrier sense
	{0x13,   0x21},               // Modem configuration. 4 byte preamble
	{0x14,   0xEE},               // Modem configuration. 97kHz channel spacing
	{0x15,   0x65},               // Modem deviation setting (when FSK modulation is enabled). 165kHz
	{0x16,   0x07},               // Main Radio Control State Machine configuration. (default)
	{0x17,   0x00},               // Main Radio Control State Machine configuration. CCA always, return to idle after TX/RX
	{0x18,   0x18},               // Main Radio Control State Machine configuration. Autocalibrate from IDLE to RX/TX
	{0x19,   0x16},               // Frequency Offset Compensation Configuration.
	{0x1a,   0x6C},               // Bit synchronization Configuration. (default)
	{0x1b,   0x07},               // AGC control.
	{0x1c,   0x40},               // AGC control. (default)
	{0x1d,   0x91},               // AGC control. (default)
	{0x1e,   0x87},               // High byte Event 0 timeout (default)
	{0x1f,   0x6B},               // Low byte Event 0 timeout (default)
	{0x20,   0xF8},               // Wake On Radio control (default)
	{0x21,   0x57},               // Front end RX configuration.
	{0x22,   0x10},               // Front end RX configuration. (default)
	{0x23,   0xE9},               // Frequency synthesizer calibration.
	{0x24,   0x2A},               // Frequency synthesizer calibration.
	{0x25,   0x00},               // Frequency synthesizer calibration.
	{0x26,   0x1F},               // Frequency synthesizer calibration.
	{0x27,   0x41},               // RC oscillator configuration (default)
	{0x28,   0x00},               // RC oscillator configuration (default)
	// {0x29,   0x59},               // Frequency synthesizer calibration control (default)
	// {0x2a,   0x7F},               // Production test (default)
	// {0x2b,   0x3C},               // AGC test (default)
	{0x2c,   0x88},               // Various test settings. (default)
	{0x2d,   0x35},               // Various test settings. (default)
	{0x2e,   0x09},               // Various test settings.
#endif
#if 0
	{CC1101_IOCFG0, 0x06},
	{CC1101_FSCTRL1, 0x06}, //       Frequency Synthesizer Control - IF:152.343Khz
	{CC1101_FSCTRL0, 0x00}, //       Frequency Synthesizer Control - Freq offset
	{CC1101_FREQ2, 0x10}, //         Frequency Control Word, High Byte - 433.999 Mhz
	{CC1101_FREQ1, 0xB1}, //         Frequency Control Word, Middle Byte
	{CC1101_FREQ0, 0x3B}, //         Frequency Control Word, Low Byte
	{CC1101_MDMCFG4, 0xF8}, //       Modem Configuration - BW: 58.035Khz
	{CC1101_MDMCFG3, 0x83}, //       Modem Configuration - 9595 Baud
	{CC1101_MDMCFG2, 0x13}, //       Modem Configuration - 30/32 sync word bits - Manchester disable - GFSK - Digital DC filter enable
	{CC1101_MDMCFG1, 0x22}, //       Modem Configuration - num of preamble bytes:4 - FEC disable
	{CC1101_MDMCFG0, 0xF8}, //       Modem Configuration - Channel spacing: 199.951Khz
	{CC1101_CHANNR, 0x00}, //        Channel Number
	{CC1101_DEVIATN, 0x15}, //       Modem Deviation Setting - 5.157Khz
	{CC1101_FREND1, 0x56}, //        Front End RX Configuration
	{CC1101_FREND0, 0x10}, //        Front End TX Configuration
	{CC1101_MCSM0, 0x18}, //         Main Radio Control State Machine Configuration - PO timeout: 64(149-155us) - Auto calibrate from idle to rx/tx
	{CC1101_FOCCFG, 0x16}, //        Frequency Offset Compensation Configuration
	{CC1101_BSCFG, 0x6C}, //         Bit Synchronization Configuration
	{CC1101_AGCCTRL2, 0x03}, //      AGC Control - target amplitude: 33dB - Maximum possible LNA + LNA 2 gain - All gain settings can be used
	{CC1101_AGCCTRL1, 0x40}, //      AGC Control - LNA gain decreased first
	{CC1101_AGCCTRL0, 0x91}, //      AGC Control - Medium hysterisis - Filter Samples: 16 - Normal AGC operation
	{CC1101_FSCAL3, 0xE9}, //        Frequency Synthesizer Calibration
	{CC1101_FSCAL2, 0x2A}, //        Frequency Synthesizer Calibration
	{CC1101_FSCAL1, 0x00}, //        Frequency Synthesizer Calibration
	{CC1101_FSCAL0, 0x1F}, //        Frequency Synthesizer Calibration
	{CC1101_FSTEST, 0x59}, //        Frequency Synthesizer Calibration Control
	{CC1101_TEST2, 0x88}, //         Various Test Settings
	{CC1101_TEST1, 0x31}, //         Various Test Settings
	{CC1101_TEST0, 0x09}, //         Various Test Settings
	{CC1101_FIFOTHR, 0x07}, //       RX FIFO and TX FIFO Thresholds - Bytes in TX FIFO:33 - Bytes in RX FIFO:32
	{CC1101_PKTCTRL1, 0x04}, //      Packet Automation Control - No address check - Automatic flush of RX FIFO is disable - sync word is always accepted
	{CC1101_PKTCTRL0, 0x05}, //      Packet Automation Control - whitening is off - RX/TX data normal mode - CRC calculation in TX and CRC check in RX - Variable packet length
	{CC1101_ADDR, 0x00}, //          Device Address
	{CC1101_PKTLEN, 0xFF}, //        Packet Length
	{CC1101_MCSM1, 0x3F}, //         Main Radio Control State Machine Configuration
	{CC1101_PATABLE, 0x60},
#endif
};

template<typename SPI,
	typename CSN,
	typename MISO,
	typename IRQ,
	typename DELAY_TIMER,
	const uint8_t CHANNEL,
	const uint16_t SYNC_WORD = 0xd391,
	const uint8_t ADDRESS = 0>
struct CC110X_T {
	static uint8_t status_byte;

	static void init(void) {
		CSN::set_high();
		DELAY_TIMER::set_and_wait(5);
		CSN::set_low();
		DELAY_TIMER::set_and_wait(10);
		CSN::set_high();
		DELAY_TIMER::set_and_wait(41);
		CSN::set_low();
		while (MISO::is_high());
		SPI::transfer(CC1101_SRES);
		while (MISO::is_high());
		CSN::set_high();
		for (int i = 0; i < ARRAY_COUNT(cc110x_init_values); i++) {
			write_reg(cc110x_init_values[i][0], cc110x_init_values[i][1]);
		}
	}

	static void disable(void) {
	}

	static void write_burst_reg(uint8_t addr, const uint8_t *buffer, uint8_t len) {
		CSN::set_low();
		SPI::transfer(WRITE_BURST | addr);
		SPI::transfer((uint8_t *) buffer, len, 0);
		CSN::set_high();
	}

	static uint8_t read_reg(uint8_t addr) {
		uint8_t r;
		CSN::set_low();
		status_byte = SPI::transfer(addr);
		r = SPI::transfer(0xff);
		CSN::set_high();
		return r;
	}

	static void read_burst_reg(uint8_t addr, uint8_t *buffer, uint8_t len) {
		CSN::set_low();
		SPI::transfer(READ_BURST | addr);
		SPI::transfer(0, len, (uint8_t *) buffer);
		CSN::set_high();
	}

	static void write_reg(uint8_t addr, uint8_t value) {
		CSN::set_low();
		status_byte = SPI::transfer(addr);
		SPI::transfer(value);
		CSN::set_high();
	}

	static void strobe(uint8_t cmd) {
	}

	static void power_down(void) {
		CSN::set_low();
		SPI::transfer(CC1101_SIDLE);
		status_byte = SPI::transfer(CC1101_SPWD);
		CSN::set_high();
	}

	static void tx_buffer(const uint8_t *data, const uint8_t len) {
		write_reg(CC1101_TXFIFO, len);
		write_burst_reg(CC1101_TXFIFO, data, len);
		CSN::set_low();
		status_byte = SPI::transfer(CC1101_STX);
		CSN::set_high();
		IRQ::wait_for_irq();
		read_reg(CC1101_STATUS_REGISTER | CC1101_MARCSTATE);
		read_reg(CC1101_STATUS_REGISTER | CC1101_TXBYTES);
	}

	template<typename RX_TIMEOUT = TIMEOUT_NEVER>
	static int rx_buffer(uint8_t *data, int max_len) {
		uint8_t n;
		CSN::set_low();
		status_byte = SPI::transfer(CC1101_SRX);
		CSN::set_high();
		IRQ::wait_for_irq();
		if (read_reg(CC1101_STATUS_REGISTER | CC1101_RXBYTES) > 0) {
			n = read_reg(CC1101_CONFIG_REGISTER | CC1101_RXFIFO);
			if (n > max_len) n = max_len;
			read_burst_reg(CC1101_RXFIFO, data, n);
			read_reg(CC1101_CONFIG_REGISTER | CC1101_RXFIFO);
			read_reg(CC1101_CONFIG_REGISTER | CC1101_RXFIFO);
		}
		CSN::set_low();
		SPI::transfer(CC1101_SIDLE);
		status_byte = SPI::transfer(CC1101_SFRX);
		CSN::set_high();
	}
};

template<typename SPI, typename CSN, typename MISO, typename IRQ, typename DELAY_TIMER, const uint8_t CHANNEL, const uint16_t SYNC_WORD, const uint8_t ADDRESS>
uint8_t CC110X_T<SPI, CSN, MISO, IRQ, DELAY_TIMER, CHANNEL, SYNC_WORD, ADDRESS>::status_byte;

#endif
