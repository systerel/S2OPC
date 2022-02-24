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

/** \file
 *
 * \brief This real-time pubsub sample shows how to interface the PubSub callbacks with a simplistic Cache.
 *
 * \note This example is only available on Linux for now.
 *
 * The Cache is a simple dictionary (see cache.h).
 * The sample follows the required step to use the PubSub library:
 * - initialize S2OPC and logs,
 * - configure the PubSub using an XML file,
 * - setup the PubSub callbacks (GetSourceVariables, SetTargetVariables),
 * - setup some very simple encryption/signature,
 * - start the PubSub,
 * - stop and cleanup.
 *
 * In its main loop (main thread), it only waits indefinitely until a SIGINT or SIGTERM is sent.
 * The work is done by the callback which are called by another thread by the S2OPC library.
 *
 * This sample is used to measure round trip times of PubSub messages.
 * The same executable should be executed on two different machines:
 * - the emitter publishes an integer that is incremented by one each cycle,
 * - the receiver subscribes and receives the values over time,
 * - the receiver is also a publisher and sends back the last value it received,
 * - the emitter is also a subscriber and receives the value that was sent back by the receiver.
 *
 * The emitter then computes round trip times and print the results on stdout or save them to a CSV file.
 *
 * By default, the program starts the Publisher thread with a high scheduling priority, which requires admin privileges.
 * It is possible to change this by modifying the priority in the call to SOPC_PubScheduler_Start
 *
 * Different environment variables control the behavior of the program.
 * These values all have a default value (see config.h):
 * - IS_LOOPBACK: set to 0 for the emitter and 1 for the receiver,
 * - LOG_PATH should point to the folder in which to save logs,
 * - RTT_SAMPLES: the number of round trip time samples to save,
 * - PUBSUB_XML_CONFIG: path to the pubsub XML configuration,
 *   usually forked from samples/PubSub/data/ XMLs,
 * - CSV_PREFIX: when set, enables CSV printing instead of stdout printing,
 *   the saved CSV will be named $CSV_PREFIX-YYYYMMDD-HHMMSS.csv according to the current date.
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* strerror */
#include <time.h>   /* Timestamped csv */

#include "p_time.h" /* SOPC_RealTime API, for now linux only */
#include "sopc_common.h"
#include "sopc_common_build_info.h"
#include "sopc_helper_string.h"
#include "sopc_log_manager.h"
#include "sopc_mem_alloc.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_sub_scheduler.h"
#include "sopc_time.h"
#include "sopc_xml_loader.h"

#include "cache.h"
#include "config.h"

#define SERVER_CMD_PUBSUB_PERIOD_MS     "ns=1;s=PubPeriodMs"
#define SERVER_STATUS_SUB_PERIOD_MS     "ns=1;s=SubPeriodMs"
#define SERVER_CMD_PUBSUB_SIGN_ENCRYPT  "ns=1;s=PubSignAndEncypt"
#define SERVER_STATUS_PUB_VAR_RESULT    "ns=1;s=PubVarResult"
#define SERVER_STATUS_SUB_VAR_LOOP      "ns=1;s=SubVarLoops"
#define SERVER_STATUS_PUB_VAR_LOOP      "ns=1;s=PubVarLoops"
#define SERVER_STATUS_PUB_VAR_INFO      "ns=1;s=PubVarInfo"
#define SERVER_STATUS_PUB_VAR_UPTIME    "ns=1;s=PubVarUpTime"


volatile sig_atomic_t stopSignal = 0;

static void doPubSubTest(void);
static void onSubVarLoopRcv(const SOPC_NodeId* nid, SOPC_DataValue* pDv);

static SOPC_NodeId* pNidSubVarPeriod = NULL;
static SOPC_NodeId* pNidPubVarPeriod = NULL;
static SOPC_NodeId* pNidSubVarLoop = NULL;
static SOPC_NodeId* pNidPubVarLoop = NULL;
static SOPC_NodeId* pNidPubVarInfo = NULL;
static SOPC_NodeId* pNidPubVarResult = NULL;
static uint32_t     gTestPeriod  = 0;
static char         gTestStatus [256];

