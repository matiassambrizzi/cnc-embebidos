#ifndef _CONFIG__H_
#define _CONFIG__H_
/*
 * Poner toda las configuraciones del usuario aca
*/

#define DEBUG

/*
 * Paso por vuelta, esta variable dependerá de cada motor
 * en este caso estoy usando un motor que vanaza 1.8° por
 * paso. Entonces 360° / 1.8° = 200 pasos/vuelta
*/
#define STEPS_PER_REV			(200)

/*
 * Pasos por milimetro, esto depende de la varilla o correa
 * que se use. En este caso estoy usando una varilla que avanza
 * 1mm por vuelta, y como para dar una vuelta tengo que dar
 * 200 pasos -> 200 pasos/mm
*/

/*
 * La correa GT2 tiene una resolucion de 0.2mm / paso o 5 pasos/mm
 * Si uso microstepping de 1/4 -> 200 pasos / vuelta -> 800 pasos / vuelta
 */
#define STEPS_PER_MM			(5)
// TODO: Poner microstepping 1/16 para tener mucha mas res
#define STEPS_PER_MM_X			(20)
#define STEPS_PER_MM_Y			(20)
#define STEPS_PER_MM_Z			(320)

/*
 * Configuracion del motor del eje X. Actualizar estos defines
 * con el pin que se utilice para controlar cada motor
*/
#define MOTOR_X_DIR			(GPIO1)
#define MOTOR_X_STEP			(GPIO3)

/*
 * Configuracion del motor del eje Y
*/
#define MOTOR_Y_DIR			(GPIO5)
#define MOTOR_Y_STEP			(GPIO7)

/*
 * Configuracion del motor del eje X
*/
#define MOTOR_Z_DIR			(GPIO2)
#define MOTOR_Z_STEP			(GPIO4)

/*
 * Motor gira a la derecha o izquierda. Con esto se pueden cambiar
 * los sentidos de giro de los motores. Asumiendo que se conectaron
 * todos de la misma forma
 * TODO: Ver de hacer una configuración distinta para cada motor
*/
#define LEFT				(0)
#define RIGHT				(~LEFT)

/*
 * El Baudrate de la comunicación serie
*/
#define COM_BAUDRATE			(115200)
#define UART_PORT			(UART_USB)
//#define UART_PORT			(UART_232)


//TODO: Definir los pines de los endstops
#define END_STOP_X			(GPIO8)
#define END_STOP_Y			(GPIO8)
#define END_STOP_Z			(GPIO8)

/*
 * Home direction X
*/
#define HOMEX				(0)

/**
* Maxima cantidad de caracters que se almacenana en el buffer de rx uart
*/
#define MAX_RX_BUFFER			(80)

#define	READY_TO_PROCESS_CHAR		('$')

#define TESTING_ACC

// maxima Velocidad medida en pasos por segundo
// 300 Steps/second -> Se taduce en un delay de aproximadamente 3ms
#define MAX_VEL_STEPS_PER_SECOND	(1000)
//Minima velocidad distinta de cero para evitar una division por cero
//en el algoritmo de aceleración
#define MIN_VEL_STEPS_PER_SECOND	(1)

#define MAX_ACCEL_STEPS_PER_SECOND_SQUARE (1000)
#endif
