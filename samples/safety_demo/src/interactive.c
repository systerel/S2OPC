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
/** \file All user-interactive commands are entered in STDIN. They are process when the
 * full line is confirmed by the "Enter" Key. The first letter of the line providse the command
 * and all remaining chars may be used as parameters. All commands are generic and created
 * using "Create_Callback"
 *
 */
/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"
#include "sopc_raw_sockets.h"
#include "sopc_time.h"

#include "interactive.h"
#include "uas.h"


/*============================================================================
 * NCURSES-LIKE DISPLAY
 *===========================================================================*/
typedef struct zDisplayContent_struct
{
    bool screenReady;
    bool isModified;
    bool isBlocked;
    bool isProvider;
} zDisplayContent_type;

static struct winsize zterminalWinsize;

#define DISPLAY_ESC "\033"
#define DISPLAY_CLEAR_EOL DISPLAY_ESC "[0K"
#define DISPLAY_CLEAR_SCREEN DISPLAY_ESC "[2J"
#define DISPLAY_GOTO_END_OF_SCREEN DISPLAY_ESC "F\n"

static void Utils_Interactive_displayInit(void);
static void Utils_Interactive_displayTerminate(void);


static zDisplayContent_type zDisplayContent;

/*============================================================================
 * NCURSES-LIKE DISPLAY IMPLEMENTATION
 *===========================================================================*/
static void Utils_Interactive_displayInit(void)
{
    ioctl(0, TIOCGWINSZ, &zterminalWinsize);
    //  printf ("lines %d\n", zterminalWinsize.ws_row);
    //  printf ("columns %d\n", zterminalWinsize.ws_col);
    zDisplayContent.screenReady = false;
}

static void Utils_Interactive_displayTerminate(void)
{
    puts(DISPLAY_GOTO_END_OF_SCREEN);
}

void Utils_Interactive_displayClearEOL(void)
{
    printf(DISPLAY_CLEAR_EOL);
}

void Utils_Interactive_displaySetCursorLocation(const UAS_UInt32 x, const UAS_UInt32 y)
{
    printf(DISPLAY_ESC "[%u;%uH" DISPLAY_CLEAR_EOL, (unsigned int) y, (unsigned int) x);
}

void Utils_Interactive_displayClearScreen(void)
{
    puts(DISPLAY_CLEAR_SCREEN);
}
void Utils_Interactive_displayPushPosition(void)
{
    //    puts( DISPLAY_ESC"7"); // TODO does not work..
}
void Utils_Interactive_displayPopPosition(void)
{
    //    puts( DISPLAY_ESC"8"); // TODO does not work..
}


