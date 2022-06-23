/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/**
 * \brief
 *  This file provides a small demo of PtP feature on ZEPHYR.
 *  - The current PTP status (internal/MASTER/SLAVE) is automatically shown when updated
 *  - The current clock correction detected using PtP is automatically shown when updated
 *  - Different tests may be launched manually using console to start some chronometer tests,
 *      or periodic events.
 *  - Periodic event tests send multicast messages which aims at providing easy TCPDUMP analysis of
 *      results. The Multicast address output is "232.1.3.4:12345"
 */

/** TODO :
 * - Show precision factor on PtP clock
 */

#include <logging/log.h>

#include <stdlib.h>
#include <stdio.h>
#include <zephyr.h>
#include <errno.h>
#include <console/console.h>
#include <sys/reboot.h>
#include <drivers/hwinfo.h>

#include "sopc_macros.h"
#include "sopc_mutexes.h"
#include "sopc_time.h"
#include "sopc_threads.h"
#include "sopc_assert.h"
#include "sopc_async_queue.h"
#include "sopc_mem_alloc.h"

#include "p_time.h"
#include "sopc_udp_sockets.h"

#include <net/net_core.h>
#include <net/gptp.h>
#include "ethernet/gptp/gptp_messages.h"
#include "ethernet/gptp/gptp_data_set.h"

#include "network_init.h"

#define MULTICAST_OUT_ADDR "232.1.3.4"
#define MULTICAST_OUT_PORT "12345"

#define SECOND_TO_100NS (10000000)
#define MS_TO_100NS (10000)
#define MS_TO_US (1000)
#define ONE_SECOND_MS (1000)

#ifndef CONFIG_NET_L2_ETHERNET
#error 'CONFIG_NET_L2_ETHERNET is required for this demo'
#endif

static int gSubTestId = 0;
static int gTestId = 0;
static int gParamI1 = 0;
static int gParamI2 = 0;
static Mutex gMutex;

static SOPC_Time_TimeSource gSource = SOPC_TIME_TIMESOURCE_INTERNAL;
static uint8_t gLastGmId [8] = {0}; //

static const char* sourceToString(const SOPC_Time_TimeSource source)
{
    switch (source) {
    case SOPC_TIME_TIMESOURCE_INTERNAL:   return "INTERNAL";
    case SOPC_TIME_TIMESOURCE_PTP_SLAVE:  return "PTP_SLAVE";
    case SOPC_TIME_TIMESOURCE_PTP_MASTER: return "PTP_MASTER";
        default: return "INVALID";
    }
}

typedef struct
{
    const uint8_t* serialNumber;
    const size_t serialNumberLen;
    const char*  ipAddr;
    const char*  deviceName;
} DeviceIdentifier;
static const DeviceIdentifier gDevices[]=
{
        // Note: Example to be adapted for each device/configuration!
        {"624248Q", 7, "192.168.42.111", "N.144/A"},
        {"652248Q", 7, "192.168.42.112", "N.144/B"},
};


/***************************************************/
/***************************************************/
static void synch_printf(const char* format, ...);
static const char* getDefaultIp(void);
static const DeviceIdentifier* getDevice(void);
static const char* DateToTimeString(const SOPC_DateTime dt);

/***************************************************/
static void gptp_phase_dis_cb(uint8_t *gm_identity,
                              uint16_t *time_base,
                              struct gptp_scaled_ns *last_gm_ph_change,
                              double *last_gm_freq_change)
{
    SOPC_UNUSED_ARG(time_base);
    SOPC_UNUSED_ARG(last_gm_ph_change);
    SOPC_UNUSED_ARG(last_gm_freq_change);

    SOPC_ASSERT(GPTP_CLOCK_ID_LEN == sizeof(gLastGmId));

    // Example : show CLOCK ID
    if (memcmp(gLastGmId, gm_identity, GPTP_CLOCK_ID_LEN) != 0)
    {
        memcpy(gLastGmId, gm_identity, GPTP_CLOCK_ID_LEN);
        synch_printf("[II] New GM clock : %02X%02X%02X:%02X%02X:%02X%02X%02X\n",
                gLastGmId[0], gLastGmId[1], gLastGmId[2], gLastGmId[3],
                gLastGmId[4], gLastGmId[5], gLastGmId[6], gLastGmId[7]);
    }

    /* Note:
     * Monitoring phase discontinuities is not used in current implementation
     * but may be used to correct/smooth time corrections
     * For example:
     */
    const uint64_t discontinuity = ((uint64_t)last_gm_ph_change->high) << 32 | last_gm_ph_change->low;
    if (discontinuity > 0)
    {
        synch_printf("[II] PTP time discontinuity: %u\n", (uint32_t) (discontinuity));
    }
}

