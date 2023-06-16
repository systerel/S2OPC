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

#include "libs2opc_server_runtime_variables.h"

#include <stdio.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "sopc_array.h"
#include "sopc_assert.h"
#include "sopc_encodeable.h"
#include "sopc_helper_string.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"

static time_t parse_build_date(const char* build_date)
{
    struct tm time;
    memset(&time, 0, sizeof(struct tm));

    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    // note: build_date format is YYYY-MM-DD
    SOPC_ReturnStatus status = SOPC_strtouint16_t(build_date, &year, 10, '-');
    if (SOPC_STATUS_OK != status)
    {
        return 0;
    }
    status = SOPC_strtouint8_t(build_date + 5, &month, 10, '-');
    if (SOPC_STATUS_OK != status)
    {
        return 0;
    }
    status = SOPC_strtouint8_t(build_date + 8, &day, 10, '\0');
    if (SOPC_STATUS_OK != status)
    {
        return 0;
    }

    time.tm_year = (int) year;
    time.tm_mon = (int) month;
    time.tm_mday = (int) day;

    if (time.tm_year < 1900 || time.tm_mon < 0 || time.tm_mon > 11 || time.tm_mday < 0 || time.tm_mday > 31)
    {
        return 0;
    }

    time.tm_year -= 1900;
    time.tm_mon--;

    return mktime(&time);
}

SOPC_Server_RuntimeVariables SOPC_RuntimeVariables_BuildDefault(SOPC_Toolkit_Build_Info build_info,
                                                                SOPC_Server_Config* server_config)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    SOPC_Server_RuntimeVariables runtimeVariables;

    runtimeVariables.serverConfig = server_config;

    runtimeVariables.secondsTillShutdown = 0;
    runtimeVariables.server_state = OpcUa_ServerState_Running;
    runtimeVariables.startTime = SOPC_Time_GetCurrentTimeUTC();
    SOPC_LocalizedText_Initialize(&runtimeVariables.shutdownReason);

    OpcUa_BuildInfo_Initialize(&runtimeVariables.build_info);

    status =
        SOPC_String_AttachFrom(&runtimeVariables.build_info.ProductUri, &server_config->serverDescription.ProductUri);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_String_AttachFromCstring(&runtimeVariables.build_info.ManufacturerName, "Systerel");
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_String_AttachFrom(&runtimeVariables.build_info.ProductName,
                                    &server_config->serverDescription.ApplicationName.defaultText);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    int cmp_res = strcmp(build_info.commonBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildVersion);
    SOPC_ASSERT(0 == cmp_res);
    status = SOPC_String_AttachFromCstring(&runtimeVariables.build_info.SoftwareVersion,
                                           build_info.clientServerBuildInfo.buildVersion);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_String_AttachFromCstring(&runtimeVariables.build_info.BuildNumber,
                                           build_info.clientServerBuildInfo.buildSrcCommit);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    time_t buildDateAsTimet = parse_build_date(build_info.clientServerBuildInfo.buildBuildDate);
    SOPC_DateTime buildDate;
    status = SOPC_Time_FromTimeT(buildDateAsTimet, &buildDate);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    runtimeVariables.build_info.BuildDate = buildDate;

    runtimeVariables.service_level = 255;
    runtimeVariables.auditing = false;

    runtimeVariables.maximum_operations_per_request = SOPC_MAX_OPERATIONS_PER_MSG;
    runtimeVariables.maximum_heavy_operations_per_request = SOPC_MAX_HEAVY_OPERATIONS_PER_MSG;

    return runtimeVariables;
}

