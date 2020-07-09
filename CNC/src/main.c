/*
 * Prubea de control de motores usando PWM
 * Se conecto el pin STEP del driver del motor
 * al pin T_FIL1 de la EDUCIAA, que se corresponde con el PWM0
 * se utilizó el GPIO1 para configurar la dirección del stepper
*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "sapi.h"

void onRx(void *);
void myTaskUART(void * taskParamPrt);

//My semáforo
//
SemaphoreHandle_t uartBinarySemaphore;

int main(void)
{
	boardInit();
	//UART config
	uartConfig(UART_USB, 115200);
	uartCallbackSet(UART_USB, UART_RECEIVE, onRx, NULL);
	uartInterrupt(UART_USB, true);

	uartBinarySemaphore = xSemaphoreCreateBinary();

	xTaskCreate(myTaskUART,
		    (const char*)"MyTaskUart",
		    configMINIMAL_STACK_SIZE*2,
		    0, //PARAMETROS
		    tskIDLE_PRIORITY+1,
		    0);

	vTaskStartScheduler();

	while(1) {

	}

	return 0;
}



/*
 * @brief Quiero hacer una tarea que se bloquee hasta que una interrupción
 * del uart de un semáforo. Luego leere todos los caracteres que esten en el
 * buff del uart
 *
 */
void myTaskUART(void * taskParamPrt)
{
	char rx_char;
	char rx_line[15];
	uint8_t i = 0;
	//Es buena práctica poner un tiempo de espera del semáforo
	const TickType_t maxExpectedBlockTime = pdMS_TO_TICKS(500UL);
	//Si todavía no lei la linea entonces
	//no puedo leer ??

	while(1) {

		if(xSemaphoreTake(uartBinarySemaphore, maxExpectedBlockTime)) {
			//Si estoy aca es porque la interrupción del UART
			//se activo y dio el semáforo
			while(uartRxReady(UART_USB)) {
				//Si estoy aca es por que quedan datos
				//en el fifo del uart
				if((rx_char = uartRxRead(UART_USB)) != ' ') {
					if((rx_line[i] = rx_char) == '\n' || i > 15){
						rx_line[i] = '\0';
						i = 0;
						uartWriteString(UART_USB, rx_line);
					} else {
						i++;
					}
				}

			}
			//Habilito otra vez la interrupcion del uart
			uartInterrupt(UART_USB, true);
		} else {

		}

	}
}



void onRx(void *noUso)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(uartBinarySemaphore, &xHigherPriorityTaskWoken);
	uartInterrupt(UART_USB, false);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}







