#include <string.h>
#include "unity.h"
#include "uart.h"
#include "uart.c"
#include "mock_test_utils.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_uart_get_buffer(void)
{
	// Simula que me llega algo por uart
	const char *msg = "Hellodabd";

	//Copyo el mensaje a rx_line
	strcpy(rx_line, msg);

	// Creo una variable para guardar el buffer recibido
	char s[MAX_RX_BUFFER];

	TEST_ASSERT(uart_get_buffer(s) == 0);
	TEST_ASSERT(strcmp(s, msg) == 0);
}


void test_fill_in_buffer()
{
	const char msg[] = "G13asdas\n";
	uint8_t i = 0;

	while(msg[i] != '\0') {
		uartRxReady_ExpectAndReturn(UART_PORT, 1);
		uartRxRead_ExpectAndReturn(UART_PORT, msg[i]);
		i++;
	}
	uartRxReady_ExpectAndReturn(UART_PORT, 0);
	xTaskNotifyGive_Ignore();

	uart_fill_in_buff();

	TEST_ASSERT_EQUAL_STRING_LEN(msg, rx_line,
				     (sizeof(msg)/sizeof(char))-2);
}
