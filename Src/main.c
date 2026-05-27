#include "main.h"
#include "MFRC522.h"
#include "I2C_LCD.h"
#include "UART.h"
#include <stdio.h>

#define SYSTEM_CORE_CLOCK 36000000UL /* chon clock 36MHz */

void SystemClock_Config_36MHz(void);
void delay_ms(uint32_t ms);
uint8_t StringCompare(char *str, char *prefix);

int main(void) {
    uint8_t tagType[2];
    uint8_t uid[5];              /* Mang chua UID */
    char PCresponse[64];         /* Mang chua phan hoi tu PC */

    /* Khoi tao Clock cho he thong */
    SystemClock_Config_36MHz();

    /* Khoi tao cac giao thuc */
    SPI1_Init();
    I2C1_Init();
    UART1_Init();

    delay_ms(50);

    /* Khoi tao cac linh kien */
    MFRC522_Init();
    delay_ms(50);

    LCD_Init();

    /* Bat dau set up voi LCD */
    LCD_Clear();
    LCD_SetCursor(0, 2);
    LCD_Print("Smart Attendance");

    LCD_SetCursor(1, 0);
    LCD_Print("By Thanh Tung");

    LCD_SetCursor(2, 0);
    LCD_Print("Scan the card here");

    LCD_SetCursor(3, 2);
    LCD_Print("Waiting card...");

    UART1_SendString("STM32 Ready\r\n");

    while (1) {
        if(MFRC522_Request(0x26, tagType) == 1) {
            if(MFRC522_Anticoll(uid) == 1) {
                LCD_Clear();
                LCD_SetCursor(0, 0);
                LCD_Print("Card detected");

                LCD_SetCursor(1, 0);
                LCD_Print("UID:");

                LCD_SetCursor(2, 0);
                LCD_PrintUID(uid);

                LCD_SetCursor(3, 0);
                LCD_Print("Sending to PC...");

                /* Gui UID qua PC */
                UART1_SendUID(uid);

                /*
                Doi PC tra loi.
                App Python gui ve:
                 - Known:<Ten>
                 - Unknown
                 - Added:<Ten>
                */
                UART1_ReadLine(PCresponse, sizeof(PCresponse));

                if(StringCompare(PCresponse, "Known:")) {
                    LCD_Clear();

                    LCD_SetCursor(0, 0);
                    LCD_Print("Card registered");

                    LCD_SetCursor(1, 0);
                    LCD_Print("Name: ");

                    /*
                    Vi du PC gui ve "Known:Thanh Tung"
                    PCresponse + 6 se bo qua "Known:"
                    va in ra "Thanh Tung"
                    */
                    LCD_SetCursor(1, 6);
                    LCD_Print(PCresponse + 6);

                    LCD_SetCursor(2, 0);
                    LCD_Print("Attendance OK");
                } else if(StringCompare(PCresponse, "Unknown")) {
                    LCD_Clear();

                    LCD_SetCursor(0, 0);
                    LCD_Print("Not registered");

                    LCD_SetCursor(1, 0);
                    LCD_Print("Register on PC");

                    LCD_SetCursor(2, 0);
                    LCD_Print("Waiting Added...");

                    /*  
                    App PC gui "Unknown" truoc
                    Sau khi nguoi dung bam Register This Card
                    app moi gui tiep "Added:<Ten>" ve STM32
                    */
                    UART1_ReadLine(PCresponse, sizeof(PCresponse));

                    if(StringCompare(PCresponse, "Added:")) {
                        LCD_Clear();

                        LCD_SetCursor(0, 0);
                        LCD_Print("Card added");

                        LCD_SetCursor(1, 0);
                        LCD_Print("Name:");

                        LCD_SetCursor(2, 0);
                        LCD_Print(PCresponse + 6);

                        LCD_SetCursor(3, 0);
                        LCD_Print("Register OK");
                    } else {
                        LCD_Clear();

                        LCD_SetCursor(0, 0);
                        LCD_Print("Add failed");

                        LCD_SetCursor(1, 0);
                        LCD_Print(PCresponse);
                    }
                } else {
                    LCD_Clear();

                    LCD_SetCursor(0, 0);
                    LCD_Print("PC response err");

                    LCD_SetCursor(1, 0);
                    LCD_Print(PCresponse);
                }

                delay_ms(3000);

                LCD_Clear();
                LCD_SetCursor(0, 2);
                LCD_Print("Smart Attendance");

                LCD_SetCursor(1, 0);
                LCD_Print("By Thanh Tung");

                LCD_SetCursor(2, 0);
                LCD_Print("Scan the card here");

                LCD_SetCursor(3, 2);
                LCD_Print("Waiting card...");
            }

            delay_ms(500);
        }

        delay_ms(100);
    }

    return 0;
}