SOPC_Server_RuntimeVariables SOPC_RuntimeVariables_Build(OpcUa_BuildInfo* build_info, SOPC_Server_Config* server_config)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;

    SOPC_Server_RuntimeVariables runtimeVariables;

    runtimeVariables.serverConfig = server_config;

    runtimeVariables.secondsTillShutdown = 0;
    runtimeVariables.server_state = OpcUa_ServerState_Running;
    runtimeVariables.startTime = SOPC_Time_GetCurrentTimeUTC();
    SOPC_LocalizedText_Initialize(&runtimeVariables.shutdownReason);

    OpcUa_BuildInfo_Initialize(&runtimeVariables.build_info);

    status = SOPC_String_AttachFrom(&runtimeVariables.build_info.ProductUri, &build_info->ProductUri);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_String_AttachFrom(&runtimeVariables.build_info.ManufacturerName, &build_info->ManufacturerName);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_String_AttachFrom(&runtimeVariables.build_info.ProductName, &build_info->ProductName);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_String_AttachFrom(&runtimeVariables.build_info.SoftwareVersion, &build_info->SoftwareVersion);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_String_AttachFrom(&runtimeVariables.build_info.BuildNumber, &build_info->BuildNumber);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    runtimeVariables.build_info.BuildDate = build_info->BuildDate;

    runtimeVariables.service_level = 255;
    runtimeVariables.auditing = false;

    runtimeVariables.maximum_operations_per_request = SOPC_MAX_OPERATIONS_PER_MSG;
    runtimeVariables.maximum_heavy_operations_per_request = SOPC_MAX_HEAVY_OPERATIONS_PER_MSG;

    return runtimeVariables;
}

static void set_write_value_id(OpcUa_WriteValue* wv, uint32_t id)
{
    wv->AttributeId = 13;
    wv->NodeId.Namespace = 0;
    wv->NodeId.IdentifierType = SOPC_IdentifierType_Numeric;
    wv->NodeId.Data.Numeric = id;
    wv->Value.Status = SOPC_GoodGenericStatus;
}

static void set_variant_scalar(SOPC_Variant* variant, SOPC_BuiltinId type_id)
{
    variant->ArrayType = SOPC_VariantArrayType_SingleValue;
    variant->BuiltInTypeId = type_id;
    variant->DoNotClear = false;
}

static OpcUa_WriteValue* append_write_values(SOPC_Array* write_values, size_t n)
{
    size_t sz = SOPC_Array_Size(write_values);

    if (!SOPC_Array_Append_Values(write_values, NULL, n))
    {
        return NULL;
    }

    OpcUa_WriteValue* values = SOPC_Array_Get_Ptr(write_values, sz);

    for (size_t i = 0; i < n; ++i)
    {
        OpcUa_WriteValue_Initialize(&values[i]);
    }

    return values;
}

static bool set_write_value_bool(OpcUa_WriteValue* wv, uint32_t id, bool value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_Boolean_Id);
    wv->Value.Value.Value.Boolean = value;
    return true;
}

static bool set_write_value_string(OpcUa_WriteValue* wv, uint32_t id, SOPC_String value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_String_Id);

    return SOPC_String_AttachFrom(&wv->Value.Value.Value.String, &value) == SOPC_STATUS_OK;
}

static bool set_write_value_datetime(OpcUa_WriteValue* wv, uint32_t id, SOPC_DateTime value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_DateTime_Id);
    wv->Value.Value.Value.Date = value;
    wv->Value.SourceTimestamp = value; /* Make source timestamp the same than time value */
    return true;
}

#if S2OPC_NANO_PROFILE
#else
/* Only used in micro profile for now */
static bool set_write_value_double(OpcUa_WriteValue* wv, uint32_t id, double value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_Double_Id);
    wv->Value.Value.Value.Doublev = value;
    return true;
}
#endif // S2OPC_NANO_PROFILE

static bool set_write_value_uint16(OpcUa_WriteValue* wv, uint32_t id, uint16_t value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_UInt16_Id);
    wv->Value.Value.Value.Uint16 = value;
    return true;
}

static bool set_write_value_uint32(OpcUa_WriteValue* wv, uint32_t id, uint32_t value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_UInt32_Id);
    wv->Value.Value.Value.Uint32 = value;
    return true;
}

static bool set_write_value_int32(OpcUa_WriteValue* wv, uint32_t id, int32_t value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_Int32_Id);
    wv->Value.Value.Value.Int32 = value;
    return true;
}

