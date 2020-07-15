#ifndef _MOTORS__H_
#define _MOTORS__H

#include "sapi.h"
#include "config.h"

/**
* @brief Inicializa los pines que se corresponden con cada motor. Los pines son
* el de dirección, step y el fin de carrera. Los inicializa como entrada o
* salida segun corresponda. Los pines que se utilizan se indican en <config.h>
* @return nothing
*/
void motorInit(void);

/**
* @brief Asocia cada pin a cada motor
* @param step_pin_x,y,z Es el pin del micro que se utiliza para enviar el
* comando de los pasos a cada motor.
* @param dir_pin_x,y,z Es el pin que indica la dirección de rotación del motor
* @param end_stop_x,y,z Es el pin al cual esta conectado el sensor de fin de
* carrera de cada motor
* @return nothing
*/
void motor_config(uint8_t step_pin_x, uint8_t dir_pin_x, uint8_t end_stop_x,
		  uint8_t step_pin_y, uint8_t dir_pin_y, uint8_t end_stop_y,
		  uint8_t step_pin_z, uint8_t dir_pin_z, uint8_t end_stop_z);

/**
* @brief ESTA FUNCIÓN NO SE USA
*/
void move_steps_x(uint32_t nsteps, bool_t dir);

/**
* @brief Mueve el motor x,y,z un paso en la dirección indicada por dir
* @param dir puede ser LEFT o RIGHT (ver config.h)
* @return nothing
*/
void motor_x_move(uint8_t dir);
void motor_y_move(uint8_t dir);
void motor_z_move(uint8_t dir);

/**
* @brief Función para chequear si el sensor de fin de carrera esta presionado
* @return true si esta presionado, de lo contrario devuelve falso.
*/
bool_t end_stop_x_is_press(void);
bool_t end_stop_y_is_press(void);
bool_t end_stop_z_is_press(void);

#endif
