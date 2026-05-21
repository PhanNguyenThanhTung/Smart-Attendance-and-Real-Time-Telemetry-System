#include "main.h"
#include "MFRC522.h"
#include "I2C_LCD.h"
#include <stdio.h>

#define SYSTEM_CORE_CLOCK 36000000UL // chon clock 36MHz

void SystemClock_Config_36MHz(void);
void delay_ms(uint32_t ms);

volatile uint8_t g_version = 0;
volatile uint8_t g_tx_control = 0;
volatile uint8_t g_error_reg = 0;
volatile uint8_t g_irq_reg = 0;
volatile uint8_t g_fifo_level = 0;
volatile uint8_t g_card_found = 0;
volatile uint8_t g_uid_ok = 0;
volatile uint8_t g_tagType[2] = {0};
volatile uint8_t g_uid[5] = {0};
volatile uint32_t g_loop_count = 0;

volatile uint8_t g_ever_card_found = 0;
volatile uint8_t g_ever_uid_ok = 0;
volatile uint8_t g_last_request_status = 0;
volatile uint8_t g_last_anticoll_status = 0;

int main(void) {
    // uint8_t tagType[2];
    // uint8_t uid[5];

    // SystemClock_Config_36MHz();
    // SPI1_Init();
    I2C1_Init();
    // delay_ms(50);
    // MFRC522_Init();
    // delay_ms(50);
    // // 0x37 = VersionReg
    // // Neu SPI va RC522 hoat dong dung, gia tri thuong la 0x91 hoac 0x92
    // g_version = MFRC522_Read(0x37);
    // g_tx_control = MFRC522_Read(0x14);

    while (1) {
        // g_loop_count++;
        // g_tx_control = MFRC522_Read(0x14); // TxControlReg
        // g_error_reg  = MFRC522_Read(0x06); // ErrorReg
        // g_irq_reg    = MFRC522_Read(0x04); // CommIrqReg
        // g_fifo_level = MFRC522_Read(0x0A); // FIFOLevelReg

        // g_last_request_status = MFRC522_Request(0x26, tagType);

        // if (g_last_request_status == 1) {
        //     g_card_found = 1;
        //     g_ever_card_found = 1;
        //     g_tagType[0] = tagType[0];
        //     g_tagType[1] = tagType[1];

        //     g_last_anticoll_status = MFRC522_Anticoll(uid);

        //     if (g_last_anticoll_status == 1) {
        //         g_uid_ok = 1;
        //         g_ever_uid_ok = 1;
        //         g_uid[0] = uid[0];
        //         g_uid[1] = uid[1];
        //         g_uid[2] = uid[2];
        //         g_uid[3] = uid[3];
        //         g_uid[4] = uid[4];
        //     }
        //     delay_ms(500);
        // }
        // delay_ms(100);
        I2C1_WriteByte(0x27, 0x08);
        delay_ms(1000);

        I2C1_WriteByte(0x27, 0x00);
        delay_ms(1000);

    }
    return 0;
}

void SystemClock_Config_36MHz(void) {
    // Bat HSE
    // RCC_CR bit 16 = HSEON
    RCC_CR |= (1 << 16);
    //Doi HSE san sang
    // RCC_CR bit 17 = HSERDY
    while (!(RCC_CR & (1 << 17)));
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

void delay_ms(uint32_t ms)
{
    SYSTICK_LOAD = (SYSTEM_CORE_CLOCK / 1000) - 1;
    // Xoa gia tri hien tai
    SYSTICK_VAL = 0;
    // Bat SysTick
    // Bit 0 = ENABLE
    // Bit 1 = TICKINT, khong dung interrupt nen de 0
    // Bit 2 = CLKSOURCE, 1 = dung clock CPU
    SYSTICK_CTRL = (1 << 0) | (1 << 2);

    for (uint32_t i = 0; i < ms; i++)
    {
        // Cho COUNTFLAG = 1
        // COUNTFLAG nam o bit 16
        while (!(SYSTICK_CTRL & (1 << 16)));
    }

    // Tat SysTick sau khi delay xong
    SYSTICK_CTRL = 0;
}
