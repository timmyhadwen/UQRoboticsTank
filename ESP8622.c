#include "board.h"
#include "stm32f4xx_hal_conf.h"
#include "debug_printf.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ESP8622.h"
#include <string.h>
#include <stdio.h>
#include "semphr.h"


#define TRUE 1
#define FALSE 0
#define NODE_ID 1

#define USART_TX_TASK_PRIORITY					( tskIDLE_PRIORITY + 1 )
#define USART_TX_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

UART_HandleTypeDef UART_Handler;
DMA_HandleTypeDef hdma_tx;
QueueHandle_t Data_Queue;	/* Queue used */

SemaphoreHandle_t USART1_Semaphore;
QueueHandle_t USART_Tx_Queue;	/* Queue used */

extern SemaphoreHandle_t esp_Semaphore;


volatile int lastTaskPassed = FALSE;
volatile int prompt = FALSE;

char line_buffer[100];
uint8_t line_buffer_index = 0;

char ip_addr_string[20];
char uart_buffer[100];

uint8_t* uart_tx_buffer;

extern int32_t time_Offset;
extern int8_t client_Pipe;
APs* Access_Points;

extern void Testing_Task( void );
/**
  * This module is for the ESP8622 'El Cheapo' Wifi Module.
  * It uses pins D10 (TX) and D2 (RX) and Serial 1 on the NucleoF401RE Dev Board
  *
  * @author Timmy Hadwen
  * @author Michael Thoreau
  */
void 	ESP8622_init( void ){
  GPIO_InitTypeDef GPIO_serial;

  __USART1_CLK_ENABLE();
  __BRD_D10_GPIO_CLK();
  __BRD_D2_GPIO_CLK();

  /* Configure the D2 as the RX pin for USART1 */
  GPIO_serial.Pin = BRD_D2_PIN;
  GPIO_serial.Mode = GPIO_MODE_AF_PP;				             	//Enable alternate mode setting
  GPIO_serial.Pull = GPIO_PULLDOWN;
  GPIO_serial.Speed = GPIO_SPEED_HIGH;
  GPIO_serial.Alternate = GPIO_AF7_USART1;	           		//Set alternate setting to USART1
  HAL_GPIO_Init(BRD_D2_GPIO_PORT, &GPIO_serial);

  /* Configure the D10 as the TX pin for USART1 */
  GPIO_serial.Pin = BRD_D10_PIN;
  GPIO_serial.Mode = GPIO_MODE_AF_PP;				             	//Enable alternate mode setting
  GPIO_serial.Pull = GPIO_PULLUP;
  GPIO_serial.Speed = GPIO_SPEED_HIGH;
  GPIO_serial.Alternate = GPIO_AF7_USART1;		          	//Set alternate setting to USART1
  HAL_GPIO_Init(BRD_D10_GPIO_PORT, &GPIO_serial);

  UART_Handler.Instance          = USART1;
  UART_Handler.Init.BaudRate     = 9600;
  UART_Handler.Init.WordLength   = UART_WORDLENGTH_8B;
  UART_Handler.Init.StopBits     = UART_STOPBITS_1;
  UART_Handler.Init.Parity       = UART_PARITY_NONE;
  UART_Handler.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UART_Handler.Init.Mode         = UART_MODE_TX_RX;
  UART_Handler.Init.OverSampling = UART_OVERSAMPLING_16;

  HAL_NVIC_SetPriority(USART1_IRQn, 10, 0);
  NVIC_SetVector(USART1_IRQn, &UART1_IRQHandler);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

  HAL_UART_Init(&UART_Handler);

  dma_Init();

  /*  Enable RXNE interrupt on USART_1 */
  if (HAL_UART_Receive_IT((UART_HandleTypeDef*)&UART_Handler, (uint8_t *)uart_buffer, 100) != HAL_OK) {
	  debug_printf("UART Interrupt init FAIL");
  }

  xTaskCreate( (void *) &UART_Processor, (const signed char *) "DATA", mainLED_TASK_STACK_SIZE * 5, NULL, mainLED_PRIORITY + 1, NULL );

  Data_Queue = xQueueCreate(20, sizeof(char[100]));
  Message_Queue = xQueueCreate(10, sizeof(char[50]));
  Access_Points = pvPortMalloc(sizeof(APs));
  Access_Points->size = 0;
  Access_Points->HEAD = NULL;
  Access_Points->TAIL = NULL;
}

