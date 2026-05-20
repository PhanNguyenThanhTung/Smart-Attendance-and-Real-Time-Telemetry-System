#include<I2C_LCD.h>

void I2C_Init(void) {
    RCC_APB1ENR |= (1 << 21);
    RCC_APB2ENR |= ((1 << 0) | (1 << 3));

    GPIOA_CRL &= ~((0xF << 24) | (0xF << 28)); //Reset chan PB6 va PB7
    GPIOA_CRL |= ((0xF << 24) | (0xF << 28)); //0xF = 1111 Output 50MHz Alternate Function Open-drain
    
    I2C1_CR1 &= ~(1 << 15); //Xoa cac biet ve 0
    I2C1_CR1 |= (1 << 15); //Under Reset

    I2C1_CR2 = 36; //Frequency APB1 = 36MHz
    I2C1_CCR = 180; //CCR = 36MHz / (2 * 100kHz) = 180;

    I2C1_TRISE = 37; // 36 + 1 = 37
    I2C1_CR1 |= (1 << 0); //Enable Peripheral (Bat I2C)
}

uint8_t I2C_WriteByte(uint8_t addr, uint8_t data) {
    
}