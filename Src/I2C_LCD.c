#include "I2C_LCD.h"

#define LCD_RS          0x01
#define LCD_RW          0x02
#define LCD_ENABLE      0x04
#define LCD_BACKLIGHT   0x08

static uint8_t lcd_backlight = LCD_BACKLIGHT;

void I2C1_Init(void) {
    RCC_APB1ENR |= (1 << 21);
    RCC_APB2ENR |= ((1 << 0) | (1 << 3));

    GPIOB_CRL &= ~((0xF << 24) | (0xF << 28)); //Reset chan PB6 va PB7
    GPIOB_CRL |= ((0xF << 24) | (0xF << 28)); //0xF = 1111 Output 50MHz Alternate Function Open-drain
    
    I2C1_CR1 |= (1 << 15); //Under Reset
    I2C1_CR1 &= ~(1 << 15); //Thoat khoi Reset

    I2C1_CR2 = 36; //Frequency APB1 = 36MHz
    I2C1_CCR = 180; //CCR = 36MHz / (2 * 100kHz) = 180;

    I2C1_TRISE = 37; // 36 + 1 = 37
    I2C1_CR1 |= (1 << 0); //Enable Peripheral (Bat I2C)
}

uint8_t I2C1_WriteByte(uint8_t addr, uint8_t data) {
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

static void LCD_Write4Bits(uint8_t data) {
    I2C1_WriteByte(LCD_I2C_ADDR, data | lcd_backlight); //Gui data ra lcd

    I2C1_WriteByte(LCD_I2C_ADDR, data | LCD_ENABLE | lcd_backlight); //Bat chan EN len de lcd nhan data
    delay_ms(1);

    I2C1_WriteByte(LCD_I2C_ADDR, (data & ~LCD_ENABLE) | lcd_backlight); //Tat chan EN de lcd chot data
    delay_ms(1);
}

static void LCD_Send(uint8_t value, uint8_t mode) {
    //Ghi gia tri vao LCD bang cach dieu khien 4 chan (D4-D7) thay vi ca 8 chan
    uint8_t highNibble = value & 0xF0; //Ghi gia tri vao 4 chan cao cua LCD (D4-D7)
    uint8_t lowNibble  = (value << 4) & 0xF0; //Sau do dich phai 4 bit de ghi gia tri con lai vao chan (D4-D7)
    //Vi du ghi ky tu "A" = 0x41 thi:
        //highNibble = 0x40 (0100_0001 & 1111_0000 = 0100_0000)
        //lowNibble = 0x10 (0100_0001 << 4 = 0001_0000 & 1111_0000 = 0001_0000)
    LCD_Write4Bits(highNibble | mode);
    LCD_Write4Bits(lowNibble | mode);//mode o day la chon cach gui la cmd hay data
}

void LCD_SendCmd(uint8_t cmd) {
    LCD_Send(cmd, 0); //RS = 0 gui cmd
}

void LCD_SendData(uint8_t data) {
    LCD_Send(data, LCD_RS); //RS = 1 gui data
}

void LCD_Init(void) {
    delay_ms(100);
    //gui 3 lan mode 8-bit (0x30) de reset cho LCD on dinh khi moi khoi dong
    LCD_Write4Bits(0x30);
    delay_ms(10);
    LCD_Write4Bits(0x30);
    delay_ms(10);
    LCD_Write4Bits(0x30);
    delay_ms(10);

    /*gui lenh yeu cau chuyen sang mode 4-bit (0x20): 
        - gui 2 lan voi moi lan 4-bit
        - 4 bit HighNibble va 4 bit LowNibble
    */
    LCD_Write4Bits(0x20);
    delay_ms(10);

    LCD_SendCmd(0x28); //4-bit, multi-line, 5x8 font
    delay_ms(2);

    LCD_SendCmd(0x08); //Display OFF
    delay_ms(2);

    LCD_SendCmd(0x01); //Clear display
    delay_ms(5);

    LCD_SendCmd(0x06); //Entry mode
    delay_ms(2);

    LCD_SendCmd(0x0C); //Display ON, cursor OFF
    delay_ms(2);
}

void LCD_Clear(void) {
    LCD_SendCmd(0x01); //Clear man hinh
    delay_ms(5);
}


void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};

    if(row > 3) row = 3;
    if(col > 19) col = 19;

    LCD_SendCmd(0x80 | (row_offsets[row] + col));
}

void LCD_Print(char *str) {
    while(*str) {
        LCD_SendData((uint8_t)*str);
        str++;
    }
}

static char HextoChar(uint8_t value) { //Ham nay se bien doi gia tri Hexa sang Char
    value &= 0x0F; //Kiem tra va dua gia tri ve dung he Hex(Thap luc phan/16)

    if(value < 10) { //Neu gia tri nam trong khoang 0-9
        return '0' + value; //Tra ve ky tu so;
    }
    //Neu gia tri la phan chu (A-F)
    return 'A' + (value - 10);
}

void LCD_PrintUID(uint8_t *uid) {
    char uidStr[12]; //Gom 8 ky tu UID, 3 ky tu ' ', 1 ky tu '\0'
    /*Giai thich vi sao phai dich phai 4 bit:
        - Vi ham HextoChar se chuyen tung gia tri nhung UID RFID theo cap (8 bit cho 2 ky tu)
        - Vi du: neu giu nguyen cap 5A = 0101 1010 khi vao dong (value &= 0x0F) thi se mat luon so 5 = 0000 1010
        - Nen phai tach no ra tung ky tu rieng le bang cach dich phai 4 bit de chuyen doi 
    */
    
    //UID[0]
    uidStr[0] = HextoChar(uid[0] >> 4); //Dich 4 bit cao ve 4 bit thap de chuyen doi
    uidStr[1] = HextoChar(uid[0]);
    uidStr[2] = ' ';
    //UID[1]
    uidStr[3] = HextoChar(uid[1] >> 4);
    uidStr[4] = HextoChar(uid[1]);
    uidStr[5] = ' ';
    //UID[2]
    uidStr[6] = HextoChar(uid[2] >> 4);
    uidStr[7] = HextoChar(uid[2]);
    uidStr[8] = ' ';
    //UID[3]
    uidStr[9] = HextoChar(uid[3] >> 4);
    uidStr[10] = HextoChar(uid[3]);
    uidStr[11] = '\0';
    //UID[4] = BCC (Khong can hien thi len LCD vi la byte kiem tra loi)

    LCD_Print(uidStr);
}