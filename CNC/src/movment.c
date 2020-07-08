#include "movment.h"

//Gloabal Actual Position
// VER DONDE PONER ESTA VARIABLE
static axis_t actual_pos = {.px=0,.py=0,.pz=0};


static float max(float x, float y, float z)
{
	return (x > y) ? ((x > z) ? x : z) : ((y > z) ? y : z);
}

static int32_t abs(int32_t t)
{
	return (t > 0) ? t : -t;
}

static uint8_t direc(int32_t d)
{
	return (d < 0) ? LEFT:RIGHT;
}

/*
 * @breif Bresenham’s Algorithm. Se basa en mover
 */
void line_move(float newx, float newy, float newz)
{
	// diferencia entre el punto actual y el nuevo
	int32_t delta_x, delta_y, delta_z;
	// máximo delta
	uint32_t max_steps = 0;
	// direccion de giro del motor
	uint8_t dir_x, dir_y, dir_z;
	// desiciones que forman parte del algoritmo
	int32_t decision1, decision2;

	// La posicion actual la guardo en pasos
	// la nueva posicion viene en milimetros
	// entonces la convierto a pasos.
	int32_t new_stepx = newx * STEPS_PER_MM;
	int32_t new_stepy = newy * STEPS_PER_MM;
	int32_t new_stepz = newz * STEPS_PER_MM;

	delta_x = new_stepx - actual_pos.px;
	delta_y = new_stepy - actual_pos.py;
	delta_z = new_stepz - actual_pos.pz;

	dir_x = direc(delta_x);
	dir_y = direc(delta_y);
	dir_z = direc(delta_z);

	int8_t xs = (delta_x == 0) ? 0 : ((delta_x > 0) ? 1 : -1);
	int8_t ys = (delta_y == 0) ? 0 : ((delta_y > 0) ? 1 : -1);
	int8_t zs = (delta_z == 0) ? 0 : ((delta_z > 0) ? 1 : -1);

	//Me olvido del signo, esta guardado en dir
	delta_x = abs(delta_x);
	delta_y = abs(delta_y);
	delta_z = abs(delta_z);

	//Calculo el maximo delta
	max_steps = max(delta_x, delta_y, delta_z);

	// Then X is driving
	if(max_steps == delta_x) {
		decision1 = 2 * delta_y - delta_x;
		decision2 = 2 * delta_z - delta_x;
		while(actual_pos.px != new_stepx) {
			actual_pos.px += xs;
			motor_x_move(dir_x);
			if(decision1 >= 0) {
					actual_pos.py += ys;
				if(ys) {
					motor_y_move(dir_y);
				}
					decision1 -= 2*delta_x;

			}
			if(decision2 >= 0) {
					actual_pos.pz += zs;
				if(zs) {
					motor_z_move(dir_z);
				}
					decision2 -= 2*delta_x;
			}
			decision1 += 2 * delta_y;
			decision2 += 2 * delta_z;
			// Delay de velocidad
			delay(10);
		}
	//Then Y is driving
	} else if(max_steps == delta_y) {
		decision1 = 2 * delta_x - delta_y;
		decision2 = 2 * delta_z - delta_y;
		while(actual_pos.py != new_stepy) {
			actual_pos.py += ys;
			motor_y_move(dir_y);
			if(decision1 >= 0) {
					actual_pos.px += xs;
				if(ys) {
					motor_x_move(dir_x);
				}
					decision1 -= 2*delta_y;

			}
			if(decision2 >= 0) {
					actual_pos.pz += zs;
				if(zs) {
					motor_z_move(dir_z);
				}
					decision2 -= 2*delta_y;
			}
			decision1 += 2 * delta_x;
			decision2 += 2 * delta_z;
			// Delay de velocidad
			delay(10);
		}
	// Then Z is driving
	} else {
		decision1 = 2 * delta_y - delta_z;
		decision2 = 2 * delta_x - delta_z;
		while(actual_pos.pz != new_stepz) {
			actual_pos.pz += zs;
			motor_z_move(dir_z);
			if(decision1 >= 0) {
					actual_pos.py += ys;
				if(ys) {
					motor_y_move(dir_y);
				}
					decision1 -= 2*delta_z;

			}
			if(decision2 >= 0) {
					actual_pos.px += xs;
				if(zs) {
					motor_x_move(dir_x);
				}
					decision2 -= 2*delta_z;
			}
			decision1 += 2 * delta_y;
			decision2 += 2 * delta_x;
			// Delay de velocidad
			delay(10);
		}
	}
}


void set_origin(void)
{
	actual_pos.px = 0;
	actual_pos.py = 0;
	actual_pos.pz = 0;
}

