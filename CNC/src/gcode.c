#include "gcode.h"

typedef struct {
	position_t future;
	movment_type_t type;
	speed_t speed;
} g_block_t;


// Instancia Privada de codigoG recibido por UART
static g_block_t rx_gcode;


static TickType_t speed_to_ticks(uint8_t s)
{
	return pdMS_TO_TICKS(101-s%101);
}

void gcode_block_reset()
{
	position_reset();
	rx_gcode.future.px = 0;
	rx_gcode.future.py = 0;
	rx_gcode.future.pz = 0;
	rx_gcode.type = FAST_MOVMENT;
	rx_gcode.speed = speed_to_ticks(90);
}


void gcode_block_set_x(const float x)
{
	rx_gcode.future.px = x;
}

void gcode_block_set_y(const float y)
{
	rx_gcode.future.py = y;
}

void gcode_block_set_z(const float z)
{
	rx_gcode.future.pz = z;
}


void gcode_block_set_speed(const uint8_t s)
{
	rx_gcode.speed = speed_to_ticks(s);
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
	return (position_t *) (&(rx_gcode.future));
}
