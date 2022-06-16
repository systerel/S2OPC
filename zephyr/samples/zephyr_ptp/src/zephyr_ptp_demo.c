/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(net_gptp_sample, LOG_LEVEL_DBG);

#include <stdlib.h>
#include <stdio.h>
#include <zephyr.h>
#include <errno.h>
#include <console/console.h>

#include "sopc_macros.h"
#include "sopc_mutexes.h"
#include "sopc_time.h"
#include "sopc_threads.h"
#include "sopc_assert.h"
#include "sopc_async_queue.h"
#include "sopc_mem_alloc.h"

#include "p_time.h"

#include <net/net_core.h>
#include <net/gptp.h>
#include "ethernet/gptp/gptp_messages.h"
#include "ethernet/gptp/gptp_data_set.h"

#define SECOND_TO_100NS (10000000)
#define MS_TO_100NS (10000)
#define MS_TO_US (1000)
#define ONE_SECOND_MS (1000)

static int gSubTestId = 0;
static int gTestId = 0;
static int gParamI = 0;
static Mutex gMutex;

static const char* sourceToString(const SOPC_Time_TimeSource source)
{
    switch (source) {
    case SOPC_TIME_TIMESOURCE_INTERNAL:   return "INTERNAL";
    case SOPC_TIME_TIMESOURCE_PTP_SLAVE:  return "PTP_SLAVE";
    case SOPC_TIME_TIMESOURCE_PTP_MASTER: return "PTP_MASTER";
        default: return "INVALID";
    }
}

/***************************************************/
static void gptp_phase_dis_cb(uint8_t *gm_identity,
                              uint16_t *time_base,
                              struct gptp_scaled_ns *last_gm_ph_change,
                              double *last_gm_freq_change)
{
    SOPC_UNUSED_ARG(gm_identity);
    SOPC_UNUSED_ARG(time_base);
    SOPC_UNUSED_ARG(last_gm_ph_change);
    SOPC_UNUSED_ARG(last_gm_freq_change);
    /* Note:
     * Monitoring phase discontinuities is not used in current implementation
     * but may be used to correct/smooth time corrections
     * For example:
     */
//    const uint64_t discontinuity = last_gm_ph_change->high << 32 | last_gm_ph_change->low;
}

static const char* DateToString(const SOPC_DateTime dt)
{
    static char dateOnce [10];
    struct tm tmDate;
    time_t seconds = 0;
    SOPC_ReturnStatus status = SOPC_Time_ToTimeT(dt, &seconds);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    status = SOPC_Time_Breakdown_UTC(seconds, &tmDate);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    size_t res = strftime(dateOnce, sizeof(dateOnce) - 1, "%H:%M:%S", &tmDate);
    SOPC_ASSERT(res == 8);
    return dateOnce;
}

static const char* DateToTimeString(const SOPC_DateTime dt)
{
    static char dateOnce [20];
    // dt / MS_TO_100NS = number of milliseconds
    // Avoid overflow by removing part greater than 1 hour
    const uint32_t i_ms = (uint32_t) ((dt / MS_TO_100NS) % (1000 * 60 * 60));

    const uint32_t i_s = (i_ms / MS_TO_US);
    const uint32_t i_min = (i_s / 60);

    snprintf(dateOnce, sizeof(dateOnce), "%02d m %02d s %03d ms",
            i_min % 60,
            i_s % 60,
            i_ms % MS_TO_US
            );
//    char* date = SOPC_Time_GetStringOfCurrentLocalTime(false);
//    SOPC_ASSERT(NULL != date);
//
//    strncpy(dateOnce, date + 11, sizeof(dateOnce));
//    SOPC_Free(date);
    return dateOnce;
}

static SOPC_AsyncQueue* printQueue;

static void* print_thread(void* context)
{
    SOPC_UNUSED_ARG(context);
    for (;;)
    {
        char* element;
        SOPC_ReturnStatus result = SOPC_AsyncQueue_BlockingDequeue(printQueue, (void**)&element);
        SOPC_ASSERT(result ==SOPC_STATUS_OK);
        printk("%s", element);
        SOPC_Free(element);
    }
    return NULL;
}

static void synch_printf(const char* format, ...)
{
    char* buffer = SOPC_Malloc(80);
    SOPC_ASSERT(NULL != buffer);

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 80, format, args);
    va_end(args);
    SOPC_AsyncQueue_BlockingEnqueue(printQueue, (void**)buffer);
}

