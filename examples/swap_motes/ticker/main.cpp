#include <msp430.h>
#include <stdint.h>
#include <string.h>

#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <io.h>
#include <timer.h>
#include <nrf24.h>
#include <flash.h>
#include <adc.h>
#ifdef __MSP430_HAS_USCI__
#include <usci_spi.h>
#include <usci_uart.h>
#else
#include <usi_spi.h>
#include <soft_uart.h>
#endif

#include <swap_mote.h>

extern constexpr uint8_t rx_addr[5] = {
	0xff, 0xf0, 0xf0, 0xf0, 0xf0
};

typedef VLOCLK_T<> VLO;
typedef DCOCLK_T<> DCO;
typedef ACLK_T<VLO> ACLK;
typedef MCLK_T<DCO> MCLK;
typedef SMCLK_T<DCO> SMCLK;

#ifdef __MSP430_HAS_USCI__
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef GPIO_MODULE_T<1, 5, 3> SCLK;
typedef GPIO_MODULE_T<1, 6, 3> MISO;
typedef GPIO_MODULE_T<1, 7, 3> MOSI;
#else
typedef GPIO_INPUT_T<1, 1> RX;
typedef GPIO_OUTPUT_T<1, 2> TX;
typedef GPIO_PIN_T<1, 5, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> SCLK;
typedef GPIO_PIN_T<1, 6, OUTPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MOSI;
typedef GPIO_PIN_T<1, 7, INPUT, LOW, INTERRUPT_DISABLED, TRIGGER_RISING, 1> MISO;
#endif
typedef GPIO_OUTPUT_T<1, 0, LOW> LED_RED;
typedef GPIO_OUTPUT_T<2, 0, LOW> CE;
typedef GPIO_OUTPUT_T<2, 1, HIGH> CSN;
typedef GPIO_INPUT_T<2, 2> IRQ;
typedef GPIO_PORT_T<1, LED_RED, SCLK, MISO, MOSI ,RX, TX> PORT1;
typedef GPIO_PORT_T<2, IRQ, CSN, CE> PORT2;

typedef TIMER_T<TIMER_A, 0, SMCLK, TIMER_MODE_CONTINUOUS> TIMER;
typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;

#ifdef __MSP430_HAS_USCI__
typedef USCI_UART_T<USCI_A, 0, SMCLK> UART;
typedef USCI_SPI_T<USCI_B, 0, SMCLK, true, 0> SPI;
#else
typedef SOFT_UART_T<TIMER, TX, RX> UART;
typedef USI_SPI_T<SMCLK, true, 0> SPI;
#endif

typedef TIMEOUT_T<WDT> TIMEOUT;

template<const uint32_t ID>
struct TICK_REGISTER_T {
	static uint16_t value;
	static constexpr uint8_t reg_id = ID;
	static bool write(SWAP_PACKET& packet) {
		packet.reg_id = ID;
		packet.reg_value[0] = value >> 8;
		packet.reg_value[1] = value & 0xff;
		packet.len = 7 + 2;
		return true;
	};
	static bool handle_command(SWAP_PACKET& packet) { return false; }
	static bool handle_query(SWAP_PACKET& packet) { return false; }
	static void update(void) { value++; };
};

template<const uint32_t ID>
uint16_t TICK_REGISTER_T<ID>::value = 0;

typedef TICK_REGISTER_T<11> TICKER;
typedef NRF24_T<SPI, CSN, CE, IRQ> NRF24;

template<typename SERIAL, typename TIMEOUT, const int MAX_LEN>
struct MOCK_RADIO_T {
	static void tx_buffer(const uint8_t tx_addr[5], const uint8_t *data, uint8_t len, bool auto_ack) {
		SERIAL::puts("TX: ");
		while (len > 0) {
			printf<SERIAL>("%02x", *data++);
			len--;
		}
		SERIAL::putc('\n');
	}

	static int rx_buffer(uint8_t *data, int max_len, uint8_t *pipe, unsigned int timeout) {
		uint8_t c, *d;
		int i, n;
		uint8_t buffer[MAX_LEN];

		SERIAL::puts("RX? ");
		TIMEOUT::set(timeout);
		i = 0;
		while (i < MAX_LEN &&
				(c = SERIAL::template getc<TIMEOUT>()) != '\n' &&
				!TIMEOUT::triggered()) {
			buffer[i] = c;
			SERIAL::putc(c);
			i++;
		}
		SERIAL::putc('\n');
		n = i / 2;
		for (i = 0; i < max_len && i < n; i++) {
			data[i] = (FROM_HEX(buffer[i]) << 4) + FROM_HEX(buffer[i + 1]);
		}
		if (pipe) *pipe = 0;
		return i;
	}

	static void start_tx(void) {
		SERIAL::puts("Start TX\n");
	}

	static void start_rx(void) {
		SERIAL::puts("Start RX\n");
	}

	static void power_down(void) {
		SERIAL::puts("Powering down radio\n");
	}

	static void read_regs(uint8_t *data) {
	}
};

typedef MOCK_RADIO_T<UART, TIMEOUT, sizeof(SWAP_PACKET) * 2> MOCK_RADIO;

typedef SWAP_MOTE_T<1, 1, 1, 1, NRF24, 70, TIMEOUT, CONFIG_STORAGE_UNUSED, TICKER> MOTE;

void dump_regs(void)
{
	uint8_t regs[64];
	NRF24::read_regs(regs);
	hex_dump_bytes<UART>(regs, sizeof(regs));
}


int main(void)
{
	DCO::init();
	ACLK::init();
	SMCLK::init();
	TIMER::init();

	WDT::init();
	WDT::enable_irq();

	PORT1::init();
	PORT2::init();
	UART::init();
	printf<UART>("Mote example start\n");
	SPI::init();
	NRF24::init();
	MOTE::init();
	TIMEOUT::set(10000);
	MOTE::announce<TIMEOUT>();
	TIMEOUT::disable();
	while (1) {
		printf<UART>("Updating registers\n");
		MOTE::update_registers();
		printf<UART>("Transmitting data\n");
		MOTE::transmit_data();
		printf<UART>("Sleeping\n");
		MOTE::sleep();
	}
	return 0;
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::count_down()) exit_idle();
}

#ifdef __MSP430_HAS_USCI__
void usci_tx_irq(void) __attribute__((interrupt(USCIAB0TX_VECTOR)));
void usci_tx_irq(void)
{
	if (UART::handle_tx_irq()) exit_idle();
	if (SPI::handle_tx_irq()) exit_idle();
}

void usci_rx_irq(void) __attribute__((interrupt(USCIAB0RX_VECTOR)));
void usci_rx_irq(void)
{
	if (UART::handle_rx_irq()) exit_idle();
	if (SPI::handle_rx_irq()) exit_idle();
}
#else
void usi_irq(void) __attribute__((interrupt(USI_VECTOR)));
void usi_irq(void)
{
	if (SPI::handle_irq()) exit_idle();
}
#endif
