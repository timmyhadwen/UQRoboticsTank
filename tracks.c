#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "tracks.h"
static TIM_HandleTypeDef TIM_Init4;
static TIM_HandleTypeDef TIM_Init7;

void tracks_init(){
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_OC_InitTypeDef PWMConfig;
  uint32_t PrescalerValue;

  __BRD_D4_GPIO_CLK();
	__BRD_D5_GPIO_CLK();
  __BRD_D6_GPIO_CLK();
  __BRD_D7_GPIO_CLK();

  __TIM1_CLK_ENABLE();
  __TIM3_CLK_ENABLE();

  //Track direction pins
	GPIO_InitStructure.Pin = BRD_D5_PIN;				//Pin
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;		//Output Mode
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;			//Enable Pull up, down or no pull resister
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;			//Pin latency
	HAL_GPIO_Init(BRD_D5_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin

	GPIO_InitStructure.Pin = BRD_D6_PIN;				//Pin
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;			//Input Mode
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;			//Enable Pull up, down or no pull resister
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;			//Pin latency
	HAL_GPIO_Init(BRD_D6_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin

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
//
//   //Track speed pins (PWM)
//   GPIO_InitStructure.Pin = BRD_D4_PIN;				//Pin
//   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP; 		//Set mode to be output alternate
//   GPIO_InitStructure.Pull = GPIO_NOPULL;			//Enable Pull up, down or no pull resister
//   GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;			//Pin latency
//   GPIO_InitStructure.Alternate = GPIO_AF2_TIM3;	//Set alternate function to be timer 3
//   HAL_GPIO_Init(BRD_D4_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin
//
//   GPIO_InitStructure.Pin = BRD_D7_PIN;				//Pin
//   GPIO_InitStructure.Mode = GPIO_MODE_AF_PP; 		//Set mode to be output alternate
//   GPIO_InitStructure.Pull = GPIO_NOPULL;			//Enable Pull up, down or no pull resister
//   GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;			//Pin latency
//   GPIO_InitStructure.Alternate = GPIO_AF1_TIM1;	//Set alternate function to be timer 3
//   HAL_GPIO_Init(BRD_D7_GPIO_PORT, &GPIO_InitStructure);	//Initialise Pin
//
//   /* Compute the prescaler value. SystemCoreClock = 168000000 - set for 50Khz clock */
//   PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 1000000) - 1;
//
//   /* Configure Timer settings */
//   TIM_Init4.Instance = TIM3;					//Enable Timer 3
//   TIM_Init4.Init.Period = 100000;			//Set for 200ms (5Hz) period
//   TIM_Init4.Init.Prescaler = PrescalerValue;	//Set presale value
//   TIM_Init4.Init.ClockDivision = 0;			//Set clock division
//   TIM_Init4.Init.RepetitionCounter = 0; 		// Set Reload Value
//   TIM_Init4.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.
//
//   /* PWM Mode configuration for Channel 2 - set pulse width*/
//   PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)
//   PWMConfig.Pulse        = 100000/2;		//1ms pulse width to 10ms
//   PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
//   PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
//   PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
//   PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
//   PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
//
//   /* Enable PWM for Timer 3, channel 2 */
//   HAL_TIM_PWM_Init(&TIM_Init4);
//   HAL_TIM_PWM_ConfigChannel(&TIM_Init4, &PWMConfig, TIM_CHANNEL_2);
//
//   /* Start PWM */
//   HAL_TIM_PWM_Start(&TIM_Init4, TIM_CHANNEL_2);
//
//   /* Configure Timer settings */
//   TIM_Init7.Instance = TIM1;					//Enable Timer 3
//   TIM_Init7.Init.Period = 100000;			//Set for 200ms (5Hz) period
//   TIM_Init7.Init.Prescaler = PrescalerValue;	//Set presale value
//   TIM_Init7.Init.ClockDivision = 0;			//Set clock division
//   TIM_Init7.Init.RepetitionCounter = 0; 		// Set Reload Value
//   TIM_Init7.Init.CounterMode = TIM_COUNTERMODE_UP;	//Set timer to count up.
//
//   /* PWM Mode configuration for Channel 2 - set pulse width*/
//   PWMConfig.OCMode       = TIM_OCMODE_PWM1;	//Set PWM MODE (1 or 2 - NOT CHANNEL)
//   PWMConfig.Pulse        = 100000/2;		//1ms pulse width to 10ms
//   PWMConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
//   PWMConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
//   PWMConfig.OCFastMode   = TIM_OCFAST_DISABLE;
//   PWMConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
//   PWMConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
//
//   /* Enable PWM for Timer 3, channel 2 */
//   HAL_TIM_PWM_Init(&TIM_Init7);
//   HAL_TIM_PWM_ConfigChannel(&TIM_Init7, &PWMConfig, TIM_CHANNEL_1);
//
//   /* Start PWM */
//   HAL_TIM_PWM_Start(&TIM_Init7, TIM_CHANNEL_1);
//   debug_printf("DEBUG: Tracks initialized\n\n");
}

//Input range -100 to 100
void set_left_track(int x){
  HAL_GPIO_WritePin(BRD_D4_GPIO_PORT, BRD_D4_PIN, x); //Enable motor
}


//Input range -100 to 100
void set_right_track(int x){
  HAL_GPIO_WritePin(BRD_D7_GPIO_PORT, BRD_D7_PIN, x); //Enable motor
}

void start( void ){
  HAL_GPIO_WritePin(BRD_D5_GPIO_PORT, BRD_D5_PIN, 1); //Enable motor
  HAL_GPIO_WritePin(BRD_D6_GPIO_PORT, BRD_D6_PIN, 1); //Enable motor
}

void stop( void ){
  HAL_GPIO_WritePin(BRD_D5_GPIO_PORT, BRD_D5_PIN, 0); //Enable motor
  HAL_GPIO_WritePin(BRD_D6_GPIO_PORT, BRD_D6_PIN, 0); //Enable motor
}
