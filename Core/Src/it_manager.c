//
// Created by dazui on 2023/4/9.
//
#include "it_manager.h"
#include "main.h"

TimIT_StateTypeDef Timer_State = {
        .Timer1 = RESET,
        .Timer2 = RESET,
        .Timer3 = RESET,
        .Timer4 = RESET
};

GPIO_IT_StateTypedef GPIO_State = {
        .KEY1 = RESET,
        .KEY2 = RESET
};

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if(htim->Instance == TIM1) { Timer_State.Timer1 = SET; }
    else if(htim->Instance == TIM3) {Timer_State.Timer3 = SET;}
}

/**
* 上一次的触发一定不能与本次相同，所以要加一个标志位（相当于互斥锁）
* RESET = SET: 上一次为下降沿中断
* RESET = RESET: 上一次为上升沿中断
*/
uint8_t KEY1_ExTi_flag = SET; // 因为按正常逻辑，按键按下的脉冲肯定是先来下降沿，为了能够正常触发,所以初始化为SET
uint32_t Falling_Tick = 0; // 下降沿触发时的Tick数

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin){
    if((GPIO_Pin == KEY_Pin) && KEY1_ExTi_flag){
        KEY1_ExTi_flag = RESET;
        Falling_Tick = HAL_GetTick();
    }
}

void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin){
    if((GPIO_Pin == KEY_Pin) && !KEY1_ExTi_flag){
        KEY1_ExTi_flag = SET;
        if((Falling_Tick+10)<HAL_GetTick()) { GPIO_State.KEY1 = SET; }
    }
}
