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

/* FreeRTOS include */
#include "FreeRTOS.h"
#include "task.h" // Used for disable task entering in user assert
#include "sys_evt.h" // Used to signal when MxNet has finished configuring Network interface

/* MIMXRT1064 includes */
//#include "fsl_debug_console.h"
//#include "p_ethernet_if.h"
#include "subscriber.h"

/* S2OPC includes */
#include "assert.h"
#include "cache.h"
#include "config_subscriber/pubsub_config_static.h"
#include "sopc_common.h"
#include "sopc_mem_alloc.h"
#include "sopc_pubsub_conf.h"
#include "sopc_pubsub_local_sks.h"
#include "sopc_sub_scheduler.h"
#include "sopc_time.h"
#include "static_security_data.h"

#include "sopc_macros.h"

#define SUBSCRIBER_THREAD_PRIORITY 3
#define SLEEP_TIMEOUT 100

/* Target callback */
bool set_target_variable(OpcUa_WriteValue* nodesToWrite, int32_t nbValues);
void SOPC_Variant_Print_U5(SOPC_Variant* pvar);

static void log_userCallback(const char* context, const char* text)
{
    SOPC_UNUSED_ARG(context);
    if (NULL != text)
    {
        LogInfo("%s\n", text);
    }
}

static void assert_userCallback(const char* context)
{
    LogInfo("ASSERT FAILED : <%p>\r\n", (void*) context);
    LogInfo("Context: <%s>", context);
    taskDISABLE_INTERRUPTS();
    for (;;)
        ;
}

