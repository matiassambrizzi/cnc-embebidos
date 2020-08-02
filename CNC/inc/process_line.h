#ifndef _PROCESS_LINE__H_
#define _PROCESS_LINE__H_


#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include "config.h"
#include "uart.h"
#include "types.h"
#include "position.h"
#include "gcode.h"
#include "motion.h"


#ifndef TEST_ALL
#include "FreeRTOS.h"
#include "sapi.h"
#include "task.h"
#include "queue.h"
#endif

#ifdef TEST_ALL
#include "test_utils.h"
#endif

extern char rx_line[MAX_RX_BUFFER];
extern QueueHandle_t xPointsQueue;

/**
* @brief freeRTOS Task, esta tarea se encarga de procesar las sentencias
* de código G que llegan por puerto serie
* @param parameters not used
* @return nothing
*/
void processGcodeLineTask(void *parameters);


int read_number(char *rxLine, uint8_t *counter, float *number);

float stringToFloat(char *);

#endif
