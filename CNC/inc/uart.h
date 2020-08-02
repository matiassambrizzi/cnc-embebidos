#ifndef _UART__H_
#define _UART__H_

#ifndef TEST_ALL
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"
#endif

#include "config.h"

#ifdef TEST_ALL
#include "test_utils.h"
#endif

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
/**
* @brief Función para indicarle al encargado de enviar los codigos G que el
* buffer no está lleno y que puede mandar las proximas sentencias
* @return nothing
*/
void ready_to_process(void);

/**
* @brief
* @param
* @return
*/
int8_t uart_get_buffer(char *s);

/**
* @brief Funcion para guarar caracteres que están almacenados en el fifo del
* uart en la variables global rx_line.
* @param
* @return
*/
void uart_fill_in_buff();



#endif