typedef struct
{
    uint64_t sumUs;
    uint64_t sumDiffSquare;
    uint64_t maxDiff;
    uint32_t nbSamples;
} PubStats;
static PubStats gPubStats;


static void signal_stop_server(int sig)
{
    (void) sig;

    if (stopSignal != 0)
    {
        exit(1);
    }
    else
    {
        stopSignal = 1;
    }
}

static void setTestStatus(const char* format, ...)
{
    if (format == NULL)
    {
        return;
    }
    memset(gTestStatus,0, sizeof(gTestStatus));
    va_list argptr;
    va_start(argptr, format);
    vsnprintf(gTestStatus, sizeof(gTestStatus) - 1, format, argptr);
    va_end(argptr);
    printf(" ** NEW STATUS : %s\n", gTestStatus);
}

static const char* getenv_default(const char* name, const char* default_value)
{
    const char* val = getenv(name);

    return (val != NULL) ? val : default_value;
}

static void userDoLog(const char* category, const char* const line)
{
    printf("%s : %s\n", category, line);
}

int main(int argc, char* const argv[])
{
    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    /* Parse command line arguments ? */
    (void) argc;
    (void) argv;
    setTestStatus("Starting...");

    pNidSubVarLoop = SOPC_NodeId_FromCString(SERVER_STATUS_SUB_VAR_LOOP,strlen(SERVER_STATUS_SUB_VAR_LOOP));
    pNidSubVarPeriod = SOPC_NodeId_FromCString(SERVER_STATUS_SUB_PERIOD_MS,strlen(SERVER_STATUS_SUB_PERIOD_MS));
    pNidPubVarPeriod = SOPC_NodeId_FromCString(SERVER_CMD_PUBSUB_PERIOD_MS,strlen(SERVER_CMD_PUBSUB_PERIOD_MS));
    pNidPubVarLoop = SOPC_NodeId_FromCString(SERVER_STATUS_PUB_VAR_LOOP,strlen(SERVER_STATUS_PUB_VAR_LOOP));
    pNidPubVarInfo = SOPC_NodeId_FromCString(SERVER_STATUS_PUB_VAR_INFO,strlen(SERVER_STATUS_PUB_VAR_INFO));
    pNidPubVarResult = SOPC_NodeId_FromCString(SERVER_STATUS_PUB_VAR_RESULT,strlen(SERVER_STATUS_PUB_VAR_RESULT));

    /* General initializations */
    const char* log_path = getenv_default("LOG_PATH", LOG_PATH);
    printf("LOG_PATH: %s\n", log_path);
    SOPC_Log_Configuration logConfiguration ;
    logConfiguration.logSystem = SOPC_LOG_SYSTEM_USER;
    logConfiguration.logSysConfig.userSystemLogConfig.doLog = userDoLog;
    logConfiguration.logLevel = SOPC_LOG_LEVEL_INFO;
    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);

    if (SOPC_STATUS_OK != status)
    {
        printf("Error while initializing logs\n");
    }

    /* Configuration of the PubSub */
    SOPC_PubSubConfiguration* config = NULL;
    if (SOPC_STATUS_OK == status)
    {
        const char* config_path = PUBSUB_XML_CONFIG_ZEPHYR_PERF;
        printf("PUBSUB_XML_CONFIG: %s\n", config_path);

        FILE* fd = fopen(config_path, "r");
        config = SOPC_PubSubConfig_ParseXML(fd);
        int closed = fclose(fd);

        status = (0 == closed && NULL != config) ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_PARAMETERS;
        if (SOPC_STATUS_OK != status)
        {
            printf("Error while loading PubSub configuration from %s\n", config_path);
        }
    }

    SOPC_PubSourceVariableConfig* sourceConfig = NULL;
    SOPC_SubTargetVariableConfig* targetConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        sourceConfig = SOPC_PubSourceVariableConfig_Create(&Cache_GetSourceVariables);
        if (NULL == sourceConfig)
        {
            printf("Error while setting Pub source variable\n");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        targetConfig = SOPC_SubTargetVariableConfig_Create(&Cache_SetTargetVariables);
        if (NULL == targetConfig)
        {
            printf("Error while setting Sub target variable\n");
            status = SOPC_STATUS_NOK;
        }
    }
    /* Don't forget the SKS */
    const char* signing_key = getenv_default("SKS_SIGNING_KEY", SKS_SIGNING_KEY);
    const char* encryption_key = getenv_default("SKS_ENCRYPTION_KEY", SKS_ENCRYPTION_KEY);
    const char* nonce = getenv_default("SKS_KEY_NONCE", SKS_KEY_NONCE);
    printf("SKS_SIGNING_KEY: %s\n", signing_key);
    printf("SKS_ENCRYPTION_KEY: %s\n", encryption_key);
    printf("SKS_KEY_NONCE: %s\n", nonce);
    SOPC_LocalSKS_init(signing_key, encryption_key, nonce);

    /* Initialize the Cache with the PubSub configuration */
    if (SOPC_STATUS_OK == status)
    {
        bool res = Cache_Initialize(config);
        if (!res)
        {
            printf("Error while initializing the cache, refer to log files\n");
            status = SOPC_STATUS_NOK;
        }
        else
        {
            Cache_SetTargetVarListener(onSubVarLoopRcv);
        }
    }

    /* Start PubSub */
    if (SOPC_STATUS_OK == status)
    {
        const uint32_t nbPub = SOPC_PubSubConfiguration_Nb_PubConnection(config);
        if (0 == nbPub)
        {
            printf("# Info: No Publisher configured\n");
        }
        else
        {
            bool res = SOPC_PubScheduler_Start(config, sourceConfig, 99);
            if (res)
            {
                printf("# Info: Publisher started\n");
            }
            else
            {
                printf("# Error while starting the Publisher, do you have administrator privileges?\n");
                status = SOPC_STATUS_NOK;
            }
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        const uint32_t nbSub = SOPC_PubSubConfiguration_Nb_SubConnection(config);
        if (0 == nbSub)
        {
            printf("# Info: No Subscriber configured\n");
        }
        else
        {
            /* Note: a state changed callback can be made to handle PubSub state changes (unused here) */
            bool res = SOPC_SubScheduler_Start(config, targetConfig, NULL, 99);
            if (res)
            {
                printf("# Info: Subscriber started\n");
            }
            else
            {
                printf("# Error while starting the Subscriber\n");
                status = SOPC_STATUS_NOK;
            }
        }
    }

    /* Wait for a signal */
    while (SOPC_STATUS_OK == status && 0 == stopSignal)
    {
        SOPC_Sleep(SLEEP_TIMEOUT);
        doPubSubTest();

    }

    /* Clean and quit */
    SOPC_NodeId_Clear(pNidSubVarLoop);
    SOPC_NodeId_Clear(pNidPubVarLoop);
    SOPC_NodeId_Clear(pNidPubVarInfo);
    SOPC_NodeId_Clear(pNidPubVarResult);
    SOPC_NodeId_Clear(pNidPubVarPeriod);
    SOPC_NodeId_Clear(pNidSubVarPeriod);
    SOPC_PubScheduler_Stop();
    SOPC_SubScheduler_Stop();
    Cache_Clear();
    SOPC_PubSourceVariableConfig_Delete(sourceConfig);
    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    SOPC_PubSubConfiguration_Delete(config);
    printf("# Info: PubSub stopped\n");

}

