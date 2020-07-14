#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"

#include "sapi.h"
#include "motors.h"
#include "config.h"
#include "gcode.h"
#include "motion.h"
#include "position.h"
#include "types.h"
#include "uart.h"
#include "process_line.h"

extern char rx_line[MAX_RX_BUFFER];
extern TaskHandle_t xHandleProcessLine;
extern QueueHandle_t xPointsQueue;
extern TaskHandle_t xHandleUART;
extern TaskHandle_t xHandleUART;

int main(void)
{
	boardInit();

	motorInit();
	motor_config(MOTOR_X_STEP, MOTOR_X_DIR, END_STOP_X,
		     MOTOR_Y_STEP, MOTOR_Y_DIR, END_STOP_Y,
		     MOTOR_Z_STEP, MOTOR_Z_DIR, END_STOP_Z);

	gcode_block_reset();

	//UART config
	uart_config();

	// Reset acceleration config
	motion_reset();

	xPointsQueue = xQueueCreate(15, sizeof(g_block_t));

	xTaskCreate(uartProcessRxEventTask,
		    (const char*)"MyTaskUart",
		    configMINIMAL_STACK_SIZE*2,
		    NULL, //PARAMETROS
		    tskIDLE_PRIORITY+1,
		    &xHandleUART);

	xTaskCreate(processGcodeLineTask,
		    (const char*)"MytaskPrint",
		    configMINIMAL_STACK_SIZE*2,
		    NULL,
		    tskIDLE_PRIORITY+2,
		    &xHandleProcessLine);

	xTaskCreate(moveMotorsTask,
		    (const char*)"moveMotorsTask",
		    configMINIMAL_STACK_SIZE*4,
		    NULL,
		    tskIDLE_PRIORITY+3,
		    NULL);

	vTaskStartScheduler();

	while(1) { }

	return 0;
}
