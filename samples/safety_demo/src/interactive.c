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

#include "config.h"
#include "interactive.h"
#include "safetyDemo.h"
#include "uam_cache.h"
#include "uam_spduEncoders.h"

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
static void displayInit(void);
static void displayClearWin2(void);
static void displaySetModified(void);
static void displayClearEOL(void)
{
    printf(DISPLAY_ESC "[0K");
}
static void displayGotoWin2(void);
static void displayExecute(SafetyDemo_interactive_Context* pContext);
static void displayClearScreen(void)
{
    puts(DISPLAY_ESC "[2J");
}
static void displaySetCursorLocation(const UAS_UInt32 x, const UAS_UInt32 y)
{
    displayClearEOL();
    printf(DISPLAY_ESC "[%u;%uH", (unsigned int) y, (unsigned int) x);
    displayClearEOL();
}
static void displayPushPosition(void)
{
    //    puts( DISPLAY_ESC"7"); // TODO does not work..
}
static void displayPopPosition(void)
{
    //    puts( DISPLAY_ESC"8"); // TODO does not work..
}
static void displayStatus(const UAS_UInt32 x0, const char* fmt, ...);
static zDisplayContent_type zDisplayContent;
static const UAS_UInt32 displayNbLinesWin2 = 10;
static const UAS_UInt32 displayNbLinesStatus = 2;

/*============================================================================
 * NCURSES-LIKE DISPLAY IMPLEMENTATION
 *===========================================================================*/
static void displayInit(void)
{
    ioctl(0, TIOCGWINSZ, &zterminalWinsize);
    //  printf ("lines %d\n", zterminalWinsize.ws_row);
    //  printf ("columns %d\n", zterminalWinsize.ws_col);
    zDisplayContent.screenReady = false;
}
/*===========================================================================*/
static void displayTerminate(void)
{
    puts(DISPLAY_ESC "F\n"); // Goto last line
}

/*===========================================================================*/
static void displayClearWin2(void)
{
    // Erase bottom screen
    displayGotoWin2();
    puts(DISPLAY_ESC "[0J");
}
/*===========================================================================*/
static void displaySetModified(void)
{
    ioctl(0, TIOCGWINSZ, &zterminalWinsize);
    zDisplayContent.isModified = true;
}

/*===========================================================================*/
static void displayCheckModified(SafetyDemo_interactive_Context* pContext)
{
    static UAS_UInt32 uRequestMnr = 0xFFFFFFFF;
    static UAS_UInt32 uResponseMnr = 0xFFFFFFFF;
    SOPC_ReturnStatus bReturn = SOPC_STATUS_OK;
    SafetyDemo_Sample_Safe1_type zSafeData;
    SafetyDemo_Sample_NonSafe1_type zNonSafeData;
    UAS_RequestSpdu_type zRequest;
    UAS_ResponseSpdu_type zResponse;
    bReturn = UAM_SpduEncoder_GetRequest(pContext->spduRequestId, &zRequest);
    if (bReturn == SOPC_STATUS_OK && uRequestMnr != zRequest.dwMonitoringNumber)
    {
        uRequestMnr = zRequest.dwMonitoringNumber;
        displaySetModified();
    }

    zResponse.pbySerializedSafetyData = (void*) &zSafeData;
    zResponse.pbySerializedNonSafetyData = (void*) &zNonSafeData;
    bReturn = UAM_SpduEncoder_GetResponse(pContext->spduRequestId, &zResponse, NULL, NULL);
    if (bReturn == SOPC_STATUS_OK && uResponseMnr != zResponse.dwMonitoringNumber)
    {
        uResponseMnr = zResponse.dwMonitoringNumber;
        displaySetModified();
    }
}

#define HAS_FLAG(v, bit) (((v) & (1 << (bit))) != 0)

