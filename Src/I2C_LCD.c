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
    uint32_t timeout;
    timeout = 100000;
    //doi bus het ban, Status_Register_2 bit 1 la Bus Busy, 1 la co 0 la ko
    while((I2C1_SR2 & (1 << 1)) && timeout--);
    if(timeout == 0) {
        return 0;
    }
    //Bat dau I2C
    I2C1_CR1 |= (1 << 8); //bit 8 trong CR1 la Start
    //Doi SB (Start Bit) thanh ghi SR1
    timeout = 100000;
    while(!(I2C1_SR1 & (1 << 0)) && timeout--);
    if(timeout == 0) {
        return 0;
    }
    //Gui dia chi Slave + Write bit 
    I2C1_DR = (addr << 1); //Dia chi gui di gom (A6 A5 A4 A3 A2 A1 | Write/Read bit) 0 la Write, 1 la Read
    //Doi Address
    timeout = 100000;
    while(!(I2C1_SR1 & (1 << 1)) && timeout--); //o che do Master bit 1 thanh ghi SR1 neu 0 la dang truyen dia chi qua cho Slave con 1 la da truyen xong
    if(timeout == 0) {
        I2C1_CR1 |= (1 << 9); //bat Stop bit
        return 0;
    }
    //Doc theo thu tu cua 2 thanh ghi de clear co ADDR
    (void)I2C1_SR1;
    (void)I2C1_SR2;
    //Doi co TXE: Transmit Emty 
    timeout = 100000;
    while (!(I2C1_SR1 & (1 << 7)) && timeout--);
    if(timeout == 0) {
        I2C1_CR1 |= (1 << 9); //bat Stop Bit
        return 0;
    }
    //Gui data
    I2C1_DR = data;
    //Doi BTF: Byte transfer finished, 0: chua xong | 1: da xong
    timeout = 100000;
    while(!(I2C1_SR1 & (1 << 2)) && timeout--);
    if(timeout == 0) {
        I2C1_CR1 |= (1 << 9);
        return 0;
    }
    //Stop
    I2C1_CR1 |= (1 << 9);

    return 1;
}

