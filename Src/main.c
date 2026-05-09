#include "main_project.h"
#include <stdio.h>

int main(void) {

    while (1) {

    }

    return 0;
}

void SPI1_Init(void) {
    RCC_APB2ENR |= (1 << 12) | (1 << 2); // SPI1 va GIPOA
    GPIOA_CRL &= ~((0xF << 16) |  (0xF << 0)); 
    GPIOA_CRL |=  ((0x1 << 16) | (0x1 << 0)); // Chan PA0(Reset) va PA4(SS) la Output push pull

    GPIOA_CRL &= ~((0xF << 20) | (0xF << 28)); 
    GPIOA_CRL |=  ((0x9 << 20) | (0x9 << 28)); // Chan PA5(SCK) va PA7(MOSI) la  Alternate function output Push-pull

    GPIOA_CRL &= ~(0xF << 24);
    GPIOA_CRL |=  (0x4 << 24); // Chan PA6 (MISO) la Input Floating

    //Caus Hinh SPI1_CR1
    SPI1_CR1 = 0;
    SPI1_CR1 |= SPI_CR1_MSTR;
    SPI1_CR1 |= SPI_CR1_BR; // Baud rate, RC522 chiu dc 10MHz
    SPI1_CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;
    
    SPI1_CR1 |= SPI_CR1_SPE;

    GPIOA_BSRR = (1 << 4); // Chan SS len 1 (chua chon)
}

uint8_t SPI1_Transfer(uint8_t data) {
    while (!(SPI1_SR & (1 << 1)));
    //Ghi dữ liệu vào thanh ghi DR
    SPI1_DR = data;
    //Chờ cho đến khi nhận được dữ liệu về
    while (!(SPI1_SR & (1 << 0)));
    //Trả về dữ liệu nhận được
    return (uint8_t)SPI1_DR;
}

void MFRC522_Write(uint8_t addr, uint8_t val) {
    //16 bit cao la bit Reset con 16 bit thap la bit Set (khi nhap 1 vao thanh ghi tuong ung se cho ra Set/Reset)
    GPIOA_BSRR = (1 << 20); // 20 la chan PA4 o muc thap. Keo SS xuong de chon (do thanh ghi BSRR chi nhan bit 1 khong nhan bit 0)
    SPI1_Transfer((addr << 1) & 0x7E);
    SPI1_Transfer(val);
    GPIOA_BSRR = (1 << 4);  // Keo SS len 
}

void MFRC522_Read(uint8_t addr) {
    uint8_t val;
    GPIOA_BSRR = (1 << 20); // SS xuong thap 

    SPI1_Transfer(((addr << 1) & 0x7E) | 0x80);
}
