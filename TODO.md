Cosas para hacer
================

1. Ver [este archivo](~/Repositories/Contiki-ng-Attacks/os/dev/serial-line.c) para sacar ideas de un buffer de lineas
1. Ver la funcion `line` [del archivo](~/Repositories/GcodeCNCDemo/GcodeCNCDemo4AxisCNCShield/GcodeCNCDemo4AxisCNCShield.ino)



UPDATED TODO
------------

1. Re diseñar el modulo de código G, para obter datos
1. Ver como actualizar la acceleracion en motion
1. Agregar algunos comandos


UPDATED
-------

1. Ver como hacer para pausar el programa: Una opcion es hacer una tarea periodica 
que lea la variable gcode.pause y se fije si es verdadera. Si es verdadera entones 
este timer no tiene que dar el semaforo de movimiento habilitado. Si es falsa da
el semáforo. Hay que ver que prioridad y con que tiempo se ejecuta a esta tarea

2. Otra tarea puede ser la que envía el carácter que hay espacio en la cola de recepción.
Esta puede ser un timer que envía el caracter si hay espacio ? si se le da un semáforo ? 
Notificaciones ?

    - ESP manda caracter de inicio de transmisiosn espera respuesta de la ciaa.
    - ESP manda lines y espera respuesta, si no la recibe entonces manda de nuevo la linea.
    - ESP manda caracter de fin de transmison.

2.bis. Otra opcion puede ser que cada vez que el esp manda una linea se quede esperando respuesta. Si no recibe respuesta 
en un a cantidad de tiempo entonces asume que no se puedo porcesar y la manda de nuevo hasta que alguien le conesta

3. Un semáforo hay que darlo y una tarea lo toma. Cuando lo toma no hay que darlo nuevamente. Hay que esperar que
la tarea que sincorniza de el semáforo

