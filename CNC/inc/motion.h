#ifndef _MOTION__H_
#define _MOTION__H_

#include <math.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "motors.h"
#include "gcode.h"
#include "position.h"
#include "uart.h"


/**
* @brief FreeRTOS Task para maover los motores
* @param param not use
* @return nothing
*/
void moveMotorsTask(void *param);

/**
* @brief Funciona para obtener los puntos intermedios entre dos puntos por
* interpolación lineal. La posición actual se guarda en una variable global
* @param newx nuevo valor de x
* @param newy nuevo valor de y
* @param newz nuevo valor de z
* @return nothing
*/
void line_move(float newx, float newy, float newz);

/**
* @brief Función para comenzar el ciclo de home de todos los motores. Cada motor
* comenzara a rotar hacia el fin de carrera correspondiente. Una vez que este
* fin de carrera se active el motor girar en sentido contrario hasta que este se
* desactive.
* @param nothing
* @return nothing
*/
void home_all();

/**
* @brief Movimiento rapido. Al usar esta función no se interpolará, sino que se
* moverán independientemente cada motor a la máxima velocidad.
* @param newx nuevo valor de x
* @param newy nuevo valor de y
* @param newz nuevo valor de z
* @return nothing
*/
void fast_move(float newx, float newy, float newz);


#endif
