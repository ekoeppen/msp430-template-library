#ifndef __USCI_H
#define __USCI_H

enum USCI_MODULE {
	USCI_A,
	USCI_B
};

static constexpr volatile void *usci_a_registers[][11] = {
	{&UCA0CTL0, &UCA0CTL1, &UCA0BR0, &UCA0BR1, &UCA0MCTL, &UCA0STAT,
		const_cast<uint8_t *>(&UCA0RXBUF), &UCA0TXBUF, 0, 0, 0},
#ifdef UCA1CTL0_
	{&UCA1CTL0, &UCA1CTL1, &UCA1BR0, &UCA1BR1, &UCA1MCTL, &UCA1STAT,
		const_cast<uint8_t *>(&UCA1RXBUF), &UCA1TXBUF, 0, 0, 0}
#endif
};

static constexpr volatile void *usci_b_registers[][11] = {
	{&UCB0CTL0, &UCB0CTL1, &UCB0BR0, &UCB0BR1, 0, &UCB0STAT,
		const_cast<uint8_t *>(&UCB0RXBUF), &UCB0TXBUF, &UCB0I2CIE, &UCB0I2COA, &UCB0I2CSA},
#ifdef UCB1CTL0_
	{&UCB1CTL0, &UCB1CTL1, &UCB1BR0, &UCB1BR1, 0, &UCB1STAT,
		const_cast<uint8_t *>(&UCB1RXBUF), &UCB1TXBUF, &UCB1I2CIE, &UCB1I2COA, &UCB1I2CSA}
#endif
};

#define USCI_REGISTER(module, instance, reg) reinterpret_cast<volatile uint8_t*>((module == USCI_A ? usci_a_registers : usci_b_registers)[instance][reg]);

template<const USCI_MODULE MODULE,
	const int INSTANCE>
struct USCI_T {
	static constexpr volatile uint8_t *CTL0 = USCI_REGISTER(MODULE, INSTANCE, 0);
	static constexpr volatile uint8_t *CTL1 = USCI_REGISTER(MODULE, INSTANCE, 1);
	static constexpr volatile uint8_t *BR0 = USCI_REGISTER(MODULE, INSTANCE, 2);
	static constexpr volatile uint8_t *BR1 = USCI_REGISTER(MODULE, INSTANCE, 3);
	static constexpr volatile uint8_t *MCTL = USCI_REGISTER(MODULE, INSTANCE, 4);
	static constexpr volatile uint8_t *STAT = USCI_REGISTER(MODULE, INSTANCE, 5);
	static constexpr volatile uint8_t *RXBUF = USCI_REGISTER(MODULE, INSTANCE, 6);
	static constexpr volatile uint8_t *TXBUF = USCI_REGISTER(MODULE, INSTANCE, 7);
	static constexpr volatile uint8_t *I2CIE = USCI_REGISTER(MODULE, INSTANCE, 8);
	static constexpr volatile uint16_t *I2COA = USCI_REGISTER(MODULE, INSTANCE, 9);
	static constexpr volatile uint8_t *I2CSA = USCI_REGISTER(MODULE, INSTANCE, 10);

	static void enable_rx_irq(void) {
		IE2 |= (MODULE == USCI_A ? UCA0RXIE : UCB0RXIE);
	}

	static void enable_tx_irq(void) {
		IE2 |= (MODULE == USCI_A ? UCA0TXIE : UCB0TXIE);
	}

	static void enable_rx_tx_irq(void) {
		IE2 |= (MODULE == USCI_A ? UCA0TXIE | UCA0RXIE : UCB0TXIE | UCB0RXIE);
	}

	static void disable_tx_irq(void) {
		IE2 &= ~(MODULE == USCI_A ? UCA0TXIE : UCB0TXIE);
	}

	static void disable_rx_irq(void) {
		IE2 &= ~(MODULE == USCI_A ? UCA0RXIE : UCB0RXIE);
	}

	static void disable_rx_tx_irq(void) {
		IE2 &= ~(MODULE == USCI_A ? UCA0TXIE | UCA0RXIE : UCB0TXIE | UCB0RXIE);
	}

	static void clear_rx_irq(void) {
		IFG2 &= ~(MODULE == USCI_A ? UCA0RXIFG : UCB0RXIFG);
	}

	static void clear_tx_irq(void) {
		IFG2 &= ~(MODULE == USCI_A ? UCA0TXIFG : UCB0TXIFG);
	}

	static bool tx_irq_pending(void) {
		return (IFG2 & (MODULE == USCI_A ? UCA0TXIFG : UCB0TXIFG));
	}

	static bool rx_irq_pending(void) {
		return (IFG2 & (MODULE == USCI_A ? UCA0RXIFG : UCB0RXIFG));
	}

};

#endif
