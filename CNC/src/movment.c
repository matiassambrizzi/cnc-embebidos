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

static uint8_t direction(int32_t d)
{
	return (d < 0) ? 0:1;
}

#ifndef UPGRADE_LINE
void line_move(float newx, float newy, float newz)
{
	int32_t delta_x, delta_y, delta_z;
	uint32_t max_steps = 0;
	//Calculo los delta de movimiento
	delta_x = (newx - actual_pos.px) * STEPS_PER_MM; //steps
	delta_y = (newy - actual_pos.py) * STEPS_PER_MM; //steps
	delta_z = (newz - actual_pos.pz) * STEPS_PER_MM; //steps

	uint8_t dir_x = direction(delta_x);
	uint8_t dir_y = direction(delta_y);
	uint8_t dir_z = direction(delta_z);

	//Me olvido del signo, esta guardado en dir_xx
	delta_x = abs(delta_x);
	delta_y = abs(delta_y);
	delta_z = abs(delta_z);

	//Calculo las iteraciones
	max_steps = max(delta_x, delta_y, delta_z);

	uint32_t over = 0;

	// Hago las iteraciones
	for(uint32_t i=0;i<max_steps; i++) {
		over += delta_x;
		if(over >= max_steps) {
			over -= max_steps;
			// Hago un paso cada 8us
			motor_x_move(dir_x);
		}
		delayInaccurateUs(800);
	}
	// Aca estoy asumiendo que efectivamente llego a este punto, mucho
	// error?
	actual_pos.px = newx;
	actual_pos.py = newy;
	actual_pos.pz = newz;
}
#else
//Mejorando la desición del algoritmo
void line_move(float newx, float newy, float newz)
{
	int32_t delta_x, delta_y, delta_z;
	uint32_t max_steps = 0;
	//Calculo los delta de movimiento
	delta_x = (newx - actual_pos.px) * STEPS_PER_MM; //steps
	delta_y = (newy - actual_pos.py) * STEPS_PER_MM; //steps
	delta_z = (newz - actual_pos.pz) * STEPS_PER_MM; //steps

	uint8_t dir_x = direction(delta_x);
	uint8_t dir_y = direction(delta_y);
	uint8_t dir_z = direction(delta_z);

	//Me olvido del signo, esta guardado en dir_xx
	delta_x = abs(delta_x);
	delta_y = abs(delta_y);
	delta_z = abs(delta_z);

	int32_t desition1 = 0;
	int32_t desition2 = 0;

	//Calculo el maximo delta
	max_steps = max(delta_x, delta_y, delta_z);
	//Quien ese el máximo ?

	uint32_t actual_pos_step_x = actual_pos.px * STEPS_PER_MM;
	uint32_t new_pos_step = newx * STEPS_PER_MM;

	if(delta_x >= delta_y && delta_x >= delta_y) {
		desition1 = 2 * delta_y - delta_x;
		desition2 = 2 * delta_z - delta_x;

		for(;max_steps >=0; max_steps--) {

			motor_x_move(dir_x);

			if(desition1 >= 0) {
				desition1 -= 2*delta_x;
			}
			if(desition2 >= 0) {
				desition2 -= 2*delta_x;
			}
			desition1 += 2*delta_y;
			desition2 += 2*delta_z;

			delay(1);
		}
	}

	actual_pos.px = newx;
	actual_pos.py = newy;
	actual_pos.pz = newz;
}
#endif
