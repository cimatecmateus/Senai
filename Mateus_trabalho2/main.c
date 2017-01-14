/* ------------------------------Descriçao----------------------------
 *  
 * Esse código tem como objetivo enviar via serial o dado digital do
 * sensor interno de temperatura para o arduino após a solicitação 
 * do mesmo. Para o envio via serial, o valor digital do ADC foi
 * convertido em dois outros valores de 1 byte.
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

unsigned int temperatureSensor; // ADC valor correspondente a temperatura
char dataLSB = 0, dataMSB = 0;  // Armazenam LSB e MSB do valor digital da temperatura

void configureClock(void);
void initializePORT1(void);
void initializeUART(void);
void initializeADC10(void);
void initializeTimerA0(void);

void main(void){

	WDTCTL = WDTPW + WDTHOLD; //Desliga watchdog

	if (CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF)	// Se dados de calibração apagados
		while(1);							        // Armadilha!

    configureClock();
    initializePORT1();
    initializeUART();
    initializeADC10();

	_BIS_SR(GIE);					// GIE Habilitada
	
	ADC10CTL0 |= ENC + ADC10SC; 	// Inicia amostragem e conversão

	while(1);

}

//*************************** ISRs *******************************

#pragma vector = ADC10_VECTOR
__interrupt void ADC10_ISR(void){
    
	temperatureSensor = ADC10MEM;                   //Armazena valor do ADC
	dataLSB = (temperatureSensor & 0xFF);
	dataMSB = ((temperatureSensor >> 8) & 0xFF);
	ADC10CTL0 &= ~ADC10IFG;     //Reseta flag de interrupção

}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void){
    
   if (UCA0RXBUF == 'a') 
   {
      P1OUT |= LED_VERMELHO + LED_VERDE;
      UC0IE |= UCA0TXIE;    // Habilita interrupção USCI_AO TX
      UCA0TXBUF = dataLSB;  // Escreve no buffer de transmissão byte menos significativo
   }
   
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void){
    
    UCA0TXBUF = dataMSB;    // Escreve no buffer de transmissão byte mais significativo
    UC0IE &= ~UCA0TXIE;     // Desabilita interrupção USCI_A0 TX
    P1OUT &= ~(LED_VERMELHO + LED_VERDE);

}

//*************************** FUNCTIONS ***************************

void configureClock(void){

    BCSCTL1 = CALBC1_1MHZ;		// Set range
    DCOCTL = CALDCO_1MHZ;		// Set DCO step + modulation
    BCSCTL3 |= LFXT1S_2;		// Low-frequency clock = VLO
    IFG1 &= ~OFIFG;				// Flag de falha do oscilador resetada

}

void initializePORT1(void){

	P2DIR = 0xFF;              // Todos os pinos do PORT2 como saída
	P2OUT &= 0x00;             // Todos os pinos do PORT2 em nível baixo
	P1SEL |= RXD + TXD ;       // P1.1 = RXD, P1.2=TXD
	P1SEL2 |= RXD + TXD ;      // P1.1 = RXD, P1.2=TXD
	P1DIR |= LED_VERMELHO + LED_VERDE; 
	P1OUT &= 0x00;

}

void initializeUART(void){

   UCA0CTL1 |= UCSWRST;     // Reseta USCI
   UCA0CTL1 |= UCSSEL_2;    // SMCLK = 1MHz 
   UCA0BR0 = 0x68;          // Baud Rate = 9600
   UCA0BR1 = 0x00;          // Baud Rate = 9600
   UCA0MCTL = UCBRS1;       // Modulation UCBRSx = 1
   UCA0CTL1 &= ~UCSWRST;    // Inicializa USCI
   UC0IE |= UCA0RXIE;       // Habilita interrupção USCI_A0 RX
}   

void initializeADC10(void){
    
    ADC10CTL1 = INCH_10 + ADC10DIV_3 + CONSEQ_2; 		                    // Temperature Sensor, ADC10CLK/5, Repeat-Single-Channel Mode
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + MSC + REFON + ADC10ON + ADC10IE;	    // Vr+ = Vref+ : Vr- = Vss, S&H time 64xADC10CLKs, Repeat-Single-Channel Mode, Reference generator on, ADC on, Interrupt Enable from ADC10
    
}
