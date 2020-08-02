#ifndef _TEST_UTILS__H_
#define _TEST_UTILS__H_

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"

#define uartConfig uartInit

#define true 1
#define false 0
//#define NULL 0


extern char rx_line[MAX_RX_BUFFER];

typedef int TaskHandle_t;
typedef int SemaphoreHandle_t;
typedef int QueueHandle_t;
typedef int TickType_t;
typedef long BaseType_t;
//typedef unsigned int uint8_t;
//typedef int int8_t;
typedef unsigned int uint32_t;
#define pdFALSE false
#define pdTRUE	true
#define portMAX_DELAY  ((TickType_t) 10000)
typedef enum {
	   UART_GPIO = 0, // Hardware UART0 via GPIO1(TX), GPIO2(RX) pins on header P0
	   UART_485  = 1, // Hardware UART0 via RS_485 A, B and GND Borns
		// Hardware UART1 not routed
	   UART_USB  = 3, // Hardware UART2 via USB DEBUG port
	   UART_ENET = 4, // Hardware UART2 via ENET_RXD0(TX), ENET_CRS_DV(RX) pins on header P0
	   UART_232  = 5, // Hardware UART3 via 232_RX and 232_tx pins on header P1
} uartMap_t;


typedef enum{
   UART_RECEIVE,
   UART_TRANSMITER_FREE
} uartEvents_t;

typedef uint8_t bool_t;
typedef void (*callBackFuncPtr_t)(void *);

void uartInit( uartMap_t uart, uint32_t baudRate );
void uartCallbackSet( uartMap_t uart, uartEvents_t event,
		      callBackFuncPtr_t callbackFunc, void* callbackParam );
void uartInterrupt( uartMap_t uart, bool_t enable );
uint32_t ulTaskNotifyTake(BaseType_t, TickType_t);
bool_t uartRxReady( uartMap_t uart );
uint8_t uartRxRead( uartMap_t uart );
void uartWriteByte( uartMap_t uart, const uint8_t value );
void uartWriteString( uartMap_t uart, const char* str );
void xTaskNotifyGive(TaskHandle_t);
void vTaskNotifyGiveFromISR(TaskHandle_t, BaseType_t *);
void portYIELD_FROM_ISR(BaseType_t);
TickType_t pdMS_TO_TICKS(unsigned long);


/*=========================[GPIO]===============================*/

typedef enum {

      VCC = -2, GND = -1,
      // P1 header
      T_FIL1,    T_COL2,    T_COL0,    T_FIL2,      T_FIL3,  T_FIL0,     T_COL1,
      CAN_TD,    CAN_RD,    RS232_TXD, RS232_RXD,
      // P2 header
      GPIO8,     GPIO7,     GPIO5,     GPIO3,       GPIO1,
      LCD1,      LCD2,      LCD3,      LCDRS,       LCD4,
      SPI_MISO,
      ENET_TXD1, ENET_TXD0, ENET_MDIO, ENET_CRS_DV, ENET_MDC, ENET_TXEN, ENET_RXD1,
      GPIO6,     GPIO4,     GPIO2,     GPIO0,
      LCDEN,
      SPI_MOSI,
      ENET_RXD0,
      // Switches
      // 36   37     38     39
      TEC1,  TEC2,  TEC3,  TEC4,
      // Leds
      // 40   41     42     43     44     45
      LEDR,  LEDG,  LEDB,  LED1,  LED2,  LED3,
      //#error EDU-CIAA-NXP
} gpioMap_t;

typedef enum {
   GPIO_INPUT, GPIO_OUTPUT,
   GPIO_INPUT_PULLUP, GPIO_INPUT_PULLDOWN,
   GPIO_INPUT_PULLUP_PULLDOWN,
   GPIO_ENABLE
} gpioInit_t;

bool_t gpioWrite( gpioMap_t pin, bool_t value );
bool_t gpioRead( gpioMap_t pin );
bool_t gpioInit( gpioMap_t pin, gpioInit_t config );



/*=========================[QUEUES]===============================*/


void xQueueSend(QueueHandle_t, void *, TickType_t);
void xQueueReceive(QueueHandle_t xQueue, void *const pvBuffer, TickType_t xTicksToWait);
void vPrintString(const char *);
void xQueueReset(QueueHandle_t);

BaseType_t uxQueueSpacesAvailable(const QueueHandle_t xQueue);

double sqrt(double __x);


void vTaskDelay(TickType_t);



char rx_line[MAX_RX_BUFFER];
QueueHandle_t xPointsQueue;


#endif
