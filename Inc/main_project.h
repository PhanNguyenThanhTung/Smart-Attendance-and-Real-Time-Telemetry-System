#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

#define RCC_BASE        0x40021000
#define RCC_APB2ENR     (*(volatile uint32_t*) (RCC_BASE + 0x18))
#define RCC_APB1ENR     (*(volatile uint32_t*) (RCC_BASE + 0x1C))

#define GPIOA_BASE      0x40010800
#define GPIOA_CRL       (*(volatile uint32_t*) (GPIOA_BASE + 0x00))
#define GPIOA_CRH       (*(volatile uint32_t*) (GPIOA_BASE + 0x04)) 
#define GPIOA_BSRR      (*(volatile uint32_t*) (GPIOA_BASE + 0x10))
#define GPIOA_ODR       (*(volatile uint32_t*) (GPIOA_BASE + 0x0C))

#define SPI1_BASE       0x40013000
#define SPI1_CR1        (*(volatile uint32_t*) (SPI1_BASE + 0x00))
#define SPI1_SR         (*(volatile uint32_t*) (SPI1_BASE + 0x08))
#define SPI1_DR         (*(volatile uint32_t*) (SPI1_BASE + 0x0C))

#define SPI_CR1_MSTR    (1 << 2)
#define SPI_CR1_BR      (1 << 3) // chia xung clock cho 4
#define SPI_CR1_SPE     (1 << 6)
#define SPI_CR1_SSM     (1 << 9)
#define SPI_CR1_SSI     (1 << 8)

#endif