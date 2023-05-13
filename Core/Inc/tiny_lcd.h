//
// Created by dazui on 2023/5/6.
//

#ifndef G030_PRESSURE_TINY_LCD_H
#define G030_PRESSURE_TINY_LCD_H

#include "main.h"
#include "i2c.h"

static uint8_t cloud[][8] = {
        {0B00000, 0B00001, 0B00111, 0B01100, 0B01000, 0B01000, 0B00110, 0B00001},
        {0B01111, 0B10000, 0B00000, 0B00000, 0B00000, 0B00000, 0B00000, 0B11111},
        {0B00000, 0B10000, 0B01000, 0B00100, 0B00010, 0B00010, 0B00100, 0B11000},
        {0B00000, 0B00110, 0B01001, 0B10000, 0B00110, 0B01001, 0B10000, 0B00000},
        {0B00000, 0B00011, 0B00100, 0B11000, 0B00011, 0B00100, 0B11000, 0B00000},
        {0B00000, 0B00001, 0B10010, 0B01100, 0B00001, 0B10010, 0B01100, 0B00000},
        {0B00000, 0B10000, 0B01001, 0B00110, 0B10000, 0B01001, 0B00110, 0B00000}
};

static const char* const frame[] = {
        "Prs:      kPa",
        "Up:30 STA:NULL"
};

static const char* const sta_str[] = {"tens ", "relax", "wait."};

/**
 * PCF8574 参数
 */
#define PCF8574_ADDRESS_A111 0x00
#define IIC_DEVICE &hi2c2
#define PCF8574_ADDR (0x40|(PCF8574_ADDRESS_A111<<1))

#define LCD_DELAY_MS    5

// send data or command
#define LCD_COMMAND     ((uint8_t)(0x00))
#define LCD_DATA        ((uint8_t)(0x01))

// commands
#define LCD_CLEARDISPLAY    0x01
#define LCD_RETURNHOME      0x02
#define LCD_ENTRYMODESET    0x04
#define LCD_DISPLAYCONTROL  0x08
#define LCD_CURSORSHIFT     0x10
#define LCD_FUNCTIONSET     0x20
#define LCD_SETCGRAMADDR    0x40
#define LCD_SETDDRAMADDR    0x80

// flags for display entry mode
#define LCD_ENTRY_DEC       0x00
#define LCD_ENTRY_INC       0x02
#define LCD_ENTRYSHIFT_ON   0x01
#define LCD_ENTRYSHIFT_OFF  0x00

// flags for display on/off control
#define LCD_DISPLAYON   0x04
#define LCD_DISPLAYOFF  0x00
#define LCD_CURSORON    0x02
#define LCD_CURSOROFF   0x00
#define LCD_BLINKON     0x01
#define LCD_BLINKOFF    0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT   0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0B00000100  // Enable bit
#define Rw 0B00000010  // Read/Write bit
#define Rs 0B00000001  // Register select bit

typedef enum{
    TENSION = 0x00,
    RELAX   = 0x01,
    WAIT    = 0x02
}muscleStatus;

void lcd_init();
static HAL_StatusTypeDef lcd_fourBit_Write(uint8_t data, uint8_t rs);
void lcd_sendCmd(uint8_t cmd);
void lcd_sendData(uint8_t data);
void lcd_setCursor(uint8_t col, uint8_t row);
void lcd_print(char const str[]);
static void lcd_createChar(uint8_t buff[], uint8_t addr);
uint8_t lcd_createChars(uint8_t buff[][8], uint8_t addr, uint8_t len);
void tiny_lcd_cloud(uint8_t col);
void lcd_print_c(char const str[], uint8_t col, uint8_t row);
void lcd_printf(char *fmt, ...);
uint8_t lcd_print_frame();
void lcd_print_val(double val);
void lcd_display_sta(muscleStatus sta);

#endif //G030_PRESSURE_TINY_LCD_H