/***************************************************/
static void net_send(void)
{
    static bool initialized = false;
    const SOPC_DateTime dt = SOPC_Time_GetCurrentTimeUTC();
    const char* dateStr = DateToTimeString(dt);

    const size_t bufLen = strlen(dateStr) + 1;
    const uint8_t* buffer_src = (const uint8_t*) dateStr;
    SOPC_ReturnStatus status;
    static SOPC_Socket_AddressInfo* addr = NULL;

    static Socket sock = SOPC_INVALID_SOCKET;
    static SOPC_Buffer* buffer= NULL;
    if (!initialized)
    {
        bool netInit = Network_Initialize(getDefaultIp());
        SOPC_ASSERT(netInit == true);

        addr = SOPC_UDP_SocketAddress_Create(false, MULTICAST_OUT_ADDR, MULTICAST_OUT_PORT);
        status = SOPC_UDP_Socket_CreateToSend (addr, NULL, 1, &sock);
        SOPC_ASSERT(status == SOPC_STATUS_OK);

        SOPC_UDP_Socket_Set_MulticastTTL(sock, 4);

        buffer = SOPC_Buffer_Create(bufLen);

        initialized = true;
    }
    SOPC_ASSERT(addr != NULL);
    SOPC_ASSERT(buffer != NULL);
    SOPC_ASSERT(sock >= 0);

    status = SOPC_Buffer_SetPosition(buffer, 0);
    SOPC_ASSERT(status == SOPC_STATUS_OK);
    status = SOPC_Buffer_Write(buffer, buffer_src, bufLen);
    status = SOPC_Buffer_SetPosition(buffer, 0);
    SOPC_ASSERT(status == SOPC_STATUS_OK);

    status = SOPC_UDP_Socket_SendTo(sock, addr, buffer);
    if(status != SOPC_STATUS_OK)
    {
        synch_printf("Failed to send net message (%d)\n", status);
    }
}

/***************************************************/
static void net_set(bool up)
{
    struct net_if* ptrNetIf = NULL;

    ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
    SOPC_ASSERT(NULL != ptrNetIf);
    int res = 0;

    if (up)
    {
        synch_printf("Starting interface (%s)\n", ptrNetIf->if_dev->dev->name);
        res = net_if_up(ptrNetIf);
    }
    else
    {
        synch_printf("Stopping interface (%s)\n", ptrNetIf->if_dev->dev->name);
        res = net_if_down(ptrNetIf);
    }
    if (res != 0)
    {
        synch_printf(".. FAILED (%d)!\n", res);
    }
}

