#include "motion.h"

//defino la cola de puntos de movimiento
QueueHandle_t xPointsQueue;

typedef struct {
	uint32_t accel;  //accel in steps per second square
	uint32_t vel_max; //vel in steps per second
	uint32_t vel_min; //vel in steps per second
	step_count_t crop_step_accel;
	step_count_t crop_step_deaccel;
} motion_t;

// Variable local que almecena todo lo correspondiente
// con el calculo del moviminento con aceleración
static motion_t motion_config;

/**
* @brief Funcion que devuelve el máximo entre 3 parametros x, y z
* @param x son los pasos que hay que hacer en x
* @param y son los pasos que hay que hacer en y
* @param y son los pasos que hay que hacer en z
* @return maximo
*/
static float max(float x, float y, float z);

/**
* @brief Funcion para calcular el valor absoluto de un numero
* @param t
* @return valor absoluto de t
*/
static int32_t my_abs(int32_t t);

/**
* @brief Funcion para obtener la dirección de giro del motor
* @param dirección
* @return
*/
static uint8_t my_direc(int32_t d);

/**
* @brief Funcion para elevar al cuadrado un numero
* @param x numero
* @return x al cuadrado
*/
static uint32_t pow2(int32_t x);

/**
* @brief Funcion para incrementar la velocidad para una dad aceleración
* @param
* @return
*/
static void updateVelocity(const float pos, float *vel, const motion_t m, int8_t ss);



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

static uint32_t pow2(int32_t x)
{
	return x*x;
}

static void updateVelocity(const float pos, float *vel, const motion_t m, int8_t ss)
{
	// El algoritmo de aceleración depende de la dirección del movimiento
	if(ss > 0) {
		if(pos <= m.crop_step_accel) {
			if(*vel < m.vel_max) {
				*vel += m.accel/(*vel);
			} else {
				*vel = m.vel_max;
			}
		}
		if(pos >= m.crop_step_deaccel) {
			if(*vel > m.vel_min) {
				*vel -= m.accel/(*vel);
			} else {
				*vel = m.vel_min;
			}
		}
	} else {

		if(pos >= m.crop_step_accel) {
			if(*vel < m.vel_max) {
				*vel += m.accel/(*vel);
			} else {
				*vel = m.vel_max;
			}
		}
		if(pos <= m.crop_step_deaccel) {
			if(*vel > m.vel_min) {
				*vel -= m.accel/(*vel);
			} else {
				*vel = m.vel_min;
			}
		}

	}
}


void moveMotorsTask(void *param)
{
	// variable de configuracion obtenida de la cola
	g_block_t gblock;

	while(1) {
		//xQueueReceive(xPointsQueue, &move, portMAX_DELAY);
		xQueueReceive(xPointsQueue, &gblock, portMAX_DELAY);

		motion_config.vel_max = gblock.velocity;
		// Estaria bueno actualizar la aceleración aca ?

		switch (gblock.type) {
			case LINE:
				//line_move(move.px, move.py, move.pz);
				line_move(gblock.target_pos.px,
					  gblock.target_pos.py,
					  gblock.target_pos.pz);
				break;
			case FAST_MOVMENT:
				//fast_move(move.px, move.py, move.pz);
				fast_move(gblock.target_pos.px,
					  gblock.target_pos.py,
					  gblock.target_pos.pz);
				break;
			case HOMING:
				home_all();
				gcode_block_reset();
				xQueueReset(xPointsQueue);
				ready_to_process();
				break;
			default:
				;
		}
	}
}

void motion_reset()
{
	// Seteo los valores máximos y minimos por defecto
	motion_config.accel =  MAX_ACCEL_STEPS_PER_SECOND_SQUARE;
	motion_config.vel_max = MAX_VEL_STEPS_PER_SECOND;
	motion_config.vel_min = MIN_VEL_STEPS_PER_SECOND;
	motion_config.crop_step_accel = 0;
	motion_config.crop_step_deaccel = 0;
}

