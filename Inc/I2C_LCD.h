#ifndef I2C_LCD_H
#define I2C_LCD_H
#include "main.h"
#include <stdint.h>

#define LCD_I2C_ADDR 0x27 /* co the la 0x3F neu la phien ban moi */

void I2C1_Init(void);
uint8_t I2C1_WriteByte(uint8_t addr, uint8_t data);

void LCD_Init(void);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_SendCmd(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_Print(char *str);
void LCD_PrintUID(uint8_t *uid);

void delay_ms(uint32_t ms);

#endif