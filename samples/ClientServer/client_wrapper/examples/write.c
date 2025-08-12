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
 * \brief A write example using the high-level client API
 *
 * Requires the toolkit_demo_server to be running.
 * Connect to the server and write the given value for the given node.
 * Then disconnect and closes the toolkit.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "sopc_askpass.h"
#include "sopc_date_time.h"
#include "sopc_encodeabletype.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

#define DEFAULT_CLIENT_CONFIG_XML "S2OPC_Client_Wrapper_Config.xml"
#define DEFAULT_CONFIG_ID "write"

static void ClientConnectionEvent(SOPC_ClientConnection* config,
                                  SOPC_ClientConnectionEvent event,
                                  SOPC_StatusCode status)
{
    SOPC_UNUSED_ARG(config);

    // We do not expect events since we use synchronous connection / disconnection, only for degraded case
    printf("ClientConnectionEvent: Unexpected connection event %d with status 0x%08" PRIX32 "\n", event, status);
}

static void PrintReadResponse(const char* nodeId, OpcUa_ReadResponse* pResp)
{
    printf("Previous node \"%s\" value:\n", nodeId);
    SOPC_DataValue* pVal = &pResp->Results[0];
    printf("StatusCode: 0x%08" PRIX32 "\n", pVal->Status);
    SOPC_Variant_Print(&(pVal->Value));
    printf("\n");
}

static SOPC_DataValue* ParseValue(SOPC_BuiltinId builtinId, const char* val)
{
    SOPC_DataValue* dv = SOPC_Calloc(1, sizeof(*dv));
    SOPC_DataValue_Initialize(dv);
    dv->Value.BuiltInTypeId = builtinId;
    dv->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    int scanRes = 0;
    /* Note: SCNu8 / SCNi8 not managed by mingw, use int and uint and then check max values */
    int i8 = 0;
    unsigned int u8 = 0;
    switch (builtinId)
    {
    case SOPC_Boolean_Id:
    case SOPC_Byte_Id:
        scanRes = sscanf(val, "%u", &u8);
        if (0 != scanRes && u8 <= UINT8_MAX)
        {
            if (SOPC_Byte_Id == builtinId)
            {
                dv->Value.Value.Byte = (SOPC_Byte) u8;
            }
            else
            {
                dv->Value.Value.Boolean = (SOPC_Boolean) u8;
            }
        }
        else
        {
            scanRes = 0;
        }
        break;
    case SOPC_SByte_Id:
        scanRes = sscanf(val, "%d", &i8);
        if (0 != scanRes && i8 <= INT8_MAX && i8 >= INT8_MIN)
        {
            dv->Value.Value.Sbyte = (SOPC_SByte) i8;
        }
        else
        {
            scanRes = 0;
        }
        break;
    case SOPC_Int16_Id:
        scanRes = sscanf(val, "%" SCNi16, &dv->Value.Value.Int16);
        break;
    case SOPC_UInt16_Id:
        scanRes = sscanf(val, "%" SCNu16, &dv->Value.Value.Uint16);
        break;
    case SOPC_Int32_Id:
        scanRes = sscanf(val, "%" SCNi32, &dv->Value.Value.Int32);
        break;
    case SOPC_UInt32_Id:
        scanRes = sscanf(val, "%" SCNu32, &dv->Value.Value.Uint32);
        break;
    case SOPC_Int64_Id:
        scanRes = sscanf(val, "%" SCNi64, &dv->Value.Value.Int64);
        break;
    case SOPC_UInt64_Id:
        scanRes = sscanf(val, "%" SCNu64, &dv->Value.Value.Uint64);
        break;
    case SOPC_Float_Id:
        scanRes = sscanf(val, "%f", &dv->Value.Value.Floatv);
        break;
    case SOPC_Double_Id:
        scanRes = sscanf(val, "%lf", &dv->Value.Value.Doublev);
        break;
    case SOPC_String_Id:
        SOPC_String_Initialize(&dv->Value.Value.String);
        if (SOPC_STATUS_OK == SOPC_String_CopyFromCString(&dv->Value.Value.String, val))
        {
            scanRes = true;
        }
        break;
    default:
        scanRes = false;
        break;
    }

    // TODO: issue #1540: add option to the sample to activate writing of value timestamp (and status)
    // Define the source timestamp of the value
    // dv.SourceTimestamp = SOPC_Time_GetCurrentTimeUTC();

    if (0 == scanRes)
    {
        SOPC_DataValue_Clear(dv);
        SOPC_Free(dv);
        dv = NULL;
    }
    return dv;
}

