
/* ------------------------------Descriçao----------------------------
 *  
 * Esse codigo tem como objetivo solicitar ao MSP430G2553 o valor da
 * temperatura obtida atraves da conversao analogica/digital do ADC
 * conectado ao seu sensor interno de temperatura, calcular a media 
 * referente a 8 amostras de tempetarura e mostrar o valor medio via
 * terminal no computador.
 * 
*/

#include <SoftwareSerial.h>
#include "TimerOne.h"
 
SoftwareSerial mspSerial(10, 11); //Cria UART por software utilizando os pinos RX e TX respectivamente

char i = 0;                       //Indexador do vetor data[]
char numSamples = 0;              //Armazena quantidade de amostras recebidas
int data[2] = {0, 0};             //Armazena os bytes LSB e MSB recebidos pela serial
unsigned int sensorTemperature;   //Armazena o valor resultante dos bytes LSB e MSB
float tempCelsius;                //Armazena a soma dos valores de temperatura
float avgTempCelsius;             //Armazena o valor médio da temperatura

void transmit(void);    //Solicita dados da serial

void setup() {
  
    Serial.begin(9600);   //Inicializa porta Serial conectada ao USB com 9600 Baud Rate
    while (!Serial);      //Esperando a conexão da porta Serial
    
    mspSerial.begin(9600);    //Inicializa UART por software com 9600 de Baud Rate     
  
    Timer1.initialize(100000);        //Inicializa Timer1 com periodo de = 100ms    
    Timer1.attachInterrupt(transmit); //Endereço da rotina de interrupçao do Timer1

}

void loop() {

    if(mspSerial.available())         //Se dado disponível na Serial
        data[i++] = mspSerial.read();   //Recebe primeiro LSB
  
    if(i == 2){
        i = 0;
        sensorTemperature = 0;
        sensorTemperature = (data[1] << 8) | data[0];                   //Transforma LSB e MSB num dado do tipo inteiro
        tempCelsius += ((1.5*sensorTemperature/1023) - 0.986)/0.00355;  //Calculo do valor da temperatura do sensor interno do MSP430G2553
        numSamples++;
        if(numSamples == 8){
            numSamples = 0;
            avgTempCelsius = tempCelsius / 8.0;
            tempCelsius = 0;
            Serial.print("Temperatura: ");
            Serial.print(avgTempCelsius);
            Serial.println("°C"); 
        }
    }
}

void transmit(){
  
    mspSerial.write('a');   //Caracter de solicitaçao de dado do MSP430
}