static const char* DateToTimeString(const SOPC_DateTime dt)
{
    static char dateOnce [60];
    // dt / MS_TO_100NS = number of milliseconds
    // Avoid overflow by removing part greater than 1 day
    const uint32_t i_ms = (uint32_t) ((dt / MS_TO_100NS) % (1000 * 60 * 60 * 24));
    const uint32_t i_day = (uint32_t) ((dt / MS_TO_100NS) /(1000 * 60 * 60 * 24));

    const uint32_t i_s = (i_ms / MS_TO_US);
    const uint32_t i_min = (i_s / 60);
    const uint32_t i_hour = (i_min / 60);

    snprintf(dateOnce, sizeof(dateOnce), "Day %u, %02d h %02d m %02d s %03d ms",
            i_day,
            i_hour % 24,
            i_min % 60,
            i_s % 60,
            i_ms % MS_TO_US
            );
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

static void test_print_DateTime(void)
{
    char * datetime = SOPC_Time_GetStringOfCurrentTimeUTC(false);

    synch_printf("\nCurrent date/time : %s\n", datetime);

    SOPC_Free(datetime);
}

static void test_infos(void)
{
    test_print_DateTime();
    synch_printf("System info\n");
    synch_printf("- Time source is %s\n", sourceToString(gSource));
    if (gSource == SOPC_TIME_TIMESOURCE_PTP_SLAVE)
    {
        const int corrPrecent = -(int)(10000 * (SOPC_RealTime_GetClockCorrection() - 1.0));
        synch_printf("- PtP Time correction is %3d.%02d%%\n", corrPrecent /100, abs(corrPrecent %100));
    }
    synch_printf("- IP ADDR = %s\n", getDefaultIp());
    const DeviceIdentifier* dev = getDevice();
    if (NULL != dev)
    {
        synch_printf("- Device ID = %s\n", dev->deviceName);
        synch_printf("- Device SN = <%s>\n", dev->serialNumber);

        synch_printf("- GM Clock Id : %02X%02X%02X:%02X%02X:%02X%02X%02X\n",
                gLastGmId[0], gLastGmId[1], gLastGmId[2], gLastGmId[3],
                gLastGmId[4], gLastGmId[5], gLastGmId[6], gLastGmId[7]);
    }
}

static void test_help(void)
{
    synch_printf("\nZephyr PtP demo running\n\n");
    synch_printf("Enter command:\n");
    synch_printf(" - 'h' show help\n");
    synch_printf(" - 'i' show infos\n");
    synch_printf(" - 'd' print datetime\n");
    synch_printf(" - 's' to start 'SLEEP' test\n");
    synch_printf(" - 'o[msPeriod] [ms offset=25]' to start 'SLEEP_UNTIL/Offset' test\n");
    synch_printf(" - 'u' to start 'SLEEP_UNTIL' test\n");
    synch_printf(" - 'c' to start 'CHRONOMETER' test\n");
    synch_printf(" - 'r' to reboot\n");
    synch_printf(" - '+' to set ETH UP\n");
    synch_printf(" - '-' to set ETH DOWN\n");
    synch_printf(" - ' ' to stop current test\n");
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

    net_send(); // Setup socket;

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
        if (newSource != gSource)
        {
            gSource = newSource;
            synch_printf("[II] Time gSource changed to %s\n", sourceToString(gSource));
        }

        const int corrPrecent = -(int)(10000 * (SOPC_RealTime_GetClockCorrection() - 1.0));
        if (corrPrecent != previousCorrection)
        {
            previousCorrection = corrPrecent;
            synch_printf("[II] Time Correction changed to %3d.%02d%%\n", corrPrecent /100, abs(corrPrecent %100));
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
            net_send();

            const int deltaRt100us = (rt2.tick100ns - rt1.tick100ns) / MS_TO_US;
            const int deltaDt100us = (dt2 - dt1) / MS_TO_US;
            synch_printf("dRT=(%3d.%01d ms) dDT=(%3d.%01d ms)\n",
                    deltaRt100us /10, abs(deltaRt100us %10),
                    deltaDt100us /10, abs(deltaDt100us %10));
        }
        else if (testId == 2)
        {
            static SOPC_DateTime dtPrec = 0;
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
            net_send();

            dtPrec = dt2;
            const int deltaRt100us = (rt2.tick100ns - rt1.tick100ns) / MS_TO_US;
            const int deltaDt100us = (dtPrec - dt1) / MS_TO_US;
            synch_printf("dRT=(%4d.%01d ms) dDT=(%4d.%01d ms) DT=(%s)\n",
                    deltaRt100us /10, abs(deltaRt100us %10),
                    deltaDt100us /10, abs(deltaDt100us %10),
                    DateToTimeString(dt2));
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
            const uint64_t periodMs = gParamI1;
            const uint64_t offsetMs = gParamI2;
            if (newTest)
            {
                synch_printf("\n");
                synch_printf("========================================\n");
                synch_printf("Test #4: Requesting 'SleepUntil(Per=%ums/Off=%ums)'\n\n", (unsigned)periodMs, (unsigned)offsetMs);
                synch_printf("----------------------------------------\n");
            }
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            SOPC_RealTime_GetTime (&rt2);
            uint64_t rt2Prec = rt2.tick100ns;
            SOPC_RealTime_AddSynchedDuration(&rt2, periodMs * MS_TO_US, offsetMs * MS_TO_US);
            SOPC_RealTime_SleepUntil(&rt2);
            dt2 = SOPC_Time_GetCurrentTimeUTC();
            net_send();

            const unsigned dt = (rt2.tick100ns - rt2Prec) / 10;
            synch_printf("Event woken Drt=(%u us)", dt);
            synch_printf(" at DT=(%s)\n",DateToTimeString(dt2));
        }
        else
        {
            SOPC_Sleep(10);
        }
    }

    return NULL;
}

