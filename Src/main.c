#include "main_project.h"
#include <stdio.h>

#define SYSTEM_CORE_CLOCK 36000000UL // chon clock 36MHz

int main(void) {

    while (1) {

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

    // HPRE = 1000: AHB prescaler /2
    RCC_CFGR |= (0x8 << 4);

    // PPRE1 = 000: APB1 prescaler /1
    RCC_CFGR |= (0x0 << 8);

    // PPRE2 = 000: APB2 prescaler /1
    RCC_CFGR |= (0x0 << 11);

    // 5. Cau hinh PLL
    // PLLSRC bit 16 = 1: HSE lam nguon PLL
    // PLLXTPRE bit 17 = 0: HSE khong chia 2
    // PLLMUL bit 21:18 = 0111: PLL x9

    RCC_CFGR &= ~((1 << 16) | (1 << 17) | (0xF << 18));

    // PLL source = HSE
    RCC_CFGR |= (1 << 16);

    // PLL multiplier = x9
    RCC_CFGR |= (0x7 << 18);

    // 6. Bat PLL
    RCC_CR |= (1 << 24);

    // 7. Doi PLL ready
    while (!(RCC_CR & (1 << 25)));

    // 8. Chon PLL lam SYSCLK
    RCC_CFGR &= ~(0x3 << 0);
    RCC_CFGR |=  (0x2 << 0);

    // 9. Doi PLL duoc chon lam system clock
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
    while (!(SPI1_SR & (1 << 1)));// neu TX dang ban thi doi cho den khi TX het ban
    SPI1_DR = data; // Ghi du lieuj vo thanh ghi
    while (!(SPI1_SR & (1 << 0)));// doi cho den khi RX nhan duoc du lieu (dang ban) xong r moi tiep tuc
    //Trả về dữ liệu nhận được
    return (uint8_t)SPI1_DR;
}

void MFRC522_Write(uint8_t addr, uint8_t val) {
    //16 bit cao la bit Reset con 16 bit thap la bit Set (khi nhap 1 vao thanh ghi tuong ung se cho ra Set/Reset)
    GPIOA_BSRR = (1 << 20); // 20 la chan PA4 o muc thap. Keo SS xuong de chon (do thanh ghi BSRR chi nhan bit 1 khong nhan bit 0)
    SPI1_Transfer((addr << 1) & 0x7E); //dich trai 1 bit vi giao tiep SPI se truyen 1 byte du lieu (trong do LSB luon = 0; bit thu 7 [MSB] : Read(1)/Write(0); bit [1-6]: la dia chi can truyen toi)
    SPI1_Transfer(val);
    GPIOA_BSRR = (1 << 4);  // Keo SS len 
}

uint8_t MFRC522_Read(uint8_t addr) {
    uint8_t val;
    GPIOA_BSRR = (1 << 20); // SS xuong thap 

    SPI1_Transfer(((addr << 1) & 0x7E) | 0x80); // Ox7E la de dam bao LSB luon = 0, sau do se setting cho MSB sau

    val = SPI1_Transfer(0x00); //gui bit gia de day du lieu ve stm32
    GPIOA_BSRR = (1 << 4);
    return val;
}

uint8_t MFRC522_ToCard(uint8_t cmd, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint8_t *backLen) {
    uint8_t status = 0;
    uint8_t irqEn = 0x77;
    uint8_t waitIRq = 0x30;

    MFRC522_Write(0x02, irqEn | 0x80); //0x02 la thanh ghi ComlEnReg
    MFRC522_Write(0x01, 0x00);         //dua ve trang thai ranh
    MFRC522_Write(0x0A, 0x80);         //FIFOLEvelReg: Flush FIFO(xoa buffer)

    for(int i = 0; i < sendLen; i++) {
        MFRC522_Write(0x09, sendData[i]);
    }

    MFRC522_Write(0x01, cmd);

    if(cmd == 0x0C) { // 0x0C = 0000 1100 (Transceive: truyen du lieu tu FIFO buffer toi anten va tu dong kich hoat thu du lieu sau khi truyen)
        uint8_t tmp = MFRC522_Read(0x0D); // 0x0D BitFramingReg (MSB = 1: bat dau truyen data neu len Transceive duoc bat)
        MFRC522_Write(0x0D, tmp | 0x80);  // lay du lieu chuan bi duoc truyen di sau do bat bit StartSend len 1 de kick hoat
    }

    uint32_t i = 2000; //Time out doi RC522 truyen du lieu di
    uint32_t n;
    do {
        n = MFRC522_Read(0x04); // lien tuc doc gia tri thanh ghi interrupt
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

    if(i != 0 && !(MFRC522_Read(0x06) & 0x1B)) { //Neu khong phat hien loi (= 1)
        status = 1;

        if (backData && backLen) {
            n = MFRC522_Read(0x0A); //  n la so byte co trong FIFO buffer
            *backLen = n;
            for(int j = 0; j < n; j++) {
                backData[j] = MFRC522_Read(0x09);// doc tung byte
            } 
        }
    }
    return status;
}

uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType)
{
    uint8_t status;
    uint8_t backLen;
    // 0x0D = BitFramingReg
    // TxLastBits = 7 vi REQA/WUPA chi gui 7 bit
    MFRC522_Write(0x0D, 0x07);
    // TagType[0] = reqMode; /* Bo cmt dong nay neu dung TagType ben trong ham status = MFRC522_ToCard(...) */
    // 0x0C = Transceive
    status = MFRC522_ToCard(0x0C, &reqMode, 1, TagType, &backLen);
    // Neu thanh cong, the se tra ve 2 byte ATQA
    if ((status != 1) || (backLen != 2)) {
        status = 0;
    }
    return status;
}

uint8_t MFRC522_Anticoll(uint8_t *serNum) {
    uint8_t status;
    uint8_t backLen;
    uint8_t serNumCheck = 0;

    MFRC522_Write(0x0D, 0x00); // gui tron 1 byte ra FIFO buffer
    // theo datasheet the s50 MIFARE cua NXP phan 9.1 0x93 va 0x20 la Anticollision CL1 phai di chung 1 mang 
    serNum[0] = 0x93; // Select command: 0x09 (se phat ra lenh chuan bi nhan du lieu va thu thap 4 byte dau tien trong chuoi UID cua the) 
    serNum[1] = 0x20; // NVV(Number of Valid Bits): 4 bit cao la so luong Byte dc gui di, 4 bit thap la so Bit le dc gui di

    //0x0C la lenh Transceive
    //gui 2 byte: 0x93 va 0x20 di
    //Nhan ve 5 byte: UID: 0, 1, 2, 3, BCC (BCC la byte kiem tra loi)
    status = MFRC522_ToCard(0x0C, serNum, 2, serNum, &backLen);

    if(status == 1) { // neu khong co loi xay ra
        //Kiet tra so byte tra ve
        if(backLen != 5) { // neu chua du 5 byte
            status = 0;
        } else { // neu du 5 byte thi tiep tuc
            //BCC = UID0 ^ UID1 ^ UID 2 ^ UID3 (^ la XOR), BCC se kiem tra loi dua tren XOR cac byte UID
            serNumCheck = serNum[0] ^ serNum[1] ^ serNum[2] ^ serNum[3];
            
            //Kiem tra bien check voi BCC xem co giong nhau khong, BCC la serNum[4]
            if(serNumCheck != serNum[4]) {
                status = 0;
            }
        }
    }
    return status;
}

void MFRC522_AntenOn(void) {
    uint8_t temp;
    //kiem tra thanh ghi TxControlReg(0x14)
    temp = MFRC522_Read(0x14);

    if(!(temp & 0x03)) {
        MFRC522_Write(0x14, temp | 0x03);
    }
}

void MFRC522_AntenOff(void) {
    uint8_t temp;

    temp = MFRC522_Read(0x14);

    MFRC522_Write(0x14, temp |= ~(0x03)); // chi thay doi bit 0 va 1 ko lam thay doi cac bit con lai
}

void MFRC522_Reset(void) {
    MFRC522_Write(0x01, 0x0F); // 0x0F = 0000 1111 SoftReset command

}
