#ifndef _MOTORS__H_
#define _MOTORS__H

#include "sapi.h"
#include "config.h"


void motorInit(void);

void motor_config(uint8_t step_pin_x, uint8_t dir_pin_x, uint8_t end_stop_x,
		  uint8_t step_pin_y, uint8_t dir_pin_y, uint8_t end_stop_y,
		  uint8_t step_pin_z, uint8_t dir_pin_z, uint8_t end_stop_z);

void move_steps_x(uint32_t nsteps, bool_t dir);

void motor_x_move(uint8_t dir);
void motor_y_move(uint8_t dir);
void motor_z_move(uint8_t dir);

bool_t end_stop_x_is_press(void);
bool_t end_stop_y_is_press(void);
bool_t end_stop_z_is_press(void);

#endif
