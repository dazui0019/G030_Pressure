//
// Created by dazui on 2023/5/6.
//

#include <string.h>
#include "tiny_lcd.h"
#include "printf.h"
#include "pressure.h"

uint8_t backlightval = LCD_BACKLIGHT;

/**
 * @brief 采用4bit模式向LCD发送数据以及命令
 * @param data[in]
 * @param rs[in] R/S引脚: rs = 1时发送数据；rs = 0时发送指令。
 * @return
 */
static HAL_StatusTypeDef lcd_fourBit_Write(uint8_t data, uint8_t rs){
    /**
     * LCD使用4Bit模式，所以要分两次发送，并且LCD的DB4~DB7连着PCF8574的P4~P7
     * 所以将 DB4~DB7 这四个端口的数据放在要发送数据中的高四位。
     */
    uint8_t HIGH_BIT = (data & 0xF0);
    uint8_t LOW_BIT = ((data << 4) & 0xF0);

    uint8_t DATA_BUFFER[4];
    // 分割成高低四位，然后还要改变EN引脚的电平，所以一共有四部分
    // 高四位
    DATA_BUFFER[0] = (HIGH_BIT | rs | En | backlightval); // EN = 1
    DATA_BUFFER[1] = (HIGH_BIT | rs | backlightval);      // EN = 0
    // 低四位
    DATA_BUFFER[2] = (LOW_BIT | rs | En | backlightval); // EN = 1
    DATA_BUFFER[3] = (LOW_BIT | rs | backlightval);      // EN = 0
    HAL_Delay(LCD_DELAY_MS);
    while((HAL_I2C_IsDeviceReady(IIC_DEVICE, PCF8574_ADDR, 1, 100)) != HAL_OK){}
    return HAL_I2C_Master_Transmit(IIC_DEVICE, PCF8574_ADDR, (uint8_t*)DATA_BUFFER, sizeof(DATA_BUFFER), 100);
}

/**
 * @brief 发送单个数据
 * @param data[in]
 */
void lcd_sendData(uint8_t data){
    lcd_fourBit_Write(data, LCD_DATA);
}

/**
 * @brief 发送命令
 * @param cmd[in]
 */
void lcd_sendCmd(uint8_t cmd){
    HAL_StatusTypeDef ret;
    ret = lcd_fourBit_Write(cmd, LCD_COMMAND);
    if(ret != HAL_OK) { printf("I2C ERROR, CODE: %x", ret); }
}

/**
 * @brief 设置光标位置（字符显示位置）
 * @param col[in] 列坐标
 * @param row[in] 行坐标
 */
void lcd_setCursor(uint8_t col, uint8_t row){
//    HAL_StatusTypeDef ret;
    //                           row:0    row:1     row:2   row:3
    uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    lcd_sendCmd(LCD_SETDDRAMADDR|(col + row_offsets[row]));
}

/**
 * @brief 初始化
 * @param i2cHandle[in] 与PCF8574通信的I2C口
 * @param addr[in] PCF8574 A0~A3 引脚的电平
 */
