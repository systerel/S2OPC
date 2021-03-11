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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sopc_atomic.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_threads.h"
#include "sopc_time.h"
#include "sopc_types.h"

#include "sopc_interrupttimer.h"
#include "sopc_msgbox.h"
#include "sopc_rt_publisher.h"
#include "sopc_rt_subscriber.h"

//#define DEBUG_RT_TEST

/*Timer test variables*/

char* chaine2 = "ABABABABABABABABABABA";
char* chaine3 = "ZCZCZCZCZCZC";

volatile uint32_t bTest01_TestStart[2] = {0, 0};
volatile uint32_t bTest02_TestPeriod[2] = {0, 0};
volatile uint32_t bTest03_TestStop[2] = {0, 0};
volatile uint32_t bTest04_TestCorrupted[2] = {0, 0};
volatile bool bFreezeTickRequest = false;
volatile uint32_t globalTick = 0;
volatile bool bQuit = false;

static void cbStart(uint32_t timerId, void* pUserContext);
static void cbStop(uint32_t timerId, void* pUserContext);
static void cbElapsed(uint32_t timerId, void* pUserContext, void* pData, uint32_t size);

SOPC_InterruptTimer* myTimer = NULL;
Thread handleThreadUpdate = (Thread) NULL;
static void cbStart(uint32_t timerId, void* pUserContext)
{
    (void) pUserContext;
    if (timerId < 2)
    {
        bTest01_TestStart[timerId]++;
    }
#ifdef DEBUG_RT_TEST
    printf("\r\nTIMER %d STARTED ", timerId);
#endif
}
static void cbStop(uint32_t timerId, void* pUserContext)
{
    (void) pUserContext;
    if (timerId < 2)
    {
        bTest03_TestStop[timerId]++;
    }
#ifdef DEBUG_RT_TEST
    printf("\r\nTIMER %d STOPPED\r\n", timerId);
#endif
}
static void cbElapsed(uint32_t timerId, void* pUserContext, void* pData, uint32_t size)
{
    (void) pUserContext;
    (void) pData;
    (void) size;

    bTest02_TestPeriod[timerId]++;

    if (size > 0)
    {
#ifdef DEBUG_RT_TEST
        printf("\r\nTIMER %d ELAPSED %d with DATA = %s\r\n", timerId, bTest02_TestPeriod[timerId], (char*) pData);
#endif
        if (strcmp(chaine2, (char*) pData) != 0 && strcmp(chaine3, (char*) pData) != 0)
        {
            bTest04_TestCorrupted[timerId]++;
        }
    }
    else
    {
#ifdef DEBUG_RT_TEST
        printf("\r\nTIMER %d ELAPSED %d with NO DATA\r\n", timerId, bTest02_TestPeriod[timerId]);
#endif
    }
}

static void* cbUpdate(void* arg)
{
    (void) arg;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    while (!bQuit && result == SOPC_STATUS_OK)
    {
        if (bFreezeTickRequest)
        {
            SOPC_Sleep(100);
            globalTick++;
            result = SOPC_InterruptTimer_Update(myTimer, globalTick);
            if (result != SOPC_STATUS_OK)
            {
#ifdef DEBUG_RT_TEST
                printf("\r\n===>ERROR INTERRUPT TIMER UPDATE\r\n");
#endif
            }
        }
        else
        {
            {
                SOPC_Sleep(100);
            }
        }
    }
    return NULL;
}

/*Message box test variables*/

#define PERIOD_MS_READER_50MS 50
#define PERIOD_MS_READER_10MS 10
#define PERIOD_MS_READER_200MS 200
#define PERIOD_MS_WRITER_50MS 50
#define PERIOD_MS_WRITER_200MS 200

SOPC_MsgBox* myMsgBox;
Thread handleThreadReadMsgBox1 = (Thread) NULL;
Thread handleThreadReadMsgBox2 = (Thread) NULL;
volatile int cptMsgRead1 = 0;
volatile int cptMsgRead2 = 0;
volatile int cptMsgRead3 = 0;
volatile int period = PERIOD_MS_READER_200MS;
volatile SOPC_MsgBox_Mode mode = SOPC_MSGBOX_MODE_GET_NORMAL;
char msgRead1[256] = {0};
char msgRead2[256] = {0};
char msgRead3[256] = {0};
char msgCmpRead1[256] = {0};
char msgCmpRead2[256] = {0};
char msgCmpRead3[256] = {0};
char msgWrite1[256] = {0};
char msgWrite2[256] = {0};
char msgWrite3[256] = {0};
volatile int cptMsgDiffRead1 = 0;
volatile int cptMsgDiffRead2 = 0;
volatile bool bFreeze = true;

/*Callback reception msg box*/
static void* cbmsgBoxRead1(void* arg);
static void* cbmsgBoxRead2(void* arg);

static void* cbmsgBoxRead1(void* arg)
{
    (void) arg;
    size_t id = 0;
    uint8_t* pData = NULL;
    uint32_t size = 0;

    uint32_t nbPendingsEvent = 0;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    while (!bQuit)
    {
        if (!bFreeze)
        {
            SOPC_MsgBox_Pop_Initialize(myMsgBox, &id);
        repeat: /* TODO: remove goto */
            if (mode ==
                SOPC_MSGBOX_MODE_GET_NORMAL) /* TODO: switch case + use mode if it contains the correct value... */
            {
                result = SOPC_MsgBox_Pop_GetEvtPtr(myMsgBox, id, 0, &pData, &size, &nbPendingsEvent,
                                                   SOPC_MSGBOX_MODE_GET_NORMAL);
            }
            else if (mode == SOPC_MSGBOX_MODE_GET_NEW_LAST)
            {
                result = SOPC_MsgBox_Pop_GetEvtPtr(myMsgBox, id, 0, &pData, &size, &nbPendingsEvent,
                                                   SOPC_MSGBOX_MODE_GET_NEW_LAST);
            }
            else if (mode == SOPC_MSGBOX_MODE_GET_LAST)
            {
                result = SOPC_MsgBox_Pop_GetEvtPtr(myMsgBox, id, 0, &pData, &size, &nbPendingsEvent,
                                                   SOPC_MSGBOX_MODE_GET_LAST);
            }
            if ((pData != NULL) && (result == SOPC_STATUS_OK) && (size > 0))
            {
#ifdef DEBUG_RT_TEST
                printf("%s ==> thread 1 - size = %d - nb pending = %d - nb msg = %d - diff = %d", (const char*) pData,
                       size, nbPendingsEvent, cptMsgRead1, cptMsgDiffRead1);
#endif

                if (mode == SOPC_MSGBOX_MODE_GET_NORMAL)
                {
                    sprintf(msgRead1, "\r\nHello world %d", cptMsgRead1);
                    if (strcmp((const char*) pData, (const char*) msgRead1) == 0)
                    {
                        cptMsgRead1++;
                    }
                }
                else
                {
                    cptMsgRead1++;
                }

                if (mode < SOPC_MSGBOX_MODE_GET_LAST)
                {
                    goto repeat;
                }
                else
                {
                    if (strcmp((const char*) pData, (const char*) msgRead1) != 0)
                    {
                        cptMsgDiffRead1++;
                    }
                    strcpy((char*) msgRead1, (const char*) pData);
                }
            }
            SOPC_MsgBox_Pop_Finalize(myMsgBox, &id);
        }

        switch (period)
        {
        case PERIOD_MS_READER_10MS:
            SOPC_Sleep(PERIOD_MS_READER_10MS);
            break;
        case PERIOD_MS_READER_50MS:
            SOPC_Sleep(PERIOD_MS_READER_50MS);
            break;
        case PERIOD_MS_READER_200MS:
            SOPC_Sleep(PERIOD_MS_READER_200MS);
            break;
        default:
            assert(false && "Unexpected period.");
            break;
        }
    }
    return NULL;
}

