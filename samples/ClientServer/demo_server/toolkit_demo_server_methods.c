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

#include "libs2opc_server.h"
#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"

/*---------------------------------------------------------------------------
 *                    Demo Methods for Call service definition
 *---------------------------------------------------------------------------*/

static const SOPC_NodeId TestObject = SOPC_NODEID_STRING(1, "TestObject");

static const SOPC_NodeId EventInstNodeId = SOPC_NODEID_STRING(1, "EventInstance_NodeId_Example");
static const SOPC_NodeId TestObject_HelloNextArg = SOPC_NODEID_STRING(1, "TestObject_HelloNextArg");

static const SOPC_NodeId TestObject_Counter = SOPC_NODEID_STRING(1, "TestObject_Counter");

static const SOPC_NodeId ObjectNodeId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_ObjectsFolder);
static const SOPC_NodeId HasComponent_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasComponent);
static const SOPC_NodeId hasChildType = SOPC_NODEID_NS0_NUMERIC(OpcUaId_HasChild);
static const SOPC_NodeId DataVariable_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_BaseDataVariableType);
static const SOPC_NodeId Organizes_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_Organizes);

#ifdef S2OPC_EVENT_MANAGEMENT
static const SOPC_NodeId BaseEvent_Type = SOPC_NODEID_NS0_NUMERIC(OpcUaId_BaseEventType);
static const SOPC_NodeId Null_Type = SOPC_NODEID_NS0_NUMERIC(0);
#endif

static const SOPC_QualifiedName TestObject_Counter_BrowseName = SOPC_QUALIFIED_NAME(1, "Counter");

