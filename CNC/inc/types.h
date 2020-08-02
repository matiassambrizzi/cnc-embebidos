#ifndef _TYPES__H_
#define _TYPES__H_

typedef struct {
	float px;
	float py;
	float pz;
} position_t;

typedef enum {
	FAST_MOVMENT,
	LINE,
	ARC, //TODO.
	HOMING
} movment_type_t;


typedef enum {
	MILLIMETERS,
	INCHES
} measurement_units_t;

typedef enum {
	RELATIVE,
	ABSOLUTE
} coordinates_t;

#endif
