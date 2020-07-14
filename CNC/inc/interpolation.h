#ifndef _INTERPOLATION__H_
#define _INTERPOLATION__H_

#include "sapi.h"
#include "motors.h"
#include "position.h"
#include "config.h"

typedef enum {
	X_AXIS,
	Y_AXIS,
	Z_AXIS
} max_axis_t;

// TODO DOC!!
void interpolation_init();
void interpolation_run_cycle();
max_axis_t get_driving_axis();
step_count_t get_max_target_steps();
int32_t get_step_dir();
int32_t get_drive_delta();

step_count_t get_updated_position();
// Seteo de las deltas y el máximo axis
void interpolation_set_deltas(int32_t delta_drive,
			      int32_t delta_second,
			      int32_t delta_third);


#endif
