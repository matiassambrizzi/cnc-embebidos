#include "position.h"

// Variable Global privada que lleva la cuenta de la posición
// actual de los motores en el plano en pasos.

/*
 * Estructura interna
 * ==================
 */

typedef struct {
	step_count_t px;
	step_count_t py;
	step_count_t pz;
} steps_t;


/*
 * Variables internas
 * ==================
 */

static steps_t actual_pos={.px=0, .py=0, .pz=0};


/*
 * Implementación de funciones púbicas
 * ===================================
 */

void position_reset()
{
	actual_pos.px = 0;
	actual_pos.py = 0;
	actual_pos.pz = 0;
}

void position_set_x(const step_count_t x)
{
	actual_pos.px = x;
}


void position_set_y(const step_count_t y)
{
	actual_pos.py = y;
}


void position_set_z(const step_count_t z)
{
	actual_pos.pz = z;
}


void position_x_increment(const int8_t x)
{
	actual_pos.px += x;
}

void position_y_increment(const int8_t y)
{
	actual_pos.py += y;
}

void position_z_increment(const int8_t z)
{
	actual_pos.pz += z;
}

step_count_t position_get_x()
{
	return actual_pos.px;
}

step_count_t position_get_y()
{
	return actual_pos.py;
}

step_count_t position_get_z()
{
	return actual_pos.pz;
}