void motion_set_accel(const uint32_t accel)
{
	if(accel > MAX_ACCEL_STEPS_PER_SECOND_SQUARE) {
		motion_config.accel = MAX_ACCEL_STEPS_PER_SECOND_SQUARE;
	} else {
		motion_config.accel = accel;
	}
}

void line_move(float newx, float newy, float newz)
{
	//para trakear la posicion y aumentar la velocidad
	step_count_t pos;
	step_count_t initPos;
	step_count_t max_new_steps;
	int8_t xxs;
	// diferencia entre el punto actual y el nuevo
	int32_t delta_x, delta_y, delta_z;
	// máximo delta
	uint32_t max_steps = 0;
	// direccion de giro del motor
	uint8_t dir_x, dir_y, dir_z;
	// desiciones que forman parte del algoritmo
	int32_t decision1, decision2;

	// TODO: ESTO NO LO USO MÁS
	speed_t delay = gcode_block_get_speed();

	// La posicion actual la guardo en pasos
	// la nueva posicion viene en milimetros
	// entonces la convierto a pasos.
	// TODO: Si estoy en inches entonces tongo que cambiar la
	// multiplicación
	int32_t new_stepx = newx * STEPS_PER_MM;
	int32_t new_stepy = newy * STEPS_PER_MM;
	int32_t new_stepz = newz * STEPS_PER_MM;

	// Cantidad de pasoso entre la posición actual
	// y la nueva posicion
	delta_x = new_stepx - position_get_x();
	delta_y = new_stepy - position_get_y();
	delta_z = new_stepz - position_get_z();

	dir_x = my_direc(delta_x);
	dir_y = my_direc(delta_y);
	dir_z = my_direc(delta_z);

	//Dirección de los pasos
	int8_t xs = (delta_x == 0) ? 0 : ((delta_x > 0) ? 1 : -1);
	int8_t ys = (delta_y == 0) ? 0 : ((delta_y > 0) ? 1 : -1);
	int8_t zs = (delta_z == 0) ? 0 : ((delta_z > 0) ? 1 : -1);

	//Me olvido del signo, esta guardado en dir
	delta_x = my_abs(delta_x);
	delta_y = my_abs(delta_y);
	delta_z = my_abs(delta_z);

	//Calculo el maximo delta
	max_steps = max(delta_x, delta_y, delta_z);
	// Velocidad tiene que ser float
	float vel = MIN_VEL_STEPS_PER_SECOND;

	// VER DE PONER ESTO EN UNA FUNCION
	// setUpAccel()
	if(max_steps == delta_x) {
		initPos = position_get_x();
		max_new_steps = new_stepx;
		xxs = xs;

	}

	if(max_steps == delta_y) {
		initPos = position_get_y();
		max_new_steps = new_stepy;
		xxs = ys;
	}

	if(max_steps == delta_z) {
		initPos = position_get_z();
		max_new_steps = new_stepz;
		xxs = zs;
	}

	uint32_t stepsToMaxVel = pow2(motion_config.vel_max)/(2*motion_config.accel);

	if(max_steps < 2*stepsToMaxVel) {
		// Hay que bajar la velocidad porque la distancia es corta
		// La nueva máxima velocidad será la cantidad de pasos
		// por la aceleración
		//motion_config.vel_max = sqrt(delta_x*motion_config.accel);
		motion_config.vel_max = sqrt(max_steps*motion_config.accel);
		stepsToMaxVel = pow2(motion_config.vel_max)/(2*motion_config.accel);
	}

	if(xxs > 0) {
		motion_config.crop_step_accel = initPos + stepsToMaxVel;
		motion_config.crop_step_deaccel = max_new_steps - stepsToMaxVel;
	} else {
		motion_config.crop_step_accel = initPos - stepsToMaxVel;
		motion_config.crop_step_deaccel = max_new_steps + stepsToMaxVel;
	}
	//FIN CALCULOS ACELERACION


	// Get actual time in ticks
	// Then X is driving
	if(max_steps == delta_x) {
		decision1 = (delta_y<<1) - delta_x;
		decision2 = (delta_z<<1) - delta_x;

		while((pos=position_get_x()) != new_stepx) {

			position_x_increment(xs);
			motor_x_move(dir_x);
			if(decision1 >= 0) {
					position_y_increment(ys);
				if(ys) {
					motor_y_move(dir_y);
				}
					decision1 -= (delta_x<<1);

			}
			if(decision2 >= 0) {
					position_z_increment(zs);
				if(zs) {
					motor_z_move(dir_z);
				}
					decision2 -= (delta_x<<1);
			}
			decision1 += (delta_y<<1);
			decision2 += (delta_z<<1);

			// Actualiza la velocidad dependiendo de la acceleracion
			// que se desee
			updateVelocity(pos, &vel, motion_config, xs);
			vTaskDelay(pdMS_TO_TICKS(my_abs(1000/vel)));
		}
	//Then Y is driving
	} else if(max_steps == delta_y) {
		decision1 = (delta_x<<1) - delta_y;
		decision2 = (delta_z<<1) - delta_y;
		while((pos=position_get_y()) != new_stepy) {
			position_y_increment(ys);
			motor_y_move(dir_y);
			if(decision1 >= 0) {
					position_x_increment(xs);
				if(ys) {
					motor_x_move(dir_x);
				}
					decision1 -= (delta_y<<1);

			}
			if(decision2 >= 0) {
					position_z_increment(zs);
				if(zs) {
					motor_z_move(dir_z);
				}
					decision2 -= (delta_y<<1);
			}
			decision1 += (delta_x<<1);
			decision2 += (delta_z<<1);
			// Delay de velocidad
			//delay(10);
			// Ver como incrementar la velocidad de a poco
			updateVelocity(pos, &vel, motion_config, ys);
			vTaskDelay(pdMS_TO_TICKS(my_abs(1000/vel)));
		}
	// Then Z is driving
	} else {
		decision1 = (delta_y<<1) - delta_z;
		decision2 = (delta_x<<1) - delta_z;
		while((pos=position_get_z()) != new_stepz) {
			position_z_increment(zs);
			motor_z_move(dir_z);
			if(decision1 >= 0) {
					position_y_increment(ys);
				if(ys) {
					motor_y_move(dir_y);
				}
					decision1 -= (delta_z<<1);

			}
			if(decision2 >= 0) {
					position_x_increment(xs);
				if(zs) {
					motor_x_move(dir_x);
				}
					decision2 -= (delta_z<<1);
			}
			decision1 += (delta_y<<1);
			decision2 += (delta_x<<1);
			// Delay de velocidad
			//delay(10);
			updateVelocity(pos, &vel, motion_config, zs);
			vTaskDelay(pdMS_TO_TICKS(my_abs(1000/vel)));
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

	bool_t xStop = false;
	bool_t yStop = false;
	bool_t zStop = false;

	speed_t delay = gcode_block_get_speed();

	while(!xStop || !yStop || !zStop) {

		if(position_get_x() != new_stepx) {
			position_x_increment(xs);
			motor_x_move(dir_x);
		} else { xStop = true; }

		if(position_get_y() != new_stepy) {
			position_y_increment(ys);
			motor_y_move(dir_y);
		} else { yStop = true;}

		if(position_get_z() != new_stepz) {
			position_z_increment(zs);
			motor_z_move(dir_z);
		} else {zStop = true;}
		vTaskDelay(delay);
	}
}


void home_all()
{

	speed_t delay = gcode_block_get_speed();

	while(!end_stop_x_is_press()) {
		motor_x_move(HOMEX);
		vTaskDelay(delay);
	}
	while(end_stop_x_is_press()) {
		motor_x_move(~HOMEX);
		vTaskDelay(delay);
	}
	while(!end_stop_y_is_press()) {
		motor_y_move(HOMEX);
		vTaskDelay(delay);
	}
	while(end_stop_y_is_press()) {
		motor_y_move(~HOMEX);
		vTaskDelay(delay);
	}

	while(!end_stop_z_is_press()) {
		motor_z_move(HOMEX);
		vTaskDelay(delay);
	}
	while(end_stop_z_is_press()) {
		motor_z_move(~HOMEX);
		vTaskDelay(delay);
	}
}


