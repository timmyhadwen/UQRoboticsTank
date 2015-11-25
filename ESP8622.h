typedef struct{
  char message[50];
} UART_Message;

QueueHandle_t Message_Queue;

#define WIFI_CMD_RST        "AT+RST\r\n"
#define WIFI_LEN_RST 9

#define WIFI_CMD_MODE_STA   "AT+CWMODE=1\r\n"

#define WIFI_CMD_MODE_AP    "AT+CWMODE=2\r\n"

#define WIFI_CMD_MODE_BOTH  "AT+CWMODE=3\r\n"
#define WIFI_LEN_MODE_BOTH 13

#define WIFI_CMD_JOIN "AT+CWJAP=\"%s\",\"%s\"\r\n"

#define WIFI_CMD_QUIT_AP "AT+CWQAP\r\n"
#define WIFI_LEN_QUIT_AP 10

#define WIFI_CMD_START_TCP "AT+CIPSTART=0,\"TCP\",\"%d.%d.%d.%d\",%d\r\n"

#define WIFI_CMD_SET_AP "AT+CWSAP=\"%s\",\"%s\",3,0\n\r"

#define WIFI_CMD_SERVE "AT+CIPSERVER=1,8888\r\n"
#define WIFI_LEN_SERVE 21

#define WIFI_CMD_MUX_1 "AT+CIPMUX=1\r\n"
#define WIFI_LEN_MUX_1 13

#define WIFI_CMD_LIST_APS "AT+CWLAP\r\n"
#define WIFI_LEN_LIST_APS 10

#define WIFI_CMD_STATUS "AT+CIPSTATUS\r\n"
#define WIFI_LEN_STATUS 14

#define WIFI_CMD_GET_IP_STA "AT+CIPSTA?\r\n"
#define WIFI_LEN_GET_IP_STA 11

#define WIFI_CMD_GET_IP_AP "AT+CIPAP?\r\n"
#define WIFI_LEN_GET_IP_AP 11

#define WIFI_CMD_SET_IP_STA "AT+CIPSTA=\"%s\"\r\n"
#define WIFI_CMD_SET_IP_AP "AT+CIPAP=\"%s\"\r\n"

#define WIFI_CMD_SEND_DATA "AT+CIPSEND=%d,%d\r\n"

#define mainLED_PRIORITY					( tskIDLE_PRIORITY + 2 )
#define mainLED_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

#define SEC 0x7FFF00
#define task_loop for(;;)





typedef struct Ap Access_Point;
// Datatypes
struct Ap {
	int channel;
	char* ESSID;
	char* BSSID;
	int RSSI;	// RSSI as negative
	Access_Point* next;
	Access_Point* prev;
};



typedef struct Access_Points {
	Access_Point* AP;
	Access_Point* HEAD;
	Access_Point* TAIL;
	uint16_t size;
} APs;


// Tasks
void UART_Processor( void );

// Starting functions
void ESP8622_init( void );
void dma_Init( void );
void ESP8622_send_test( void );


// Library functions
void Wifi_reset( void );
void Wifi_join(char SSID[50], char password[50]);
void Wifi_setmode( void );
void Wifi_listAPs( void );
void Wifi_status( void );
void Wifi_senddata(uint8_t pipe_no, char data[50], int length);
void Wifi_checkfirmware();
void Wifi_connectTCP( char ip[50], int port);
void Wifi_timesync();
void handle_RSSI_Data(uint8_t node, uint8_t* data_String,  uint8_t* raw_data);


void Wifi_get_station_IP();
void Wifi_get_AP_IP();
void Wifi_set_station_IP(char* IP_Addr);
void Wifi_set_AP_IP(char* IP_Addr);



//linked list helpers
void remove_AP(char*);
void add_AP(Access_Point*);


// Helpers
uint8_t esp_Send(uint8_t*);
void waitForPassed(int timeout);
void waitForPrompt();
void Delay(int x);
float RSSItoDistance(int rssi);
void handle_Access_Point (char* apString);
void handle_data(char* data);
void handle_Messages(uint8_t pipe_no, uint8_t* message, uint8_t* raw_data);


// Interrupts
void UART1_IRQHandler(void);
void UART1_DMA_TX_IRQHandler(void);
