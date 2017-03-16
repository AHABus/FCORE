/* Blink FreeRTOSConfig overrides.

   This is intended as an example of overriding some of the default FreeRTOSConfig settings,
   which are otherwise found in FreeRTOS/Source/include/FreeRTOSConfig.h
*/

#define configUSE_PREEMPTION        1
#define configMINIMAL_STACK_SIZE    128

#include_next<FreeRTOSConfig.h>