void dma_Init(void) {
	USART1_Semaphore = xSemaphoreCreateMutex();
	  /*##-1- Enable peripherals and GPIO Clocks #################################*/

	  /* Enable DMA1 clock */
	  __HAL_RCC_DMA2_CLK_ENABLE();

	  /*##-3- Configure the DMA streams ##########################################*/
	  /* Configure the DMA handler for Transmission process */
	  hdma_tx.Instance                 = DMA2_Stream7;
	  hdma_tx.Init.Channel             = DMA_CHANNEL_4;
	  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
	  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
	  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	  hdma_tx.Init.Mode                = DMA_NORMAL;
	  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
	  hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
	  hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	  hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
	  hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;

	  //HAL_DMA_(DMA1_Stream2, DMA_IT_TC, ENABLE);

	  HAL_DMA_Init(&hdma_tx);

	  /* Associate the initialized DMA handle to the the UART handle */
	  //__HAL_LINKDMA(SpiHandle, hdmatx, hdma_tx);

	  UART_Handler.hdmatx = &hdma_tx;
	  hdma_tx.Parent = &UART_Handler;

	  /*##-4- Configure the NVIC for DMA #########################################*/
	  /* NVIC configuration for DMA transfer complete interrupt (UART1_TX) */
	  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 10, 1);
	  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

	  NVIC_SetVector(DMA2_Stream7_IRQn, (uint32_t)&UART1_DMA_TX_IRQHandler);

	  /* initialise UART_Buffer	*/
	  uart_tx_buffer = pvPortMalloc(sizeof(uint8_t)* 100);
	  //HAL_UART_Transmit(&UART_Handler, "asd", 3, 10);
	  //esp_send("test");

	  /* start usart sender task	*/
//	  xTaskCreate( (void *) &UART_Tx_Task, (const signed char *) "USART", USART_TX_TASK_STACK_SIZE, NULL, USART_TX_TASK_PRIORITY, NULL );
//	  USART_Tx_Queue = xQueueCreate(20, sizeof(char[100]));
}



/*
 * Task for processing UART data and adding new data to a data queue
 */
void UART_Processor( void ){
  char new_data[100];

  for(;;){
      if(xQueueReceive(Data_Queue, &new_data, 10) && new_data[0] != '\r'){
        debug_printf("LINE RX: %s\n", new_data);
        //We have new data analyze it
        if(strncmp(&(new_data[0]), "+IPD", 4) == 0){
          BRD_LEDToggle();
        	debug_printf("1: %s\n", new_data);
          debug_printf("Data: %s\n", &(new_data[5]));
          handle_data(new_data+5);

        } else if(strncmp(&(new_data[0]), "OK", 2) == 0 || strncmp(&(new_data[0]), "ready", 5) == 0
    || strncmp(&(new_data[0]), "no change", 9) == 0 || strncmp(&(new_data[0]), "SEND OK", 7) == 0) {
          //Set the last task passed flag
          lastTaskPassed = TRUE;

        } else if(new_data[0] == '>'){
          prompt = TRUE;

        } else if (strncmp(new_data, "192.168", 7) == 0) {
        	memset(ip_addr_string, 0, 20);
			    memcpy(ip_addr_string, new_data, 20);
			    debug_printf("IP Address: %s\n", ip_addr_string);

  	    } else if (strncmp(new_data, "+CWLAP:", 7) == 0){
  				handle_Access_Point(new_data);
  	    }
      }
      vTaskDelay(1);
  }
}

/*
 * Usart_1 interrupt
 */
