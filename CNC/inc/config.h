#ifndef _CONFIG__H_
#define _CONFIG__H_


/*
 * Poner toda las configuraciones del usuario aca
*/



/*
 * Paso por vuelta, esta variable dependerá de cada motor
 * en este caso estoy usando un motor que vanaza 1.8° por
 * paso. Entonces 360° / 1.8° = 200 pasos/vuelta
*/
#define STEPS_PER_REV		200

/*
 * Pasos por milimetro, esto depende de la varilla o correa
 * que se use. En este caso estoy usando una varilla que avanza
 * 1mm por vuelta, y como para dar una vuelta tengo que dar
 * 200 pasos -> 200 pasos/mm
*/
#define STEPS_PER_MM		200

/*
 * Configuracion del motor del eje X. Actualizar estos defines
 * con el pin que se utilice para controlar cada motor
*/
#define MOTOR_X_DIR		GPIO1
#define MOTOR_X_STEP		GPIO3

/*
 * Configuracion del motor del eje Y
*/
#define MOTOR_Y_DIR		GPIO2
#define MOTOR_Y_STEP		GPIO4

/*
 * Configuracion del motor del eje X
*/
#define MOTOR_Z_DIR		GPIO5
#define MOTOR_Z_STEP		GPIO7

/*
 * Motor gira a la derecha o izquierda. Con esto se pueden cambiar
 * los sentidos de giro de los motores. Asumiendo que se conectaron
 * todos de la misma forma
 * TODO: Ver de hacer una configuración distinta para cada motor
*/
#define LEFT			0
#define RIGHT			~LEFT

/*
 * El Baudrate de la comunicación serie
*/
#define COM_BAUDRATE		115200
#define UART_PORT		UART_USB


/*
 * Home direction X
*/
#define HOMEX			0



#endif
