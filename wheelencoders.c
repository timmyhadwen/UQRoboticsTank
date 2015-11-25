/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"

#include "FreeRTOS.h"

int count_left = 0;
int count_right = 0;

void LEFT_IRQ(void);
void RIGHT_IRQ(void);

extern void wheelencoders_init() {
    
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    GPIO_InitTypeDef  GPIO_InitStructure2;
    
    __BRD_D8_GPIO_CLK();
    __BRD_D12_GPIO_CLK();

    /* Configure D0 pin as pull down input */
    GPIO_InitStructure.Pin = BRD_D3_PIN;				//Pin
    GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;		//interrupt Mode
    GPIO_InitStructure.Pull = GPIO_PULLUP;			//Enable Pull up, down or no pull resister
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;			//Pin latency
    HAL_GPIO_Init(BRD_D3_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin
    
    /* Configure D0 pin as pull down input */
    GPIO_InitStructure2.Pin = BRD_D2_PIN;				//Pin
    GPIO_InitStructure2.Mode = GPIO_MODE_IT_RISING;		//interrupt Mode
    GPIO_InitStructure2.Pull = GPIO_PULLUP;			//Enable Pull up, down or no pull resister
    GPIO_InitStructure2.Speed = GPIO_SPEED_HIGH;			//Pin latency
    HAL_GPIO_Init(BRD_D2_GPIO_PORT, &GPIO_InitStructure2);	//Initialise Pin
    
    
    /* Set priority of external GPIO Interrupt [0 (HIGH priority) to 15(LOW priority)] */
    /* 	DO NOT SET INTERRUPT PRIORITY HIGHER THAN 3 */
    HAL_NVIC_SetPriority(BRD_D3_EXTI_IRQ, 13, 6);	//Set Main priority ot 10 and sub-priority ot 0

    //Enable external GPIO interrupt and interrupt vector for pin DO
    NVIC_SetVector(BRD_D3_EXTI_IRQ, (uint32_t)&LEFT_IRQ);  
    NVIC_EnableIRQ(BRD_D3_EXTI_IRQ);
    
    /* Set priority of external GPIO Interrupt [0 (HIGH priority) to 15(LOW priority)] */
    /* 	DO NOT SET INTERRUPT PRIORITY HIGHER THAN 3 */
    HAL_NVIC_SetPriority(BRD_D2_EXTI_IRQ, 14, 7);	//Set Main priority ot 10 and sub-priority ot 0

    //Enable external GPIO interrupt and interrupt vector for pin DO
    NVIC_SetVector(BRD_D2_EXTI_IRQ, (uint32_t)&RIGHT_IRQ);  
    NVIC_EnableIRQ(BRD_D2_EXTI_IRQ);
}

/**
  * @brief  NP2_D0_EXTI Interrupt handler - see netduinoplus2.h
  * @param  None.
  * @retval None
  */
void LEFT_IRQ(void) {
    HAL_GPIO_EXTI_IRQHandler(BRD_D3_PIN);	//Clear D0 pin external interrupt flag

    count_left++;		//increment press count, everytime the interrupt occurs
}

/**
  * @brief  NP2_D0_EXTI Interrupt handler - see netduinoplus2.h
  * @param  None.
  * @retval None
  */
void RIGHT_IRQ(void) {
    HAL_GPIO_EXTI_IRQHandler(BRD_D2_PIN);	//Clear D0 pin external interrupt flag

    count_right++;		//increment press count, everytime the interrupt occurs
}