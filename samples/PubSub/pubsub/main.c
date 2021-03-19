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

#include <assert.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "p_time.h"
#include "sopc_common.h"
#include "sopc_mem_alloc.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_sub_scheduler.h"
#include "sopc_time.h"
#include "sopc_xml_loader.h"

#include "cache.h"
#include "config.h"

#include <time.h> /* For now, requires struct timespec */


volatile sig_atomic_t stopSignal = 0;
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

static const char* getenv_default(const char* name, const char* default_value)
{
    const char* val = getenv(name);

    return (val != NULL) ? val : default_value;
}

/* PubSub callbacks (for the emitter) */
static SOPC_DataValue* get_source_increment(OpcUa_ReadValueId* nodesToRead, int32_t nbValues);
static bool set_target_compute_rtt(OpcUa_WriteValue* nodesToWrite, int32_t nbValues);

/* RTT calculations */
SOPC_RealTime *g_ts_emissions = NULL;
long *g_rtt = NULL;
size_t g_n_samples = 0;
static inline long diff_timespec(struct timespec *a, struct timespec *b);

int main(int argc, char* const argv[])
{
    /* Signal handling: close the server gracefully when interrupted */
    signal(SIGINT, signal_stop_server);
    signal(SIGTERM, signal_stop_server);

    /* Parse command line arguments ? */
    /* argparse */
    (void) argc;
    (void) argv;

    /* General initializations */
    const char* log_path = getenv_default("LOG_PATH", LOG_PATH);
    printf("LOG_PATH: %s\n", log_path);
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = log_path;
    logConfiguration.logLevel = SOPC_LOG_LEVEL_INFO;
    SOPC_ReturnStatus status = SOPC_Common_Initialize(logConfiguration);

    if (SOPC_STATUS_OK != status)
    {
        printf("Error while initializing logs\n");
    }

    sscanf(getenv_default("RTT_SAMPLES", RTT_SAMPLES), "%zu", &g_n_samples);
    printf("RTT_SAMPLES: %zu\n", g_n_samples);
    g_ts_emissions = SOPC_Calloc(g_n_samples, sizeof(struct timespec));
    g_rtt = SOPC_Calloc(g_n_samples, sizeof(long));
    if (NULL == g_ts_emissions || NULL == g_rtt)
    {
        printf("Error while allocating %zd round-trip time measurements\n", g_n_samples);
        status = SOPC_STATUS_NOK;
    }

    /* Configuration of the PubSub */
    const bool is_loopback = atoi(getenv_default("IS_LOOPBACK", IS_LOOPBACK)) != 0;
    printf("IS_LOOPBACK: %d\n", is_loopback);
    SOPC_PubSubConfiguration* config = NULL;
    if (SOPC_STATUS_OK == status)
    {
        const char* config_path = getenv_default("PUBSUB_XML_CONFIG", is_loopback ? PUBSUB_XML_CONFIG_LOOP : PUBSUB_XML_CONFIG_EMIT);
        printf("PUBSUB_XML_CONFIG: %s\n", config_path);

        FILE* fd = fopen(config_path, "r");
        config = SOPC_PubSubConfig_ParseXML(fd);
        int closed = fclose(fd);

        status = (0 == closed && NULL != config) ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_PARAMETERS;
        if(SOPC_STATUS_OK != status)
        {
            printf("Error while loading PubSub configuration from %s\n", config_path);
        }
    }

    SOPC_PubSourceVariableConfig* sourceConfig = NULL;
    SOPC_SubTargetVariableConfig* targetConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        if (is_loopback)
        {
            sourceConfig = SOPC_PubSourceVariableConfig_Create(&Cache_GetSourceVariables);
        }
        else
        {
            sourceConfig = SOPC_PubSourceVariableConfig_Create(&get_source_increment);
        }
        if (NULL == sourceConfig)
        {
            printf("Error while loading PubSub configuration\n");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        if (is_loopback)
        {
            targetConfig = SOPC_SubTargetVariableConfig_Create(&Cache_SetTargetVariables);
        }
        else
        {
            targetConfig = SOPC_SubTargetVariableConfig_Create(&set_target_compute_rtt);
        }
        if (NULL == sourceConfig)
        {
            printf("Error while loading PubSub configuration\n");
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
            bool res = SOPC_PubScheduler_Start(config, sourceConfig);
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
            /* TODO: state changed callback */
            bool res = SOPC_SubScheduler_Start(config, targetConfig, NULL);
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
    }

    /* Clean and quit */
    SOPC_PubScheduler_Stop();
    SOPC_SubScheduler_Stop();
    Cache_Clear();
    SOPC_PubSourceVariableConfig_Delete(sourceConfig);
    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    SOPC_PubSubConfiguration_Delete(config);
    printf("# Info: PubSub stopped\n");

    /* print the non-empty RTT */
    for (size_t idx=0; idx<g_n_samples; ++idx)
    {
        if (0 != g_rtt[idx])
        {
            printf("% 9zd: round-trip time: % 12ld ns\n", idx, g_rtt[idx]);
        }
    }
    SOPC_Free(g_ts_emissions);
    SOPC_Free(g_rtt);
}

static SOPC_DataValue* get_source_increment(OpcUa_ReadValueId* nodesToRead, int32_t nbValues)
{
    /* Pre-hook: filter and increment the counter */
    static SOPC_NodeId nid_counter = NODEID_COUNTER_SEND;

    for (int32_t i=0; i<nbValues; ++i)
    {
        int32_t cmp = 0;
        SOPC_ReturnStatus status = SOPC_NodeId_Compare(&nid_counter, &nodesToRead[i].NodeId, &cmp);
        if (SOPC_STATUS_OK == status && 0 == cmp)
        {
            /* Get the value and modify it in place */
            SOPC_DataValue *dv_counter = Cache_Get(&nid_counter);
            assert(NULL != dv_counter);
            SOPC_Variant *var = &dv_counter->Value;
            assert(SOPC_VariantArrayType_SingleValue == var->ArrayType);
            assert(SOPC_UInt32_Id == var->BuiltInTypeId);
            ++var->Value.Uint32;

            /* Record the current time */
            size_t idx = var->Value.Uint32;
            if (idx < g_n_samples)
            {
                bool ok = SOPC_RealTime_GetTime(&g_ts_emissions[idx]);
                (void) ok;
            }
        }
    }

    /* Let the cache handle the memory and treatment of this request */
    return Cache_GetSourceVariables(nodesToRead, nbValues);
}

static bool set_target_compute_rtt(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    /* Pre-hook: filter and use the new counter value to compute the round trip time */
    static SOPC_NodeId nid_counter = NODEID_COUNTER_RECV;

    for (int32_t i=0; i<nbValues; ++i)
    {
        int32_t cmp = 0;
        SOPC_ReturnStatus status = SOPC_NodeId_Compare(&nid_counter, &nodesToWrite[i].NodeId, &cmp);
        if (SOPC_STATUS_OK == status && 0 == cmp)
        {
            /* Compute the round-trip time */
            static SOPC_RealTime now = {0}; /* Requires a static initializer to be abstracted in p_time interface */
            bool ok = SOPC_RealTime_GetTime(&now);
            if (ok)
            {
                SOPC_Variant *var = &nodesToWrite[i].Value.Value;
                size_t idx = var->Value.Uint32;
                if (idx < g_n_samples)
                {
                    ok &= SOPC_VariantArrayType_SingleValue == var->ArrayType;
                    ok &= SOPC_UInt32_Id == var->BuiltInTypeId;
                    if (ok)
                    {
                        g_rtt[idx] = diff_timespec(&now, &g_ts_emissions[idx]);
                    }
                }
            }
        }
    }

    /* Let the cache handle the memory and treatment of this request */
    /* TODO: lock before write or double buffer */
    return Cache_SetTargetVariables(nodesToWrite, nbValues);
}

/* TODO: implement this in a platform independent fashion */
static inline long diff_timespec(struct timespec *a, struct timespec *b)
{
    if (!a || !b)
    {
        return LONG_MAX;
    }

    bool invert = false;
    if (b->tv_sec > a->tv_sec || (b->tv_sec == a->tv_sec && b->tv_nsec > a->tv_nsec))
    {
        invert = true;
        struct timespec *c = a;
        a = b;
        b = c;
    }

    time_t sec = a->tv_sec - b->tv_sec;
    long ret = a->tv_nsec - b->tv_nsec;
    if (ret < 0) {
        --sec;
        ret += 1000000000;
    }
    if (sec > LONG_MAX/1000000000 || ret > LONG_MAX-1000000000*sec) /* Would overflow */
    {
        ret = LONG_MAX;
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

