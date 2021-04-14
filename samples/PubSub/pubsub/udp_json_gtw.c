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

/* ---------------------------------------- */
/* INCLUDES */
/* ---------------------------------------- */
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "udp_json_gtw.h"
#include "sopc_atomic.h"
#include "sopc_time.h"
#include "p_time.h"
#include "sopc_builtintypes.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"
#include "cache.h"

/* ---------------------------------------- */
/* DEFINE */
/* ---------------------------------------- */
#define ONE_HUNDRED_MILLISEC 100
#define TEN_MILLISEC 10
#define FIVE_MILLISEC 5
#define ONE_MILLISEC 1
#define TWO_MILLISEC 2
#define ONE_SEC_IN_MILLISEC 1000

#define BOARD1 0
#define BOARD2 1

#if BOARD1
#define UDP_PORT_IN 5000
#define UDP_PORT_OUT 5002
#define SUB_SUFFIX "_Sub"
#endif


#if BOARD2
#define UDP_PORT_IN 6000
#define UDP_PORT_OUT 6002
#define SUB_SUFFIX ""
#endif

#define MAXSIZE 65536
#define NBDEVICES 30
#define NBDATA 2
#define DATAID "ABCDEFGHIJK"
#define MAXNBCHAR 100

#if 0
#define DATA0 "data0"
#define DATA0_TYPE SOPC_UInt32_Id
#define DATA1 "data1"
#define DATA1_TYPE SOPC_Int16_Id
#define DATA2 "data2"
#define DATA2_TYPE SOPC_Int32_Id
#define DATA3 "data3"
#define DATA3_TYPE SOPC_String_Id
#endif

#define PREFIX "D"
#define DATA_TYPE SOPC_UInt32_Id

#define NODEID_PREFIX "ns=1;s="

#define REQUEST_HEADER_1 "POST HTTP/1.1\nHost UDP:"
#define REQUEST_HEADER_2 ",\n\"data\":{ "
#define REQUEST_END "}\n"

#define REQUEST_LENGTH 65535

/* ---------------------------------------- */
/* TYPES DEFINITION */
/* ---------------------------------------- */
typedef struct DataCfg
{
    char DataId;
    SOPC_BuiltinId type;
} DataCfg;

typedef struct DevCfg
{
    int DeviceId;
    DataCfg Data[NBDATA];
} DevCfg;

/* ---------------------------------------- */
/* CONFIGURATION */
/* ---------------------------------------- */
static DevCfg Cfg[NBDEVICES];

/* ---------------------------------------- */
/* GLOBAL VARIABLES */
/* ---------------------------------------- */
static char* localhost_ip = "127.0.0.1";

/* Reception global variables */
static int32_t UDP_recepRunning = 0;
static int recepSockfd = 0;
static struct sockaddr_in recepSock;
static Thread recepThreadId = 0;

/* Sending global variables */
static int32_t UDP_sendRunning = 0;
static int sendSockfd = 0;
static struct sockaddr_in sendSock;
static Thread sendThreadId = 0;

/* ---------------------------------------- */
/* STATIC FUNCTIONS */
/* ---------------------------------------- */
static SOPC_ReturnStatus UDP_ReceptionConfigure (void);
static SOPC_ReturnStatus UDP_SendingConfigure (void);
static void* UDP_ManageReception (void* arg);
static void* UDP_ManageSending (void* arg);
static void UDP_DecodeKeyValuesUpdateCache(char* buffer);
static void UDP_CreateNodeId(char* dataName, char* nodeId_Str);
static void UDP_CreateRequest(char* request);

