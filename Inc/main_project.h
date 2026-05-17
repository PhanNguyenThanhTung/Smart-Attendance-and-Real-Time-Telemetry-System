#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

//Clock
#define RCC_BASE        0x40021000
#define RCC_APB2ENR     (*(volatile uint32_t*) (RCC_BASE + 0x18))
#define RCC_APB1ENR     (*(volatile uint32_t*) (RCC_BASE + 0x1C))
#define RCC_CR          (*(volatile uint32_t*) (RCC_BASE + 0x00))
#define RCC_CFGR        (*(volatile uint32_t*) (RCC_BASE + 0x04))

#define FLASH_BASE      0x40022000
#define FLASH_ACR       (*(volatile uint32_t*) (FLASH_BASE + 0x00))
//GPIOA
#define GPIOA_BASE      0x40010800
#define GPIOA_CRL       (*(volatile uint32_t*) (GPIOA_BASE + 0x00))
#define GPIOA_CRH       (*(volatile uint32_t*) (GPIOA_BASE + 0x04)) 
#define GPIOA_BSRR      (*(volatile uint32_t*) (GPIOA_BASE + 0x10))
#define GPIOA_ODR       (*(volatile uint32_t*) (GPIOA_BASE + 0x0C))

//SPI
#define SPI1_BASE       0x40013000
#define SPI1_CR1        (*(volatile uint32_t*) (SPI1_BASE + 0x00))
#define SPI1_SR         (*(volatile uint32_t*) (SPI1_BASE + 0x08))
#define SPI1_DR         (*(volatile uint32_t*) (SPI1_BASE + 0x0C))

//SPI config
#define SPI_CR1_MSTR    (1 << 2)
#define SPI_CR1_BR      (1 << 3) // chia xung clock cho 4
#define SPI_CR1_SPE     (1 << 6)
#define SPI_CR1_SSM     (1 << 9)
#define SPI_CR1_SSI     (1 << 8)

//Systick
#define SYSTICK_BASE    0xE000E010 //Table 33/Muc 4.1 About the STM32 core peripherals/Tai lieu PM0056 Programming manual
#define SYSTICK_CTRL    (*(volatile uint32_t*)(SYSTICK_BASE + 0x00))
#define SYSTICK_LOAD    (*(volatile uint32_t*)(SYSTICK_BASE + 0x04))
#define SYSTICK_VAL     (*(volatile uint32_t*)(SYSTICK_BASE + 0x08))
#define SYSTICK_CALIB   (*(volatile uint32_t*)(SYSTICK_BASE + 0x0C))

#endif