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

	uint8_t counter = 0;
	char letter;
	float number=0;
	uint8_t int_val = 0;
	bool_t movment = false;

	while(1) {

		// Guardar la notificación de la tarea
		ulEventsToProcess = ulTaskNotifyTake(pdTRUE, maxExpectedBlockTime);

		if(ulEventsToProcess) {
			//Reset variables
			counter = 0;
			letter = '\0';
			number = 0;
			int_val = 0;
			movment = 0;

			uartWriteString(UART_PORT, rx_line);
			uartWriteString(UART_PORT, "\r\n");
			//Check for EOL
			while(rx_line[counter] != '\0') {

				letter = rx_line[counter];
				if((read_number(rx_line, &counter, &number)) != 0) {
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
					}
					//Si es un código G entonces tengo que ver que tipo es
					break;

				case 'X':
					gcode_block_set_x(number);
					movment = true;
					break;
				case 'Y':
					gcode_block_set_y(number);
					movment = true;
					break;
				case 'Z':
					gcode_block_set_z(number);
					movment = true;
					break;
				case 'F':
					gcode_block_set_speed(number);
					//Updated Speed
					break;

				default:
					uartWriteString(UART_PORT, "Defaultcase value\r\n");
					;
				}
			}
			if(movment) {
				// Si el comando corresponde con G28(HOMING)
				// Se envia a la cola una posicion futura random
				// Se llega alguna otra posición se pierde.
				// Luego de generar homing tiene que ser una
				// tarea aparte con prioridad alta ?
				// y cuando se quiera hcaer un homing se notique
				if(uxQueueSpacesAvailable(xPointsQueue) != 1 &&
				   gcode_block_get_movement() != HOMING) {
					ready_to_process();
				}
				xQueueSend(xPointsQueue, (gcode_block_get_position()), portMAX_DELAY);
			}

		} else {
			;
		}
	}
}

int read_number(char *rxLine, uint8_t *counter, float *number)
{
	char *iterator = rxLine + *counter;
	char *aux;
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
	if(*(iterator+i) != '0') {
		*number = strtof(iterator+i, &aux);
		//*number = atof(iterator+i);
		if(aux == NULL) {
			return -1;
		}
	}
	else { *number = 0;  }

	while(!isLetter(*(iterator+i)) && *(iterator+i) != '\0') { i++;}
	//update the counter to the next letter
	//or to the end of the string
	*counter += i;

	//All good
	return 0;
}