/*===========================================================================*/
static void displayShowRequest(const UAM_SpduRequestHandle handle, UAS_UInt32* pYPos)
{
    SOPC_ReturnStatus bReturn = SOPC_STATUS_OK;
    UAS_RequestSpdu_type zSpdu;
    assert(pYPos != NULL);
    displaySetCursorLocation(14, *pYPos);
    (*pYPos)++;
    printf("- SPDU Request Id=%d", (int) handle);
    bReturn = UAM_SpduEncoder_GetRequest(handle, &zSpdu);
    if (bReturn == SOPC_STATUS_OK)
    {
        displaySetCursorLocation(18, *pYPos);
        (*pYPos)++;
        printf("- MNR=%08Xh, CONSID=%08Xh, FLAGS= [", zSpdu.dwMonitoringNumber, zSpdu.dwSafetyConsumerId);
        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_COMMUNICATION_ERROR))
        {
            printf("COMM ERROR, ");
        }
        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_FSV_ACTIVATED))
        {
            printf("FSV act, ");
        }
        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED))
        {
            printf("ACK req, ");
        }
        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_FSV_ACTIVATED))
        {
            printf("FSV act, ");
        }
        printf("]");
    }
}

/*===========================================================================*/
static void displayShowProvider(const UAS_UInt8 handle, UAS_UInt32* pYPos)
{
    assert(pYPos != NULL);
    assert(handle < UASDEF_MAX_SAFETYPROVIDERS);
    UAS_SafetyProvider_type* pzInstance = &azUAS_SafetyProviders[handle];

    displaySetCursorLocation(14, *pYPos);
    (*pYPos)++;
    printf("- IN  =");
    if (pzInstance->zInputSAPI.bActivateFsv)
    {
        printf("FSV,");
    }
    if (pzInstance->zInputSAPI.bEnableTestMode)
    {
        printf("TEST,");
    }
    if (pzInstance->zInputSAPI.bOperatorAckProvider)
    {
        printf("ACK PROV,");
    }
    if (pzInstance->zInputSAPI.pbySerializedSafetyData)
    {
        const SafetyDemo_Sample_Safe1_union uUnion = {.pzVoid = pzInstance->zInputSAPI.pbySerializedSafetyData};
        const SafetyDemo_Sample_Safe1_type* pzSafeData = uUnion.pzType;

        displaySetCursorLocation(18, *pYPos);
        (*pYPos)++;
        printf("- SAFE = [b1=%d,b2=%d,v1=%02X, v2=%02X,... , txt=<%s>]\033[0K\n", pzSafeData->bData1,
               pzSafeData->bData2, pzSafeData->u8Val1, pzSafeData->u8Val2, pzSafeData->sText10);
    }

    displaySetCursorLocation(14, *pYPos);
    (*pYPos)++;
    printf("- OUT =");
    if (pzInstance->zOutputSAPI.bOperatorAckRequested)
    {
        printf("ACK requested,");
    }
}

/*===========================================================================*/
static void displayShowConsumer(const UAS_UInt8 handle, UAS_UInt32* pYPos)
{
    assert(pYPos != NULL);
    assert(handle < UASDEF_MAX_SAFETYCONSUMERS);
    UAS_SafetyConsumer_type* pzInstance = &azUAS_SafetyConsumers[handle];

    displaySetCursorLocation(14, *pYPos);
    (*pYPos)++;
    printf("- IN  = [");
    if (pzInstance->zInputSAPI.bEnable)
    {
        printf("ENABLE,");
    }
    if (pzInstance->zInputSAPI.bOperatorAckConsumer)
    {
        printf("ACK CONS,");
    }
    printf("]");
    displayClearEOL();

    displaySetCursorLocation(14, *pYPos);
    (*pYPos)++;
    printf("- OUT = [");
    if (pzInstance->zOutputSAPI.bFsvActivated)
    {
        printf("FSV,");
    }
    if (pzInstance->zOutputSAPI.bOperatorAckProvider)
    {
        printf("ACK PROV,");
    }
    if (pzInstance->zOutputSAPI.bOperatorAckRequested)
    {
        printf("ACK requested,");
    }
    printf("]");
    displayClearEOL();
}

