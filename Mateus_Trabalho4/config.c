#include <msp430g2553.h>
#include "config.h"

void configureClock(void){

	BCSCTL1 = CALBC1_8MHZ;		// Set range
	DCOCTL = CALDCO_8MHZ;		// Set DCO step + modulation
	BCSCTL2 = DIVS_3;			// SMCLK / 8
	IFG1 &= ~OFIFG;				// Flag de falha do oscilador resetada
}

void initializePORT1(void){

	P1SEL |= RXD + TXD ;       					// P1.1 = RXD, P1.2=TXD
	P1SEL2 |= RXD + TXD ;      					// P1.1 = RXD, P1.2=TXD
	P1DIR |= LED_VERMELHO + LED_VERDE + RE + DE; // P1.0, P1.6, P1.3 and P1.4 as output
	P1OUT &= 0x00;

}

void initializeTimerA0(void){

   	TACTL = TASSEL_2 + MC_1;	// Fonte de clock = SMCLK
    TACCR0 = 1000;				// Interrupção a cada 1us
    TACTL |= TACLR;				// Timer parado
}

void initializeTimerA1(void){

   	TA1CTL = TASSEL_2 + MC_1;		// Fonte de clock = SMCLK
    TA1CCR0 = 1000;					// Interrupção a cada 1us
    TA1CTL |= TACLR;				// Timer parado
}



