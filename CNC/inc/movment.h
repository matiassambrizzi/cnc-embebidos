#ifndef __MOVMENT_H__
#define __MOVMENT_H__

#include "sapi.h"
#include "config.h"
#include "motors.h"

typedef uint32_t speed_t;

typedef enum {
	LINEAR,
	FAST,
	CIRCULAR,
	IDLE
} movment_type_t;

typedef enum {
	RELATIVE,
	ABSOLUTE
} coordinates_type_t

typedef struct {
	float px;
	float py;
	float pz;
} axis_t;

typedef struct {
	int32_t px;
	int32_t py;
	int32_t pz;
} steps_t;

typedef struct {
	movment_type_t type;
	coordinates_type_t coordinates;
	steps_t next_pos;
	speed_t speed;
} movment_t;

/*
 * @brief funcion calcular los puntos intermedios entre una variable global que
 * guarda registro de la posición actual y los nuevos parámetros que se le pasen
 * @param newx nuevo valor que quiero en x
 * @param newy nuevo valor que quiero en y
 * @param newz nuevo valor que quiero en z
 * TODO: Esto está implementado solo para 1 eje
 */
void line_move(float newx, float newy, float newz);

/*
 * @brief Setear esta posición como el origen de coordenadas
*/
void set_origin();

#endif
