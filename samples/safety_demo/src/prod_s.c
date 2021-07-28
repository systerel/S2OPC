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
 * \file This is the main program of the safe producer. This will probably be a single-threaded application
 * \usage
 *  optional parameter: v<X> to set verbose level of UAM/APP modules to X (0 = No logs  to 5 = verbooooose)
 *  optional parameter: V<X> to set verbose level of UAS module to X (0 = No logs  to 5 = verbooooose)
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

#include "safetyDemo.h"
#include "config.h"

#include "uas_logitf.h"
#include "uam_s_libs.h"
//
#include "uam.h"
#include "uam_s.h"
#include "uam_s2ns_itf.h"


/* ============================================================================
   ============================================================================
   ============================================================================
   ============================================================================ */
/** Note:  this section is aimed to be auto-generated, so that it will be more maintainable
 * to create a template file for each connection (PROD or CONS)
 */

static UAM_S_ProviderHandle hProviderHandle = UAM_NoHandle;
static const UAM_SafetyConfiguration_type yInstanceConfigSample1 =
{
        .dwSessionId = SAMPLE1_SESSION_UAM_ID,
        .wSafetyDataLength = SAMPLE1_SAFETY_DATA_LEN,
        .wNonSafetyDataLength = SAMPLE1_UNSAFE_DATA_LEN
};

static const UAS_SafetyProviderSPI_type SPI1P_Sample = {.dwSafetyProviderId = SAMPLE_PROVID1_ID,
                                                        .zSafetyBaseId = SAMPLE_PROVID1_GUID,
                                                        .dwSafetyStructureSignature = SAMPLE_PROVID1_SIGN};
static UAS_Bool fProvider1SampleCycle(const UAM_SafetyConfiguration_type* const pzConfiguration,
                                  const UAM_S_ProviderSAPI_Input* pzAppInputs,
                                  UAM_S_ProviderSAPI_Output* pzAppOutputs)
{
    UAM_S_LIBS_ASSERT(pzConfiguration != NULL);
    UAM_S_LIBS_ASSERT(pzAppInputs != NULL);
    UAM_S_LIBS_ASSERT(pzAppOutputs != NULL);

    // Do some simulation stuff...
    static UAS_UInt32 u32Cycle = 0;
    static char sSafetyDataText10[10] = {'0','1','2','3','4','5','6','7','8','9'};
    static UAS_UInt32 uDemoCounter = 0;
    static SafetyDemo_Sample_Safe1_type safeData = {0, 0, 0, 0, 0, 0, {0}};
    uDemoCounter++;
    u32Cycle ++ ;

    UAM_S_DoLog_UHex32(UAM_S_LOG_INFO, "# Execution of PROVIDER1 APP, cycle = ", u32Cycle);
    if (safeData.u8Val1 < 100)
    {
        safeData.u8Val1++;
    }
    else
    {
        safeData.u8Val1 = 10;
    }
    safeData.bData1 = 0;
    safeData.bData2 = 1;
    safeData.u8Val2 = 12;

    UAM_S_LIBS_MemCopy(safeData.sText10, sSafetyDataText10, 10);

    // Set output flags
    pzAppOutputs->bOperatorAckProvider = 0;

    // Set Output Non Safe data
    UAM_S_LIBS_ASSERT(pzAppOutputs->pbySerializedNonSafetyData != NULL);
    UAM_S_LIBS_MemZero(pzAppOutputs->pbySerializedNonSafetyData, pzConfiguration->wNonSafetyDataLength);
    snprintf((char*) pzAppOutputs->pbySerializedNonSafetyData, pzConfiguration->wSafetyDataLength, "NonSafety Data %u",
             pzAppInputs->dwMonitoringNumber);

    // Set Output Safe data
    UAM_S_LIBS_ASSERT(pzAppOutputs->pbySerializedSafetyData != NULL);
    UAM_S_LIBS_ASSERT(sizeof(safeData) == pzConfiguration->wSafetyDataLength);
    UAM_S_LIBS_MemCopy(pzAppOutputs->pbySerializedSafetyData, (void*) &safeData, sizeof(safeData));

    return 1;
}


/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

typedef struct
{
    bool isProvider;
    volatile sig_atomic_t stopSignal;
} prod_s_interactive_Context;


/*============================================================================
 * LOCAL PROTOTYPES
 *===========================================================================*/
static void signal_stop_server(int sig);
static void prod_s_init(int argc, char* argv[]);
static void prod_s_cycle(void);
static void prod_s_stop(void);

/*============================================================================
 * LOCAL VARIABLES
 *===========================================================================*/
static SOPC_ReturnStatus g_status = SOPC_STATUS_OK;
static prod_s_interactive_Context gContext;

/*============================================================================
 * LOCAL FUNCTIONS
 *===========================================================================*/