static void* cbmsgBoxRead2(void* arg)
{
    /* TODO: remove copy paste and factorize these functions */
    (void) arg;
    size_t id = 0;
    uint8_t* pData = NULL;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    uint32_t size = 0;

    uint32_t nbPendingsEvent = 0;

    while (!bQuit)
    {
        if (!bFreeze)
        {
            SOPC_MsgBox_Pop_Initialize(myMsgBox, &id);
        repeat:
            if (mode == SOPC_MSGBOX_MODE_GET_NORMAL)
            {
                result = SOPC_MsgBox_Pop_GetEvtPtr(myMsgBox, id, 1, &pData, &size, &nbPendingsEvent,
                                                   SOPC_MSGBOX_MODE_GET_NORMAL);
            }
            else if (mode == SOPC_MSGBOX_MODE_GET_NEW_LAST)
            {
                result = SOPC_MsgBox_Pop_GetEvtPtr(myMsgBox, id, 1, &pData, &size, &nbPendingsEvent,
                                                   SOPC_MSGBOX_MODE_GET_NEW_LAST);
            }
            else if (mode == SOPC_MSGBOX_MODE_GET_LAST)
            {
                result = SOPC_MsgBox_Pop_GetEvtPtr(myMsgBox, id, 1, &pData, &size, &nbPendingsEvent,
                                                   SOPC_MSGBOX_MODE_GET_LAST);
            }
            if ((pData != NULL) && (result == SOPC_STATUS_OK) && (size > 0))
            {
#ifdef DEBUG_RT_TEST
                printf("%s ==> thread 2 - size = %d - nb pending = %d - nb msg = %d - diff = %d", (const char*) pData,
                       size, nbPendingsEvent, cptMsgRead2, cptMsgDiffRead2);
#endif
                if (mode == SOPC_MSGBOX_MODE_GET_NORMAL)
                {
                    sprintf(msgRead2, "\r\nHello world %d", cptMsgRead2);
                    if (strcmp((const char*) pData, (const char*) msgRead2) == 0)
                    {
                        cptMsgRead2++;
                    }
                }
                else
                {
                    cptMsgRead2++;
                }

                if (mode < SOPC_MSGBOX_MODE_GET_LAST)
                {
                    goto repeat;
                }
                else
                {
                    if (strcmp((const char*) pData, (const char*) msgRead2) != 0)
                    {
                        cptMsgDiffRead2++;
                    }
                    strcpy((char*) msgRead2, (const char*) pData);
                }
            }
            SOPC_MsgBox_Pop_Finalize(myMsgBox, &id);
        }

        switch (period)
        {
        case PERIOD_MS_READER_10MS:
            SOPC_Sleep(PERIOD_MS_READER_10MS);
            break;
        case PERIOD_MS_READER_50MS:
            SOPC_Sleep(PERIOD_MS_READER_50MS);
            break;
        case PERIOD_MS_READER_200MS:
            SOPC_Sleep(PERIOD_MS_READER_200MS);
            break;
        default:
            assert(false && "Unexpected period.");
            break;
        }
    }
    return NULL;
}

/**** RT Publisher test variables ***/

SOPC_RT_Publisher* myRTPub = NULL;
SOPC_RT_Publisher_Initializer* myRTPubInitializer = NULL;
Thread handleThreadBeatHeartRTPub = (Thread) NULL;
uint32_t idMsg1 = 0;
uint32_t idMsg2 = 0;
uint32_t idMsg3 = 0;
char* msg1Ctx = "CONTEXT 1";
char* msg2Ctx = "CONTEXT 2";
char* msg3Ctx = "CONTEXT 3";
volatile SOPC_ReturnStatus resultUpdateThread = SOPC_STATUS_NOK;

static void cbStartPubMsg(uint32_t msgId,     // Message instance identifier
                          void* pUserContext) // User context
{
    printf("\r\nMsg id = %u - User context %s - Started\r\n", msgId, (char*) pUserContext);
}

// Stop callback, called when timer switch from ENABLED to DISABLED
static void cbStopPubMsg(uint32_t msgId,     // Message instance identifier
                         void* pUserContext) // User context
{
    printf("\r\nMsg id = %u - User context %s - Stopped\r\n", msgId, (char*) pUserContext);
}

// Elapsed callback, called when timer reach its configured period
static void cbSendPubMsg(uint32_t msgId,     // Message instance identifier
                         void* pUserContext, // User context
                         void* pData,        // Data published by set data API
                         uint32_t size)      // Data size in bytes
{
    (void) pUserContext;
    (void) size;
    if (msgId == 0)
    {
        if (strcmp(msgRead1, pData) != 0)
        {
            sprintf(msgCmpRead1, "Hello world %d for msg 1 :)", cptMsgRead1);
            if (strcmp(msgCmpRead1, pData) == 0)
            {
                cptMsgRead1++;
            }
            strncpy(msgRead1, pData, 255);
        }
    }

    if (msgId == 1)
    {
        if (strcmp(msgRead2, pData) != 0)
        {
            sprintf(msgCmpRead2, "Hello world %d for msg 2 :)", cptMsgRead2);
            if (strcmp(msgCmpRead2, pData) == 0)
            {
                cptMsgRead2++;
            }
            strncpy(msgRead2, pData, 255);
        }
    }

    if (msgId == 2)
    {
        if (strcmp(msgRead3, pData) != 0)
        {
            sprintf(msgCmpRead3, "Hello world %d for msg 3 :)", cptMsgRead3);
            if (strcmp(msgCmpRead3, pData) == 0)
            {
                cptMsgRead3++;
            }
            strncpy(msgRead3, pData, 255);
        }
    }
#ifdef DEBUG_RT_TEST
    printf(
        "\r\nMsg id = %u - User context %s - data = %s - size = %u - msgCounter1 = %d - msgCounter2 = %d - msgCounter3 "
        "= %d\r\n",
        msgId, (char*) pUserContext, (char*) pData, size, cptMsgRead1, cptMsgRead2, cptMsgRead3);
#endif
}

static void* cbBeatHeart(void* arg)
{
    (void) arg;

    static volatile uint32_t beatHeart = 0;
#ifdef DEBUG_RT_TEST
    printf("\r\n==>RT_PUBLISHER : Beat heart thread launched !!!\r\n");
#endif
    while (!bQuit)
    {
        resultUpdateThread = SOPC_RT_Publisher_HeartBeat(myRTPub, beatHeart++);
        if (resultUpdateThread != SOPC_STATUS_OK)
        {
#ifdef DEBUG_RT_TEST
            printf("\r\n==>RT_PUBLISHER : SOPC_RT_Publisher_BeatHeart error = %d\r\n", resultUpdateThread);
#endif
        }

        SOPC_Sleep(10);
    }
    return NULL;
}

/**** RT Subscriber test variables ***/

SOPC_RT_Subscriber_Initializer* myRTSubInit = NULL;
SOPC_RT_Subscriber* myRTSub = NULL;
Thread handleThreadBeatHeatSubscriber = (Thread) NULL;
Thread handleThreadOutputReader1 = (Thread) NULL;
Thread handleThreadOutputReader2 = (Thread) NULL;
uint32_t input_num[1] = {0};
uint32_t output_num[2] = {0};

