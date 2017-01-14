/* ---------------------------------Descriçao-------------------------------
 *
 * Esse código tem como objetivo receber um valor em hexadecimal via serial
 * e alterar o brilho do led utilizando o hardware de PWM do MSP430G2553.
 *
*/

#include <msp430g2553.h>

#ifndef TIMER0_A1_VECTOR
#define TIMER0_A1_VECTOR TIMERA1_VECTOR
#define TIMER0_A0_VECTOR TIMERA0_VECTOR
#endif

#define LED_VERMELHO BIT0   // Pin P1.0
#define LED_VERDE BIT6      // Pin P1.6
#define TXD BIT2            // Pin P1.2
#define RXD BIT1            // Pin P1.1

typedef enum {FALSE, TRUE} boolean;

char receiveBuffer[6];		// Buffer de armazenamento da string do valor hexadecimal
boolean hexaReceive = FALSE;
int pwmValue;				// Armazena valor de ton
int i = 0;

void configureClock(void);
void initializePORT(void);
void initializeUART(void);
void initializeTimerA0(void);
int hexToInt(char hexString[]);

void main(void){

	WDTCTL = WDTPW + WDTHOLD; // Desliga watchdog

	if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)	// Se dados de calibração apagados
		while(1);							        // Armadilha!

    configureClock();
    initializePORT();
    initializeUART();
    initializeTimerA0();

	_BIS_SR(GIE);					// GIE Habilitada
	
	while(TRUE){

		if(hexaReceive){		// Se código hexadecimal recebido

            pwmValue = hexToInt(receiveBuffer);		// Converte o código hexa em decimal

			if(pwmValue >= 999 )		// Se o valor recebido for maior que o período
			    TACCR1 = 998;			// Duty Cycle igual a 99%
			else
				TACCR1 = pwmValue - 1;

            hexaReceive = FALSE;

		}
	}
}

//*************************** ISRs *******************************

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void){

	receiveBuffer[i] = UCA0RXBUF;		// Recebe caracter
	i++;		// Incrementa indexador do buffer de entrada

	if(i == 7){		// Se string em hexa recebida completamente

		i = 0;
		hexaReceive = TRUE;			// Habilita mudança de duty cycle

	}

}
//*************************** FUNCTIONS ***************************

void configureClock(void){

	BCSCTL1 = CALBC1_1MHZ;	// DCO = 1MHz
	DCOCTL = CALDCO_1MHZ;	// DCO = 1MHz
	IFG1 &= ~OFIFG;			// Flag de falha do oscilador resetada

}

void initializePORT(void){

	P1DIR |= LED_VERMELHO + LED_VERDE;	// Pinos ligados aos leds setados como saída
	P1SEL |= RXD + TXD + LED_VERDE;		// P1.1 = RXD, P1.2=TXD, P1.6 = TA0.1
	P1SEL2 |= RXD + TXD ;      			// P1.1 = RXD, P1.2=TXD
	P1OUT &= 0x00;

}

void initializeUART(void){

	UCA0CTL1 |= UCSWRST;	// Reseta USCI
	UCA0CTL1 |= UCSSEL_2;	// SMCLK = 1MHz
	UCA0BR0 = 0x68;			// Baud Rate = 9600
	UCA0BR1 = 0x00;			// Baud Rate = 9600
	UCA0MCTL = UCBRS1;		// Configuração de modulação (Tabela 15-4 User Guide)
	UCA0CTL1 &= ~UCSWRST;	// Inicializa USCI
	UC0IE |= UCA0RXIE;		// Habilita interrupção USCI_A0 RX

}

void initializeTimerA0(void){

   	TACTL = TASSEL_2 + MC_1;	// Fonte de clock = SMCLK, Up-mode
	TACCTL1 |= OUTMOD_7;		// Output mode = reset/set
    TACCR0 = 999;				// Frequência = 1kHz --> (T = (1/SMCLK)*(TACCR0 + 1)
	TACCR1 = 499;				// Inicia com duty cycle = 50%

}

int hexToInt(char hexString[]){		//Converte uma string com valor em hexa para inteiro

    int j;
    int charToInt[4];
    int intValue = 0;

    for(j = 2; j <= 6; j++){

		if(hexString[j] >= '0' && hexString[j] <=  '9')
			charToInt[j-2] = hexString[j] - 48;			// Tabela ASCII --> '0' = 48 em decimal
		else
			if(hexString[j] >= 'a' && hexString[j] <=  'f')
				charToInt[j-2] = hexString[j] - 87;     // Tabela ASCII --> 'a' = 97 em decimal
			else
				if(hexString[j] >= 'A' && hexString[j] <=  'F')
					charToInt[j-2] = hexString[j] - 55;     // Tabela ASCII --> 'A' = 65 em decimal

	}

	intValue |= (charToInt[0] << 12);
	intValue |= (charToInt[1] << 8);
	intValue |= (charToInt[2] << 4);
	intValue |= charToInt[3];

	return intValue;

}