//
///*===========================================================================*/
//static void displayCheckModified(SafetyDemo_interactive_Context* pContext)
//{
//    static UAS_UInt32 uRequestMnr = 0xFFFFFFFF;
//    static UAS_UInt32 uResponseMnr = 0xFFFFFFFF;
//    SOPC_ReturnStatus bReturn = SOPC_STATUS_OK;
//    SafetyDemo_Sample_Safe1_type zSafeData;
//    SafetyDemo_Sample_NonSafe1_type zNonSafeData;
//    UAS_RequestSpdu_type zRequest;
//    UAS_ResponseSpdu_type zResponse;
//    bReturn = UAM_SpduEncoder_GetRequest(pContext->spduRequestId, &zRequest);
//    if (bReturn == SOPC_STATUS_OK && uRequestMnr != zRequest.dwMonitoringNumber)
//    {
//        uRequestMnr = zRequest.dwMonitoringNumber;
//        displaySetModified();
//    }
//
//    zResponse.pbySerializedSafetyData = (void*) &zSafeData;
//    zResponse.pbySerializedNonSafetyData = (void*) &zNonSafeData;
//    bReturn = UAM_SpduEncoder_GetResponse(pContext->spduRequestId, &zResponse, NULL, NULL);
//    if (bReturn == SOPC_STATUS_OK && uResponseMnr != zResponse.dwMonitoringNumber)
//    {
//        uResponseMnr = zResponse.dwMonitoringNumber;
//        displaySetModified();
//    }
//}
//
//#define HAS_FLAG(v, bit) (((v) & (1 << (bit))) != 0)
//
///*===========================================================================*/
//static void displayShowRequest(const UAM_SpduRequestHandle handle, UAS_UInt32* pYPos)
//{
//    SOPC_ReturnStatus bReturn = SOPC_STATUS_OK;
//    UAS_RequestSpdu_type zSpdu;
//    assert(pYPos != NULL);
//    displaySetCursorLocation(14, *pYPos);
//    (*pYPos)++;
//    printf("- SPDU Request Id=%d", (int) handle);
//    bReturn = UAM_SpduEncoder_GetRequest(handle, &zSpdu);
//    if (bReturn == SOPC_STATUS_OK)
//    {
//        displaySetCursorLocation(18, *pYPos);
//        (*pYPos)++;
//        printf("- MNR=%08Xh, CONSID=%08Xh, FLAGS= [", zSpdu.dwMonitoringNumber, zSpdu.dwSafetyConsumerId);
//        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_COMMUNICATION_ERROR))
//        {
//            printf("COMM ERROR, ");
//        }
//        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_FSV_ACTIVATED))
//        {
//            printf("FSV act, ");
//        }
//        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED))
//        {
//            printf("ACK req, ");
//        }
//        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_FSV_ACTIVATED))
//        {
//            printf("FSV act, ");
//        }
//        printf("]");
//    }
//}
//
///*===========================================================================*/
//static void displayShowProvider(const UAS_UInt8 handle, UAS_UInt32* pYPos)
//{
//    assert(pYPos != NULL);
//    assert(handle < UASDEF_MAX_SAFETYPROVIDERS);
//    UAS_SafetyProvider_type* pzInstance = &azUAS_SafetyProviders[handle];
//
//    displaySetCursorLocation(14, *pYPos);
//    (*pYPos)++;
//    printf("- IN  =");
//    if (pzInstance->zInputSAPI.bActivateFsv)
//    {
//        printf("FSV,");
//    }
//    if (pzInstance->zInputSAPI.bEnableTestMode)
//    {
//        printf("TEST,");
//    }
//    if (pzInstance->zInputSAPI.bOperatorAckProvider)
//    {
//        printf("ACK PROV,");
//    }
//    if (pzInstance->zInputSAPI.pbySerializedSafetyData)
//    {
//        const SafetyDemo_Sample_Safe1_union uUnion = {.pzVoid = pzInstance->zInputSAPI.pbySerializedSafetyData};
//        const SafetyDemo_Sample_Safe1_type* pzSafeData = uUnion.pzType;
//
//        displaySetCursorLocation(18, *pYPos);
//        (*pYPos)++;
//        printf("- SAFE = [b1=%d,b2=%d,v1=%02X, v2=%02X,... , txt=<%s>]\033[0K\n", pzSafeData->bData1,
//               pzSafeData->bData2, pzSafeData->u8Val1, pzSafeData->u8Val2, pzSafeData->sText10);
//    }
//
//    displaySetCursorLocation(14, *pYPos);
//    (*pYPos)++;
//    printf("- OUT =");
//    if (pzInstance->zOutputSAPI.bOperatorAckRequested)
//    {
//        printf("ACK requested,");
//    }
//}
//
///*===========================================================================*/
//static void displayShowConsumer(const UAS_UInt8 handle, UAS_UInt32* pYPos)
//{
//    assert(pYPos != NULL);
//    assert(handle < UASDEF_MAX_SAFETYCONSUMERS);
//    UAS_SafetyConsumer_type* pzInstance = &azUAS_SafetyConsumers[handle];
//
//    displaySetCursorLocation(14, *pYPos);
//    (*pYPos)++;
//    printf("- IN  = [");
//    if (pzInstance->zInputSAPI.bEnable)
//    {
//        printf("ENABLE,");
//    }
//    if (pzInstance->zInputSAPI.bOperatorAckConsumer)
//    {
//        printf("ACK CONS,");
//    }
//    printf("]");
//    displayClearEOL();
//
//    displaySetCursorLocation(14, *pYPos);
//    (*pYPos)++;
//    printf("- OUT = [");
//    if (pzInstance->zOutputSAPI.bFsvActivated)
//    {
//        printf("FSV,");
//    }
//    if (pzInstance->zOutputSAPI.bOperatorAckProvider)
//    {
//        printf("ACK PROV,");
//    }
//    if (pzInstance->zOutputSAPI.bOperatorAckRequested)
//    {
//        printf("ACK requested,");
//    }
//    printf("]");
//    displayClearEOL();
//}
//
///*===========================================================================*/
//static void displayShowResponse(const UAM_SpduResponseHandle handle, UAS_UInt32* pYPos)
//{
//    SOPC_ReturnStatus bReturn = SOPC_STATUS_OK;
//    UAS_ResponseSpdu_type zSpdu;
//    SafetyDemo_Sample_Safe1_type zSafeData;
//    SafetyDemo_Sample_NonSafe1_type zNonSafeData;
//
//    assert(pYPos != NULL);
//    displaySetCursorLocation(14, *pYPos);
//    (*pYPos)++;
//    printf("- SPDU Response Id=%d", (int) handle);
//    zSpdu.pbySerializedSafetyData = (void*) &zSafeData;
//    zSpdu.pbySerializedNonSafetyData = (void*) &zNonSafeData;
//
//    bReturn = UAM_SpduEncoder_GetResponse(handle, &zSpdu, NULL, NULL);
//    if (bReturn == SOPC_STATUS_OK)
//    {
//        displaySetCursorLocation(18, *pYPos);
//        (*pYPos)++;
//        printf("- MNR=%08Xh, CONSID=%08Xh, CRC=%08Xh, FLAGS= [", zSpdu.dwMonitoringNumber, zSpdu.dwSafetyConsumerId,
//               zSpdu.dwCrc);
//        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_ACTIVATE_FSV))
//        {
//            printf("FSV, ");
//        }
//        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_TEST_MODE_ACTIVATED))
//        {
//            printf("TEST MODE, ");
//        }
//        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_OPERATOR_ACK_PROVIDER))
//        {
//            printf("ACK PROV, ");
//        }
//        printf("]");
//    }
//}
//


