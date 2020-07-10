/*
 * Prubea de control de motores usando PWM
 * Se conecto el pin STEP del driver del motor
 * al pin T_FIL1 de la EDUCIAA, que se corresponde con el PWM0
 * se utilizó el GPIO1 para configurar la dirección del stepper
*/

#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "sapi.h"
#include "motors.h"
#include "config.h"

typedef struct {
	float px;
	float py;
	float pz;
} position_t;

typedef struct {
	uint32_t px;
	uint32_t py;
	uint32_t pz;
} steps_t;

void onRx(void *);
void myTaskUART(void * taskParamPrt);
void myTaskPrint(void *parameters);
void moveMotors(void *param);

void line_move(float newx, float newy, float newz);
void fast_move(float newx, float newy, float newz);
void process_line(char *rxLine);
int read_number(char *rxLine, size_t *counter, float *number);
bool_t isLetter(const uint8_t c);
bool_t isNumber(const uint8_t c);

static TaskHandle_t xHandleTask = NULL;
static TaskHandle_t xHandlePrint = NULL;
static QueueHandle_t xPointsQueue = NULL;

// Variable global que almacena las lineas
char rx_line[15];
steps_t actual_pos={.px=0, .py=0, .pz=0};
position_t future_pos = {.px=0, .py=0, .pz=0};

typedef enum {
	FAST_MOVMENT,
	LINE,
	ARC //TODO.
} movment_type_t;

typedef TickType_t speed_t;

typedef struct {
	position_t future;
	movment_type_t type;
	speed_t speed;
} g_block_t;

g_block_t block;

static float max(float x, float y, float z)
{
	return (x > y) ? ((x > z) ? x : z) : ((y > z) ? y : z);
}

static int32_t my_abs(int32_t t)
{
	return (t > 0) ? t : -t;
}

static uint8_t my_direc(int32_t d)
{
	return (d < 0) ? LEFT:RIGHT;
}

static TickType_t speed_to_ticks(float s)
{
	if(s < 1) {
		s = 1;
	}
	if(s > 100) {
		s = 100;
	}

	return pdMS_TO_TICKS(101 - s);
}

int main(void)
{
	boardInit();

	gpioInit(MOTOR_X_DIR, GPIO_OUTPUT);
	gpioInit(MOTOR_X_STEP, GPIO_OUTPUT);
	gpioInit(MOTOR_Y_DIR, GPIO_OUTPUT);
	gpioInit(MOTOR_Y_STEP, GPIO_OUTPUT);
	gpioInit(MOTOR_Z_DIR, GPIO_OUTPUT);
	gpioInit(MOTOR_Z_STEP, GPIO_OUTPUT);

	block.type = 0;
	block.future.px = 0;
	block.future.py = 0;
	block.future.pz = 0;
	block.speed = speed_to_ticks(100);

	motor_config(MOTOR_X_STEP, MOTOR_X_DIR,
		     MOTOR_Y_STEP, MOTOR_Y_DIR,
		     MOTOR_Z_STEP, MOTOR_Z_DIR);

	//UART config
	uartConfig(UART_USB, 115200);
	uartCallbackSet(UART_USB, UART_RECEIVE, onRx, NULL);
	uartInterrupt(UART_USB, true);

	xPointsQueue = xQueueCreate(5, sizeof(position_t));

	xTaskCreate(myTaskUART,
		    (const char*)"MyTaskUart",
		    configMINIMAL_STACK_SIZE*2,
		    0, //PARAMETROS
		    tskIDLE_PRIORITY+1,
		    &xHandleTask);

	xTaskCreate(myTaskPrint,
		    (const char*)"MytaskPrint",
		    configMINIMAL_STACK_SIZE*2,
		    NULL,
		    tskIDLE_PRIORITY+2,
		    &xHandlePrint);

	xTaskCreate(moveMotors,
		    (const char*)"moveMotors",
		    configMINIMAL_STACK_SIZE*4,
		    NULL,
		    tskIDLE_PRIORITY+3,
		    NULL);

	vTaskStartScheduler();

	while(1) { }

	return 0;
}



void moveMotors(void *param)
{

	position_t move;

	while(1) {
		xQueueReceive(xPointsQueue, &move, portMAX_DELAY);

		switch (block.type) {
			case LINE:
				line_move(move.px, move.py, move.pz);
				break;
			case FAST_MOVMENT:
				fast_move(move.px, move.py, move.pz);
				break;
			default:
				;
		}

	}


}
/*
 * @brief Quiero hacer una tarea que se bloquee hasta que una interrupción
 * del uart de un semáforo. Luego leere todos los caracteres que esten en el
 * buff del uart
 */