static bool set_write_value_localized_text(OpcUa_WriteValue* wv, uint32_t id, SOPC_LocalizedText value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_LocalizedText_Id);
    wv->Value.Value.Value.LocalizedText = SOPC_Malloc(sizeof(SOPC_LocalizedText));
    SOPC_ASSERT(NULL != wv->Value.Value.Value.LocalizedText);
    SOPC_LocalizedText_Initialize(wv->Value.Value.Value.LocalizedText);
    SOPC_ReturnStatus status = SOPC_LocalizedText_Copy(wv->Value.Value.Value.LocalizedText, &value);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    return true;
}

static bool set_write_value_build_info(OpcUa_WriteValue* wv, const OpcUa_BuildInfo* build_info)
{
    /* create extension object */
    SOPC_ExtensionObject* extObject = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
    OpcUa_BuildInfo* build_info_in_extObject = NULL;
    SOPC_ReturnStatus status;

    SOPC_ExtensionObject_Initialize(extObject);
    status =
        SOPC_Encodeable_CreateExtension(extObject, &OpcUa_BuildInfo_EncodeableType, (void**) &build_info_in_extObject);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status = SOPC_EncodeableObject_Copy(&OpcUa_BuildInfo_EncodeableType, build_info_in_extObject, build_info);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    /* Prepare write of this extension object */
    set_write_value_id(wv, OpcUaId_Server_ServerStatus_BuildInfo);
    wv->Value.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    wv->Value.Value.BuiltInTypeId = SOPC_ExtensionObject_Id;
    wv->Value.Value.Value.ExtObject = extObject;

    return true;
}

static bool set_server_server_status_build_info_variables(SOPC_Array* write_values, const OpcUa_BuildInfo* build_info)
{
    OpcUa_WriteValue* values = append_write_values(write_values, 7);

    return values != NULL &&
           set_write_value_string(&values[0], OpcUaId_Server_ServerStatus_BuildInfo_ProductUri,
                                  build_info->ProductUri) &&
           set_write_value_string(&values[1], OpcUaId_Server_ServerStatus_BuildInfo_ManufacturerName,
                                  build_info->ManufacturerName) &&
           set_write_value_string(&values[2], OpcUaId_Server_ServerStatus_BuildInfo_ProductName,
                                  build_info->ProductName) &&
           set_write_value_string(&values[3], OpcUaId_Server_ServerStatus_BuildInfo_SoftwareVersion,
                                  build_info->SoftwareVersion) &&
           set_write_value_string(&values[4], OpcUaId_Server_ServerStatus_BuildInfo_BuildNumber,
                                  build_info->BuildNumber) &&
           set_write_value_datetime(&values[5], OpcUaId_Server_ServerStatus_BuildInfo_BuildDate,
                                    build_info->BuildDate) &&
           set_write_value_build_info(&values[6], build_info);
}

static bool set_server_server_status_state_value(OpcUa_WriteValue* wv, OpcUa_ServerState state)
{
    set_write_value_id(wv, OpcUaId_Server_ServerStatus_State);
    set_variant_scalar(&wv->Value.Value, SOPC_Int32_Id);
    wv->Value.Value.Value.Int32 = (int32_t) state;
    return true;
}

static bool set_write_value_server_status(OpcUa_WriteValue* wv,
                                          SOPC_Server_RuntimeVariables* vars,
                                          SOPC_DateTime currentTime)
{
    const OpcUa_BuildInfo* build_info = &vars->build_info;
    /* create extension object */
    SOPC_ExtensionObject* extObject = SOPC_Calloc(1, sizeof(SOPC_ExtensionObject));
    OpcUa_ServerStatusDataType* server_status_in_extObject = NULL;
    SOPC_ReturnStatus status;

    SOPC_ExtensionObject_Initialize(extObject);
    status = SOPC_Encodeable_CreateExtension(extObject, &OpcUa_ServerStatusDataType_EncodeableType,
                                             (void**) &server_status_in_extObject);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    status =
        SOPC_EncodeableObject_Copy(&OpcUa_BuildInfo_EncodeableType, &server_status_in_extObject->BuildInfo, build_info);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    server_status_in_extObject->CurrentTime = currentTime;
    server_status_in_extObject->SecondsTillShutdown = vars->secondsTillShutdown;
    status = SOPC_LocalizedText_Copy(&server_status_in_extObject->ShutdownReason, &vars->shutdownReason);
    SOPC_ASSERT(SOPC_STATUS_OK == status);
    server_status_in_extObject->State = vars->server_state;
    server_status_in_extObject->StartTime = vars->startTime;

    /* Prepare write of this extension object */
    set_write_value_id(wv, OpcUaId_Server_ServerStatus);
    wv->Value.SourceTimestamp = currentTime; /* Make source timestamp the same than current time value */
    wv->Value.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    wv->Value.Value.BuiltInTypeId = SOPC_ExtensionObject_Id;
    wv->Value.Value.Value.ExtObject = extObject;

    return true;
}

