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
