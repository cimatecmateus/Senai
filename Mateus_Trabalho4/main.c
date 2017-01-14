#include <msp430g2553.h>
#include <msp430.h>
#include <intrinsics.h>
#include <stdint.h>
#include <stddef.h>
#include "uart.h"
#include "config.h"
#include "DHT11.h"

#ifndef TIMER0_A1_VECTOR
#define TIMER0_A1_VECTOR TIMERA1_VECTOR
#define TIMER0_A0_VECTOR TIMERA0_VECTOR
#endif

typedef enum{HUMIDITY, TEMPERATURE}REQUEST;

void delay_ms(int x_ms);

char umidade = 0;
char temperatura = 0;
char recebido;
char msb = 33;

void main(void){

	WDTCTL = WDTPW + WDTHOLD; //Desliga watchdog

	if (CALBC1_8MHZ ==0xFF || CALDCO_8MHZ == 0xFF)	// Se dados de calibração apagados
		while(1);							        // Armadilha!

    configureClock();
    initializeTimerA0();
    initializeTimerA1();
    initializePORT1();

    uart_config_t config;
    config.baud = 9600;

    if(uart_init(&config) != 0)		//If error in the uart config, trap!
    	while(1);

	_BIS_SR(GIE);					// GIE enable

	while(1){

		recebido = uart_getchar();

		if(recebido != -1 && readDht(&umidade, &temperatura) == DHT_OK){

			P1OUT |= LED_VERMELHO;

			switch(recebido){
				case 't':
					uart_putchar(temperatura);
					break;
				case 'h':
					uart_putchar(umidade);
					break;
				default:
					uart_putchar(0);
			}
		}
		else
			P1OUT &= ~LED_VERMELHO;
	}
}

void delay_ms(int x_ms){
	int i;
	for(i = 0; i < x_ms; i++){
		TA1CTL |= MC_1;
		while(!(TA1CCTL0 & CCIFG));
		TA1CCTL0 &= ~CCIFG;
		TA1CTL |= TACLR;
	}
}





