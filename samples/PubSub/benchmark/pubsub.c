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

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cache.h"
#include "sopc_assert.h"
#include "sopc_common.h"
#include "sopc_encodeable.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_platform_time.h"
#include "sopc_pub_scheduler.h"
#include "sopc_pubsub_conf.h"
#include "sopc_sub_scheduler.h"
#include "sopc_xml_loader.h"

#include "pubsub.h"

// Cyclic time to update global value
#define SLEEP_TIMEOUT_MS 20

#define NB_FIELDS_PER_DSM 7

static int32_t gDsmValues[NB_FIELDS_PER_DSM] = {1, 2, 3, 4, 5, 6, 7};

SOPC_Mutex globalValue_lock;

volatile sig_atomic_t stopSignal = 0;

SOPC_DataValue* get_source_value(const OpcUa_ReadValueId* nodesToRead,
                                 const int32_t nbValues); /* Source callback for Publisher */
static void signal_stop_program(int sig);                 /* catch stop signal to exit properly the program */

static void update_value(void)
{
    SOPC_Mutex_Lock(&globalValue_lock);
    for (int i = 0; i < NB_FIELDS_PER_DSM; i++)
    {
        gDsmValues[i] += i + 1;
    }
    SOPC_Mutex_Unlock(&globalValue_lock);
}

bool PubSub_common_init(void)
{
    /* Signal handling: close the program gracefully when interrupted */
    signal(SIGINT, signal_stop_program);
    signal(SIGTERM, signal_stop_program);

    SOPC_Mutex_Initialization(&globalValue_lock);

    /* Set log to debug trace and log path to directory "logs/" */
    const char* logPath = "./logs/";
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logLevel = SOPC_LOG_LEVEL_ERROR;
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = logPath;

    bool res = (SOPC_STATUS_OK == SOPC_Common_Initialize(logConfiguration));

    if (!res)
    {
        printf("ERROR: Toolkit Initialisation failed\n");
    }
    return res;
}

void PubSub_common_clear(void)
{
    SOPC_Common_Clear();
    SOPC_Mutex_Clear(&globalValue_lock);
}

static int setConfig_fromFile(SOPC_PubSubConfiguration** config, const char* configPath)
{
    if (NULL == config || NULL == configPath)
    {
        printf("Invalid parameters setConfig_fromFile\n");
        return false;
    }

    bool res = true;

    FILE* fd = fopen(configPath, "r");
    if (NULL == fd)
    {
        res = false;
        printf("ERROR: Failed to open file %s\n", configPath);
    }
    if (res && NULL != fd)
    {
        *config = SOPC_PubSubConfig_ParseXML(fd);
        if (NULL == *config)
        {
            res = false;
            printf("Failed to parse XML configuration file %s\n", configPath);
        }
        int closed = fclose(fd);
        if (0 != closed)
        {
            printf("ERROR: Failed to close file %s with status %d\n", configPath, closed);
            res = false;
        }
    }
    if (res)
    {
        printf("Publisher configured with file %s\n", configPath);
    }
    return res;
}

bool PubSub_publisher_bench(int priority)
{
    if (priority < 0 || priority > 99)
    {
        return false;
    }
    bool res = true;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PubSubConfiguration* config = NULL;
    SOPC_PubSourceVariableConfig* sourceConfig = NULL;

    res = setConfig_fromFile(&config, CONFIG_PUBLISHER_PATH);

    if (res)
    {
        sourceConfig = SOPC_PubSourceVariableConfig_Create(&get_source_value);
        if (sourceConfig == NULL)
        {
            printf("ERROR: publisher callback configuration hasn't been set\n");
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == status)
        {
            res = SOPC_PubScheduler_Start(config, sourceConfig, priority);
            if (!res)
            {
                printf("ERROR: publisher scheduler failed starting. Check logs\n");
            }
            else
            {
                printf("Publisher started \n");
            }
        }
        /* Wait for a signal and do your work */
        while (SOPC_STATUS_OK == status && res && 0 == stopSignal)
        {
            SOPC_Sleep(SLEEP_TIMEOUT_MS);
            update_value();
        }
    }
    if (stopSignal)
    {
        printf("Program stop by signal handling\n");
    }
    else
    {
        printf("Program stop caused by previous error. Check logs\n");
    }
    SOPC_PubScheduler_Stop();
    SOPC_PubSubConfiguration_Delete(config);
    SOPC_PubSourceVariableConfig_Delete(sourceConfig);
    return res;
}