static bool set_server_server_status_variables(SOPC_Array* write_values, SOPC_Server_RuntimeVariables* vars)
{
    OpcUa_WriteValue* values = append_write_values(write_values, 6);
    SOPC_DateTime currentTime = SOPC_Time_GetCurrentTimeUTC();
    return values != NULL &&
           set_write_value_datetime(&values[0], OpcUaId_Server_ServerStatus_StartTime, vars->startTime) &&
           set_write_value_datetime(&values[1], OpcUaId_Server_ServerStatus_CurrentTime, currentTime) &&
           set_server_server_status_state_value(&values[2], vars->server_state) &&
           set_write_value_uint32(&values[3], OpcUaId_Server_ServerStatus_SecondsTillShutdown,
                                  vars->secondsTillShutdown) &&
           set_write_value_localized_text(&values[4], OpcUaId_Server_ServerStatus_ShutdownReason,
                                          vars->shutdownReason) &&
           set_write_value_server_status(&values[5], vars, currentTime) &&
           set_server_server_status_build_info_variables(write_values, &vars->build_info);
}

static bool set_server_server_array_value(OpcUa_WriteValue* wv, const SOPC_String* server_uri)
{
    set_write_value_id(wv, OpcUaId_Server_ServerArray);
    wv->Value.Value.ArrayType = SOPC_VariantArrayType_Array;
    wv->Value.Value.BuiltInTypeId = SOPC_String_Id;

    SOPC_String* uri_copy = SOPC_Calloc(1, sizeof(SOPC_String));

    if (uri_copy == NULL)
    {
        return false;
    }

    SOPC_String_Initialize(uri_copy);

    if (SOPC_String_Copy(uri_copy, server_uri) != SOPC_STATUS_OK)
    {
        SOPC_String_Clear(uri_copy);
        SOPC_Free(uri_copy);
        return false;
    }

    wv->Value.Value.Value.Array.Content.StringArr = uri_copy;
    wv->Value.Value.Value.Array.Length = 1;

    return true;
}

static bool set_server_namespace_array_value(OpcUa_WriteValue* wv, char** app_namespace_uris)
{
    set_write_value_id(wv, OpcUaId_Server_NamespaceArray);
    wv->Value.Value.ArrayType = SOPC_VariantArrayType_Array;
    wv->Value.Value.BuiltInTypeId = SOPC_String_Id;

    size_t n_app_namespace_uris = 1;

    if (app_namespace_uris != NULL)
    {
        for (size_t i = 0; app_namespace_uris[i] != NULL; ++i)
        {
            ++n_app_namespace_uris;
        }
    }

    SOPC_ASSERT(n_app_namespace_uris <= INT32_MAX);

    SOPC_String* uris = SOPC_Calloc(n_app_namespace_uris, sizeof(SOPC_String));

    if (uris == NULL)
    {
        return false;
    }

    for (size_t i = 0; i < n_app_namespace_uris; ++i)
    {
        SOPC_String_Initialize(&uris[i]);
    }

    wv->Value.Value.Value.Array.Content.StringArr = uris;
    wv->Value.Value.Value.Array.Length = (int32_t) n_app_namespace_uris;

    if (SOPC_String_CopyFromCString(&uris[0], "http://opcfoundation.org/UA/") != SOPC_STATUS_OK)
    {
        return false;
    }

    for (size_t i = 0; i < n_app_namespace_uris - 1; ++i)
    {
        if (SOPC_String_CopyFromCString(&uris[1 + i], app_namespace_uris[i]) != SOPC_STATUS_OK)
        {
            wv->Value.Value.Value.Array.Length = (int32_t) i; // limit to previous index for clear operation
            SOPC_Variant_Clear(&wv->Value.Value);
            return false;
        }
    }

    return true;
}

