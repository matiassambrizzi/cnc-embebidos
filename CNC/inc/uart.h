#ifndef _UART__H_
#define _UART__H_

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"
#include "config.h"

/**
* @brief Configuración del UART. Configura el UART_PORT (ver config.h), setea el
* badurate COM_BAUDATE, activa la interruucpión del perfiferico y en el NVIC y
* setea una función de callback
* @return nothing
*/
void uart_config();
/**
* @brief Tarea de FreeRTOS encargada de leeer el buffer del puerto serie.
* La interrupcion de UARTRx desbloqueará esta tarea
* @return
*/
void uartProcessRxEventTask(void * taskParamPrt);
/**
* @brief UART rx Callback for uart interrupt
* @param not used
* @return nothing
*/
void onRx(void *);





#endif
