#include "uart.h"


/*
 * Variables globales
 * ==================
 */

TaskHandle_t xHandleProcessLine;
TaskHandle_t xHandleUART;
// Variable global que almacena las lineas
//
//

char rx_line[MAX_RX_BUFFER];
uint8_t count=0;
/*
 * Implementación de funciones públicas
 * ====================================
 */

void uart_config()
{
	uartConfig(UART_PORT, COM_BAUDRATE);
	uartCallbackSet(UART_PORT, UART_RECEIVE, onRx, NULL);
	uartInterrupt(UART_PORT, true);
}


void uartProcessRxEventTask(void * taskParamPrt)
{
	char rx_char;
	//uint8_t i = 0;
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
			uart_fill_in_buff();
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


void ready_to_process()
{
	//Bloking Send Byte
#ifdef DEBUG
	printf("Sending $$$\n");
#endif
	uartWriteByte(UART_PORT, READY_TO_PROCESS_CHAR);
	uartWriteByte(UART_PORT, '\n');
}

void uart_fill_in_buff()
{
	char rx_char;

	while(uartRxReady(UART_PORT)) {
		if(count >= MAX_RX_BUFFER) { count = 0; }

		if((rx_char = uartRxRead(UART_PORT)) != ' ') {
			if((rx_line[count] = rx_char) == '\n'){
				rx_line[count] = '\0';
				count = 0;
				xTaskNotifyGive(xHandleProcessLine);
			} else { count++; }
		}
	}
}


int8_t uart_get_buffer(char *s)
{
	for(uint8_t i = 0; i < MAX_RX_BUFFER; i++) {
		s[i] = rx_line[i];
	}
	return 0;
}