/* TODO: implement this in a platform independent fashion */
static inline long diff_timespec(struct timespec* a, struct timespec* b)
{
    if (!a || !b)
    {
        return LONG_MAX;
    }

    bool invert = false;
    if (b->tv_sec > a->tv_sec || (b->tv_sec == a->tv_sec && b->tv_nsec > a->tv_nsec))
    {
        invert = true;
        struct timespec* c = a;
        a = b;
        b = c;
    }

    time_t sec = a->tv_sec - b->tv_sec;
    long ret = a->tv_nsec - b->tv_nsec;
    if (ret < 0)
    {
        --sec;
        ret += 1000000000;
    }
    if (sec > LONG_MAX / 1000000000 || ret > LONG_MAX - 1000000000 * sec) /* Would overflow */
    {
        ret = LONG_MAX;
    }
    else
    {
        ret += 1000000000 *sec;
    }

    if (invert)
    {
        if (ret < LONG_MAX)
        {
            ret *= -1;
        }
        else
        {
            ret = LONG_MIN; /* This leaves -LONG_MAX undistinguishable from LONG_MIN */
        }
    }

    return ret;
}


static void doPubSubTest(void)
{
    static bool testRunning = true;
    if (gPubStats.nbSamples > 0)
    {
        testRunning = true;
        const double sumSquare = (double)gPubStats.sumDiffSquare;
        const int32_t meanPubUs = (int32_t) (gPubStats.sumUs / gPubStats.nbSamples);
        const int32_t deviationPubUs = (int32_t) sqrt(sumSquare / gPubStats.nbSamples);

        //setTestStatus
        setTestStatus("Pub:%d smpls, Per.: mean %d us, stand. dev. = %d us, max. dev. =%llu us ",
                gPubStats.nbSamples, meanPubUs, deviationPubUs, gPubStats.maxDiff);
    }
    else
    {
        if (testRunning)
        {
            setTestStatus("No test running");
            testRunning = false;
        }
    }
}

