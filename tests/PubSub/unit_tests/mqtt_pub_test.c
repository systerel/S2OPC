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
#include <inttypes.h>
#include <signal.h>
#include <stdlib.h>

#include "sopc_atomic.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_macros.h"
#include "sopc_mqtt_transport_layer.h"
#include "sopc_network_layer.h"
#include "sopc_time.h"

static const char* ADRESSE_MQTT_BROKER = "127.0.0.1:1883";

/**** Variables for tests asynchronous API *****/

typedef struct T_ID
{
    volatile uint16_t idx;
    volatile bool bIsConnected;
    volatile bool bTestEnded1;
    volatile bool bTestEnded2;
} t_id;

static int cptBeforeQuit = 0;

static t_id id1 = {MQTT_INVALID_TRANSPORT_ASYNC_HANDLE, 0, false, false};
static t_id id2 = {MQTT_INVALID_TRANSPORT_ASYNC_HANDLE, 0, false, false};

static void pFctGetHandleSuccess(MqttTransportAsyncHandle idx, void* pCtx)
{
    ((t_id*) pCtx)->idx = idx;
    ((t_id*) pCtx)->bIsConnected = 0;
    printf("***** CB GET HANDLE : %d \r\n", idx);
}

static void pFctGetHandleFailure(MqttTransportAsyncHandle idx, void* pCtx)
{
    ((t_id*) pCtx)->idx = idx;
    ((t_id*) pCtx)->bIsConnected = 0;
    printf("***** CB GET HANDLE FAILURE : %d \r\n", idx);
}

static void pFctHandleRdy(MqttTransportAsyncHandle idx, void* pCtx)
{
    printf("***** CB HANDLE READY : %d \r\n", idx);
    ((t_id*) pCtx)->idx = idx;
    ((t_id*) pCtx)->bIsConnected = 1;
}

static void pFctHandleNotRdy(MqttTransportAsyncHandle idx, void* pCtx)
{
    printf("***** CB HANDLE NOT READY : %d \r\n", idx);
    ((t_id*) pCtx)->idx = idx;
    ((t_id*) pCtx)->bIsConnected = 0;
}

static void pFctMessageRcv(MqttTransportAsyncHandle idx, uint8_t* data, uint16_t size, void* pCtx)
{
    SOPC_UNUSED_ARG(idx);
    printf("***** CB RCV MSG %s : size = %d \r\n", data, size);
    if (strncmp((char*) data, "MESSAGE_CLIENT_1", size) == 0)
    {
        ((t_id*) pCtx)->bTestEnded1 = true;
    }
    if (strncmp((char*) data, "MESSAGE_CLIENT_2", size) == 0)
    {
        ((t_id*) pCtx)->bTestEnded2 = true;
    }
}
static void pFctReleaseHandle(MqttTransportAsyncHandle idx, void* pCtx)
{
    printf("***** CB RELEASE HANDLE : %d \r\n", idx);
    ((t_id*) pCtx)->idx = 0xFFFF;
    ((t_id*) pCtx)->bIsConnected = 0;
    ((t_id*) pCtx)->bTestEnded1 = false;
    ((t_id*) pCtx)->bTestEnded2 = false;
}

/**** Variables for tests of synchronous API *****/

static Thread receiveThread;
volatile bool bQuit = false;
volatile bool bResult = true;
volatile uint16_t gIdx = 0;
volatile uint16_t lIdx = 0;

static const char* expectedResultsBuffer[10] = {"MESSAGE 1", "MESSAGE 2", "MESSAGE 3", "MESSAGE 4", "MESSAGE 5",
                                                "MESSAGE 6", "MESSAGE 7", "MESSAGE 8", "MESSAGE 9", "MESSAGE 10"};

static void pFctReceiveMessage(MqttTransportHandle* pCtx, uint8_t* data, uint16_t size, void* ctx)
{
    SOPC_UNUSED_ARG(ctx);
    printf("***** SYNC GET MSG : handle %p - size = %" PRId16, (void*) pCtx, size);

    printf("\r\nMessage read = %s\r\n", data);

    if (gIdx < (sizeof(expectedResultsBuffer) / sizeof(const char*)))

        if (strncmp(expectedResultsBuffer[gIdx], (char*) data, strlen(expectedResultsBuffer[gIdx])) != 0)
        {
            printf("\r\nError, unexpected message received %s\r\n", (char*) data);
            bResult = false;
        }

    gIdx++;
}

