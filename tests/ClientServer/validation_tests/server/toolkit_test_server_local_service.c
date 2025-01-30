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

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "opcua_Custom_identifiers.h"
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_address_space.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_encodeabletype.h"
#include "sopc_encoder.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_threads.h"

#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"
#include "libs2opc_server_config_custom.h"

#include "embedded/sopc_addspace_loader.h"

#include "test_results.h"
#include "testlib_read_response.h"
#include "testlib_write.h"

#include "custom_types.h"

#define XML_UA_NODESET_PATH "./S2OPC_Test_NodeSet.xml"

#define SOPC_PKI_PATH "./S2OPC_Demo_PKI"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"

static int32_t endpointClosed = false;
static int32_t nonRegReadWriteTest = false;
static int32_t nonRegWriteResponses = 0;

static uint32_t cptReadResps = 0;

static char* newNodeId = "NewNodeId";

static void SOPC_ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&endpointClosed, true);
}

static void SOPC_LocalServiceAsyncRespCallback(SOPC_EncodeableType* encType, void* response, uintptr_t appContext)
{
    if (true != SOPC_Atomic_Int_Get(&nonRegReadWriteTest))
    {
        if (encType == &OpcUa_ReadResponse_EncodeableType)
        {
            printf("<Test_Server_Local_Service: received local service ReadResponse \n");
            OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) response;
            cptReadResps++;
            // Check context value is same as those provided with request
            SOPC_ASSERT(cptReadResps == appContext);
            if (cptReadResps <= 1)
            {
                test_results_set_service_result(
                    test_read_request_response(readResp, readResp->ResponseHeader.ServiceResult, 0) ? true : false);
            }
            else
            {
                // Second read response is to test write effect (through read result)
                test_results_set_service_result(
                    tlibw_verify_response_remote(test_results_get_WriteRequest(), readResp));
            }
        }
        else if (encType == &OpcUa_WriteResponse_EncodeableType)
        {
            // Check context value is same as one provided with request
            SOPC_ASSERT(1 == appContext);
            printf("<Test_Server_Local_Service: received local service  WriteResponse \n");
            OpcUa_WriteResponse* writeResp = (OpcUa_WriteResponse*) response;
            test_results_set_service_result(tlibw_verify_response(test_results_get_WriteRequest(), writeResp));
        }
        else if (encType == &OpcUa_AddNodesResponse_EncodeableType)
        {
            // Check context value is same as one provided with request
            SOPC_ASSERT(1 == appContext);
            printf("<Test_Server_Local_Service: received local service AddNodeResponse \n");
            OpcUa_AddNodesResponse* addNodeResp = (OpcUa_AddNodesResponse*) response;
            // Check that a node with good NodeId has been added
            if (1 == addNodeResp->NoOfResults)
            {
                const char* nodeIdAdded = SOPC_String_GetRawCString(&addNodeResp->Results->AddedNodeId.Data.String);
                printf("NodeId of thee added node: '%s'\n", nodeIdAdded);
                test_results_set_service_result(0 == strcmp(nodeIdAdded, newNodeId));
            }
            else
            {
                test_results_set_service_result(false);
            }
        }
    }
    else
    {
        printf("<Test_Server_Local_Service: received local service ReadResponse (with concurrent write) \n");
        if (encType == &OpcUa_ReadResponse_EncodeableType)
        {
            OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) response;
            for (int32_t i = 0; i < readResp->NoOfResults; i++)
            {
                // Access the read value, absence of errors with ASAN activated shows issue #1544 is fixed
                char* stringContent = SOPC_String_GetCString(&readResp->Results[i].Value.Value.String);
                printf("Read string value: '%s'\n", stringContent);
                SOPC_Free(stringContent);
            }
        }
        else if (encType == &OpcUa_WriteResponse_EncodeableType)
        {
            // count number of write responses
            (void) SOPC_Atomic_Int_Add(&nonRegWriteResponses, 1);
        }
    }
}

