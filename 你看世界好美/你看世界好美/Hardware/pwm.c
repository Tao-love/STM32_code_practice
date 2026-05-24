#include "stm32f10x.h"                  // 设备头文件

void PWM_Init(void){
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能TIM4和相关GPIO的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // 设置GPIO模式和配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_9);

    // 定时器基本配置
    TIM_TimeBaseStructure.TIM_Period = 100;
    TIM_TimeBaseStructure.TIM_Prescaler = 72-1;  // 预分频为72
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    // 定时器输出比较配置（PWM设置）
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 50;  // 初始50%占空比
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC4Init(TIM4, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);

    // 启用TIM4定时器
    TIM_Cmd(TIM4, ENABLE);
}

void Set_PWM(uint16_t period, uint16_t pulse){
    if (pulse <= period) {
        TIM_SetAutoreload(TIM4, period);
        TIM_SetCompare4(TIM4, pulse);
    } else {
        // 错误处理: 占空比不能超过周期
    }
}