static void* test_thread(void* context)
{
    SOPC_UNUSED_ARG(context);
    SOPC_RealTime t0;
    SOPC_RealTime rt1;
    SOPC_RealTime rt2;
    SOPC_DateTime dt1 = SOPC_Time_GetCurrentTimeUTC();
    SOPC_DateTime dt2;
    int previousCorrection = 0;

    SOPC_Time_TimeSource source = SOPC_TIME_TIMESOURCE_INTERNAL;

    SOPC_RealTime_GetTime (&t0);

    for (;;)
    {
        // Test management
        Mutex_Lock(&gMutex);
        const bool newTest = (gSubTestId == 0);
        gSubTestId ++;
        const int testId = gTestId;
        Mutex_Unlock(&gMutex);

        const SOPC_Time_TimeSource newSource = SOPC_Time_GetTimeSource();
        if (newSource != source)
        {
            source = newSource;
            synch_printf("[II] Time source changed to %s\n", sourceToString(source));
        }

        const int corrPrecent = -(int)(1000 * (SOPC_RealTime_GetClockCorrection() - 1.0));
        if (corrPrecent != previousCorrection)
        {
            previousCorrection = corrPrecent;
            synch_printf("[II] Time Correction changed to %3d.%01d%%\n", corrPrecent /10, abs(corrPrecent %10));
        }
        if (testId == 1)
        {
            if (newTest)
            {
                synch_printf("\n");
                synch_printf("===================================\n");
                synch_printf("Test #1: Requesting 'Sleep(%dms)'\n\n", ONE_SECOND_MS);
                synch_printf("-----------------------------------\n");
                synch_printf("Delta RT, delta DateTime, Ptp Corr\n");
            }
            SOPC_RealTime_GetTime (&rt1);
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            SOPC_Sleep(ONE_SECOND_MS);
            SOPC_RealTime_GetTime (&rt2);
            dt2 = SOPC_Time_GetCurrentTimeUTC();
            const int deltaRt100us = (rt2.tick100ns - rt1.tick100ns) / MS_TO_US;
            const int deltaDt100us = (dt2 - dt1) / MS_TO_US;
            synch_printf("dRT=(%3d.%01d ms) dDT=(%3d.%01d ms)\n",
                    deltaRt100us /10, abs(deltaRt100us %10),
                    deltaDt100us /10, abs(deltaDt100us %10));
        }
        else if (testId == 2)
        {
            if (newTest)
            {
                synch_printf("\n");
                synch_printf("========================================\n");
                synch_printf("Test #2: Requesting 'SleepUntil(+%dms)'\n\n", ONE_SECOND_MS);
                synch_printf("----------------------------------------\n");
                synch_printf("Delta RT, delta DateTime, Ptp Corr\n");
            }
            SOPC_RealTime_GetTime (&rt1);
            SOPC_RealTime_GetTime (&rt2);
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            SOPC_RealTime_AddSynchedDuration(&rt2, ONE_SECOND_MS * MS_TO_US, 0);
            SOPC_RealTime_SleepUntil(&rt2);
            dt2 = SOPC_Time_GetCurrentTimeUTC();
            const int deltaRt100us = (rt2.tick100ns - rt1.tick100ns) / MS_TO_US;
            const int deltaDt100us = (dt2 - dt1) / MS_TO_US;
            synch_printf("dRT=(%4d.%01d ms) dDT=(%4d.%01d ms) DT=(%s)\n",
                    deltaRt100us /10, abs(deltaRt100us %10),
                    deltaDt100us /10, abs(deltaDt100us %10),
                    DateToString(dt2));
        }
        else if (testId == 3)
        {
            static int pre = 0;
            // Chrono test
            if (newTest)
            {
                synch_printf("\n");
                synch_printf("========================================\n");
                synch_printf("Test #3: Chrono test\n\n");
                synch_printf("----------------------------------------\n");
                SOPC_RealTime_GetTime (&rt1);
                dt1 = SOPC_Time_GetCurrentTimeUTC();
                pre = 5;
            }

            if (pre > 0)
            {
                synch_printf("Starting in %d\n", pre);
            }
            else if (pre == 0)
            {
                synch_printf(" !!! GO !!!\n");
                SOPC_RealTime_GetTime (&rt1);
                dt1 = SOPC_Time_GetCurrentTimeUTC();
            }
            SOPC_Sleep(ONE_SECOND_MS);
            pre --;

            if (pre < -1)
            {
                dt2 = SOPC_Time_GetCurrentTimeUTC();
                SOPC_RealTime_GetTime (&rt2);

                const int deltaRts = (rt2.tick100ns - rt1.tick100ns) / SECOND_TO_100NS;
                const int deltaDts = (dt2 - dt1) / SECOND_TO_100NS;
                synch_printf("dRT=(%6d s) dDT=(%6d s)\n",
                        deltaRts,
                        deltaDts);
            }
        }
        else if (testId == 4)
        {
            static const uint64_t periodMs = 1000;
            const uint64_t offsetMs = gParamI;
            if (newTest)
            {
                synch_printf("\n");
                synch_printf("========================================\n");
                synch_printf("Test #4: Requesting 'SleepUntil(Per=%ums/Off=%ums)'\n\n", (unsigned)periodMs, (unsigned)offsetMs);
                synch_printf("----------------------------------------\n");
            }
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            SOPC_RealTime_GetTime (&rt2);
            SOPC_RealTime_AddSynchedDuration(&rt2, periodMs * MS_TO_US, offsetMs * MS_TO_US);
            // printk("RT2B=%llu\n",rt2.tick100ns);
            SOPC_RealTime_SleepUntil(&rt2);
            dt2 = SOPC_Time_GetCurrentTimeUTC();

            synch_printf("Event woken from DT=(%s)",DateToTimeString(dt1));
            synch_printf(" at DT=(%s)\n",DateToTimeString(dt2));
        }
        else
        {
            // No test
            if (newTest)
            {
                synch_printf("\n");
                synch_printf("No test running\n");
            }
            SOPC_Sleep(10);
        }
    }

    return NULL;
}