// Non regression test for issue #1544 leading to access freed memory
// when using local read service on variable concurrently written
static void test_concurrent_write_read_non_reg(void)
{
    SOPC_Atomic_Int_Set(&nonRegReadWriteTest, true);
    const uint32_t sleepTimeout = 50;
    // Loop timeout in milliseconds
    uint32_t loopTimeout = 5000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    const size_t nbReadWrites = 5;
    const size_t nbReads = 1;
    // Note: we need a non-native type which has allocated memory that might be freed by server
    const SOPC_NodeId stringVar = SOPC_NODEID_NUMERIC(1, 1004);

    OpcUa_ReadRequest* readReq = NULL;
    OpcUa_WriteRequest* writeReq = NULL;
    SOPC_DataValue stringVarDv;
    SOPC_DataValue_Initialize(&stringVarDv);
    stringVarDv.Value.BuiltInTypeId = SOPC_String_Id;
    SOPC_String_Initialize(&stringVarDv.Value.Value.String);
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&stringVarDv.Value.Value.String, "Draw me a sheep");
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    for (size_t i = 0; i < nbReadWrites; i++)
    {
        // Prepare read <nbReads> times the string variable value
        readReq = SOPC_ReadRequest_Create(nbReads, OpcUa_TimestampsToReturn_Neither);
        SOPC_ASSERT(NULL != readReq);
        for (size_t j = 0; j < nbReads; j++)
        {
            status = SOPC_ReadRequest_SetReadValue(readReq, j, &stringVar, SOPC_AttributeId_Value, NULL);
            SOPC_ASSERT(SOPC_STATUS_OK == status);
        }
        // Prepare to write the string variable value
        writeReq = SOPC_WriteRequest_Create(1);
        SOPC_ASSERT(NULL != writeReq);
        status = SOPC_WriteRequest_SetWriteValue(writeReq, 0, &stringVar, SOPC_AttributeId_Value, NULL, &stringVarDv);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        // Call local services read and then write,
        // read is returning freed memory from address space on write prior fix
        status = SOPC_ServerHelper_LocalServiceAsync(readReq, 12);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
        status = SOPC_ServerHelper_LocalServiceAsync(writeReq, 42);
        SOPC_ASSERT(SOPC_STATUS_OK == status);
    }

    /* Wait until all write responses are received */
    loopCpt = 0;
    while ((int32_t) nbReadWrites != SOPC_Atomic_Int_Get(&nonRegWriteResponses) &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }
    SOPC_ASSERT(loopCpt * sleepTimeout <= loopTimeout);
    SOPC_Atomic_Int_Set(&nonRegReadWriteTest, false);
}

static bool checkGetEndpointsResponse(OpcUa_GetEndpointsResponse* getEndpointsResp)
{
    printf("<Test_Server_Local_Service: received GetEndpointsResponse \n");
    SOPC_String endpointUrl;
    SOPC_String_Initialize(&endpointUrl);
    SOPC_ReturnStatus testStatus = SOPC_String_AttachFromCstring(&endpointUrl, DEFAULT_ENDPOINT_URL);
    bool validEndpoints = true;

    if (testStatus != SOPC_STATUS_OK || getEndpointsResp->NoOfEndpoints <= 0)
    {
        // At least one endpoint shall be described with the correct endpoint URL
        validEndpoints = false;
    }
    for (int32_t idx = 0; idx < getEndpointsResp->NoOfEndpoints && validEndpoints != false; idx++)
    {
        validEndpoints = SOPC_String_Equal(&getEndpointsResp->Endpoints[idx].EndpointUrl, &endpointUrl);
    }

    OpcUa_GetEndpointsResponse_Clear(getEndpointsResp);
    SOPC_Free(getEndpointsResp);

    return validEndpoints;
}

/* Function to build the read service request message */
static void* getReadRequest_message(void)
{
    return read_new_read_request();
}

/* Function to build the verification read request */
static void* getReadRequest_verif_message(void)
{
    return tlibw_new_ReadRequest_check();
}

