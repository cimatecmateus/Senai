#include <msp430g2553.h>

#ifndef TIMER0_A1_VECTOR
#define TIMER0_A1_VECTOR TIMERA1_VECTOR
#define TIMER0_A0_VECTOR TIMERA0_VECTOR
#endif

enum states {OFF, SHOWTIME, ADJHOUR, ADJMIN, SHOWSEC};		//Estados da m�quina
enum events {PRESS4SEC, PRESS8SEC, PRESSFAST, INACTIVITY};	//Eventos da m�quina
enum ocurrences{NO, YES};									//Ocorr�ncias de eventos

#define LED1 BIT0	//Pino referente ao led vermelho
#define LED2 BIT6	//Pino referente ao led verde
#define BUTTON BIT3	//Pino referente ao bot�o da lauchpad

//*************************** Vari�veis globais ********************************

int cont = 0;				//Aux�lia o controle do per�odo do pisca leds
int currentState = OFF;		//Estado atual da m�quina de estado
int nextState = OFF;		//Pr�ximo estado da m�quina de estado
int occurredEvent = NO;		//Usado como flag para ocorr�ncia de evento
int timePressed = 0;		//Usado para armazenar o tempo em que bot�o fica pressionado
int period = 0;				//Controla o per�odo do pisca leds
int led;					//Usado para designar qual led piscar�
int ton;					//Controla o ciclo de trabalho dos leds
int inactivityTime = 0;		//Usado para armazenar o tempo de inatividade

//*************************** Prot�tipo de fun��es ********************************

void initializeTimerA0(void);	//Fun��o para configura��o do TIMERA0
void initializeTimerA1(void);	//Fun��o para configura��o do TIMERA1
void initializePORT1(void);		//Fun��o para configura��o do PORT1
void configureClock(void);		//Fun��o para configura��o do CLOCK do Mcu
void isrTimerA0(void);			//ISR do canal de compara��o TA0CCR0
void isrTimerA1(void);			//ISR do canal de compara��o TA1CCR0
void StateMachine(int event);	//Fun��o da m�quina de estados
int eventSelect(void);			//Fun��o que verifica qual evento foi gerado
void stateSelect(int state);	//Fun��o que seleciona o pr�ximo estado da m�quina de estados
void setBlink(int _led, float freq, float dutyCycle);	//Fun��o que define como os leds irão piscar
void wakeUp(void);		//Fun��o que acorda a CPU

void main(void) {

    WDTCTL = WDTPW | WDTHOLD;	//Para o WatchDog

    if (CALBC1_1MHZ == 0xFF || CALDCO_1MHZ == 0xFF)	//Se dados de calibra��o do DCO foram apagados
    	while(1);									//Trave a CPU

    initializePORT1();		//Configura PORT1
    configureClock();		//Configura CLOCK
    initializeTimerA0();	//Configura TIMERA1
    initializeTimerA1();	//Configura TIMERA0

    __bis_SR_register(LPM3_bits + GIE);		//CPU no LPM3

	while(1){

		if(occurredEvent == YES){
			StateMachine(eventSelect());
			stateSelect(nextState);
			timePressed = 0;
			inactivityTime = 0;
			occurredEvent = NO;
		}

	}

}

//*************************** STATE MACHINE ********************************

void StateMachine(int event){ 		//Fun��o chamada para alterar o pr�ximo estado da m�quina

	switch (currentState){
		case OFF:
			switch(event){
				case PRESS4SEC:
					nextState = SHOWTIME;
					break;
			}
		break;

		case SHOWTIME:
			switch(event){
				case PRESS4SEC:
					nextState = ADJHOUR;
					break;
				case PRESS8SEC:
					nextState = OFF;
					break;
				case PRESSFAST:
					nextState = SHOWSEC;
					break;
			}
		break;

		case ADJHOUR:
			switch(event){
				case PRESS4SEC:
					nextState = ADJMIN;
					break;
				case PRESS8SEC:
					nextState = OFF;
					break;
				case INACTIVITY:
					nextState = SHOWTIME;
					break;
			}
		break;

		case ADJMIN:
			switch(event){
				case PRESS4SEC:case INACTIVITY:
					nextState = SHOWTIME;
					break;
				case PRESS8SEC:
					nextState = OFF;
			}
		break;

		case SHOWSEC:
			switch(event){
				case PRESS8SEC:
					nextState = OFF;
					break;
				case PRESSFAST:
					nextState = SHOWTIME;
					break;
			}
		break;

	}

}

int eventSelect(void){		//Fun��o que detecta qual evento foi gerado

	if(inactivityTime >= 250)
		return INACTIVITY;

	if((timePressed > 0) && (timePressed < 50))
		return PRESSFAST;
	else
		if((timePressed >= 50) && (timePressed < 100))
			return PRESS4SEC;
		else
			return PRESS8SEC;

}

