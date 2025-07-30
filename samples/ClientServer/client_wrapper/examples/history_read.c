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
 * \brief A history read example using the high-level client API
 *
 * Requires the toolkit_demo_server to be running.
 * Connect to the server and reads external historic of the value of the given node.
 * Then disconnect and closes the toolkit.
 *
 */

#include <stdio.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "sopc_askpass.h"
#include "sopc_encodeabletype.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#define DEFAULT_CLIENT_CONFIG_XML "S2OPC_Client_Wrapper_Config.xml"
#define DEFAULT_CONFIG_ID "read"

static bool AskKeyPass_FromTerminal(char** outPassword)
{
    return SOPC_AskPass_CustomPromptFromTerminal("Password for client key:", outPassword);
}

static void ClientConnectionEvent(SOPC_ClientConnection* config,
                                  SOPC_ClientConnectionEvent event,
                                  SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);

    // We do not expect events since we use synchronous connection / disconnection, only for degraded case
    printf("ClientConnectionEvent: Unexpected connection event %d with status 0x%08" PRIX32 "\n", event, status);
}

static void PrintHistoryReadResponse(char* const* nodeIds, OpcUa_HistoryReadResponse* pResp)
{
    for (int32_t i = 0; i < pResp->NoOfResults; ++i)
    {
        OpcUa_HistoryReadResult* pVal = &pResp->Results[i];
        printf("HistoryRead response of item \"%s\":\n", nodeIds[i]);
        printf("StatusCode of the item response: 0x%08" PRIX32 "\n", pVal->StatusCode);
        // Print data values
        if (SOPC_ExtObjBodyEncoding_Object == pVal->HistoryData.Encoding &&
            &OpcUa_HistoryData_EncodeableType == pVal->HistoryData.Body.Object.ObjType)
        {
            OpcUa_HistoryData* dataResponse = (OpcUa_HistoryData*) pVal->HistoryData.Body.Object.Value;
            for (int32_t j = 0; j < dataResponse->NoOfDataValues; ++j)
            {
                SOPC_Variant_Print(&dataResponse->DataValues[j].Value);
            }
        }
    }
}

int main(int argc, char* const argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <nodeId> [<nodeId>] (e.g. %s \"ns=1;i=1012\").\nThe '" DEFAULT_CONFIG_ID
               "' connection configuration "
               "from " DEFAULT_CLIENT_CONFIG_XML " is used.\n",
               argv[0], argv[0]);
        return -2;
    }
    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_history_read_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_Initialize();
    }

    size_t nbConfigs = 0;
    SOPC_SecureConnection_Config** scConfigArray = NULL;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_ConfigureFromXML(DEFAULT_CLIENT_CONFIG_XML, NULL, &nbConfigs, &scConfigArray);

        if (SOPC_STATUS_OK != status)
        {
            printf("<Example_wrapper_history_read: failed to load XML config file %s\n", DEFAULT_CLIENT_CONFIG_XML);
        }
    }

    SOPC_SecureConnection_Config* readConnCfg = NULL;

    if (SOPC_STATUS_OK == status)
    {
        readConnCfg = SOPC_ClientConfigHelper_GetConfigFromId(DEFAULT_CONFIG_ID);

        if (NULL == readConnCfg)
        {
            printf("<Example_wrapper_history_read: failed to load configuration id '" DEFAULT_CONFIG_ID
                   "' from XML config file %s\n",
                   DEFAULT_CLIENT_CONFIG_XML);

            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Define callback to retrieve the client's private key password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(&AskKeyPass_FromTerminal);
    }

    /* connect to the endpoint */
    SOPC_ClientConnection* secureConnection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(readConnCfg, ClientConnectionEvent, &secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Example_wrapper_history_read: Failed to connect\n");
        }
    }

    // Create request and response
    OpcUa_HistoryReadRequest* historyReadRequest = NULL;
    OpcUa_HistoryReadResponse* historyReadResponse = NULL;
    // Create parameters of the request
    bool releaseContinuationPoints = true;
    SOPC_DateTime end = SOPC_Time_GetCurrentTimeUTC();
    SOPC_DateTime start = 0;
    SOPC_ExtensionObject* historyReadDetails =
        SOPC_HistoryReadRequest_CreateReadRawModifiedDetails(false, start, end, 0, true);
    if (SOPC_STATUS_OK == status && NULL != historyReadDetails)
    {
        historyReadRequest = SOPC_HistoryReadRequest_Create((size_t)(argc - 1), OpcUa_TimestampsToReturn_Both,
                                                            releaseContinuationPoints, historyReadDetails);
        if (NULL != historyReadRequest)
        {
            for (size_t i = 1; SOPC_STATUS_OK == status && i < (size_t) argc; i++)
            {
                status = SOPC_HistoryReadRequest_SetHistoryReadValueFromStrings(historyReadRequest, i - 1, argv[i],
                                                                                NULL, NULL, NULL);
            }
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, historyReadRequest, (void**) &historyReadResponse);
    }

    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(historyReadResponse->ResponseHeader.ServiceResult) &&
            (argc - 1) == historyReadResponse->NoOfResults)
        {
            PrintHistoryReadResponse(&argv[1], historyReadResponse);
        }
        else
        {
            printf("History Read failed with status: 0x%08" PRIX32 "\n",
                   historyReadResponse->ResponseHeader.ServiceResult);

            status = SOPC_STATUS_NOK;
        }
    }
    if (NULL != historyReadResponse)
    {
        SOPC_EncodeableObject_Delete(historyReadResponse->encodeableType, (void**) &historyReadResponse);
    }

    // Close the connection
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<Example_wrapper_history_read: Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    res = (SOPC_STATUS_OK == status ? 0 : -1);
    return res;
}