static SOPC_ReturnStatus cbBeatHeartCallback(SOPC_RT_Subscriber* pSub,
                                             void* pContext,
                                             void* pInputContext,
                                             uint32_t input_number,
                                             uint8_t* pData,
                                             uint32_t size)
{
    (void) pSub;
    (void) pContext;
    (void) input_number;
    (void) pData;
    (void) size;
    (void) pInputContext;

    char msgModified[256];

    SOPC_ReturnStatus result = SOPC_STATUS_NOK;
#ifdef DEBUG_RT_TEST
    printf("\r\nRT SUBSCRIBER INTERNAL | INPUT PIN %d : size = %u chaine = %s  \r\n", input_number, size,
           (char*) pData);
#endif
    // Forward to output

    sprintf(msgModified, "Forward on output 1 : %s", pData);
    result = SOPC_RT_Subscriber_Output_Write(pSub, 0, (uint8_t*) msgModified, (uint32_t)(strlen(msgModified) + 1));

    if (SOPC_STATUS_OK != result)
    {
#ifdef DEBUG_RT_TEST
        printf("\r\nRT SUBSCRIBER INTERNAL ERROR : %u\r\n", result);
#endif
    }

    if (SOPC_STATUS_OK == result)
    {
        sprintf(msgModified, "Forward on output 2 : %s", pData);
        result = SOPC_RT_Subscriber_Output_Write(pSub, 1, (uint8_t*) msgModified, (uint32_t)(strlen(msgModified) + 1));
    }
    if (SOPC_STATUS_OK != result)
    {
#ifdef DEBUG_RT_TEST
        printf("\r\nRT SUBSCRIBER INTERNAL ERROR : %u\r\n", result);
#endif
    }

    return result;
}

static void* cbBeatHeartThreadCallback(void* arg)
{
    (void) arg;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    while (!bQuit)
    {
        result = SOPC_RT_Subscriber_HeartBeat(myRTSub);
        if (SOPC_STATUS_OK != result)
        {
#ifdef DEBUG_RT_TEST
            printf("\r\nRT SUBSCRIBER BEAT HEART ERROR : %u\r\n", result);
#endif
        }

        SOPC_Sleep(200);
    }

    return NULL;
}

static void* cbSubscriberReaderCallback(void* arg)
{
    (void) arg;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    uint32_t size = 0;
    uint8_t* pData = NULL;

    uint32_t clientId = *((uint32_t*) arg);
    size_t token[2] = {0};
    uint32_t cpt[2] = {0};

    while (!bQuit)
    {
        result = SOPC_RT_Subscriber_Output_Read_Initialize(myRTSub, output_num[0], &token[0]);

        if (result == SOPC_STATUS_OK)
        {
            result = SOPC_RT_Subscriber_Output_Read_Initialize(myRTSub, output_num[1], &token[1]);
        }

        if (SOPC_STATUS_OK == result)
        {
            for (uint32_t i = 0; i < 2; i++)
            {
                do
                {
                    result = SOPC_RT_Subscriber_Output_Read(myRTSub,                  //
                                                            output_num[i],            //
                                                            clientId,                 //
                                                            token[i],                 //
                                                            SOPC_PIN_MODE_GET_NORMAL, //
                                                            &pData,                   //
                                                            &size                     //
                    );                                                                //

                    if (pData != NULL && size > 0 && SOPC_STATUS_OK == result)
                    {
                        switch (clientId)
                        {
                        case 1:
                            sprintf(msgCmpRead2, "Forward on output %u : %s %u", i + 1, "Hello world", cpt[i]++);
                            if (strcmp((char*) pData, msgCmpRead2) == 0)
                            {
                                cptMsgRead2++;
                            }
#ifdef DEBUG_RT_TEST
                            printf("\r\nRT SUBSCRIBER | READER | success = %d | pin num = %d | client %u read %s\r\n",
                                   cptMsgRead2, i, clientId, (char*) pData);
#endif
                            break;
                        case 0:
                            sprintf(msgCmpRead1, "Forward on output %u : %s %u", i + 1, "Hello world", cpt[i]++);
                            if (strcmp((char*) pData, msgCmpRead1) == 0)
                            {
                                cptMsgRead1++;
                            }
#ifdef DEBUG_RT_TEST
                            printf("\r\nRT SUBSCRIBER | READER | success = %d | pin num = %d | client %u read %s\r\n",
                                   cptMsgRead1, i, clientId, (char*) pData);
#endif
                            break;
                        default:
                            assert(false && "unknown client id");
                            break;
                        }
                    }
                } while (SOPC_STATUS_OK == result && size > 0 && pData != NULL);
            }
            SOPC_RT_Subscriber_Output_Read_Finalize(myRTSub, output_num[0], &token[0]);
            SOPC_RT_Subscriber_Output_Read_Finalize(myRTSub, output_num[1], &token[1]);
        }
        SOPC_Sleep(400);
    }

    return NULL;
}

