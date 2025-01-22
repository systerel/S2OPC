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

/** \file
 *
 * \brief Tests of degraded and nominal cases of AddNodes not tested in UACTT.
 *
 * Requires the toolkit_test_server to be running.
 * The test will do nothing if expat is not found or S2OPC is not compiled with S2OPC_NODE_MANAGEMENT.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_request_builder.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_common.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#ifndef WITH_STATIC_SECURITY_DATA
#ifdef WITH_EXPAT
#if 0 != S2OPC_NODE_MANAGEMENT

// Asynchronous service response callback
static void SOPC_Client_AsyncRespCb(SOPC_EncodeableType* encType, const void* response, uintptr_t appContext)
{
    if (encType == &OpcUa_AddNodesResponse_EncodeableType)
    {
        // nothing for the moment
        SOPC_UNUSED_ARG(response);
        SOPC_UNUSED_ARG(appContext);
    }
}

// Connection event callback (only for unexpected events)
static void SOPC_Client_ConnEventCb(SOPC_ClientConnection* config,
                                    SOPC_ClientConnectionEvent event,
                                    SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(status);
    SOPC_ASSERT(false && "Unexpected connection event");
}

static SOPC_ReturnStatus Client_Initialize(void)
{
    // Print Toolkit Configuration
    SOPC_Toolkit_Build_Info build_info = SOPC_ToolkitConfig_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Initialize client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_add_nodes_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }

    return status;
}

static bool SOPC_GetClientUserKeyPassword(const SOPC_SecureConnection_Config* secConnConfig,
                                          const char* cert1Sha1,
                                          char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    bool res = SOPC_TestHelper_AskPassWithContext_FromEnv(cert1Sha1, outPassword);
    return res;
}

static SOPC_ReturnStatus Client_LoadClientConfiguration(size_t* nbSecConnCfgs,
                                                        SOPC_SecureConnection_Config*** secureConnConfigArray)
{
    /* Retrieve XML configuration file path from environment variables TEST_CLIENT_XML_CONFIG*/
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    const char* xml_client_config_path = NULL;

#ifndef WITH_STATIC_SECURITY_DATA
    xml_client_config_path = getenv("TEST_CLIENT_XML_CONFIG");

    if (NULL != xml_client_config_path)
    {
        status = SOPC_ClientConfigHelper_ConfigureFromXML(xml_client_config_path, NULL, nbSecConnCfgs,
                                                          secureConnConfigArray);
    }
    else
    {
        printf(
            "Error. Client config is loaded by XML: variable TEST_CLIENT_XML_CONFIG needs "
            "to be set.\n");
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Set callback necessary to retrieve client key password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);
    }
    // Set callback necessary to retrieve user key password (from environment variable)
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetUserKeyPasswordCallback(&SOPC_GetClientUserKeyPassword);
    }
#endif
    if (SOPC_STATUS_OK == status)
    {
        printf(">>Test_Client_Toolkit: Client configured\n");
    }
    else
    {
        printf(">>Test_Client_Toolkit: Client configuration failed\n");
    }

    return status;
}

static OpcUa_AddNodesResponse* add_node_invalid_BrowseName(SOPC_ClientConnection* secureConnection,
                                                           SOPC_ExpandedNodeId* parentNodeId,
                                                           SOPC_NodeId* referenceTypeId,
                                                           SOPC_ExpandedNodeId* reqNodeId,
                                                           SOPC_QualifiedName* browseName,
                                                           SOPC_ExpandedNodeId* typeDefinition)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_AddNodesResponse* addNodesResp = NULL;
    OpcUa_AddNodesRequest* addNodesReq = NULL;

    // Use "Objects" node for parent node
    parentNodeId->NodeId.Data.Numeric = OpcUaId_ObjectsFolder;
    // Reference type "Organizes"
    referenceTypeId->Data.Numeric = OpcUaId_Organizes;
    // Use Type definition is OpcUaId_BaseObjectType
    typeDefinition->NodeId.Data.Numeric = OpcUaId_BaseObjectType;
    // NodeId requested
    reqNodeId->NodeId.Namespace = 1;
    reqNodeId->NodeId.IdentifierType = SOPC_IdentifierType_String;
    status = SOPC_String_AttachFromCstring(&reqNodeId->NodeId.Data.String, "NewNodeId");
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }

    addNodesReq = SOPC_AddNodesRequest_Create(1);
    if (NULL == addNodesReq)
    {
        return NULL;
    }

    // Do no set the BrowseName so it is invalid
    status = SOPC_AddNodeRequest_SetObjectAttributes(addNodesReq, 0, parentNodeId, referenceTypeId, reqNodeId,
                                                     browseName, typeDefinition, NULL, NULL, NULL, NULL,
                                                     (const SOPC_Byte*) "1");

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) addNodesReq, (void**) &addNodesResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &addNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    // Clear data set
    SOPC_ExpandedNodeId_Clear(reqNodeId);

    return addNodesResp;
}

