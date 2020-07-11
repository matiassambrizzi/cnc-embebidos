#include "process_line.h"


extern char rx_line[MAX_RX_BUFFER];
extern QueueHandle_t xPointsQueue;

//Make this static
static bool_t isLetter(const uint8_t c)
{
	return (c >= 'A' && c <= 'Z');
}

static bool_t isNumber(const uint8_t c)
{
	return (c>= '0' && c<= '9');
}



void processGcodeLineTask(void *parameters)
{

	const TickType_t maxExpectedBlockTime = pdMS_TO_TICKS(500UL);
	uint32_t ulEventsToProcess;

	while(1) {

		ulEventsToProcess = ulTaskNotifyTake(pdTRUE, maxExpectedBlockTime);

		if(ulEventsToProcess) {
			process_line(rx_line);
		} else {
			;
		}
	}
}




void process_line(char *rxLine)
{
	size_t counter = 0;
	char letter;
	float number=0;
	uint8_t int_val;
	bool_t movment = false;

	uartWriteString(UART_PORT, rxLine);
	//Check for EOL
	while(rxLine[counter] != '\0') {

		letter = rxLine[counter];
		if((read_number(rxLine, &counter, &number)) != 0) {
			//Error reading number Discart line
			uartWriteString(UART_PORT, "Descarto valor");
			return;
		}

		int_val = (uint8_t) number;

	switch(letter) {

		case 'G':
			switch (int_val) {
			case 0:
				//Fast Move
				uartWriteString(UART_PORT, "Movimiento Rapido\r\n");
				gcode_block_set_movment(FAST_MOVMENT);
				break;
			case 1:
				//line move
				uartWriteString(UART_PORT, "Movimiento Lineal\r\n");
				gcode_block_set_movment(LINE);
				break;
			case 2:
				//TODO: Arc Move
				gcode_block_set_movment(ARC);
				break;
			case 10:
				//printf("%d,", actual_pos.px);
				//printf("%d,", actual_pos.py);
				//printf("%d\n", actual_pos.pz);
				// Set this pos as origin ?
				//set_origin();
				break;
			case 21:
				//TODO mm metric
				break;
			case 28:
				//TODO: Homing
				gcode_block_set_movment(HOMING);
				movment = true;
				break;

			case 90:
				uartWriteString(UART_PORT, "ModoAbsoluto\r\n");
				//Absolute mode
				break;
			case 91:
				//Relative Mode
				break;
			case 92:
				//Set this pos as 0,0,0
				break;
			default:
				uartWriteString(UART_PORT, "Defaultcase num\r\n");
				//Do nothing
				break;
			}
			//Si es un código G entonces tengo que ver que tipo es
			break;

		case 'X':
			//uartWriteString(UART_PORT, "Setting X\r\n");
			//printf("%d\n", (int) (number*100));
			gcode_block_set_x(number);
			movment = true;
			break;
		case 'Y':
			//uartWriteString(UART_PORT, "Setting Y\r\n");
			//printf("%d\n", (int) number);
			gcode_block_set_y(number);
			movment = true;
			break;
		case 'Z':
			//uartWriteString(UART_PORT, "Setting Z\r\n");
			//printf("%d\n", (int) number);
			gcode_block_set_z(number);
			movment = true;
			break;
		case 'F':
			gcode_block_set_speed(number);
			//Updated Speed
			break;

		default:
			uartWriteString(UART_PORT, "Defaultcase value\r\n");
			//Do nothing
			;
		}
	}

	// Las colas almacenan copias
	if(movment) {
		xQueueSend(xPointsQueue, (gcode_block_get_position()), portMAX_DELAY);
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
	/*
	if(*(iterator+i) < '0' || *(iterator+i) > '9') {
		//Error nan;
		return -1;
	}
	*/
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

