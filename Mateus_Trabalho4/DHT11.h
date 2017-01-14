/*
 * DHT11.h
 *
 *  Created on: 16 de dez de 2016
 *      Author: menezes
 */

#ifndef DHT11_H_
#define DHT11_H_

typedef enum{TIMEOUT, CHECKSUM_ERROR, DHT_OK}DHT_STATUS;

#define DHPIN BIT3

DHT_STATUS readDht(char *_umidade, char *_temperatura);

#endif /* DHT11_H_ */
