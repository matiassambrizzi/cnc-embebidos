#include "process_line.h"

extern SemaphoreHandle_t xSemaphore;

/*
 * Prototipos funciones privadas
 * =============================
 */
static bool_t isNumber(const uint8_t c);
static bool_t isLetter(const uint8_t c);
static bool_t isValidLetter(const char c);

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

static bool_t isValidLetter(const char c)
{
	return (c == 'G') || (c == 'F') ||
		(c == '$') || (c == 'X') ||
		(c == 'Y') || (c == 'Z') ||
		(c == 'M');
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

				letter = rx_line[counter];

				// Chequear si en la linea hay al menos una
				// X Y Z G F $$
				if(!isValidLetter(letter)) { counter++; continue;}

				if(letter == '$') {
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
#ifdef DEBUG
				printf("Number is: %d\n",(int) number);
#endif

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
						gcode_set_coordinates(ABSOLUTE);
						break;
					case 91:
						gcode_set_coordinates(RELATIVE);
						//Relative Mode
						break;
					case 92:
						//Set this pos as 0,0,0
						position_reset();
						break;
					default:
						;
					}
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
#ifdef DEBUG
						printf("Numero %d\r\n", (int32_t) number);
#endif
						motion_set_accel(number);
						break;
					case '$':
						printf("X: %d, ", position_get_x() / STEPS_PER_MM_X);
						printf("Y: %d, ", position_get_y() / STEPS_PER_MM_Y);
						printf("Z: %d\r\n", position_get_z() / STEPS_PER_MM_Z);
						break;
					default:
						;
					}
					break;

				default:
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
			 //if(gcode_get_pause() != true) {
				//xSemaphoreGive(xSemaphore);
			 //}
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
				// TODO: Si estoy en modo relativo entonces aca
				// tengo que limpiar rx_gcode_block x, y, z
				// Pero si cambio el modo entonces voy a tener
				// pos 0 0 0.....
				//gcode_reset_xyz();
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
	uint8_t i = 0;

	i++;
	// If iterator+i is letter -> the function returns 0
	// TODO: Status_t
	*number = stringToFloat(iterator+i);
	while(!isLetter(*(iterator+i)) && *(iterator+i) != '\0') { i++;}
	*counter += i;

	//All good
	return 0;
}

float stringToFloat(char *str)
{
	uint32_t i = 0;
	char aux;
	float result = 0;
	float base = 0.1;
	bool_t neg = false;

	aux = str[i];

	// Chequeo de signo
	if(aux == '-') {
		neg = true;
		i++;
		aux = str[i];
	}
	else if(aux == '+') {
		i++;
		aux = str[i];
	}
	// Parte entera
	while(aux != '.' && aux != '\0') {
		if(!isNumber(aux)) {
			// st = is_letter;
			//return IS_LETTER;
			break;
		}
		result = result*10 + aux - '0';
		i++;
		aux = str[i];
	}
	// Parte decimal
	if(aux == '.') {
		i++;
		aux = str[i];
		while(aux != '\0' && isNumber(aux)) {
			result = result + base*(float)(aux-'0');
			base /= 10;
			i++;
			aux = str[i];
		}
	}

	if(neg == true) {
		result *= -1;
	}

	return result;
}