void cbToolkit_subscriber(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Block until the network interface is connected */
    ( void ) xEventGroupWaitBits( xSystemEvents,
                                  EVT_MASK_NET_CONNECTED,
                                  0x00,
                                  pdTRUE,
                                  portMAX_DELAY );

    vTaskDelay(5000);

    //eEthernetIfResult ethResult = ETHERNET_IF_RESULT_NOK;
    SOPC_PubSubConfiguration* config = NULL;
    bool res = false;

    // Set user assert
    SOPC_Assert_Set_UserCallback(&assert_userCallback);

    /* Wait for ethernet module initialisation to start */
//    while (ETHERNET_IF_RESULT_OK != ethResult)
//    {
//        ethResult = P_ETHERNET_IF_IsReady(1000);
//    }

    /* Initialize toolkit and configure logs */
    const SOPC_Log_Configuration logConfig = {.logLevel = SOPC_LOG_LEVEL_DEBUG,
                                              .logSystem = SOPC_LOG_SYSTEM_USER,
                                              .logSysConfig = {.userSystemLogConfig = {.doLog = &log_userCallback}}};
    status = SOPC_Common_Initialize(logConfig);

    if (SOPC_STATUS_OK == status)
    {
        config = SOPC_PubSubConfig_GetStatic();
        if (NULL == config)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* Configure data target handler */
    SOPC_SubTargetVariableConfig* targetConfig = NULL;
    if (SOPC_STATUS_OK == status)
    {
        targetConfig = SOPC_SubTargetVariableConfig_Create(&set_target_variable); //&Cache_SetTargetVariables (put in cache) //&set_target_variable (print)
        if (NULL == targetConfig)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Initialize key bunch for sign or sign and encrypt communication */
        SOPC_KeyBunch_init_static(pubSub_keySign, sizeof(pubSub_keySign), pubSub_keyEncrypt, sizeof(pubSub_keyEncrypt),
                                  pubSub_keyNonce, sizeof(pubSub_keyNonce));

        res = Cache_Initialize(config);
        if (!res)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        res = SOPC_SubScheduler_Start(config, targetConfig, NULL, NULL, SUBSCRIBER_THREAD_PRIORITY);
        if (!res)
        {
            status = SOPC_STATUS_NOK;
        }
    }
    /* Wait for a signal */
    while (SOPC_STATUS_OK == status)
    {
        SOPC_Sleep(SLEEP_TIMEOUT);
    }

    /* Clean and quit */
    SOPC_SubScheduler_Stop();
    SOPC_SubTargetVariableConfig_Delete(targetConfig);
    SOPC_PubSubConfiguration_Delete(config);
    SOPC_Common_Clear();
    Cache_Clear();
}


/* Target callback */
bool set_target_variable(OpcUa_WriteValue* nodesToWrite, int32_t nbValues)
{
    bool ok = true;

    for (int i = 0; i < nbValues; i++)
    {
        OpcUa_WriteValue* wv = &nodesToWrite[i];
        SOPC_DataValue* dv = &wv->Value;

        /* Print Variant from nodes that have been received */
        // BEGIN
        if (SOPC_VariantArrayType_SingleValue == dv->Value.ArrayType)
        {
            switch (dv->Value.BuiltInTypeId)
            {
            case SOPC_Int16_Id:
                LogInfo("New value of dv->Value.Value.Int16 = %d\n", dv->Value.Value.Int16);
                break;
            case SOPC_UInt16_Id:
                LogInfo("New value of dv->Value.Value.Uint16 = %d\n", dv->Value.Value.Uint16);
                break;
            case SOPC_Int32_Id:
                LogInfo("New value of dv->Value.Value.Int32 = %d\n", dv->Value.Value.Int32);
                break;
            case SOPC_UInt32_Id:
                LogInfo("New value of dv->Value.Value.Uint32 = %d\n", dv->Value.Value.Uint32);
                break;
            case SOPC_Int64_Id:
                LogInfo("New value of dv->Value.Value.Int64 = %ld\n", dv->Value.Value.Int64);
                break;
            case SOPC_UInt64_Id:
                LogInfo("New value of dv->Value.Value.Uint64 = %ld\n", dv->Value.Value.Uint64);
                break;
            case SOPC_String_Id:
                /* Variant is a SOPC_String. You can add some text to Variant.Value.String */
                // BEGIN
                SOPC_Variant_Print_U5(&dv->Value);
                // END
                break;
            default:
                SOPC_Variant_Print_U5(&dv->Value);
                break;
            }
        }


        // END

        /* As we have ownership of the wv, clear it */
        OpcUa_WriteValue_Clear(wv);
    }

    SOPC_Free(nodesToWrite);

    return ok;
}


void SOPC_Variant_Print_U5(SOPC_Variant* pvar)
{
    size_t i;
    uint8_t c;
    //LogDebug("Variant @%p", (void*) pvar);
    char* s = NULL;

    if (NULL == pvar)
        return;

    int32_t dimensions = -1;
    switch (pvar->ArrayType)
    {
    case SOPC_VariantArrayType_SingleValue:
        dimensions = 0;
        break;
    case SOPC_VariantArrayType_Array:
        dimensions = 1;
        break;
    case SOPC_VariantArrayType_Matrix:
        dimensions = pvar->Value.Matrix.Dimensions;
        break;
    default:
        break;
    }

    //LogDebug(":\n  TypeId %i [dim=%" PRIi32 "]: ", (int) pvar->BuiltInTypeId, dimensions);

    if (0 != dimensions)
    {
    	//LogDebug("[...]\n");
        return;
    }

    switch (pvar->BuiltInTypeId)
    {
    case SOPC_Null_Id:
        LogInfo("Null\n");
        break;
    case SOPC_Boolean_Id:
        LogInfo("Boolean : Value: %" PRIu8 "\n", pvar->Value.Boolean);
        break;
    case SOPC_SByte_Id:
        LogInfo("SByte : Value: %" PRIi8 "\n", pvar->Value.Sbyte);
        break;
    case SOPC_Byte_Id:
        LogInfo("Byte : Value: %" PRIu8 "\n", pvar->Value.Byte);
        break;
    case SOPC_Int16_Id:
        LogInfo("Int16 : Value: %" PRIi16 "\n", pvar->Value.Int16);
        break;
    case SOPC_UInt16_Id:
        LogInfo("UInt16 : Value: %" PRIu16 "\n", pvar->Value.Uint16);
        break;
    case SOPC_Int32_Id:
        LogInfo("Int32 : Value: %" PRIi32 "\n", pvar->Value.Int32);
        break;
    case SOPC_UInt32_Id:
        LogInfo("UInt32 : Value: %" PRIu32 "\n", pvar->Value.Uint32);
        break;
    case SOPC_Int64_Id:
        LogInfo("Int64 : Value: %" PRIi64 "\n", pvar->Value.Int64);
        break;
    case SOPC_UInt64_Id:
        LogInfo("UInt64 : Value: %" PRIu64 "\n", pvar->Value.Uint64);
        break;
    case SOPC_Float_Id:
        LogInfo("Float : Value: %g\n", pvar->Value.Floatv);
        break;
    case SOPC_Double_Id:
        LogInfo("Double : Value: %g\n", pvar->Value.Doublev);
        break;
    case SOPC_String_Id:
        LogInfo("String : Value: \"%*.*s\"\n", pvar->Value.String.Length, pvar->Value.String.Length,
                            pvar->Value.String.Data);
        break;
    case SOPC_ByteString_Id:
        LogInfo("ByteString : Length: %" PRIi32 "\n  Value: \"", pvar->Value.Bstring.Length);
        /* Pretty print */
        for (i = 0; i < (size_t) pvar->Value.Bstring.Length; ++i)
        {
            c = pvar->Value.Bstring.Data[i];
            if (0x20 <= c && c < 0x80)
                /* Displayable ascii range */
                LogInfo("%c", c);
            else
                /* Special char */

            	LogInfo("\\x%02" PRIX8, c);
        }
        LogInfo("\"\n");
        break;
    case SOPC_XmlElement_Id:
        LogInfo("XmlElement : Length: %" PRIi32 "\n  Value: \"", pvar->Value.XmlElt.Length);
        /* Pretty print */
        for (i = 0; i < (size_t) pvar->Value.XmlElt.Length; ++i)
        {
            c = pvar->Value.XmlElt.Data[i];
            if (0x20 <= c && c < 0x80)
                /* Displayable ascii range */
                LogInfo("%c", c);
            else
                /* Special char */
                LogInfo("\\x%02" PRIX8, c);
        }
        LogInfo("\"\n");
        break;
    case SOPC_NodeId_Id:
        s = SOPC_NodeId_ToCString(pvar->Value.NodeId);
        LogInfo("NodeId : Value: %s\n", s);
        SOPC_Free(s);
        s = NULL;
        break;
    case SOPC_StatusCode_Id:
        LogInfo("StatusCode : Value: %" PRIX32 "\n", pvar->Value.Status);
        break;
    case SOPC_DateTime_Id:
        s = SOPC_Time_GetString(pvar->Value.Date, true, false);
        LogInfo("DateTime = %s\n", s);
        SOPC_Free(s);
        s = NULL;
        break;
    case SOPC_Guid_Id:
        s = SOPC_Guid_ToCString(pvar->Value.Guid);
        LogInfo("Guid = '%s'\n", s);
        SOPC_Free(s);
        s = NULL;
        break;
    case SOPC_ExpandedNodeId_Id:
        s = SOPC_NodeId_ToCString(&pvar->Value.ExpNodeId->NodeId);
        LogInfo("ExpandedNodeId : NsURI: %s\n  NodeId: %s\n  ServerIdx: %" PRIu32 "\n",
                            SOPC_String_GetRawCString(&pvar->Value.ExpNodeId->NamespaceUri), s,
                            pvar->Value.ExpNodeId->ServerIndex);
        SOPC_Free(s);
        s = NULL;
        break;
    case SOPC_QualifiedName_Id:
        LogInfo("QualifiedName = %" PRIu16 ":%s\n", pvar->Value.Qname->NamespaceIndex,
                            SOPC_String_GetRawCString(&pvar->Value.Qname->Name));
        break;
    case SOPC_LocalizedText_Id:
        LogInfo("LocalizedText (default only) = [%s] %s\n",
                            SOPC_String_GetRawCString(&pvar->Value.LocalizedText->defaultLocale),
                            SOPC_String_GetRawCString(&pvar->Value.LocalizedText->defaultText));
        break;
    case SOPC_ExtensionObject_Id:
        LogInfo("ExtensionObject: <print not implemented>\n");
        break;
    case SOPC_DataValue_Id:
        LogInfo("DataValue: <print not implemented>\n");
        break;
    case SOPC_Variant_Id: /* This one does not have an implementation at all */
        LogInfo("Variant: <print not implemented>\n");
        break;
    case SOPC_DiagnosticInfo_Id:
        LogInfo("DiagnosticInfo: <print not implemented>\n");
        break;
    default:
        LogInfo("<print not implemented>\n");
        break;
    }
}