static OpcUa_AddNodesResponse* add_two_nodes_same_BrowseName(SOPC_ClientConnection* secureConnection,
                                                             SOPC_ExpandedNodeId* parentNodeId,
                                                             SOPC_NodeId* referenceTypeId,
                                                             SOPC_ExpandedNodeId* reqNodeId,
                                                             SOPC_ExpandedNodeId* reqNodeId2,
                                                             SOPC_QualifiedName* browseName,
                                                             SOPC_ExpandedNodeId* typeDefinition)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_AddNodesResponse* addNodesResp = NULL;
    OpcUa_AddNodesRequest* addNodesReq = NULL;

    // Use "Objects" node for parent node
    parentNodeId->NodeId.Data.Numeric = OpcUaId_ObjectsFolder;
    // Reference type "Organizes"
    referenceTypeId->Data.Numeric = OpcUaId_Organizes;
    // Use Type definition is OpcUaId_BaseObjectType
    typeDefinition->NodeId.Data.Numeric = OpcUaId_BaseObjectType;
    // Two NodeIds requested
    reqNodeId->NodeId.Namespace = 1;
    reqNodeId->NodeId.IdentifierType = SOPC_IdentifierType_String;
    reqNodeId2->NodeId.Namespace = 1;
    reqNodeId2->NodeId.IdentifierType = SOPC_IdentifierType_String;
    status = SOPC_String_AttachFromCstring(&reqNodeId->NodeId.Data.String, "NewNodeId_1");
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }
    status = SOPC_String_AttachFromCstring(&reqNodeId2->NodeId.Data.String, "NewNodeId_2");
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }
    // Set one single BrowseName that will be used for both nodes
    browseName->NamespaceIndex = 1;
    status = SOPC_String_AttachFromCstring(&browseName->Name, "BrowseName_Unique");
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }

    addNodesReq = SOPC_AddNodesRequest_Create(2);
    if (NULL == addNodesReq)
    {
        return NULL;
    }

    status = SOPC_AddNodeRequest_SetObjectAttributes(addNodesReq, 0, parentNodeId, referenceTypeId, reqNodeId,
                                                     browseName, typeDefinition, NULL, NULL, NULL, NULL,
                                                     (const SOPC_Byte*) "1");

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_AddNodeRequest_SetObjectAttributes(addNodesReq, 1, parentNodeId, referenceTypeId, reqNodeId2,
                                                         browseName, typeDefinition, NULL, NULL, NULL, NULL,
                                                         (const SOPC_Byte*) "1");
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) addNodesReq, (void**) &addNodesResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &addNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    // Clear data set
    SOPC_ExpandedNodeId_Clear(reqNodeId);
    SOPC_ExpandedNodeId_Clear(reqNodeId2);
    SOPC_QualifiedName_Clear(browseName);

    return addNodesResp;
}