static SOPC_ReturnStatus addNodesForCustomDataTypeTests(SOPC_AddressSpace* address_space,
                                                        SOPC_AddressSpace_Node* varNodeCustomDT,
                                                        SOPC_AddressSpace_Node* customDefaultBinaryNode,
                                                        SOPC_AddressSpace_Node* customDTNode,
                                                        SOPC_AddressSpace_Node* varNodeCustomDT2,
                                                        SOPC_AddressSpace_Node* custom2DefaultBinaryNode,
                                                        SOPC_AddressSpace_Node* varNodeStructDT)
{
    // Init is necessary to manage correctly clear in case of read only / not read only address space
    SOPC_AddressSpace_Node_Initialize(address_space, varNodeCustomDT, OpcUa_NodeClass_Variable);
    SOPC_AddressSpace_Node_Initialize(address_space, customDefaultBinaryNode, OpcUa_NodeClass_Object);
    SOPC_AddressSpace_Node_Initialize(address_space, customDTNode, OpcUa_NodeClass_DataType);
    SOPC_AddressSpace_Node_Initialize(address_space, varNodeCustomDT2, OpcUa_NodeClass_Variable);
    SOPC_AddressSpace_Node_Initialize(address_space, custom2DefaultBinaryNode, OpcUa_NodeClass_Object);
    SOPC_AddressSpace_Node_Initialize(address_space, varNodeStructDT, OpcUa_NodeClass_Variable);

    if (SOPC_AddressSpace_AreReadOnlyNodes(address_space))
    {
        return SOPC_STATUS_OK;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Add a variable node with custom DataType
    OpcUa_VariableNode* varNode = &varNodeCustomDT->data.variable;
    OpcUa_VariableNode_Initialize(varNode);
    varNode->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = 421000};
    varNode->NodeClass = OpcUa_NodeClass_Variable;
    varNode->DataType = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_Custom_CustomDataType};
    varNode->ValueRank = -2; // Any
    status = SOPC_AddressSpace_Append(address_space, varNodeCustomDT);

    if (SOPC_STATUS_OK == status)
    {
        // Add the "Default Binary" node that references the custom DataType
        OpcUa_ObjectNode* objNode = &customDefaultBinaryNode->data.object;
        OpcUa_ObjectNode_Initialize(objNode);
        objNode->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1,
                                        .Data.Numeric = OpcUaId_Custom_CustomDataType_Encoding_DefaultBinary};
        objNode->NodeClass = OpcUa_NodeClass_Object;

        objNode->References = SOPC_Calloc(1, sizeof(OpcUa_ReferenceNode));
        SOPC_ASSERT(NULL != objNode->References);
        objNode->NoOfReferences = 1;
        OpcUa_ReferenceNode* refNode = &objNode->References[0];
        OpcUa_ReferenceNode_Initialize(refNode);
        refNode->ReferenceTypeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasEncoding};
        refNode->IsInverse = true;
        refNode->TargetId = (SOPC_ExpandedNodeId){
            {SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_Custom_CustomDataType}, {0}, 0};
        status = SOPC_AddressSpace_Append(address_space, customDefaultBinaryNode);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Add the "DataType" node that references the Structure DataType
        OpcUa_DataTypeNode* dtNode = &customDTNode->data.data_type;
        OpcUa_DataTypeNode_Initialize(dtNode);
        dtNode->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_Custom_CustomDataType};
        dtNode->NodeClass = OpcUa_NodeClass_DataType;

        dtNode->References = SOPC_Calloc(1, sizeof(OpcUa_ReferenceNode));
        SOPC_ASSERT(NULL != dtNode->References);
        dtNode->NoOfReferences = 1;
        OpcUa_ReferenceNode* refNode = &dtNode->References[0];
        OpcUa_ReferenceNode_Initialize(refNode);
        refNode->ReferenceTypeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasSubtype};
        refNode->IsInverse = true;
        refNode->TargetId =
            (SOPC_ExpandedNodeId){{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Structure}, {0}, 0};
        status = SOPC_AddressSpace_Append(address_space, customDTNode);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Add a variable node with custom DataType2
        OpcUa_VariableNode* varNode2 = &varNodeCustomDT2->data.variable;
        OpcUa_VariableNode_Initialize(varNode2);
        varNode2->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = 421000 * 2};
        varNode2->NodeClass = OpcUa_NodeClass_Variable;
        varNode2->DataType =
            (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_Custom_CustomDataType * 2};
        varNode2->ValueRank = -1;
        status = SOPC_AddressSpace_Append(address_space, varNodeCustomDT2);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Add the "Default Binary" node that references the custom 2 DataType
        // (without DataType node in nodeset => ok for variable with exact type)
        OpcUa_ObjectNode* objNode = &custom2DefaultBinaryNode->data.object;
        OpcUa_ObjectNode_Initialize(objNode);
        objNode->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1,
                                        .Data.Numeric = OpcUaId_Custom_CustomDataType_Encoding_DefaultBinary * 2};
        objNode->NodeClass = OpcUa_NodeClass_Object;

        objNode->References = SOPC_Calloc(1, sizeof(OpcUa_ReferenceNode));
        SOPC_ASSERT(NULL != objNode->References);
        objNode->NoOfReferences = 1;
        OpcUa_ReferenceNode* refNode = &objNode->References[0];
        OpcUa_ReferenceNode_Initialize(refNode);
        refNode->ReferenceTypeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasEncoding};
        refNode->IsInverse = true;
        refNode->TargetId = (SOPC_ExpandedNodeId){
            {SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_Custom_CustomDataType * 2}, {0}, 0};
        status = SOPC_AddressSpace_Append(address_space, custom2DefaultBinaryNode);
    }

    // Add a variable node with abstract Structure DT
    if (SOPC_STATUS_OK == status)
    {
        varNode = &varNodeStructDT->data.variable;
        OpcUa_VariableNode_Initialize(varNode);
        varNode->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = 421001};
        varNode->NodeClass = OpcUa_NodeClass_Variable;
        varNode->DataType = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Structure};
        varNode->ValueRank = -3; // ScalarOrOneDimension
        status = SOPC_AddressSpace_Append(address_space, varNodeStructDT);
    }
    return status;
}

