#ifndef _TEST_UTILS__H_
#define _TEST_UTILS__H_

#include <stdint.h>
#include <stdio.h>

#define uartConfig uartInit

#define true 1
#define false 0
//#define NULL 0

typedef int TaskHandle_t;
typedef int TickType_t;
typedef long BaseType_t;
//typedef unsigned int uint8_t;
//typedef int int8_t;
typedef unsigned int uint32_t;

#define pdFALSE false
#define pdTRUE	true

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
void xTaskNotifyGive(TaskHandle_t);
void vTaskNotifyGiveFromISR(TaskHandle_t, BaseType_t *);
void portYIELD_FROM_ISR(BaseType_t);
TickType_t pdMS_TO_TICKS(unsigned long);
#endif
