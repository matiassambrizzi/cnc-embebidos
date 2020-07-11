#include "uart.h"

TaskHandle_t xHandleProcessLine = NULL;
TaskHandle_t xHandleUART = NULL;
// Variable global que almacena las lineas
char rx_line[15];

void uartProcessRxEventTask(void * taskParamPrt)
{
	char rx_char;
	uint8_t i = 0;
	uint32_t ulEventsToProcess;
	//Es buena práctica poner un tiempo de espera del semáforo
	const TickType_t maxExpectedBlockTime = pdMS_TO_TICKS(500UL);
	//Si todavía no lei la linea entonces
	//no puedo leer ??

	while(1) {

		ulEventsToProcess = ulTaskNotifyTake(pdTRUE, maxExpectedBlockTime);

		if(ulEventsToProcess) {
			//Si estoy aca es porque la interrupción del UART
			//se activo y dio el semáforo
			while(uartRxReady(UART_PORT)) {
				//Si estoy aca es por que quedan datos
				//en el fifo del uart
				if((rx_char = uartRxRead(UART_PORT)) != ' ') {
					if((rx_line[i] = rx_char) == '\n' || i > 15){
						rx_line[i] = '\0';
						i = 0;
						// Aviso que se leyó una linea
						// Cambiar esto por una cola de
						// strings ?
						xTaskNotifyGive(xHandleProcessLine);
						//Tengo que guardar esta linea
						//en una cola para poder
						//procesarla luego
					} else {
						i++;
					}
				}

			}
			//Habilito otra vez la interrupcion del uart
			uartInterrupt(UART_PORT, true);
		} else {

		}

	}
}

void onRx(void *noUso)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(xHandleUART, &xHigherPriorityTaskWoken);
	uartInterrupt(UART_PORT, false);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
