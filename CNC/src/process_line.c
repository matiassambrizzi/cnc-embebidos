#include "process_line.h"


//Make this static
static bool_t isLetter(const uint8_t c)
{
	return (c >= 'A' && c <= 'Z');
}

static bool_t isNumber(const uint8_t c)
{
	return (c >= '0' && c<= '9');
}

void processGcodeLineTask(void *parameters)
{

	const TickType_t maxExpectedBlockTime = pdMS_TO_TICKS(500UL);
	uint32_t ulEventsToProcess;

	uint8_t counter = 0;
	char letter;
	char command_letter;
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
			movment = false;

			uartWriteString(UART_PORT, rx_line);
			uartWriteString(UART_PORT, "\r\n");
			//Check for EOL
			while(rx_line[counter] != '\0') {

				letter = rx_line[counter];

				//We have a command
				if(letter == '$') {

					// read the next letter
					// update counter and save command
					// letter
					uartWriteString(UART_USB,"Me llego un comando\r\n");
					counter++;
					command_letter = rx_line[counter];
				}

				if((read_number(rx_line, &counter, &number)) != 0) {
					//Error reading number Discart line
					uartWriteString(UART_PORT, "Descarto valor");
					letter = 'D';
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
						uartWriteString(UART_PORT, "Movimiento en Arco\r\n");
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
						uartWriteString(UART_PORT, "HOMING\r\n");
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

				//Command case
				case '$':
					switch (command_letter) {
					case 'a':
						// set accel = number
						printf("Numero %d\r\n", (int32_t) number);
						motion_set_accel(number);
						break;
					case '$':
						// Imprimir la configuración
						// actual
						break;
					default:
						printf("Default Command", (int32_t) number);
						;
					}
					break;

				default:
					uartWriteString(UART_PORT, "Defaultcase value\r\n");
					;
				}
			}
			// Me fijo si el comando corresponde con un movimiento
			// si esto es verdad esntones pongo el bloque en la cola
			if(movment) {
				// Si el comando corresponde con G28(HOMING)
				// Se envia a la cola una posicion futura random
				if(uxQueueSpacesAvailable(xPointsQueue) != 1 &&
				   gcode_block_get_movement() != HOMING) {
					ready_to_process();
				}
				xQueueSend(xPointsQueue, (gcode_get_block()),
					   portMAX_DELAY);
			}

		} else {
			// Si estoy aca es porque no me llego ningun dato en
			// 500ms -> mando que estoy listo si tengo espacio en la
			// cola. Si estoy en medio de un HOMING tampoco mando
			// que estoy listo. Cuando termine ese cilco mandare el
			// ready y reseteare el gCodeblock.
			//if(uxQueueSpacesAvailable(xPointsQueue) != 1 &&
			 //  gcode_block_get_movement() != HOMING) {
			//	ready_to_process();
			//}
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
	// TODO: STRTOF: Cuando le mando 0.1 a 0.9 me lo reconoce como 0 a menos
	// que el agregegue el signo WTF??
	if(*(iterator+i) != '0') {
		*number = strtod(iterator+i, &aux);
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

