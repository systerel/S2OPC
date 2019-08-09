#include <stdint.h>

#include "FreeRTOSConfig.h"

#if (configUSE_BOGOMIPS == 1)
/* portable version */
static void delay(int loops)
{
    long i;
    for (i = loops; i >= 0; i--)
        ;
}

float external_bogomips()
{
    unsigned long loops_per_sec = 1;
    unsigned long ticks;

    while ((loops_per_sec <<= 1))
    {
        ticks = portGET_RUN_TIME_COUNTER_VALUE();
        delay(loops_per_sec);
        ticks = portGET_RUN_TIME_COUNTER_VALUE() - ticks;
        if (ticks >= 10000)
        {
            loops_per_sec = (loops_per_sec / ticks) * 10000;
            return (float) loops_per_sec / 500000;
        }
    }
    return -1;
}
#endif