static const DeviceIdentifier* getDevice(void)
{
    static const DeviceIdentifier* result = NULL;
    if (NULL == result)
    {
        uint8_t buffer[16];
        const ssize_t len = hwinfo_get_device_id(buffer, sizeof(buffer) - 1);
        if (len > 0)
        {
            for (size_t i = 0 ; i < sizeof(gDevices)/sizeof(*gDevices) ; i++)
            {
                const DeviceIdentifier* dev = &gDevices[i];
                if (0 == memcmp(dev->serialNumber, buffer, dev->serialNumberLen))
                {
                    printf("Identified device '%s'\n", dev->deviceName);
                    result = dev;
                }
            }
        }
    }
    return result;
}

static const char* getDefaultIp(void)
{
    const DeviceIdentifier* dev = getDevice();
    if (dev != NULL)
    {
        return dev->ipAddr;
    }
    return NULL;
}

void main(void)
{
    printk("\nBUILD DATE : " __DATE__ " " __TIME__ "\n");
    SOPC_Sleep(100);

	static struct gptp_phase_dis_cb phase_dis;
	gptp_register_phase_dis_cb(&phase_dis, gptp_phase_dis_cb);

	SOPC_ReturnStatus result = SOPC_AsyncQueue_Init(&printQueue, "PRINT");
	SOPC_ASSERT(SOPC_STATUS_OK == result);

    console_getline_init();
    Thread thread = P_THREAD_Create(&test_thread, NULL, "demo", CONFIG_SOPC_THREAD_DEFAULT_PRIORITY, false);
    Thread threadPrint = P_THREAD_Create(&print_thread, NULL, "print", CONFIG_SOPC_THREAD_DEFAULT_PRIORITY, false);

    SOPC_ASSERT(thread != NULL);
    SOPC_ASSERT(threadPrint != NULL);

    test_help();

	for (;;)
	{
	    char * datetime = SOPC_Time_GetStringOfCurrentTimeUTC(false);
        synch_printf("\n[%s]>", datetime);
	    SOPC_Free(datetime);

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
        else if (line[0] == '+') // Sleep until test
        {
            net_set(1);
        }
        else if (line[0] == '-') // Sleep until test
        {
            net_set(0);
        }
        else if (line[0] == 'h')
        {
            test_help();
        }
        else if (line[0] == 'i')
        {
            test_infos();
        }
        else if (line[0] == 'o') // Sleep until test
        {
            gSubTestId  = 0;
            gTestId = 4;
            char* param2S = NULL;
            gParamI1 = 200;
            gParamI2 = 20;
            if (line[1] > 0)
            {
                gParamI1 = strtol(line + 1, &param2S, 10);
                if (param2S != NULL && param2S[0] != 0)
                {
                    gParamI2 = strtol(param2S, NULL, 10);
                }
            }
        }
        else if (line[0] == 'c') // Chronometer start
        {
            gSubTestId = 0;
            gTestId = 3;
        }
        else if (line[0] == 'd') // Date
        {
            test_print_DateTime();
        }
        else if (line[0] == 'r')
        {
            sys_reboot(SYS_REBOOT_COLD);
        }
        else
        {
            printk("\n[EE] Invalid command ; <%s>\n", line);
        }

        Mutex_Unlock(&gMutex);
	}
}