static const SOPC_QualifiedName TestObject_BrowseName = SOPC_QUALIFIED_NAME(0, "TestObject");

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

    OpcUa_ReferenceDescription* references = NULL;
    int32_t nbOfReferences = 0;

    // Browse TestObject to illustrate browse usage
    SOPC_StatusCode stCode =
        SOPC_AddressSpaceAccess_BrowseNode(addSpAccess, objectId, OpcUa_BrowseDirection_Forward, &HasComponent_Type,
                                           false, 0, 0, &references, &nbOfReferences);

    if (SOPC_IsGoodStatus(stCode))
    {
        bool found = false;
        for (int i = 0; i < nbOfReferences && !found; i++)
        {
            if (0 == references[i].NodeId.ServerIndex)
            {
                found = SOPC_NodeId_Equal(&TestObject_Counter, &references[i].NodeId.NodeId);
            }
        }
        SOPC_ASSERT(found);
    }
    else
    {
        printf("Browse service failed with status code : %0x\n", stCode);
    }

    // Read, increment and write the counter
    SOPC_DataValue* dv = NULL;
    stCode = SOPC_AddressSpaceAccess_ReadValue(addSpAccess, &TestObject_Counter, NULL, &dv);
    if (!SOPC_IsGoodStatus(stCode) || SOPC_UInt32_Id != dv->Value.BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != dv->Value.ArrayType)
    {
        stCode = OpcUa_BadInvalidState;
    }

    if (SOPC_IsGoodStatus(stCode))
    {
        dv->Value.Value.Uint32++;

        SOPC_DateTime ts = 0; // will set current time as source TS
        stCode =
            SOPC_AddressSpaceAccess_WriteValue(addSpAccess, &TestObject_Counter, NULL, &dv->Value, NULL, &ts, NULL);
        if (!SOPC_IsGoodStatus(stCode))
        {
            stCode = OpcUa_BadInvalidState;
        }
    }

    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);
    SOPC_Clear_Array(&nbOfReferences, (void**) &references, sizeof(*references), OpcUa_ReferenceDescription_Clear);

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
        stCode = OpcUa_BadInvalidState;
    }

    if (SOPC_IsGoodStatus(stCode))
    {
        dv->Value.Value.Uint32 += inputArgs[0].Value.Uint32;

        SOPC_DateTime ts = 0; // will set current time as source TS
        stCode =
            SOPC_AddressSpaceAccess_WriteValue(addSpAccess, &TestObject_Counter, NULL, &dv->Value, NULL, &ts, NULL);
        if (!SOPC_IsGoodStatus(stCode))
        {
            stCode = OpcUa_BadInvalidState;
        }
    }

    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);

    *nbOutputArgs = 0;
    *outputArgs = NULL;
    return stCode;
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
    const SOPC_NodeId* counterNode = NULL;
    SOPC_DataValue* dv = NULL;

    // Get NodeId of Counter from Object with path Object->TestObject->Counter
    OpcUa_RelativePath pathToCounter;
    OpcUa_RelativePath_Initialize(&pathToCounter);

    pathToCounter.NoOfElements = 2;
    pathToCounter.Elements = SOPC_Calloc((size_t) pathToCounter.NoOfElements, sizeof(*pathToCounter.Elements));
    SOPC_ASSERT(NULL != pathToCounter.Elements);
    OpcUa_RelativePathElement* element = &pathToCounter.Elements[0];
    element->IncludeSubtypes = false;
    element->IsInverse = false;
    element->ReferenceTypeId = Organizes_Type;
    element->TargetName = TestObject_BrowseName;
    element = &pathToCounter.Elements[1];
    element->IncludeSubtypes = true;
    element->IsInverse = false;
    element->ReferenceTypeId = hasChildType;
    element->TargetName = TestObject_Counter_BrowseName;
    SOPC_StatusCode stCode =
        SOPC_AddressSpaceAccess_TranslateBrowsePath(addSpAccess, &ObjectNodeId, &pathToCounter, &counterNode);

    if (SOPC_IsGoodStatus(stCode) && NULL != counterNode)
    {
        // Read the counter value from the Counter node
        stCode = SOPC_AddressSpaceAccess_ReadValue(addSpAccess, counterNode, NULL, &dv);
    }
    if (SOPC_IsGoodStatus(stCode) && NULL != dv && SOPC_UInt32_Id == dv->Value.BuiltInTypeId &&
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

    OpcUa_RelativePath_Clear(&pathToCounter);
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
        }
        SOPC_Free(helloSentence);
    }
    else
    {
        stCode = OpcUa_BadInvalidState;
    }

    // Write next hello argument
    if (SOPC_IsGoodStatus(stCode))
    {
        SOPC_DateTime ts = 0; // will set current time as source TS
        stCode = SOPC_AddressSpaceAccess_WriteValue(addSpAccess, &TestObject_HelloNextArg, NULL, &inputArgs[0], NULL,
                                                    &ts, NULL);
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

#ifdef S2OPC_EVENT_MANAGEMENT

static void forEachEventVar(const char* qnPath,
                            SOPC_Variant* var,
                            const SOPC_NodeId* dataType,
                            int32_t valueRank,
                            uintptr_t user_data)
{
    SOPC_UNUSED_ARG(user_data);
    SOPC_UNUSED_ARG(qnPath);
    if (SOPC_Null_Id == var->BuiltInTypeId && valueRank < 0 && OPCUA_NAMESPACE_INDEX == dataType->Namespace &&
        SOPC_IdentifierType_Numeric == dataType->IdentifierType && dataType->Data.Numeric <= SOPC_LocalizedText_Id &&
        dataType->Data.Numeric > SOPC_Null_Id)
    {
        bool res = true;
        SOPC_ReturnStatus status = SOPC_STATUS_OK;
        switch (dataType->Data.Numeric)
        {
        case SOPC_Boolean_Id:
            var->Value.Boolean = true;
            break;
        case SOPC_SByte_Id:
            var->Value.Sbyte = INT8_MAX;
            break;
        case SOPC_Byte_Id:
            var->Value.Byte = UINT8_MAX;
            break;
        case SOPC_Int16_Id:
            var->Value.Int16 = INT16_MAX;
            break;
        case SOPC_UInt16_Id:
            var->Value.Uint16 = UINT16_MAX;
            break;
        case SOPC_Int32_Id:
            var->Value.Int32 = INT32_MAX;
            break;
        case SOPC_UInt32_Id:
            var->Value.Uint32 = UINT32_MAX;
            break;
        case SOPC_Int64_Id:
            var->Value.Int64 = INT64_MAX;
            break;
        case SOPC_UInt64_Id:
            var->Value.Uint64 = UINT64_MAX;
            break;
        case SOPC_Float_Id:
            var->Value.Floatv = (float) 3.14;
            break;
        case SOPC_Double_Id:
            var->Value.Doublev = 2.0 / 3.0;
            break;
        case SOPC_String_Id:
            status = SOPC_String_AttachFromCstring(&var->Value.String, "An event string variable content !");
            res = (SOPC_STATUS_OK == status);
            break;
        case SOPC_DateTime_Id:
            var->Value.Date = SOPC_Time_GetCurrentTimeUTC();
            break;
        case SOPC_Guid_Id:
            var->Value.Guid = SOPC_Calloc(1, sizeof(*var->Value.Guid));
            res = false;
            if (NULL != var->Value.Guid)
            {
                status = SOPC_Guid_FromCString(var->Value.Guid, "53f474c1-c9bd-4b5d-803a-767c3a45e4e0",
                                               strlen("53f474c1-c9bd-4b5d-803a-767c3a45e4e0"));
                res = (SOPC_STATUS_OK == status);
            }
            break;
        case SOPC_ByteString_Id:
            status = SOPC_String_AttachFromCstring(&var->Value.String, "An event BYTESTRING variable content !");
            res = (SOPC_STATUS_OK == status);
            break;
        case SOPC_XmlElement_Id:
            status =
                SOPC_String_AttachFromCstring(&var->Value.String, "<xml>An event XML Element variable content !</xml>");
            res = (SOPC_STATUS_OK == status);
            break;
        case SOPC_NodeId_Id:
            var->Value.NodeId = SOPC_NodeId_FromCString("ns=42;s=Example of NodeId");
            res = (NULL != var->Value.Guid);
            break;
        case SOPC_ExpandedNodeId_Id:
            var->Value.ExpNodeId = SOPC_Calloc(1, sizeof(*var->Value.ExpNodeId));
            res = false;
            if (NULL != var->Value.ExpNodeId)
            {
                SOPC_ExpandedNodeId_Initialize(var->Value.ExpNodeId);
                var->Value.ExpNodeId->NodeId.Namespace = 42;
                var->Value.ExpNodeId->NodeId.IdentifierType = SOPC_IdentifierType_String;
                status =
                    SOPC_String_AttachFromCstring(&var->Value.ExpNodeId->NodeId.Data.String, "Example of ExpNodeId");
                res = (SOPC_STATUS_OK == status);
            }
            break;
        case SOPC_StatusCode_Id:
            var->Value.Status = OpcUa_UncertainSimulatedValue;
            break;
        case SOPC_QualifiedName_Id:
            var->Value.Qname = SOPC_Calloc(1, sizeof(*var->Value.Qname));
            res = false;
            if (NULL != var->Value.Qname)
            {
                status = SOPC_QualifiedName_ParseCString(var->Value.Qname, "42:Example of event QName");
                res = (SOPC_STATUS_OK == status);
            }
            break;
        case SOPC_LocalizedText_Id:
            var->Value.LocalizedText = SOPC_Calloc(1, sizeof(*var->Value.LocalizedText));
            res = false;
            if (NULL != var->Value.LocalizedText)
            {
                SOPC_LocalizedText_Initialize(var->Value.LocalizedText);
                status =
                    SOPC_String_AttachFromCstring(&var->Value.LocalizedText->defaultText, "Example of event LText");
                res = (SOPC_STATUS_OK == status);
            }
            break;
        default:
            res = false;
            break;
        }
        if (res)
        {
            var->BuiltInTypeId = dataType->Data.Numeric;
        }
    } // else: variant already set/initialized
}

SOPC_StatusCode SOPC_Method_Func_GenEvent(const SOPC_CallContext* callContextPtr,
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
    SOPC_ASSERT(NULL != inputArgs);
    SOPC_ASSERT(NULL != nbOutputArgs);
    SOPC_ASSERT(NULL != outputArgs);
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    if (4 != nbInputArgs || SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[0].ArrayType || SOPC_NodeId_Id != inputArgs[1].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[1].ArrayType || SOPC_UInt32_Id != inputArgs[2].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[2].ArrayType || SOPC_UInt32_Id != inputArgs[3].BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != inputArgs[3].ArrayType)
    {
        return OpcUa_BadInvalidArgument;
    }

    if (!SOPC_NodeId_Equal(&TestObject, objectId))
    {
        // Unexpected NodeId to apply method
        return OpcUa_BadNodeIdInvalid;
    }

    const SOPC_NodeId* eventTypeId = inputArgs[1].Value.NodeId;
    bool isNullId = SOPC_NodeId_Equal(&Null_Type, inputArgs[1].Value.NodeId);
    if (isNullId)
    {
        eventTypeId = &BaseEvent_Type;
    }

    SOPC_Event* eventInst = NULL;
    SOPC_ReturnStatus status = SOPC_ServerHelper_CreateEvent(eventTypeId, &eventInst);
    if (SOPC_STATUS_OK != status)
    {
        // Accepts only built in types
        return OpcUa_BadInvalidArgument;
    }

    status = SOPC_Event_SetNodeId(eventInst, &EventInstNodeId);

    if (SOPC_STATUS_OK == status)
    {
        SOPC_Event_ForEachVar(eventInst, forEachEventVar, 0);
    }

    SOPC_LocalizedText lt;
    SOPC_LocalizedText_Initialize(&lt);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_CopyFromCString(&lt.defaultText, "Manually generated event with GenEvent method");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetMessage(eventInst, &lt);
    }
    SOPC_String sourceName;
    SOPC_String_Initialize(&sourceName);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_AttachFromCstring(&sourceName, "GenEvent method");
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetSourceName(eventInst, &sourceName);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Event_SetTime(eventInst, SOPC_Time_GetCurrentTimeUTC());
    }

    uint32_t nbEvents = 1;
    if (inputArgs[0].Value.Uint32 > 1)
    {
        nbEvents = inputArgs[0].Value.Uint32;
    }

    for (uint32_t i = 0; SOPC_STATUS_OK == status && i < nbEvents; i++)
    {
        SOPC_Event* eventInstCopy = SOPC_Event_CreateCopy(eventInst, true);
        SOPC_ASSERT(NULL != eventInstCopy);
        status =
            SOPC_ServerHelper_TriggerEvent(&TestObject, eventInstCopy, SOPC_CallContext_GetSessionId(callContextPtr),
                                           inputArgs[2].Value.Uint32, inputArgs[3].Value.Uint32);
    }

    SOPC_Event_Delete(&eventInst);

    SOPC_LocalizedText_Clear(&lt);
    SOPC_String_Clear(&sourceName);

    if (SOPC_STATUS_OK != status)
    {
        return OpcUa_BadUnexpectedError;
    }
    return SOPC_GoodGenericStatus;
}
#endif

