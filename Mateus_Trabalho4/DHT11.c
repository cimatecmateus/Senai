#include <msp430g2553.h>
#include <msp430.h>
#include <stdint.h>
#include "config.h"
#include "DHT11.h"

DHT_STATUS readDht(char *_umidade, char *_temperatura){
	uint8_t checksum;
	unsigned char idx = 0;
	unsigned char cnt = 7;
	unsigned char bits[5];
	int i;

	for(i = 0; i < 5; i++)
		bits[i] = 0;

	TACCTL0 &= ~CCIFG;

	P2DIR |= DHPIN;
	P2OUT &= ~DHPIN;		//DHPIN em nível baixo

	TACCR0 = 18000;			//Espera 18ms
	TACTL |= MC_1;
	while(!(TACCTL0 & CCIFG));
	TACCTL0 &= ~CCIFG;
	TACTL |= TACLR;

	P2OUT |= DHPIN;

	TACCR0 = 40;
	TACTL |= MC_1;
	while(!(TACCTL0 & CCIFG));
	TACCTL0 &= ~CCIFG;
	TACTL |= TACLR;

	P2DIR &= ~DHPIN;

	TACCR0 = 100;
	TACTL |= MC_1;
	while(!(P2IN & DHPIN)){
		if(TACCTL0 & CCIFG){
			TACCTL0 &= ~CCIFG;
			return TIMEOUT;
		}
	}
	TACCTL0 &= ~CCIFG;
	TACTL |= TACLR;

	TACCR0 = 100;
	TACTL |= MC_1;
	while(P2IN & DHPIN){
		if(TACCTL0 & CCIFG){
			TACCTL0 &= ~CCIFG;
			return TIMEOUT;
		}
	}
	TACCTL0 &= ~CCIFG;
	TACTL |= TACLR;

	for(i = 0; i < 40; i++){

		TACCR0 = 100;
		TACTL |= MC_1;
		while(!(P2IN & DHPIN)){
			if(TACCTL0 & CCIFG){
				TACCTL0 &= ~CCIFG;
				return TIMEOUT;
			}
		}
		TACCTL0 &= ~CCIFG;
		TACTL |= TACLR;

		TACCR0 = 100;
		TACTL |= MC_1;
		while(P2IN & DHPIN){
			if(TACCTL0 & CCIFG){
				TACCTL0 &= ~CCIFG;
				return TIMEOUT;
			}
		}
		TACCTL0 &= MC_3;
		char timer = TAR;
		TACCTL0 &= ~CCIFG;
		TACTL |= TACLR;

		if(timer > 40)
			bits[idx] |= (1 << cnt);

		if(cnt == 0){
			cnt = 7;
			idx++;
		}
		else
			cnt--;


	}

	*(_umidade) = bits[0];
	*(_temperatura) = bits[2];

	checksum = bits[0] + bits[2];

	if(checksum != bits[4])
		return CHECKSUM_ERROR;

	return DHT_OK;

}
