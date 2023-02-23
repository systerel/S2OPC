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

#include "toolkit_demo_server_methods.h"

#include <stdio.h>
#include <string.h>

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/*---------------------------------------------------------------------------
 *                    Demo Methods for Call service definition
 *---------------------------------------------------------------------------*/

static const SOPC_NodeId TestObject = {.IdentifierType = SOPC_IdentifierType_String,
                                       .Namespace = 1,
                                       .Data.String = {sizeof("TestObject") - 1, 1, (SOPC_Byte*) "TestObject"}};

static const SOPC_NodeId TestObject_HelloNextArg = {
    .IdentifierType = SOPC_IdentifierType_String,
    .Namespace = 1,
    .Data.String = {sizeof("TestObject_HelloNextArg") - 1, 1, (SOPC_Byte*) "TestObject_HelloNextArg"}};

static const SOPC_NodeId TestObject_Counter = {
    .IdentifierType = SOPC_IdentifierType_String,
    .Namespace = 1,
    .Data.String = {sizeof("TestObject_Counter") - 1, 1, (SOPC_Byte*) "TestObject_Counter"}};

static const SOPC_NodeId HasComponent_Type = {SOPC_IdentifierType_Numeric, 0, .Data.Numeric = OpcUaId_HasComponent};
static const SOPC_NodeId DataVariable_Type = {SOPC_IdentifierType_Numeric, 0,
                                              .Data.Numeric = OpcUaId_BaseDataVariableType};

SOPC_StatusCode SOPC_Method_Func_IncCounter(const SOPC_CallContext* callContextPtr,
                                            const SOPC_NodeId* objectId,
                                            uint32_t nbInputArgs,
                                            const SOPC_Variant* inputArgs,
                                            uint32_t* nbOutputArgs,
                                            SOPC_Variant** outputArgs,
                                            void* param)
{
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);

    SOPC_ASSERT(NULL != callContextPtr);
    SOPC_ASSERT(NULL != objectId);
    SOPC_ASSERT(NULL != nbOutputArgs);
    SOPC_ASSERT(NULL != outputArgs);

    if (!SOPC_NodeId_Equal(&TestObject, objectId))
    {
        // Unexpected NodeId to apply method
        return OpcUa_BadNodeIdInvalid;
    }

    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    SOPC_DataValue* dv = NULL;
    SOPC_StatusCode stCode = SOPC_AddressSpaceAccess_ReadValue(addSpAccess, &TestObject_Counter, NULL, &dv);
    if (!SOPC_IsGoodStatus(stCode) || SOPC_UInt32_Id != dv->Value.BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != dv->Value.ArrayType)
    {
        return OpcUa_BadInvalidState;
    }

    dv->Value.Value.Uint32++;

    SOPC_DateTime ts = 0; // will set current time as source TS
    stCode = SOPC_AddressSpaceAccess_Write(addSpAccess, &TestObject_Counter, SOPC_AttributeId_Value, NULL, &dv->Value,
                                           NULL, &ts, NULL);
    if (!SOPC_IsGoodStatus(stCode))
    {
        return OpcUa_BadInvalidState;
    }

    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);

    *nbOutputArgs = 0;
    *outputArgs = NULL;
    return SOPC_GoodGenericStatus;
}

SOPC_StatusCode SOPC_Method_Func_AddToCounter(const SOPC_CallContext* callContextPtr,
                                              const SOPC_NodeId* objectId,
                                              uint32_t nbInputArgs,
                                              const SOPC_Variant* inputArgs,
                                              uint32_t* nbOutputArgs,
                                              SOPC_Variant** outputArgs,
                                              void* param)
{
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);

    SOPC_ASSERT(NULL != callContextPtr);
    SOPC_ASSERT(NULL != objectId);
    SOPC_ASSERT(NULL != inputArgs);
    SOPC_ASSERT(NULL != nbOutputArgs);
    SOPC_ASSERT(NULL != outputArgs);

    if (1 != nbInputArgs || SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[0].ArrayType)
    {
        return OpcUa_BadInvalidArgument;
    }

    if (!SOPC_NodeId_Equal(&TestObject, objectId))
    {
        // Unexpected NodeId to apply method
        return OpcUa_BadNodeIdInvalid;
    }

    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    SOPC_DataValue* dv = NULL;
    SOPC_StatusCode stCode = SOPC_AddressSpaceAccess_ReadValue(addSpAccess, &TestObject_Counter, NULL, &dv);
    if (!SOPC_IsGoodStatus(stCode) || SOPC_UInt32_Id != dv->Value.BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != dv->Value.ArrayType)
    {
        return OpcUa_BadInvalidState;
    }

    dv->Value.Value.Uint32 += inputArgs[0].Value.Uint32;

    SOPC_DateTime ts = 0; // will set current time as source TS
    stCode = SOPC_AddressSpaceAccess_Write(addSpAccess, &TestObject_Counter, SOPC_AttributeId_Value, NULL, &dv->Value,
                                           NULL, &ts, NULL);
    if (!SOPC_IsGoodStatus(stCode))
    {
        return OpcUa_BadInvalidState;
    }

    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);

    *nbOutputArgs = 0;
    *outputArgs = NULL;
    return SOPC_GoodGenericStatus;
}

