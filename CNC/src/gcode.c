#include "gcode.h"

// Instancia Privada de codigoG recibido por UART
static g_block_t rx_gcode;

// TODO: Ver esta función
static TickType_t speed_to_ticks(uint8_t s)
{
	return pdMS_TO_TICKS(101-s%101);
}

void gcode_block_reset()
{
	position_reset();
	rx_gcode.target_pos.px = 0;
	rx_gcode.target_pos.py = 0;
	rx_gcode.target_pos.pz = 0;
	rx_gcode.type = FAST_MOVMENT;
	rx_gcode.speed = speed_to_ticks(90);
	rx_gcode.velocity = MAX_VEL_STEPS_PER_SECOND * 0.9;
	rx_gcode.units = MILLIMETERS;
	rx_gcode.pause = false;
}


void gcode_block_set_x(const float x)
{
	rx_gcode.target_pos.px = x;
}

void gcode_block_set_y(const float y)
{
	rx_gcode.target_pos.py = y;
}

void gcode_block_set_z(const float z)
{
	rx_gcode.target_pos.pz = z;
}


void gcode_block_set_speed(const float s)
{
//	s es un valor porcentual
	if(s <= 20 || s > 100) {
		return;
	}
	const float aux = s / 100.0;
	rx_gcode.velocity = MAX_VEL_STEPS_PER_SECOND * aux;
}

void gcode_block_set_movment(const movment_type_t m)
{
	rx_gcode.type = m;
}


speed_t gcode_block_get_speed()
{
	return rx_gcode.speed;
}

movment_type_t gcode_block_get_movement()
{
	return rx_gcode.type;
}

position_t * gcode_block_get_position()
{
	return (position_t *) (&(rx_gcode.target_pos));
}


gBlockPtr gcode_get_block()
{
	return (gBlockPtr) (&(rx_gcode));
}


bool_t gcode_get_pause()
{
	return rx_gcode.pause;
}

void gcode_set_pause(const bool_t p)
{
	rx_gcode.pause = p;
}