static SOPC_ReturnStatus ReadPreviousValue(SOPC_ClientConnection* secureConnection,
                                           const char* nodeIdStr,
                                           SOPC_BuiltinId* outBuiltinTypeId,
                                           SOPC_VariantArrayType* outArrayType)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    // Create a read request on value to write
    OpcUa_ReadRequest* readRequest = NULL;
    OpcUa_ReadResponse* readResponse = NULL;

    readRequest = SOPC_ReadRequest_Create(1, OpcUa_TimestampsToReturn_Both);
    if (NULL != readRequest)
    {
        status = SOPC_ReadRequest_SetReadValueFromStrings(readRequest, 0, nodeIdStr, SOPC_AttributeId_Value, NULL);
    }
    else
    {
        status = SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Call the read service
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, readRequest, (void**) &readResponse);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(readResponse->ResponseHeader.ServiceResult))
        {
            if (1 == readResponse->NoOfResults && SOPC_IsGoodStatus(readResponse->Results[0].Status))
            {
                PrintReadResponse(nodeIdStr, readResponse);
                *outBuiltinTypeId = readResponse->Results[0].Value.BuiltInTypeId;
                *outArrayType = readResponse->Results[0].Value.ArrayType;
            }
            else
            {
                printf("Pre-required read value failed with status: 0x%08" PRIX32 "\n",
                       readResponse->Results[0].Status);
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            printf("Pre-required read service failed with status: 0x%08" PRIX32 "\n",
                   readResponse->ResponseHeader.ServiceResult);

            status = SOPC_STATUS_NOK;
        }
    }
    if (NULL != readResponse)
    {
        SOPC_EncodeableObject_Delete(readResponse->encodeableType, (void**) &readResponse);
    }

    return status;
}

static bool AskUserKeyPass_FromTerminal(const SOPC_SecureConnection_Config* secConnConfig,
                                        const char* userCertThumb,
                                        char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    const char* promptPrefix = "Password for user certificate thumbprint ";
    size_t promptPrefixLen = strlen(promptPrefix);
    size_t userLen = strlen(userCertThumb);
    char* prompt = SOPC_Calloc(promptPrefixLen + userLen + 2, sizeof(*prompt));
    memcpy(prompt, promptPrefix, promptPrefixLen);
    memcpy(prompt + promptPrefixLen, userCertThumb, userLen);
    memcpy(prompt + promptPrefixLen + userLen, ":", 2);

    bool res = SOPC_AskPass_CustomPromptFromTerminal(prompt, outPassword);
    SOPC_Free(prompt);
    return res;
}

static bool AskUserNamePass_FromTerminal(const SOPC_SecureConnection_Config* secConnConfig,
                                         char** userName,
                                         char** outPassword)
{
    SOPC_UNUSED_ARG(secConnConfig);
    const char* prompt1 = "UserName of user (e.g.: 'user1') : ";
    const char* prompt2 = "Password for user : ";

    /* For test use case: use environment variable TEST_USER as user */
    const char* userEnv = getenv("TEST_USER");
    if (NULL != userEnv)
    {
        *userName = SOPC_strdup(userEnv);
    }
    else
    {
        bool res = SOPC_AskPass_CustomPromptFromTerminal(prompt1, userName);
        if (!res)
        {
            return false;
        }
    }
    bool res = SOPC_AskPass_CustomPromptFromTerminal(prompt2, outPassword);
    if (!res)
    {
        SOPC_Free(*userName);
        return false;
    }
    return res;
}

static bool AskKeyPass_FromTerminal(char** outPassword)
{
    return SOPC_AskPass_CustomPromptFromTerminal("Password for client key:", outPassword);
}

