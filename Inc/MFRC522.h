#ifndef MFRC522_H
#define MFRC522_H

#include "main.h"
#include <stdint.h>

void SPI1_Init(void);
uint8_t SPI1_Transfer(uint8_t data);
void MFRC522_Write(uint8_t addr, uint8_t val);
uint8_t MFRC522_Read(uint8_t addr);
uint8_t MFRC522_ToCard(uint8_t cmd, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen);
uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType);
uint8_t MFRC522_Anticoll(uint8_t *serNum);
void MFRC522_AntenOn(void);
void MFRC522_AntenOff(void);
void MFRC522_Reset(void);
void MFRC522_Init(void);

/* Ham delay nam ben main.c, MFRC522.c can dung khi reset/init */
void delay_ms(uint32_t ms);

#endif
