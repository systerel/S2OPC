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

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_address_space.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_encodeable.h"
#include "sopc_encoder.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_time.h"

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
#include "opcua_S2OPC_identifiers.h"

#define SOPC_PKI_PATH "./S2OPC_Demo_PKI"

#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"

static int32_t endpointClosed = false;

static uint32_t cptReadResps = 0;

static void SOPC_ServerStoppedCallback(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&endpointClosed, true);
}

static void SOPC_LocalServiceAsyncRespCallback(SOPC_EncodeableType* encType, void* response, uintptr_t appContext)
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
            test_results_set_service_result(tlibw_verify_response_remote(test_results_get_WriteRequest(), readResp));
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
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (!SOPC_AddressSpace_AreReadOnlyNodes(address_space))
    {
        // Add a variable node with custom DataType
        varNodeCustomDT->node_class = OpcUa_NodeClass_Variable;
        OpcUa_VariableNode* varNode = &varNodeCustomDT->data.variable;
        OpcUa_VariableNode_Initialize(varNode);
        varNode->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = 421000};
        varNode->NodeClass = OpcUa_NodeClass_Variable;
        varNode->DataType = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_S2OPC_CustomDataType};
        varNode->ValueRank = -2; // Any
        status = SOPC_AddressSpace_Append(address_space, varNodeCustomDT);

        if (SOPC_STATUS_OK == status)
        {
            // Add the "Default Binary" node that references the custom DataType
            customDefaultBinaryNode->node_class = OpcUa_NodeClass_Object;
            OpcUa_ObjectNode* objNode = &customDefaultBinaryNode->data.object;
            OpcUa_ObjectNode_Initialize(objNode);
            objNode->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1,
                                            .Data.Numeric = OpcUaId_S2OPC_CustomDataType_Encoding_DefaultBinary};
            objNode->NodeClass = OpcUa_NodeClass_Object;

            objNode->References = SOPC_Calloc(1, sizeof(OpcUa_ReferenceNode));
            SOPC_ASSERT(NULL != objNode->References);
            objNode->NoOfReferences = 1;
            OpcUa_ReferenceNode* refNode = &objNode->References[0];
            OpcUa_ReferenceNode_Initialize(refNode);
            refNode->ReferenceTypeId =
                (SOPC_NodeId){SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasEncoding};
            refNode->IsInverse = true;
            refNode->TargetId = (SOPC_ExpandedNodeId){
                {SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_S2OPC_CustomDataType}, {0}, 0};
            status = SOPC_AddressSpace_Append(address_space, customDefaultBinaryNode);
        }

        if (SOPC_STATUS_OK == status)
        {
            // Add the "DataType" node that references the Structure DataType
            customDTNode->node_class = OpcUa_NodeClass_DataType;
            OpcUa_DataTypeNode* dtNode = &customDTNode->data.data_type;
            OpcUa_DataTypeNode_Initialize(dtNode);
            dtNode->NodeId =
                (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_S2OPC_CustomDataType};
            dtNode->NodeClass = OpcUa_NodeClass_DataType;

            dtNode->References = SOPC_Calloc(1, sizeof(OpcUa_ReferenceNode));
            SOPC_ASSERT(NULL != dtNode->References);
            dtNode->NoOfReferences = 1;
            OpcUa_ReferenceNode* refNode = &dtNode->References[0];
            OpcUa_ReferenceNode_Initialize(refNode);
            refNode->ReferenceTypeId =
                (SOPC_NodeId){SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasSubtype};
            refNode->IsInverse = true;
            refNode->TargetId =
                (SOPC_ExpandedNodeId){{SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Structure}, {0}, 0};
            status = SOPC_AddressSpace_Append(address_space, customDTNode);
        }

        if (SOPC_STATUS_OK == status)
        {
            // Add a variable node with custom DataType2
            varNodeCustomDT2->node_class = OpcUa_NodeClass_Variable;
            OpcUa_VariableNode* varNode2 = &varNodeCustomDT2->data.variable;
            OpcUa_VariableNode_Initialize(varNode2);
            varNode2->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = 421000 * 2};
            varNode2->NodeClass = OpcUa_NodeClass_Variable;
            varNode2->DataType =
                (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_S2OPC_CustomDataType * 2};
            varNode2->ValueRank = -1;
            status = SOPC_AddressSpace_Append(address_space, varNodeCustomDT2);
        }

        if (SOPC_STATUS_OK == status)
        {
            // Add the "Default Binary" node that references the custom 2 DataType
            // (without DataType node in nodeset => ok for variable with exact type)
            custom2DefaultBinaryNode->node_class = OpcUa_NodeClass_Object;
            OpcUa_ObjectNode* objNode = &custom2DefaultBinaryNode->data.object;
            OpcUa_ObjectNode_Initialize(objNode);
            objNode->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1,
                                            .Data.Numeric = OpcUaId_S2OPC_CustomDataType_Encoding_DefaultBinary * 2};
            objNode->NodeClass = OpcUa_NodeClass_Object;

            objNode->References = SOPC_Calloc(1, sizeof(OpcUa_ReferenceNode));
            SOPC_ASSERT(NULL != objNode->References);
            objNode->NoOfReferences = 1;
            OpcUa_ReferenceNode* refNode = &objNode->References[0];
            OpcUa_ReferenceNode_Initialize(refNode);
            refNode->ReferenceTypeId =
                (SOPC_NodeId){SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasEncoding};
            refNode->IsInverse = true;
            refNode->TargetId = (SOPC_ExpandedNodeId){
                {SOPC_IdentifierType_Numeric, 1, .Data.Numeric = OpcUaId_S2OPC_CustomDataType * 2}, {0}, 0};
            status = SOPC_AddressSpace_Append(address_space, custom2DefaultBinaryNode);
        }

        // Add a variable node with abstract Structure DT
        if (SOPC_STATUS_OK == status)
        {
            varNodeStructDT->node_class = OpcUa_NodeClass_Variable;
            varNode = &varNodeStructDT->data.variable;
            OpcUa_VariableNode_Initialize(varNode);
            varNode->NodeId = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 1, .Data.Numeric = 421001};
            varNode->NodeClass = OpcUa_NodeClass_Variable;
            varNode->DataType = (SOPC_NodeId){SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_Structure};
            varNode->ValueRank = -3; // ScalarOrOneDimension
            status = SOPC_AddressSpace_Append(address_space, varNodeStructDT);
        }
    }
    return status;
}