static OpcUa_AddNodesResponse* add_node_invalid_ref_type(SOPC_ClientConnection* secureConnection,
                                                         SOPC_ExpandedNodeId* parentNodeId,
                                                         SOPC_NodeId* referenceTypeId,
                                                         SOPC_ExpandedNodeId* reqNodeId,
                                                         SOPC_QualifiedName* browseName,
                                                         SOPC_ExpandedNodeId* typeDefinition)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_AddNodesResponse* addNodesResp = NULL;
    OpcUa_AddNodesRequest* addNodesReq = NULL;

    // Use "Objects" node for parent node
    parentNodeId->NodeId.Data.Numeric = OpcUaId_ObjectsFolder;
    // Reference "HasProperty" forbidden when target node is Object
    referenceTypeId->Data.Numeric = OpcUaId_HasProperty;
    // Use Type definition is OpcUaId_BaseObjectType
    typeDefinition->NodeId.Data.Numeric = OpcUaId_BaseObjectType;
    // NodeId requested
    reqNodeId->NodeId.Namespace = 1;
    reqNodeId->NodeId.IdentifierType = SOPC_IdentifierType_String;
    status = SOPC_String_AttachFromCstring(&reqNodeId->NodeId.Data.String, "NewNodeId");
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }
    // BrowseName
    browseName->NamespaceIndex = 1;
    status = SOPC_String_AttachFromCstring(&browseName->Name, "BrowseName_NewNode");
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }

    addNodesReq = SOPC_AddNodesRequest_Create(1);
    if (NULL == addNodesReq)
    {
        return NULL;
    }

    status = SOPC_AddNodeRequest_SetObjectAttributes(addNodesReq, 0, parentNodeId, referenceTypeId, reqNodeId,
                                                     browseName, typeDefinition, NULL, NULL, NULL, NULL,
                                                     (const SOPC_Byte*) "1");

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) addNodesReq, (void**) &addNodesResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &addNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    // Clear data set
    SOPC_ExpandedNodeId_Clear(reqNodeId);
    SOPC_QualifiedName_Clear(browseName);

    return addNodesResp;
}

static OpcUa_AddNodesResponse* add_node_invalid_ref_type_to_parent(SOPC_ClientConnection* secureConnection,
                                                                   SOPC_ExpandedNodeId* parentNodeId,
                                                                   SOPC_NodeId* referenceTypeId,
                                                                   SOPC_ExpandedNodeId* reqNodeId,
                                                                   SOPC_QualifiedName* browseName,
                                                                   SOPC_ExpandedNodeId* typeDefinition)
{                
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_AddNodesResponse* addNodesResp = NULL;
    OpcUa_AddNodesRequest* addNodesReq = NULL;

    // Use parent which is not Object, ObjectType, DataVariable, nor VariableType.
    // A Method for example.
    parentNodeId->NodeId.Data.Numeric = OpcUaId_FileType_Open;
    // Reference type "HasComponent"
    referenceTypeId->Data.Numeric = OpcUaId_HasComponent;
    // Use Type definition is OpcUaId_BaseDataVariableType for variable
    typeDefinition->NodeId.Data.Numeric = OpcUaId_BaseDataVariableType;
    // NodeId requested
    reqNodeId->NodeId.Namespace = 1;
    reqNodeId->NodeId.IdentifierType = SOPC_IdentifierType_String;
    status = SOPC_String_AttachFromCstring(&reqNodeId->NodeId.Data.String, "NewNodeId");
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }
    // BrowseName
    browseName->NamespaceIndex = 1;
    status = SOPC_String_AttachFromCstring(&browseName->Name, "BrowseName_NewNode");
    if (SOPC_STATUS_OK != status)
    {
        return NULL;
    }

    addNodesReq = SOPC_AddNodesRequest_Create(1);
    if (NULL == addNodesReq)
    {
        return NULL;
    }

    status = SOPC_AddNodeRequest_SetVariableAttributes(addNodesReq, 0, parentNodeId, referenceTypeId, reqNodeId,
                                                       browseName, typeDefinition, NULL, NULL, NULL, NULL, NULL, NULL,
                                                       NULL, 0, NULL, NULL, NULL, NULL, NULL);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) addNodesReq, (void**) &addNodesResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &addNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    // Clear data set
    SOPC_ExpandedNodeId_Clear(reqNodeId);
    SOPC_QualifiedName_Clear(browseName);

    return addNodesResp;
}

