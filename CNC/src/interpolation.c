#include "interpolation.h"

// Prototipo de funciones privadas
static void run_bersenham_interpolation();
static void set_x_is_driving();
static void set_y_is_driving();
static void set_z_is_driving();

typedef void (*move_motor)(uint8_t);
typedef step_count_t (*get_position)(void);
typedef void (*increment_position)(const int8_t);

typedef struct {
	move_motor move;
	get_position get_pos;
	increment_position inc_pos;
} axis_t;


// Variables locales del algoritmo de interpolación
static int32_t move_decision_one;
static int32_t move_decision_two;

// cantidad de pasos que hay que moverse
// OBS: Con delta se calcula xs y la dirección de giro
// despues ver de tomar valor absoluto
static int32_t delta_drive_axis;
static int32_t delta_second_axis;
static int32_t delta_third_axis;

// Funciones de cada eje
static axis_t driving;
static axis_t second;
static axis_t third;

static int8_t drive_step;
static int8_t second_step;
static int8_t third_step;

static uint8_t motor_drive_dir;
static uint8_t motor_second_dir;
static uint8_t motor_third_dir;

// Default value
static max_axis_t max_axis = X_AXIS;

static step_count_t max_target_pos;

static int32_t my_abs(int32_t t)
{
	return (t > 0) ? t : -t;
}

static void set_max_axis(uint32_t x, uint32_t y, uint32_t z)
{
	if(x >= y && x >= z) {
		max_axis = X_AXIS;
		return;
	}
	if(y >= x && y >= z) {
		max_axis = Y_AXIS;
		return ;
	}
	if(z >= x && z >= y) {
		max_axis = Z_AXIS;
		return;
	}
	return;
}

static uint8_t my_direc(int32_t d)
{
	return (d < 0) ? LEFT:RIGHT;
}

static int32_t update_step_direction(const int32_t delta)
{
	return (delta == 0) ? 0 : ((delta > 0) ? 1 : -1);
}

void interpolation_init()
{
	//hacer una funcion static
	//bresenham_init_agorithm
	move_decision_one = (delta_second_axis << 1) - delta_drive_axis;
	move_decision_two = (delta_third_axis << 1) - delta_drive_axis;

	// Esto euivale al xs ys zs
	drive_step = update_step_direction(delta_drive_axis);
	second_step = update_step_direction(delta_second_axis);
	third_step = update_step_direction(delta_third_axis);

	motor_drive_dir = my_direc(drive_step);
	motor_second_dir = my_direc(second_step);
	motor_third_dir = my_direc(third_step);

	// Luego de setear las direcciones guardo el valor absoluto
	// del delta
	delta_drive_axis = my_abs(delta_drive_axis);
	delta_second_axis = my_abs(delta_second_axis);
	delta_third_axis = my_abs(delta_third_axis);
}


void interpolation_run_cycle()
{
	run_bersenham_interpolation();
}

static void run_bersenham_interpolation()
{
	driving.inc_pos(drive_step);
	driving.move(motor_drive_dir);

	if(move_decision_one >= 0) {
			second.inc_pos(second_step);
		if(second_step) {
			second.move(motor_second_dir);
		}
			move_decision_one -= (delta_drive_axis<<1);

	}
	if(move_decision_two >= 0) {
			third.inc_pos(third_step);
		if(third_step) {
			third.move(motor_third_dir);
		}
		move_decision_two -= (delta_drive_axis << 1);
	}

	// Update decision
	move_decision_one += (delta_second_axis<<1);
	move_decision_two += (delta_third_axis<<1);
}


void interpolation_set_deltas(int32_t new_stepx,
			      int32_t new_stepy,
			      int32_t new_stepz)
{
	int32_t delta_x = new_stepx - position_get_x();
	int32_t delta_y = new_stepy - position_get_y();
	int32_t delta_z = new_stepz - position_get_z();

	set_max_axis(my_abs(delta_x), my_abs(delta_y), my_abs(delta_z));

	switch (max_axis) {
	case X_AXIS:
		delta_drive_axis = delta_x;
		delta_second_axis = delta_y;
		delta_third_axis = delta_z;
		set_x_is_driving();
		max_target_pos = new_stepx;
		break;
	case Y_AXIS:
		delta_drive_axis = delta_y;
		delta_second_axis = delta_x;
		delta_third_axis = delta_z;
		set_y_is_driving();
		max_target_pos = new_stepy;
		break;
	case Z_AXIS:
		delta_drive_axis = delta_z;
		delta_second_axis = delta_y;
		delta_third_axis = delta_x;
		set_z_is_driving();
		max_target_pos = new_stepz;
		break;
	default:
		;
	}

}

static void set_x_is_driving()
{
	driving.get_pos = position_get_x;
	driving.inc_pos = position_x_increment;
	driving.move = motor_x_move;

	second.get_pos = position_get_y;
	second.inc_pos = position_y_increment;
	second.move = motor_y_move;

	third.get_pos = position_get_z;
	third.inc_pos = position_z_increment;
	third.move = motor_z_move;
}



static void set_y_is_driving()
{
	driving.get_pos = position_get_y;
	driving.inc_pos = position_y_increment;
	driving.move = motor_y_move;

	second.get_pos = position_get_x;
	second.inc_pos = position_x_increment;
	second.move = motor_x_move;

	third.get_pos = position_get_z;
	third.inc_pos = position_z_increment;
	third.move = motor_z_move;
}

static void set_z_is_driving()
{
	driving.get_pos = position_get_z;
	driving.inc_pos = position_z_increment;
	driving.move = motor_z_move;

	second.get_pos = position_get_y;
	second.inc_pos = position_y_increment;
	second.move = motor_y_move;

	third.get_pos = position_get_x;
	third.inc_pos = position_x_increment;
	third.move = motor_x_move;
}



max_axis_t get_driving_axis()
{
	return max_axis;
}

step_count_t get_updated_position()
{
	return driving.get_pos();
}

step_count_t get_max_target_steps()
{
	return max_target_pos;
}

int32_t get_step_dir()
{
	return drive_step;
}

int32_t get_drive_delta()
{
	return delta_drive_axis;
}


