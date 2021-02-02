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
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "udp_json_gtw.h"
#include "sopc_atomic.h"
#include "sopc_time.h"
#include "sopc_builtintypes.h"
#include "sopc_mem_alloc.h"
#include "server.h"
#include "config.h"

/* ---------------------------------------- */
/* DEFINE */
/* ---------------------------------------- */
#define UDP_PORT_IN 5000
#define UDP_PORT_OUT 5002

#define MAXSIZE 1024
#define NBDATA 4
#define MAXNBCHAR 100

#define DATA0 "data0"
#define DATA0_TYPE SOPC_UInt32_Id
#define DATA1 "data1"
#define DATA1_TYPE SOPC_Int16_Id
#define DATA2 "data2"
#define DATA2_TYPE SOPC_Int32_Id
#define DATA3 "data3"
#define DATA3_TYPE SOPC_String_Id

#define NODEID_PREFIX "ns=1;s="

/* ---------------------------------------- */
/* TYPES DEFINITION */
/* ---------------------------------------- */
typedef struct DataType
{
    char name[MAXNBCHAR];
    union {
        int16_t int16;
        uint32_t uint32;
        int32_t int32;
        char text[MAXNBCHAR];
    } Value;

} DataType;

typedef struct DataCfg
{
    char name[MAXNBCHAR];
    SOPC_BuiltinId type;
} DataCfg;

/* ---------------------------------------- */
/* CONFIGURATION */
/* ---------------------------------------- */
static DataCfg Cfg[NBDATA];

/* ---------------------------------------- */
/* GLOBAL VARIABLES */
/* ---------------------------------------- */
static char* localhost_ip = "127.0.0.1";

/* Reception global variables */
static int32_t UDP_recepRunning = 0;
static int recepSockfd = 0;
static struct sockaddr_in servaddr;
static pthread_t recepThreadId = 0;

/* Sending global variables */
static int32_t UDP_sendRunning = 0;
static int sendSockfd = 0;
static struct sockaddr_in cltaddr;
static pthread_t sendThreadId = 0;

/* Data global variables */
static DataType Data[NBDATA];

/* ---------------------------------------- */
/* STATIC FUNCTIONS */
/* ---------------------------------------- */
static SOPC_ReturnStatus UDP_ReceptionConfigure (void);
static SOPC_ReturnStatus UDP_SendingConfigure (void);
static void* UDP_ManageReception (void* arg);
static void* UDP_ManageSending (void* arg);
static void UDP_DecodeDataAndUpdateServer(char* buffer);
static void UDP_DecodeData(char* buffer);
static bool UDP_UpdateServer(void);
static void UDP_CreateWriteValue(OpcUa_WriteValue* writeValue, uint8_t dataIdx);


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
        /* Configure server information */
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(localhost_ip);
        servaddr.sin_port = htons(UDP_PORT_IN);

        /* Bind reception socket to the server address */
        if (bind(recepSockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
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

    // TODO

    return status;
}


SOPC_ReturnStatus UDP_Configure (void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Set configuration */
    strcpy(Cfg[0].name, DATA0);
    Cfg[0].type = DATA0_TYPE;
    strcpy(Cfg[1].name, DATA1);
    Cfg[1].type = DATA1_TYPE;
    strcpy(Cfg[2].name, DATA2);
    Cfg[2].type = DATA2_TYPE;
    strcpy(Cfg[3].name, DATA3);
    Cfg[3].type = DATA3_TYPE;

    /* Configure Reception */
    status = UDP_ReceptionConfigure();

    if (SOPC_STATUS_OK == status)
    {
        /* Configure Sending */
        status = UDP_SendingConfigure();
    }

    return status;
}

static void UDP_CreateWriteValue(OpcUa_WriteValue* writeValue, uint8_t dataIdx)
{
    SOPC_DataValue* dataValue = &writeValue->Value;
    SOPC_Variant* val = &dataValue->Value;

    writeValue->AttributeId = SOPC_AttributeId_Value;
    dataValue->SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();
    val->BuiltInTypeId = Cfg[dataIdx].type;
    val->ArrayType = SOPC_VariantArrayType_SingleValue;
    switch (Cfg[dataIdx].type)
    {
        case SOPC_String_Id :
            val->Value.String.Length = (int32_t) strlen(Data[dataIdx].Value.text);
            val->Value.String.DoNotClear = true;
            val->Value.String.Data = (SOPC_Byte*) Data[dataIdx].Value.text;
            break;

        case SOPC_Int32_Id :
            val->Value.Int32 = Data[dataIdx].Value.int32;
            break;

        case SOPC_UInt32_Id :
            val->Value.Uint32 = Data[dataIdx].Value.uint32;
            break;

        case SOPC_Int16_Id :
            val->Value.Int16 =  Data[dataIdx].Value.int16;
            break;

        default :
            printf("# Info: Unexpected type in UDP configuration: %d\n", Cfg[dataIdx].type);
            break;
    }
}


static bool UDP_UpdateServer(void)
{
    OpcUa_WriteValue* writeValues = SOPC_Calloc(NBDATA, sizeof(OpcUa_WriteValue));
    bool status = false;
    char nodeId_Str[2*MAXNBCHAR];
    SOPC_NodeId* nodeId = NULL;

    /* For each data, create OPC UA Node Id and Write Value */
    for (uint8_t i = 0; i < NBDATA; i++)
    {
        /* Generate Node Id*/
        memset(nodeId_Str, '\0', 2*MAXNBCHAR);
        strcpy(nodeId_Str, NODEID_PREFIX);
        strcat(nodeId_Str, Data[i].name);
        printf("# Info: Update Node Id  %s\n", nodeId_Str);
        nodeId = SOPC_NodeId_FromCString(nodeId_Str, (int32_t) strlen(nodeId_Str));

        /* Create OPC UA Write value */
        UDP_CreateWriteValue(&writeValues[i], i);
        status = SOPC_NodeId_Copy(&writeValues[i].NodeId, nodeId);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Send create values to the server for update */
        status = Server_SetTargetVariables(writeValues, NBDATA);
    }
    else
    {
        SOPC_Free(writeValues);
        writeValues = NULL;
    }
    return status;
}


static void UDP_DecodeData(char* buffer)
{
    uint8_t dataIdx = 0;
    char* dataArr[NBDATA];
    char comma[] = ",";
    char *data = strtok(buffer, comma);

    /* Split different data under format [ Name :Value] */
    /* Data are separated by commas */
    while ((data != NULL) && (dataIdx < NBDATA))
    {
        printf("# Info: UDP received data : %s\n", data);
        dataArr[dataIdx] = data;
        data = strtok(NULL, comma);
        dataIdx++;
    }

    /* For each data, split value and name */
    for (int i = 0; i < NBDATA; i++)
    {
        switch (Cfg[i].type)
        {
            case SOPC_String_Id :
                sscanf(dataArr[i], "%s :%s" , Data[i].name,  Data[i].Value.text);
                printf("# Info: Data name: %s\n", Data[i].name);
                printf("# Info: Data value: %s\n", Data[i].Value.text);
                break;

            case SOPC_Int32_Id :
                sscanf(dataArr[i], "%s :%" SCNi32, Data[i].name, &Data[i].Value.int32);
                printf("# Info: Data name: %s\n", Data[i].name);
                printf("# Info: Data value: %d\n", Data[i].Value.int32);
                break;

            case SOPC_UInt32_Id :
                sscanf(dataArr[i], "%s :%" SCNu32, Data[i].name, &Data[i].Value.uint32);
                printf("# Info: Data name: %s\n", Data[i].name);
                printf("# Info: Data value: %d\n", Data[i].Value.uint32);
                break;

            case SOPC_Int16_Id :
                sscanf(dataArr[i], "%s :%" SCNi16, Data[i].name, &Data[i].Value.int16);
                printf("# Info: Data name: %s\n", Data[i].name);
                printf("# Info: Data value: %d\n", Data[i].Value.int16);
                break;

            default :
                printf("# Info: Unexpected type in UDP configuration: %d\n", Cfg[i].type);
                break;
        }
    }
}

static void UDP_DecodeDataAndUpdateServer(char* buffer)
{
    char dataBuffer[MAXSIZE];
    char c = buffer[NBDATA];
    uint8_t nbCurlyBracket = 0;
    uint8_t buffIdx = 0;
    uint8_t startIdx = 0;
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
    } while (!dataIsolated);

    printf("# Info: UDP data part of the frame has been isolated : %s\n", dataBuffer);

    /* Decode Data name and value */
    UDP_DecodeData(dataBuffer);

    /* Update OPC UA Server nodes */
    if (!UDP_UpdateServer())
    {
        printf("# Error: UDP data have not been updated into OPC UA Server.\n");
    }
}


