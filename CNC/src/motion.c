#include "motion.h"


/*
 * Variable Global
 * ===============
 */
QueueHandle_t xPointsQueue;
SemaphoreHandle_t xSemaphore;

/*
 * Estructura interna
 * ==================
 */

typedef step_count_t (*get_pos)();
typedef struct {
	uint32_t accel;  //accel in steps per second square
	// get pos para el algoritmo de acceleración
	get_pos pos;
	uint32_t vel_max; //vel in steps per second
	uint32_t vel_min; //vel in steps per second
	step_count_t crop_step_accel;
	step_count_t crop_step_deaccel;
} motion_t;

/*
 * Variables internas
 * ==================
 */
static motion_t motion_config;
static step_count_t init_pos;
// TODO: NOMBRE MAL PUESTO
static step_count_t max_new_steps;
// TODO: NOMBRE MAL PUESTO
static int32_t max_delta;
static int8_t direction;
static uint32_t steps_to_reach_max_vel;

/*
 * Prototipos funciones privadas
 * =============================
 */

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
static void updateVelocity(const float pos, float *vel, const motion_t m);


/**
* @brief TODO
* @param
* @return
*/
static void set_up_velocity(step_count_t initPos,
			    step_count_t steps,
			    step_count_t delta,
			    step_count_t direc);

/*
 * Implementación de funciones privadas
 * ====================================
 */

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

static void updateVelocity(const float pos, float *vel, const motion_t m)
{
	// El algoritmo de aceleración depende de la dirección del movimiento
	if(direction > 0) {
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


static void set_up_velocity(step_count_t initPos,
			    step_count_t steps,
			    step_count_t delta,
			    step_count_t direc)
{
	init_pos = initPos;
	max_new_steps = steps;
	max_delta = delta;
	direction = direc;

	steps_to_reach_max_vel = pow2(motion_config.vel_max)/(2*motion_config.accel);

	if(max_delta < 2*steps_to_reach_max_vel) {
		// Hay que bajar la velocidad porque la distancia es corta
		// La nueva máxima velocidad será la cantidad de pasos
		// por la aceleración
		motion_config.vel_max = sqrt(max_delta*motion_config.accel);
		steps_to_reach_max_vel = pow2(motion_config.vel_max)/(2*motion_config.accel);
	}

	if(direction > 0) {
		motion_config.crop_step_accel = init_pos + steps_to_reach_max_vel;
		motion_config.crop_step_deaccel = max_new_steps - steps_to_reach_max_vel;
	} else {
		motion_config.crop_step_accel = init_pos - steps_to_reach_max_vel;
		motion_config.crop_step_deaccel = max_new_steps + steps_to_reach_max_vel;
	}


}
/*
 * Implementación de funciones púbicas
 * ===================================
 */
void moveMotorsTask(void *param)
{
	g_block_t gblock;

	while(1) {

	// Try to take the movement semaphore
	//if((xSemaphoreTake(xSemaphore, portMAX_DELAY)) == pdTRUE) {
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
		// Vuelvo a dar el semaforo
		//xSemaphoreGive(xSemaphore);
//	}
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
	} else if(accel < 20){
		motion_config.accel = 20;
	} else {
		motion_config.accel = accel;
	}
}

void line_move(float newx, float newy, float newz)
{
	step_count_t pos;

	// Velocidad de los motores, la inicializo con la velocidad mínima
	float vel = MIN_VEL_STEPS_PER_SECOND;

	const int32_t new_stepx = newx * STEPS_PER_MM;
	const int32_t new_stepy = newy * STEPS_PER_MM;
	const int32_t new_stepz = newz * STEPS_PER_MM;

	// Inicio el protocolo de interpolacion
	interpolation_set_deltas(new_stepx, new_stepy, new_stepz);
	interpolation_init();

	// Seteo la velocidad máxima dependiendo de las distancia y la
	// aceleracón que se seteo. Si la aceleración es baja y las distancias
	// cortas no se podrá ir a la velocidad máxima
	set_up_velocity(get_updated_position(),
			get_max_target_steps(),
			get_drive_delta(),
			get_step_dir());

	pos = init_pos;
	while(pos != max_new_steps) {
		// STEP 1: Correr un ciclo de interpolacion y mover los motores
		interpolation_run_cycle();
		// STEP 2: Calcular la velocidad teniendo en cuenta la aceleración
		//updateVelocity(pos, &vel, motion_config, xxs);
		updateVelocity(pos, &vel, motion_config);
		// STEP 3: Poner un delay para manejar la velocidad de giro
		vTaskDelay(pdMS_TO_TICKS(my_abs(1000/vel)));
		// STEP4: Actualizar la posición
		pos = get_updated_position();
	}

}

// TODO: ACELERACION
// No es tan facil meter aceleración en el movimiento rápido
// A menos que mueva un eje a la vez. Me parece que en el movimeinto rápido lo
// único que puedo hacer es moverme a una velocidad relativamente baja. Por que
// al no haber aceleración muy probablemente pierda pasos. VER
void fast_move(float newx, float newy, float newz)
{
	step_count_t pos;
	float vel = MIN_VEL_STEPS_PER_SECOND;
	int32_t delta_x, delta_y, delta_z;
	uint8_t dir_x, dir_y, dir_z;
	const int32_t new_stepx = newx * STEPS_PER_MM;
	const int32_t new_stepy = newy * STEPS_PER_MM;
	const int32_t new_stepz = newz * STEPS_PER_MM;

	delta_x = new_stepx - position_get_x();
	delta_y = new_stepy - position_get_y();
	delta_z = new_stepz - position_get_z();

	dir_x = my_direc(delta_x);
	dir_y = my_direc(delta_y);
	dir_z = my_direc(delta_z);

	const int8_t xs = (delta_x == 0) ? 0 : ((delta_x > 0) ? 1 : -1);
	const int8_t ys = (delta_y == 0) ? 0 : ((delta_y > 0) ? 1 : -1);
	const int8_t zs = (delta_z == 0) ? 0 : ((delta_z > 0) ? 1 : -1);

	delta_x = my_abs(delta_x);
	delta_y = my_abs(delta_y);
	delta_z = my_abs(delta_z);

	// Tengo que agarrar el MOVIMIENTO minimo
	// si delta_XXX == 0 -> No hay movimiento
	if(delta_x >= delta_y && delta_x >= delta_z) {
		set_up_velocity(position_get_x(),
				new_stepx,
				delta_x,
				xs);
		motion_config.pos = position_get_x;
	}

	if(delta_y >= delta_x && delta_y >= delta_z) {
		set_up_velocity(position_get_y(),
				new_stepy,
				delta_y,
				ys);
		motion_config.pos = position_get_y;
	}

	if(delta_z >= delta_y && delta_z >= delta_x) {
		set_up_velocity(position_get_z(),
				new_stepz,
				delta_z,
				zs);
		motion_config.pos = position_get_z;
	}

	bool_t xStop = false;
	bool_t yStop = false;
	bool_t zStop = false;

	pos = motion_config.pos();

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
		updateVelocity(pos, &vel, motion_config);
		vTaskDelay(1000/vel);
		pos = motion_config.pos();
	}
}


// TODO: ACELERACION
void home_all()
{

	speed_t delay = pdMS_TO_TICKS(10);

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
