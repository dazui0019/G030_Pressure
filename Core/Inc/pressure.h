//
// Created by dazui on 2023/5/5.
//

#ifndef LCD_PRESSURE_H
#define LCD_PRESSURE_H

#include "main.h"
#include "adc.h"

#define MAXSIZE 20

#define CVT_BUFFER 20

/**
 * @brief 传感器结构体
*/
typedef struct{
    uint32_t adcVal[MAXSIZE+1]; // r[0] 存放滤波后的ADC原始数据， 因此length就是数组最后一个元素的真实下标
    uint8_t length;
    float voltage;  // 滤波后的电压值
    float prs_unit; // 单位值（1kPa对应的电压值）
}Prs_HandleTypeDef;

float set_unit();
float get_currentVol(Prs_HandleTypeDef* L);
double get_pressure();
void pressure_init(ADC_HandleTypeDef* ghadc, Prs_HandleTypeDef* L);

/** Sort **/
static void swap(Prs_HandleTypeDef* L, int i, int j);
static void bubbleSort(Prs_HandleTypeDef* L);

/**
 * stack
 */
typedef int32_t SElemType;
#define MAXSIZE 20
typedef struct{
    SElemType data[MAXSIZE];
    int top;
}SqStack;

uint8_t Push(SqStack* S, SElemType e);
uint8_t Pop(SqStack* S, SElemType* e);

/**
 * @brief
 * @param s[out]
 * @param val[in]
 * @param n[in]
 * @return
 */
uint8_t bon_ftos(char s[], double val, uint8_t n);
/**
 * @brief x^y
 * @param x[in]
 * @param y[in]
 * @return
 */
int32_t int_pow(int32_t x, int32_t y);

#endif //LCD_PRESSURE_H