void myTaskUART(void * taskParamPrt)
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
			while(uartRxReady(UART_USB)) {
				//Si estoy aca es por que quedan datos
				//en el fifo del uart
				if((rx_char = uartRxRead(UART_USB)) != ' ') {
					if((rx_line[i] = rx_char) == '\n' || i > 15){
						rx_line[i] = '\0';
						i = 0;
						// Aviso que se leyó una linea
						// Cambiar esto por una cola de
						// strings ?
						xTaskNotifyGive(xHandlePrint);
						//Tengo que guardar esta linea
						//en una cola para poder
						//procesarla luego
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

void myTaskPrint(void *parameters)
{

	const TickType_t maxExpectedBlockTime = pdMS_TO_TICKS(500UL);
	uint32_t ulEventsToProcess;

	while(1) {

		//xQueueReceive(xQueue, str, portMAX_DELAY);

		ulEventsToProcess = ulTaskNotifyTake(pdTRUE, maxExpectedBlockTime);

		if(ulEventsToProcess) {
			process_line(rx_line);
		} else {
			;
		}

	}



}


void onRx(void *noUso)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(xHandleTask, &xHigherPriorityTaskWoken);
	uartInterrupt(UART_USB, false);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void process_line(char *rxLine)
{
	size_t counter = 0;
	char letter;
	float number=0;
	uint8_t int_val;
	bool_t movment = false;

	uartWriteString(UART_USB, rxLine);
	//Check for EOL
	while(rxLine[counter] != '\0') {

		letter = rxLine[counter];
		if((read_number(rxLine, &counter, &number)) != 0) {
			//Error reading number Discart line
			uartWriteString(UART_USB, "Descarto valor");
			return;
		}

		int_val = (uint8_t) number;

	switch(letter) {

		case 'G':
			switch (int_val) {
			case 0:
				//Fast Move
				uartWriteString(UART_USB, "Movimiento Rapido\r\n");
				block.type = FAST_MOVMENT;
				break;
			case 1:
				//line move
				uartWriteString(UART_USB, "Movimiento Lineal\r\n");
				block.type = LINE;
				break;
			case 2:
				//TODO: Arc Move
				block.type = ARC;
				break;
			case 10:
				printf("%u,", actual_pos.px);
				printf("%u,", actual_pos.py);
				printf("%u\n", actual_pos.pz);
				// Set this pos as origin ?
				//set_origin();
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
				uartWriteString(UART_USB, "Defaultcase num\r\n");
				//Do nothing
				break;
			}
			//Si es un código G entonces tengo que ver que tipo es
			break;

		case 'X':
			//uartWriteString(UART_USB, "Setting X\r\n");
			//printf("%d\n", (int) (number*100));
			future_pos.px = number;
			block.future.px = number;
			movment = true;
			break;
		case 'Y':
			//uartWriteString(UART_USB, "Setting Y\r\n");
			//printf("%d\n", (int) number);
			future_pos.py = number;
			block.future.py = number;
			movment = true;
			break;
		case 'Z':
			//uartWriteString(UART_USB, "Setting Z\r\n");
			//printf("%d\n", (int) number);
			future_pos.pz = number;
			block.future.pz = number;
			movment = true;
			break;
		case 'F':
			block.speed = speed_to_ticks(number);
			//Updated Speed
			break;

		default:
			uartWriteString(UART_USB, "Defaultcase value\r\n");
			//Do nothing
			;
		}
	}

	// Las colas almacenan copias
	if(movment) {
		xQueueSend(xPointsQueue, &future_pos, portMAX_DELAY);
		// ACA TENGO QUE MANDAR UN CARACTER PARA AVISAR SI
		// QUEDA ESPACIO PARA EL PROXIMO CARACTER O NO
		// VER
	}
	// aca podria subir a una cola una estructura y la leo en el movimiento
	// de los motores
	//grbl ejecutal el comando dentro de esta función

}



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
	//atof is secure ?
	if(*(iterator+i) != '0')
		*number = atof(iterator+i);
	else
		*number = 0;

	while(!isLetter(*(iterator+i)) && *(iterator+i) != '\0') { i++;}
	//update the counter to the next letter
	//or to the end of the string
	*counter += i;

	//All good
	return 0;
}

//Make this static
bool_t isLetter(const uint8_t c)
{
	return (c >= 'A' && c <= 'Z');
}

bool_t isNumber(const uint8_t c)
{
	return (c>= '0' && c<= '9');
}



void line_move(float newx, float newy, float newz)
{
	// diferencia entre el punto actual y el nuevo
	int32_t delta_x, delta_y, delta_z;
	// máximo delta
	uint32_t max_steps = 0;
	// direccion de giro del motor
	uint8_t dir_x, dir_y, dir_z;
	// desiciones que forman parte del algoritmo
	int32_t decision1, decision2;

	// La posicion actual la guardo en pasos
	// la nueva posicion viene en milimetros
	// entonces la convierto a pasos.
	int32_t new_stepx = newx * STEPS_PER_MM;
	int32_t new_stepy = newy * STEPS_PER_MM;
	int32_t new_stepz = newz * STEPS_PER_MM;

	delta_x = new_stepx - actual_pos.px;
	delta_y = new_stepy - actual_pos.py;
	delta_z = new_stepz - actual_pos.pz;

	dir_x = my_direc(delta_x);
	dir_y = my_direc(delta_y);
	dir_z = my_direc(delta_z);

	int8_t xs = (delta_x == 0) ? 0 : ((delta_x > 0) ? 1 : -1);
	int8_t ys = (delta_y == 0) ? 0 : ((delta_y > 0) ? 1 : -1);
	int8_t zs = (delta_z == 0) ? 0 : ((delta_z > 0) ? 1 : -1);

	//Me olvido del signo, esta guardado en dir
	delta_x = my_abs(delta_x);
	delta_y = my_abs(delta_y);
	delta_z = my_abs(delta_z);

	//Calculo el maximo delta
	max_steps = max(delta_x, delta_y, delta_z);


	// Then X is driving
	if(max_steps == delta_x) {
		decision1 = 2 * delta_y - delta_x;
		decision2 = 2 * delta_z - delta_x;
		while(actual_pos.px != new_stepx) {
			actual_pos.px += xs;
			motor_x_move(dir_x);
			if(decision1 >= 0) {
					actual_pos.py += ys;
				if(ys) {
					motor_y_move(dir_y);
				}
					decision1 -= 2*delta_x;

			}
			if(decision2 >= 0) {
					actual_pos.pz += zs;
				if(zs) {
					motor_z_move(dir_z);
				}
					decision2 -= 2*delta_x;
			}
			decision1 += 2 * delta_y;
			decision2 += 2 * delta_z;
			// Delay de velocidad
			vTaskDelay(block.speed);
			//delay(10);
		}
	//Then Y is driving
	} else if(max_steps == delta_y) {
		decision1 = 2 * delta_x - delta_y;
		decision2 = 2 * delta_z - delta_y;
		while(actual_pos.py != new_stepy) {
			actual_pos.py += ys;
			motor_y_move(dir_y);
			if(decision1 >= 0) {
					actual_pos.px += xs;
				if(ys) {
					motor_x_move(dir_x);
				}
					decision1 -= 2*delta_y;

			}
			if(decision2 >= 0) {
					actual_pos.pz += zs;
				if(zs) {
					motor_z_move(dir_z);
				}
					decision2 -= 2*delta_y;
			}
			decision1 += 2 * delta_x;
			decision2 += 2 * delta_z;
			// Delay de velocidad
			//delay(10);
			vTaskDelay(block.speed);
		}
	// Then Z is driving
	} else {
		decision1 = 2 * delta_y - delta_z;
		decision2 = 2 * delta_x - delta_z;
		while(actual_pos.pz != new_stepz) {
			actual_pos.pz += zs;
			motor_z_move(dir_z);
			if(decision1 >= 0) {
					actual_pos.py += ys;
				if(ys) {
					motor_y_move(dir_y);
				}
					decision1 -= 2*delta_z;

			}
			if(decision2 >= 0) {
					actual_pos.px += xs;
				if(zs) {
					motor_x_move(dir_x);
				}
					decision2 -= 2*delta_z;
			}
			decision1 += 2 * delta_y;
			decision2 += 2 * delta_x;
			// Delay de velocidad
			//delay(10);
			vTaskDelay(block.speed);
		}
	}
}





void fast_move(float newx, float newy, float newz)
{
	int32_t delta_x, delta_y, delta_z;
	uint8_t dir_x, dir_y, dir_z;
	int32_t new_stepx = newx * STEPS_PER_MM;
	int32_t new_stepy = newy * STEPS_PER_MM;
	int32_t new_stepz = newz * STEPS_PER_MM;

	delta_x = new_stepx - actual_pos.px;
	delta_y = new_stepy - actual_pos.py;
	delta_z = new_stepz - actual_pos.pz;

	dir_x = my_direc(delta_x);
	dir_y = my_direc(delta_y);
	dir_z = my_direc(delta_z);

	int8_t xs = (delta_x == 0) ? 0 : ((delta_x > 0) ? 1 : -1);
	int8_t ys = (delta_y == 0) ? 0 : ((delta_y > 0) ? 1 : -1);
	int8_t zs = (delta_z == 0) ? 0 : ((delta_z > 0) ? 1 : -1);


	while(actual_pos.px != new_stepx) {
		actual_pos.px += xs;
		motor_x_move(dir_x);
		vTaskDelay(block.speed);
	}
	while(actual_pos.py != new_stepy) {
		actual_pos.py += ys;
		motor_y_move(dir_y);
		vTaskDelay(block.speed);
	}
	while(actual_pos.pz != new_stepz) {
		actual_pos.pz += zs;
		motor_z_move(dir_z);
		vTaskDelay(block.speed);
	}

}









