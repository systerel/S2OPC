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

#include "runtime_variables.h"

#include <assert.h>
#include <stdlib.h>

#include "opcua_identifiers.h"
#include "sopc_array.h"
#include "sopc_time.h"
#include "sopc_toolkit_async_api.h"
#include "sopc_types.h"

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

static bool set_write_value_string(OpcUa_WriteValue* wv, uint32_t id, const char* value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_String_Id);

    return SOPC_String_CopyFromCString(&wv->Value.Value.Value.String, value) == SOPC_STATUS_OK;
}

static bool set_write_value_datetime(OpcUa_WriteValue* wv, uint32_t id, SOPC_DateTime value)
{
    set_write_value_id(wv, id);
    set_variant_scalar(&wv->Value.Value, SOPC_DateTime_Id);
    wv->Value.Value.Value.Date = value;
    return true;
}

static bool set_server_server_status_build_info_variables(SOPC_Array* write_values,
                                                          const RuntimeVariablesBuildInfo* build_info)
{
    OpcUa_WriteValue* values = append_write_values(write_values, 6);
    SOPC_DateTime build_date;
    SOPC_ReturnStatus status = SOPC_Time_FromTimeT(build_info->build_date, &build_date);

    return values != NULL && status == SOPC_STATUS_OK &&
           set_write_value_string(&values[0], OpcUaId_Server_ServerStatus_BuildInfo_ProductUri,
                                  build_info->product_uri) &&
           set_write_value_string(&values[1], OpcUaId_Server_ServerStatus_BuildInfo_ManufacturerName,
                                  build_info->manufacturer_name) &&
           set_write_value_string(&values[2], OpcUaId_Server_ServerStatus_BuildInfo_ProductName,
                                  build_info->product_name) &&
           set_write_value_string(&values[3], OpcUaId_Server_ServerStatus_BuildInfo_SoftwareVersion,
                                  build_info->software_version) &&
           set_write_value_string(&values[4], OpcUaId_Server_ServerStatus_BuildInfo_BuildNumber,
                                  build_info->build_number) &&
           set_write_value_datetime(&values[5], OpcUaId_Server_ServerStatus_BuildInfo_BuildDate, build_date);
}

static bool set_server_server_status_state_value(OpcUa_WriteValue* wv, OpcUa_ServerState state)
{
    set_write_value_id(wv, OpcUaId_Server_ServerStatus_State);
    set_variant_scalar(&wv->Value.Value, SOPC_Int32_Id);
    wv->Value.Value.Value.Int32 = (int32_t) state;
    return true;
}

static bool set_server_server_status_variables(SOPC_Array* write_values, const RuntimeVariables* vars)
{
    OpcUa_WriteValue* values = append_write_values(write_values, 3);
    return values != NULL &&
           set_write_value_datetime(&values[0], OpcUaId_Server_ServerStatus_StartTime, SOPC_Time_GetCurrentTimeUTC()) &&
           set_write_value_datetime(&values[1], OpcUaId_Server_ServerStatus_CurrentTime,
                                    SOPC_Time_GetCurrentTimeUTC()) &&
           set_server_server_status_state_value(&values[2], vars->server_state) &&
           set_server_server_status_build_info_variables(write_values, &vars->build_info);
}

static bool set_server_server_array_value(OpcUa_WriteValue* wv, const char* server_uri)
{
    set_write_value_id(wv, OpcUaId_Server_ServerArray);
    wv->Value.Value.ArrayType = SOPC_VariantArrayType_Array;
    wv->Value.Value.BuiltInTypeId = SOPC_String_Id;

    SOPC_String* uri_copy = calloc(1, sizeof(SOPC_String));

    if (uri_copy == NULL)
    {
        return false;
    }

    SOPC_String_Initialize(uri_copy);

    if (SOPC_String_CopyFromCString(uri_copy, server_uri) != SOPC_STATUS_OK)
    {
        SOPC_String_Clear(uri_copy);
        free(uri_copy);
        return false;
    }

    wv->Value.Value.Value.Array.Content.StringArr = uri_copy;
    wv->Value.Value.Value.Array.Length = 1;

    return true;
}

static bool set_server_namespace_array_value(OpcUa_WriteValue* wv, const char** app_namespace_uris)
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

    assert(n_app_namespace_uris <= INT32_MAX);

    SOPC_String* uris = calloc(n_app_namespace_uris, sizeof(SOPC_String));

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

static bool set_server_variables(SOPC_Array* write_values, const RuntimeVariables* vars)
{
    OpcUa_WriteValue* values = append_write_values(write_values, 5);

    return values != NULL && set_server_server_array_value(&values[0], vars->server_uri) &&
           set_server_namespace_array_value(&values[1], vars->app_namespace_uris) &&
           set_server_service_level_value(&values[2], vars->service_level) &&
           set_write_value_bool(&values[3], OpcUaId_Server_Auditing, vars->auditing) &&
           set_write_value_bool(&values[4], OpcUaId_Server_ServerDiagnostics_EnabledFlag, false) &&
           set_server_server_status_variables(write_values, vars);
}

bool set_runtime_variables(uint32_t endpoint_config_idx, const RuntimeVariables* vars)
{
    OpcUa_WriteRequest* request = calloc(1, sizeof(OpcUa_WriteRequest));
    SOPC_Array* write_values = SOPC_Array_Create(sizeof(OpcUa_WriteValue), 0, OpcUa_WriteValue_Clear);

    bool ok = (write_values != NULL && request != NULL && set_server_variables(write_values, vars));

    if (!ok)
    {
        SOPC_Array_Delete(write_values);
        free(request);
        return false;
    }

    size_t n_values = SOPC_Array_Size(write_values);
    assert(n_values <= INT32_MAX);

    OpcUa_WriteRequest_Initialize(request);
    request->NodesToWrite = SOPC_Array_Into_Raw(write_values);
    request->NoOfNodesToWrite = (int32_t) n_values;
    SOPC_ToolkitServer_AsyncLocalServiceRequest(endpoint_config_idx, request, 0);

    return true;
}
