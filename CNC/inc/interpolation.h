#ifndef _INTERPOLATION__H_
#define _INTERPOLATION__H_

#ifdef TEST_ALL
#include <stdint.h>
#include <stdio.h>
#include "test_utils.h"
#endif

#ifndef TEST_ALL
#include "sapi.h"
#endif

#include "motors.h"
#include "position.h"
#include "config.h"

typedef enum {
	X_AXIS,
	Y_AXIS,
	Z_AXIS
} max_axis_t;

/**
* @brief Funcion para inicializar el algoritmo de interpolación. Inicialzia las
* distintas variables que se utilizan en el algoritmo que se implemente
* @return nothing
*/
void interpolation_init();

/**
* @brief Funcion para ejecutar un ciclo del algoritmo de interpolación que se
* utilice.
* @return nothing
*/
void interpolation_run_cycle();

/**
* @brief Esta función devolver cual es el eje que dará la mayor cantidad de
* pasos. Este eje es que se utiliza de referencia en el algoritmo de
* interpolación
* @param
* @return Variable del tipo max_axis_t
*/
max_axis_t get_driving_axis();

/**
* @brief Funcion que devuelve la mayot cantidad de pasos que se tiene que dar para
* llegar a la posición target
* @return step_count_t maxima cantidad de pasos
*/
step_count_t get_max_target_steps();

/**
* @brief Funcion que devuelve la direccion del movimiento del eje que da la
* mayor cantidad de pasos
* @return 1 o -1.
*/
int32_t get_step_dir();

int32_t get_drive_delta();

/**
* @brief Funion que devuelve la posición actualizada del eje que da la mayor
* conatidad de pasos. i.e el eje que drivea el algoritmo de interpolación lineal
* @return Posición actual del eje drive
*/
step_count_t get_updated_position();

// Seteo de las deltas y el máximo axis
void interpolation_set_deltas(int32_t delta_drive,
			      int32_t delta_second,
			      int32_t delta_third);


#endif