void UART1_IRQHandler(void)
{
	uint8_t c;
	 //check data available
    if ((USART1->SR & USART_FLAG_RXNE) != (uint16_t)RESET) {
    	// clear the flag
    	__HAL_USART_CLEAR_FLAG(&UART_Handler, USART_IT_RXNE);
    	c = USART1->DR & 0xFF;		/* don't need no HAL */

    	//logic
    	// if not \n or \r add to line buffer
    	//if \n or \r & line buffer not empty -> put line buffer on queue
    	//
      //
  	  //add to queue

    	if (c != '\n' && c != '\r') {
    		line_buffer[line_buffer_index] = c;
    		line_buffer_index++;
    	} else if (index != 0) {
    			xQueueSendToBackFromISR(Data_Queue, line_buffer, ( BaseType_t* ) 4 );
    			// clear line buffer
    			memset(line_buffer, 0, 100);
    			line_buffer_index = 0;
    	}
    } else {	// cleanup other flags
    	HAL_UART_IRQHandler((UART_HandleTypeDef *)&UART_Handler);
    }
}



/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void UART1_DMA_TX_IRQHandler(void) {
	HAL_DMA_IRQHandler(UART_Handler.hdmatx);
  	xSemaphoreGiveFromISR(USART1_Semaphore, NULL);
}


uint8_t esp_send(uint8_t* send_String) {
	if (USART1_Semaphore != NULL) {
		if( xSemaphoreTake( USART1_Semaphore, ( TickType_t ) 1000 ) == pdTRUE ) {
			uint8_t l = strlen(send_String);
			memcpy(uart_tx_buffer, send_String,  l);
			if (HAL_UART_Transmit_DMA(&UART_Handler, (uint8_t*)uart_tx_buffer, l) != HAL_OK) {
				return 0;
			}
			return 1;
		}
	}
	return 0;
}

void handle_Access_Point (char* apString) { //(0,"Visitor-UQconnect",-71,"00:25:45:a2:ea:92",6)
	char zero = 0;
	char* essid = pvPortMalloc(sizeof(char)*30);
	char rssi[3];
	int rssii = 0;
	char* bssid = pvPortMalloc(sizeof(char)*30);
	char channel[5];

	sscanf(apString, "+CWLAP:(%c,\"%[^\"]\",-%[^,],\"%[^\"]\",%[^)])", zero, essid, rssi, bssid, channel);

	Access_Point* access_Point = pvPortMalloc(sizeof(Access_Point));
	access_Point->RSSI = atoi(rssi);
	access_Point->channel = atoi(channel);
	access_Point->ESSID = pvPortMalloc(sizeof(char)*30);
	access_Point->BSSID = pvPortMalloc(sizeof(char)*30);

	memcpy(access_Point->ESSID, essid, 30);
	memcpy(access_Point->BSSID, bssid, 30);

	add_AP(access_Point);

	rssii = atoi(rssi);

	vPortFree(essid);
	vPortFree(bssid);
 }


void handle_data(char* data) {
	uint8_t pipestr[10], lengthstr[10];
	uint8_t pipe_no = 0, length = 0;
	char message[50];
	char* raw_message = pvPortMalloc(sizeof(uint8_t)*50);
	memset(message, 0, 50);
	char trash;

	sscanf(data, "%[^,],%[^:]:%s", pipestr, lengthstr, message);
	pipe_no = atoi(pipestr);
	length = atoi(lengthstr);
	raw_message = strcpy(raw_message, message);

	xQueueSendToBack(Message_Queue, &data, 10);

	debug_printf("Sent message to queue\n");

	vPortFree(raw_message);
}

void handle_Messages(uint8_t pipe_no, uint8_t* message, uint8_t* raw_data) {
	uint8_t l = 0, dest, source, type;
	uint8_t* data_String = pvPortMalloc(sizeof(uint8_t)*30);
	uint8_t c, i;
	uint8_t ack_String[20];
	if (*message < 0x30 || *message > 0x39 || (l = strlen(message)) < 4) {
		debug_printf("Invalid data: %s\n\r", message);
		return;
	}
	dest = *(message++) - '0';
	source = *(message++) - '0';
	type = *(message++) - '0';
	debug_printf("Pipe: %d\n\r", pipe_no);
	debug_printf("Destination: %d\n\r", dest);
	debug_printf("Source: %d\n\r", source);
	debug_printf("Type: %d\n\r", type);
	i = 0;
	do {
		c = *(message++);
		if (c != ']') {
			*(data_String+(i++)) = c;

		} else {
			*(data_String+(i++)) = 0;
			break;
		}
	} while (c);

	debug_printf("Data received: %s\n\r", data_String);

	sprintf(ack_String, "DA:[%d%d7ACK]", source, NODE_ID);
	//Wifi_senddata(pipe_no, ack_String, strlen(ack_String));

	/* 	setup client	*/
	if (source == 0) {
		client_Pipe = pipe_no;
		debug_printf("client connected\n\r");
		/*	reply with same data	*/

	}


	if ( type == 5) {
		//handle_Ultrasonic_Data(source, data_String, raw_data);

	}
	if ( type == 4) {
		handle_RSSI_Data(source, data_String,  raw_data);
	}
	vPortFree(data_String);

}