SOPC_StatusCode SOPC_Method_Func_AddRole(const SOPC_CallContext* callContextPtr,
                                         const SOPC_NodeId* objectId,
                                         uint32_t nbInputArgs,
                                         const SOPC_Variant* inputArgs,
                                         uint32_t* nbOutputArgs,
                                         SOPC_Variant** outputArgs,
                                         void* param)
{
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(nbInputArgs);

    SOPC_ASSERT(NULL != callContextPtr);
    SOPC_ASSERT(NULL != objectId);
    SOPC_ASSERT(NULL != inputArgs);
    SOPC_ASSERT(NULL != nbOutputArgs);
    SOPC_ASSERT(NULL != outputArgs);
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    return OpcUa_BadNotSupported;
}

SOPC_StatusCode SOPC_Method_Func_RemoveRole(const SOPC_CallContext* callContextPtr,
                                            const SOPC_NodeId* objectId,
                                            uint32_t nbInputArgs,
                                            const SOPC_Variant* inputArgs,
                                            uint32_t* nbOutputArgs,
                                            SOPC_Variant** outputArgs,
                                            void* param)

{
    SOPC_UNUSED_ARG(param);
    SOPC_UNUSED_ARG(nbInputArgs);

    SOPC_ASSERT(NULL != callContextPtr);
    SOPC_ASSERT(NULL != objectId);
    SOPC_ASSERT(NULL != inputArgs);
    SOPC_ASSERT(NULL != nbOutputArgs);
    SOPC_ASSERT(NULL != outputArgs);
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    return OpcUa_BadNotSupported;
}

