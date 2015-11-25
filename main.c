/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "tracks.h"
#include <string.h>
#include "wheelencoders.c"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SECOND 0x7FFF00
#define _PWM_DRIVE
#define mainLED_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define mainLED_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 5 )
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int left_goal = 0;
int right_goal = 0;

static int P = 0.5;


extern int count_left;
extern int count_right;

/* Private function prototypes -----------------------------------------------*/
void Main_Task( void );
void Tracks_Task( void );
void Hardware_init( void );

void main(void) {
	BRD_init();
	Hardware_init();

	/* Start the task to flash the LED. */
	xTaskCreate( (void *) &Main_Task, (const signed char *) "LED", mainLED_TASK_STACK_SIZE, NULL, mainLED_PRIORITY, NULL );
        
        xTaskCreate( (void *) &Tracks_Task, (const signed char *) "TRA", mainLED_TASK_STACK_SIZE, NULL, mainLED_PRIORITY, NULL );
        
	vTaskStartScheduler();
}

void Main_Task(){
    
    left_goal = 500;
    
    right_goal = 500;
    
    for(;;){
        BRD_LEDToggle();

        vTaskDelay(1000);
    }
}

void Tracks_Task(){
	for(;;){
            
            // P Controller
            set_left_speed((left_goal - count_left)*P); 
            set_right_speed((right_goal - count_right)*P);
            
            vTaskDelay(10);
	}
}

void Hardware_init() {

	portDISABLE_INTERRUPTS();	//Disable interrupts

	BRD_LEDInit();
	BRD_LEDOn();
        
        tracks_init();
        
        wheelencoders_init();

	portENABLE_INTERRUPTS();	//Disable interrupts
}

/**
  * @brief  Application Tick Task.
  * @param  None
  * @retval None
  */
void vApplicationTickHook( void ) {
	BRD_LEDOff();
}

/**
  * @brief  Idle Application Task
  * @param  None
  * @retval None
  */
void vApplicationIdleHook( void ) {
	static portTickType xLastTx = 0;

	BRD_LEDOff();

	for (;;) {

		/* The idle hook simply prints the idle tick count, every second */
		if ((xTaskGetTickCount() - xLastTx ) > (1000 / portTICK_RATE_MS)) {

			xLastTx = xTaskGetTickCount();

			//debug_printf("IDLE Tick %d\n", xLastTx);

			/* Blink Alive LED */
//			BRD_LEDToggle();
		}
	}
}
/**
  * @brief  vApplicationStackOverflowHook
  * @param  Task Handler and Task Name
  * @retval None
  */
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName ) {
	/* This function will get called if a task overflows its stack.   If the
	parameters are corrupt then inspect pxCurrentTCB to find which was the
	offending task. */

	BRD_LEDOff();
	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
}