void handle_RSSI_Data(uint8_t node, uint8_t* data_String,  uint8_t* raw_data) {
	uint8_t data[30];
	debug_printf("RSSI data from Node: %d - %d\n\r", node, atoi(data_String));

	/* forward to client	*/
	if (client_Pipe >= 0 && client_Pipe <= 4) {
		Wifi_senddata(client_Pipe, raw_data, 10);
		/* HACK HERE	*/
	}
}

//############################ HELPER FUNCTIONS ###############################


void waitForPassed(int timeout){
  while(!lastTaskPassed && --timeout){
    vTaskDelay(1);
  }

  if(timeout == 0){
    debug_printf("Failure.\n\n");
  } else {
    debug_printf("Success.\n\n");
  }

  lastTaskPassed = FALSE;
}

void waitForPrompt(){
  while(!prompt){
    vTaskDelay(100);
  }
  prompt = FALSE;
}

/* Resets the wifi module */
void Wifi_reset(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  char command[20] = WIFI_CMD_RST;

  debug_printf("Reseting module... Please wait\n");

  //HAL_UART_Transmit(&UART_Handler, &(command[0]), WIFI_LEN_RST, 10);
  esp_send(command);
  waitForPassed(5000);

  waitForPassed(5000);
		}
	xSemaphoreGive(esp_Semaphore);
	}
}

/* Joins my home network */
void Wifi_join(char SSID[50], char password[50]){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
			char command[50];
			int len = 0;
			len = sprintf(&(command[0]), WIFI_CMD_JOIN, SSID, password);

			debug_printf("Joining network\n");

			//HAL_UART_Transmit(&UART_Handler, &(command[0]), len, 10);
			esp_send(( uint8_t* )command);
			waitForPassed(5000);
		}
	xSemaphoreGive(esp_Semaphore);
	}
}

/* Currently sets mode to 3 -Both AP and ST) */
void Wifi_setmode(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
			char command[50] = WIFI_CMD_MODE_BOTH;
			debug_printf("Setting module mode\n");

			//HAL_UART_Transmit(&UART_Handler, &(command[0]), WIFI_LEN_MODE_BOTH, 10);
			esp_send(command);
			waitForPassed(5000);
		}
	xSemaphoreGive(esp_Semaphore);
	}
}

/* Lists the AP names in return type
 * PROBLEMS
 * ===============
 * - For some reason the uart receive code wont work. Just gives 'A' once
 * @unfinsihed
 */
void Wifi_listAPs(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  char command[50] = WIFI_CMD_LIST_APS;

  //HAL_UART_Transmit(&UART_Handler, &(command[0]), WIFI_LEN_LIST_APS, 10);
  esp_send(command);
  debug_printf("Getting AP Names\n");

  waitForPassed(5000);

  vTaskDelay(1000);
		}
			xSemaphoreGive(esp_Semaphore);
			}
}

/* Sends the status command
 * @unfinished
 */
void Wifi_status(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  char command[50] = WIFI_CMD_STATUS;
  //HAL_UART_Transmit(&UART_Handler, &(command[0]), WIFI_LEN_STATUS, 10);
  esp_send(command);
		}
			xSemaphoreGive(esp_Semaphore);
			}
}

/* Sets the wifi ap
 * @param sec 0 for no password
 */