static void onSubVarLoopRcv(const SOPC_NodeId* nid, SOPC_DataValue* pDv)
{
    if (pDv == NULL || pDv->Status != 0)
    {
        return;
    }
    static bool skipFirst = true;
    static uint32_t lastLoopNum = 0;

    int32_t result = -1;
    // Check listener
    SOPC_NodeId_Compare(pNidSubVarPeriod, nid, &result);
    if (result == 0)
    {
        // received New test period
        assert(pDv->Value.ArrayType == SOPC_VariantArrayType_SingleValue &&
                pDv->Value.BuiltInTypeId == SOPC_UInt32_Id);
        const uint32_t newValue = pDv->Value.Value.Uint32;
        if (newValue > 0 && newValue != gTestPeriod)
        {
            gTestPeriod = newValue;
            gPubStats.sumUs = 0;
            gPubStats.sumDiffSquare = 0;
            gPubStats.nbSamples = 0;
            lastLoopNum = 0;
            skipFirst = true;
            setTestStatus("Test period changed to %u", newValue);
        }
        return;
    }

    SOPC_NodeId_Compare(pNidSubVarLoop, nid, &result);
    if (result == 0)
    {
        assert(pDv->Value.ArrayType == SOPC_VariantArrayType_SingleValue &&
                pDv->Value.BuiltInTypeId == SOPC_UInt32_Id);
        const uint32_t newValue = pDv->Value.Value.Uint32;
        if (newValue == 0 )
        {
            gPubStats.sumUs = 0;
            gPubStats.sumDiffSquare = 0;
            gPubStats.nbSamples = 0;
            lastLoopNum = 0;
            skipFirst = true;
        }
        else if (newValue != lastLoopNum)
        {
            static SOPC_RealTime lastReception;
            SOPC_RealTime now;
            SOPC_RealTime_GetTime(&now);
            if (! skipFirst)
            {
                uint64_t diffUs = (uint64_t)abs(diff_timespec(&now, &lastReception)/1000);

                gPubStats.nbSamples ++;
                gPubStats.sumUs += (diffUs);

                uint64_t rateDiffUs;
                if (diffUs > gTestPeriod*1000)
                {
                    rateDiffUs = diffUs - gTestPeriod*1000;
                }
                else
                {
                    rateDiffUs = gTestPeriod*1000 - diffUs;
                }
                gPubStats.sumDiffSquare += (uint64_t) (rateDiffUs * rateDiffUs);
                if (rateDiffUs > gPubStats.maxDiff)
                {
                    gPubStats.maxDiff = rateDiffUs;
                }
            }

            skipFirst = false;

            lastReception = now;
            lastLoopNum = newValue;
        }
        return;
    }
    /*
    char* nidStr = SOPC_NodeId_ToCString(nid);
    printf("Unhandled noded Id : %s\n", nidStr);
    SOPC_Free(nidStr);
    */
}
