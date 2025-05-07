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
 * \brief Tests of degraded and nominal cases of DeleteNodes not tested in UACTT.
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
#include "sopc_threads.h"
#include "sopc_toolkit_config.h"
#include "sopc_types.h"

#ifndef WITH_STATIC_SECURITY_DATA
#ifdef WITH_EXPAT
#if 0 != S2OPC_NODE_MANAGEMENT

static bool received_notif_badNodeIdUnknow = false;
static bool received_notif_goodOrUncertainStatus = false;
// Sleep timeout in milliseconds
static const uint32_t sleepTimeout = 200;
// Loop timeout in milliseconds
static const uint32_t loopTimeout = 10000;

static void SubscriptionNotification_Cb(const SOPC_ClientHelper_Subscription* subscription,
                                        SOPC_StatusCode status,
                                        SOPC_EncodeableType* notificationType,
                                        uint32_t nbNotifElts,
                                        const void* notification,
                                        uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(subscription);
    SOPC_UNUSED_ARG(monitoredItemCtxArray);

    if (SOPC_IsGoodStatus(status) && &OpcUa_DataChangeNotification_EncodeableType == notificationType)
    {
        const OpcUa_DataChangeNotification* notifs = (const OpcUa_DataChangeNotification*) notification;
        for (uint32_t i = 0; i < nbNotifElts; i++)
        {
            if (OpcUa_BadNodeIdUnknown == notifs->MonitoredItems[i].Value.Status)
            {
                received_notif_badNodeIdUnknow = true;
            }
            else if (SOPC_IsGoodOrUncertainStatus(notifs->MonitoredItems[i].Value.Status))
            {
                received_notif_goodOrUncertainStatus = true;
            }
        }
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
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_client_delete_nodes_logs/";
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
            "to be set, e.g.: TEST_CLIENT_XML_CONFIG=./S2OPC_Client_Test_Config.xml "
            "TEST_PASSWORD_PRIVATE_KEY=password ./toolkit_test_client_add_nodes\n");
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

static OpcUa_DeleteNodesResponse* delete_one_node(SOPC_ClientConnection* secureConnection,
                                                  const char* nodeIdString,
                                                  bool deleteTargetReferences)
{
    OpcUa_DeleteNodesResponse* deleteNodesResp = NULL;
    OpcUa_DeleteNodesRequest* deleteNodesReq = SOPC_DeleteNodesRequest_Create(1);
    SOPC_ASSERT(NULL != deleteNodesReq);
    SOPC_NodeId* nodeIdToDelete = SOPC_NodeId_FromCString(nodeIdString);
    SOPC_ReturnStatus status =
        SOPC_DeleteNodesRequest_SetParameters(deleteNodesReq, 0, nodeIdToDelete, deleteTargetReferences);
    SOPC_NodeId_Clear(nodeIdToDelete);
    SOPC_Free(nodeIdToDelete);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) deleteNodesReq, (void**) &deleteNodesResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(&OpcUa_DeleteNodesRequest_EncodeableType, (void**) &deleteNodesReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }
    return deleteNodesResp;
}

static OpcUa_BrowseResponse* browse(SOPC_ClientConnection* secureConnection, const char* nodeIdString)
{
    OpcUa_BrowseResponse* browseResp = NULL;
    OpcUa_BrowseRequest* browseReq = SOPC_BrowseRequest_Create(1, 0, NULL);
    SOPC_ASSERT(NULL != browseReq);
    SOPC_ReturnStatus status = SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(
        browseReq, 0, nodeIdString, OpcUa_BrowseDirection_Both, NULL, true, 0,
        OpcUa_BrowseResultMask_ReferenceTypeId | OpcUa_BrowseResultMask_DisplayName);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, (void*) browseReq, (void**) &browseResp);
    }
    else
    {
        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(&OpcUa_BrowseRequest_EncodeableType, (void**) &browseReq);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }
    return browseResp;
}

