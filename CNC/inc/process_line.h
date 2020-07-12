#ifndef _PROCESS_LINE__H_
#define _PROCESS_LINE__H_


#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "gcode.h"
#include "sapi.h"
#include "task.h"
#include "queue.h"
#include "config.h"
#include "uart.h"

/**
* @brief freeRTOS Task, esta tarea se encarga de procesar las sentencias
* de código G que llegan por puerto serie
* @param parameters not used
* @return nothing
*/
void processGcodeLineTask(void *parameters);


//void process_line(char *rxLine);
int read_number(char *rxLine, uint8_t *counter, float *number);


#endif