static bool set_server_capabilities_locale_ids_array_value(OpcUa_WriteValue* wv, char** app_locale_ids)
{
    set_write_value_id(wv, OpcUaId_Server_ServerCapabilities_LocaleIdArray);
    wv->Value.Value.ArrayType = SOPC_VariantArrayType_Array;
    wv->Value.Value.BuiltInTypeId = SOPC_String_Id;

    size_t n_locale_ids = 0;

    if (app_locale_ids != NULL)
    {
        for (size_t i = 0; app_locale_ids[i] != NULL; ++i)
        {
            ++n_locale_ids;
        }
    }

    SOPC_ASSERT(n_locale_ids <= INT32_MAX);

    SOPC_String* localeIds = NULL;

    if (n_locale_ids > 0)
    {
        localeIds = SOPC_Calloc(n_locale_ids, sizeof(SOPC_String));
        if (localeIds == NULL)
        {
            return false;
        }
    }

    for (size_t i = 0; i < n_locale_ids; i++)
    {
        SOPC_String_Initialize(&localeIds[i]);
    }

    wv->Value.Value.Value.Array.Content.StringArr = localeIds;
    wv->Value.Value.Value.Array.Length = (int32_t) n_locale_ids;

    for (size_t i = 0; i < n_locale_ids; i++)
    {
        if (SOPC_String_CopyFromCString(&localeIds[i], app_locale_ids[i]) != SOPC_STATUS_OK)
        {
            wv->Value.Value.Value.Array.Length = (int32_t) i; // limit to previous index for clear operation
            SOPC_Variant_Clear(&wv->Value.Value);
            return false;
        }
    }

    return true;
}

static bool set_server_service_level_value(OpcUa_WriteValue* wv, SOPC_Byte level)
{
    set_write_value_id(wv, OpcUaId_Server_ServiceLevel);
    set_variant_scalar(&wv->Value.Value, SOPC_Byte_Id);
    wv->Value.Value.Value.Byte = level;
    return true;
}

static bool set_server_capabilities_max_variables(SOPC_Array* write_values)
{
    size_t nbNodesToSet = 4;
#if S2OPC_NANO_PROFILE
#else
    nbNodesToSet += 1; // Subscription property
#endif // S2OPC_NANO_PROFILE

    OpcUa_WriteValue* values = append_write_values(write_values, nbNodesToSet);
    if (NULL == values)
    {
        return false;
    }

    /* MaxBrowseContinuationPoints: only 1 supported per session */
    set_write_value_uint16(&values[0], OpcUaId_Server_ServerCapabilities_MaxBrowseContinuationPoints, 1);
    /* MaxArrayLength */
    set_write_value_uint32(&values[1], OpcUaId_Server_ServerCapabilities_MaxArrayLength, SOPC_DEFAULT_MAX_ARRAY_LENGTH);
    /* MaxStringLength */
    set_write_value_uint32(&values[2], OpcUaId_Server_ServerCapabilities_MaxStringLength,
                           SOPC_DEFAULT_MAX_STRING_LENGTH);
    /* MaxByteStringLength */
    set_write_value_uint32(&values[3], OpcUaId_Server_ServerCapabilities_MaxByteStringLength,
                           SOPC_DEFAULT_MAX_STRING_LENGTH);
#if S2OPC_NANO_PROFILE
#else
    /* MinSupportedSampleRate */
    set_write_value_double(&values[4], OpcUaId_Server_ServerCapabilities_MinSupportedSampleRate, 0);
#endif // S2OPC_NANO_PROFILE

    return true;
}