static void* UDP_ManageReception (void* arg)
{
    /*unused parameter */
    (void) arg;

    char buffer[MAXSIZE];
    ssize_t data_len;

    printf("# Info: UDP Reception thread running.\n");

    while (SOPC_Atomic_Int_Get(&UDP_recepRunning))
    {
        /* Wait for data reception */
        data_len = recv(recepSockfd, &buffer, MAXSIZE, MSG_DONTWAIT);

        if (data_len < 0)
        {
            SOPC_Sleep(SLEEP_TIMEOUT);
        }
        else
        {
            /* Set end of line */
            buffer[data_len] = '\0';
            printf("# Info: Received value is %s.\n", buffer);

            /* Decode received data and update OPC UA Server nodes */
            UDP_DecodeDataAndUpdateServer (buffer);
        }
    }

    /* Exit current thread */
    printf("# Info: Exit UDP reception thread.\n");
    pthread_exit(NULL);
}


static void* UDP_ManageSending (void* arg)
{
    /*unused parameter */
    (void) arg;

    printf("# Info: UDP Sending thread running.\n");

    while (SOPC_Atomic_Int_Get(&UDP_sendRunning))
    {
        // TODO
    }

    /* Exit current thread */
    printf("# Info: Exit UDP sending thread.\n");
    pthread_exit(NULL);
}


bool UDP_Start(void)
{
    bool UDP_recepOK = false;
    bool UDP_sendOK = true;

    /* Create and start reception thread */
    SOPC_Atomic_Int_Set(&UDP_recepRunning, 1);
    if (0 == pthread_create(&recepThreadId, NULL, &UDP_ManageReception, NULL))
    {
        UDP_recepOK = true;
        printf("# Info: UDP Reception thread %ld created.\n", recepThreadId);
    }

    /* Create and start sending thread */
    SOPC_Atomic_Int_Set(&UDP_sendRunning, 1);
    if (0 == pthread_create(&sendThreadId, NULL, &UDP_ManageSending, NULL))
    {
        UDP_sendOK = true;
        printf("# Info: UDP Sending thread %ld created.\n", sendThreadId);
    }

    return UDP_recepOK || UDP_sendOK;
}


void UDP_Stop(void)
{
    printf("# Info: Stop UDP Gateway.\n");
    SOPC_Atomic_Int_Set(&UDP_recepRunning, 0);
    SOPC_Atomic_Int_Set(&UDP_sendRunning, 0);

    /* Wait for threads to be reset*/
    pthread_join(recepThreadId, NULL);
    pthread_join(sendThreadId, NULL);
}


void UDP_StopAndClear(void)
{
    UDP_Stop();

    /* Clear */
    // TODO
}
