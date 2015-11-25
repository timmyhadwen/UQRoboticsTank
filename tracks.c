#include "tracks.h"

static TIM_HandleTypeDef TIM_Init5;
static TIM_HandleTypeDef TIM_Init6;

extern void tracks_init(){
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_OC_InitTypeDef PWMConfig;
  uint32_t PrescalerValue;

  __BRD_D4_GPIO_CLK();
  __BRD_D5_GPIO_CLK();
  __BRD_D6_GPIO_CLK();
  __BRD_D7_GPIO_CLK();

  //Track direction pins
  GPIO_InitStructure.Pin = BRD_D4_PIN;				//Pin
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		//Output Mode
  GPIO_InitStructure.Pull = GPIO_PULLDOWN;			//Enable Pull up, down or no pull resister
  GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;			//Pin latency
  HAL_GPIO_Init(BRD_D4_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin

  GPIO_InitStructure.Pin = BRD_D7_PIN;				//Pin
  GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//Input Mode
  GPIO_InitStructure.Pull = GPIO_PULLDOWN;			//Enable Pull up, down or no pull resister
  GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;			//Pin latency
  HAL_GPIO_Init(BRD_D7_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin

  //Track ENABLE pins
  __TIM2_CLK_ENABLE();
  __TIM3_CLK_ENABLE();

   /* Compute the prescaler value. SystemCoreClock = 168000000 - set for 50Khz clock */
   PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 1000000) - 1;

    //Track speed pins (PWM)
    GPIO_InitStructure.Pin = BRD_D5_PIN;				//Pin
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP; 		//Set mode to be output alternate
    GPIO_InitStructure.Pull = GPIO_NOPULL;			//Enable Pull up, down or no pull resister
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;			//Pin latency
    GPIO_InitStructure.Alternate = GPIO_AF2_TIM3;	//Set alternate function to be timer 3
    HAL_GPIO_Init(BRD_D5_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin

   /* Configure Timer settings */
   TIM_Init5.Instance = TIM3;					//Enable Timer 3
   TIM_Init5.Init.Period = 10000;			//Set for 200ms (5Hz) period
   TIM_Init5.Init.Prescaler = PrescalerValue;	//Set presale value
   TIM_Init5.Init.ClockDivision = 0;			//Set clock division
   TIM_Init5.Init.RepetitionCounter = 0; 		// Set Reload Value
   TIM_Init5.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.

   /* PWM Mode configuration for Channel 2 - set pulse width*/
   PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)
   PWMConfig.Pulse        = 7500;		//1ms pulse width to 10ms
   PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
   PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
   PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
   PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
   PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

   /* Enable PWM for Timer 3, channel 2 */
   HAL_TIM_PWM_Init(&TIM_Init5);
   HAL_TIM_PWM_ConfigChannel(&TIM_Init5, &PWMConfig, TIM_CHANNEL_1);

   /* Start PWM */
   HAL_TIM_PWM_Start(&TIM_Init5, TIM_CHANNEL_1);

   GPIO_InitStructure.Pin = BRD_D6_PIN;				//Pin
   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP; 		//Set mode to be output alternate
   GPIO_InitStructure.Pull = GPIO_NOPULL;			//Enable Pull up, down or no pull resister
   GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;			//Pin latency
   GPIO_InitStructure.Alternate = GPIO_AF1_TIM2;	//Set alternate function to be timer 3
   HAL_GPIO_Init(BRD_D6_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin

   /* Configure Timer settings */
   TIM_Init6.Instance = TIM2;					//Enable Timer 3
   TIM_Init6.Init.Period = 10000;			//Set for 200ms (5Hz) period
   TIM_Init6.Init.Prescaler = PrescalerValue;	//Set presale value
   TIM_Init6.Init.ClockDivision = 0;			//Set clock division
   TIM_Init6.Init.RepetitionCounter = 0; 		// Set Reload Value
   TIM_Init6.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.

   /* PWM Mode configuration for Channel 2 - set pulse width*/
   PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)
   PWMConfig.Pulse        = 7500;		//1ms pulse width to 10ms
   PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
   PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
   PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
   PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
   PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

   /* Enable PWM for Timer 3, channel 2 */
   HAL_TIM_PWM_Init(&TIM_Init6);
   HAL_TIM_PWM_ConfigChannel(&TIM_Init6, &PWMConfig, TIM_CHANNEL_3);

   /* Start PWM */
   HAL_TIM_PWM_Start(&TIM_Init6, TIM_CHANNEL_3);

}

extern void set_left_direction(int x){
  HAL_GPIO_WritePin(BRD_D4_GPIO_PORT, BRD_D4_PIN, x); //Enable motor
}

extern void set_right_direction(int x){
  HAL_GPIO_WritePin(BRD_D7_GPIO_PORT, BRD_D7_PIN, x); //Enable motor
}

extern void set_left_speed(enum TrackSpeed x){
    TIM_OC_InitTypeDef PWMConfig;
    PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)

    PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
    PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    switch(x){
        case SPEED_FAST:
            PWMConfig.Pulse  = 10000;
            break;
        case SPEED_SLOW:
            PWMConfig.Pulse  = 5000;
            break;
        case SPEED_STOP:
            PWMConfig.Pulse = 0;
            break;
    }

    HAL_TIM_PWM_ConfigChannel(&TIM_Init5, &PWMConfig, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&TIM_Init5, TIM_CHANNEL_1);
}

extern void set_left_speed_fine(int x) {
    int perc = x*100;
    
    if(perc > 10000)
        perc = 10000;
    
    if(perc < 0)
        perc = 0;
    
    TIM_OC_InitTypeDef PWMConfig;
    PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)

    PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
    PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
            
    PWMConfig.Pulse  = perc;

    HAL_TIM_PWM_ConfigChannel(&TIM_Init5, &PWMConfig, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&TIM_Init5, TIM_CHANNEL_1);
}

extern void set_right_speed(enum TrackSpeed x){
    TIM_OC_InitTypeDef PWMConfig;
    PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)
    PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
    PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    switch(x){
        case SPEED_FAST:
            PWMConfig.Pulse  = 10000;
            break;
        case SPEED_SLOW:
            PWMConfig.Pulse  = 5000;
            break;
        case SPEED_STOP:
            PWMConfig.Pulse = 0;
            break;
    }

    HAL_TIM_PWM_ConfigChannel(&TIM_Init6, &PWMConfig, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&TIM_Init6, TIM_CHANNEL_3);
}

extern void set_right_speed_fine(int x) {
    int perc = x*100;
    
    if(perc > 10000)
        perc = 10000;
    
    if(perc < 0)
        perc = 0;
    
    TIM_OC_InitTypeDef PWMConfig;
    PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)

    PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
    PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
            
    PWMConfig.Pulse  = perc;

    HAL_TIM_PWM_ConfigChannel(&TIM_Init6, &PWMConfig, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&TIM_Init6, TIM_CHANNEL_3);
}