void SystemClock_Config_36MHz(void) {
    /* Bat HSE */
    /* RCC_CR bit 16 = HSEON */
    RCC_CR |= (1 << 16);

    /* Doi HSE san sang */
    /* RCC_CR bit 17 = HSERDY */
    while(!(RCC_CR & (1 << 17)));

    /* Cau hinh Flash latency */
    /* 36MHz can 1 wait state */
    /* FLASH_ACR bit 0-2 = LATENCY */
    /* FLASH_ACR bit 4 = PRFTBE */
    FLASH_ACR &= ~(0x7 << 0);
    FLASH_ACR |=  (0x1 << 0);   /* 1 wait state */
    FLASH_ACR |=  (1 << 4);     /* Enable prefetch buffer */

    /* Cau hinh prescaler */
    /* SYSCLK = 72MHz tu PLL */
    /* HCLK   = SYSCLK / 2 = 36MHz */
    /* APB1   = HCLK / 1   = 36MHz */
    /* APB2   = HCLK / 1   = 36MHz */

    /* Clear HPRE, PPRE1, PPRE2 */
    RCC_CFGR &= ~((0xF << 4) | (0x7 << 8) | (0x7 << 11));

    /* HPRE = 1000: AHB prescaler/2 */
    RCC_CFGR |= (0x8 << 4);

    /* PPRE1 = 000: APB1 prescaler/1 */
    RCC_CFGR |= (0x0 << 8);

    /* PPRE2 = 000: APB2 prescaler/1 */
    RCC_CFGR |= (0x0 << 11);

    /* Cau hinh PLL */
    /* PLLSRC bit 16 = 1: HSE lam nguon PLL */
    /* PLLXTPRE bit 17 = 0: HSE khong chia 2 */
    /* PLLMUL bit 21:18 = 0111: PLL x9 */
    RCC_CFGR &= ~((1 << 16) | (1 << 17) | (0xF << 18));

    /* PLL source = HSE */
    RCC_CFGR |= (1 << 16);

    /* PLL multiplier = x9 */
    RCC_CFGR |= (0x7 << 18);

    /* Bat PLL */
    RCC_CR |= (1 << 24);

    /* Doi PLL ready */
    while (!(RCC_CR & (1 << 25)));

    /* Chon PLL lam SYSCLK */
    RCC_CFGR &= ~(0x3 << 0);
    RCC_CFGR |=  (0x2 << 0);

    /* Doi PLL duoc chon lam system clock */
    while (((RCC_CFGR >> 2) & 0x3) != 0x2);
}

void delay_ms(uint32_t ms) {
    SYSTICK_LOAD = (SYSTEM_CORE_CLOCK / 1000) - 1;

    /* Xoa gia tri hien tai */
    SYSTICK_VAL = 0;

    /* Bat SysTick */
    /* Bit 0 = ENABLE */
    /* Bit 1 = TICKINT, khong dung interrupt nen de 0 */
    /* Bit 2 = CLKSOURCE, 1 = dung clock CPU */
    SYSTICK_CTRL = (1 << 0) | (1 << 2);

    for(uint32_t i = 0; i < ms; i++) {
        /* Cho COUNTFLAG = 1 */
        /* COUNTFLAG nam o bit 16 */
        while(!(SYSTICK_CTRL & (1 << 16)));
    }

    /* Tat SysTick sau khi delay xong */
    SYSTICK_CTRL = 0;
}

uint8_t StringCompare(char *str, char *prefix)
{
    /* So sanh xem chuoi str co bat dau bang prefix hay ko */
    /* Vi du: "Known:Tung" bat dau bang "Known:" thi tra ve 1 */
    while (*prefix)
    {
        if (*str != *prefix)
        {
            return 0; /* Khac 1 ky tu thi ko khop */
        }

        str++;
        prefix++;
    }

    return 1; /* Da so sanh het prefix va deu khop */
}