#include "UART.h"

void UART1_Init(void) {
    RCC_APB2ENR |= (1 << 2);
    RCC_APB2ENR |= (1 << 14);

    /* PA9 = TX, Alternate function push-pull, 50MHz */
    GPIOA_CRH &= ~(0xF << 4);
    GPIOA_CRH |=  (0xB << 4);

    /* PA10 = RX, input floating */
    GPIOA_CRH &= ~(0xF << 8);
    GPIOA_CRH |=  (0x4 << 8);

    /* PCLK2 = 36MHz, baudrate = 9600 */
    USART1_BRR = 0xEA6;

    /* UE = bit 13, TE = bit 3, RE = bit 2 */
    USART1_CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
}

void UART1_SendChar(char x) {
    /* TXE bit 7 = 1 nghia la data register trong, co the gui byte moi */
    while (!(USART1_SR & (1 << 7)));

    /* Ghi du lieu vao thanh ghi Data */
    USART1_DR = x;
}

void UART1_SendString(char *str) {
    while (*str) {
        UART1_SendChar(*str);
        str++;
    }
}

static char HextoChar_UART(uint8_t value) {
    value &= 0x0F;

    /* Giai thich: giong ham HextoChar ben phan LCD */
    if (value < 10) {
        return '0' + value;
    }

    return 'A' + (value - 10);
}

void UART1_SendHexByte(uint8_t data) {
    /* Tach rieng 4 bit de chuyen doi nhu phan LCD */
    UART1_SendChar(HextoChar_UART(data >> 4));
    UART1_SendChar(HextoChar_UART(data));
}

void UART1_SendUID(uint8_t *uid) {
    UART1_SendString("CARD:");
    /* Gui 4 byte UID sang UART */
    UART1_SendHexByte(uid[0]);
    UART1_SendHexByte(uid[1]);
    UART1_SendHexByte(uid[2]);
    UART1_SendHexByte(uid[3]);
    /* Dua con tro ve dau dong \r va  xuong dong \n */
    UART1_SendString("\r\n");
}