static SOPC_ReturnStatus check_writeCustomDataType(SOPC_AddressSpace* address_space,
                                                   SOPC_AddressSpace_Node* varNodeCustomDT,
                                                   SOPC_AddressSpace_Node* varNodeCustomDT2,
                                                   SOPC_AddressSpace_Node* varNodeStructDT)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (!SOPC_AddressSpace_AreReadOnlyNodes(address_space))
    {
        // Create an extension object of an unreferenced encodeable type
        SOPC_ExtensionObject extObj;
        SOPC_ExtensionObject_Initialize(&extObj);

        OpcUa_S2OPC_CustomDataType* instCDT = NULL;

        status =
            SOPC_Encodeable_CreateExtension(&extObj, &OpcUa_S2OPC_CustomDataType_EncodeableType, (void**) &instCDT);
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
        extObj.TypeId.NodeId.Data.Numeric = OpcUaId_S2OPC_CustomDataType_Encoding_DefaultBinary * 2;

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
            OpcUaId_S2OPC_CustomDataType_Encoding_DefaultBinary * 2;

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
            if (SOPC_IsGoodStatus(writeResp->ResponseHeader.ServiceResult) &&
                SOPC_IsGoodStatus(writeResp->Results[0]) && SOPC_IsGoodStatus(writeResp->Results[2]) &&
                !SOPC_IsGoodStatus(writeResp->Results[3]) && SOPC_IsGoodStatus(writeResp->Results[4]) &&
                SOPC_IsGoodStatus(writeResp->Results[5]) && !SOPC_IsGoodStatus(writeResp->Results[6]) &&
                SOPC_IsGoodStatus(writeResp->Results[7]))
            {
                // [1] shall only succeed if dynamic type resolution is active
                // otherwise custom data type cannot be resolved to be the subtype of Structure
                if (S2OPC_DYNAMIC_TYPE_RESOLUTION == SOPC_IsGoodStatus(writeResp->Results[1]))
                {
                    status = SOPC_STATUS_OK;
                }
                else
                {
                    status = SOPC_STATUS_INVALID_STATE;
                }
            }
            else
            {
                status = SOPC_STATUS_INVALID_STATE;
            }
        }

        if (NULL != writeResp)
        {
            SOPC_Encodeable_Delete(writeResp->encodeableType, (void**) &writeResp);
        }
        SOPC_Buffer_Delete(buf);
    }
    return status;
}

int main(int argc, char* argv[])
{
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);

    SOPC_AddressSpace_Node varNodeCustomDT;
    memset(&varNodeCustomDT, 0, sizeof(varNodeCustomDT));

    SOPC_AddressSpace_Node customDefaultBinaryNode;
    memset(&customDefaultBinaryNode, 0, sizeof(customDefaultBinaryNode));
    SOPC_AddressSpace_Node customDTNode;
    memset(&customDTNode, 0, sizeof(customDTNode));

    SOPC_AddressSpace_Node varNodeCustomDT2;
    memset(&varNodeCustomDT, 0, sizeof(varNodeCustomDT2));

    SOPC_AddressSpace_Node custom2DefaultBinaryNode;
    memset(&custom2DefaultBinaryNode, 0, sizeof(custom2DefaultBinaryNode));

    SOPC_AddressSpace_Node varNodeStructureDT;
    memset(&varNodeStructureDT, 0, sizeof(varNodeStructureDT));

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
    SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);

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
    if (SOPC_STATUS_OK == status)
    {
        address_space = SOPC_Embedded_AddressSpace_Load();
        status = (NULL != address_space) ? SOPC_STATUS_OK : SOPC_STATUS_NOK;
    }

    // Add nodes for custom DataType with unknown encoder typechecking
    if (SOPC_STATUS_OK == status)
    {
        status =
            addNodesForCustomDataTypeTests(address_space, &varNodeCustomDT, &customDefaultBinaryNode, &customDTNode,
                                           &varNodeCustomDT2, &custom2DefaultBinaryNode, &varNodeStructureDT);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetAddressSpace(address_space);
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
        status = check_writeCustomDataType(address_space, &varNodeCustomDT, &varNodeCustomDT2, &varNodeStructureDT);
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
    if (!SOPC_AddressSpace_AreReadOnlyNodes(address_space))
    {
        SOPC_AddressSpace_Node_Clear(address_space, &varNodeCustomDT);
        SOPC_AddressSpace_Node_Clear(address_space, &customDefaultBinaryNode);
        SOPC_AddressSpace_Node_Clear(address_space, &customDTNode);

        SOPC_AddressSpace_Node_Clear(address_space, &varNodeCustomDT2);
        SOPC_AddressSpace_Node_Clear(address_space, &custom2DefaultBinaryNode);

        SOPC_AddressSpace_Node_Clear(address_space, &varNodeStructureDT);
    }
    SOPC_ServerConfigHelper_Clear();
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
