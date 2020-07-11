#ifndef _POSITION__H_
#define _POSITION__H_

#include "sapi.h"

typedef int32_t step_count_t;

typedef struct {
	step_count_t px;
	step_count_t py;
	step_count_t pz;
} steps_t;

/**
* @brief resetear la posición actual
* @return nothing
*/
void position_reset();

void position_set_x(const step_count_t x);
void position_set_y(const step_count_t y);
void position_set_z(const step_count_t z);

void position_x_increment(const int8_t x);
void position_y_increment(const int8_t y);
void position_z_increment(const int8_t z);

step_count_t position_get_x();
step_count_t position_get_y();
step_count_t position_get_z();
#endif