int main(int argc, char* const argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <nodeId> <value> (e.g. %s \"ns=1;i=1012\" 42).\nThe '" DEFAULT_CONFIG_ID
               "' connection configuration "
               "from " DEFAULT_CLIENT_CONFIG_XML
               " is used.\n"
               "The node is first read and the concrete value type is used (cannot be a NULL value).\n",
               argv[0], argv[0]);
        return -2;
    }
    const char* nodeIdStr = argv[1];
    const char* valueStr = argv[2];
    int res = 0;

    /* Initialize client/server toolkit and client wrapper */

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./s2opc_wrapper_write_logs/";
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
            printf("<Example_wrapper_write: failed to load XML config file %s\n", DEFAULT_CLIENT_CONFIG_XML);
        }
    }

    SOPC_SecureConnection_Config* writeConnCfg = NULL;

    if (SOPC_STATUS_OK == status)
    {
        writeConnCfg = SOPC_ClientConfigHelper_GetConfigFromId(DEFAULT_CONFIG_ID);

        if (NULL == writeConnCfg)
        {
            printf("<Example_wrapper_get_endpoints: failed to load configuration id '" DEFAULT_CONFIG_ID
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

    /* Define callback to retrieve the client's user password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(&AskUserNamePass_FromTerminal);
    }

    /* Define callback to retrieve the client's user password */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientConfigHelper_SetUserKeyPasswordCallback(&AskUserKeyPass_FromTerminal);
    }

    /* Update UserPolicyId */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_SecureConnectionConfig_UpdateUserPolicyId(writeConnCfg);
    }

    /* connect to the endpoint */
    SOPC_ClientConnection* secureConnection = NULL;
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_Connect(writeConnCfg, ClientConnectionEvent, &secureConnection);
        if (SOPC_STATUS_OK != status)
        {
            printf("<Example_wrapper_write: Failed to connect\n");
        }
    }

    // Read previous value and retrieve concrete type information
    SOPC_BuiltinId builtinTypeId = SOPC_Null_Id;
    SOPC_VariantArrayType arrayType = SOPC_VariantArrayType_SingleValue;
    if (SOPC_STATUS_OK == status)
    {
        status = ReadPreviousValue(secureConnection, nodeIdStr, &builtinTypeId, &arrayType);
    }

    // Parse the value to write depending on current variable value concrete type
    SOPC_DataValue* writeValue = NULL;
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_VariantArrayType_SingleValue == arrayType)
        {
            writeValue = ParseValue(builtinTypeId, valueStr);
            if (NULL == writeValue)
            {
                printf(
                    "Parsing of provided value %s failed for the read value concrete type. Value is invalid or type is "
                    "not supported.\n",
                    valueStr);
                status = SOPC_STATUS_INVALID_PARAMETERS;
            }
        }
        else
        {
            printf("Only single value is supported and the current value is an array/matrix\n");
            status = SOPC_STATUS_NOK;
        }
    }

    // Create a write request to write the given node value
    OpcUa_WriteRequest* writeRequest = NULL;
    OpcUa_WriteResponse* writeResponse = NULL;
    if (SOPC_STATUS_OK == status)
    {
        writeRequest = SOPC_WriteRequest_Create(1);
        if (NULL != writeRequest)
        {
            status = SOPC_WriteRequest_SetWriteValueFromStrings(writeRequest, 0, nodeIdStr, SOPC_AttributeId_Value,
                                                                NULL, writeValue);
        }
        else
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (NULL != writeValue)
    {
        SOPC_DataValue_Clear(writeValue);
        SOPC_Free(writeValue);
        writeValue = NULL;
    }

    // Call the write service
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ClientHelper_ServiceSync(secureConnection, writeRequest, (void**) &writeResponse);
    }
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_IsGoodStatus(writeResponse->ResponseHeader.ServiceResult))
        {
            if (1 == writeResponse->NoOfResults && SOPC_IsGoodStatus(writeResponse->Results[0]))
            {
                printf("Write of value %s succeeded\n", valueStr);
            }
            else
            {
                printf("Write of value %s failed with status: 0x%08" PRIX32 "\n", valueStr, writeResponse->Results[0]);
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            printf("Write service failed with status: 0x%08" PRIX32 "\n", writeResponse->ResponseHeader.ServiceResult);

            status = SOPC_STATUS_NOK;
        }
    }
    if (NULL != writeResponse)
    {
        SOPC_EncodeableObject_Delete(writeResponse->encodeableType, (void**) &writeResponse);
    }

    // Close the connection
    if (NULL != secureConnection)
    {
        SOPC_ReturnStatus localStatus = SOPC_ClientHelper_Disconnect(&secureConnection);
        if (SOPC_STATUS_OK != localStatus)
        {
            printf("<Example_wrapper_read: Failed to disconnect\n");
        }
    }

    /* Close the toolkit */
    SOPC_ClientConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    res = (SOPC_STATUS_OK == status ? 0 : -1);
    return res;
}
