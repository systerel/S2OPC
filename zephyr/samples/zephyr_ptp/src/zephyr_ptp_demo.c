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
 *  - The current PTP status (internal/MASTER/SLAVE) is automatically shown when updated.
 *  - The current clock correction detected using PtP is automatically shown when updated.
 *  - Different tests may be launched manually using console to start some time tests,
 *      or periodic events.
 *  - Periodic event tests send multicast messages which aims at providing easy TCPDUMP analysis of
 *      results. The multicast address output is "232.1.3.4:12345" and the data contains the local clock
 *      value as a string.
 */

#include <logging/log.h>

#include <console/console.h>
#include <drivers/hwinfo.h>
#include <errno.h>
#include <shell/shell.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <zephyr.h>

#include "sopc_assert.h"
#include "sopc_async_queue.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_version.h"

#include "sopc_udp_sockets.h"
#include "sopc_zephyr_time.h"

#include <net/net_core.h>

// Note: spaces required because the order is mandatory for dependancies
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

// use USE_ASYNCH_PRINTF to ensure that no print operation is executed during measurements tests.
#define USE_ASYNCH_PRINTF 1
#if USE_ASYNCH_PRINTF
static void synch_printf(const char* format, ...);
#else
#define synch_printf printf
#endif

static int gSubTestId = 0;
static int gTestId = 0;
static int gParamI1 = 0;
static int gParamI2 = 0;
static Mutex gMutex;

static SOPC_Time_TimeSource gSource = SOPC_TIME_TIMESOURCE_INTERNAL;
static uint8_t gLastGmId[8] = {0}; //

static const char* sourceToString(const SOPC_Time_TimeSource source)
{
    switch (source)
    {
    case SOPC_TIME_TIMESOURCE_INTERNAL:
        return "INTERNAL";
    case SOPC_TIME_TIMESOURCE_PTP_SLAVE:
        return "PTP_SLAVE";
    case SOPC_TIME_TIMESOURCE_PTP_MASTER:
        return "PTP_MASTER";
    default:
        return "INVALID";
    }
}

typedef struct
{
    const uint8_t* serialNumber;
    const char* ipAddr;
    const char* deviceName;
} DeviceIdentifier;

static const DeviceIdentifier gDevices[] = {
    // Note: Example to be adapted for each device/configuration!
    {"624248Q", "192.168.42.111", "N.144/A"},
    {"652248Q", "192.168.42.112", "N.144/B"},
    // Last element will match all devices (default)
    {"", "192.168.42.110", "Unknown device"},
};

/***************************************************/
/***************************************************/

static const char* getDefaultIp(void);
static const DeviceIdentifier* getDevice(void);
static const char* DateToTimeString(const SOPC_DateTime dt);

