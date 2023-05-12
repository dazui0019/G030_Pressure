//
// Created by dazui on 2023/4/9.
//

#ifndef ENCODER_IT_MANAGER_H
#define ENCODER_IT_MANAGER_H

#include "main.h"

/**
 * 定时器中断产生标志，由软件使用完后手动清零
 */
typedef struct{
    uint8_t Timer1;
    uint8_t Timer2;
    uint8_t Timer3;
    uint8_t Timer4;
} TimIT_StateTypeDef;

typedef struct{
    uint8_t KEY1;
    uint8_t KEY2;
}GPIO_IT_StateTypedef;

extern TimIT_StateTypeDef Timer_State;
extern GPIO_IT_StateTypedef GPIO_State;

#endif //ENCODER_IT_MANAGER_H