static bool set_server_capabilities_operation_limits_variables(SOPC_Array* write_values,
                                                               SOPC_Server_RuntimeVariables* vars)
{
    size_t nbNodesToSet = 5;
#if S2OPC_NANO_PROFILE
#else
#if S2OPC_NODE_MANAGEMENT
    nbNodesToSet += 3; // Subscription, MethodCall and AddNodes operations
#else
    nbNodesToSet += 2; // Subscription and MethodCall operations
#endif // S2OPC_NODE_MANAGEMENT
#endif // S2OPC_NANO_PROFILE
    OpcUa_WriteValue* values = append_write_values(write_values, nbNodesToSet);
    if (NULL == values)
    {
        return false;
    }
    // Set limits for implemented services only, other keep NULL default value
    set_write_value_uint32(&values[0], OpcUaId_Server_ServerCapabilities_OperationLimits_MaxNodesPerRead,
                           vars->maximum_operations_per_request);
    set_write_value_uint32(&values[1], OpcUaId_Server_ServerCapabilities_OperationLimits_MaxNodesPerWrite,
                           vars->maximum_operations_per_request);
    set_write_value_uint32(&values[2], OpcUaId_Server_ServerCapabilities_OperationLimits_MaxNodesPerBrowse,
                           vars->maximum_operations_per_request);
    set_write_value_uint32(&values[3], OpcUaId_Server_ServerCapabilities_OperationLimits_MaxNodesPerRegisterNodes,
                           vars->maximum_operations_per_request);
    set_write_value_uint32(&values[4],
                           OpcUaId_Server_ServerCapabilities_OperationLimits_MaxNodesPerTranslateBrowsePathsToNodeIds,
                           vars->maximum_operations_per_request);
#if S2OPC_NANO_PROFILE
#else
    set_write_value_uint32(&values[5], OpcUaId_Server_ServerCapabilities_OperationLimits_MaxMonitoredItemsPerCall,
                           vars->maximum_operations_per_request);
    set_write_value_uint32(&values[6], OpcUaId_Server_ServerCapabilities_OperationLimits_MaxNodesPerMethodCall,
                           vars->maximum_operations_per_request);
#if S2OPC_NODE_MANAGEMENT
    set_write_value_uint32(&values[7], OpcUaId_Server_ServerCapabilities_OperationLimits_MaxNodesPerNodeManagement,
                           vars->maximum_heavy_operations_per_request);
#endif // S2OPC_NODE_MANAGEMENT
#endif // S2OPC_NANO_PROFILE
    return true;
}

static bool set_server_capabilities_server_profile_array(OpcUa_WriteValue* wv)
{
    set_write_value_id(wv, OpcUaId_Server_ServerCapabilities_ServerProfileArray);
    wv->Value.Value.ArrayType = SOPC_VariantArrayType_Array;
    wv->Value.Value.BuiltInTypeId = SOPC_String_Id;

    SOPC_String* profiles = SOPC_Calloc(1, sizeof(*profiles));
    if (profiles == NULL)
    {
        return false;
    }

    SOPC_String_Initialize(&profiles[0]);

    wv->Value.Value.Value.Array.Content.StringArr = profiles;
    wv->Value.Value.Value.Array.Length = 1;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

#if S2OPC_NANO_PROFILE
    status = SOPC_String_CopyFromCString(&profiles[0], "http://opcfoundation.org/UA-Profile/Server/NanoEmbeddedDevice");
#else
    status =
        SOPC_String_CopyFromCString(&profiles[0], "http://opcfoundation.org/UA-Profile/Server/MicroEmbeddedDevice2017");
#endif // S2OPC_NANO_PROFILE

    if (status != SOPC_STATUS_OK)
    {
        return false;
    }

    return true;
}

