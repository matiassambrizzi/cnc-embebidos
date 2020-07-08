/*
 * Prubea de control de motores usando PWM
 * Se conecto el pin STEP del driver del motor
 * al pin T_FIL1 de la EDUCIAA, que se corresponde con el PWM0
 * se utilizó el GPIO1 para configurar la dirección del stepper
*/
#include <stdlib.h>
#include <stdio.h>
#include "sapi.h"
#include "config.h"
#include "motors.h"
#include "movment.h"
#include "sapi_delay.h"


//Global Future Movment, buffer?
static axis_t future_pos = {.px=0,.py=0,.pz=0};
//configs?
// - Type of movment
// - Speed
// - pos to fo

volatile char rxLine[15];
volatile uint8_t count = 0;
volatile bool_t receivedLine = false;

void process_line(char *rxLine);
int read_number(char *rxLine, size_t *counter, float *number);
bool_t isLetter(const uint8_t c);
bool_t isNumber(const uint8_t c);
void onRx(void *);

int main(void)
{
	boardInit();
	//init Motors
	gpioInit(MOTOR_X_DIR, GPIO_OUTPUT);
	gpioInit(MOTOR_X_STEP, GPIO_OUTPUT);
	gpioInit(MOTOR_Y_DIR, GPIO_OUTPUT);
	gpioInit(MOTOR_Y_STEP, GPIO_OUTPUT);
	gpioInit(MOTOR_Z_DIR, GPIO_OUTPUT);
	gpioInit(MOTOR_Z_STEP, GPIO_OUTPUT);

	motor_config(MOTOR_X_STEP, MOTOR_X_DIR,
		     MOTOR_Y_STEP, MOTOR_Y_DIR,
		     MOTOR_Z_STEP, MOTOR_Z_DIR);

	//UART config
	//uartConfig(UART_USB, 115200);
	//uartCallbackSet(UART_USB, UART_RECEIVE, onRx, NULL);
	//uartInterrupt(UART_USB, true);

	while(1) {
		if(receivedLine) {
			receivedLine = false;
			uartWriteString(UART_USB, (char *)rxLine);
			uartWriteByte(UART_USB, '\n');
			//aca tengo que procesar la linea
			process_line((char*) rxLine);
			line_move(future_pos.px, 1, 1);
		}
	}

	return 0;
}


void onRx(void *noUso)
{
	char aux_char;
	//Si todavía no lei la linea entonces
	//no puedo leer ??
	if(!receivedLine) {
		aux_char = uartRxRead(UART_USB);
		if(aux_char == ' ') {
			//Discart spaces
			return;
		}
		if((rxLine[count] = aux_char) == '\n') {
			//Agrego /0 al final de la linea
			rxLine[count] = '\0';
			count = 0;
			receivedLine = true;
			return;
		}
		if(count++ >= 15) {count = 0;}
	}
}


/*
 * @brief proceso una linea de codigo G, asumo que no tiene espacios
 * TODO: TEST THIS
 */
void process_line(char *rxLine)
{
	size_t counter = 0;
	char letter;
	float number;
	uint8_t int_val;

	//Check for EOL
	while(rxLine[counter] != '\0') {

		letter = rxLine[counter];
		if((read_number(rxLine, &counter, &number)) != 0) {
			//Error reading number Discart line
			return;
		}

		int_val = (uint8_t) number;

	switch(letter) {

		case 'G':
			switch (int_val) {
			case 0:
				//Fast Move
				uartWriteString(UART_USB, "Movimiento Rapido\r\n");
				break;
			case 1:
				//line move
				uartWriteString(UART_USB, "Movimiento Lineal\r\n");
				break;
			case 2:
				//TODO: Arc Move
				break;
			case 21:
				//TODO mm metric
				break;
			case 28:
				//TODO: Homing
				break;

			case 90:
				uartWriteString(UART_USB, "ModoAbsoluto\r\n");
				//Absolute mode
				break;
			case 91:
				//Relative Mode
				break;
			case 92:
				//Set this pos as 0,0,0
				break;
			default:
				//Do nothing
				;
			}
			//Si es un código G entonces tengo que ver que tipo es
			break;

		case 'X':
			uartWriteString(UART_USB, "Setting X\r\n");
			printf("%d\n", (int) (number*100));
			future_pos.px = number;
			break;
		case 'Y':
			uartWriteString(UART_USB, "Setting Y\r\n");
			printf("%d\n", (int) number);
			future_pos.pz = number;
			break;
		case 'Z':
			uartWriteString(UART_USB, "Setting Z\r\n");
			printf("%d\n", (int) number);
			future_pos.pz = number;
			//Update coordinates
			break;
		case 'F':
			//Updated Speed
			break;

		default:
			//Do nothing
			;
		}
	}

	//grbl ejecutal el comando dentro de esta función

}

/*
 * @brief Funcion que recibe una linea de código G y se queda con el numero
 * que identifia al código
 * @param rxLine es la linea recibida por UART_USB
 * @param counter es un contador en la linea, esta función lo actualiza
 * @param number es el numero que se extrajo
 * TODO: TESTING HISTORY:
 *	- Probe la función con varios floats y los convierte bien
 *	- Agrege un chequeo antes de llamar a ATOF porque esa funcion devuelve 0
 *	si algo male sal
 * */
int read_number(char *rxLine, size_t *counter, float *number)
{
	char *iterator = rxLine + *counter;
	uint8_t i = 0;

	i++;
	// TODO: Si iterator+i no es un número atof() devuelve 0
	// No es seguro.. Hay que fijarse si es un numero de otra forma devolver
	// error
	if(*(iterator+i) < '0' || *(iterator+i) > '9') {
		//Error nan;
		return -1;
	}
	*number = atof(iterator+i);
	//atof is secure ?
	while(!isLetter(*(iterator+i)) && *(iterator+i) != '\0') { i++;}
	//update the counter to the next letter
	//or to the end of the string
	*counter += i;

	//All good
	return 0;
}

//Make this static
bool_t isLetter(const uint8_t c) {
	return (c >= 'A' && c <= 'Z');
}

bool_t isNumber(const uint8_t c) {
	return (c>= '0' && c<= '9');
}