/***************************************************/
static void gptp_phase_dis_cb(uint8_t* gm_identity,
                              uint16_t* time_base,
                              struct gptp_scaled_ns* last_gm_ph_change,
                              double* last_gm_freq_change)
{
    SOPC_UNUSED_ARG(time_base);
    SOPC_UNUSED_ARG(last_gm_ph_change);
    SOPC_UNUSED_ARG(last_gm_freq_change);

    SOPC_ASSERT(GPTP_CLOCK_ID_LEN == sizeof(gLastGmId));

    // Example : show CLOCK ID
    if (memcmp(gLastGmId, gm_identity, GPTP_CLOCK_ID_LEN) != 0)
    {
        memcpy(gLastGmId, gm_identity, GPTP_CLOCK_ID_LEN);
        synch_printf("[II] New GM clock : %02X%02X%02X:%02X%02X:%02X%02X%02X\n", gLastGmId[0], gLastGmId[1],
                     gLastGmId[2], gLastGmId[3], gLastGmId[4], gLastGmId[5], gLastGmId[6], gLastGmId[7]);
    }

    /* Note:
     * Monitoring phase discontinuities is not used in current implementation
     * but may be used to correct/smooth time corrections
     * For example:
     */
    /*
    const uint64_t discontinuity = ((uint64_t)last_gm_ph_change->high) << 32 | last_gm_ph_change->low;
    if (discontinuity > 0)
    {
        synch_printf("[II] PTP time discontinuity: %u\n", (uint32_t) (discontinuity));
        ... take into account discontinuity ...
    }
    */
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
    static SOPC_Buffer* buffer = NULL;
    if (!initialized)
    {
        bool netInit = Network_Initialize(getDefaultIp());
        SOPC_ASSERT(netInit == true);

        addr = SOPC_UDP_SocketAddress_Create(false, MULTICAST_OUT_ADDR, MULTICAST_OUT_PORT);
        status = SOPC_UDP_Socket_CreateToSend(addr, NULL, 1, &sock);
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
    if (status != SOPC_STATUS_OK)
    {
        synch_printf("Failed to send net message (%d)\n", status);
    }
}

static const char* DateToTimeString(const SOPC_DateTime dt)
{
    static char dateOnce[60];
    // dt / MS_TO_100NS = number of milliseconds
    // Avoid overflow by removing part greater than 1 day
    const uint32_t i_ms = (uint32_t)((dt / MS_TO_100NS) % (1000 * 60 * 60 * 24));
    const uint32_t i_day = (uint32_t)((dt / MS_TO_100NS) / (1000 * 60 * 60 * 24));

    const uint32_t i_s = (i_ms / MS_TO_US);
    const uint32_t i_min = (i_s / 60);
    const uint32_t i_hour = (i_min / 60);

    snprintf(dateOnce, sizeof(dateOnce), "Day %u, %02d h %02d m %02d s %03d ms", i_day, i_hour % 24, i_min % 60,
             i_s % 60, i_ms % MS_TO_US);
    return dateOnce;
}

#if USE_ASYNCH_PRINTF
static SOPC_AsyncQueue* printQueue;

static void* print_thread(void* context)
{
    SOPC_UNUSED_ARG(context);
    for (;;)
    {
        char* element;
        SOPC_ReturnStatus result = SOPC_AsyncQueue_BlockingDequeue(printQueue, (void**) &element);
        SOPC_ASSERT(result == SOPC_STATUS_OK);
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
    SOPC_AsyncQueue_BlockingEnqueue(printQueue, (void**) buffer);
}
#endif

static void test_print_DateTime(void)
{
    char* datetime = SOPC_Time_GetStringOfCurrentTimeUTC(false);

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
        const int corrPrecent = -(int) (10000 * (SOPC_RealTime_GetClockCorrection() - 1.0));
        synch_printf("- PtP Time correction is %3d.%02d%%\n", corrPrecent / 100, abs(corrPrecent % 100));
    }
    const int clockPrec = (int) (10000 * (SOPC_RealTime_GetClockPrecision()));
    synch_printf("- PtP Time precision is %3d.%02d%%\n", clockPrec / 100, abs(clockPrec % 100));

    const char* const ip = getDefaultIp();
    synch_printf("- IP ADDR = %s\n", (ip != NULL ? ip : "<NULL>"));
    const DeviceIdentifier* dev = getDevice();
    if (NULL != dev)
    {
        synch_printf("- Device ID = %s\n", dev->deviceName);
        synch_printf("- Device SN = <%s>\n", dev->serialNumber);

        synch_printf("- GM Clock Id : %02X%02X%02X:%02X%02X:%02X%02X%02X\n", gLastGmId[0], gLastGmId[1], gLastGmId[2],
                     gLastGmId[3], gLastGmId[4], gLastGmId[5], gLastGmId[6], gLastGmId[7]);
    }
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

    SOPC_RealTime_GetTime(&t0);

    for (;;)
    {
        // Test management
        Mutex_Lock(&gMutex);
        const bool newTest = (gSubTestId == 0);
        gSubTestId++;
        const int testId = gTestId;
        Mutex_Unlock(&gMutex);

        const SOPC_Time_TimeSource newSource = SOPC_Time_GetTimeSource();
        if (newSource != gSource)
        {
            gSource = newSource;
            synch_printf("[II] Time gSource changed to %s\n", sourceToString(gSource));
        }

        const int corrPrecent = -(int) (10000 * (SOPC_RealTime_GetClockCorrection() - 1.0));
        if (corrPrecent != previousCorrection)
        {
            previousCorrection = corrPrecent;
            synch_printf("[II] Time Correction changed to %3d.%02d%%\n", corrPrecent / 100, abs(corrPrecent % 100));
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
            SOPC_RealTime_GetTime(&rt1);
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            SOPC_Sleep(ONE_SECOND_MS);
            SOPC_RealTime_GetTime(&rt2);
            dt2 = SOPC_Time_GetCurrentTimeUTC();
            net_send();

            const int deltaRt100us = (rt2.tick100ns - rt1.tick100ns) / MS_TO_US;
            const int deltaDt100us = (dt2 - dt1) / MS_TO_US;
            synch_printf("dRT=(%3d.%01d ms) dDT=(%3d.%01d ms)\n", deltaRt100us / 10, abs(deltaRt100us % 10),
                         deltaDt100us / 10, abs(deltaDt100us % 10));
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
            SOPC_RealTime_GetTime(&rt1);
            SOPC_RealTime_GetTime(&rt2);
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            SOPC_RealTime_AddSynchedDuration(&rt2, ONE_SECOND_MS * MS_TO_US, 0);
            SOPC_RealTime_SleepUntil(&rt2);
            dt2 = SOPC_Time_GetCurrentTimeUTC();
            net_send();

            dtPrec = dt2;
            const int deltaRt100us = (rt2.tick100ns - rt1.tick100ns) / MS_TO_US;
            const int deltaDt100us = (dtPrec - dt1) / MS_TO_US;
            synch_printf("dRT=(%4d.%01d ms) dDT=(%4d.%01d ms) DT=(%s)\n", deltaRt100us / 10, abs(deltaRt100us % 10),
                         deltaDt100us / 10, abs(deltaDt100us % 10), DateToTimeString(dt2));
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
                SOPC_RealTime_GetTime(&rt1);
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
                SOPC_RealTime_GetTime(&rt1);
                dt1 = SOPC_Time_GetCurrentTimeUTC();
            }
            SOPC_Sleep(ONE_SECOND_MS);
            pre--;

            if (pre < -1)
            {
                dt2 = SOPC_Time_GetCurrentTimeUTC();
                SOPC_RealTime_GetTime(&rt2);

                const int deltaRts = (rt2.tick100ns - rt1.tick100ns) / SECOND_TO_100NS;
                const int deltaDts = (dt2 - dt1) / SECOND_TO_100NS;
                synch_printf("dRT=(%6d s) dDT=(%6d s)\n", deltaRts, deltaDts);
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
                synch_printf("Test #4: Requesting 'SleepUntil(Per=%ums/Off=%ums)'\n\n", (unsigned) periodMs,
                             (unsigned) offsetMs);
                synch_printf("----------------------------------------\n");
            }
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            SOPC_RealTime_GetTime(&rt2);
            uint64_t rt2Prec = rt2.tick100ns;
            SOPC_RealTime_AddSynchedDuration(&rt2, periodMs * MS_TO_US, offsetMs * MS_TO_US);
            SOPC_RealTime_SleepUntil(&rt2);
            dt2 = SOPC_Time_GetCurrentTimeUTC();
            net_send();

            const unsigned dt = (rt2.tick100ns - rt2Prec) / 10;
            synch_printf("Event woken Drt=(%u us)", dt);
            synch_printf(" at DT=(%s)\n", DateToTimeString(dt2));
        }
        else if (testId == 5)
        {
            static const unsigned int thresholdPer10000 = 9980;
            if (newTest)
            {
                synch_printf("\n");
                synch_printf("========================================\n");
                synch_printf("Test #5: WAITING FOR PTP SYNCHRO, thr = %03d.%02u%%'\n\n", thresholdPer10000 / 100,
                             thresholdPer10000 % 100);
                synch_printf("----------------------------------------\n");
            }

            // Reading SOPC_Time_GetCurrentTimeUTC forces PtP clock synchronization protocol.
            // See details in "opc_zephyr_time.h"
            dt1 = SOPC_Time_GetCurrentTimeUTC();
            (void) dt1;
            SOPC_Sleep(1000);

            const int clockPrec = (int) (10000 * (SOPC_RealTime_GetClockPrecision()));
            const int corrPrecent = -(int) (10000 * (SOPC_RealTime_GetClockCorrection() - 1.0));
            synch_printf("- PtP Time correction is %3d.%02d%%\n", corrPrecent / 100, abs(corrPrecent % 100));
            synch_printf("- PtP Time precision is %3d.%02d%%\n", clockPrec / 100, abs(clockPrec % 100));

            if (clockPrec >= thresholdPer10000)
            {
                synch_printf("DONE!\n");
                gSubTestId = 0;
                gTestId = 0;
            }
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
            for (size_t i = 0; result == NULL && i < sizeof(gDevices) / sizeof(*gDevices); i++)
            {
                const DeviceIdentifier* dev = &gDevices[i];
                SOPC_ASSERT(NULL != dev->serialNumber);
                size_t len = strlen(dev->serialNumber);
                if (len > sizeof(buffer))
                {
                    len = sizeof(buffer);
                }
                if (0 == memcmp(dev->serialNumber, buffer, len))
                {
                    printf("Identified device '%s'\n", dev->deviceName);
                    result = dev;
                }
            }
        }
        if (NULL == result)
        {
            printf("Unidentified device : [");
            for (size_t i = 0; i < len; i++)
            {
                const uint8_t c = buffer[i];
                if (c == '\\')
                {
                    printf("\\");
                }
                if (c >= 0x20 && c < 0x7F)
                {
                    // Displayable char
                    printf("%c", (char) c);
                }
                else
                {
                    printf("\\x%02X", (int) c);
                }
            }
            printf("]\n");
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

    Thread thread = P_THREAD_Create(&test_thread, NULL, "demo", CONFIG_SOPC_THREAD_DEFAULT_PRIORITY, false);
    SOPC_ASSERT(thread != NULL);

#if USE_ASYNCH_PRINTF
    SOPC_ReturnStatus result = SOPC_AsyncQueue_Init(&printQueue, "PRINT");
    SOPC_ASSERT(SOPC_STATUS_OK == result);

    Thread threadPrint = P_THREAD_Create(&print_thread, NULL, "print", CONFIG_SOPC_THREAD_DEFAULT_PRIORITY, false);
    SOPC_ASSERT(threadPrint != NULL);
#endif
}

/***************************************************/
static int cmd_demo_info(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    synch_printf("\nZephyr S2OPC PTP demo\n");
    synch_printf("Server toolkit version: %s\n", SOPC_TOOLKIT_VERSION);
    test_infos();
    return 0;
}

/***************************************************/
static int cmd_demo_sleep(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    gSubTestId = 0;
    gTestId = 1;
    return 0;
}

/***************************************************/
static int cmd_demo_stop(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    gSubTestId = 0;
    gTestId = 0;
    return 0;
}

/***************************************************/
static int cmd_demo_sleep_until(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    gSubTestId = 0;
    gTestId = 2;
    return 0;
}

/***************************************************/
static int cmd_demo_wait(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    gSubTestId = 0;
    gTestId = 5;
    return 0;
}

/***************************************************/
static int cmd_demo_chrono(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    gSubTestId = 0;
    gTestId = 3;
    return 0;
}

/***************************************************/
static int cmd_demo_date(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    test_print_DateTime();
    return 0;
}

/***************************************************/
static int cmd_demo_reboot(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    sys_reboot(SYS_REBOOT_COLD);
    return 0;
}

/***************************************************/
static int cmd_demo_offset(const struct shell* shell, size_t argc, char** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    const char* param1 = (argc < 2 ? "200" : argv[1]);
    const char* param2 = (argc < 3 ? "20" : argv[2]);

    gSubTestId = 0;
    gTestId = 4;
    gParamI1 = strtol(param1, NULL, 10);
    gParamI2 = strtol(param2, NULL, 10);

    return 0;
}

/* Creating subcommands (level 1 command) array for command "demo". */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_demo,
                               SHELL_CMD(info, NULL, "Show demo info", cmd_demo_info),
                               SHELL_CMD(stop, NULL, "Stop current test", cmd_demo_stop),
                               SHELL_CMD(sleep, NULL, "Sleep test", cmd_demo_sleep),
                               SHELL_CMD(until, NULL, "Sleep until test", cmd_demo_sleep_until),
                               SHELL_CMD(wait, NULL, "Wait for TPP synchro", cmd_demo_wait),
                               SHELL_CMD(offset, NULL, "Time offset test", cmd_demo_offset),
                               SHELL_CMD(chrono, NULL, "Chronometer test", cmd_demo_chrono),
                               SHELL_CMD(date, NULL, "Print date", cmd_demo_date),
                               SHELL_CMD(reboot, NULL, "Reboot target", cmd_demo_reboot),
                               SHELL_SUBCMD_SET_END);

/* Creating root (level 0) command "demo" */
SHELL_CMD_REGISTER(demo, &sub_demo, "Demo commands", NULL);
