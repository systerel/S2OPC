/*
 * Copyright (c) 2018 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(net_gptp_sample, LOG_LEVEL_DBG);

#include <stdlib.h>
#include <zephyr.h>
#include <errno.h>
#include <console/console.h>

#include "sopc_mutexes.h"
#include "sopc_time.h"
#include "sopc_threads.h"
#include "sopc_assert.h"
#include "sopc_mem_alloc.h"

#include "p_time.h"

#include <net/net_core.h>
#include <net/gptp.h>
#include "ethernet/gptp/gptp_messages.h"
#include "ethernet/gptp/gptp_data_set.h"

static int gSubTestId = 0;
static int gTestId = 0;
static Mutex gMutex;

static int init_app(void)
{

	return 0;
}

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
    (void)gm_identity;
    (void)time_base;
    (void)last_gm_ph_change;
    (void)last_gm_freq_change;
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
    struct tm tm;
    time_t seconds = 0;
    SOPC_ReturnStatus status = SOPC_Time_ToTimeT(dt, &seconds);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    status = SOPC_Time_Breakdown_UTC(seconds, &tm);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    size_t res = strftime(dateOnce, sizeof(dateOnce) - 1, "%H:%M:%S", &tm);
    SOPC_ASSERT(res == 8);
    return dateOnce;
}

static void* test_thread(void* context)
{
    (void)context;
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
            printk("[II] Time source changed to %s\n", sourceToString(source));
        }

        const int corrPrecent = -(int)(1000 * (SOPC_RealTime_GetClockCorrection() - 1.0));
        if (corrPrecent != previousCorrection)
        {
            previousCorrection = corrPrecent;
            printk("[II] Time Correction changed to %3d.%01d%%\n", corrPrecent /10, abs(corrPrecent %10));
        }
        if (testId == 1)
        {
            if (newTest)
            {
                printk("\n");
                printk("===================================\n");
                printk("Test #1: Requesting 'Sleep(1000ms)'\n\n");
                printk("-----------------------------------\n");
                printk("Delta RT, delta DateTime, Ptp Corr\n");
            }
            SOPC_RealTime_GetTime (&rt1);
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            SOPC_Sleep(1000);
            SOPC_RealTime_GetTime (&rt2);
            dt2 = SOPC_Time_GetCurrentTimeUTC();
            const int deltaRt100us = (rt2.tick100ns - rt1.tick100ns) / 1000;
            const int deltaDt100us = (dt2 - dt1) / 1000;
            printk("dRT=(%3d.%01d ms) dDT=(%3d.%01d ms)\n",
                    deltaRt100us /10, abs(deltaRt100us %10),
                    deltaDt100us /10, abs(deltaDt100us %10));
        }
        else if (testId == 2)
        {
            if (newTest)
            {
                printk("\n");
                printk("========================================\n");
                printk("Test #2: Requesting 'SleepUntil(+1000ms)'\n\n");
                printk("----------------------------------------\n");
                printk("Delta RT, delta DateTime, Ptp Corr\n");
            }
            SOPC_RealTime_GetTime (&rt1);
            SOPC_RealTime_GetTime (&rt2);
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            SOPC_RealTime_AddDuration(&rt2, 1000);
            SOPC_RealTime_SleepUntil(&rt2);
            dt2 = SOPC_Time_GetCurrentTimeUTC();
            const int deltaRt100us = (rt2.tick100ns - rt1.tick100ns) / 1000;
            const int deltaDt100us = (dt2 - dt1) / 1000;
            printk("dRT=(%4d.%01d ms) dDT=(%4d.%01d ms) DT=(%s)\n",
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
                printk("\n");
                printk("========================================\n");
                printk("Test #3: Chrono test\n\n");
                printk("----------------------------------------\n");
                SOPC_RealTime_GetTime (&rt1);
                dt1 = SOPC_Time_GetCurrentTimeUTC();
                pre = 5;
            }

            if (pre > 0)
            {
                printk("Starting in %d\n", pre);
            }
            else if (pre == 0)
            {
                printk(" !!! GO !!!\n");
                SOPC_RealTime_GetTime (&rt1);
                dt1 = SOPC_Time_GetCurrentTimeUTC();
            }
            SOPC_Sleep(1000);
            pre --;

            if (pre < -1)
            {
                dt2 = SOPC_Time_GetCurrentTimeUTC();
                SOPC_RealTime_GetTime (&rt2);

                const int deltaRts = (rt2.tick100ns - rt1.tick100ns) / 10000000;
                const int deltaDts = (dt2 - dt1) / 10000000;
                printk("dRT=(%6d s) dDT=(%6d s)\n",
                        deltaRts,
                        deltaDts);
            }
        }
        else
        {
            // No test
            if (newTest)
            {
                printk("\n");
                printk("No test running\n");
            }
            SOPC_Sleep(10);
        }
    }

    return NULL;
}

void main(void)
{
	init_app();

	static struct gptp_phase_dis_cb phase_dis;
	gptp_register_phase_dis_cb(&phase_dis, gptp_phase_dis_cb);

    console_getline_init();
    Thread thread = P_THREAD_Create(&test_thread, NULL, "demo", CONFIG_SOPC_THREAD_DEFAULT_PRIORITY, false);

    SOPC_ASSERT(thread != NULL);

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
