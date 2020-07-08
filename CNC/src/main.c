/*
 * Prubea de control de motores usando PWM
 * Se conecto el pin STEP del driver del motor
 * al pin T_FIL1 de la EDUCIAA, que se corresponde con el PWM0
 * se utilizó el GPIO1 para configurar la dirección del stepper
*/
#include "sapi.h"
#include "sapi_delay.h"
#include <stdlib.h>
#include <stdio.h>

#define STEPS_PER_REV		200
#define STEPS_PER_MM		200 // Tardo 200 pasos en hacer 1mm
#define FREQ			1000
#define XAXIS			PWM0
#define MOTOR_DIR		GPIO1

typedef struct {
	float px;
	float py;
	float pz;
} axis_t;

//Gloabal Actual Position
static axis_t actual_pos = {.px=0,.py=0,.pz=0};
//Global Future Movment, buffer?
static axis_t future_pos = {.px=0,.py=0,.pz=0};
//configs?
// - Type of movment
// - Speed
// - pos to fo

volatile char rxLine[15];
volatile uint8_t count = 0;
volatile bool_t receivedLine = false;

void motorMoveSteps(uint32_t nsteps, uint8_t axis, bool_t dir);
void process_line(char *rxLine);
int read_number(char *rxLine, size_t *counter, float *number);
bool_t isLetter(const uint8_t c);
bool_t isNumber(const uint8_t c);
void line_move(float newx, float newy, float newz);
float my_max(float x, float y, float z);
void onRx(void *);

int main(void)
{
	// Set DIR pin as OUTPUT
	gpioInit(GPIO1, GPIO_OUTPUT);
	gpioInit(GPIO3, GPIO_OUTPUT);
	boardInit();
	//UART config
	uartConfig(UART_USB, 115200);
	uartCallbackSet(UART_USB, UART_RECEIVE, onRx, NULL);
	uartInterrupt(UART_USB, true);

	while(1) {
		/*
		delay(1000);
		motorMoveSteps(200, XAXIS, 0);
		delay(1000);
		*/
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


void motorMoveSteps(uint32_t nsteps, const uint8_t axis, const bool_t dir)
{
	gpioWrite(MOTOR_DIR, dir);
	gpioWrite(GPIO3, 1);
	gpioWrite(GPIO3, 0);
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



/*
 * @brief funcion calcular los puntos intermedios entre una variable global que
 * guarda registro de la posición actual y los nuevos parámetros que se le pasen
 * @param newx nuevo valor que quiero en x
 * @param newy nuevo valor que quiero en y
 * @param newz nuevo valor que quiero en z
 * TODO: Esto está implementado solo para 1 eje
 */
void line_move(float newx, float newy, float newz)
{
	int32_t delta_x, delta_y, delta_z;
	uint32_t max_steps = 0;
	//Calculo los delta de movimiento
	delta_x = (newx - actual_pos.px) * STEPS_PER_MM; //steps
	delta_y = (newy - actual_pos.py) * STEPS_PER_MM; //steps
	delta_z = (newz - actual_pos.pz) * STEPS_PER_MM; //steps

	uint8_t dir_x = (delta_x < 0) ? 0:1;
	uint8_t dir_y = (delta_y < 0) ? 0:1;
	uint8_t dir_z = (delta_z < 0) ? 0:1;

	//Me olvido del signo, esta guardado en dir_xx
	delta_x = (delta_x < 0) ? -delta_x : delta_x;
	delta_y = (delta_y < 0) ? -delta_y : delta_y;
	delta_z = (delta_z < 0) ? -delta_z : delta_z;

	//Calculo las iteraciones
	max_steps = my_max(delta_x, delta_y, delta_z);

	uint32_t over = 0;

	// Hago las iteraciones
	for(uint32_t i=0;i<max_steps; i++) {
		over += delta_x;
		if(over >= max_steps) {
			over -= max_steps;
			// Muevo el motor 1 paso
			motorMoveSteps((uint32_t) 1, XAXIS, dir_x);
		}
		//VER COMO CONFIGURAR ESTO
		//Si bajo este delay voy más rápido
		//pero tengo que darle mas corriente al motor
		delayInaccurateUs(500);
	}

	actual_pos.px = newx;
	actual_pos.py = newy;
	actual_pos.pz = newz;
}

float my_max(float x, float y, float z)
{
	return (x > y) ? ((x > z) ? x : z) : ((y > z) ? y : z);
}















