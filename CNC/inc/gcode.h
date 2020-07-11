#ifndef _GCODE_BLOCK__H
#define _GCODE_BLOCK__H

#include "FreeRTOS.h"
#include "position.h"
#include "types.h"

#define speed_t   TickType_t

typedef struct {
	float px;
	float py;
	float pz;
} position_t;

/**
 * @brief Función para resetear la estructura bloque de gcode
 * @param nothing
 * @return nothing
*/
void gcode_block_reset();


/**
 * @brief Función para setear la posición en X
 * @param x es el valor de la posición en X recibida por código G
 * @return nothing
*/
void gcode_block_set_x(const float x);
void gcode_block_set_y(const float y);
void gcode_block_set_z(const float z);


/**
* @brief Funcion para setear la velocidad de la máquina
* @param s velocidad entre 1 a 100% recibida por puerto serie
* @return nothing
*/
void gcode_block_set_speed(const uint8_t s);

/**
* @brief Funcion para setear el timpo de movimiento que se quiere realizar
* @param m tipo de dato movimeinto
* @return nothing
*/
void gcode_block_set_movment(const movment_type_t m);


speed_t gcode_block_get_speed();

movment_type_t gcode_block_get_movement();

position_t * gcode_block_get_position();
#endif
