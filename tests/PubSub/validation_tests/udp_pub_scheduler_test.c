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
#include <stdlib.h>

#include "sopc_atomic.h"
#include "sopc_dataset_layer.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_mem_alloc.h"
#include "sopc_network_layer.h"
#include "sopc_pubsub_conf.h"
#include "sopc_time.h"
#include "sopc_udp_sockets.h"
#include "sopc_xml_loader.h"

#include "sopc_pub_scheduler.h"

static int32_t stopPublisher = false;

static void Test_StopSignal(int sig)
{
    /* avoid unused parameter compiler warning */
    (void) sig;

    if (SOPC_Atomic_Int_Get(&stopPublisher) != false)
    {
        exit(1);
    }
    else
    {
        SOPC_Atomic_Int_Set(&stopPublisher, true);
    }
}

#define NB_VARS 7

SOPC_Variant varArr[NB_VARS] = {
    {true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 12071982}},    // 0
    {true, SOPC_Byte_Id, SOPC_VariantArrayType_SingleValue, {.Byte = 239}},             // 1
    {true, SOPC_Int16_Id, SOPC_VariantArrayType_SingleValue, {.Int16 = 5462}},          // 2
    {true, SOPC_Float_Id, SOPC_VariantArrayType_SingleValue, {.Floatv = (float) 0.12}}, // 3
    {true, SOPC_UInt32_Id, SOPC_VariantArrayType_SingleValue, {.Uint32 = 369852}},      // 4
    {true, SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = true}},      // 5
    {true, SOPC_Boolean_Id, SOPC_VariantArrayType_SingleValue, {.Boolean = false}}      // 6
};

static SOPC_ReturnStatus GetSourceVariablesRequest(
    SOPC_EventHandler* eventHandler, // message queue where response must be sent
    uintptr_t msgCtx,                // messageCtx, used by scheduler when it received response
    OpcUa_ReadValueId* lrv,
    int32_t nbValues)
{
    // Note: Return result is mandatory. If SOPC_STATUS_OK is not returned, then
    // READY status is set to messageCtx. So, request can be performed.
    // Else, BUSY is set to messageCtx. Next request will be ignored.
    // This is important to avoid memory issue in case of
    // treatment of request by services thread takes a long time.

    if (NULL == lrv || 0 >= nbValues)
    {
        return SOPC_STATUS_NOK;
    }

    SOPC_PubSheduler_GetVariableRequestContext* requestContext =
        SOPC_Calloc(1, sizeof(SOPC_PubSheduler_GetVariableRequestContext));

    if (NULL == requestContext)
    {
        return SOPC_STATUS_NOK;
    }
    requestContext->msgCtxt = msgCtx;            // Message context, forward by "0" timer event
    requestContext->eventHandler = eventHandler; // Message queue
    requestContext->ldv = NULL;                  // Datavalue request result
    requestContext->NoOfNodesToRead = nbValues;  // Use to alloc SOPC_DataValue by GetResponse

    /* Simulate response */
    assert(nbValues <= NB_VARS);
    assert(0 < nbValues);
    requestContext->ldv = SOPC_Calloc((size_t) nbValues, sizeof(*requestContext->ldv));
    assert(NULL != requestContext->ldv);
    for (int32_t i = 0; i < nbValues; i++)
    {
        SOPC_DataValue* dataValue = &requestContext->ldv[i];
        SOPC_DataValue_Initialize(dataValue);

        OpcUa_ReadValueId* readValue = &lrv[i];
        uint32_t index = readValue->NodeId.Data.Numeric;
        assert(13 == readValue->AttributeId); // Value => AttributeId=13
        assert(SOPC_IdentifierType_Numeric == readValue->NodeId.IdentifierType);
        assert(1 == readValue->NodeId.Namespace);
        // index node id
        assert(NB_VARS > index);
        dataValue->Value.ArrayType = varArr[index].ArrayType;
        dataValue->Value.BuiltInTypeId = varArr[index].BuiltInTypeId;
        dataValue->Value.Value = varArr[index].Value;

        OpcUa_ReadValueId_Clear(lrv);
    }
    SOPC_Free(lrv);

    SOPC_PubScheduler_EnqueueComEvent(SOPC_PUBSCHEDULER_EVENT_PUBLISH_RESPONSE, 0, (uintptr_t) requestContext, 0);

    return SOPC_STATUS_OK;
}

static SOPC_DataValue* GetSourceVariablesResponse(SOPC_PubSheduler_GetVariableRequestContext* requestResponse)
{
    SOPC_DataValue* ldv = NULL;

    if (NULL == requestResponse)
    {
        return NULL;
    }

    SOPC_PubSheduler_GetVariableRequestContext* requestContext =
        (SOPC_PubSheduler_GetVariableRequestContext*) requestResponse;

    if (NULL == requestResponse->ldv)
    {
        SOPC_Free(requestContext);
        return NULL;
    }

    ldv = requestContext->ldv;

    SOPC_Free(requestContext);

    return ldv;
}

/* Give XML file name as unique parameter
   If there is no parameter, default file name is config_pub_scheduler.xml
*/
int main(int argc, char** argv)
{
    char* filename;
    if (1 < argc)
    {
        filename = argv[1];
    }
    else
    {
        filename = "./config_pub_scheduler.xml";
    }

    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, Test_StopSignal);
    signal(SIGTERM, Test_StopSignal);

    // Get XML configuration
    FILE* fd = fopen(filename, "r");
    assert(NULL != fd);
    SOPC_PubSubConfiguration* config = SOPC_PubSubConfig_ParseXML(fd);
    int closed = fclose(fd);
    assert(0 == closed);
    assert(NULL != config);

    // Get Source Configuration
    SOPC_PubSourceVariableConfig* sourceConfig =
        SOPC_PubSourceVariableConfig_Create(GetSourceVariablesRequest, GetSourceVariablesResponse);
    assert(NULL != sourceConfig);

    // Start
    bool status = SOPC_PubScheduler_Start(config, sourceConfig);
    assert(status);

    // Wait until sending some message
    // SOPC_Sleep(5000);
    while (SOPC_Atomic_Int_Get(&stopPublisher) == false)
    {
        SOPC_Sleep(100);
    }

    // Stop
    SOPC_PubScheduler_Stop();
    SOPC_PubScheduler_Finalize();

    // Clear config
    SOPC_PubSubConfiguration_Delete(config);
    SOPC_PubSourceVariableConfig_Delete(sourceConfig);

    return 0;
}
