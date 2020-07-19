#include "unity.h"
#include <string.h>

#include "mock_test_utils.h"
#include "mock_position.h"
#include "mock_uart.h"
#include "mock_gcode.h"
#include "mock_motion.h"
#include "process_line.h"


void setUp(void)
{
}

void tearDown(void)
{
}

void test_process_line_readnumber(void)
{
	uint8_t counter = 0;
	uint8_t i = 0;
	float number;
	char rxLine[20] = "G13.231X2Y3";

	uint8_t expected_counter[] = {7, 9, 11};
	float expected_float[] = {13.231, 2.0, 3.0};
	while(rxLine[counter] != '\0') {
		read_number(rxLine, &counter, &number);
		TEST_ASSERT_EQUAL_UINT(expected_counter[i], counter);
		TEST_ASSERT_EQUAL_FLOAT(expected_float[i], number);
		i++;
	}
}



/**
* @brief Lo que quiero hacer es una funcion que me transforme un string en un
* float sin utilizar notacion cientifica. Y que me tome todo el nuero hasta
* encontrar una letra o un '\0'
*/
void test_my_string_to_float(void)
{
	char s[100] = "1";
	float number;

	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(1.0, number);

	strcpy(s, "4");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(4.0, number);

	strcpy(s, "6.4");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(6.4, number);

	strcpy(s, "7.78");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(7.78, number);

	strcpy(s, "17.78");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(17.78, number);

	strcpy(s, "-1.6");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(-1.6, number);

	strcpy(s, "EE1231231");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(0, number);

	strcpy(s, "1.3E");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(1.3, number);

	strcpy(s, "-1.3E");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(-1.3, number);

	strcpy(s, "+1.3E");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(1.3, number);

	strcpy(s, "");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(0, number);

	strcpy(s, "0.4");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(0.4, number);


	strcpy(s, "0.9999999999999999999999999999999999");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(0.9999999999999999999999999999999999,
				number);

	strcpy(s, "-.1");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(-0.1,
				number);
	strcpy(s, "1,3");
	number = stringToFloat(s);
	TEST_ASSERT_EQUAL_FLOAT(1,
				number);
}



