static SOPC_ReturnStatus check_writeCustomDataType(const SOPC_AddressSpace* address_space,
                                                   SOPC_AddressSpace_Node* varNodeCustomDT,
                                                   SOPC_AddressSpace_Node* varNodeCustomDT2,
                                                   SOPC_AddressSpace_Node* varNodeStructDT)
{
    if (SOPC_AddressSpace_AreReadOnlyNodes(address_space))
    {
        return SOPC_STATUS_OK;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Create an extension object of an unreferenced encodeable type
    SOPC_ExtensionObject extObj;
    SOPC_ExtensionObject_Initialize(&extObj);

    OpcUa_Custom_CustomDataType* instCDT = NULL;

    status = SOPC_ExtensionObject_CreateObject(&extObj, &OpcUa_Custom_CustomDataType_EncodeableType, (void**) &instCDT);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    instCDT->fieldb = true;
    instCDT->fieldu = 1000;

    // Encode the extension object into a buffer (success since it contains direct reference to encType)
    SOPC_Buffer* buf = SOPC_Buffer_Create(1024);
    SOPC_ASSERT(NULL != buf);
    status = SOPC_ExtensionObject_Write(&extObj, buf, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    // Clear extension object content
    SOPC_ExtensionObject_Clear(&extObj);

    // Read the extension object from buffer (decoder not available => ByteString format retrieved)
    status = SOPC_Buffer_SetPosition(buf, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_ExtensionObject_Read(&extObj, buf, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_ASSERT(extObj.Encoding == SOPC_ExtObjBodyEncoding_ByteString);

    // Write the extension object into the node with compatible DataType (expecting OK)
    SOPC_DataValue dataValue;
    SOPC_DataValue_Initialize(&dataValue);
    dataValue.Value.BuiltInTypeId = SOPC_ExtensionObject_Id;
    dataValue.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dataValue.Value.Value.ExtObject = &extObj;
    OpcUa_WriteRequest* writeRequest = SOPC_WriteRequest_Create(8);
    SOPC_ASSERT(NULL != writeRequest);
    status = SOPC_WriteRequest_SetWriteValue(writeRequest, 0, &varNodeCustomDT->data.variable.NodeId,
                                             SOPC_AttributeId_Value, NULL, &dataValue);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    // Write the custom data type object into a compatible abstract DataType (expecting OK)
    status = SOPC_WriteRequest_SetWriteValue(writeRequest, 1, &varNodeStructDT->data.variable.NodeId,
                                             SOPC_AttributeId_Value, NULL, &dataValue);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    // Modify TypeId for custom 2 DT encoding Default Binary node
    extObj.TypeId.NodeId.Data.Numeric = OpcUaId_Custom_CustomDataType_Encoding_DefaultBinary * 2;

    // Write the extension object into the node with compatible DataType2 (expecting OK)
    status = SOPC_WriteRequest_SetWriteValue(writeRequest, 2, &varNodeCustomDT2->data.variable.NodeId,
                                             SOPC_AttributeId_Value, NULL, &dataValue);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    // Write the custom data type object into a compatible abstract DataType2 (expecting NOK)
    // note: type is compatible but since custom 2 DataType node is not present in nodeset
    //       we are not able to consider it a subtype of structure
    status = SOPC_WriteRequest_SetWriteValue(writeRequest, 3, &varNodeStructDT->data.variable.NodeId,
                                             SOPC_AttributeId_Value, NULL, &dataValue);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    SOPC_ExtensionObject_Clear(&extObj);
    SOPC_DataValue_Initialize(&dataValue);

    dataValue.Value.BuiltInTypeId = SOPC_ExtensionObject_Id;
    dataValue.Value.ArrayType = SOPC_VariantArrayType_Array;
    dataValue.Value.Value.Array.Length = 0;
    // Write an empty array into the node with compatible DataType (expecting OK)
    status = SOPC_WriteRequest_SetWriteValue(writeRequest, 4, &varNodeCustomDT->data.variable.NodeId,
                                             SOPC_AttributeId_Value, NULL, &dataValue);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    dataValue.Value.Value.Array.Length = 2;
    dataValue.Value.Value.Array.Content.ExtObjectArr = SOPC_Calloc(2, sizeof(SOPC_ExtensionObject));
    SOPC_ASSERT(NULL != dataValue.Value.Value.Array.Content.ExtObjectArr);

    status = SOPC_Buffer_SetPosition(buf, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_ExtensionObject_Read(&dataValue.Value.Value.Array.Content.ExtObjectArr[0], buf, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_Buffer_SetPosition(buf, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    status = SOPC_ExtensionObject_Read(&dataValue.Value.Value.Array.Content.ExtObjectArr[1], buf, 0);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    // Write an array with 2 elements with expected DataType into the node with compatible DataType (expecting OK)
    status = SOPC_WriteRequest_SetWriteValue(writeRequest, 5, &varNodeCustomDT->data.variable.NodeId,
                                             SOPC_AttributeId_Value, NULL, &dataValue);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    // Modify TypeId for custom 2 DT encoding Default Binary node for second element in array
    dataValue.Value.Value.Array.Content.ExtObjectArr[1].TypeId.NodeId.Data.Numeric =
        OpcUaId_Custom_CustomDataType_Encoding_DefaultBinary * 2;

    // Write an array with 2 elements with different DataType into the node with custom DataType (expecting NOK)
    status = SOPC_WriteRequest_SetWriteValue(writeRequest, 6, &varNodeCustomDT->data.variable.NodeId,
                                             SOPC_AttributeId_Value, NULL, &dataValue);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    // Write an array with 2 elements with different DataType into the node with Strucutre DataType (expecting OK)
    status = SOPC_WriteRequest_SetWriteValue(writeRequest, 7, &varNodeStructDT->data.variable.NodeId,
                                             SOPC_AttributeId_Value, NULL, &dataValue);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    SOPC_DataValue_Clear(&dataValue);

    OpcUa_WriteResponse* writeResp = NULL;
    status = SOPC_ServerHelper_LocalServiceSync(writeRequest, (void**) &writeResp);

    if (SOPC_STATUS_OK == status)
    {
        // [0]: Write custom object into same custom DataType variable node shall succeed
        // [1]: Write custom object into abstract Structure DataType will succeed because
        //      custom DataType node exists and is a subtype of abstract Structure DataType
        // [2]: Write custom object 2 into exact same custom 2 DataType variable node shall succeed
        // [3]: Write custom object 2 into abstract Structure DataType will fail since we cannot resolve
        //      it in nodeset (custom 2 DataType node is not present thus type resolution cannot succeed)
        // [4]: Write empty array into custom DataType variable node shall succeed
        // [5]: Write array with 2 custom objects into same custom DataType variable node shall succeed
        // [6]: Write array with 2 different DataType into custom Datatype variable node shall fail
        // [7]: Write array with 2 different DataType into abstract Structure Datatype variable node shall succeed
        if (SOPC_IsGoodStatus(writeResp->ResponseHeader.ServiceResult) && SOPC_IsGoodStatus(writeResp->Results[0]) &&
            SOPC_IsGoodStatus(writeResp->Results[1]) && SOPC_IsGoodStatus(writeResp->Results[2]) &&
            !SOPC_IsGoodStatus(writeResp->Results[3]) && SOPC_IsGoodStatus(writeResp->Results[4]) &&
            SOPC_IsGoodStatus(writeResp->Results[5]) && !SOPC_IsGoodStatus(writeResp->Results[6]) &&
            SOPC_IsGoodStatus(writeResp->Results[7]))
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
    }

    if (NULL != writeResp)
    {
        SOPC_EncodeableObject_Delete(writeResp->encodeableType, (void**) &writeResp);
    }
    SOPC_Buffer_Delete(buf);

    return status;
}

static SOPC_ReturnStatus check_readDataTypeDefinition(const SOPC_AddressSpace* address_space)
{
    if (SOPC_AddressSpace_AreReadOnlyNodes(address_space))
    {
        return SOPC_STATUS_OK;
    }
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_ReadRequest* readReq = SOPC_ReadRequest_Create(3, OpcUa_TimestampsToReturn_Neither);
    const SOPC_NodeId structureDTid = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ComplexNumberType);
    const SOPC_NodeId enumDTid = SOPC_NODEID_NS0_NUMERIC(OpcUaId_SecurityTokenRequestType);
    const SOPC_NodeId abstractStructureDTid = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Structure);

    status = SOPC_STATUS_OUT_OF_MEMORY;
    if (NULL != readReq)
    {
        status = SOPC_ReadRequest_SetReadValue(readReq, 0, &structureDTid, SOPC_AttributeId_DataTypeDefinition, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ReadRequest_SetReadValue(readReq, 1, &enumDTid, SOPC_AttributeId_DataTypeDefinition, NULL);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ReadRequest_SetReadValue(readReq, 2, &abstractStructureDTid, SOPC_AttributeId_DataTypeDefinition,
                                               NULL);
    }

    OpcUa_ReadResponse* readResp = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceSync(readReq, (void**) &readResp);
    }

    OpcUa_StructureDefinition* structureDT = NULL;
    OpcUa_EnumDefinition* enumDT = NULL;

    // Check: [0] and [1] have attributed defined and [2] has not (abstract structure type)
    if (SOPC_STATUS_OK == status && SOPC_IsGoodStatus(readResp->ResponseHeader.ServiceResult) &&
        SOPC_IsGoodStatus(readResp->Results[0].Status) && SOPC_IsGoodStatus(readResp->Results[1].Status) &&
        OpcUa_BadAttributeIdInvalid == readResp->Results[2].Status)
    {
        // Check: [0] is a StructureDefinition value and store it in structureDT
        if (SOPC_VariantArrayType_SingleValue == readResp->Results[0].Value.ArrayType &&
            SOPC_ExtensionObject_Id == readResp->Results[0].Value.BuiltInTypeId &&
            SOPC_ExtObjBodyEncoding_Object == readResp->Results[0].Value.Value.ExtObject->Encoding &&
            &OpcUa_StructureDefinition_EncodeableType ==
                readResp->Results[0].Value.Value.ExtObject->Body.Object.ObjType)
        {
            structureDT = (OpcUa_StructureDefinition*) readResp->Results[0].Value.Value.ExtObject->Body.Object.Value;
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        // Check:  [1] is an EnumDefinition value and store it in enumDT
        if (SOPC_STATUS_OK == status && SOPC_VariantArrayType_SingleValue == readResp->Results[1].Value.ArrayType &&
            SOPC_ExtensionObject_Id == readResp->Results[1].Value.BuiltInTypeId &&
            SOPC_ExtObjBodyEncoding_Object == readResp->Results[1].Value.Value.ExtObject->Encoding &&
            &OpcUa_StructureDefinition_EncodeableType ==
                readResp->Results[0].Value.Value.ExtObject->Body.Object.ObjType)
        {
            enumDT = (OpcUa_EnumDefinition*) readResp->Results[1].Value.Value.ExtObject->Body.Object.Value;
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }
    // Check structureDT content for ComplexNumberType
    if (SOPC_STATUS_OK == status)
    {
        const SOPC_NodeId complexNumberDTdefaultBinaryId = {
            SOPC_IdentifierType_Numeric, OPCUA_NAMESPACE_INDEX,
            .Data.Numeric = OpcUaId_ComplexNumberType_Encoding_DefaultBinary};
        const SOPC_NodeId floatDTid = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Float);

        if (!SOPC_NodeId_Equal(&structureDT->DefaultEncodingId, &complexNumberDTdefaultBinaryId))
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == status && !SOPC_NodeId_Equal(&structureDT->BaseDataType, &abstractStructureDTid))
        {
            status = SOPC_STATUS_NOK;
        }
        if (SOPC_STATUS_OK == status && OpcUa_StructureType_Structure == structureDT->StructureType &&
            2 != structureDT->NoOfFields)
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            OpcUa_StructureField* field = &structureDT->Fields[0];
            if (0 != strcmp("Real", SOPC_String_GetRawCString(&field->Name)))
            {
                status = SOPC_STATUS_NOK;
            }
            if (SOPC_STATUS_OK != status || !SOPC_NodeId_Equal(&field->DataType, &floatDTid))
            {
                status = SOPC_STATUS_NOK;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            OpcUa_StructureField* field = &structureDT->Fields[1];
            if (0 != strcmp("Imaginary", SOPC_String_GetRawCString(&field->Name)))
            {
                status = SOPC_STATUS_NOK;
            }
            if (SOPC_STATUS_OK != status || !SOPC_NodeId_Equal(&field->DataType, &floatDTid))
            {
                status = SOPC_STATUS_NOK;
            }
        }
    }

    // Check enumDT content for SecurityTokenRequestType
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_STATUS_OK == status && 2 != enumDT->NoOfFields)
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            OpcUa_EnumField* field = &enumDT->Fields[0];
            if (0 != strcmp("Issue", SOPC_String_GetRawCString(&field->Name)) || 0 != field->Value)
            {
                status = SOPC_STATUS_NOK;
            }
        }
        if (SOPC_STATUS_OK == status)
        {
            OpcUa_EnumField* field = &enumDT->Fields[1];
            if (0 != strcmp("Renew", SOPC_String_GetRawCString(&field->Name)) || 1 != field->Value)
            {
                status = SOPC_STATUS_NOK;
            }
        }
    }

    if (NULL != readResp)
    {
        SOPC_EncodeableObject_Delete(readResp->encodeableType, (void**) &readResp);
    }
    return status;
}

int main(int argc, char* argv[])
{
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);

    SOPC_AddressSpace_Node* varNodeCustomDT = SOPC_Calloc(1, sizeof(SOPC_AddressSpace_Node));
    SOPC_ASSERT(NULL != varNodeCustomDT);
    SOPC_AddressSpace_Node* customDefaultBinaryNode = SOPC_Calloc(1, sizeof(SOPC_AddressSpace_Node));
    SOPC_ASSERT(NULL != customDefaultBinaryNode);
    SOPC_AddressSpace_Node* customDTNode = SOPC_Calloc(1, sizeof(SOPC_AddressSpace_Node));
    SOPC_ASSERT(NULL != customDTNode);
    SOPC_AddressSpace_Node* varNodeCustomDT2 = SOPC_Calloc(1, sizeof(SOPC_AddressSpace_Node));
    SOPC_ASSERT(NULL != varNodeCustomDT2);
    SOPC_AddressSpace_Node* custom2DefaultBinaryNode = SOPC_Calloc(1, sizeof(SOPC_AddressSpace_Node));
    SOPC_ASSERT(NULL != custom2DefaultBinaryNode);
    SOPC_AddressSpace_Node* varNodeStructureDT = SOPC_Calloc(1, sizeof(SOPC_AddressSpace_Node));
    SOPC_ASSERT(NULL != varNodeStructureDT);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_WriteRequest* pWriteReqSent = NULL;
    OpcUa_WriteRequest* pWriteReqCopy = NULL;

    // Configure log
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_local_service_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Local_Service: Failed initializing\n");
    }
    else
    {
        printf("<Test_Server_Local_Service: initialized\n");
    }

    const uint32_t sleepTimeout = 50;
    // Loop timeout in milliseconds
    uint32_t loopTimeout = 5000;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    // Secu policy configuration
    if (SOPC_STATUS_OK == status)
    {
        SOPC_Endpoint_Config* ep = SOPC_ServerConfigHelper_CreateEndpoint(DEFAULT_ENDPOINT_URL, true);
        SOPC_SecurityPolicy* sp = SOPC_EndpointConfig_AddSecurityConfig(ep, SOPC_SecurityPolicy_Basic256Sha256);

        if (NULL == ep || NULL == sp)
        {
            SOPC_Free(ep);
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_SecurityConfig_SetSecurityModes(sp, SOPC_SecurityModeMask_SignAndEncrypt);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_SecurityConfig_AddUserTokenPolicy(sp, &SOPC_UserTokenPolicy_Anonymous);
        }
    }

    // Configure the callback
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }

    // Server certificates configuration
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetKeyCertPairFromPath("./server_public/server_2k_cert.der",
                                                                "./server_private/encrypted_server_2k_key.pem", true);
    }

    // Set PKI configuration
    if (SOPC_STATUS_OK == status)
    {
        SOPC_PKIProvider* pkiProvider = NULL;
        status = SOPC_PKIProvider_CreateFromStore(SOPC_PKI_PATH, &pkiProvider);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerConfigHelper_SetPKIprovider(pkiProvider);
        }
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Local_Service: Failed loading certificates and key (check paths are valid)\n");
    }
    else
    {
        printf("<Test_Server_Local_Service: Certificates and key loaded\n");
    }

    if (SOPC_STATUS_OK == status)
    {
        // Set namespaces
        const char* namespaces[] = {DEFAULT_APPLICATION_URI};
        status = SOPC_ServerConfigHelper_SetNamespaces(1, namespaces);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Local_Service: Failed setting namespaces\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetApplicationDescription(DEFAULT_APPLICATION_URI, DEFAULT_PRODUCT_URI,
                                                                   "S2OPC toolkit server example", NULL,
                                                                   OpcUa_ApplicationType_DiscoveryServer);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Test_Server_Local_Service: Failed setting application description \n");
        }
    }

    // Address space configuration
    SOPC_AddressSpace* address_space = NULL;