/*===========================================================================*/
static void displayShowResponse(const UAM_SpduResponseHandle handle, UAS_UInt32* pYPos)
{
    SOPC_ReturnStatus bReturn = SOPC_STATUS_OK;
    UAS_ResponseSpdu_type zSpdu;
    SafetyDemo_Sample_Safe1_type zSafeData;
    SafetyDemo_Sample_NonSafe1_type zNonSafeData;

    assert(pYPos != NULL);
    displaySetCursorLocation(14, *pYPos);
    (*pYPos)++;
    printf("- SPDU Response Id=%d", (int) handle);
    zSpdu.pbySerializedSafetyData = (void*) &zSafeData;
    zSpdu.pbySerializedNonSafetyData = (void*) &zNonSafeData;

    bReturn = UAM_SpduEncoder_GetResponse(handle, &zSpdu, NULL, NULL);
    if (bReturn == SOPC_STATUS_OK)
    {
        displaySetCursorLocation(18, *pYPos);
        (*pYPos)++;
        printf("- MNR=%08Xh, CONSID=%08Xh, CRC=%08Xh, FLAGS= [", zSpdu.dwMonitoringNumber, zSpdu.dwSafetyConsumerId,
               zSpdu.dwCrc);
        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_ACTIVATE_FSV))
        {
            printf("FSV, ");
        }
        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_TEST_MODE_ACTIVATED))
        {
            printf("TEST MODE, ");
        }
        if (HAS_FLAG(zSpdu.byFlags, UAS_BITPOS_OPERATOR_ACK_PROVIDER))
        {
            printf("ACK PROV, ");
        }
        printf("]");
    }
}