static SOPC_ReturnStatus wait_for_notification_node(bool goodStatus, const char* tcStepStr)
{
    uint32_t loopCpt = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    while (!received_notif_badNodeIdUnknow && !received_notif_goodOrUncertainStatus &&
           loopCpt * sleepTimeout <= loopTimeout)
    {
        loopCpt++;
        // Retrieve received messages on socket
        SOPC_Sleep(sleepTimeout);
    }
    if (received_notif_badNodeIdUnknow && !goodStatus)
    {
        printf("%s. Notif BadNodeIdUnknow well received!\n", tcStepStr);
        status = SOPC_STATUS_OK;
    }
    else if (received_notif_goodOrUncertainStatus && goodStatus)
    {
        printf("%s. Notif GoodOrUncertainStatus well received!\n", tcStepStr);
        status = SOPC_STATUS_OK;
    }
    else if (received_notif_badNodeIdUnknow || received_notif_goodOrUncertainStatus)
    {
        printf("%s. UNEXPECTED notif BadNodeIdUnknow or GoodStatus received!\n", tcStepStr);
        status = SOPC_STATUS_NOK;
    }
    else
    {
        printf("%s. Timeout waiting for the notif BadNodeIdUnknow or GoodStatus.\n", tcStepStr);
    }
    received_notif_badNodeIdUnknow = false;
    received_notif_goodOrUncertainStatus = false;
    return status;
}

static SOPC_ReturnStatus client_add_node_variable(SOPC_ClientConnection* secureConnection,
                                                  const char* nodeIdStr,
                                                  const char* parentNodeIdStr,
                                                  const char* browseName)
{
    OpcUa_AddNodesResponse* addNodesResp = NULL;
    OpcUa_AddNodesRequest* addNodesReq = SOPC_AddNodesRequest_Create(1);
    if (NULL == addNodesReq)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ExpandedNodeId parentExpNodeId;
    SOPC_ExpandedNodeId_Initialize(&parentExpNodeId);
    SOPC_ExpandedNodeId nodeExpNodeId;
    SOPC_ExpandedNodeId_Initialize(&nodeExpNodeId);
    SOPC_NodeId referenceTypeId;
    SOPC_NodeId_Initialize(&referenceTypeId);
    referenceTypeId.Data.Numeric = OpcUaId_HasComponent;
    SOPC_ExpandedNodeId typeDefinition;
    SOPC_ExpandedNodeId_Initialize(&typeDefinition);
    typeDefinition.NodeId.Data.Numeric = OpcUaId_BaseDataVariableType;

    SOPC_QualifiedName nodeToAddBrowseName;
    SOPC_QualifiedName_Initialize(&nodeToAddBrowseName);
    SOPC_ReturnStatus status = SOPC_QualifiedName_ParseCString(&nodeToAddBrowseName, browseName);

    SOPC_NodeId* parentNodeId = SOPC_NodeId_FromCString(parentNodeIdStr);
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(nodeIdStr);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Copy(&parentExpNodeId.NodeId, parentNodeId);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_NodeId_Copy(&nodeExpNodeId.NodeId, nodeId);
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_AddNodeRequest_SetVariableAttributes(
            addNodesReq, 0, &parentExpNodeId, &referenceTypeId, &nodeExpNodeId, &nodeToAddBrowseName, &typeDefinition,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL);
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

    if (SOPC_STATUS_OK == status)
    {
        SOPC_ASSERT(NULL != addNodesResp);
        if (!SOPC_IsGoodStatus(addNodesResp->ResponseHeader.ServiceResult))
        {
            printf("Bad status code returned. Status: 0x%08" PRIX32 "\n", addNodesResp->ResponseHeader.ServiceResult);
            status = SOPC_STATUS_NOK;
        }
        else
        {
            if (addNodesResp->NoOfResults != 1)
            {
                printf("Bad number of result in response. Expected one, got:%d\n", addNodesResp->NoOfResults);
                status = SOPC_STATUS_NOK;
            }
            if (!SOPC_NodeId_Equal(nodeId, &addNodesResp->Results[0].AddedNodeId))
            {
                printf("NodeId added is not equal to the one requested to add!\nData string:%s\n",
                       SOPC_String_GetRawCString(&addNodesResp->Results[0].AddedNodeId.Data.String));
                status = SOPC_STATUS_NOK;
            }
        }

        SOPC_ReturnStatus delStatus =
            SOPC_EncodeableObject_Delete(addNodesResp->encodeableType, (void**) &addNodesResp);
        SOPC_ASSERT(SOPC_STATUS_OK == delStatus);
    }

    SOPC_NodeId_Clear(parentNodeId);
    SOPC_Free(parentNodeId);
    SOPC_NodeId_Clear(nodeId);
    SOPC_Free(nodeId);
    SOPC_QualifiedName_Clear(&nodeToAddBrowseName);

    return status;
}

#endif
#endif
#endif

