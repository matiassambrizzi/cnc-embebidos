#include "process_line.h"


extern SemaphoreHandle_t xSemaphore;
/*
 * Prototipos funciones privadas
 * =============================
 */
static bool_t isNumber(const uint8_t c);
static bool_t isLetter(const uint8_t c);

/*
 * Implementación de funciones privadas
 * ====================================
 */
static bool_t isLetter(const uint8_t c)
{
	return (c >= 'A' && c <= 'Z');
}

static bool_t isNumber(const uint8_t c)
{
	return (c >= '0' && c<= '9');
}

/*
 * Implementación de funciones públicas
 * ====================================
 */
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

#ifdef DEBUG
			vPrintString(rx_line);
#endif
			//Check for EOL
			while(rx_line[counter] != '\0') {

				// Chequear si en la linea hay al menos una
				// X Y Z G F $$
				letter = rx_line[counter];

				//We have a command
				if(letter == '$') {

					// read the next letter
					// update counter and save command
					// letter
#ifdef DEBUG
					vPrintString("Me llego un comando\r\n");
#endif
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
#ifdef DEBUG
						vPrintString("Movimiento Rapido\r\n");
#endif
						gcode_block_set_movment(FAST_MOVMENT);
						break;
					case 1:
						//line move
#ifdef DEBUG
						vPrintString("Movimiento Lineal\r\n");
#endif
						gcode_block_set_movment(LINE);
						break;
					case 2:
						//TODO: Arc Move
#ifdef DEBUG
						vPrintString("Movimiento en Arco\r\n");
#endif
						//gcode_block_set_movment(ARC);
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
#ifdef DEBUG
						vPrintString("HOMING\r\n");
#endif
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
						;
#ifdef DEBUG
						//vPrintString("Default Case\r\n");
#endif
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

				case 'M':
					switch (int_val) {
					case 2:
						vPrintString("Seting pause\r\n");
						gcode_set_pause(true);
						break;
					case 3:
						gcode_set_pause(false);
						break;
					default:
						;
					}
					break;

				//Command case
				case '$':
					switch (command_letter) {
					case 'a':
						// set accel = number
#ifdef DEBUG
						printf("Numero %d\r\n", (int32_t) number);
#endif
						motion_set_accel(number);
						break;
					case '$':
						// Imprimir la configuración
						// actual
						break;
					default:
#ifdef DEBUG
						printf("Default Command", (int32_t) number);
#endif
						;
					}
					break;

				default:
#ifdef DEBUG
					//vPrintString("Default Case\r\n");
#endif
					;
				}
			}

			// Agrego un nuevo campo en la estructura de codigo G
			// Si hay una pausa entonces no doy el semaforo de
			// movimiento
			// Esto no funciona, porque la tarea que da el semaforo
			// se tiene que estar ejecutando periodicamente no solo
			// cuando llega algo por uart.
			// Tengo que hacer una tarea que se ejecute cada algun
			// tiempo y se fije si se quiere hacer una pausa.
			// TODO: ESTO NO FUNCIONA
			 if(gcode_get_pause() != true) {
				xSemaphoreGive(xSemaphore);
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
			if(uxQueueSpacesAvailable(xPointsQueue) != 1 &&
			   gcode_block_get_movement() != HOMING) {
				ready_to_process();
			}
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
	//if(*(iterator+i) >= '0'  && *(iterator+i) <= 9) {
		*number = strtod(iterator+i, &aux);
		//*number = atof(iterator+i);
		if(aux == NULL) {
			return -1;
		}
	//}
	//else { *number = 0;  }

	while(!isLetter(*(iterator+i)) && *(iterator+i) != '\0') { i++;}
	//update the counter to the next letter
	//or to the end of the string
	*counter += i;

	//All good
	return 0;
}