/*===========================================================================*/
static void displayExecute(SafetyDemo_interactive_Context* pContext)
{
    assert(NULL != pContext);
    UAS_UInt32 index = 0;
    UAS_UInt16 indexReader = 0;
    UAS_UInt8 indexDataSet = 0;

    const UAS_UInt32 uLastLine = zterminalWinsize.ws_row - displayNbLinesWin2 - displayNbLinesStatus;

    if (!zDisplayContent.screenReady)
    {
        zDisplayContent.isModified = true;
        displayClearScreen();
        zDisplayContent.screenReady = true;
        zDisplayContent.isBlocked = false;
    }

    displayCheckModified(pContext);
    if (zDisplayContent.isModified && !zDisplayContent.isBlocked)
    {
        UAS_UInt32 yPos = 1;
        UAS_UInt16 nbReaders = 0;
        UAS_UInt16 nbDataSets = 0;
        const UAS_UInt32 nbPubConnections = SOPC_PubSubConfiguration_Nb_PubConnection(pContext->pConfig);
        const UAS_UInt32 nbSubConnections = SOPC_PubSubConfiguration_Nb_SubConnection(pContext->pConfig);
        SOPC_ReaderGroup* pzReader = NULL;
        zDisplayContent.isProvider = pContext->isProvider;

        displayPushPosition();

        displaySetCursorLocation(1, yPos);

        if (pContext->isProvider)
        {
            printf("PROVIDER CONFIGURATION");
        }
        else
        {
            printf("CONSUMER CONFIGURATION");
        }
        yPos++;
        displayShowRequest(pContext->spduRequestId, &yPos);
        displayShowResponse(pContext->spduResponseId, &yPos);

        if (pContext->isProvider)
        {
            displayShowProvider(0, &yPos);
        }
        else
        {
            displayShowConsumer(0, &yPos);
        }
        yPos++;
        displaySetCursorLocation(1, yPos);

        printf("PUBLISHERS:");
        yPos++;
        for (index = 0; index < nbPubConnections && yPos < uLastLine; index++)
        {
            SOPC_PubSubConnection* pzPubSubCxn = SOPC_PubSubConfiguration_Get_PubConnection_At(pContext->pConfig, 0);
            if (NULL != pzPubSubCxn)
            {
                displaySetCursorLocation(10, yPos);
                yPos++;
                printf("- PUB #%d,  ADDR = %s", (int) (index + 1), SOPC_PubSubConnection_Get_Address(pzPubSubCxn));

                nbReaders = SOPC_PubSubConnection_Nb_ReaderGroup(pzPubSubCxn);
                for (indexReader = 0; indexReader < nbReaders; ++indexReader)
                {
                    pzReader = SOPC_PubSubConnection_Get_ReaderGroup_At(pzPubSubCxn, indexReader);
                    nbDataSets = SOPC_ReaderGroup_Nb_DataSetReader(pzReader);
                    for (indexDataSet = 0; indexDataSet < nbDataSets; ++indexDataSet)
                    {
                        SOPC_DataSetReader* pzDataSet = SOPC_ReaderGroup_Get_DataSetReader_At(pzReader, indexDataSet);
                        uint16_t nbMeta = SOPC_DataSetReader_Nb_FieldMetaData(pzDataSet);
                        for (uint16_t indexMeta = 0; indexMeta < nbMeta; indexMeta++)
                        {
                            SOPC_FieldMetaData* pzField = SOPC_DataSetReader_Get_FieldMetaData_At(pzDataSet, indexMeta);
                            const SOPC_PublishedVariable* pzVariable =
                                SOPC_FieldMetaData_Get_PublishedVariable(pzField);
                            if (pzVariable != NULL)
                            {
                                const SOPC_NodeId* pzNodeId = SOPC_PublishedVariable_Get_NodeId(pzVariable);
                                if (NULL != pzNodeId)
                                {
                                    char* sNodeName = SOPC_NodeId_ToCString(pzNodeId);
                                    displaySetCursorLocation(14, yPos);
                                    yPos++;
                                    printf("- Node [%s]", sNodeName);
                                    SOPC_Free(sNodeName);
                                }
                            }
                        }
                    }
                }
            }
        }
        displaySetCursorLocation(1, yPos);
        printf("SUBSCRIBERS:");
        yPos++;
        for (index = 0; index < nbSubConnections && yPos < uLastLine; index++)
        {
            SOPC_PubSubConnection* pzPubSubCxn = SOPC_PubSubConfiguration_Get_SubConnection_At(pContext->pConfig, 0);
            if (NULL != pzPubSubCxn)
            {
                displaySetCursorLocation(10, yPos);
                yPos++;
                printf("- SUB #%d,  ADDR = %s", (int) (index + 1), SOPC_PubSubConnection_Get_Address(pzPubSubCxn));
                nbReaders = SOPC_PubSubConnection_Nb_ReaderGroup(pzPubSubCxn);
                for (indexReader = 0; indexReader < nbReaders; ++indexReader)
                {
                    pzReader = SOPC_PubSubConnection_Get_ReaderGroup_At(pzPubSubCxn, indexReader);
                    nbDataSets = SOPC_ReaderGroup_Nb_DataSetReader(pzReader);
                    for (indexDataSet = 0; indexDataSet < nbDataSets; ++indexDataSet)
                    {
                        SOPC_DataSetReader* pzDataSet = SOPC_ReaderGroup_Get_DataSetReader_At(pzReader, indexDataSet);
                        uint16_t nbMeta = SOPC_DataSetReader_Nb_FieldMetaData(pzDataSet);
                        for (uint16_t indexMeta = 0; indexMeta < nbMeta; indexMeta++)
                        {
                            SOPC_FieldMetaData* pzField = SOPC_DataSetReader_Get_FieldMetaData_At(pzDataSet, indexMeta);
                            const SOPC_PublishedVariable* pzVariable =
                                SOPC_FieldMetaData_Get_PublishedVariable(pzField);
                            if (pzVariable != NULL)
                            {
                                const SOPC_NodeId* pzNodeId = SOPC_PublishedVariable_Get_NodeId(pzVariable);
                                if (NULL != pzNodeId)
                                {
                                    char* sNodeName = SOPC_NodeId_ToCString(pzNodeId);
                                    displaySetCursorLocation(14, yPos);
                                    yPos++;
                                    printf("- Node [%s]", sNodeName);
                                    SOPC_Free(sNodeName);
                                }
                            }
                        }
                    }
                }
            }
        }
        yPos++;
        displayPopPosition();
    }
    if (zDisplayContent.isModified)
    {
        //    displayPushPosition();
        displayStatus(1, "Command:");
        zDisplayContent.isModified = false;
    }
    fflush(0);
}

/*===========================================================================*/
static void displayGotoWin2(void)
{
    displaySetCursorLocation(1, zterminalWinsize.ws_row - displayNbLinesStatus - displayNbLinesWin2);
}

/*===========================================================================*/
static void displayStatus(const UAS_UInt32 x0, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    displaySetCursorLocation(x0, zterminalWinsize.ws_row - displayNbLinesStatus);
    vprintf(fmt, args);
    va_end(args);
}

/*============================================================================
 * LOCAL PROTOTYPES
 *===========================================================================*/

/**
 * \brief execute a command requested by interactive thread
 * \return true in case of success
 */
static bool SafetyDemo_interactive_process(SafetyDemo_interactive_Context* pContext, const char* input);

