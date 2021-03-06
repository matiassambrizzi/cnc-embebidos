#ifndef _GCODE_BLOCK__H
#define _GCODE_BLOCK__H

#ifndef TEST_ALL
#include "FreeRTOS.h"
#endif

#ifdef TEST_ALL
#include "test_utils.h"
#include <stdint.h>
#endif

#include "position.h"
#include "types.h"
#include "config.h"

// TODO: Esto no se usa
#define speed_t			TickType_t
#define		MAX_AXIS	3

typedef struct g_block_t {
	position_t target_pos;
	measurement_units_t units;
	speed_t speed; // TODO: Esto es un porcentaje (0~100) de la máxima velocidad se pasa con el comando F
	// Mejor que sea float?
	uint32_t velocity; // Velocidad en steps/sec
	movment_type_t type;
	bool_t pause;
	coordinates_t cord;
	bool_t movement[MAX_AXIS];
} g_block_t;

typedef struct g_block_t * gBlockPtr;

/**
* @brief Funcion que devuelve la dirección de memoria del bloque de
* codigo G
* @return Gcode block memory location
*/
gBlockPtr gcode_get_block();

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
void gcode_block_set_speed(const float s);

/**
* @brief Funcion para setear el timpo de movimiento que se quiere realizar
* @param m tipo de dato movimeinto
* @return nothing
*/
void gcode_block_set_movment(const movment_type_t m);

// TODO : DOC
speed_t gcode_block_get_speed();
movment_type_t gcode_block_get_movement();

/**
* @brief Funcion que devuelve la posición a la que se quiere llegar. Esta
* posicion se recibio por puerto serie
* @return target position
*/
position_t * gcode_block_get_position();


//position_t get_target_position();
//void get_target_speed();


bool_t gcode_get_pause();

void gcode_set_pause(const bool_t p);
void gcode_set_coordinates(const coordinates_t cord);
void gcode_reset_xyz(void);

void gcode_move_x(void);
void gcode_move_y(void);
void gcode_move_z(void);

void gcode_reset_move();

bool_t gcode_x_want_to_move();
bool_t gcode_y_want_to_move();
bool_t gcode_z_want_to_move();

coordinates_t gcode_get_coordinates();
#endif
