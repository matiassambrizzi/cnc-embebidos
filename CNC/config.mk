# Compile options

VERBOSE=n
OPT=g
USE_NANO=y
SEMIHOST=n
USE_FPU=y

## Cosas de freeRTOS
USE_FREERTOS=y
FREERTOS_HEAP_TYPE=4
# Libraries

USE_LPCOPEN=y
USE_SAPI=y
DEFINES+=SAPI_USE_INTERRUPTS
DEFINES+=TICK_OVER_RTOS
DEFINES+=USE_FREERTOS
##DEFINES+=UPGRADE_LINE