static bool doHelp(SafetyDemo_interactive_Context* pContext, const char* params);
static bool doQuit(SafetyDemo_interactive_Context* pContext, const char* params);
static bool doSleep(SafetyDemo_interactive_Context* pContext, const char* params);
static bool doBlock(SafetyDemo_interactive_Context* pContext, const char* params);
static bool doChangeAck(SafetyDemo_interactive_Context* pContext, const char* params);
static bool doChangeEnable(SafetyDemo_interactive_Context* pContext, const char* params);
static bool doEditSafeContent(SafetyDemo_interactive_Context* pContext, const char* params);
static bool doListCache(SafetyDemo_interactive_Context* pContext, const char* params);
static bool doTest1(SafetyDemo_interactive_Context* pContext, const char* params);

/*============================================================================
 * LOCAL TYPES
 *===========================================================================*/
/**
 * A user-event callback
 * \param pContext The execution context
 * \param params The parameters following the command char.
 * \return true if command succeeded
 */
typedef bool (*Interactive_Callback)(SafetyDemo_interactive_Context* pContext, const char* params);

/**
 * Configuration of a user event.
 */
typedef struct
{
    Interactive_Callback callback;
    const char* descr;
} Interactive_CallbackCfg;

/**
 * List of all possible callback (indexed on unsigned char)
 */
static Interactive_CallbackCfg g_callbackList[0x100];

/**
 * \brief Registers a user interactive callback.
 * \param key The char key which triggers that command
 * \param callback The user-event to be called when activated
 * \param descr The command short description (that can be used to generate HELP text)
 */
static void Create_Callback(const char key, Interactive_Callback callback, const char* descr);

/*============================================================================
 * IMPLEMENTATION
 *===========================================================================*/
#include <unistd.h>
#define STDIN 0
#define USER_ENTRY_MAXSIZE (128u)
/*===========================================================================*/
void SafetyDemo_Interactive_execute(SafetyDemo_interactive_Context* pContext)
{
    assert(NULL != pContext);
    SOPC_SocketSet fdSet;
    int maxfd = STDIN;
    int result = 0;
    ssize_t nbRead = 0;
    bool bResult = false;

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
        pContext->stopSignal = 1;
    }

    if (SOPC_SocketSet_IsPresent(STDIN, &fdSet))
    {
        displayClearWin2();
        nbRead = read(STDIN, entry, USER_ENTRY_MAXSIZE - 1);
        while (nbRead > 0 && entry[nbRead - 1] < ' ')
        {
            entry[nbRead - 1] = 0;
            nbRead--;
        }
        if (nbRead > 0)
        {
            displayClearScreen();
            displayGotoWin2();
            entry[nbRead] = 0;
            bResult = SafetyDemo_interactive_process(pContext, entry);
            if (!bResult)
            {
                printf("Error while processing command \"%s\"\n", entry);
                doHelp(NULL, NULL);
            }
        }
        displaySetModified();
    }
    displayExecute(pContext);
}

/*===========================================================================*/
void SafetyDemo_Interactive_Initialize(SafetyDemo_interactive_Context* pzContext)
{
    assert(NULL != pzContext);
    int i = 0;
    for (i = 0; i < 0x100; i++)
    {
        g_callbackList[i].callback = NULL;
        g_callbackList[i].descr = NULL;
    }
    Create_Callback('h', doHelp, "This help");
    Create_Callback('q', doQuit, "Quit");
    Create_Callback('s', doSleep, "Sleep, param=<ms>");
    Create_Callback('t', doTest1, "JCH TODO"); // TODO
    Create_Callback('l', doListCache, "List cache content");
    Create_Callback('b', doBlock, "Block/Unblock refresh");
    Create_Callback('a', doChangeAck, "Acknowledge");
    if (pzContext->isProvider)
    {
        Create_Callback('e', doEditSafeContent, "Edit Safe content");
    }
    else
    {
        Create_Callback('e', doChangeEnable, "Switch ENABLE flag");
    }

    displayInit();
    printf("Interactive session initialized. Press 'h' for help\n");
}

/*===========================================================================*/
void SafetyDemo_Interactive_Clear(void)
{
    displayTerminate();
}

/*===========================================================================*/
void SafetyDemo_Interactive_ForceRefresh(void)
{
    zDisplayContent.isModified = true;
}