SOPC_StatusCode SOPC_Method_Func_GetCounterValue(const SOPC_CallContext* callContextPtr,
                                                 const SOPC_NodeId* objectId,
                                                 uint32_t nbInputArgs,
                                                 const SOPC_Variant* inputArgs,
                                                 uint32_t* nbOutputArgs,
                                                 SOPC_Variant** outputArgs,
                                                 void* param)
{
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);

    SOPC_ASSERT(NULL != callContextPtr);
    SOPC_ASSERT(NULL != objectId);
    SOPC_ASSERT(NULL != nbOutputArgs);
    SOPC_ASSERT(NULL != outputArgs);

    // Allocate output parameters
    *outputArgs = SOPC_Calloc(1, sizeof(**outputArgs));
    if (NULL == *outputArgs)
    {
        return OpcUa_BadOutOfMemory;
    }
    SOPC_Variant_Initialize(*outputArgs);
    *nbOutputArgs = 1;

    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    SOPC_DataValue* dv = NULL;
    SOPC_StatusCode stCode = SOPC_AddressSpaceAccess_ReadValue(addSpAccess, &TestObject_Counter, NULL, &dv);
    if (SOPC_IsGoodStatus(stCode) && SOPC_UInt32_Id == dv->Value.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == dv->Value.ArrayType)
    {
        SOPC_ReturnStatus status = SOPC_Variant_Copy(*outputArgs, &dv->Value);
        if (SOPC_STATUS_OK != status)
        {
            stCode = OpcUa_BadOutOfMemory;
        }
    }
    else
    {
        stCode = OpcUa_BadInvalidState;
    }

    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);

    if (!SOPC_IsGoodStatus(stCode))
    {
        SOPC_Free(*outputArgs);
        *outputArgs = NULL;
        *nbOutputArgs = 0;
    }

    return stCode;
}

SOPC_StatusCode SOPC_Method_Func_UpdateAndGetPreviousHello(const SOPC_CallContext* callContextPtr,
                                                           const SOPC_NodeId* objectId,
                                                           uint32_t nbInputArgs,
                                                           const SOPC_Variant* inputArgs,
                                                           uint32_t* nbOutputArgs,
                                                           SOPC_Variant** outputArgs,
                                                           void* param)
{
    SOPC_UNUSED_ARG(param);

    SOPC_ASSERT(NULL != callContextPtr);
    SOPC_ASSERT(NULL != objectId);
    SOPC_ASSERT(NULL != nbOutputArgs);
    SOPC_ASSERT(NULL != outputArgs);

    if (1 != nbInputArgs || SOPC_String_Id != inputArgs[0].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[0].ArrayType || inputArgs[0].Value.String.Length <= 0)
    {
        return OpcUa_BadInvalidArgument;
    }

    // Allocate output parameters
    *outputArgs = SOPC_Calloc(1, sizeof(**outputArgs));
    if (NULL == *outputArgs)
    {
        return OpcUa_BadOutOfMemory;
    }
    SOPC_Variant_Initialize(*outputArgs);
    *nbOutputArgs = 1;

    // Retrieve previous hello argument recorded
    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    SOPC_DataValue* dv = NULL;
    SOPC_StatusCode stCode = SOPC_AddressSpaceAccess_ReadValue(addSpAccess, &TestObject_HelloNextArg, NULL, &dv);

    // Compute output parameter
    if (SOPC_IsGoodStatus(stCode) && SOPC_String_Id == dv->Value.BuiltInTypeId &&
        SOPC_VariantArrayType_SingleValue == dv->Value.ArrayType)
    {
        const char* helloTemplate = "Hello  !";
        const size_t helloArgLen = (size_t)(dv->Value.Value.String.Length > 0 ? dv->Value.Value.String.Length : 0);
        size_t helloSize = strlen(helloTemplate) + helloArgLen + 1;

        char* helloSentence = SOPC_Malloc(helloSize * sizeof(char));

        if (NULL != helloSentence &&
            (int) (helloSize - 1) !=
                snprintf(helloSentence, helloSize, "Hello %s !", SOPC_String_GetRawCString(&dv->Value.Value.String)))
        {
            stCode = OpcUa_BadOutOfMemory;
        }

        if (SOPC_IsGoodStatus(stCode))
        {
            (*outputArgs)[0].BuiltInTypeId = SOPC_String_Id;
            (*outputArgs)[0].ArrayType = SOPC_VariantArrayType_SingleValue;
            SOPC_String_Initialize(&(*outputArgs)[0].Value.String);
            SOPC_ReturnStatus status = SOPC_String_CopyFromCString(&(*outputArgs)[0].Value.String, helloSentence);

            if (SOPC_STATUS_OK != status)
            {
                stCode = OpcUa_BadOutOfMemory;
            }
            SOPC_Free(helloSentence);
        }
    }
    else
    {
        stCode = OpcUa_BadInvalidState;
    }

    // Write next hello argument
    if (SOPC_IsGoodStatus(stCode))
    {
        SOPC_DateTime ts = 0; // will set current time as source TS
        stCode = SOPC_AddressSpaceAccess_Write(addSpAccess, &TestObject_HelloNextArg, SOPC_AttributeId_Value, NULL,
                                               &inputArgs[0], NULL, &ts, NULL);
    }

    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);

    if (!SOPC_IsGoodStatus(stCode))
    {
        SOPC_Variant_Clear(&(*outputArgs)[0]);
        SOPC_Free(*outputArgs);
        *outputArgs = NULL;
        *nbOutputArgs = 0;
    }

    return stCode;
}

