/*
 * Prubea de control de motores usando PWM
 * Se conecto el pin STEP del driver del motor
 * al pin T_FIL1 de la EDUCIAA, que se corresponde con el PWM0
 * se utilizó el GPIO1 para configurar la dirección del stepper
*/
#include "sapi.h"
#include "sapi_delay.h"
#include "sapi_gpio.h"
#include "sapi_peripheral_map.h"
#include "sapi_pwm.h"
#include "sapi_uart.h"
#include <cstdint>

#define STEPS_PER_REV		200
#define FREQ			1000
#define XAXIS			PWM0
#define MOTOR_DIR		GPIO1

volatile char rxLine[15];
volatile uint8_t count = 0;
volatile bool_t receivedLine = false;

void motorMoveSteps(uint32_t nsteps, uint8_t axis, bool_t dir);
void process_line(const char *rxLine);
void read_number(const char *rxLine, size_t *counter, float *number);
void onRx(void *);

int main(void)
{
	// Set DIR pin as OUTPUT
	gpioInit(GPIO1, GPIO_OUTPUT);

	//UART config
	uartConfig(UART_USB, 115200);
	uartCallbackSet(UART_USB, UART_RECEIVE, onRx, NULL);
	uartInterrupt(UART_USB, true);

	//Init pwm with a frec of 1khz
	pwmConfig(0, PWM_ENABLE);
	pwmConfig(XAXIS, PWM_ENABLE_OUTPUT);
	pwmWrite(XAXIS, 255);

	while(1) {
		//delay(1000);
		//motorMoveSteps(10, XAXIS, 0);
		//delay(1000);
		//motorMoveSteps(10, XAXIS, 1);
		if(receivedLine) {
			receivedLine = false;
			uartWriteString(UART_USB, (char *)rxLine);
			//aca tengo que procesar la linea
			//process_line(rxLine);
		}
	}

	return 0;
}


void motorMoveSteps(uint32_t nsteps, const uint8_t axis, const bool_t dir)
{
	gpioWrite(MOTOR_DIR, dir);
	pwmWrite(axis, 125);
	for(;nsteps>0;nsteps--) {
		delay(200);
	}
	pwmWrite(axis, 255);
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
 *
 */
void process_line(const char *rxLine)
{
	size_t counter = 0;
	char letter;
	float number;

	//Check for EOL
	while(rxLine[counter] != '\0') {

		letter = rxLine[counter];
		read_number(rxLine, counter, &number);

	switch(letter) {

		case 'G':
			//Si es un código G entonces tengo que ver que tipo es

			break;

		default:
			;
		}
	}


}


/*
 * @brief Funcion que recibe una linea de código G y se queda con el numero
 * que identifia al código
 * @param rxLine es la linea
 * */
void read_number(const char *rxLine, size_t *counter, float *number)
{
	char *iterator = rxLine + *counter;
	bool_t isdecimal = false;
	uint8_t c;
	uint32_t intvalue = 0;

	// No chequeo el signo porque no hay singo (?
	// Decimal o float?
	
	// Ver que hacer con los decimales
	while(!isLetter((c = *iterator++))) {
		c-='0';
		if(c >= 0 && c <= 9) {
			//We have a number
			//aca hay que hacer inVal*10 + c
			intvalue = intvalue*10 + c;
		} else if(c == '.'-'0') {
			isdecimal = true;
		}
	}
}


bool_t isLetter(const uint8_t c) {
	return (c >= 'A' && c <= 'Z');
}