/*===========================================================================*/
static void Create_Callback(const char key, Interactive_Callback callback, const char* descr)
{
    unsigned char uKey;
    assert(callback != NULL && descr != NULL);
    assert(key > 0);
    uKey = (unsigned char) key;
    g_callbackList[uKey].callback = callback;
    g_callbackList[uKey].descr = descr;
}

/*===========================================================================*/
static bool SafetyDemo_interactive_process(SafetyDemo_interactive_Context* pContext, const char* input)
{
    assert(pContext != NULL);
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
    if (cfg->callback != NULL)
    {
        result = (*cfg->callback)(pContext, input);
    }

    SafetyDemo_Interactive_ForceRefresh();
    return result;
}

/*===========================================================================*/
static bool doHelp(SafetyDemo_interactive_Context* pContext, const char* params)
{
    (void) pContext;
    (void) params;
    printf("help:\n");

    unsigned int i = 0;
    for (i = 0; i < 0x100; i++)
    {
        Interactive_CallbackCfg* cfg = &g_callbackList[i];
        if (cfg->descr != NULL)
        {
            printf("%c     :%s\n", (char) i, cfg->descr);
        }
    }
    return true;
}

/*===========================================================================*/
static bool doQuit(SafetyDemo_interactive_Context* pContext, const char* params)
{
    (void) pContext;
    (void) params;
    printf("Interactive exit requested\n");
    pContext->stopSignal = 1;
    return true;
}

/*===========================================================================*/
// May be usefull for scripting ...
static bool doSleep(SafetyDemo_interactive_Context* pContext, const char* params)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    uint16_t timeMs;

    (void) pContext;

    result = SOPC_strtouint16_t(params, &timeMs, 10, 0);
    if (SOPC_STATUS_OK == result)
    {
        printf("Sleep %d ms\n", timeMs);
        SOPC_Sleep(timeMs);
    }
    return true;
}

/*===========================================================================*/
static bool doChangeAck(SafetyDemo_interactive_Context* pContext, const char* params)
{
    (void) params;
    (void) pContext;
    SafetyDemo_DoAck();
    return true;
}

/*===========================================================================*/
static bool doBlock(SafetyDemo_interactive_Context* pContext, const char* params)
{
    (void) pContext;
    (void) params;
    zDisplayContent.isBlocked = !zDisplayContent.isBlocked;
    return true;
}

/*===========================================================================*/
static bool doListCache(SafetyDemo_interactive_Context* pContext, const char* params)
{
    (void) pContext;
    (void) params;
    UAM_Cache_Lock();
    UAM_Cache_List();
    UAM_Cache_Unlock();
    return true;
}

/*===========================================================================*/
static bool doEditSafeContent(SafetyDemo_interactive_Context* pContext, const char* params)
{
    (void) pContext;

    UAS_UInt8 pos = 0;
    UAS_UInt8 pos2 = 0;
    while (params[pos] != 0 && params[pos] != ' ')
    {
        pos++;
    }
    pos2 = pos;
    while (params[pos2] == ' ')
    {
        pos2++;
    }
    displayClearWin2();
    displayGotoWin2();
    printf("Modifying SAFE data content\n");
    if (params[pos] == ' ')
    {
        const int uParam2 = atoi(&params[pos2]);
        if (strncmp(params, "b1", strlen("b1")) == 0)
        {
            printf("Set B1 to %u\n", uParam2);
            SafetyDemo_SetSafetyDataB1(uParam2);
        }
        else if (strncmp(params, "b2", strlen("b2")) == 0)
        {
            printf("Set B2 to %u\n", uParam2);
            SafetyDemo_SetSafetyDataB2(uParam2);
        }
        else if (strncmp(params, "v1", strlen("v1")) == 0)
        {
            printf("Set V1 to %u\n", uParam2);
            SafetyDemo_SetSafetyDataV1((uint8_t) uParam2);
        }
        else if (strncmp(params, "v2", strlen("v2")) == 0)
        {
            printf("Set v2 to %u\n", uParam2);
            SafetyDemo_SetSafetyDataV2((uint8_t) uParam2);
        }
        else if (strncmp(params, "txt", strlen("txt")) == 0)
        {
            char txt[10] = {0};
            strncpy(txt, &params[pos2], 9);
            printf("Set Txt to <%s>\n", txt);
            SafetyDemo_SetSafetyDataTxt(&params[pos2]);
        }
        else
        {
            printf("Unknown param=%s\n", params);
        }
    }
    else
    {
        printf("Syntax : e <fieldname> <value>\n");
    }

    return true;
}

