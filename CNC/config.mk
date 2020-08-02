# Compile options

VERBOSE=y
OPT=g
USE_NANO=y
SEMIHOST=n
USE_FPU=y

## Cosas de freeRTOS
USE_FREERTOS=y
## en heap 1 freertos no usa memoria dinámica para
## alocar las tareas en memoria. 
FREERTOS_HEAP_TYPE=1
# Libraries

USE_LPCOPEN=y
USE_SAPI=y
DEFINES+=SAPI_USE_INTERRUPTS
DEFINES+=TICK_OVER_RTOS
DEFINES+=USE_FREERTOS
##DEFINES+=UPGRADE_LINE