static int SOPC_TEST_RT_SUBSCRIBER(void)
{
    int res = 0;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    bQuit = false;
    uint32_t clientId1 = 0;
    uint32_t clientId2 = 1;
    cptMsgRead2 = 0;
    cptMsgRead1 = 0;

    printf("\r\nRT SUBSCRIBER CREATION ...\r\n");
    myRTSub = SOPC_RT_Subscriber_Create();

    if (myRTSub == NULL)
    {
        result = SOPC_STATUS_NOK;
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZER CREATION ...\r\n");
        myRTSubInit = SOPC_RT_Subscriber_Initializer_Create(cbBeatHeartCallback, NULL);
        if (myRTSubInit == NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZER ADD INPUT ...\r\n");
        result = SOPC_RT_Subscriber_Initializer_AddInput(myRTSubInit, 16, 256, SOPC_PIN_MODE_GET_NORMAL, NULL,
                                                         &input_num[0]);
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZER ADD OUTPUT ...\r\n");
        result = SOPC_RT_Subscriber_Initializer_AddOutput(myRTSubInit, 2, 16, 256, &output_num[0]);
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZER ADD OUTPUT ...\r\n");
        result = SOPC_RT_Subscriber_Initializer_AddOutput(myRTSubInit, 2, 16, 256, &output_num[1]);
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\nRT SUBSCRIBER : Beat heart thread launch\r\n");
        // Thread slept each 400ms
        SOPC_Thread_Create(&handleThreadBeatHeatSubscriber, cbBeatHeartThreadCallback, NULL, "Test thread 1");
        if (handleThreadBeatHeatSubscriber == (Thread) NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\nRT SUBSCRIBER : Reader 0\r\n");
        // Thread slept each 400ms
        SOPC_Thread_Create(&handleThreadOutputReader1, cbSubscriberReaderCallback, &clientId1, "Test read 0");
        if (handleThreadOutputReader1 == (Thread) NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\nRT SUBSCRIBER : Reader 1\r\n");
        // Thread slept each 10ms
        SOPC_Thread_Create(&handleThreadOutputReader2, cbSubscriberReaderCallback, &clientId2, "Test read 1");
        if (handleThreadOutputReader2 == (Thread) NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    printf("\r\nRT SUBSCRIBER : Wait 1 s before init\r\n");
    SOPC_Sleep(1000);

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZATION ...\r\n");
        result = SOPC_RT_Subscriber_Initialize(myRTSub, myRTSubInit);
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER TEST ...\r\n");
        for (volatile uint32_t i = 0; (i < 50) && (SOPC_STATUS_OK == result); i++)
        {
            sprintf(msgWrite1, "Hello world %u", i);
            result = SOPC_RT_Subscriber_Input_Write(myRTSub,                            //
                                                    input_num[0],                       //
                                                    (uint8_t*) msgWrite1,               //
                                                    (uint32_t)(strlen(msgWrite1) + 1)); //
            SOPC_Sleep(100);
        }
    }

    SOPC_Sleep(1000);

    while (SOPC_RT_Subscriber_DeInitialize(myRTSub) == SOPC_STATUS_INVALID_STATE)
    {
        printf("\r\nRT SUBSCRIBER DE INITIALIZATION FAILED, RETRY\r\n");
        SOPC_Sleep(10);
    }

    if (!(cptMsgRead1 == cptMsgRead2 && cptMsgRead1 == 100))
    {
        result = SOPC_STATUS_NOK;
    }

    bQuit = true;

    if (handleThreadBeatHeatSubscriber != (Thread) NULL)
    {
        SOPC_Thread_Join(handleThreadBeatHeatSubscriber);
        handleThreadBeatHeatSubscriber = (Thread) NULL;
    }
    if (handleThreadOutputReader1 != (Thread) NULL)
    {
        SOPC_Thread_Join(handleThreadOutputReader1);
        handleThreadOutputReader1 = (Thread) NULL;
    }
    if (handleThreadOutputReader2 != (Thread) NULL)
    {
        SOPC_Thread_Join(handleThreadOutputReader2);
        handleThreadOutputReader2 = (Thread) NULL;
    }

    SOPC_RT_Subscriber_Initializer_Destroy(&myRTSubInit);
    SOPC_RT_Subscriber_Destroy(&myRTSub);

    if (result == SOPC_STATUS_NOK)
    {
        res = -1;
    }
    else
    {
        res = 0;
    }

    printf("\r\nRT SUBSCRIBER TEST RESULT = %d\r\n", res);

    return res;
}

static int SOPC_TEST_RT_SUBSCRIBER_WITH_DATAHANDLE_WRITER(void)
{
    int res = 0;
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    bQuit = false;
    uint32_t clientId1 = 0;
    uint32_t clientId2 = 1;
    cptMsgRead2 = 0;
    cptMsgRead1 = 0;

    printf("\r\nRT SUBSCRIBER CREATION ...\r\n");
    myRTSub = SOPC_RT_Subscriber_Create();

    if (myRTSub == NULL)
    {
        result = SOPC_STATUS_NOK;
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZER CREATION ...\r\n");
        myRTSubInit = SOPC_RT_Subscriber_Initializer_Create(cbBeatHeartCallback, NULL);
        if (myRTSubInit == NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZER ADD INPUT ...\r\n");
        result = SOPC_RT_Subscriber_Initializer_AddInput(myRTSubInit, 16, 256, SOPC_PIN_MODE_GET_NORMAL, NULL,
                                                         &input_num[0]);
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZER ADD OUTPUT ...\r\n");
        result = SOPC_RT_Subscriber_Initializer_AddOutput(myRTSubInit, 2, 16, 256, &output_num[0]);
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZER ADD OUTPUT ...\r\n");
        result = SOPC_RT_Subscriber_Initializer_AddOutput(myRTSubInit, 2, 16, 256, &output_num[1]);
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\nRT SUBSCRIBER : Beat heart thread launch\r\n");
        // Thread slept each 400ms
        SOPC_Thread_Create(&handleThreadBeatHeatSubscriber, cbBeatHeartThreadCallback, NULL, "Test thread 1");
        if (handleThreadBeatHeatSubscriber == (Thread) NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\nRT SUBSCRIBER : Reader 0\r\n");
        // Thread slept each 400ms
        SOPC_Thread_Create(&handleThreadOutputReader1, cbSubscriberReaderCallback, &clientId1, "Test read 0");
        if (handleThreadOutputReader1 == (Thread) NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\nRT SUBSCRIBER : Reader 1\r\n");
        // Thread slept each 10ms
        SOPC_Thread_Create(&handleThreadOutputReader2, cbSubscriberReaderCallback, &clientId2, "Test read 1");
        if (handleThreadOutputReader2 == (Thread) NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    printf("\r\nRT SUBSCRIBER : Wait 1 s before init\r\n");
    SOPC_Sleep(1000);

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER INITIALIZATION ...\r\n");
        result = SOPC_RT_Subscriber_Initialize(myRTSub, myRTSubInit);
    }

    if (result == SOPC_STATUS_OK)
    {
        printf("\r\nRT SUBSCRIBER TEST WITH WRITE DATA HANDLE ...\r\n");

        for (volatile uint32_t i = 0; (i < 50) && (SOPC_STATUS_OK == result); i++)
        {
            // Create SOPC Buffer structure with data pointer set to NULL
            SOPC_Buffer buffer;
            memset(&buffer, 0, sizeof(SOPC_Buffer));

            // Initialize the data pointer and max allowed data
            result = SOPC_RT_Subscriber_Input_WriteDataHandle_GetBuffer(myRTSub,      //
                                                                        input_num[0], //
                                                                        &buffer);     //
            if (SOPC_STATUS_OK == result)
            {
                // If successful, work on the buffer
                sprintf(msgWrite1, "Hello world %u", i);
                result = SOPC_Buffer_Write(&buffer, (const uint8_t*) msgWrite1, (uint32_t)(strlen(msgWrite1) + 1));
                // printf("\r\nSOPC_Buffer_Write result = %d\r\n",result);
                //                result = SOPC_RT_Subscriber_Input_Write(myRTSub,                            //
                //                        input_num[0],                       //
                //                        (uint8_t*) msgWrite1,               //
                //                        (uint32_t)(strlen(msgWrite1) + 1)); //

                // Shall be always released.
                SOPC_RT_Subscriber_Input_WriteDataHandle_ReleaseBuffer(myRTSub, input_num[0], &buffer);
            }

            SOPC_Sleep(100);
        }
    }

    SOPC_Sleep(1000);

    while (SOPC_RT_Subscriber_DeInitialize(myRTSub) == SOPC_STATUS_INVALID_STATE)
    {
        printf("\r\nRT SUBSCRIBER DE INITIALIZATION FAILED, RETRY\r\n");
        SOPC_Sleep(10);
    }

    if (!(cptMsgRead1 == cptMsgRead2 && cptMsgRead1 == 100))
    {
        result = SOPC_STATUS_NOK;
    }

    bQuit = true;

    if (handleThreadBeatHeatSubscriber != (Thread) NULL)
    {
        SOPC_Thread_Join(handleThreadBeatHeatSubscriber);
        handleThreadBeatHeatSubscriber = (Thread) NULL;
    }
    if (handleThreadOutputReader1 != (Thread) NULL)
    {
        SOPC_Thread_Join(handleThreadOutputReader1);
        handleThreadOutputReader1 = (Thread) NULL;
    }
    if (handleThreadOutputReader2 != (Thread) NULL)
    {
        SOPC_Thread_Join(handleThreadOutputReader2);
        handleThreadOutputReader2 = (Thread) NULL;
    }

    SOPC_RT_Subscriber_Initializer_Destroy(&myRTSubInit);
    SOPC_RT_Subscriber_Destroy(&myRTSub);

    if (result == SOPC_STATUS_NOK)
    {
        res = -1;
    }
    else
    {
        res = 0;
    }

    printf("\r\nRT SUBSCRIBER TEST RESULT = %d\r\n", res);

    return res;
}

/*** ***/

static int SOPC_TEST_RT_PUBLISHER(void)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    int res = 0;
    bQuit = false;
    bFreeze = true;

    memset(msgRead1, 0, sizeof(msgRead1));
    memset(msgRead2, 0, sizeof(msgRead1));
    cptMsgRead1 = 0;
    cptMsgRead2 = 0;

    // Create initializer

    printf("\r\n==>RT_PUBLISHER : Initializer creation\r\n");
    myRTPubInitializer = SOPC_RT_Publisher_Initializer_Create(1024);

    if (myRTPubInitializer == NULL)
    {
        result = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\n==>RT_PUBLISHER : Initializer add msg1\r\n");
        // Add a message
        result = SOPC_RT_Publisher_Initializer_AddMessage(myRTPubInitializer,                       //
                                                          10,                                       // 100ms
                                                          0,                                        //
                                                          msg1Ctx,                                  //
                                                          cbStartPubMsg,                            //
                                                          cbSendPubMsg,                             //
                                                          cbStopPubMsg,                             //
                                                          SOPC_RT_PUBLISHER_MSG_PUB_STATUS_ENABLED, //
                                                          &idMsg1);                                 //
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\n==>RT_PUBLISHER : Initializer add msg2\r\n");
        // Add a message
        result = SOPC_RT_Publisher_Initializer_AddMessage(myRTPubInitializer,                       //
                                                          50,                                       // 500ms
                                                          0,                                        //
                                                          msg2Ctx,                                  //
                                                          cbStartPubMsg,                            //
                                                          cbSendPubMsg,                             //
                                                          cbStopPubMsg,                             //
                                                          SOPC_RT_PUBLISHER_MSG_PUB_STATUS_ENABLED, //
                                                          &idMsg2);                                 //
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\n==>RT_PUBLISHER : Initializer add msg2\r\n");
        // Add a message
        result = SOPC_RT_Publisher_Initializer_AddMessage(myRTPubInitializer,                       //
                                                          25,                                       // 250ms
                                                          0,                                        //
                                                          msg3Ctx,                                  //
                                                          cbStartPubMsg,                            //
                                                          cbSendPubMsg,                             //
                                                          cbStopPubMsg,                             //
                                                          SOPC_RT_PUBLISHER_MSG_PUB_STATUS_ENABLED, //
                                                          &idMsg3);                                 //
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\n==>RT_PUBLISHER : Publisher creation\r\n");
        // Create RT publisher
        myRTPub = SOPC_RT_Publisher_Create();

        if (myRTPub == NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\n==>RT_PUBLISHER : Beat heart thread launch\r\n");
        // Thread slept each 10ms
        SOPC_Thread_Create(&handleThreadBeatHeartRTPub, cbBeatHeart, NULL, "Test thread 1");
        if (handleThreadBeatHeartRTPub == (Thread) NULL)
        {
            result = SOPC_STATUS_NOK;
        }
    }

    SOPC_Sleep(200);

    if (resultUpdateThread != SOPC_STATUS_INVALID_STATE)
    {
        result = SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == result)
    {
        printf("\r\n==>RT_PUBLISHER : Initialize\r\n");
        result = SOPC_RT_Publisher_Initialize(myRTPub, myRTPubInitializer);
    }

    SOPC_Sleep(200);

    if (resultUpdateThread != SOPC_STATUS_OK || result != SOPC_STATUS_OK)
    {
        result = SOPC_STATUS_NOK;
    }
    else
    {
        printf("\r\n==>RT_PUBLISHER : Start message modification\r\n");
    }

    const int loopNb = 5;
    for (volatile int i = 0; i < loopNb && result == SOPC_STATUS_OK; i++)
    {
#ifdef DEBUG_RT_TEST
        printf("\r\n==>RT_PUBLISHER : Modify message 1 value\r\n");
#endif
        sprintf(msgWrite1, "Hello world %d for msg 1 :)", i);
        result = SOPC_RT_Publisher_SetMessageValue(myRTPub,                            //
                                                   idMsg1,                             //
                                                   (uint8_t*) msgWrite1,               //
                                                   (uint32_t)(strlen(msgWrite1) + 1)); //
#ifdef DEBUG_RT_TEST
        printf("\r\n==>RT_PUBLISHER : Modify message 2 value\r\n");
#endif
        sprintf(msgWrite2, "Hello world %d for msg 2 :)", i);
        result = SOPC_RT_Publisher_SetMessageValue(myRTPub,                            //
                                                   idMsg2,                             //
                                                   (uint8_t*) msgWrite2,               //
                                                   (uint32_t)(strlen(msgWrite2) + 1)); //

#ifdef DEBUG_RT_TEST
        printf("\r\n==>RT_PUBLISHER : Modify message 3 value\r\n");
#endif
        sprintf(msgWrite3, "Hello world %d for msg 3 :)", i);
        result = SOPC_RT_Publisher_SetMessageValue(myRTPub,                            //
                                                   idMsg3,                             //
                                                   (uint8_t*) msgWrite3,               //
                                                   (uint32_t)(strlen(msgWrite3) + 1)); //
        SOPC_Sleep(1000);
    }

    printf("\r\n==>RT_PUBLISHER : Stop timers value\r\n");

    //SOPC_RT_Publisher_StopMessagePublishing(myRTPub, idMsg1);
    //SOPC_RT_Publisher_StopMessagePublishing(myRTPub, idMsg2);
    //SOPC_RT_Publisher_StopMessagePublishing(myRTPub, idMsg3);

    SOPC_Sleep(200);

    SOPC_ReturnStatus stopResult = SOPC_RT_Publisher_DeInitialize(myRTPub);

    while (stopResult == SOPC_STATUS_INVALID_STATE)
    {
        printf("\r\n==>RT_PUBLISHER : Stop failed, in use...\r\n");
        stopResult = SOPC_RT_Publisher_DeInitialize(myRTPub);
        for (volatile int i = 0; i < loopNb && result == SOPC_STATUS_OK; i++)
        {
            printf("\r\n==>RT_PUBLISHER : Wait %d s...\r\n", loopNb - i);
            SOPC_Sleep(1000);
        }
    }

    if (SOPC_STATUS_OK == result && cptMsgRead1 == cptMsgRead2 && cptMsgRead2 == cptMsgRead3 && cptMsgRead1 == loopNb)
    {
        res = 0;
    }
    else
    {
        res = -1;
    }

    printf("\r\nRT_PUBLISHER RESULT = %d\r\n", res);

    bQuit = true;
    if (handleThreadBeatHeartRTPub != (Thread) NULL)
    {
        SOPC_Thread_Join(handleThreadBeatHeartRTPub);
    }

    SOPC_RT_Publisher_Initializer_Destroy(&myRTPubInitializer);
    SOPC_RT_Publisher_Destroy(&myRTPub);

    return res;
}

/*** ***/

static int SOPC_TEST_MSG_BOX(void)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    int res = 0;
    bQuit = false;
    bFreeze = true;

    myMsgBox = SOPC_MsgBox_Create(2,    // Max clients
                                  16,   // Max evts
                                  512); // Max data evts

    if (myMsgBox == NULL)
    {
        return -1;
    }

    // Thread slept each 200ms
    SOPC_Thread_Create(&handleThreadReadMsgBox1, cbmsgBoxRead1, NULL, "Test thread 1");
    SOPC_Thread_Create(&handleThreadReadMsgBox2, cbmsgBoxRead2, NULL, "Test thread 2");

    if ((handleThreadReadMsgBox1 == (Thread) NULL) || (handleThreadReadMsgBox2 == (Thread) NULL))
    {
        bQuit = true;
        if (handleThreadReadMsgBox1 != (Thread) NULL)
        {
            SOPC_Thread_Join(handleThreadReadMsgBox1);
        }

        if (handleThreadReadMsgBox2 != (Thread) NULL)
        {
            SOPC_Thread_Join(handleThreadReadMsgBox2);
        }

        SOPC_MsgBox_Destroy(&myMsgBox);
        return -1;
    }

    printf("\r\n===>START MSGBOX TEST WITH 2 SLOW READERS\r\n");

    mode = SOPC_MSGBOX_MODE_GET_NORMAL;
    period = PERIOD_MS_READER_200MS;

    bFreeze = false;

    // Push data each 50 ms

    for (volatile uint32_t i = 0; i < 10 && result == SOPC_STATUS_OK; i++)
    {
        sprintf(msgWrite1, "\r\nHello world %u", i);
        SOPC_MsgBox_Push(myMsgBox, (uint8_t*) msgWrite1, (uint32_t)(strlen(msgWrite1) + 1));

        SOPC_Sleep(PERIOD_MS_WRITER_50MS);
    }

    SOPC_Sleep(PERIOD_MS_READER_200MS);
    SOPC_MsgBox_Reset(myMsgBox);
    bFreeze = true;

    SOPC_Sleep(PERIOD_MS_READER_200MS);

    if ((cptMsgRead1 == cptMsgRead2) && (cptMsgRead1 == 10))
    {
        res = 0;
    }
    else
    {
        res = -1;
    }

    printf("\r\n===>MSGBOX SLOW READER TEST RESULT = %d\r\n", res);

    SOPC_Sleep(PERIOD_MS_READER_200MS);

    if (res == 0)
    {
        printf("\r\n===>START MSGBOX TEST WITH 2 FAST READERS\r\n");

        cptMsgRead1 = 0;
        cptMsgRead2 = 0;
        cptMsgDiffRead1 = 0;
        cptMsgDiffRead2 = 0;

        period = PERIOD_MS_READER_50MS;

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        bFreeze = false;

        // Set data each 50 ms
        for (volatile uint32_t i = 0; i < 10 && result == SOPC_STATUS_OK; i++)
        {
            sprintf(msgWrite1, "\r\nHello world %u", i);
            SOPC_MsgBox_Push(myMsgBox, (uint8_t*) msgWrite1, (uint32_t)(strlen(msgWrite1) + 1));
            SOPC_Sleep(PERIOD_MS_WRITER_50MS);
        }

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        SOPC_MsgBox_Reset(myMsgBox);

        bFreeze = true;

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        if ((cptMsgRead1 == cptMsgRead2) && (cptMsgRead1 == 10))
        {
            res = 0;
        }
        else
        {
            res = -1;
        }

        printf("\r\n===>MSGBOX FAST READER TEST RESULT = %d\r\n", res);
    }

    SOPC_Sleep(PERIOD_MS_READER_200MS);

    if (res == 0)
    {
        printf("\r\n===>START MSGBOX TEST WITH 2 FAST READERS AND WITH DATA HANDLE WRITE\r\n");

        cptMsgRead1 = 0;
        cptMsgRead2 = 0;
        cptMsgDiffRead1 = 0;
        cptMsgDiffRead2 = 0;

        period = PERIOD_MS_READER_50MS;

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        bFreeze = false;

        SOPC_MsgBox_DataHandle* pDataHandle = SOPC_MsgBox_DataHandle_Create(myMsgBox);

        if (pDataHandle != NULL)
        {
            // Set data each 50 ms
            for (volatile uint32_t i = 0; i < 80 && result == SOPC_STATUS_OK; i++)
            {
                result = SOPC_MsgBox_DataHandle_Initialize(pDataHandle);
                if (SOPC_STATUS_OK == result)
                {
                    uint8_t* pData;
                    uint32_t max_allowed_size;
                    result = SOPC_MsgBox_DataHandle_GetDataEvt(pDataHandle, &pData, &max_allowed_size);
                    if (SOPC_STATUS_OK == result)
                    {
                        snprintf((char*) pData, max_allowed_size, "\r\nHello world %u", i);
                        result = SOPC_MsgBox_DataHandle_UpdateDataEvtSize(pDataHandle,
                                                                          (uint32_t)(strlen((char*) pData) + 1));
                    }
                    if (SOPC_STATUS_OK == result)
                    {
                        result = SOPC_MsgBox_DataHandle_Finalize(pDataHandle, false);
                    }
                    else
                    {
                        result = SOPC_MsgBox_DataHandle_Finalize(pDataHandle, true);
                    }
                }
                SOPC_Sleep(PERIOD_MS_WRITER_50MS);
            }
        }

        SOPC_MsgBox_DataHandle_Destroy(&pDataHandle);

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        SOPC_MsgBox_Reset(myMsgBox);

        bFreeze = true;

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        if ((cptMsgRead1 == cptMsgRead2) && (cptMsgRead1 == 80))
        {
            res = 0;
        }
        else
        {
            res = -1;
        }

        printf("\r\n===>MSGBOX FAST READER WITH DATA HANDLE WRITE TEST RESULT = %d\r\n", res);
    }

    SOPC_Sleep(PERIOD_MS_READER_200MS);

    if (res == 0)
    {
        printf("\r\n===>START MSGBOX TEST WITH 2 SLOW READERS GET NEW LAST MODE\r\n");

        cptMsgRead1 = 0;
        cptMsgRead2 = 0;
        cptMsgDiffRead1 = 0;
        cptMsgDiffRead2 = 0;

        period = PERIOD_MS_READER_200MS;
        mode = SOPC_MSGBOX_MODE_GET_NEW_LAST;

        bFreeze = false;

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        // Push data each 50 ms
        for (volatile uint32_t i = 0; i < 10 && result == SOPC_STATUS_OK; i++)
        {
            sprintf(msgWrite1, "\r\nHello world %u", i);
            SOPC_MsgBox_Push(myMsgBox, (uint8_t*) msgWrite1, (uint32_t)(strlen(msgWrite1) + 1));
            SOPC_Sleep(PERIOD_MS_WRITER_50MS);
        }

        SOPC_Sleep(PERIOD_MS_READER_200MS);
        SOPC_MsgBox_Reset(myMsgBox);
        bFreeze = true;

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        if ((cptMsgRead1 >= 2 && cptMsgRead1 <= 4) && (cptMsgRead2 >= 2 && cptMsgRead2 <= 4))
        {
            res = 0;
        }
        else
        {
            res = -1;
        }

        printf("\r\n===>MSGBOX SLOW READER GET LAST MODE TEST RESULT = %d\r\n", res);
    }

    SOPC_Sleep(PERIOD_MS_READER_200MS);

    if (res == 0)
    {
        printf("\r\n===>START MSGBOX TEST WITH 2 SLOW READERS GET LAST MODE JUST PEEK\r\n");

        cptMsgRead1 = 0;
        cptMsgRead2 = 0;
        cptMsgDiffRead1 = 0;
        cptMsgDiffRead2 = 0;

        period = PERIOD_MS_READER_200MS;
        mode = SOPC_MSGBOX_MODE_GET_LAST;

        bFreeze = false;
        SOPC_Sleep(PERIOD_MS_READER_200MS);

        // Push data each 50 ms
        for (volatile uint32_t i = 0; i < 10 && result == SOPC_STATUS_OK; i++)
        {
            sprintf(msgWrite1, "\r\nHello world %u", i);
            SOPC_MsgBox_Push(myMsgBox, (uint8_t*) msgWrite1, (uint32_t)(strlen(msgWrite1) + 1));

            SOPC_Sleep(PERIOD_MS_WRITER_50MS);
        }

        SOPC_Sleep(PERIOD_MS_READER_200MS);
        SOPC_MsgBox_Reset(myMsgBox);
        bFreeze = true;

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        if ((cptMsgDiffRead2 >= 2 && cptMsgDiffRead2 <= 4) && (cptMsgDiffRead1 >= 2 && cptMsgDiffRead1 <= 4))
        {
            res = 0;
        }
        else
        {
            res = -1;
        }

        printf("\r\n===>MSGBOX SLOW READERS GET LAST MODE JUST PEEK TEST RESULT = %d\r\n", res);
    }

    SOPC_Sleep(PERIOD_MS_READER_200MS);

    if (res == 0)
    {
        printf("\r\n===>START MSGBOX TEST WITH 2 FAST READERS GET LAST MODE JUST PEEK\r\n");

        cptMsgRead1 = 0;
        cptMsgRead2 = 0;
        cptMsgDiffRead1 = 0;
        cptMsgDiffRead2 = 0;

        period = PERIOD_MS_READER_50MS;
        mode = SOPC_MSGBOX_MODE_GET_LAST;

        bFreeze = false;

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        // Push data each 200 ms
        for (volatile uint32_t i = 0; i < 10 && result == SOPC_STATUS_OK; i++)
        {
            sprintf(msgWrite1, "\r\nHello world %u", i);
            SOPC_MsgBox_Push(myMsgBox, (uint8_t*) msgWrite1, (uint32_t)(strlen(msgWrite1) + 1));

            SOPC_Sleep(PERIOD_MS_WRITER_200MS);
        }

        SOPC_Sleep(PERIOD_MS_READER_200MS);
        SOPC_MsgBox_Reset(myMsgBox);
        bFreeze = true;

        SOPC_Sleep(PERIOD_MS_READER_200MS);

        if ((cptMsgDiffRead2 >= 8 && cptMsgDiffRead2 <= 10) && (cptMsgDiffRead1 >= 8 && cptMsgDiffRead1 <= 10))
        {
            res = 0;
        }
        else
        {
            res = -1;
        }

        printf("\r\n===>MSGBOX FAST READERS GET LAST MODE JUST PEEK TEST RESULT = %d\r\n", res);
    }

    printf("\r\n===>MSGBOX RESULT = %d\r\n", res);

    bQuit = true;

    SOPC_Thread_Join(handleThreadReadMsgBox1);
    SOPC_Thread_Join(handleThreadReadMsgBox2);
    SOPC_MsgBox_Destroy(&myMsgBox);

    return res;
}

static int SOPC_TEST_IRQ_TIMER(void)
{
    int res = 0;
    SOPC_ReturnStatus result;

    bQuit = false;

    printf("\r\n===>INTERRUPT TIMER CREATION\r\n");
    myTimer = SOPC_InterruptTimer_Create();

    bool toggleChaine = false;

    if (myTimer != NULL)
    {
        result = SOPC_InterruptTimer_Initialize(myTimer, 2, 256);
        if (result != SOPC_STATUS_OK)
        {
            printf("\r\n===>ERROR INTERRUPT TIMER INITIALIZATION\r\n");
            res = -1;
        }
        else
        {
            printf("\r\n===>INTERRUPT TIMER INITIALIZED : OK\r\n");
        }

        if (result == SOPC_STATUS_OK)
        {
            handleThreadUpdate = (Thread) NULL;
            result = SOPC_Thread_Create(&handleThreadUpdate, cbUpdate, NULL, "Test thread");
            if (result != SOPC_STATUS_OK)
            {
                printf("\r\n===>ERROR PERIODIC UPDATE THREAD CAN'T BE CREATED\r\n");
                res = -1;
            }
            else
            {
                printf("\r\n===>PERIODIC UPDATE THREAD CREATED\r\n");
            }
        }

        if (result == SOPC_STATUS_OK)
        {
            // Timer with tick period = 4, with 100ms of periodic call (400 ms)
            result = SOPC_InterruptTimer_Instance_Init(myTimer, 0, 4, 0, NULL, cbStart, cbElapsed, cbStop,
                                                       SOPC_INTERRUPT_TIMER_STATUS_ENABLED);
            if (result != SOPC_STATUS_OK)
            {
                printf("\r\n===>ERROR INTERRUPT TIMER INSTANCE 0 CAN'T BE INITIALIZED AND STARTED\r\n");
                res = -1;
            }
            else
            {
                printf("\r\n===>INTERRUPT TIMER INSTANCE 0 INITIALIZED AND STARTED : OK\r\n");
            }
        }

        if (result == SOPC_STATUS_OK)
        {
            // Timer with tick period = 1, with 100ms of periodic call (100ms)
            result = SOPC_InterruptTimer_Instance_Init(myTimer, 1, 1, 0, NULL, cbStart, cbElapsed, cbStop,
                                                       SOPC_INTERRUPT_TIMER_STATUS_ENABLED);
            if (result != SOPC_STATUS_OK)
            {
                printf("\r\n===>ERROR INTERRUPT TIMER INSTANCE 1 CAN'T BE INITIALIZED AND STARTED\r\n");
                res = -1;
            }
            else
            {
                printf("\r\n===>INTERRUPT TIMER INSTANCE 1 INITIALIZED AND STARTED : OK\r\n");
            }
        }

        printf("\r\n===>PERIODIC UPDATE THREAD UNFREEZE TICKS\r\n");
        bFreezeTickRequest = true;

        printf("\r\n===>Starting SET DATA each 1ms\r\n");
        // Set data each 1 ms
        for (volatile uint32_t i = 0; i < 1000 && result == SOPC_STATUS_OK; i++)
        {
            if (toggleChaine)
            {
#ifdef DEBUG_RT_TEST
                printf("\r\n===>SET DATA %s\r\n", chaine2);
#endif
                result = SOPC_InterruptTimer_Instance_SetData(myTimer, 1, (uint8_t*) chaine2,
                                                              (uint32_t)(strlen(chaine2) + 1));
                if (result != SOPC_STATUS_OK)
                {
                    printf("\r\n===>ERROR SET DATA %s\r\n", chaine2);
                    res = -1;
                }
            }
            else
            {
#ifdef DEBUG_RT_TEST
                printf("\r\n===>SET DATA %s\r\n", chaine3);
#endif
                result = SOPC_InterruptTimer_Instance_SetData(myTimer, 1, (uint8_t*) chaine3,
                                                              (uint32_t) strlen(chaine3) + 1);
                if (result != SOPC_STATUS_OK)
                {
                    printf("\r\n===>ERROR SET DATA %s\r\n", chaine3);
                    res = -1;
                }
            }
            toggleChaine = !toggleChaine;
            SOPC_Sleep(1);
        }

        printf("\r\n===>Starting SET DATA each 100ms\r\n");
        // Set data each 100 ms
        for (volatile uint32_t i = 0; i < 10 && result == SOPC_STATUS_OK; i++)
        {
            if (toggleChaine)
            {
#ifdef DEBUG_RT_TEST
                printf("\r\n===>SET DATA %s\r\n", chaine2);
#endif
                result = SOPC_InterruptTimer_Instance_SetData(myTimer, 1, (uint8_t*) chaine2,
                                                              (uint32_t)(strlen(chaine2) + 1));
                if (result != SOPC_STATUS_OK)
                {
                    printf("\r\n===>ERROR SET DATA %s\r\n", chaine2);
                    res = -1;
                }
            }
            else
            {
#ifdef DEBUG_RT_TEST
                printf("\r\n===>SET DATA %s\r\n", chaine3);
#endif
                result = SOPC_InterruptTimer_Instance_SetData(myTimer, 1, (uint8_t*) chaine3,
                                                              (uint32_t)(strlen(chaine3) + 1));
                if (result != SOPC_STATUS_OK)
                {
                    printf("\r\n===>ERROR SET DATA %s\r\n", chaine3);
                    res = -1;
                }
            }
            toggleChaine = !toggleChaine;
            SOPC_Sleep(100);
        }

        printf("\r\n===>Starting SET DATA each 200ms\r\n");
        // Set data each 200 ms
        for (volatile uint32_t i = 0; i < 10 && result == SOPC_STATUS_OK; i++)
        {
            if (toggleChaine)
            {
#ifdef DEBUG_RT_TEST
                printf("\r\n===>SET DATA %s\r\n", chaine2);
#endif
                result = SOPC_InterruptTimer_Instance_SetData(myTimer, 1, (uint8_t*) chaine2,
                                                              (uint32_t)(strlen(chaine2) + 1));
                if (result != SOPC_STATUS_OK)
                {
                    printf("\r\n===>ERROR SET DATA %s\r\n", chaine2);
                    res = -1;
                }
            }
            else
            {
#ifdef DEBUG_RT_TEST
                printf("\r\n===>SET DATA %s\r\n", chaine3);
#endif
                result = SOPC_InterruptTimer_Instance_SetData(myTimer, 1, (uint8_t*) chaine3,
                                                              (uint32_t)(strlen(chaine3) + 1));
                if (result != SOPC_STATUS_OK)
                {
                    printf("\r\n===>ERROR SET DATA %s\r\n", chaine3);
                    res = -1;
                }
            }
            toggleChaine = !toggleChaine;
            SOPC_Sleep(200);
        }

        SOPC_Sleep(200);

        printf("\r\n===>PERIODIC UPDATE THREAD FREEZE TICKS\r\n");
        bFreezeTickRequest = false;

        SOPC_Sleep(200);

        printf("\r\n===>STOP TIMERS\r\n");

        SOPC_IrqTimer_InstanceStatus status = 0xFFFFFFFF;
        result = SOPC_InterruptTimer_Instance_LastStatus(myTimer, 0, &status);

        printf("\r\n===>STOP TIMER 0 NEXT STATUS BEFORE STOP CMD = %u\r\n", status);
        if (SOPC_STATUS_OK == result && SOPC_INTERRUPT_TIMER_STATUS_ENABLED == status)
        {
            result = SOPC_InterruptTimer_Instance_Stop(myTimer, 0);

            if (SOPC_STATUS_OK == result)
            {
                status = 0xFFFFFFFF;
                result = SOPC_InterruptTimer_Instance_LastStatus(myTimer, 0, &status);
                printf("\r\n===>STOP TIMER 0 NEXT STATUS AFTER STOP CMD = %u\r\n", status);

                if (status != SOPC_INTERRUPT_TIMER_STATUS_DISABLED)
                {
                    result = SOPC_STATUS_NOK;
                }
            }
        }

        if (result != SOPC_STATUS_OK)
        {
            status = 0xFFFFFFFF;
            SOPC_InterruptTimer_Instance_LastStatus(myTimer, 0, &status);
            printf("\r\n===>ERROR STOP TIMER 0 - STATUS = %u\r\n", status);
            res = -1;
        }

        result = SOPC_InterruptTimer_Instance_Stop(myTimer, 1);
        if (result != SOPC_STATUS_OK)
        {
            printf("\r\n===>ERROR STOP TIMER 1\r\n");
            res = -1;
        }

        SOPC_Sleep(400);

        printf("\r\n T0 NB START = %u , T1 NB START = %u  \r\n", bTest01_TestStart[0], bTest01_TestStart[1]);
        printf("\r\n T0 NB TICKS = %u , T1 NB TICKS = %u , GLOBAL TICK = %u\r\n", bTest02_TestPeriod[0],
               bTest02_TestPeriod[1], globalTick);
        printf("\r\n T0 NB CORRUPTED DATA = %u , T1 NB CORRUPTED DATA = %u  \r\n", bTest04_TestCorrupted[0],
               bTest04_TestCorrupted[1]);

        if (!((globalTick == bTest02_TestPeriod[1])          //
              && ((globalTick / 4) == bTest02_TestPeriod[0]) //
              && (0 == bTest04_TestCorrupted[1])             //
              && (0 == bTest04_TestCorrupted[0])             //
              && (1 == bTest01_TestStart[1])                 //
              && (1 == bTest01_TestStart[0]))                //
        )
        {
            res = -1;
        }

        printf("\r\n===>PERIODIC UPDATE THREAD UNFREEZE TICKS\r\n");
        bFreezeTickRequest = true;
        SOPC_Sleep(200);

        printf("\r\n T0 NB STOP = %u , T1 NB STOP = %u  \r\n", bTest03_TestStop[0], bTest03_TestStop[1]);

        if (!(1 == bTest03_TestStop[1] && 1 == bTest03_TestStop[0]))
        {
            res = -1;
        }

        bQuit = true;

        if (handleThreadUpdate != (Thread) NULL)
        {
            printf("\r\n===>WAIT END OF UPDATE TEST\r\n");
            SOPC_Thread_Join(handleThreadUpdate);
        }

        printf("\r\n===>INTERRUPT TIMER DEINITIALIZATION\r\n");
        result = SOPC_InterruptTimer_DeInitialize(myTimer);
        if (result != SOPC_STATUS_OK)
        {
            printf("\r\n===>ERROR INTERRUPT TIMER DEINITIALIZATION\r\n");
            res = -1;
        }
        else
        {
            printf("\r\n===>INTERRUPT TIMER DESTRUCTION\r\n");
            SOPC_InterruptTimer_Destroy(&myTimer);
        }

        printf("\r\nINTERRUPT TIMER RESULT = %d\r\n", res);
        return res;
    }
    else
    {
        printf("\r\n===>ERROR INTERRUPT TIMER CREATION\r\n");
        return -1;
    }

    return -1;
}

int main(void)
{
    /* TODO: use libcheck */
    int res = 0;

    res = SOPC_TEST_IRQ_TIMER();
    if (res == 0)
    {
        res = SOPC_TEST_MSG_BOX();
    }
    if (res == 0)
    {
        res = SOPC_TEST_RT_PUBLISHER();
    }
    if (res == 0)
    {
        res = SOPC_TEST_RT_SUBSCRIBER();
    }
    if (res == 0)
    {
        res = SOPC_TEST_RT_SUBSCRIBER_WITH_DATAHANDLE_WRITER();
    }

    return res;
}