void Wifi_setAP(char SSID[50], char password[50], uint8_t chan, uint8_t sec){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  char command[50];
  int len;

  debug_printf("Setting AP details (probably crashing the wifi)\n");

  len = sprintf(&(command[0]), WIFI_CMD_SET_AP, SSID, password, chan, sec);
  //HAL_UART_Transmit(&UART_Handler, &(command[0]), len, 10);
  esp_send(command);
  waitForPassed(5000);
		}
			xSemaphoreGive(esp_Semaphore);
			}
}

/* Checks the IP address */
void Wifi_checkcon(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  char command[50] = "AT+CWJAP\n\r";
  //HAL_UART_Transmit(&UART_Handler, &(command[0]), 12, 10);
  esp_send(command);
		}
			xSemaphoreGive(esp_Semaphore);
			}
}

void Wifi_get_station_IP(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  char command[50] = WIFI_CMD_GET_IP_STA;
  //HAL_UART_Transmit(&UART_Handler, &(command[0]), WIFI_LEN_GET_IP_STA, 10);
  esp_send(command);
  waitForPassed(5000);
		}
			xSemaphoreGive(esp_Semaphore);
			}
}

void Wifi_get_AP_IP(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  char command[50] = WIFI_CMD_GET_IP_AP;
  //HAL_UART_Transmit(&UART_Handler, &(command[0]), WIFI_LEN_GET_IP_AP, 10);
  esp_send(command);
  waitForPassed(5000);
		}
			xSemaphoreGive(esp_Semaphore);
			}
}

 void Wifi_set_station_IP(char* IP_Addr){
	 if (esp_Semaphore != NULL) {
		 if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
 	int len;
 	char command[50];

 	len = sprintf(command, WIFI_CMD_SET_IP_STA, IP_Addr);
 	//HAL_UART_Transmit(&UART_Handler, command, len, 10);
 	esp_send(command);
   waitForPassed(5000);
		 }
		 	xSemaphoreGive(esp_Semaphore);
		 	}
 }

 void Wifi_set_AP_IP(char* IP_Addr){
	 if (esp_Semaphore != NULL) {
		 if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
 	int len;
 	char command[50];

 	len = sprintf(command, WIFI_CMD_SET_IP_AP, IP_Addr);
 	//HAL_UART_Transmit(&UART_Handler, command, len, 10);
 	esp_send(command);
   waitForPassed(5000);
		 }
		 	xSemaphoreGive(esp_Semaphore);
		 	}
 }

/*
 * Enables a TCP server on port 8888
 */
void Wifi_enserver(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  char command[50] = WIFI_CMD_MUX_1;

  debug_printf("Enabling a server on 8888\n");

  //Set MUX to 1
  //HAL_UART_Transmit(&UART_Handler, &(command[0]), WIFI_LEN_MUX_1, 10);
  esp_send(command);
  waitForPassed(5000);

  //Enable the TCP server on 8888
  memcpy(&(command[0]), WIFI_CMD_SERVE, WIFI_LEN_SERVE);
  //HAL_UART_Transmit(&UART_Handler, &(command[0]), WIFI_LEN_SERVE, 10);
  esp_send(command);
  waitForPassed(5000);
		}
			xSemaphoreGive(esp_Semaphore);
			}
}

void Wifi_connecttest(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
 //HAL_UART_Transmit(&UART_Handler, "AT+CIPSTART=0,\"TCP\",\"192.168.4.1\",8888\r\n", 40, 10);
	esp_send("AT+CIPSTART=0,\"TCP\",\"192.168.4.1\",8888\r\n");
  waitForPassed(5000);
		}
			xSemaphoreGive(esp_Semaphore);
			}
}

void Wifi_checkfirmware(){
	if (esp_Semaphore != NULL) {
		if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  //HAL_UART_Transmit(&UART_Handler, "AT+GMR\r\n", 8, 10);
	esp_send("AT+GMR\r\n");
  waitForPassed(5000);
		}
			xSemaphoreGive(esp_Semaphore);
			}
}

void Wifi_connectTCP( char ip[50], int port){
	if (esp_Semaphore != NULL) {
	if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
  char command[50];
  int len = sprintf(command, "AT+CIPSTART=0,\"TCP\",\"%s\",%d\r\n", ip, 8888);
  //HAL_UART_Transmit(&UART_Handler, command, len, 10);
  esp_send(command);
  waitForPassed(5000);
	}
		xSemaphoreGive(esp_Semaphore);
		}
}

