#ifndef __USCI_H
#define __USCI_H

enum USCI_MODULE {
	USCI_A,
	USCI_B
};

static constexpr int usci_a_registers[][11] = {
	{UCA0CTL0_, UCA0CTL1_, UCA0BR0_, UCA0BR1_, UCA0MCTL_, UCA0STAT_, UCA0RXBUF_, UCA0TXBUF_, 0, 0, 0},
#ifdef UCA1CTL0_
	{UCA1CTL0_, UCA1CTL1_, UCA1BR0_, UCA1BR1_, UCA1MCTL_, UCA1STAT_, UCA1RXBUF_, UCA1TXBUF_, 0, 0, 0}
#endif
};

static constexpr int usci_b_registers[][11] = {
	{UCB0CTL0_, UCB0CTL1_, UCB0BR0_, UCB0BR1_, 0, UCB0STAT_, UCB0RXBUF_, UCB0TXBUF_, UCB0I2CIE_, UCB0I2COA_, UCB0I2CSA_},
#ifdef UCB1CTL0_
	{UCB1CTL0_, UCB1CTL1_, UCB1BR0_, UCB1BR1_, 0, UCB1STAT_, UCB1RXBUF_, UCB1TXBUF_, UCB1I2CIE_, UCB1I2COA_, UCB1I2CSA_}
#endif
};

#define USCI_REGISTER(module, instance, reg) ((unsigned char *) (module == USCI_A ? usci_a_registers : usci_b_registers)[instance][reg]);

template<const USCI_MODULE MODULE,
	const int INSTANCE>
struct USCI_T {
	static constexpr volatile unsigned char *CTL0 = USCI_REGISTER(MODULE, INSTANCE, 0);
	static constexpr volatile unsigned char *CTL1 = USCI_REGISTER(MODULE, INSTANCE, 1);
	static constexpr volatile unsigned char *BR0 = USCI_REGISTER(MODULE, INSTANCE, 2);
	static constexpr volatile unsigned char *BR1 = USCI_REGISTER(MODULE, INSTANCE, 3);
	static constexpr volatile unsigned char *MCTL = USCI_REGISTER(MODULE, INSTANCE, 4);
	static constexpr volatile unsigned char *STAT = USCI_REGISTER(MODULE, INSTANCE, 5);
	static constexpr volatile unsigned char *RXBUF = USCI_REGISTER(MODULE, INSTANCE, 6);
	static constexpr volatile unsigned char *TXBUF = USCI_REGISTER(MODULE, INSTANCE, 7);
	static constexpr volatile unsigned char *I2CIE = USCI_REGISTER(MODULE, INSTANCE, 8);
	static constexpr volatile unsigned char *I2COA = USCI_REGISTER(MODULE, INSTANCE, 9);
	static constexpr volatile unsigned char *I2CSA = USCI_REGISTER(MODULE, INSTANCE, 10);

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