void lcd_init(){
    if(!(HAL_I2C_IsDeviceReady(IIC_DEVICE, PCF8574_ADDR, 1, 100)))
        printf("PCF8574 Inited!\r\n");
    else
        printf("PCF8574 Init Failed!\r\n");
//     screen init
    HAL_Delay(500); // 等待LCD上电复位
    lcd_sendCmd(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS);
    lcd_sendCmd(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
    lcd_sendCmd(LCD_ENTRYMODESET | LCD_ENTRY_INC | LCD_ENTRYSHIFT_OFF);
    lcd_sendCmd(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
    lcd_sendCmd(LCD_RETURNHOME);
    lcd_sendCmd(LCD_CLEARDISPLAY);
    lcd_setCursor(0, 0);
    printf("LCD Inited\r\n");
}

/**
 * @brief 打印字符串到LCD1602
 * @param str[in]
 */
void lcd_print(char const str[]){
    uint8_t i = 0;
    while(str[i] != '\0') { lcd_sendData((uint8_t)(str[i++])); }
}

/**
 * @brief 打印字符串到指定坐标
 * @param str[in]
 * @param col[in] 列坐标
 * @param row[in] 行坐标
 */
void lcd_print_c(char const str[], uint8_t col, uint8_t row){
    lcd_setCursor(col, row);
    int i = 0;
    while(str[i] != '\0') { lcd_sendData((uint8_t)(str[i++])); }
}

/**
 * @brief 格式化打印到LCD1602
 * @param fmt
 * @param ...
 */
void lcd_printf(char *fmt, ...){
    va_list va;
    va_start(va, fmt);
    char str[10] = "0";
    uint8_t i = 0;
    memset(str, '\0', sizeof(char)*10);
    sprintf(str, fmt, va_arg(va, int));
    while(str[i] != '\0') { lcd_sendData((uint8_t)(str[i++])); }
    va_end(va);
}

/**
 * @brief 写入单个字符块
 * @param buff[in]
 * @param addr[in] start addr: 0B000 000、0B001 000、0B010 000、 ....、 0B111 000
 */
static void lcd_createChar(uint8_t buff[], uint8_t addr){
    lcd_sendCmd(LCD_SETCGRAMADDR | addr);
    for (int i = 0; i < 8; i++) {
        lcd_sendData(buff[i]);
    }
}

/**
 * @brief 写入多个字符块
 * @param buff[in]
 * @param addr[in] start addr: 0B000 000、0B001 000、0B010 000、 ....、 0B111 000
 */
uint8_t lcd_createChars(uint8_t buff[][8], uint8_t addr, uint8_t len){
    if((((addr & 0B111000)>>3)+len)>8)
        return 1;
    for(int i=0; i<len; i++){
        lcd_createChar(&buff[i][0], addr+(i*0x08));
    }
    return 0;
}

void backLight_blink(){
    uint8_t DATA_BUFFER[6];
    // 分割成高低四位，然后还要改变EN引脚的电平，所以一共有四部分
    // 高四位
    DATA_BUFFER[0] = (0x00 | En | backlightval); // EN = 1
    DATA_BUFFER[1] = (0x00 | backlightval);      // EN = 0
    // 低四位
    DATA_BUFFER[2] = (0x00 | En | backlightval); // EN = 1
    DATA_BUFFER[3] = (0x00 | backlightval);      // EN = 0
    // 低四位
    DATA_BUFFER[4] = (0x00 | En | backlightval); // EN = 1
    DATA_BUFFER[5] = (0x00 | backlightval);      // EN = 0
    HAL_Delay(LCD_DELAY_MS);
    while((HAL_I2C_IsDeviceReady(IIC_DEVICE, PCF8574_ADDR, 1, 100)) != HAL_OK){}
    HAL_I2C_Master_Transmit(IIC_DEVICE, PCF8574_ADDR, (uint8_t*)DATA_BUFFER, 2, 100);
    HAL_Delay(200);
    HAL_I2C_Master_Transmit(IIC_DEVICE, PCF8574_ADDR, (uint8_t*)(DATA_BUFFER+2), 2, 100);
    HAL_Delay(200);
    HAL_I2C_Master_Transmit(IIC_DEVICE, PCF8574_ADDR, (uint8_t*)(DATA_BUFFER+4), 2, 100);
}

// 显示框架
uint8_t lcd_print_frame(){
    lcd_print_c(frame[0],0,0);
#ifdef MSL_STA
    lcd_print_c(frame[1],0,1);
#endif
    return 0;
}

/**
 * @brief 打印气压值到LCD1602
 * @param val[in] 气压值
 */
void lcd_print_val(double val){
    /**
     * 数值相同的标志，或许可以改善记录startTick的时机，
     * 但实际上除了0，很少会出现相同的数值。所以效果有限
     */
    static uint8_t sameFlag = RESET;
    static double lastVal = 1000.0; // 初始化成一个不可能出现的值

    // 检测本次要显示的数据是否与上一次相同，若相同则直接退出。
    if(lastVal != val){
        lastVal = val;
        sameFlag = RESET;
    }else{ sameFlag = SET; }

    if(sameFlag) { return; }

    uint8_t len = 0;
    int32_t temp = 0;

    char str_float[CVT_BUFFER]; // 浮点数转换成字符串的缓冲区
     memset(str_float, '\0', sizeof(char)*CVT_BUFFER); // 清理缓冲区，将缓冲区元素全部设置为字符串结束符'\0'。

    SqStack charStack = {.top = -1};
    bon_ftos(str_float, val, 1); // 将浮点数转换成字符串

    /**
     * 用stack，做一个倒序排列
     * 然后从后往前，显示在LCD1602上（ENTRY_DEC模式）
     */
    Push(&charStack, '\0');
    for(uint8_t i = 0; str_float[i] != '\0'; )
        Push(&charStack, str_float[i++]);
    memset(str_float, '\0', sizeof(char)*CVT_BUFFER);
    for(int i = 0; !Pop(&charStack, &temp); ) { str_float[i++] = (char)temp; }
    lcd_sendCmd(LCD_ENTRYMODESET | LCD_ENTRY_DEC | LCD_ENTRYSHIFT_OFF);
    lcd_print_c(str_float, 15-3-3, 0);
    while(str_float[len]!='\0') { len+=1; }
    for(int i = (6-len); i > 0; i--) { lcd_print(" "); } // 留给数据的显示区域一共有6位，多余的位用空格“ ”来覆盖刷新。
    lcd_sendCmd(LCD_ENTRYMODESET | LCD_ENTRY_INC | LCD_ENTRYSHIFT_OFF);
}

// 显示云的动画
void tiny_lcd_cloud(uint8_t col){
    static int flag = 0;
    switch (flag) {
        case 1:
            lcd_setCursor(col, 1);
            lcd_sendData(0x04);
            lcd_sendData(0x05);
            lcd_sendData(0x06);
            flag = 2;
            break;
        case 2:
            lcd_setCursor(col, 1);
            lcd_sendData(0x03);
            lcd_sendData(0x04);
            lcd_sendData(0x05);
            flag = 1;
            break;
        default:
        case 0:
            lcd_setCursor(col,0);
            lcd_sendData(0x00);
            lcd_sendData(0x01);
            lcd_sendData(0x02);
            flag = 1;
            break;
    }
}

/**
 * @brief 打印肌肉状态
 * @param sta
 */
void lcd_display_sta(measStatus sta){
    lcd_print_c(sta_str[sta], 10, 1);
}

/**
 * @brief 打印肌肉状态, 并闪烁背光
 * @param sta
 */
void lcd_display_sta_b(measStatus sta){
    for(uint8_t i = 3; i--; ){
        backlightval = LCD_NOBACKLIGHT;
        lcd_print_c(sta_str[sta], 10, 1);
        HAL_Delay(150);
        backlightval = LCD_BACKLIGHT;
        lcd_print_c(sta_str[sta], 10, 1);
        HAL_Delay(150);
    }
}
