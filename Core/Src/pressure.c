//
// Created by dazui on 2023/5/5.
//

#include "pressure.h"
#include <string.h>
#include <math.h>
#include "printf.h"

char cvtStr[CVT_BUFFER];

Prs_HandleTypeDef* pres_list;

/* 设置传感器输出电压的偏移量, 实际测得0kPa时的电压要多100mV, 这个多出来的电压就是偏移量 */
void set_offset(Prs_HandleTypeDef* L){
    L->offset = get_currentVol(L)-2700;
    printf("offset: %s\r\n", FtoS(L->offset, 2));
}

/**
 * @brief 获取气压值
 * @return
 */
double get_pressure(){
//    printf("voltage: %ld\r\n", (uint32_t)get_currentVol(pres_list));
    return ((get_currentVol(pres_list)-(pres_list->offset))/25-108);
}

/**
 * @brief 获取ADC当前的电压值，单位 mV
 * @param L[in]
 * @return 当前的电压值
 */
float get_currentVol(Prs_HandleTypeDef* L){
    uint32_t temp = 0;
    Prs_HandleTypeDef temp_list = *L; // 由于DMA是一直在运行的，为了防止ADC数据在计算的过程中发生变化(排序后会被打乱)， 所以将数据复制到另一个结构体中。
    /**
     * 均值滤波，但是将最大值和最小值去除
     */
    bubbleSort(&temp_list);
    temp_list.adcVal[1] = 0;
    temp_list.adcVal[temp_list.length]=0;
    for(int i=1; i<=temp_list.length; i+=1)
        temp+=temp_list.adcVal[i];
    float flitted_adc = (float)temp/(float)(temp_list.length-2);// 滤波后的ADC原始数据
//    printf("flitted_adc: %ld\r\n", (uint32_t)flitted_adc);
    return L->voltage = (float)((flitted_adc/4095.0f)*3300.0f); // 滤波后的电压值
}

/**
 * @brief 设置单位值（1kPa对应的电压值）,已废弃
 * @return
 */
float set_unit(){
    HAL_Delay(100); // 等待DMA转换。
    float current_val = get_currentVol(pres_list);
    return pres_list->prs_unit =  (current_val-200)/100;
}

/**
 * @brief 初始化 ADC
 * @param adc[in]
 * @param L[in]
 */
void pressure_init(ADC_HandleTypeDef* adc, Prs_HandleTypeDef* L){
    pres_list = L;
    HAL_ADCEx_Calibration_Start(adc); // ADC自动标定（自动校准）
    HAL_ADC_Start_DMA(adc, &(pres_list->adcVal[1]), pres_list->length);
    HAL_Delay(100);
    set_offset(pres_list);
}

/**
 * 交换数组元素i和j的位置
*/
static void swap(Prs_HandleTypeDef* L, int i, int j){
    uint32_t temp = L->adcVal[i];
    L->adcVal[i] = L->adcVal[j];
    L->adcVal[j] = temp;
}

/**
 * @brief 冒泡排序
 * @param L
 */
static void bubbleSort(Prs_HandleTypeDef* L){
    int i, j;
    int flag = 1; // 记录是否发生了数据交换

    for(i=1; i<(L->length) && flag; i+=1){
        flag = 0;   // 若本次循环没有发生数据交换, 则flag一直为0, 代表后面的数据都已经齐了。
        for(j=(L->length)-1; j>=i; j-=1)
            if((L->adcVal[j])>(L->adcVal[j+1])){
                swap(L, j, j+1);
                flag = 1;   // 发生了数据交换, 则置1
            }
    }
}

/**
 * @brief x^y
 * @param x
 * @param y
 * @return
 */
int32_t int_pow(int32_t x, int32_t y){
    int32_t temp = 1;
    if(x == 0)
        return 0;
    else if(y == 0)
        return 1;
    else{
        while(y--)
            temp*=x;
        return temp;
    }
}

/**
 * @brief 出栈
 * @param S[in]
 * @param e[out]
 * @return
 */
uint8_t Pop(SqStack* S, SElemType* e){
    if(S->top == -1)
        return 1; //栈空
    *e=S->data[S->top];
    S->top -= 1; //先pop再减
    return 0;
}

/**
 * @brief 入栈
 * @param S[in]
 * @param e[in]
 * @return
 */
uint8_t Push(SqStack* S, SElemType e){
    if(S->top == MAXSIZE-1)
        return 1; // 栈满
    (S->top)+=1; // 先自增栈顶指针
    S->data[S->top] = e;
    return 0;
}

/**
 * @brief 浮点数转换成字符串。
 * @param s[in]
 * @param val[in]
 * @param n[in] 需要保留的小数位长度
 * @return 0: 转换完成 1: 超出缓冲区
 */
uint8_t bon_ftos(char s[], double val, uint8_t n){
    memset(s, '\0', sizeof(char)*CVT_BUFFER);
    // val == 0的情况下单独处理。
    if(val == 0){
        memset(s, '0', sizeof(char)*CVT_BUFFER); // 将所有的元素都设置为'0'， 这样就只需要设置整数位、小数点以及结束符就行了，不用管小数点有多少位。
        s[0] = '0';
        s[1] = '.';
        s[n+2] = '\0'; // 将最后一位设置为'\0'
        return 0;
    }
    // 去除小数点，转换成整数
    int32_t temp = (int32_t)(val*int_pow(10, n));
    // 在字符串中添加符号，负数加'-'，正数加' '(这个空格后面会被舍弃)
    if(val<0){
        s[0] = '-';
        temp*=(-1); // 转换成正数方便后面处理
    }
    else
        s[0] = ' ';
    /** 计算数字的总长度（10进制）, 包括小数位和整数位 **/
    uint8_t len = 0;
    if(fabs(val) < 1.0) { len += 1; } // 绝对值小于1时，len会有问题，例如val = 0.5 那么len只能得到1，但实际上有2。所以要加 +1
    // 去除小数点，转换成整数
    for(int32_t i = temp; i >= 1; i/=10) { len += 1; }

    // 检测长度是否会超出缓冲区, 应该放在添加符号那一段的前面（以后改）
    if(len+1 > CVT_BUFFER) { return 1; }
    /**
     * 由于取余提取出来的字符顺序是倒的，所以还要用栈来倒序排列字符。
     */
    SqStack stack = {.top = -1};
    Push(&stack, ('\0'-'0')); // 字符串结尾有个 '\0'， 由于是倒序，所以先将最末尾的结束符入栈。并且，由于后面的出栈操作中会添加'0'， 所以这里先减掉。
    for(uint8_t i = 0; i < len; i+=1){
        if(i==n) // 添加小数点
            Push(&stack, ('.'-'0')); // 由于后面出栈处理中要加上'0', 所以这里先减去'0'
        Push(&stack, (temp / int_pow(10, i)) % 10);
    }
    /**
     * 由于负数需要添加’-‘符号，而正数不需要有符号，
     * 所以通过判断s[0]是否为'-'， 来设置i为0或1。
     * 负数保留s[0]里的'-'，而正数则覆盖s[0]里的' '
     */
    for(int i = (1-(s[0]=='-'? 0:1)); !Pop(&stack, &temp); i+=1)
        s[i]=(char)temp+'0';
    // 没有给末尾加上'\0'，因为之前memset中已经把s里的元素全部设为'0'了，所以只要别溢出，就不会出问题。
    return 0;
}

char* FtoS(double val, uint8_t n){
    bon_ftos(cvtStr, val, n);
    return cvtStr;
}