void Wifi_senddata(uint8_t pipe_no, char data[50], int length){
	if (esp_Semaphore != NULL) {
			if( xSemaphoreTake( esp_Semaphore, ( TickType_t ) 10 ) == pdTRUE ) {
				  char command[50];
				  char send_data[50];
				//  char tmp[20];
				//
				//  int len = sprintf(tmp, WIFI_CMD_SEND_DATA, length);
				  int len = sprintf(command, WIFI_CMD_SEND_DATA, pipe_no, length);

				  debug_printf("Sending data\n");

				  //HAL_UART_Transmit(&UART_Handler, &(command[0]), len, 10);
				  esp_send(command);
				  vTaskDelay(500);

				  len = sprintf(send_data, "%s\n\r", data);

				  //HAL_UART_Transmit(&UART_Handler, send_data, len, 10);
				  esp_send(send_data);
				  waitForPassed(5000);
				 xSemaphoreGive(esp_Semaphore);
			}
		}
}

void Wifi_timesync(){
  char data[25];
  int len = sprintf(data, "TS:[%d]", xTaskGetTickCount() + 100);
  Wifi_senddata(0, data, len);
}


/* Returns the distance in meters */
float RSSItoDistance(int rssi){
  return 0.0039*rssi*rssi - 0.0935*rssi + 0.6208;
}


/*      AP list helpers     */
void index_Add_AP(Access_Point* access_Point, uint8_t index) {
    uint8_t i = 0;
    if (index == 0) {
        // add to start
    } else {
        for ( i = 0; i < index+1; i++) {

        }
    }
}


Access_Point* get_AP(char* essid) {
	int i;
	Access_Point* current_AP = Access_Points->HEAD;
	for ( i = 0; i < Access_Points->size; i++) {
		if (strncmp(current_AP->ESSID, essid, 30) == 0) {
			return current_AP;
		}
		current_AP = current_AP->next;
	}
	return NULL;
}

/*
 * @Brief adds access points to start of the list, removes old entries
 * @param Access_Point* access_Point
 */
 void add_AP(Access_Point* access_Point) {
    remove_AP(access_Point->BSSID);
    /*  always add to start of list	*/
    if (Access_Points->size == 0) {
    	access_Point->next = NULL;
    	access_Point->prev = NULL;
    	Access_Points->HEAD = access_Point;
    	Access_Points->TAIL = access_Point;
    } else {
    	access_Point->next = Access_Points->HEAD;
    	access_Point->prev = NULL;
    	Access_Points->HEAD->prev = access_Point;
    	Access_Points->HEAD = access_Point;
    }
    Access_Points->size++;
 }

 /*
  * @Brief removes access point absed on BSSID
  * @Param char* BSSID   30 characters!!
  */
 void remove_AP(char* BSSID) {
     uint8_t i = 0;
     if (Access_Points->size != 0) {
		Access_Point* current_AP = Access_Points->HEAD;
		for ( i = 0; i < Access_Points->size; i++) {
			if (strncmp(current_AP->BSSID, BSSID, 30) == 0) {
				/*		remove from linked list		*/
				if (current_AP->next != NULL) {
					//Access_Point* compilerIsWrong  = current_AP->next;
					current_AP->next->prev = current_AP->prev;
				}
				if (current_AP->prev != NULL) {
					//Access_Point* compilerIsWrong2  = current_AP->prev;
					 current_AP->prev->next = current_AP->next;
				}
				if (Access_Points->TAIL == current_AP) {
					if (current_AP->prev != NULL) {
						Access_Points->TAIL = current_AP->prev;
					}
				}
				if (Access_Points->HEAD == current_AP) {
					if (current_AP->next != NULL) {
						Access_Points->HEAD = current_AP->next;
					}
				}
				Access_Points->size--;
				vPortFree(current_AP->BSSID);
				vPortFree(current_AP->ESSID);
				vPortFree(current_AP);
			}
			current_AP = current_AP->next;
		}

     }
 }
