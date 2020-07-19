#include "unity.h"
#include <stdio.h>
#include <stdlib.h>
#include "position.h"
#include "position.c"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_position_set_x_y_z(void)
{
	TEST_MESSAGE("Testing the global pos setter");

	step_count_t x;
	step_count_t y;
	step_count_t z;

	for(int i = 0; i < 1000; i++) {
		x = rand();
		y = rand();
		z = rand();

		position_set_x(x);
		position_set_y(y);
		position_set_z(z);

		TEST_ASSERT_EQUAL_INT32(x, actual_pos.px);
		TEST_ASSERT_EQUAL_INT32(y, actual_pos.py);
		TEST_ASSERT_EQUAL_INT32(z, actual_pos.pz);
	}
}