/*============================================================================
 * LOCAL PROTOTYPES
 *===========================================================================*/

/**
 * \brief execute a command requested by interactive thread
 * \return true in case of success
 */
static bool SafetyDemo_interactive_process(const char* input);


/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/

/**
 * Configuration of a user event.
 */
typedef struct
{
    pfUtils_Interactive_Event pfEvent;
    const char* descr;
    void* pUserParam ;
} Interactive_CallbackCfg;

/**
 * List of all possible callback (indexed on unsigned char)
 */
static Interactive_CallbackCfg g_callbackList[0x100];

/*============================================================================
 * IMPLEMENTATION
 *===========================================================================*/
#include <unistd.h>
#define STDIN 0
#define USER_ENTRY_MAXSIZE (128u)


/*===========================================================================*/
static bool SafetyDemo_interactive_process(const char* input)
{
    bool result = false;
    if (input[0] == 0)
    {
        return true;
    }

    const unsigned char uKey = (unsigned char) (input[0]);
    input++;

    while (input[0] <= ' ' && input[0] > 0)
    {
        input++;
    }

    Interactive_CallbackCfg* cfg = &g_callbackList[uKey];
    if (cfg->pfEvent != NULL)
    {
        result = (*cfg->pfEvent)(input, cfg->pUserParam);
        Utils_Interactive_ForceRefresh();
    }

    return result;
}


/*============================================================================
 * IMPLEMENTATION OF INTERNAL SERVICES
 *===========================================================================*/
/*===========================================================================*/
void Utils_Interactive_Initialize(void)
{
    int i = 0;
    for (i = 0; i < 0x100; i++)
    {
        g_callbackList[i].pfEvent = NULL;
        g_callbackList[i].pUserParam = NULL;
        g_callbackList[i].descr = NULL;
    }

    Utils_Interactive_displayInit();
}

/*===========================================================================*/
void Utils_Interactive_Clear(void)
{
    Utils_Interactive_displayTerminate();
}

/*===========================================================================*/
void Utils_Interactive_AddCallback(const char key, const char* pDescr, pfUtils_Interactive_Event pfEvent, void* pUserParam)
{
    unsigned char uKey = (unsigned char) (key);
    assert(pfEvent != NULL);
    assert(key > 0);
    uKey = (unsigned char) key;
    g_callbackList[uKey].pfEvent = pfEvent;
    g_callbackList[uKey].pUserParam = pUserParam;
    g_callbackList[uKey].descr = (pDescr == NULL ? "" : pDescr);
}

/*===========================================================================*/
void Utils_Interactive_ForceRefresh(void)
{
    ioctl(0, TIOCGWINSZ, &zterminalWinsize);
    zDisplayContent.isModified = true;
}

/*===========================================================================*/
void Utils_Interactive_execute(void)
{
    SOPC_SocketSet fdSet;
    int result = 0;
    ssize_t nbRead = 0;
    bool bResult = false;
    char entry[USER_ENTRY_MAXSIZE];

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    SOPC_SocketSet_Clear(&fdSet);
    SOPC_SocketSet_Add(STDIN, &fdSet);

    result = select(STDIN + 1, &fdSet.set, NULL, NULL, &tv);
    if (result > 0 && SOPC_SocketSet_IsPresent(STDIN, &fdSet))
    {
        nbRead = read(STDIN, entry, USER_ENTRY_MAXSIZE - 1);
        while (nbRead > 0 && entry[nbRead - 1] < ' ')
        {
            entry[nbRead - 1] = 0;
            nbRead--;
        }
        if (nbRead > 0)
        {
            entry[nbRead] = 0;
            bResult = SafetyDemo_interactive_process(entry);
            if (!bResult)
            {
                printf("Error while processing command \"%s\"\n", entry);
                Utils_Interactive_printHelp();
            }
        }
    }

}

/*===========================================================================*/
void Utils_Interactive_printHelp(void)
{
    printf("Available commands:\n");

    unsigned int i = 0;
    for (i = 0; i < 0x100; i++)
    {
        Interactive_CallbackCfg* cfg = &g_callbackList[i];
        if (cfg->descr != NULL)
        {
            printf("%c     :%s\n", (char) i, cfg->descr);
        }
    }
    return ;
}