static SOPC_ReturnStatus add_node_variable_in_added_node_object(SOPC_ClientConnection* secureConnection,
                                                                SOPC_ExpandedNodeId* parentNodeId,
                                                                SOPC_NodeId* referenceTypeId,
                                                                SOPC_ExpandedNodeId* reqNodeId,
                                                                SOPC_QualifiedName* browseName,
                                                                SOPC_ExpandedNodeId* typeDefinition)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    OpcUa_AddNodesResponse* addNodesResp = NULL;
    OpcUa_AddNodesRequest* addNodesReq = NULL;

    // 1) Add node Object

    // Use "Objects" node for parent node
    parentNodeId->NodeId.Data.Numeric = OpcUaId_ObjectsFolder;
    // Reference type "Organizes"
    referenceTypeId->Data.Numeric = OpcUaId_Organizes;
    // Use Type definition is OpcUaId_BaseObjectType
    typeDefinition->NodeId.Data.Numeric = OpcUaId_BaseObjectType;
    // NodeId requested
    reqNodeId->NodeId.Namespace = 1;
    reqNodeId->NodeId.IdentifierType = SOPC_IdentifierType_String;
    status = SOPC_String_AttachFromCstring(&reqNodeId->NodeId.Data.String, "NewObject");
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    // BrowseName
    browseName->NamespaceIndex = 1;
    status = SOPC_String_AttachFromCstring(&browseName->Name, "BrowseName_NewObject");
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    addNodesReq = SOPC_AddNodesRequest_Create(1);
    if (NULL == addNodesReq)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    status = SOPC_AddNodeRequest_SetObjectAttributes(addNodesReq, 0, parentNodeId, referenceTypeId, reqNodeId,
                                                     browseName, typeDefinition, NULL, NULL, NULL, NULL,
                                                     (const SOPC_Byte*) "1");

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) addNodesReq, (void**) &addNodesResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &addNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != addNodesResp);
        if (!SOPC_IsGoodStatus(addNodesResp->ResponseHeader.ServiceResult) || addNodesResp->NoOfResults != 1 ||
            addNodesResp->Results[0].StatusCode ||
            !SOPC_NodeId_Equal(&reqNodeId->NodeId, &addNodesResp->Results[0].AddedNodeId))
        {
            status = SOPC_STATUS_NOK;
        }

        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    if (SOPC_STATUS_OK != status)
    {
        return status;
    }

    // 2) Add node Variable in the new Object

    // Set parent to new node
    SOPC_ExpandedNodeId_Clear(parentNodeId);
    status = SOPC_ExpandedNodeId_Copy(parentNodeId, reqNodeId);
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    // Clear string data set and keep other data set.
    SOPC_String_Clear(&reqNodeId->NodeId.Data.String);
    SOPC_String_Clear(&browseName->Name);

    // NodeId requested
    status = SOPC_String_AttachFromCstring(&reqNodeId->NodeId.Data.String, "NewVariable");
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    // BrowseName
    status = SOPC_String_AttachFromCstring(&browseName->Name, "BrowseName_NewVariableInObject");
    if (SOPC_STATUS_OK != status)
    {
        return status;
    }
    // Reference type "HasComponent"
    referenceTypeId->Data.Numeric = OpcUaId_HasComponent;
    // Use Type definition is OpcUaId_BaseDataVariableType for variable
    typeDefinition->NodeId.Data.Numeric = OpcUaId_BaseDataVariableType;

    addNodesReq = SOPC_AddNodesRequest_Create(1);
    if (NULL == addNodesReq)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    status = SOPC_AddNodeRequest_SetVariableAttributes(addNodesReq, 0, parentNodeId, referenceTypeId, reqNodeId,
                                                       browseName, typeDefinition, NULL, NULL, NULL, NULL, NULL, NULL,
                                                       NULL, 0, NULL, NULL, NULL, NULL, NULL);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) addNodesReq, (void**) &addNodesResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(&OpcUa_AddNodesRequest_EncodeableType, (void**) &addNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != addNodesResp);
        if (!SOPC_IsGoodStatus(addNodesResp->ResponseHeader.ServiceResult) || addNodesResp->NoOfResults != 1 ||
            addNodesResp->Results[0].StatusCode ||
            !SOPC_NodeId_Equal(&reqNodeId->NodeId, &addNodesResp->Results[0].AddedNodeId))
        {
            status = SOPC_STATUS_NOK;
        }

        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    // Clear data set
    SOPC_ExpandedNodeId_Clear(parentNodeId);
    SOPC_ExpandedNodeId_Clear(reqNodeId);
    SOPC_QualifiedName_Clear(browseName);

    return status;
}

#endif
#endif
#endif

