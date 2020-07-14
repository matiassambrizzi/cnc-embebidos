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
		// TODO: actualizar la aceleración aca!

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
	//para trackaar la posicion y aumentar la velocidad
	step_count_t pos;
	// La posicion actual la guardo en pasos
	// la nueva posicion viene en milimetros
	// entonces la convierto a pasos.
	// TODO: Si estoy en inches entonces tongo que cambiar la
	const int32_t new_stepx = newx * STEPS_PER_MM;
	const int32_t new_stepy = newy * STEPS_PER_MM;
	const int32_t new_stepz = newz * STEPS_PER_MM;

	interpolation_set_deltas(new_stepx, new_stepy, new_stepz);
	interpolation_init();

	// Velocidad tiene que ser float
	float vel = MIN_VEL_STEPS_PER_SECOND;

	const step_count_t initPos = get_updated_position();
	const step_count_t max_new_steps = get_max_target_steps();
	const int32_t max_delta = get_drive_delta();
	const int8_t xxs = get_step_dir();


	// TODO MODULARIZAR EL MANEJOR DE LA ACELERACIOn
	//CALCULOS ACELERACION
	uint32_t stepsToMaxVel = pow2(motion_config.vel_max)/(2*motion_config.accel);

	if(max_delta < 2*stepsToMaxVel) {
		// Hay que bajar la velocidad porque la distancia es corta
		// La nueva máxima velocidad será la cantidad de pasos
		// por la aceleración
		motion_config.vel_max = sqrt(max_delta*motion_config.accel);
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

	pos = initPos;
	while(pos != max_new_steps) {
		// STEP 1: Correr un ciclo de interpolacion y mover los motores
		interpolation_run_cycle();
		// STEP 2: Calcular la velocidad teniendo en cuenta la aceleración
		updateVelocity(pos, &vel, motion_config, xxs);
		// STEP 3: Poner un delay para manejar la velocidad de giro
		vTaskDelay(pdMS_TO_TICKS(my_abs(1000/vel)));
		// STEP4: Actualizar la posición
		pos = get_updated_position();
	}

}

// TODO: ACELERACION
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


// TODO: ACELERACION
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