static bool set_server_variables(SOPC_Array* write_values, SOPC_Server_RuntimeVariables* vars)
{
    OpcUa_WriteValue* values = append_write_values(write_values, 8);
    return values != NULL &&
           set_server_server_array_value(&values[0], &vars->serverConfig->serverDescription.ProductUri) &&
           set_server_namespace_array_value(&values[1], vars->serverConfig->namespaces) &&
           set_server_service_level_value(&values[2], vars->service_level) &&
           set_write_value_bool(&values[3], OpcUaId_Server_Auditing, vars->auditing) &&
           set_write_value_bool(&values[4], OpcUaId_Server_ServerDiagnostics_EnabledFlag, false) &&
           set_write_value_int32(&values[5], OpcUaId_Server_ServerRedundancy_RedundancySupport,
                                 OpcUa_RedundancySupport_None) &&
           set_server_capabilities_server_profile_array(&values[6]) &&
           set_server_capabilities_locale_ids_array_value(&values[7], vars->serverConfig->localeIds) &&
           set_server_server_status_variables(write_values, vars) &&
           set_server_capabilities_max_variables(write_values) &&
           set_server_capabilities_operation_limits_variables(write_values, vars);
}

OpcUa_WriteRequest* SOPC_RuntimeVariables_BuildWriteRequest(SOPC_Server_RuntimeVariables* vars)
{
    OpcUa_WriteRequest* request = SOPC_Calloc(1, sizeof(OpcUa_WriteRequest));
    SOPC_Array* write_values = SOPC_Array_Create(sizeof(OpcUa_WriteValue), 0, OpcUa_WriteValue_Clear);

    bool ok = (write_values != NULL && request != NULL && set_server_variables(write_values, vars));

    if (!ok)
    {
        SOPC_Array_Delete(write_values);
        SOPC_Free(request);
        return NULL;
    }

    size_t n_values = SOPC_Array_Size(write_values);
    SOPC_ASSERT(n_values <= INT32_MAX);

    OpcUa_WriteRequest_Initialize(request);
    request->NodesToWrite = SOPC_Array_Into_Raw(write_values);
    request->NoOfNodesToWrite = (int32_t) n_values;

    return request;
}

OpcUa_WriteRequest* SOPC_RuntimeVariables_BuildUpdateServerStatusWriteRequest(SOPC_Server_RuntimeVariables* vars)
{
    OpcUa_WriteRequest* request = SOPC_Calloc(1, sizeof(OpcUa_WriteRequest));
    SOPC_Array* write_values = SOPC_Array_Create(sizeof(OpcUa_WriteValue), 0, OpcUa_WriteValue_Clear);

    bool ok = (write_values != NULL && request != NULL && set_server_server_status_variables(write_values, vars));

    if (!ok)
    {
        SOPC_Array_Delete(write_values);
        SOPC_Free(request);
        return NULL;
    }

    size_t n_values = SOPC_Array_Size(write_values);
    SOPC_ASSERT(n_values <= INT32_MAX);

    OpcUa_WriteRequest_Initialize(request);
    request->NodesToWrite = SOPC_Array_Into_Raw(write_values);
    request->NoOfNodesToWrite = (int32_t) n_values;

    return request;
}

OpcUa_WriteRequest* SOPC_RuntimeVariables_UpdateCurrentTimeWriteRequest(SOPC_Server_RuntimeVariables* vars)
{
    OpcUa_WriteRequest* request = SOPC_Calloc(1, sizeof(*request));
    SOPC_Array* write_values = SOPC_Array_Create(sizeof(OpcUa_WriteValue), 2, OpcUa_WriteValue_Clear);

    bool ok = (NULL != write_values && NULL != request);
    SOPC_DateTime currentTime = SOPC_Time_GetCurrentTimeUTC();

    OpcUa_WriteValue* values = NULL;
    if (ok)
    {
        values = append_write_values(write_values, 2);
        ok = (NULL != values);
    }

    ok = (ok && set_write_value_datetime(&values[0], OpcUaId_Server_ServerStatus_CurrentTime, currentTime) &&
          set_write_value_server_status(&values[1], vars, currentTime));

    if (ok)
    {
        OpcUa_WriteRequest_Initialize(request);
        request->NodesToWrite = SOPC_Array_Into_Raw(write_values);
        request->NoOfNodesToWrite = 2;
    }
    else
    {
        SOPC_Array_Delete(write_values);
        SOPC_Free(request);
        request = NULL;
    }
    return request;
}