static void* cbMessageReceived(void* pHdl)
{
    SOPC_Buffer* pBuffer;

    while (SOPC_MQTT_TRANSPORT_SYNCH_ReadMessage((MqttTransportHandle*) pHdl, &pBuffer, 500) !=
           SOPC_STATUS_INVALID_STATE)
    {
        if (pBuffer != NULL)
        {
            printf("\r\nMessage read = %s\r\n", pBuffer->data);

            if (gIdx < (sizeof(expectedResultsBuffer) / sizeof(const char*)))
            {
                if (strncmp(expectedResultsBuffer[gIdx], (char*) pBuffer->data, strlen(expectedResultsBuffer[gIdx])) !=
                    0)
                {
                    printf("\r\nError, unexpected message received %s\r\n", pBuffer->data);
                    bResult = false;
                }
            }

            gIdx++;

            SOPC_Buffer_Delete(pBuffer);
            pBuffer = NULL;
        }
        else
        {
            printf("\r\nNo messages, tmo => quit...\r\n");
            break;
        }
    }
    return NULL;
}

/*** Tests ***/

int main(void)
{
    SOPC_ReturnStatus result;
    MqttManagerHandle* pWks = NULL;
    MqttTransportHandle* pHandleSynchroContext = NULL;
    if (1)
    {
        /* Test SYNCHRONE SOPC_MQTT_TRANSPORT SYNCHRONE API without reception callback*/
        for (uint16_t i = 0; i < 1 && bResult == true; i++)
        {
            printf("\r\n Test 1-%d start : test SOPC_MQTT_TRANSPORT SYNCHRONE API without rcv callback\r\n", i);

            cptBeforeQuit = 0;
            bResult = true;
            bQuit = false;
            gIdx = 0;
            lIdx = 0;

            result = SOPC_MQTT_MGR_Create(&pWks);

            if (result == SOPC_STATUS_OK)
            {
                printf("\r\nSOPC MQTT Manager created\r\n");
                pHandleSynchroContext =
                    SOPC_MQTT_TRANSPORT_SYNCH_GetHandle(pWks, ADRESSE_MQTT_BROKER, MQTT_LIB_TOPIC_NAME, NULL, NULL);
                if (pHandleSynchroContext != NULL)
                {
                    printf("\r\nSOPC MQTT Transport Context created\r\n");
                    if (SOPC_STATUS_OK ==
                        SOPC_Thread_Create(&receiveThread, cbMessageReceived, pHandleSynchroContext, "Test thread"))
                    {
                        printf("\r\nThread test to receive message created\r\n");
                        while (bQuit == false)
                        {
                            SOPC_Sleep(100);
                            if (pHandleSynchroContext != NULL)
                            {
                                uint8_t buffer[32];
                                sprintf((char*) buffer, "MESSAGE %d", lIdx + 1);
                                result = SOPC_MQTT_TRANSPORT_SYNCH_SendMessage(
                                    pHandleSynchroContext, buffer, (uint16_t)(strlen((char*) buffer) + 1), 5000);
                                if (result == SOPC_STATUS_OK)
                                {
                                    lIdx++;
                                    cptBeforeQuit++;
                                    printf("\r\nSOPC MQTT Message %s sent\r\n", buffer);
                                }
                                else
                                {
                                    cptBeforeQuit++;
                                    printf("\r\nSOPC MQTT Message not sent, MQTT connection not ready...\r\n");
                                    bResult = false;
                                }
                            }

                            if ((cptBeforeQuit) == 20)
                            {
                                if (pHandleSynchroContext != NULL)
                                {
                                    bQuit = true;
                                    SOPC_Thread_Join(receiveThread);
                                    result = SOPC_MQTT_TRANSPORT_SYNCH_ReleaseHandle(&pHandleSynchroContext);
                                    if (result != SOPC_STATUS_OK)
                                    {
                                        printf("\r\nSOPC MQTT Transport context release error\r\n");
                                        bResult = false;
                                    }
                                    else
                                    {
                                        printf("\r\nSOPC MQTT Transport context release ok\r\n");
                                    }
                                    receiveThread = (Thread) NULL;
                                }
                            }
                        }
                    }
                    else
                    {
                        printf("\r\nTest thread of reception can't be invoked\r\n");
                        result = SOPC_MQTT_TRANSPORT_SYNCH_ReleaseHandle(&pHandleSynchroContext);
                        if (result != SOPC_STATUS_OK)
                        {
                            printf("\r\nSOPC MQTT Transport context release error\r\n");
                        }
                        else
                        {
                            printf("\r\nSOPC MQTT Transport context release ok\r\n");
                        }
                        bResult = false;
                    }
                }
                else
                {
                    printf("\r\nSOPC MQTT Transport Context creation failed\r\n");
                    bResult = false;
                }

                SOPC_MQTT_MGR_Destroy(&pWks);
                printf("\r\nSOPC MQTT MGR Destroyed\r\n");
            }
            else
            {
                printf("\r\nSOPC MQTT MGR creation failed\r\n");
                bResult = false;
            }

            if (gIdx == 0)
            {
                bResult = false;
            }

            printf("\r\n ======================== Test 1-%d result : %s\r\n", i, bResult == false ? "KO" : "OK");
        }

        /* Test SYNCHRONE SOPC_MQTT_TRANSPORT SYNCHRONE API with reception callback*/

        for (uint16_t i = 0; i < 1 && bResult == true; i++)
        {
            printf("\r\n Test 2-%d start : test SOPC_MQTT_TRANSPORT SYNCHRONE API with rcv callback\r\n", i);
            result = SOPC_MQTT_MGR_Create(&pWks);
            if (result == SOPC_STATUS_OK)
            {
                printf("\r\nSOPC MQTT Manager created\r\n");
                SOPC_MQTT_MGR_Destroy(&pWks);
                printf("\r\nSOPC MQTT Manager destroyed\r\n");
            }

            result = SOPC_MQTT_MGR_Create(&pWks);
            if (result == SOPC_STATUS_OK)
            {
                printf("\r\nSOPC MQTT Manager created\r\n");
                SOPC_MQTT_MGR_Destroy(&pWks);
                printf("\r\nSOPC MQTT Manager destroyed\r\n");
            }
            result = SOPC_MQTT_MGR_Create(&pWks);
            if (result == SOPC_STATUS_OK)
            {
                printf("\r\nSOPC MQTT Manager created\r\n");
                SOPC_MQTT_MGR_Destroy(&pWks);
                printf("\r\nSOPC MQTT Manager destroyed\r\n");
            }
            result = SOPC_MQTT_MGR_Create(&pWks);

            cptBeforeQuit = 0;
            bResult = true;
            bQuit = false;
            gIdx = 0;
            lIdx = 0;

            if (result == SOPC_STATUS_OK)
            {
                printf("\r\nSOPC MQTT Manager created\r\n");
                pHandleSynchroContext = SOPC_MQTT_TRANSPORT_SYNCH_GetHandle(
                    pWks, ADRESSE_MQTT_BROKER, MQTT_LIB_TOPIC_NAME, pFctReceiveMessage, NULL);
                if (pHandleSynchroContext != NULL)
                {
                    printf("\r\nSOPC MQTT Transport Context created\r\n");

                    while (bQuit == false)
                    {
                        SOPC_Sleep(100);
                        if (pHandleSynchroContext != NULL)
                        {
                            uint8_t buffer[32];
                            sprintf((char*) buffer, "MESSAGE %d", lIdx + 1);
                            result = SOPC_MQTT_TRANSPORT_SYNCH_SendMessage(
                                pHandleSynchroContext, buffer, (uint16_t)(strlen((char*) buffer) + 1), 5000);
                            if (result == SOPC_STATUS_OK)
                            {
                                lIdx++;
                                cptBeforeQuit++;
                                printf("\r\nSOPC MQTT Message sent\r\n");
                            }
                            else
                            {
                                cptBeforeQuit++;
                                printf("\r\nSOPC MQTT Message not sent, MQTT connection not ready...\r\n");
                                bResult = false;
                            }
                        }

                        if ((cptBeforeQuit) == 20)
                        {
                            if (pHandleSynchroContext != NULL)
                            {
                                bQuit = true;
                                result = SOPC_MQTT_TRANSPORT_SYNCH_ReleaseHandle(&pHandleSynchroContext);
                                if (result != SOPC_STATUS_OK)
                                {
                                    printf("\r\nSOPC MQTT Transport context release error\r\n");
                                    bResult = false;
                                }
                                else
                                {
                                    printf("\r\nSOPC MQTT Transport context release ok\r\n");
                                }
                            }
                            else
                            {
                                bResult = false;
                            }
                        }
                    }
                }
                else
                {
                    printf("\r\nSOPC MQTT Transport Context creation failed\r\n");
                    bResult = false;
                }

                printf("\r\nSOPC MQTT MGR Destruction\r\n");
                SOPC_MQTT_MGR_Destroy(&pWks);
                pWks = NULL;
            }
            else
            {
                printf("\r\nSOPC MQTT MGR creation failed\r\n");
                bResult = false;
            }

            if (gIdx == 0)
            {
                bResult = false;
            }

            printf("\r\n ======================== Test 2-%d result : %s\r\n", i, (bResult == false) ? "KO" : "OK");
        }
    }

    if (1)
    {
        /* Test ASYNCHRONE SOPC_MQTT_TRANSPORT SYNCHRONE API with callbacks*/
        for (uint16_t i = 0; i < 1 && bResult == true; i++)
        {
            printf("\r\n Test 3-%d start - SOPC_MQTT_TRANSPORT ASYNCHRONE with 2 clients which sending a message \r\n",
                   i);
            bResult = false;
            result = SOPC_MQTT_MGR_Create(&pWks);
            cptBeforeQuit = 0;
            bQuit = false;

            if (result == SOPC_STATUS_OK)
            {
                SOPC_MQTT_TRANSPORT_ASYNC_GetHandle(pWks, &id1, ADRESSE_MQTT_BROKER, MQTT_LIB_TOPIC_NAME,
                                                    pFctGetHandleSuccess, pFctGetHandleFailure, pFctHandleRdy,
                                                    pFctHandleNotRdy, pFctMessageRcv, pFctReleaseHandle);

                SOPC_MQTT_TRANSPORT_ASYNC_GetHandle(pWks, &id2, ADRESSE_MQTT_BROKER, MQTT_LIB_TOPIC_NAME,
                                                    pFctGetHandleSuccess, pFctGetHandleFailure, pFctHandleRdy,
                                                    pFctHandleNotRdy, pFctMessageRcv, pFctReleaseHandle);

                while (bQuit == false)
                {
                    SOPC_Sleep(100);
                    if (pWks != NULL)
                    {
                        if (id1.idx < MQTT_INVALID_TRANSPORT_ASYNC_HANDLE)
                        {
                            if (id1.bIsConnected)
                            {
                                result = SOPC_MQTT_TRANSPORT_ASYNC_SendMessage(
                                    pWks, id1.idx, (uint8_t*) "MESSAGE_CLIENT_1", strlen("MESSAGE_CLIENT_1") + 1);
                                if (result == SOPC_STATUS_OK)
                                {
                                    printf("\r\nMessage sent to client 1\r\n");
                                }
                                else
                                {
                                    printf("\r\nMessage not sent to client 1\r\n");
                                }
                            }
                        }

                        if (id2.idx < MQTT_INVALID_TRANSPORT_ASYNC_HANDLE)
                        {
                            if (id2.bIsConnected)
                            {
                                result = SOPC_MQTT_TRANSPORT_ASYNC_SendMessage(
                                    pWks, id2.idx, (uint8_t*) "MESSAGE_CLIENT_2", strlen("MESSAGE_CLIENT_2") + 1);
                                if (result == SOPC_STATUS_OK)
                                {
                                    printf("\r\nMessage sent to client 2\r\n");
                                }
                                else
                                {
                                    printf("\r\nMessage not sent to client 2\r\n");
                                }
                            }
                        }
                    }

                    cptBeforeQuit++;

                    if (id1.bTestEnded1 && id2.bTestEnded1 && id1.bTestEnded2 && id2.bTestEnded2)
                    {
                        bResult = true;
                        cptBeforeQuit = 20;
                    }

                    if (cptBeforeQuit == 20)
                    {
                        SOPC_MQTT_MGR_Destroy(&pWks);
                        bQuit = true;
                    }
                }
            }

            printf("\r\n ======================== Test 3-%d result = %s \r\n", i, bResult == true ? "OK" : "KO");
        }
    }

    if (bResult == true)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}
