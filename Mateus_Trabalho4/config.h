
#ifndef CONFIG_H_
#define CONFIG_H_

#define LED_VERMELHO BIT0   // Pin P1.0
#define LED_VERDE BIT6      // Pin P1.6
#define RXD BIT1            // Pin P1.1
#define TXD BIT2            // Pin P1.2
#define DE BIT3				// Pin P1.3
#define RE BIT4				// Pin P1.4


void configureClock(void);
void initializePORT1(void);
void initializeTimerA0(void);


#endif /* CONFIG_H_ */