#ifdef WITH_EXPAT
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_ConfigureFromXML(NULL, XML_UA_NODESET_PATH, NULL, NULL);
        if (SOPC_STATUS_OK == status)
        {
            address_space = SOPC_ServerConfigHelper_GetAddressSpace();
            status = (NULL != address_space) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
        }
    }
#else
    if (SOPC_STATUS_OK == status)
    {
        address_space = SOPC_Embedded_AddressSpace_Load();
        status = (NULL != address_space) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetAddressSpace(address_space);
    }
#endif

    // Add nodes for custom DataType with unknown encoder typechecking
    if (SOPC_STATUS_OK == status)
    {
        status = addNodesForCustomDataTypeTests(address_space, varNodeCustomDT, customDefaultBinaryNode, customDTNode,
                                                varNodeCustomDT2, custom2DefaultBinaryNode, varNodeStructureDT);
    }
    else
    {
        SOPC_Free(varNodeCustomDT);
        SOPC_Free(customDefaultBinaryNode);
        SOPC_Free(customDTNode);
        SOPC_Free(varNodeCustomDT2);
        SOPC_Free(custom2DefaultBinaryNode);
        SOPC_Free(varNodeStructureDT);
    }

    // Configure the local service asynchronous response callback
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetLocalServiceAsyncResponse(SOPC_LocalServiceAsyncRespCallback);
    }

    // Asynchronous request to start server
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(SOPC_ServerStoppedCallback);
    }

    /*
     * LOCAL SERVICE: GetEndpoints
     */

    /* Synchronous request to get endpoints */
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Local_Service: Server started\n");

        // Use 1 as getEndpoints request context
        OpcUa_GetEndpointsResponse* resp = NULL;
        status =
            SOPC_ServerHelper_LocalServiceSync(SOPC_GetEndpointsRequest_Create(DEFAULT_ENDPOINT_URL), (void**) &resp);

        if (SOPC_STATUS_OK == status)
        {
            bool res = checkGetEndpointsResponse(resp);
            status = (res ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
        }

        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: Get endpoints local service synchronous call: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: Get endpoints local  service synchronous call: NOK\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        OpcUa_RegisterServer2Response* resp = NULL;
        status = SOPC_ServerHelper_LocalServiceSync(SOPC_RegisterServer2Request_CreateFromServerConfiguration(),
                                                    (void**) &resp);

        if (SOPC_STATUS_OK == status && 0 == (resp->ResponseHeader.ServiceResult & SOPC_GoodStatusOppositeMask))
        {
            status = resp->NoOfConfigurationResults > 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
            for (int32_t i = 0; SOPC_STATUS_OK == status && i < resp->NoOfConfigurationResults; i++)
            {
                if (0 != (resp->ConfigurationResults[i] & SOPC_GoodStatusOppositeMask))
                {
                    // Status is not good, configuration failed
                    status = SOPC_STATUS_NOK;
                }
            }
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }

        OpcUa_RegisterServer2Response_Clear(resp);
        SOPC_Free(resp);

        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: RegisterServer2 local service synchronous call: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: RegisterServer2 local  service synchronous call: NOK\n");
        }
    }

    /*
     * LOCAL SERVICE: Read initial node attributes values
     */

    if (SOPC_STATUS_OK == status)
    {
        /* Create a service request message and send it through session (read service)*/
        // msg freed when sent
        // Use 1 as read request context
        status = SOPC_ServerHelper_LocalServiceAsync(getReadRequest_message(), 1);
        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: local read asynchronous request: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: local read asynchronous request: NOK\n");
        }
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    /*
     * LOCAL SERVICE: Write node values
     */

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);

        // Create WriteRequest to be sent (deallocated by toolkit) */
        pWriteReqSent = tlibw_new_WriteRequest(address_space);

        // Create same WriteRequest to check results on response reception */
        pWriteReqCopy = tlibw_new_WriteRequest(address_space);
        test_results_set_WriteRequest(pWriteReqCopy);

        // Use 1 as write request context
        status = SOPC_ServerHelper_LocalServiceAsync(pWriteReqSent, 1);
        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: local write asynchronous request: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: local write asynchronous request: NOK\n");
        }
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    /*
     * LOCAL SERVICE: Read node values that were written and check values are modified as requested
     */

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);
        /* Sends another ReadRequest, to verify that the AddS has changed */
        /* The callback will call the verification */
        // msg freed when sent
        // Use 2 as read request context
        status = SOPC_ServerHelper_LocalServiceAsync(getReadRequest_verif_message(), 2);
        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: local read asynchronous request: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: local read asynchronous request: NOK\n");
        }
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    /* Now the request can be freed */
    test_results_set_WriteRequest(NULL);
    tlibw_free_WriteRequest((OpcUa_WriteRequest**) &pWriteReqCopy);

    // Test write of value with known DataType NodeId but unknown encoder
    if (SOPC_STATUS_OK == status)
    {
        status = check_writeCustomDataType(address_space, varNodeCustomDT, varNodeCustomDT2, varNodeStructureDT);
    }

    // Check read of DataType node DataTypeDefinition attribute
    if (SOPC_STATUS_OK == status)
    {
        status = check_readDataTypeDefinition(address_space);
    }

    // Non regression test on concurrent read / write
    test_concurrent_write_read_non_reg();

    /*
     * LOCAL SERVICE: very basic AddNode
     */

    if (SOPC_STATUS_OK == status)
    {
        // Reset expected result
        test_results_set_service_result(false);

        // Create AddNodeRequest to be sent (deallocated by toolkit) */
        OpcUa_AddNodesRequest* addNodesReq = SOPC_AddNodesRequest_Create(1);
        // Create params of the request
        // 1) parent
        SOPC_ExpandedNodeId parentNode;
        SOPC_ExpandedNodeId_Initialize(&parentNode);
        parentNode.NodeId.Data.Numeric = OpcUaId_ObjectsFolder;
        // 2) ref type from parent to the node to add
        SOPC_NodeId referenceTypeId;
        SOPC_NodeId_Initialize(&referenceTypeId);
        referenceTypeId.Data.Numeric = OpcUaId_Organizes;
        // 3) Node to add (nodeId etc)
        SOPC_ExpandedNodeId nodeToAdd;
        SOPC_ExpandedNodeId_Initialize(&nodeToAdd);
        nodeToAdd.NodeId.Namespace = 1;
        nodeToAdd.NodeId.IdentifierType = SOPC_IdentifierType_String;
        status = SOPC_String_AttachFromCstring(&nodeToAdd.NodeId.Data.String, newNodeId);
        // 4) BrowseName of the node to add
        SOPC_QualifiedName nodeToAddBrowseName;
        SOPC_QualifiedName_Initialize(&nodeToAddBrowseName);
        nodeToAddBrowseName.NamespaceIndex = 1;
        status = SOPC_String_AttachFromCstring(&nodeToAddBrowseName.Name, "NewAddedNode");
        // 5) Type def of the node to add
        SOPC_ExpandedNodeId typeDefinition;
        SOPC_ExpandedNodeId_Initialize(&typeDefinition);
        typeDefinition.NodeId.Data.Numeric = OpcUaId_FolderType;

        status = SOPC_AddNodeRequest_SetObjectAttributes(addNodesReq, 0, &parentNode, &referenceTypeId, &nodeToAdd,
                                                         &nodeToAddBrowseName, &typeDefinition, NULL, NULL, NULL, NULL,
                                                         (const SOPC_Byte*) "1");

        // Use 1 as AddNode request context
        status = SOPC_ServerHelper_LocalServiceAsync(addNodesReq, 1);
        if (SOPC_STATUS_OK == status)
        {
            printf("<Test_Server_Local_Service: local addNode asynchronous request: OK\n");
        }
        else
        {
            printf("<Test_Server_Local_Service: local addNode asynchronous request: NOK\n");
        }
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && test_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        status = SOPC_STATUS_TIMEOUT;
    }

    // Asynchronous request to stop the server
    SOPC_ReturnStatus stopStatus = SOPC_ServerHelper_StopServer();

    // Wait until endpoint is closed
    loopCpt = 0;
    loopTimeout = 1000;
    while (SOPC_STATUS_OK == stopStatus && SOPC_Atomic_Int_Get(&endpointClosed) == false &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }

    if (loopCpt * sleepTimeout > loopTimeout)
    {
        stopStatus = SOPC_STATUS_TIMEOUT;
    }

    // Clear the toolkit configuration and stop toolkit threads
    bool areNodeReleased = SOPC_AddressSpace_AreNodesReleasable(address_space);
    SOPC_ServerConfigHelper_Clear();
    if (!areNodeReleased)
    {
        // Node variant content was cleared but allocated nodes still need to be released
        SOPC_AddressSpace_Node_Clear(address_space, varNodeCustomDT);
        SOPC_Free(varNodeCustomDT);
        SOPC_AddressSpace_Node_Clear(address_space, customDefaultBinaryNode);
        SOPC_Free(customDefaultBinaryNode);
        SOPC_AddressSpace_Node_Clear(address_space, customDTNode);
        SOPC_Free(customDTNode);
        SOPC_AddressSpace_Node_Clear(address_space, varNodeCustomDT2);
        SOPC_Free(varNodeCustomDT2);
        SOPC_AddressSpace_Node_Clear(address_space, custom2DefaultBinaryNode);
        SOPC_Free(custom2DefaultBinaryNode);
        SOPC_AddressSpace_Node_Clear(address_space, varNodeStructureDT);
        SOPC_Free(varNodeStructureDT);
    }
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK == status && SOPC_STATUS_OK == stopStatus)
    {
        printf("<Test_Server_Local_Service: final result: OK\n");
    }
    else
    {
        printf("<Test_Server_Local_Service: final result NOK with status = '%d' and stopStatus = '%d'\n", status,
               stopStatus);
    }

    return (SOPC_STATUS_OK == status && SOPC_STATUS_OK == stopStatus) ? 0 : 1;
}
