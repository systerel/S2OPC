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

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/

/**
 * \file This is the main program of the safe producer.
 */

/*============================================================================
 * INCLUDES
 *===========================================================================*/
#include <signal.h>
//#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include <assert.h>

#include "safetyDemo.h"

#include "sopc_threads.h"
#include "sopc_time.h"

#include "uas_logitf.h"
//
#include "uam.h"
#include "uam_s.h"
#include "uam_s2ns_itf.h"

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

typedef struct
{
    bool isProvider;
    volatile sig_atomic_t stopSignal;
    UAM_SpduRequestHandle spduRequestId;
    UAM_SpduResponseHandle spduResponseId;
} prod_s_interactive_Context;


/*============================================================================
 * LOCAL PROTOTYPES
 *===========================================================================*/
static void signal_stop_server(int sig);
static void prod_s_init(void);
static void prod_s_cycle(void);
static void prod_s_stop(void);

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
static SOPC_ReturnStatus g_status = SOPC_STATUS_OK;
static prod_s_interactive_Context gContext;
static Thread gCycleThread;

/*============================================================================
 * LOCAL FUNCTIONS
 *===========================================================================*/
static void signal_stop_server(int sig)
{
    (void) sig;

    if (gContext.stopSignal != 0)
    {
        exit(1);
    }
    else
    {
        gContext.stopSignal = 1;
    }
}

/**
 * ENVIRONNEMENT CONFIGURATION AND SETUP
 */

/*===========================================================================*/
static void prod_s_initialize_logs(void)
{
    if (SOPC_STATUS_OK == g_status)
    {
        // TODO : a special feature for SAFE log may have to be imagined
        // E.g: throwing logs over or any channel to Non safe, so that they can be
        // processed real-time by NonSafe
    }
}

/*===========================================================================*/
static void prod_s_initialize_uam(void)
{
    // TODO
}

/*===========================================================================*/
static void prod_s_init(void)
{
    if (g_status == SOPC_STATUS_OK)
    {
        prod_s_initialize_logs();

        prod_s_initialize_uam();
    }
}

/*===========================================================================*/
static void* prod_s_threadImpl (void* arg)
{
    assert (arg == NULL);
    LOG_Trace(LOG_DEBUG, "APP:Start Cycles");
    // Wait for a signal
    while (SOPC_STATUS_OK == g_status && 0 == gContext.stopSignal)
    {
        static const uint32_t msCycle = 50;
        prod_s_cycle();

        // Wait for next cycle
        SOPC_Sleep(msCycle);
    }
    LOG_Trace(LOG_DEBUG, "APP:End Cycles");
    return NULL;
}

/*===========================================================================*/
static void prod_s_stop(void)
{
    // TODO stop cleany everything

    UAM_S_Clear();
    printf("# EXITING (code =%02X)\n", g_status);
}

/*===========================================================================*/
static void prod_s_cycle(void)
{
    if (g_status == SOPC_STATUS_OK)
    {
        // TODO : replace UAM_NS_CheckSpduReception by an event-based reading rather than periodic polling

        // TODO  UAM_S_CheckSpduReception(SESSION_UAM_ID);
    }
}


/*===========================================================================*/
/*===========================================================================*/
// TODO : move that "interactive" part in a new  file
#include <sys/ioctl.h>
#include "sopc_raw_sockets.h"
#include <unistd.h>
#define STDIN 0
#define USER_ENTRY_MAXSIZE (128u)

/*===========================================================================*/
static bool prod_s_interactive_processCommand (const char* cmd)
{
    switch (cmd[0])
    {
        case 'q':
            gContext.stopSignal = 1;
            return true;
            break;
        default:
            break;
    }
    return false;
}

/*===========================================================================*/
static void prod_s_interactive_cycle(void)
{
    if (g_status == SOPC_STATUS_OK)
    {
        SOPC_SocketSet fdSet;
        int maxfd = STDIN;
        int result = 0;
        ssize_t nbRead = 0;

        struct timeval* ptv = NULL;
    #define WAIT_MS 10 * 1000
    #if WAIT_MS > 0
        struct timeval tv;
        tv.tv_sec = WAIT_MS / (1000 * 1000);
        tv.tv_usec = WAIT_MS % (1000 * 1000);
        ptv = &tv;
    #endif
        char entry[USER_ENTRY_MAXSIZE];

        SOPC_SocketSet_Clear(&fdSet);
        SOPC_SocketSet_Add(STDIN, &fdSet);

        result = select(maxfd + 1, &fdSet.set, NULL, NULL, ptv);
        if (result < 0)
        {
            printf("SELECT failed: %d\n", result);
            gContext.stopSignal = 1;
        }
        if (SOPC_SocketSet_IsPresent(STDIN, &fdSet))
        {
            nbRead = read(STDIN, entry, USER_ENTRY_MAXSIZE - 1);
            while (nbRead > 0 && entry[nbRead - 1] < ' ')
            {
                entry[nbRead - 1] = 0;
                nbRead--;
            }
            if (nbRead > 0)
            {
                prod_s_interactive_processCommand(entry);
            }
        }
    }
}


/*===========================================================================*/
int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    gContext.stopSignal = 0;

    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    prod_s_init();

    SOPC_Thread_Create(&gCycleThread, &prod_s_threadImpl, NULL, "prod_s_threadImpl");

    // Wait for a signal
    while (0 == gContext.stopSignal)
    {
        // Note : interactions with keyboard only work correctly in main thread.
        prod_s_interactive_cycle ();
        SOPC_Sleep(10);
    }

    // Clean and quit
    prod_s_stop();
    return 0;
}
