#include "motors.h"

//Estructura privada
typedef struct {
	uint8_t step_pin;
	uint8_t dir_pin;
} motor_t;

// Instancias privadas de los motores
// Estas solo se podrán acceder con funciones de la API
motor_t x_motor;
motor_t y_motor;
motor_t z_motor;

static void onestep(motor_t m)
{
	gpioWrite(m.step_pin, true);
	gpioWrite(m.step_pin, false);
}

void motor_config(uint8_t step_pin_x, uint8_t dir_pin_x,
		  uint8_t step_pin_y, uint8_t dir_pin_y,
		  uint8_t step_pin_z, uint8_t dir_pin_z)
{
	x_motor.step_pin = step_pin_x;
	x_motor.dir_pin = dir_pin_x;
	y_motor.step_pin = step_pin_y;
	y_motor.dir_pin = dir_pin_y;
	z_motor.step_pin = step_pin_z;
	z_motor.dir_pin = dir_pin_z;
}

void motor_x_move(uint8_t dir)
{
	gpioWrite(x_motor.dir_pin, dir);
	onestep(x_motor);
}

void motor_y_move(uint8_t dir)
{
	gpioWrite(y_motor.dir_pin, dir);
	onestep(y_motor);
}

void motor_z_move(uint8_t dir)
{
	gpioWrite(z_motor.dir_pin, dir);
	onestep(z_motor);
}