void main(void)
{
    printk("\nBUILD DATE : " __DATE__ " " __TIME__ "\n");

	static struct gptp_phase_dis_cb phase_dis;
	gptp_register_phase_dis_cb(&phase_dis, gptp_phase_dis_cb);

	SOPC_ReturnStatus result = SOPC_AsyncQueue_Init(&printQueue, "PRINT");
	SOPC_ASSERT(SOPC_STATUS_OK == result);

    console_getline_init();
    Thread thread = P_THREAD_Create(&test_thread, NULL, "demo", CONFIG_SOPC_THREAD_DEFAULT_PRIORITY, false);
    Thread threadPrint = P_THREAD_Create(&print_thread, NULL, "print", CONFIG_SOPC_THREAD_DEFAULT_PRIORITY, false);

    SOPC_ASSERT(thread != NULL);
    SOPC_ASSERT(threadPrint != NULL);

    printk("\nZephyr PtP demo running\n\n");
    printk("Enter command:\n");
    printk(" - 's' to start 'SLEEP' test\n");
    printk(" - 'o[ms offset=25]' to start 'SLEEP_UNTIL/Offset' test\n");
    printk(" - 'u' to start 'SLEEP_UNTIL' test\n");
    printk(" - 'c' to start 'CHRONOMETER' test\n");
    printk(" - ' ' to stop current test\n");

	for (;;)
	{
        const char* line = console_getline();

        Mutex_Lock(&gMutex);
        if (NULL == line)
        {
            printk("\n Serial read failed\n");
            SOPC_ASSERT(false);
        }
        else if (line[0] < ' ') // Empty or invalid line : stop tests
        {
            gSubTestId  = 0;
            gTestId = 0;
        }
        else if (line[0] == 's') // Sleep test
        {
            gSubTestId  = 0;
            gTestId = 1;
        }
        else if (line[0] == 'u') // Sleep until test
        {
            gSubTestId  = 0;
            gTestId = 2;
        }
        else if (line[0] == 'o') // Sleep until test
        {
            gSubTestId  = 0;
            gTestId = 4;
            if (line[1] > 0)
            {
                gParamI = atoi (line + 1);
            }
            else
            {
                gParamI = 25;
            }
        }
        else if (line[0] == 'c') // Chronometer start
        {
            gSubTestId = 0;
            gTestId = 3;
        }
        else
        {
            printk("\n[EE] Invalid command ; <%s>\n", line);
        }

        Mutex_Unlock(&gMutex);
	}
}