static void signal_stop_server(int sig)
{
    (void) sig;
    UAM_S_DoLog_Int (UAM_S_LOG_DEFAULT, "Trapped signal :", sig);
    if (gContext.stopSignal != 0)
    {
        exit(1);
    }
    else
    {
        gContext.stopSignal = 1;
    }
}



/*===========================================================================*/
/*===========================================================================*/
// TODO : move that "interactive" part in a new  file ?
#include <sys/ioctl.h>
#include "sopc_raw_sockets.h" // TODO: remove dependency to S2OPC!
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
            UAM_S_DoLog(UAM_S_LOG_ERROR, "SELECT failed");
            gContext.stopSignal = 1;
        }
        else if (SOPC_SocketSet_IsPresent(STDIN, &fdSet))
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
/**
 * ENVIRONNEMENT CONFIGURATION AND SETUP
 */

/*===========================================================================*/
static void prod_s_initialize_uam(UAM_S_LOG_LEVEL initLogLevel)
{
    UAS_UInt8 uResult;
    UAM_S_Initialize(initLogLevel);

    uResult = UAM_S_InitSafetyProvider (&yInstanceConfigSample1, &SPI1P_Sample, &fProvider1SampleCycle, &hProviderHandle);
    if (UAS_OK == uResult)
    {
        UAM_S_DoLog_UHex32(UAM_S_LOG_INFO, "UAM_S_InitSafetyProvider Succeeded for HDL : ", hProviderHandle);

    }
    else
        {
        UAM_S_DoLog_UHex32(UAM_S_LOG_ERROR, "Fatal error: UAM_S_InitSafetyProvider failed with code : ", uResult);
        signal_stop_server(0);
    }

    // Note: That would be the place to call UAM_S_InitSafetyConsumer .

    if (UAS_OK == uResult)
    {
        uResult = UAM_S_StartSafety();
        if (UAS_OK == uResult)
        {
            UAM_S_DoLog_UHex32(UAM_S_LOG_INFO, "UAM_S_StartSafety Succeeded for HDL : ", hProviderHandle);

        }
        else
            {
            UAM_S_DoLog_UHex32(UAM_S_LOG_ERROR, "Fatal error: UAM_S_StartSafety failed with code : ", uResult);
            signal_stop_server(0);
        }
    }
}

/*===========================================================================*/
static void prod_s_init(int argc, char* argv[])
{
    UAM_S_LOG_LEVEL initLogLevel = UAM_S_LOG_WARN;
    LOG_LEVEL uasLogLevel = LOG_WARN;
    for (int argp = 1 ; argp < argc; argp++)
    {
        const char c = argv[argp][0];
        if (c == 'v')
        {
            initLogLevel = (UAM_S_LOG_LEVEL) atoi(&argv[argp][1]);
        }
        else if (c == 'V')
        {
            uasLogLevel = (LOG_LEVEL) atoi(&argv[argp][1]);
        }
        else
        {
            UAM_S_DoLog(UAM_S_LOG_DEFAULT, "Usage: ./prod_s [vX]");
            UAM_S_DoLog(UAM_S_LOG_DEFAULT, "Options:");
            UAM_S_DoLog(UAM_S_LOG_DEFAULT, "  optional parameter: v<X> to set verbose level of UAM/APP modules to X (0 = No logs  to 5 = verbooooose)");
            UAM_S_DoLog(UAM_S_LOG_DEFAULT, "  optional parameter: V<X> to set verbose level of UAS module to X (0 = No logs  to 5 = verbooooose)");
        }
    }

    if (g_status == SOPC_STATUS_OK)
    {
        prod_s_initialize_uam(initLogLevel);
        LOG_SetLevel((LOG_LEVEL)uasLogLevel);
    }
}

/*===========================================================================*/
static void prod_s_stop(void)
{
    // TODO stop cleany everything. USE LIBASAN ?

    UAM_S_Clear();
    UAM_S_DoLog_UHex32(UAM_S_LOG_INFO, "# EXITING ; code = ", g_status);
}

/*===========================================================================*/
static void prod_s_cycle(void)
{
    UAS_UInt8 uResult;
    if (g_status == SOPC_STATUS_OK)
    {
        uResult = UAM_S_Cycle ();

        if (UAS_OK != uResult)
        {
            UAM_S_DoLog_UHex32(UAM_S_LOG_ERROR, "Fatal error: UAM_S_Cycle failed with code : ", uResult);
            gContext.stopSignal = 1;
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

    prod_s_init(argc, argv);

    UAM_S_DoLog(UAM_S_LOG_DEFAULT, "End of Initialization. Starting Cyclic treatment");
    // Wait for a signal
    while (0 == gContext.stopSignal)
    {
        // Note : interactions with keyboard only work correctly in main thread.
        prod_s_interactive_cycle ();

        prod_s_cycle();
        usleep(1000 * USER_APP_CYCLE_DURATION_MS);
    }

    // Clean and quit
    prod_s_stop();
    return 0;
}
