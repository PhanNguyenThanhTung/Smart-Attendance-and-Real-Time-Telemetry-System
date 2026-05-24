#include "main.h"
#include "MFRC522.h"
#include "I2C_LCD.h"
#include "UART.h"
#include <stdio.h>

#define SYSTEM_CORE_CLOCK 36000000UL // chon clock 36MHz

void SystemClock_Config_36MHz(void);
void delay_ms(uint32_t ms);

int main(void) {
    uint8_t tagType[2];
    uint8_t uid[5];

    SystemClock_Config_36MHz();
    //Khoir tao cac giao thuc
    SPI1_Init();
    I2C1_Init();
    UART1_Init();
    delay_ms(50);
    //Khoi tao cac linh kien
    MFRC522_Init();
    delay_ms(50);
    LCD_Init();

    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_Print("Smart Attendance");

    LCD_SetCursor(1, 0);
    LCD_Print("Waiting card...");

    while (1) {
        if(MFRC522_Request(0x26, tagType) == 1)
        {
            if(MFRC522_Anticoll(uid) == 1)
            {
                UART1_SendUID(uid);

                LCD_Clear();

                LCD_SetCursor(0, 0);
                LCD_Print("Card detected");

                LCD_SetCursor(1, 0);
                LCD_Print("UID:");

                LCD_SetCursor(2, 0);
                LCD_PrintUID(uid);

                LCD_SetCursor(3, 0);
                LCD_Print("Sent to PC");

                delay_ms(1500);

                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_Print("Smart Attendance");

                LCD_SetCursor(1, 0);
                LCD_Print("Waiting card...");
            }
        }

        delay_ms(100);
    }

    return 0;
}

void SystemClock_Config_36MHz(void) {
    // Bat HSE
    // RCC_CR bit 16 = HSEON
    RCC_CR |= (1 << 16);
    //Doi HSE san sang
    // RCC_CR bit 17 = HSERDY
    while(!(RCC_CR & (1 << 17)));
    // Cau hinh Flash latency
    // 36MHz can 1 wait state
    // FLASH_ACR bit 0-2 = LATENCY
    // FLASH_ACR bit 4 = PRFTBE
    FLASH_ACR &= ~(0x7 << 0);
    FLASH_ACR |=  (0x1 << 0);   // 1 wait state
    FLASH_ACR |=  (1 << 4);     // Enable prefetch buffer
    // Cau hinh prescaler
    // SYSCLK = 72MHz tu PLL
    // HCLK   = SYSCLK / 2 = 36MHz
    // APB1   = HCLK / 1   = 36MHz
    // APB2   = HCLK / 1   = 36MHz
    // Clear HPRE, PPRE1, PPRE2
    RCC_CFGR &= ~((0xF << 4) | (0x7 << 8) | (0x7 << 11));
    // HPRE = 1000: AHB prescaler/2
    RCC_CFGR |= (0x8 << 4);
    // PPRE1 = 000: APB1 prescaler/1
    RCC_CFGR |= (0x0 << 8);
    // PPRE2 = 000: APB2 prescaler/1
    RCC_CFGR |= (0x0 << 11);
    // Cau hinh PLL
    // PLLSRC bit 16 = 1: HSE lam nguon PLL
    // PLLXTPRE bit 17 = 0: HSE khong chia 2
    // PLLMUL bit 21:18 = 0111: PLL x9
    RCC_CFGR &= ~((1 << 16) | (1 << 17) | (0xF << 18));
    // PLL source = HSE
    RCC_CFGR |= (1 << 16);
    // PLL multiplier = x9
    RCC_CFGR |= (0x7 << 18);
    // Bat PLL
    RCC_CR |= (1 << 24);
    // Doi PLL ready
    while (!(RCC_CR & (1 << 25)));
    // Chon PLL lam SYSCLK
    RCC_CFGR &= ~(0x3 << 0);
    RCC_CFGR |=  (0x2 << 0);

    // Doi PLL duoc chon lam system clock
    while (((RCC_CFGR >> 2) & 0x3) != 0x2);
}

void delay_ms(uint32_t ms) {
    SYSTICK_LOAD = (SYSTEM_CORE_CLOCK / 1000) - 1;
    // Xoa gia tri hien tai
    SYSTICK_VAL = 0;
    // Bat SysTick
    // Bit 0 = ENABLE
    // Bit 1 = TICKINT, khong dung interrupt nen de 0
    // Bit 2 = CLKSOURCE, 1 = dung clock CPU
    SYSTICK_CTRL = (1 << 0) | (1 << 2);

    for(uint32_t i = 0; i < ms; i++) {
        // Cho COUNTFLAG = 1
        // COUNTFLAG nam o bit 16
        while(!(SYSTICK_CTRL & (1 << 16)));
    }

    // Tat SysTick sau khi delay xong
    SYSTICK_CTRL = 0;
}