SOPC_StatusCode SOPC_Method_Func_AddVariable(const SOPC_CallContext* callContextPtr,
                                             const SOPC_NodeId* objectId,
                                             uint32_t nbInputArgs,
                                             const SOPC_Variant* inputArgs,
                                             uint32_t* nbOutputArgs,
                                             SOPC_Variant** outputArgs,
                                             void* param)
{
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(nbInputArgs);
    SOPC_UNUSED_ARG(inputArgs);

    SOPC_ASSERT(NULL != callContextPtr);
    SOPC_ASSERT(NULL != objectId);
    SOPC_ASSERT(NULL != inputArgs);
    SOPC_ASSERT(NULL != nbOutputArgs);
    SOPC_ASSERT(NULL != outputArgs);
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    if (2 != nbInputArgs || SOPC_NodeId_Id != inputArgs[0].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[0].ArrayType || SOPC_NodeId_Id != inputArgs[1].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[1].ArrayType)
    {
        return OpcUa_BadInvalidArgument;
    }

    if (0 != inputArgs[1].Value.NodeId->Namespace ||
        SOPC_IdentifierType_Numeric != inputArgs[1].Value.NodeId->IdentifierType ||
        SOPC_Null_Id == inputArgs[1].Value.NodeId->Data.Numeric ||
        SOPC_BUILTINID_MAX < inputArgs[1].Value.NodeId->Data.Numeric)
    {
        // Accepts only built in types
        return OpcUa_BadInvalidArgument;
    }

    if (!SOPC_NodeId_Equal(&TestObject, objectId))
    {
        // Unexpected NodeId to apply method
        return OpcUa_BadNodeIdInvalid;
    }

    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);

    SOPC_ExpandedNodeId pNid;
    SOPC_ExpandedNodeId_Initialize(&pNid);
    pNid.NodeId = TestObject;

    char* myVarId = SOPC_NodeId_ToCString(inputArgs[0].Value.NodeId);
    SOPC_QualifiedName browseName;
    SOPC_QualifiedName_Initialize(&browseName);
    browseName.NamespaceIndex = 1;
    SOPC_ReturnStatus status = SOPC_String_AttachFromCstring(&browseName.Name, myVarId);
    SOPC_ASSERT(SOPC_STATUS_OK == status);

    OpcUa_VariableAttributes attrs;
    OpcUa_VariableAttributes_Initialize(&attrs);
    attrs.SpecifiedAttributes =
        OpcUa_NodeAttributesMask_DataType | OpcUa_NodeAttributesMask_AccessLevel | OpcUa_NodeAttributesMask_Value;
    attrs.DataType = *inputArgs[1].Value.NodeId;
    attrs.AccessLevel = 99;
    SOPC_Variant_Initialize(&attrs.Value);
    attrs.Value.BuiltInTypeId = inputArgs[1].Value.NodeId->Data.Numeric;

    SOPC_ExpandedNodeId typeDefId;
    SOPC_ExpandedNodeId_Initialize(&typeDefId);
    typeDefId.NodeId = DataVariable_Type;

    SOPC_StatusCode sc = SOPC_AddressSpaceAccess_AddVariableNode(
        addSpAccess, &pNid, &HasComponent_Type, inputArgs[0].Value.NodeId, &browseName, &attrs, &typeDefId);

    SOPC_Free(myVarId);
    return sc;
}