static SOPC_ReturnStatus UDP_ReceptionConfigure (void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Create socket file descriptor for reception */
    if ((recepSockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("# Error: reception socket creation failed.\n");
        status = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Configure reception information */
        recepSock.sin_family = AF_INET;
        recepSock.sin_addr.s_addr = inet_addr(localhost_ip);
        recepSock.sin_port = htons(UDP_PORT_IN);

        /* Bind reception socket to the server address */
        if (bind(recepSockfd, (const struct sockaddr *)&recepSock, sizeof(recepSock)) < 0 )
        {
            printf("# Error: reception socket binding failed.\n");
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        printf("# Info: UDP reception socket configured.\n");
    }
    else
    {
        printf("# Error: UDP reception socket configuration failed.\n");
    }

    return status;
}


static SOPC_ReturnStatus UDP_SendingConfigure (void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Create socket file descriptor for sending */
    if ((sendSockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("# Error: sending socket creation failed.\n");
        status = SOPC_STATUS_NOK;
    }

    /* Configure sending information */
    sendSock.sin_family = AF_INET;
    sendSock.sin_addr.s_addr = inet_addr(localhost_ip);
    sendSock.sin_port = htons(UDP_PORT_OUT);

    return status;
}


SOPC_ReturnStatus UDP_Configure (void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

#if 0
    /* Set configuration */
    strcpy(Cfg[0].name, DATA0);
    Cfg[0].type = DATA0_TYPE;
    strcpy(Cfg[1].name, DATA1);
    Cfg[1].type = DATA1_TYPE;
    strcpy(Cfg[2].name, DATA2);
    Cfg[2].type = DATA2_TYPE;
    strcpy(Cfg[3].name, DATA3);
    Cfg[3].type = DATA3_TYPE;
#endif

    /* Initialize device and data configuration */
    for (int i = 0; i < NBDEVICES; i++)
    {
        Cfg[i].DeviceId = i;
        for (int j = 0; j < NBDATA; j++)
        {
            Cfg[i].Data[j].DataId = DATAID[j];
            Cfg[i].Data[j].type = DATA_TYPE;
        }
    }

    /* Configure Reception */
    status = UDP_ReceptionConfigure();

    if (SOPC_STATUS_OK == status)
    {
        /* Configure Sending */
        status = UDP_SendingConfigure();
    }

    return status;
}

static void UDP_CreateNodeId(char* dataName, char* nodeId_Str)
{
    memset(nodeId_Str, '\0', 2*MAXNBCHAR);
    strcpy(nodeId_Str, NODEID_PREFIX);
    strcat(nodeId_Str, dataName);
}


static void UDP_DecodeKeyValuesUpdateCache(char* buffer)
{
    uint8_t dataIdx = 0;
    char* dataArr[NBDATA*NBDEVICES] = {NULL};
    char nodeId_Str[2*MAXNBCHAR] = {0};
    char comma[] = ",";
    char *data = strtok(buffer, comma);

    /* Split different data under format [ Name :Value] */
    /* Data are separated by commas */
    while ((data != NULL) && (dataIdx < (NBDATA*NBDEVICES)))
    {
        dataArr[dataIdx] = data;
        data = strtok(NULL, comma);
        dataIdx++;
    }
    if (dataIdx < (NBDATA*NBDEVICES))
    {
        printf("# Warning: received only %" PRIu8 " values\n", dataIdx);
    }

    /* For each data, split value and name */
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    Cache_Lock();
    for (int i = 0; SOPC_STATUS_OK == status && i < dataIdx; i++)
    {
        /* The cache keeps the ownership of the NodeId and the DataValue, so we must create new ones */
        SOPC_NodeId* nodeId = NULL;
        SOPC_DataValue* dataValue = SOPC_Calloc(1, sizeof(SOPC_DataValue));
        assert(NULL != dataValue);
        SOPC_Variant* val = &dataValue->Value;

        char name[MAXNBCHAR];
        char text[MAXNBCHAR];

        /* Parse the name and the value */
        switch (Cfg[i/NBDATA].Data[i%NBDATA].type)
        {
            case SOPC_String_Id :
                sscanf(dataArr[i], "%s :%s" , name,  text);
                status = SOPC_String_InitializeFromCString(&val->Value.String, text);
                break;

            case SOPC_Int32_Id :
                sscanf(dataArr[i], "%s :%" SCNi32, name, &val->Value.Int32);
                break;

            case SOPC_UInt32_Id :
                sscanf(dataArr[i], "%s :%" SCNu32, name, &val->Value.Uint32);
                break;

            case SOPC_Int16_Id :
                sscanf(dataArr[i], "%s :%" SCNi16, name, &val->Value.Int16);
                break;

            default :
                printf("# Warning: Unexpected type in UDP configuration: %d\n", Cfg[i/NBDATA].Data[i%NBDATA].type);
                status = SOPC_STATUS_NOK;
                break;
        }

        /* Prepare the NodeId, finalize the DataValue */
        if (SOPC_STATUS_OK == status)
        {
            dataValue->SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
            val->ArrayType = SOPC_VariantArrayType_SingleValue;
            val->BuiltInTypeId = Cfg[i/NBDATA].Data[i%NBDATA].type;

            UDP_CreateNodeId(name, nodeId_Str);
            nodeId = SOPC_NodeId_FromCString(nodeId_Str, (int32_t) strlen(nodeId_Str));
            if (NULL == nodeId)
            {
                status = SOPC_STATUS_NOK;
                printf("# Error: Failed to create Node Id\n");
            }
        }

        /* Update the cache */
        if (SOPC_STATUS_OK == status)
        {
            bool ok = Cache_Set(nodeId, dataValue);
            if (! ok)
            {
                status = SOPC_STATUS_NOK;
                printf("# Error: Failed to insert received value in cache\n");
            }
        }
    }
    Cache_Unlock();
}

static void UDP_DecodeDataAndUpdateCache(char* buffer)
{
    char dataBuffer[MAXSIZE] = {0};
    char c = 0;
    uint8_t nbCurlyBracket = 0;
    size_t buffIdx = 0;
    size_t startIdx = 0;
    bool dataIsolated = false;

    /* Isolate data part of the received frame */
    do
    {
        /* Find the beginning of the data frame part*/
        if ('{' == c)
        {
            nbCurlyBracket++;
            if (2 == nbCurlyBracket)
            {
                /* Store the data part of the frame that is after the second curly bracket*/
                strcpy(dataBuffer, &buffer[buffIdx + 1]);
                /* Store the start index */
                startIdx = (uint8_t)(buffIdx + 1);
            }
        }

        /* Find the end of the data frame part by looking for a closing curly bracket */
        if ('}' == c)
        {
            dataBuffer[buffIdx - startIdx] = '\0';
            dataIsolated = true;
        }

        buffIdx++;
        c = buffer[buffIdx];
    } while ((!dataIsolated) && (buffIdx < MAXSIZE) && (c != '\0'));

    /* Decode Data name and value */
    UDP_DecodeKeyValuesUpdateCache(dataBuffer);
}


static void* UDP_ManageReception (void* arg)
{
    /*unused parameter */
    (void) arg;

    char buffer[MAXSIZE]  = {0};
    ssize_t data_len = 0;
    SOPC_RealTime *next_timeout = SOPC_RealTime_Create(NULL);
    SOPC_RealTime *now = SOPC_RealTime_Create(NULL);
    assert(NULL != next_timeout && NULL != now);
    bool warned = false;
    bool ok;

    printf("# Info: UDP Reception thread running.\n");

    while (SOPC_Atomic_Int_Get(&UDP_recepRunning))
    {
        ok = SOPC_RealTime_GetTime(now);
        assert(ok && "Failed GetTime");

        /* Wait for data reception */
        data_len = recv(recepSockfd, &buffer, MAXSIZE, MSG_DONTWAIT);

        if (data_len < 0 && ok)
        {
            SOPC_RealTime_AddDuration(next_timeout, 0.5);
            if (SOPC_RealTime_IsExpired(next_timeout, now) && !warned)
            {
                /* The next_timeout has already expired, don't sleep! */
                /* TODO: find other message ID, such as the PublisherId */
                printf("# Warning: UDP JSON could not be received in time\n");
                warned = true; /* Avoid being spammed @ 10kHz and being even slower because of this */
            }
            else
            {
                ok = SOPC_RealTime_SleepUntil(next_timeout) == 0;
                assert(ok && "Failed NanoSleep");
            }
        }
        else
        {
            /* Set end of line */
            buffer[data_len] = '\0';

            /* Decode received data and update OPC UA Server nodes */
            UDP_DecodeDataAndUpdateCache(buffer);
        }
    }

    /* Exit current thread */
    printf("# Info: Exit UDP reception thread.\n");
    return NULL;
}


static void UDP_CreateRequest(char* request)
{
    char buffer[MAXNBCHAR] = {0};
    char name[MAXNBCHAR];
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Reset Request content */
    memset(request, '\0', REQUEST_LENGTH);

    /* Build request header */
    strcpy(request, REQUEST_HEADER_1);

    sprintf(buffer, "%d;", UDP_PORT_IN);
    strncat(request, buffer, REQUEST_LENGTH-strlen(request)-1);
    strncat(request, localhost_ip, REQUEST_LENGTH-strlen(request));
    sprintf(buffer, ";%d", UDP_PORT_OUT);
    strncat(request, buffer, REQUEST_LENGTH-strlen(request));
    strncat(request, REQUEST_HEADER_2, REQUEST_LENGTH-strlen(request));

    /* Add request data */
    Cache_Lock();
    for (int i = 0; i < NBDEVICES; i++)
    {
        for (int j = 0; j < NBDATA; j++)
        {
            /* Compute data name : "data<deviceId><dataId>" */
            memset(name, '\0', MAXNBCHAR);
            snprintf(name, MAXNBCHAR, "%s%d%c", PREFIX, Cfg[i].DeviceId, Cfg[i].Data[j].DataId);

            strncat(request, name, REQUEST_LENGTH-strlen(request));
            strncat(request, " :", REQUEST_LENGTH-strlen(request));

            /* Attach the name to a local NodeId to avoid the allocation a NodeId for each Cache_Get */
            SOPC_NodeId nid = {.IdentifierType = SOPC_IdentifierType_String,
                    .Namespace = 1};

            /* Add suffix in order to retrieve received value in cache */
            strcat(name, SUB_SUFFIX);
            status = SOPC_String_AttachFromCstring(&nid.Data.String, name);
            assert(SOPC_STATUS_OK == status);

            SOPC_DataValue *dv = Cache_Get(&nid);
            if (NULL == dv)
            {
                printf("# Error: \"%s\" not found in cache\n", name);
                continue;
            }

            SOPC_Variant *val = &dv->Value;

            /* Encode the value in the JSON */
            switch (val->BuiltInTypeId)
            {
                case SOPC_String_Id :
                    if (val->Value.String.Length <= 0)
                    {
                        strncat(request, "\"\"", REQUEST_LENGTH-2);
                    }
                    else
                    {
                        strncat(request, (char*)val->Value.String.Data, REQUEST_LENGTH-strlen(request));
                    }
                    break;

                case SOPC_Int32_Id :
                    sprintf(buffer, "%d", val->Value.Int32);
                    strncat(request, buffer, REQUEST_LENGTH-strlen(request));
                    break;

                case SOPC_UInt32_Id :
                    sprintf(buffer, "%d", val->Value.Uint32);
                    strncat(request, buffer, REQUEST_LENGTH-strlen(request));
                    break;

                case SOPC_Int16_Id :
                    sprintf(buffer, "%d", val->Value.Int16);
                    strncat(request, buffer, REQUEST_LENGTH-strlen(request));
                    break;

                default :
                    printf("# Warning: Received unexpected type : %d\n", val->BuiltInTypeId);
                    break;
            }

            if (NBDATA-1 != i)
            {
                strncat(request, ", ", REQUEST_LENGTH-strlen(request));
            }
        }
    }
    Cache_Unlock();

    /* Finalize request (both open curly bracket to be closed) */
    strncat(request, REQUEST_END, REQUEST_LENGTH-strlen(request));
    strncat(request, REQUEST_END, REQUEST_LENGTH-strlen(request));
}


static void* UDP_ManageSending (void* arg)
{
    /*unused parameter */
    (void) arg;

    char request[REQUEST_LENGTH] = {0};
    SOPC_RealTime *next_timeout = SOPC_RealTime_Create(NULL);
    SOPC_RealTime *now = SOPC_RealTime_Create(NULL);
    assert(NULL != next_timeout && NULL != now);
    bool warned = false;
    bool ok;

    printf("# Info: UDP Sending thread running.\n");

    while (SOPC_Atomic_Int_Get(&UDP_sendRunning))
    {
        ok = SOPC_RealTime_GetTime(now);
        assert(ok && "Failed GetTime");
        /* Create request with received data values */
        UDP_CreateRequest(request);

        /* Send UDP datagram */
        sendto(sendSockfd, request, strlen(request),
               MSG_CONFIRM, (const struct sockaddr *) &sendSock,
               sizeof(sendSock));

        /* Compute the next timeout, adding the wait period in ms (as a float) */
        SOPC_RealTime_AddDuration(next_timeout, TWO_MILLISEC);
        if (SOPC_RealTime_IsExpired(next_timeout, now) && !warned)
        {
            /* The next_timeout has already expired, don't sleep! */
            /* TODO: find other message ID, such as the PublisherId */
            printf("# Warning: UDP JSON could not be sent in time\n");
            warned = true; /* Avoid being spammed @ 10kHz and being even slower because of this */
        }
        else
        {
            ok = SOPC_RealTime_SleepUntil(next_timeout) == 0;
            assert(ok && "Failed NanoSleep");
        }
    }

    /* Free timers */
    SOPC_RealTime_Delete(&next_timeout);
    SOPC_RealTime_Delete(&now);

    /* Exit current thread */
    printf("# Info: Exit UDP sending thread.\n");
    return NULL;
}


bool UDP_Start(void)
{
    bool UDP_recepOK = false;
    bool UDP_sendOK = true;

    /* Create and start reception thread */
    SOPC_Atomic_Int_Set(&UDP_recepRunning, 1);
    SOPC_ReturnStatus status = SOPC_Thread_Create(&recepThreadId, &UDP_ManageReception, NULL, "ReceiveJSON");

    if (SOPC_STATUS_OK == status)
    {
        UDP_recepOK = true;
        printf("# Info: UDP Reception thread %ld created.\n", recepThreadId);
    }

    /* Create and start sending thread */
    SOPC_Atomic_Int_Set(&UDP_sendRunning, 1);
#if BOARD1
    status = SOPC_Thread_CreatePrioritized(&sendThreadId, &UDP_ManageSending, NULL, 80, "SendJSON");
#endif
#if BOARD2
    status = SOPC_Thread_Create(&sendThreadId, &UDP_ManageSending, NULL, "SendJSON");
#endif
    if (SOPC_STATUS_OK == status)
    {
        UDP_sendOK = true;
        printf("# Info: UDP Sending thread %ld created.\n", sendThreadId);
    }

    return UDP_recepOK && UDP_sendOK;
}


void UDP_Stop(void)
{
    printf("# Info: Stop UDP Gateway.\n");
    SOPC_Atomic_Int_Set(&UDP_recepRunning, 0);
    SOPC_Atomic_Int_Set(&UDP_sendRunning, 0);

    /* Wait for threads to be reset*/
    SOPC_Thread_Join(recepThreadId);
    SOPC_Thread_Join(sendThreadId);
}


void UDP_StopAndClear(void)
{
    UDP_Stop();

    /* Clear */
    // TODO
}