SOPC_ReturnStatus SOPC_DemoServerConfig_AddMethods(SOPC_MethodCallManager* mcm)
{
    char* sNodeId;
    SOPC_NodeId* methodId;
    SOPC_MethodCallFunc_Ptr* methodFunc;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == mcm)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Add methods implementation in the method call manager used */
    /* No input, no output */
    sNodeId = "ns=1;s=MethodNoArg";
    methodId = SOPC_NodeId_FromCString(sNodeId);
    if (NULL != methodId)
    {
        methodFunc = &SOPC_Method_Func_IncCounter;
        status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "No input, no output", NULL);
        SOPC_NodeId_Clear(methodId);
        SOPC_Free(methodId);
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        /* Only input, no output */
        sNodeId = "ns=1;s=MethodI";
        methodId = SOPC_NodeId_FromCString(sNodeId);
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_AddToCounter;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "Only input, no output", NULL);
            SOPC_NodeId_Clear(methodId);
            SOPC_Free(methodId);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* No input, only output */
        sNodeId = "ns=1;s=MethodO";
        methodId = SOPC_NodeId_FromCString(sNodeId);
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_GetCounterValue;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "No input, only output", NULL);
            SOPC_NodeId_Clear(methodId);
            SOPC_Free(methodId);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        /* Input, output */
        sNodeId = "ns=1;s=MethodIO";
        methodId = SOPC_NodeId_FromCString(sNodeId);
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_UpdateAndGetPreviousHello;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "Input, output", NULL);
            SOPC_NodeId_Clear(methodId);
            SOPC_Free(methodId);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        sNodeId = "ns=1;s=AddVariableMethod";
        methodId = SOPC_NodeId_FromCString(sNodeId);
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_AddVariable;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "AddVariable", NULL);
            SOPC_NodeId_Clear(methodId);
            SOPC_Free(methodId);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

#ifdef S2OPC_EVENT_MANAGEMENT
    if (SOPC_STATUS_OK == status)
    {
        sNodeId = "ns=1;s=GenEventMethod";
        methodId = SOPC_NodeId_FromCString(sNodeId);
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_GenEvent;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, "GenEvent", NULL);
            SOPC_NodeId_Clear(methodId);
            SOPC_Free(methodId);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }
#endif

    if (SOPC_STATUS_OK == status)
    {
        sNodeId = "i=16301";
        methodId = SOPC_NodeId_FromCString(sNodeId);
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_AddRole;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, NULL, NULL);
            SOPC_NodeId_Clear(methodId);
            SOPC_Free(methodId);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        sNodeId = "i=16304";
        methodId = SOPC_NodeId_FromCString(sNodeId);
        if (NULL != methodId)
        {
            methodFunc = &SOPC_Method_Func_RemoveRole;
            status = SOPC_MethodCallManager_AddMethod(mcm, methodId, methodFunc, NULL, NULL);
            SOPC_NodeId_Clear(methodId);
            SOPC_Free(methodId);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}