int main(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool glob_res = true;

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

    if (SOPC_STATUS_OK == status)
    {
        // Use user_2k_cert, he is allowed to make DeleteNodes.
        status = SOPC_ClientHelper_Connect(secureConnConfigArray[2], SOPC_Client_ConnEventCb, &secureConnection);
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Declare nodes. We will delete two nodes (one deleted with param DeleteTargetReferences = TRUE and
           the other one with param false), that have the same parent node and same type node. */
        const char* firstNodeToDelete = "ns=1;i=1012";
        const char* secondNodeToDelete = "ns=1;i=1002";
        const char* parentNode = "ns=1;i=1000";
        int32_t nbr_refs_of_parentNode = 0;
        const char* baseDataVariableTypeNode = "i=63";
        int32_t nbr_refs_of_baseDataVariableTypeNode = 0;

        /* 1.0. Delete one node with DeleteTargetReferences = TRUE */
        // Get initial numbers of references of the parent node and of the type node.
        OpcUa_BrowseResponse* browseResp = browse(secureConnection, parentNode);
        SOPC_ASSERT(NULL != browseResp && SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult));
        nbr_refs_of_parentNode = browseResp->Results[0].NoOfReferences;
        SOPC_ASSERT(SOPC_STATUS_OK == SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
        browseResp = browse(secureConnection, baseDataVariableTypeNode);
        SOPC_ASSERT(NULL != browseResp && SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult));
        nbr_refs_of_baseDataVariableTypeNode = browseResp->Results[0].NoOfReferences;
        SOPC_ASSERT(SOPC_STATUS_OK == SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));

        OpcUa_DeleteNodesResponse* deleteNodesResp = delete_one_node(secureConnection, firstNodeToDelete, false);
        /* Check status code of response. Must be ok */
        if (NULL != deleteNodesResp && 1 == deleteNodesResp->NoOfResults &&
            SOPC_IsGoodStatus(deleteNodesResp->ResponseHeader.ServiceResult))
        {
            printf("1.0. Delete one node without DeleteTargetReferences ok.\n");
            /* 1.1. Try to Browse the deleted node: must be BadNodeIdUnknown */
            browseResp = browse(secureConnection, firstNodeToDelete);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                OpcUa_BadNodeIdUnknown == browseResp->Results[0].StatusCode)
            {
                printf("1.1. Browse on deleted node returned OpcUa_BadNodeIdUnknown, as expected.\n");
            }
            else
            {
                glob_res = false;
                printf("1.1. Browse on deleted node returned unexpected answer.\n");
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
            /* 1.2. Check that parent node of the deleted node has same number of references (because parameter
               deleteTargetReferences = FALSE) using Browse service */
            browseResp = browse(secureConnection, parentNode);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult) &&
                nbr_refs_of_parentNode == browseResp->Results[0].NoOfReferences)
            {
                printf(
                    "1.2. Browse on parent node of the deleted node has unchanged number of references, as "
                    "expected.\n");
            }
            else
            {
                glob_res = false;
                printf("1.2. Browse on parent node returned unexpected answer.\n");
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
            browseResp = browse(secureConnection, baseDataVariableTypeNode);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult) &&
                nbr_refs_of_baseDataVariableTypeNode == browseResp->Results[0].NoOfReferences)
            {
                printf(
                    "1.2.bis Browse on type node of the deleted node has unchanged number of references, as "
                    "expected.\n");
            }
            else
            {
                glob_res = false;
                printf("1.2.bis Browse on type node returned unexpected answer.\n");
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
        }
        else
        {
            glob_res = false;
            printf("1.0. Delete one node without DeleteTargetReferences NOK.\n");
        }
        SOPC_ASSERT(SOPC_STATUS_OK ==
                    SOPC_EncodeableObject_Delete(deleteNodesResp->encodeableType, (void**) &deleteNodesResp));

        /* 2. Delete one node with DeleteTargetReferences = TRUE */
        printf("\n");
        // This NodeId has same references as the first one.
        deleteNodesResp = delete_one_node(secureConnection, secondNodeToDelete, true);
        /* Check status code of response. Must be ok */
        if (NULL != deleteNodesResp && 1 == deleteNodesResp->NoOfResults &&
            SOPC_IsGoodStatus(deleteNodesResp->ResponseHeader.ServiceResult))
        {
            printf("2.0. Delete one node with DeleteTargetReferences ok\n");
            /* 2.1. Try to Browse the delete node: must be BadNodeIdUnknown */
            browseResp = browse(secureConnection, secondNodeToDelete);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                OpcUa_BadNodeIdUnknown == browseResp->Results[0].StatusCode)
            {
                printf("2.1. Browse on deleted node returned OpcUa_BadNodeIdUnknown as expected.\n");
            }
            else
            {
                glob_res = false;
                printf("2.1. Browse on deleted node returned unexpected answer.\n");
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
            /* 2.2. Check that parent node of the deleted node has not the same number of references (because parameter
               deleteTargetReferences = TRUE) using Browse service */
            browseResp = browse(secureConnection, parentNode);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult) &&
                nbr_refs_of_parentNode - 1 == browseResp->Results[0].NoOfReferences)
            {
                printf("2.2. Parent node has the good number of references (decreased by 1).\n");
                /* 2.2.1. Check the missing ref is the good one. */
                SOPC_NodeId* secondNodeIdToDelete = SOPC_NodeId_FromCString(secondNodeToDelete);
                bool found = false;
                for (int32_t indexReference = 0; indexReference < nbr_refs_of_parentNode - 1; indexReference++)
                {
                    OpcUa_ReferenceDescription* reference = &browseResp->Results[0].References[indexReference];
                    int32_t nodeId_comparison = -1;
                    status = SOPC_NodeId_Compare(secondNodeIdToDelete, &reference->NodeId.NodeId, &nodeId_comparison);
                    if (0 == nodeId_comparison && SOPC_STATUS_OK == status)
                    {
                        found = true;
                    }
                }
                SOPC_NodeId_Clear(secondNodeIdToDelete);
                SOPC_Free(secondNodeIdToDelete);
                if (!found)
                {
                    printf("2.2.1. Reference from parent to deleted node not found, as expected!\n");
                }
                else
                {
                    glob_res = false;
                    printf(
                        "2.2.1. One reference from parent node to deleted node has been found, delete has not been "
                        "well made.\n");
                }
            }
            else
            {
                glob_res = false;
                printf("2.2. Browse on first reference deleted unexpected answer.\n");
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
            browseResp = browse(secureConnection, baseDataVariableTypeNode);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult) &&
                nbr_refs_of_baseDataVariableTypeNode - 1 == browseResp->Results[0].NoOfReferences)
            {
                printf(
                    "2.2.bis Browse on type node of the deleted node has the good number of references (decreased by "
                    "1)\n");
            }
            else
            {
                glob_res = false;
                printf("2.2.bis Browse on type node returned unexpected answer.\n");
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
        }
        else
        {
            glob_res = false;
            printf("2.0. Delete one node with DeleteTargetReferences NOK.\n");
        }
        SOPC_ASSERT(SOPC_STATUS_OK ==
                    SOPC_EncodeableObject_Delete(deleteNodesResp->encodeableType, (void**) &deleteNodesResp));

        /* 3. Check that notif is received when deleted monitored item */
        /* create a subscription */
        printf("\n");
        char* nodeToMonitor[1] = {"ns=1;s=MethodI_Input"};
        SOPC_ClientHelper_Subscription* subscription =
            SOPC_ClientHelper_CreateSubscription(secureConnection, SOPC_CreateSubscriptionRequest_CreateDefault(),
                                                 SubscriptionNotification_Cb, (uintptr_t) NULL);
        if (NULL == subscription)
        {
            glob_res = false;
            printf("3.0. Failed to create subscription\n");
        }
        else
        {
            printf("3.0. Subscription created.\n");
            // Prepare the creation request for monitored items
            OpcUa_CreateMonitoredItemsRequest* createMIreq = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
                0, 1, nodeToMonitor, OpcUa_TimestampsToReturn_Both);
            // Create the monitored items
            // Response is necessary to know if creation succeeded or not
            OpcUa_CreateMonitoredItemsResponse createMIresp;
            OpcUa_CreateMonitoredItemsResponse_Initialize(&createMIresp);
            status =
                SOPC_ClientHelper_Subscription_CreateMonitoredItems(subscription, createMIreq, NULL, &createMIresp);
            if (SOPC_STATUS_OK != status)
            {
                glob_res = false;
                printf("3.1. Failed to create monitored item\n");
            }
            else
            {
                printf("3.1. Success at creating monitored item. Going to delete node now.\n");
            }
            OpcUa_CreateMonitoredItemsResponse_Clear(&createMIresp);
        }

        /* 4. Delete one node with childs on two level deep. Check that nodes have been deleted.
              (option S2OPC_NODE_DELETE_CHILD_NODES is set by default) */
        /* Nodes hierarchy is as follow:
           ns=1;s=TestObject
           |
           |_____ ns=1;s=MethodI
                  |
                  |_____ns=1;s=MethodI_Input
        */
        const char* objectNodeToDelete = "ns=1;s=TestObject";
        const char* nodeMethodI = "ns=1;s=MethodI";
        const char* nodeMethodI_Input = "ns=1;s=MethodI_Input";
        received_notif_badNodeIdUnknow = false;
        received_notif_goodOrUncertainStatus = false;
        deleteNodesResp = delete_one_node(secureConnection, objectNodeToDelete, true);
        // Wait for 3.2. success before next
        status = wait_for_notification_node(false, "3.2");
        if (SOPC_STATUS_OK != status)
        {
            glob_res = false;
        }
        // Close the subscription
        if (NULL != subscription)
        {
            SOPC_ReturnStatus localStatus = SOPC_ClientHelper_DeleteSubscription(&subscription);
            if (SOPC_STATUS_OK != localStatus)
            {
                glob_res = false;
                printf("3.3. Failed to delete subscription\n");
            }
        }

        printf("\n");
        /* Check status code of response. Must be ok */
        if (NULL != deleteNodesResp && 1 == deleteNodesResp->NoOfResults &&
            SOPC_IsGoodStatus(deleteNodesResp->ResponseHeader.ServiceResult))
        {
            printf("4.0. Delete one Object node with childs node ok.\n");
            /* 4.1. Try to Browse the first ref of node: must be BadNodeIdUnknown */
            browseResp = browse(secureConnection, nodeMethodI);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                OpcUa_BadNodeIdUnknown == browseResp->Results[0].StatusCode)
            {
                printf(
                    "4.1. Browse on deleted node's referenced node %s returned OpcUa_BadNodeIdUnknown, as expected.\n",
                    nodeMethodI);
            }
            else
            {
                glob_res = false;
                printf("4.1. Browse on deleted node's referenced node %s returned unexpected answer.\n", nodeMethodI);
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
            /* 3.2. Try to Browse the second ref of node: must be BadNodeIdUnknown */
            browseResp = browse(secureConnection, nodeMethodI_Input);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                OpcUa_BadNodeIdUnknown == browseResp->Results[0].StatusCode)
            {
                printf(
                    "4.2. Browse on deleted node's referenced node %s returned OpcUa_BadNodeIdUnknown, as expected.\n",
                    nodeMethodI_Input);
            }
            else
            {
                glob_res = false;
                printf("4.2. Browse on deleted node's referenced node %s returned unexpected answer.\n",
                       nodeMethodI_Input);
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
        }
        else
        {
            glob_res = false;
            printf("4.0. Delete one Object node with childs NOK\n");
        }
        SOPC_ASSERT(SOPC_STATUS_OK ==
                    SOPC_EncodeableObject_Delete(deleteNodesResp->encodeableType, (void**) &deleteNodesResp));

        /* 5.0. Delete another node with DeleteTargetReferences = TRUE with subscription on deleted node and add it
         * again*/
        char* thirdNodeToDelete = "ns=1;i=1011";
        const char* thirdNodeToDeleteParent = "ns=1;i=1000";
        nbr_refs_of_parentNode = 0;
        // Get initial numbers of refs of the node references
        browseResp = browse(secureConnection, thirdNodeToDeleteParent);
        SOPC_ASSERT(NULL != browseResp && SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult));
        nbr_refs_of_parentNode = browseResp->Results[0].NoOfReferences;
        SOPC_ASSERT(SOPC_STATUS_OK == SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));

        printf("\n");
        char* nodeToMonitor2[1] = {thirdNodeToDelete};
        subscription =
            SOPC_ClientHelper_CreateSubscription(secureConnection, SOPC_CreateSubscriptionRequest_CreateDefault(),
                                                 SubscriptionNotification_Cb, (uintptr_t) NULL);
        if (NULL == subscription)
        {
            glob_res = false;
            printf("5.0. Failed to create subscription\n");
        }
        else
        {
            printf("5.0. Subscription created.\n");
            // Prepare the creation request for monitored items
            OpcUa_CreateMonitoredItemsRequest* createMIreq = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
                0, 1, nodeToMonitor2, OpcUa_TimestampsToReturn_Both);
            // Create the monitored items
            // Response is necessary to know if creation succeeded or not
            OpcUa_CreateMonitoredItemsResponse createMIresp;
            OpcUa_CreateMonitoredItemsResponse_Initialize(&createMIresp);
            status =
                SOPC_ClientHelper_Subscription_CreateMonitoredItems(subscription, createMIreq, NULL, &createMIresp);
            if (SOPC_STATUS_OK != status)
            {
                glob_res = false;
                printf("5.1. Failed to create monitored item\n");
            }
            else
            {
                printf("5.1. Success at creating monitored item. Going to delete node now.\n");
            }
            OpcUa_CreateMonitoredItemsResponse_Clear(&createMIresp);
        }
        received_notif_badNodeIdUnknow = false;
        received_notif_goodOrUncertainStatus = false;
        deleteNodesResp = delete_one_node(secureConnection, thirdNodeToDelete, true);
        // Wait for 5.2. success before next
        status = wait_for_notification_node(false, "5.2");
        if (SOPC_STATUS_OK != status)
        {
            glob_res = false;
        }
        /* Check status code of response. Must be ok */
        if (NULL != deleteNodesResp && 1 == deleteNodesResp->NoOfResults &&
            SOPC_IsGoodStatus(deleteNodesResp->ResponseHeader.ServiceResult))
        {
            printf("5.3. Delete node %s with DeleteTargetReferences ok.\n", thirdNodeToDelete);
            /* 5.3. Try to Browse the delete node: must be BadNodeIdUnknown */
            browseResp = browse(secureConnection, thirdNodeToDelete);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                OpcUa_BadNodeIdUnknown == browseResp->Results[0].StatusCode)
            {
                printf("5.3. Browse on deleted node returned OpcUa_BadNodeIdUnknown as expected.\n");
            }
            else
            {
                glob_res = false;
                printf("5.3. Browse unexpected answer.\n");
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
            /* 5.4. Check that parent node of the deleted node has not the same number of references (because parameter
               deleteTargetReferences = TRUE) using Browse service */
            browseResp = browse(secureConnection, thirdNodeToDeleteParent);
            if (NULL != browseResp && 1 == browseResp->NoOfResults &&
                SOPC_IsGoodStatus(browseResp->ResponseHeader.ServiceResult) &&
                nbr_refs_of_parentNode - 1 == browseResp->Results[0].NoOfReferences)
            {
                printf("5.4. Browse on parent of deleted node has one reference less as expected.\n");
            }
            else
            {
                glob_res = false;
                printf("5.4. Browse on parent of deleted node has unexpected number of references.\n");
            }
            SOPC_ASSERT(SOPC_STATUS_OK ==
                        SOPC_EncodeableObject_Delete(browseResp->encodeableType, (void**) &browseResp));
        }
        else
        {
            glob_res = false;
            printf("5.3. Delete node %s with DeleteTargetReferences nok\n", thirdNodeToDelete);
        }
        SOPC_ASSERT(SOPC_STATUS_OK ==
                    SOPC_EncodeableObject_Delete(deleteNodesResp->encodeableType, (void**) &deleteNodesResp));

        /* 5.5. Add the variable node again and wait for GoodStatus notification */
        if (SOPC_STATUS_OK == status)
        {
            received_notif_badNodeIdUnknow = false;
            received_notif_goodOrUncertainStatus = false;
            status = client_add_node_variable(secureConnection, thirdNodeToDelete, thirdNodeToDeleteParent, "Int32");
            if (SOPC_STATUS_OK != status)
            {
                printf("5.5. Failed to add node %s variable\n", thirdNodeToDelete);
                glob_res = false;
            }
            else
            {
                printf("5.5. Add node %s variable succeeded.\n", thirdNodeToDelete);
            }
        }

        // Wait for notification with GoodStatus
        status = wait_for_notification_node(true, "5.5");
        if (SOPC_STATUS_OK != status)
        {
            glob_res = false;
        }

        /* 6. Delete one node with a child that has another parent. Check that this child has not been deleted. */

        // Only hand tested. Not tested since it's a bad OPC UA practise to have such nodes in the address space,
        // and it would imply that we make a such address space only for this test.

        /* 7. Check that option S2OPC_NODE_DELETE_CHILD_NODES works well (ie childs are not deleted if the option is
           unset). Tested in check_helpers: check_addressSpace_access */
    }

    /* Close the connection */
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            glob_res = false;
            printf("<Test_Client_Delete_Nodes: Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

#endif
#endif
#endif

    // Conclude test
    if (SOPC_STATUS_OK == status)
    {
        printf(">>Client: Test DeleteNodes Success\n");
    }
    else
    {
        printf(">>Client: Test DeleteNodes Failed\n");
    }
    return glob_res ? EXIT_SUCCESS : EXIT_FAILURE;
}
