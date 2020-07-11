#include "motion.h"

//defino la cola de puntos de movimiento
QueueHandle_t xPointsQueue;

static float max(float x, float y, float z)
{
	return (x > y) ? ((x > z) ? x : z) : ((y > z) ? y : z);
}

static int32_t my_abs(int32_t t)
{
	return (t > 0) ? t : -t;
}

static uint8_t my_direc(int32_t d)
{
	return (d < 0) ? LEFT:RIGHT;
}

void moveMotorsTask(void *param)
{

	position_t move;

	while(1) {
		xQueueReceive(xPointsQueue, &move, portMAX_DELAY);

		switch (gcode_block_get_movement()) {
			case LINE:
				line_move(move.px, move.py, move.pz);
				break;
			case FAST_MOVMENT:
				fast_move(move.px, move.py, move.pz);
				break;
			case HOMING:
				home_all();
				gcode_block_reset();
				xQueueReset(xPointsQueue);
				break;
			default:
				;
		}

	}


}

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

	delta_x = new_stepx - position_get_x();
	delta_y = new_stepy - position_get_y();
	delta_z = new_stepz - position_get_z();

	dir_x = my_direc(delta_x);
	dir_y = my_direc(delta_y);
	dir_z = my_direc(delta_z);

	int8_t xs = (delta_x == 0) ? 0 : ((delta_x > 0) ? 1 : -1);
	int8_t ys = (delta_y == 0) ? 0 : ((delta_y > 0) ? 1 : -1);
	int8_t zs = (delta_z == 0) ? 0 : ((delta_z > 0) ? 1 : -1);

	//Me olvido del signo, esta guardado en dir
	delta_x = my_abs(delta_x);
	delta_y = my_abs(delta_y);
	delta_z = my_abs(delta_z);

	//Calculo el maximo delta
	max_steps = max(delta_x, delta_y, delta_z);


	// Then X is driving
	if(max_steps == delta_x) {
		decision1 = 2 * delta_y - delta_x;
		decision2 = 2 * delta_z - delta_x;
		while(position_get_x() != new_stepx) {
			position_x_increment(xs);
			motor_x_move(dir_x);
			if(decision1 >= 0) {
					position_y_increment(ys);
				if(ys) {
					motor_y_move(dir_y);
				}
					decision1 -= 2*delta_x;

			}
			if(decision2 >= 0) {
					position_z_increment(zs);
				if(zs) {
					motor_z_move(dir_z);
				}
					decision2 -= 2*delta_x;
			}
			decision1 += 2 * delta_y;
			decision2 += 2 * delta_z;
			// Delay de velocidad
			vTaskDelay(gcode_block_get_speed());
			//delay(10);
		}
	//Then Y is driving
	} else if(max_steps == delta_y) {
		decision1 = 2 * delta_x - delta_y;
		decision2 = 2 * delta_z - delta_y;
		while(position_get_y() != new_stepy) {
			position_y_increment(ys);
			motor_y_move(dir_y);
			if(decision1 >= 0) {
					position_x_increment(xs);
				if(ys) {
					motor_x_move(dir_x);
				}
					decision1 -= 2*delta_y;

			}
			if(decision2 >= 0) {
					position_z_increment(zs);
				if(zs) {
					motor_z_move(dir_z);
				}
					decision2 -= 2*delta_y;
			}
			decision1 += 2 * delta_x;
			decision2 += 2 * delta_z;
			// Delay de velocidad
			//delay(10);
			vTaskDelay(gcode_block_get_speed());
		}
	// Then Z is driving
	} else {
		decision1 = 2 * delta_y - delta_z;
		decision2 = 2 * delta_x - delta_z;
		while(position_get_z() != new_stepz) {
			position_z_increment(zs);
			motor_z_move(dir_z);
			if(decision1 >= 0) {
					position_y_increment(ys);
				if(ys) {
					motor_y_move(dir_y);
				}
					decision1 -= 2*delta_z;

			}
			if(decision2 >= 0) {
					position_x_increment(xs);
				if(zs) {
					motor_x_move(dir_x);
				}
					decision2 -= 2*delta_z;
			}
			decision1 += 2 * delta_y;
			decision2 += 2 * delta_x;
			// Delay de velocidad
			//delay(10);
			vTaskDelay(gcode_block_get_speed());
		}
	}
}

// TODO: Modificar esta función para que se muevan todos los ejes a la
// misma velocidad
void fast_move(float newx, float newy, float newz)
{
	int32_t delta_x, delta_y, delta_z;
	uint8_t dir_x, dir_y, dir_z;
	int32_t new_stepx = newx * STEPS_PER_MM;
	int32_t new_stepy = newy * STEPS_PER_MM;
	int32_t new_stepz = newz * STEPS_PER_MM;

	delta_x = new_stepx - position_get_x();
	delta_y = new_stepy - position_get_y();
	delta_z = new_stepz - position_get_z();

	dir_x = my_direc(delta_x);
	dir_y = my_direc(delta_y);
	dir_z = my_direc(delta_z);

	int8_t xs = (delta_x == 0) ? 0 : ((delta_x > 0) ? 1 : -1);
	int8_t ys = (delta_y == 0) ? 0 : ((delta_y > 0) ? 1 : -1);
	int8_t zs = (delta_z == 0) ? 0 : ((delta_z > 0) ? 1 : -1);


	while(position_get_x() != new_stepx) {
		position_x_increment(xs);
		motor_x_move(dir_x);
		vTaskDelay(gcode_block_get_speed());
	}
	while(position_get_y() != new_stepy) {
		position_y_increment(ys);
		motor_y_move(dir_y);
		vTaskDelay(gcode_block_get_speed());
	}
	while(position_get_z() != new_stepz) {
		position_z_increment(zs);
		motor_z_move(dir_z);
		vTaskDelay(gcode_block_get_speed());
	}

}


void home_all()
{

	while(!end_stop_x_is_press()) {
		motor_x_move(HOMEX);
		vTaskDelay(gcode_block_get_speed());
	}
	while(end_stop_x_is_press()) {
		motor_x_move(~HOMEX);
		vTaskDelay(gcode_block_get_speed());
	}
/*
	while(!end_stop_y_is_press()) {
		motor_y_move(HOMEX);
		vTaskDelay(gcode_block_get_speed());
	}
	while(end_stop_y_is_press()) {
		motor_y_move(~HOMEX);
		vTaskDelay(gcode_block_get_speed());
	}

	while(!end_stop_z_is_press()) {
		motor_z_move(HOMEX);
		vTaskDelay(gcode_block_get_speed());
	}
	while(end_stop_z_is_press()) {
		motor_z_move(~HOMEX);
		vTaskDelay(gcode_block_get_speed());
	}
	*/
}