void stateSelect(int state){		//Fun��o chamada para configurar as a��es de cada estado
		switch (state){
			case OFF:
				P1OUT &= ~(LED1 + LED2);			//Apaga leds
				TA0CCTL0 &= ~(CCIE);				//Desabilita interrup��o do TA0CCR0
				TA1CCTL0 &= ~(CCIE);				//Desabilita interrup��o do TA1CCR0
				TA1CCR0 = 0;						//TA1R parado
				P1IFG &= ~(BUTTON);					//Reseta flag de interrup��o do bot�o
				P1IE |= BUTTON;						//HAbilita interrup��o do bot�o
				__bis_SR_register(LPM3_bits);		//CPU em LPM3
				break;
			case SHOWTIME:
				TA0CCTL0 |= CCIE;
				P1OUT &= ~(LED1 + LED2);
				setBlink((LED1 + LED2), 1, 0.9);
				break;
			case ADJHOUR:
				P1OUT &= ~(LED1 + LED2);
				setBlink(LED1, 1, 0.5);
				break;
			case ADJMIN:
				P1OUT &= ~(LED1 + LED2);
				setBlink(LED2, 1, 0.5);
				break;
			case SHOWSEC:
				P1OUT &= ~(LED1 + LED2);
				setBlink((LED1 + LED2), 1, 0.1);
				break;
		}

	currentState = nextState;
}

//*************************** ISRs *******************************

#pragma vector = TIMER0_A0_VECTOR		//Isr chamada cada 0,05s --> (TA0CCR0 + 1)/12kHz
__interrupt void trataTimer0(void){

	cont++;

	if(cont <= ton)
		P1OUT |= led;
	else
		if(cont > ton && cont < period)
			P1OUT &= ~led;
		else
			cont = 0;

}


#pragma vector = TIMER1_A0_VECTOR		//Isr chamada a cada 0,08s --> (TA1CCR0 + 1)/12kHz
__interrupt void trataTimer1_A0(void){
	volatile int buttonState;

	buttonState = ~P1IN;

	if((buttonState & BUTTON) && (occurredEvent != YES)){		//Se bot�o pressionado e nenhuma ocorrência pendente

		inactivityTime = 0;

		timePressed++;											//Incremente tempo do bot�o pressionado (timePressed = 1 equivale a 0,08s)

		if(timePressed >= 45 && timePressed <= 55){				//Bloco que serve para alertar o usu�rio que se passaram 4s

    		switch(timePressed){								//Pisca os leds quando o bot�o for pressionado por 4s
    		    case 46:
    		        P1OUT &= ~(LED1 + LED2);
    		        break;
    		    case 48:
    		        P1OUT |= (LED1 + LED2);
    		        break;
    		    case 50:
    		        P1OUT &= ~(LED1 + LED2);
    		        break;
    		}
		}

		if(timePressed == 100)		//Se passado 8s (100*0,08) ativa flag de ocorr�ncia de evento
			occurredEvent = YES;

	}
	else{		//Se o bot�o n�o estiver pressionado
	        inactivityTime++;		//Incrementa tempode inatividade
	        if(inactivityTime == 250 || timePressed != 0)		//Se passado 20s (250*0,08) ativa flag de evento
	            occurredEvent = YES;

	}


}

#pragma vector = PORT1_VECTOR
__interrupt void wakeUp(void){

	P1IE &= ~(BUTTON);		//Desabilita interrup��o do bot�o
	TA1CCR0 = 959;			//Liga TA1R
	TA1CCTL0 |= CCIE;		//Habilita interrup��o do canal de captura
	P1IFG &= ~(BUTTON);		//Reseta flag de interrup��o do bot�o
	__bic_SR_register_on_exit(LPM3_bits);		//Sai do modo LPM3

}

//*************************** fun��es *******************************

void initializeTimerA0(void){

    TA0CTL = TASSEL_1 + MC_1;	//Fonte de clock = ACLK e Up mode
    TA0CCR0 = 599;				//Per�odo de interrup��o de 0,05s

}

void initializeTimerA1(void){

    TA1CTL = TASSEL_1 + MC_1;	//Fonte de clock = ACLK e Up mode
    TA1CCR0 = 0;				//TA1R parado

}

void configureClock(void){

    BCSCTL1 = CALBC1_1MHZ;		//Set range
    DCOCTL = CALDCO_1MHZ;		//Set DCO step + modulation
    BCSCTL3 |= LFXT1S_2;		//Low-frequency clock = VLO
    IFG1 &= ~OFIFG;				//Flag de falha do oscilador resetada

}

void initializePORT1(void){

    P1DIR = 0x41;				//Pinos P1.0 e P1.6 como sa�da
    P1REN = BUTTON;				//Resistor do pino P1.3 habilitado
    P1OUT = BUTTON;				//Resistor do pino P1.3 como pull-up
    P1IE  = BUTTON;				//interrup��o habilitada
    P1IES = BUTTON;				//interrup��o na borda de descida
    P1IFG = 0;					//Zera todas flags do PORT1

}

void setBlink(int _led, float freq, float dutyCycle){
	period = (1/freq) * 20;			//period * 0,05 = (1/freq)
	ton = (dutyCycle/freq) * 20;	//dutycyle = ton / period
	led = _led;
	cont = 0;
}

