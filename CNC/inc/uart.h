#ifndef _UART__H_
#define _UART__H_

#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"
#include "config.h"

/**
* @brief Tarea de FreeRTOS encargada de leeer el buffer del puerto serie.
* La interrupcion de UARTRx desbloqueará esta tarea
* @return
*/
void uartProcessRxEventTask(void * taskParamPrt);


/**
* @brief UART rx Callback
* @param
* @return
*/
void onRx(void *);

#endif