static bool subscriber_bench(int priority)
{
    if (priority < 0 || priority > 99)
    {
        return false;
    }
    bool res = true;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_PubSubConfiguration* config = NULL;
    SOPC_SubTargetVariableConfig* targetConfig = NULL;

    res = setConfig_fromFile(&config, CONFIG_SUBSCRIBER_PATH);

    if (res)
    {
        targetConfig = SOPC_SubTargetVariableConfig_Create(&Cache_SetTargetVariables);
        if (targetConfig == NULL)
        {
            printf("ERROR: subscriber callback configuration hasn't been set\n");
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == status)
        {
            /* Initialize Cache */
            res = Cache_Initialize(config);
            if (!res)
            {
                printf("ERROR: initializing cache failed\n");
                status = SOPC_STATUS_NOK;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            res = SOPC_SubScheduler_Start(config, targetConfig, NULL, NULL, NULL, priority);
            if (!res)
            {
                printf("ERROR: subscriber scheduler failed starting. Check logs\n");
            }
            else
            {
                printf("Subscriber started \n");
            }
        }
        /* Wait for a signal and do your work */
        while (SOPC_STATUS_OK == status && res && 0 == stopSignal)
        {
            SOPC_Sleep(SLEEP_TIMEOUT_MS);
        }
    }
    if (stopSignal)
    {
        printf("Program stop by signal\n");
    }
    else
    {
        printf("Program stop caused by previous error. Check logs\n");
    }
    SOPC_SubScheduler_Stop();
    SOPC_PubSubConfiguration_Delete(config);
    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    Cache_Clear();
    return res;
}

// core_id = 0, 1, ... n-1, where n is the system's number of cores
static int stick_this_thread_to_core(int core_id)
{
    long num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id < 0 || core_id >= num_cores)
        return -1;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET((size_t) core_id, &cpuset);

    pid_t currentPid = getpid();
    return sched_setaffinity(currentPid, sizeof(cpu_set_t), &cpuset);
}

bool PubSub_subscriber_bench(int priority)
{
    // launch a publisher in another CPU
    bool res = true;
    pid_t pidPublisher = fork();

    // Let run the two processes on different cores
    if (0 == pidPublisher)
    {
        int corePublisher = 1;
        int result = stick_this_thread_to_core(corePublisher);
        if (0 != result)
        {
            printf("Failed to attach publisher process to core %d\n", corePublisher);
            res = false;
        }
        else
        {
            printf("Publisher process attach to core %d\n", corePublisher);
            res = PubSub_publisher_bench(priority);
        }
    }
    else
    {
        int coreSubscriber = 0;
        int result = stick_this_thread_to_core(coreSubscriber);
        if (0 != result)
        {
            printf("Failed to attach subscriber process to core %d\n", coreSubscriber);
            res = false;
        }
        else
        {
            printf("subscriber process attach to core %d\n", coreSubscriber);
            res = subscriber_bench(priority);
        }
        kill(pidPublisher, SIGTERM);
    }
    return res;
}

/* Source callback */
SOPC_DataValue* get_source_value(const OpcUa_ReadValueId* nodesToRead, const int32_t nbValues)
{
    SOPC_ASSERT(NULL != nodesToRead && nbValues > 0);
    SOPC_ASSERT(nbValues <= NB_FIELDS_PER_DSM);

    SOPC_DataValue* dvs = SOPC_Calloc((size_t) nbValues, sizeof(SOPC_DataValue));
    SOPC_ASSERT(dvs != NULL);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    for (int32_t i = 0; SOPC_STATUS_OK == status && i < nbValues; ++i)
    {
        SOPC_DataValue* dv = &dvs[i];

        SOPC_DataValue_Initialize(dv);
        SOPC_Variant* var = &dv->Value;
        var->BuiltInTypeId = SOPC_Int32_Id;
        var->ArrayType = SOPC_VariantArrayType_SingleValue;

        SOPC_Mutex_Lock(&globalValue_lock);
        var->Value.Int32 = gDsmValues[i];
        SOPC_Mutex_Unlock(&globalValue_lock);
    }

    if (SOPC_STATUS_OK != status)
    {
        for (int32_t i = 0; i < nbValues; ++i)
        {
            SOPC_DataValue_Clear(&dvs[i]);
        }
        SOPC_Free(dvs);
        dvs = NULL;
    }

    SOPC_ASSERT(status == SOPC_STATUS_OK);

    return dvs;
}

static void signal_stop_program(int sig)
{
    SOPC_UNUSED_ARG(sig);
    if (stopSignal != 0)
    {
        exit(1);
    }
    else
    {
        stopSignal = 1;
    }
}
