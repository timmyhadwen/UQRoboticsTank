//
// Created by Tim on 13/05/15.
//
#include "claw.h"

static TIM_HandleTypeDef TIM_Init;

extern void claw_open(){
    TIM_OC_InitTypeDef PWMConfig;
    /* PWM Mode configuration for Channel 2 - set pulse width*/
    PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)
    PWMConfig.Pulse        = 500;		//1ms pulse width to 10ms
    PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
    PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    HAL_TIM_PWM_ConfigChannel(&TIM_Init, &PWMConfig, TIM_CHANNEL_2);

    /* Start PWM */
    HAL_TIM_PWM_Start(&TIM_Init, TIM_CHANNEL_2);
}

extern void claw_close(){
    TIM_OC_InitTypeDef PWMConfig;
    /* PWM Mode configuration for Channel 2 - set pulse width*/
    PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)
    PWMConfig.Pulse        = 1600;		//1ms pulse width to 10ms
    PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
    PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    HAL_TIM_PWM_ConfigChannel(&TIM_Init, &PWMConfig, TIM_CHANNEL_2);

    /* Start PWM */
    HAL_TIM_PWM_Start(&TIM_Init, TIM_CHANNEL_2);
}

extern void claw_init(){
    uint16_t PrescalerValue = 0;
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_OC_InitTypeDef PWMConfig;

    __TIM2_CLK_ENABLE();
    __BRD_D3_GPIO_CLK();

    GPIO_InitStructure.Pin = BRD_D3_PIN;				//Pin
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP; 		//Set mode to be output alternate
    GPIO_InitStructure.Pull = GPIO_NOPULL;			//Enable Pull up, down or no pull resister
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;			//Pin latency
    GPIO_InitStructure.Alternate = GPIO_AF1_TIM2;	//Set alternate function to be timer 3
    HAL_GPIO_Init(BRD_D3_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin

    /* Compute the prescaler value. SystemCoreClock = 168000000 - set for 500Khz clock */
    PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 500000) - 1;

    /* Configure Timer settings */
    TIM_Init.Instance = TIM2;					//Enable Timer 3
    TIM_Init.Init.Period = 20000;			//Set for 200ms (5Hz) period
    TIM_Init.Init.Prescaler = PrescalerValue;	//Set presale value
    TIM_Init.Init.ClockDivision = 0;			//Set clock division
    TIM_Init.Init.RepetitionCounter = 0; 		// Set Reload Value
    TIM_Init.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.

    /* PWM Mode configuration for Channel 2 - set pulse width*/
    PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)
    PWMConfig.Pulse        = 100;		//1ms pulse width to 10ms
    PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
    PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    /* Enable PWM for Timer 2, channel 2 */
    HAL_TIM_PWM_Init(&TIM_Init);
    HAL_TIM_PWM_ConfigChannel(&TIM_Init, &PWMConfig, TIM_CHANNEL_2);

    /* Start PWM */
    HAL_TIM_PWM_Start(&TIM_Init, TIM_CHANNEL_2);
}