/*===========================================================================*/
static bool doChangeEnable(SafetyDemo_interactive_Context* pContext, const char* params)
{
    (void) pContext;
    (void) params;
    SafetyDemo_SwitchEnableFlag();
    SafetyDemo_Interactive_ForceRefresh();
    return true;
}

/*===========================================================================*/
static void autotestResponseMsg(void)
{
    SOPC_ReturnStatus bStatus = SOPC_STATUS_OK;
    UAS_ResponseSpdu_type response;

    static int x = 10;
    x++;

    bStatus = UAM_SpduEncoder_GetResponse(NODEID_SPDU_RESPONSE_NUM, &response, NULL, NULL);
    assert(bStatus == SOPC_STATUS_OK);

    printf("Current RESPONSE= [F=%02Xh, CONSID=%08Xh, MNR = %d, CRC=%08Xh]\n", response.byFlags,
           response.dwSafetyConsumerId, response.dwMonitoringNumber, response.dwCrc);
}

/*===========================================================================*/
static void autotestRequestMsg(void)
{
    UAS_RequestSpdu_type zRequest;
    SOPC_ReturnStatus result = SOPC_STATUS_NOK;
    const UAS_UInt32 dwHandle = NODEID_SPDU_REQUEST_NUM;
    result = UAM_SpduEncoder_GetRequest(dwHandle, &zRequest);
    assert(result == SOPC_STATUS_OK);
    printf("Send MNR=%X\n", zRequest.dwMonitoringNumber);
}

/*===========================================================================*/
static bool doTest1(SafetyDemo_interactive_Context* pContext, const char* params)
{
    SOPC_NodeId nid;
    SOPC_DataValue* dv = NULL;
    SOPC_ReturnStatus result = SOPC_STATUS_NOK;
    int i;
    (void) pContext;
    (void) params;

    SOPC_NodeId_Initialize(&nid);
    char nodeName[20];

    UAM_Cache_Lock();
    if (params[0] == 0)
    {
        if (pContext->isProvider)
        {
            autotestResponseMsg();
        }
        else
        {
            autotestRequestMsg();
        }
        result = SOPC_STATUS_OK;
    }
    if (result != SOPC_STATUS_OK)
    {
        sprintf(nodeName, "ns=1;i=%s", params);
        params = nodeName;
        result = SOPC_NodeId_InitializeFromCString(&nid, params, (int32_t)(strlen(params)));
        if (result != SOPC_STATUS_OK)
        {
            printf("Bad node Id name : <%s>, res= %d \n", params, result);
        }
        else
        {
            dv = UAM_Cache_Get(&nid);
            assert(dv != NULL);
            if (SOPC_UInt32_Id == dv->Value.BuiltInTypeId)
            {
                dv->Value.Value.Uint32++;
                printf("Changed %s to %u \n", params, dv->Value.Value.Uint32);
            }
            else if (SOPC_ByteString_Id == dv->Value.BuiltInTypeId)
            {
                if (dv->Value.Value.Bstring.Length < 5)
                {
                    SOPC_Free(dv->Value.Value.Bstring.Data);
                    dv->Value.Value.Bstring.Length = 5;
                    dv->Value.Value.Bstring.Data = SOPC_Malloc((size_t) dv->Value.Value.Bstring.Length);
                }
                static int offset = 0;
                for (i = 0; i < dv->Value.Value.Bstring.Length; i++)
                {
                    dv->Value.Value.Bstring.Data[i] = (SOPC_Byte)(i + offset);
                }
                offset++;
                dv->Value.Value.Bstring.DoNotClear = false;
                printf("Changed %s to", params);
                UAM_Cache_Show(&nid);
            }
            else
            {
                printf("Don't know how to change type %d \n", dv->Value.BuiltInTypeId);
            }
            dv->Status = SOPC_GoodGenericStatus;
        }
    }
    UAM_Cache_Unlock();

    SOPC_NodeId_Clear(&nid);

    return result == SOPC_STATUS_OK;
}
