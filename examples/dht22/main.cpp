#include <gpio.h>
#include <clocks.h>
#include <wdt.h>
#include <spi.h>
#include <uart.h>
#include <io.h>
#include <timer.h>

typedef ACLK_T<ACLK_SOURCE_VLOCLK> ACLK;
typedef SMCLK_T<> SMCLK;

typedef GPIO_OUTPUT_T<1, 0> LED_RED;
typedef GPIO_MODULE_T<1, 1, 3> RX;
typedef GPIO_MODULE_T<1, 2, 3> TX;
typedef GPIO_PIN_T<1, 4, OUTPUT, HIGH> DHT_PIN;
typedef GPIO_PORT_T<1, RX, TX, LED_RED, DHT_PIN> PORT1;

typedef WDT_T<ACLK, WDT_TIMER, WDT_INTERVAL_512> WDT;
typedef UART_T<USCI_A, 0, SMCLK> UART;

typedef TIMER_T<TIMER_A, 0, SMCLK, TASSEL_2 + MC_2> TIMER;
typedef TIMEOUT_T<WDT> TIMEOUT;

template<typename T, typename PIN>
int read_dht(unsigned char *p)
{
	// Note: TimerA must be continuous mode (MC_2) at 1 MHz
        const unsigned b = BIT4;                                    // I/O bit
	const unsigned char *end = p + 6;                           // End of data buffer
	unsigned char m = 1;                                        // First byte will have only start bit
	unsigned st, et;                                            // Start and end times

	p[0] = p[1] = p[2] = p[3] = p[4] = p[5] = 0;                // Clear data buffer

	PIN::set_low();                                             // Pull low
	PIN::set_output();                                          // Output
	PIN::disable_resistor();                                    // Pull low
	st = TIMER::counter(); while((TIMER::counter() - st) < 18000);  // Wait 18 ms
	PIN::enable_resistor();                                     // Pull low
	PIN::set_high();                                            // Pull high
	PIN::set_input();                                           // Input

	st = TIMER::counter();                                      // Get start time for timeout
	while(PIN::is_high())
		if((TIMER::counter() - st) > 100) return -1;        // Wait while high, return if no response
	et = TIMER::counter();                                      // Get start time for timeout
	do {
		st = et;                                                // Start time of this bit is end time of previous bit
		while(PIN::is_low())
			if((TIMER::counter() - st) > 100) return -2;    // Wait while low, return if stuck low
		while(PIN::is_high())
			if((TIMER::counter() - st) > 200) return -3;    // Wait while high, return if stuck high
		et = TIMER::counter();                                  // Get end time
		if((et - st) > 110) *p |= m;                            // If time > 110 us, then it is a one bit
		if(!(m >>= 1)) m = 0x80, ++p;                           // Shift mask, move to next byte when mask is zero
	} while(p < end);                                           // Do until array is full

	p -= 6;                                                     // Point to start of buffer
	if(p[0] != 1) return -4;                                    // No start bit
	if(((p[1] + p[2] + p[3] + p[4]) & 0xFF) != p[5]) return -5; // Bad checksum

	return 0;                                                   // Good read
}

int main(void)
{
	unsigned char buffer[6];
	int r;

	ACLK::init();
	SMCLK::init();
	WDT::init();
	TIMER::init();
	PORT1::init();
	UART::init();
	WDT::enable_irq();
	printf<UART>("DHT22 example\n");
	while (1) {
		LED_RED::toggle();
		r = read_dht<TIMER, DHT_PIN>(buffer);
		printf<UART>("%d %d (%d)\n",
		       ((buffer[1] << 8) + buffer[2]), ((buffer[3] << 8) + buffer[4]), r);
		TIMEOUT::set_timeout(1000);
		enter_idle<TIMEOUT>();
	}
}

void watchdog_irq(void) __attribute__((interrupt(WDT_VECTOR)));
void watchdog_irq(void)
{
	if (TIMEOUT::timeout_triggered()) exit_idle();
}