int main(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* TESTS */
#ifndef WITH_STATIC_SECURITY_DATA
// do not allow WITH_STATIC_SECURITY_DATA since expat is mandatory for the test we will load everything by XML.
#ifdef WITH_EXPAT
#if 0 != S2OPC_NODE_MANAGEMENT

    SOPC_ClientConnection* secureConnection = NULL;
    SOPC_SecureConnection_Config** secureConnConfigArray = NULL;
    size_t nbSecConnCfgs = 0;

    status = Client_Initialize();
    
    if (SOPC_STATUS_OK == status)
    {
        status = Client_LoadClientConfiguration(&nbSecConnCfgs, &secureConnConfigArray);
    }

    // Set asynchronous response callback
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetServiceAsyncResponse(SOPC_Client_AsyncRespCb);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Use user_2k_cert, he is allowed to make AddNodes.
        status = SOPC_ClientHelper_Connect(secureConnConfigArray[2], SOPC_Client_ConnEventCb, &secureConnection);
    }

    if (SOPC_STATUS_OK == status)
    {
        // Declare variables that will be filled during requests
        OpcUa_AddNodesResponse* addNodesResp = NULL;
        SOPC_ExpandedNodeId parentNodeId;
        SOPC_NodeId referenceTypeId;
        SOPC_ExpandedNodeId reqNodeId;
        SOPC_ExpandedNodeId reqNodeId2;
        SOPC_QualifiedName browseName;
        SOPC_ExpandedNodeId typeDefinition;
        SOPC_ExpandedNodeId_Initialize(&parentNodeId);
        SOPC_NodeId_Initialize(&referenceTypeId);
        SOPC_ExpandedNodeId_Initialize(&reqNodeId);
        SOPC_ExpandedNodeId_Initialize(&reqNodeId2);
        SOPC_QualifiedName_Initialize(&browseName);
        SOPC_ExpandedNodeId_Initialize(&typeDefinition);

        /* 1. BrowseName invalid */
        addNodesResp = add_node_invalid_BrowseName(secureConnection, &parentNodeId, &referenceTypeId, &reqNodeId,
                                                   &browseName, &typeDefinition);
        /* Check status code of response is OpcUa_BadBrowseNameInvalid and delete */
        SOPC_ASSERT(NULL != addNodesResp && OpcUa_BadBrowseNameInvalid == addNodesResp->Results[0].StatusCode);
        SOPC_ReturnStatus del_status = SOPC_EncodeableObject_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == del_status);

        /* 2. BrowseName duplicated among parent's references */
        addNodesResp = add_two_nodes_same_BrowseName(secureConnection, &parentNodeId, &referenceTypeId, &reqNodeId,
                                                     &reqNodeId2, &browseName, &typeDefinition);
        /* Check status code of responses, one should be Bad_BrowseNameDuplicated and delete. */
        SOPC_ASSERT(NULL != addNodesResp && (OpcUa_BadBrowseNameDuplicated == addNodesResp->Results[0].StatusCode ||
                                            OpcUa_BadBrowseNameDuplicated == addNodesResp->Results[1].StatusCode));
        del_status = SOPC_EncodeableObject_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == del_status);

        /* 3. Ref type is not valid for the node to add */
        addNodesResp = add_node_invalid_ref_type(secureConnection, &parentNodeId, &referenceTypeId, &reqNodeId,
                                                 &browseName, &typeDefinition);
        /* Check status code of response is OpcUa_BadReferenceNotAllowed and delete. */
        SOPC_ASSERT(NULL != addNodesResp && OpcUa_BadReferenceNotAllowed == addNodesResp->Results[0].StatusCode);
        del_status = SOPC_EncodeableObject_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == del_status);

        /* 4. Ref type is not valid for the node to add to this parent */
        addNodesResp = add_node_invalid_ref_type_to_parent(secureConnection, &parentNodeId, &referenceTypeId, &reqNodeId,
                                                           &browseName, &typeDefinition);
        /* Check status code of response is OpcUa_BadReferenceNotAllowed and delete. */
        SOPC_ASSERT(NULL != addNodesResp && OpcUa_BadReferenceNotAllowed == addNodesResp->Results[0].StatusCode);
        del_status = SOPC_EncodeableObject_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == del_status);

        /* 5. Good case. Add a Variable in the new Object node added */
        status = add_node_variable_in_added_node_object(secureConnection, &parentNodeId, &referenceTypeId, &reqNodeId,
                                                        &browseName, &typeDefinition);
    }

    // Conclude test
    if (SOPC_STATUS_OK == status)
    {
        printf(">>Client: Test AddNodes Success\n");
    }
    else
    {
        printf(">>Client: Test AddNodes Failed\n");
    }

    /* Close the connection */
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<Test_Client_Add_Nodes: Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

#endif
#endif
#endif

    if (SOPC_STATUS_OK == status)
    {
        printf(">>Test_Client_Add_Nodes final result: OK\n");
    }
    else
    {
        printf(">>Test_Client_Add_Nodes final result: NOK\n");
    }
    return (SOPC_STATUS_OK == status ? EXIT_SUCCESS : EXIT_FAILURE);
}
