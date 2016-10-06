/* ========================================================================
 * Copyright (c) 2005-2016 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 * ======================================================================*/

/* standard */
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

/* stack */
#include <ua_encoder.h>

/* types */
#include <opcua_identifiers.h>

/* self */
#include <ua_types.h>

#ifndef OPCUA_EXCLUDE_Node
/*============================================================================
 * UA_Node_Initialize
 *===========================================================================*/
void UA_Node_Initialize(UA_Node* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
    }
}

/*============================================================================
 * UA_Node_Clear
 *===========================================================================*/
void UA_Node_Clear(UA_Node* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
    }
}

/*============================================================================
 * UA_Node_Encode
 *===========================================================================*/
StatusCode UA_Node_Encode(UA_Node* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_Node_Decode
 *===========================================================================*/
StatusCode UA_Node_Decode(UA_Node* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_Node_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);

    if(status != STATUS_OK){
        UA_Node_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_Node_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_Node_EncodeableType =
{
    "Node",
    OpcUaId_Node,
    OpcUaId_Node_Encoding_DefaultBinary,
    OpcUaId_Node_Encoding_DefaultXml,
    NULL,
    sizeof(UA_Node),
    (UA_EncodeableObject_PfnInitialize*)UA_Node_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_Node_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_Node_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_Node_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_Node_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_InstanceNode
/*============================================================================
 * UA_InstanceNode_Initialize
 *===========================================================================*/
void UA_InstanceNode_Initialize(UA_InstanceNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
    }
}

/*============================================================================
 * UA_InstanceNode_Clear
 *===========================================================================*/
void UA_InstanceNode_Clear(UA_InstanceNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
    }
}

/*============================================================================
 * UA_InstanceNode_Encode
 *===========================================================================*/
StatusCode UA_InstanceNode_Encode(UA_InstanceNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_InstanceNode_Decode
 *===========================================================================*/
StatusCode UA_InstanceNode_Decode(UA_InstanceNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_InstanceNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);

    if(status != STATUS_OK){
        UA_InstanceNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_InstanceNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_InstanceNode_EncodeableType =
{
    "InstanceNode",
    OpcUaId_InstanceNode,
    OpcUaId_InstanceNode_Encoding_DefaultBinary,
    OpcUaId_InstanceNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_InstanceNode),
    (UA_EncodeableObject_PfnInitialize*)UA_InstanceNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_InstanceNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_InstanceNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_InstanceNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_InstanceNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TypeNode
/*============================================================================
 * UA_TypeNode_Initialize
 *===========================================================================*/
void UA_TypeNode_Initialize(UA_TypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
    }
}

/*============================================================================
 * UA_TypeNode_Clear
 *===========================================================================*/
void UA_TypeNode_Clear(UA_TypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
    }
}

/*============================================================================
 * UA_TypeNode_Encode
 *===========================================================================*/
StatusCode UA_TypeNode_Encode(UA_TypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_TypeNode_Decode
 *===========================================================================*/
StatusCode UA_TypeNode_Decode(UA_TypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_TypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);

    if(status != STATUS_OK){
        UA_TypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_TypeNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_TypeNode_EncodeableType =
{
    "TypeNode",
    OpcUaId_TypeNode,
    OpcUaId_TypeNode_Encoding_DefaultBinary,
    OpcUaId_TypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_TypeNode),
    (UA_EncodeableObject_PfnInitialize*)UA_TypeNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_TypeNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_TypeNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_TypeNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_TypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ObjectNode
/*============================================================================
 * UA_ObjectNode_Initialize
 *===========================================================================*/
void UA_ObjectNode_Initialize(UA_ObjectNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
        Byte_Initialize(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * UA_ObjectNode_Clear
 *===========================================================================*/
void UA_ObjectNode_Clear(UA_ObjectNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
        Byte_Clear(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * UA_ObjectNode_Encode
 *===========================================================================*/
StatusCode UA_ObjectNode_Encode(UA_ObjectNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);
    status &= Byte_Write(&a_pValue->EventNotifier, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ObjectNode_Decode
 *===========================================================================*/
StatusCode UA_ObjectNode_Decode(UA_ObjectNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ObjectNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);
    status &= Byte_Read(&a_pValue->EventNotifier, msgBuf);

    if(status != STATUS_OK){
        UA_ObjectNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ObjectNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ObjectNode_EncodeableType =
{
    "ObjectNode",
    OpcUaId_ObjectNode,
    OpcUaId_ObjectNode_Encoding_DefaultBinary,
    OpcUaId_ObjectNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ObjectNode),
    (UA_EncodeableObject_PfnInitialize*)UA_ObjectNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ObjectNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ObjectNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ObjectNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ObjectNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeNode
/*============================================================================
 * UA_ObjectTypeNode_Initialize
 *===========================================================================*/
void UA_ObjectTypeNode_Initialize(UA_ObjectTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_ObjectTypeNode_Clear
 *===========================================================================*/
void UA_ObjectTypeNode_Clear(UA_ObjectTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_ObjectTypeNode_Encode
 *===========================================================================*/
StatusCode UA_ObjectTypeNode_Encode(UA_ObjectTypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ObjectTypeNode_Decode
 *===========================================================================*/
StatusCode UA_ObjectTypeNode_Decode(UA_ObjectTypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ObjectTypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        UA_ObjectTypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ObjectTypeNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ObjectTypeNode_EncodeableType =
{
    "ObjectTypeNode",
    OpcUaId_ObjectTypeNode,
    OpcUaId_ObjectTypeNode_Encoding_DefaultBinary,
    OpcUaId_ObjectTypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ObjectTypeNode),
    (UA_EncodeableObject_PfnInitialize*)UA_ObjectTypeNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ObjectTypeNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ObjectTypeNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ObjectTypeNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ObjectTypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_VariableNode
/*============================================================================
 * UA_VariableNode_Initialize
 *===========================================================================*/
void UA_VariableNode_Initialize(UA_VariableNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
        Variant_Initialize(&a_pValue->Value);
        NodeId_Initialize(&a_pValue->DataType);
        Int32_Initialize(&a_pValue->ValueRank);
        UA_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Byte_Initialize(&a_pValue->AccessLevel);
        Byte_Initialize(&a_pValue->UserAccessLevel);
        Double_Initialize(&a_pValue->MinimumSamplingInterval);
        Boolean_Initialize(&a_pValue->Historizing);
    }
}

/*============================================================================
 * UA_VariableNode_Clear
 *===========================================================================*/
void UA_VariableNode_Clear(UA_VariableNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
        Variant_Clear(&a_pValue->Value);
        NodeId_Clear(&a_pValue->DataType);
        Int32_Clear(&a_pValue->ValueRank);
        UA_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
        Byte_Clear(&a_pValue->AccessLevel);
        Byte_Clear(&a_pValue->UserAccessLevel);
        Double_Clear(&a_pValue->MinimumSamplingInterval);
        Boolean_Clear(&a_pValue->Historizing);
    }
}

/*============================================================================
 * UA_VariableNode_Encode
 *===========================================================================*/
StatusCode UA_VariableNode_Encode(UA_VariableNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);
    status &= Variant_Write(&a_pValue->Value, msgBuf);
    status &= NodeId_Write(&a_pValue->DataType, msgBuf);
    status &= Int32_Write(&a_pValue->ValueRank, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Byte_Write(&a_pValue->AccessLevel, msgBuf);
    status &= Byte_Write(&a_pValue->UserAccessLevel, msgBuf);
    status &= Double_Write(&a_pValue->MinimumSamplingInterval, msgBuf);
    status &= Boolean_Write(&a_pValue->Historizing, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_VariableNode_Decode
 *===========================================================================*/
StatusCode UA_VariableNode_Decode(UA_VariableNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_VariableNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);
    status &= Variant_Read(&a_pValue->Value, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Byte_Read(&a_pValue->AccessLevel, msgBuf);
    status &= Byte_Read(&a_pValue->UserAccessLevel, msgBuf);
    status &= Double_Read(&a_pValue->MinimumSamplingInterval, msgBuf);
    status &= Boolean_Read(&a_pValue->Historizing, msgBuf);

    if(status != STATUS_OK){
        UA_VariableNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_VariableNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_VariableNode_EncodeableType =
{
    "VariableNode",
    OpcUaId_VariableNode,
    OpcUaId_VariableNode_Encoding_DefaultBinary,
    OpcUaId_VariableNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_VariableNode),
    (UA_EncodeableObject_PfnInitialize*)UA_VariableNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_VariableNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_VariableNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_VariableNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_VariableNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeNode
/*============================================================================
 * UA_VariableTypeNode_Initialize
 *===========================================================================*/
void UA_VariableTypeNode_Initialize(UA_VariableTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
        Variant_Initialize(&a_pValue->Value);
        NodeId_Initialize(&a_pValue->DataType);
        Int32_Initialize(&a_pValue->ValueRank);
        UA_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_VariableTypeNode_Clear
 *===========================================================================*/
void UA_VariableTypeNode_Clear(UA_VariableTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
        Variant_Clear(&a_pValue->Value);
        NodeId_Clear(&a_pValue->DataType);
        Int32_Clear(&a_pValue->ValueRank);
        UA_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_VariableTypeNode_Encode
 *===========================================================================*/
StatusCode UA_VariableTypeNode_Encode(UA_VariableTypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);
    status &= Variant_Write(&a_pValue->Value, msgBuf);
    status &= NodeId_Write(&a_pValue->DataType, msgBuf);
    status &= Int32_Write(&a_pValue->ValueRank, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_VariableTypeNode_Decode
 *===========================================================================*/
StatusCode UA_VariableTypeNode_Decode(UA_VariableTypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_VariableTypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);
    status &= Variant_Read(&a_pValue->Value, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        UA_VariableTypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_VariableTypeNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_VariableTypeNode_EncodeableType =
{
    "VariableTypeNode",
    OpcUaId_VariableTypeNode,
    OpcUaId_VariableTypeNode_Encoding_DefaultBinary,
    OpcUaId_VariableTypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_VariableTypeNode),
    (UA_EncodeableObject_PfnInitialize*)UA_VariableTypeNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_VariableTypeNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_VariableTypeNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_VariableTypeNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_VariableTypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeNode
/*============================================================================
 * UA_ReferenceTypeNode_Initialize
 *===========================================================================*/
void UA_ReferenceTypeNode_Initialize(UA_ReferenceTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
        Boolean_Initialize(&a_pValue->Symmetric);
        LocalizedText_Initialize(&a_pValue->InverseName);
    }
}

/*============================================================================
 * UA_ReferenceTypeNode_Clear
 *===========================================================================*/
void UA_ReferenceTypeNode_Clear(UA_ReferenceTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
        Boolean_Clear(&a_pValue->Symmetric);
        LocalizedText_Clear(&a_pValue->InverseName);
    }
}

/*============================================================================
 * UA_ReferenceTypeNode_Encode
 *===========================================================================*/
StatusCode UA_ReferenceTypeNode_Encode(UA_ReferenceTypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);
    status &= Boolean_Write(&a_pValue->Symmetric, msgBuf);
    status &= LocalizedText_Write(&a_pValue->InverseName, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReferenceTypeNode_Decode
 *===========================================================================*/
StatusCode UA_ReferenceTypeNode_Decode(UA_ReferenceTypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReferenceTypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);
    status &= Boolean_Read(&a_pValue->Symmetric, msgBuf);
    status &= LocalizedText_Read(&a_pValue->InverseName, msgBuf);

    if(status != STATUS_OK){
        UA_ReferenceTypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReferenceTypeNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReferenceTypeNode_EncodeableType =
{
    "ReferenceTypeNode",
    OpcUaId_ReferenceTypeNode,
    OpcUaId_ReferenceTypeNode_Encoding_DefaultBinary,
    OpcUaId_ReferenceTypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReferenceTypeNode),
    (UA_EncodeableObject_PfnInitialize*)UA_ReferenceTypeNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReferenceTypeNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReferenceTypeNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReferenceTypeNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReferenceTypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MethodNode
/*============================================================================
 * UA_MethodNode_Initialize
 *===========================================================================*/
void UA_MethodNode_Initialize(UA_MethodNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->Executable);
        Boolean_Initialize(&a_pValue->UserExecutable);
    }
}

/*============================================================================
 * UA_MethodNode_Clear
 *===========================================================================*/
void UA_MethodNode_Clear(UA_MethodNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->Executable);
        Boolean_Clear(&a_pValue->UserExecutable);
    }
}

/*============================================================================
 * UA_MethodNode_Encode
 *===========================================================================*/
StatusCode UA_MethodNode_Encode(UA_MethodNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->Executable, msgBuf);
    status &= Boolean_Write(&a_pValue->UserExecutable, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_MethodNode_Decode
 *===========================================================================*/
StatusCode UA_MethodNode_Decode(UA_MethodNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_MethodNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->Executable, msgBuf);
    status &= Boolean_Read(&a_pValue->UserExecutable, msgBuf);

    if(status != STATUS_OK){
        UA_MethodNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_MethodNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_MethodNode_EncodeableType =
{
    "MethodNode",
    OpcUaId_MethodNode,
    OpcUaId_MethodNode_Encoding_DefaultBinary,
    OpcUaId_MethodNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_MethodNode),
    (UA_EncodeableObject_PfnInitialize*)UA_MethodNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_MethodNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_MethodNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_MethodNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_MethodNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ViewNode
/*============================================================================
 * UA_ViewNode_Initialize
 *===========================================================================*/
void UA_ViewNode_Initialize(UA_ViewNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->ContainsNoLoops);
        Byte_Initialize(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * UA_ViewNode_Clear
 *===========================================================================*/
void UA_ViewNode_Clear(UA_ViewNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->ContainsNoLoops);
        Byte_Clear(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * UA_ViewNode_Encode
 *===========================================================================*/
StatusCode UA_ViewNode_Encode(UA_ViewNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->ContainsNoLoops, msgBuf);
    status &= Byte_Write(&a_pValue->EventNotifier, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ViewNode_Decode
 *===========================================================================*/
StatusCode UA_ViewNode_Decode(UA_ViewNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ViewNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->ContainsNoLoops, msgBuf);
    status &= Byte_Read(&a_pValue->EventNotifier, msgBuf);

    if(status != STATUS_OK){
        UA_ViewNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ViewNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ViewNode_EncodeableType =
{
    "ViewNode",
    OpcUaId_ViewNode,
    OpcUaId_ViewNode_Encoding_DefaultBinary,
    OpcUaId_ViewNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ViewNode),
    (UA_EncodeableObject_PfnInitialize*)UA_ViewNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ViewNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ViewNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ViewNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ViewNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DataTypeNode
/*============================================================================
 * UA_DataTypeNode_Initialize
 *===========================================================================*/
void UA_DataTypeNode_Initialize(UA_DataTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_DataTypeNode_Clear
 *===========================================================================*/
void UA_DataTypeNode_Clear(UA_DataTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnClear*) UA_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_DataTypeNode_Encode
 *===========================================================================*/
StatusCode UA_DataTypeNode_Encode(UA_DataTypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnEncode*) UA_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DataTypeNode_Decode
 *===========================================================================*/
StatusCode UA_DataTypeNode_Decode(UA_DataTypeNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DataTypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceNode), (UA_EncodeableObject_PfnDecode*) UA_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        UA_DataTypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DataTypeNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DataTypeNode_EncodeableType =
{
    "DataTypeNode",
    OpcUaId_DataTypeNode,
    OpcUaId_DataTypeNode_Encoding_DefaultBinary,
    OpcUaId_DataTypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DataTypeNode),
    (UA_EncodeableObject_PfnInitialize*)UA_DataTypeNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DataTypeNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DataTypeNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DataTypeNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DataTypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReferenceNode
/*============================================================================
 * UA_ReferenceNode_Initialize
 *===========================================================================*/
void UA_ReferenceNode_Initialize(UA_ReferenceNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsInverse);
        ExpandedNodeId_Initialize(&a_pValue->TargetId);
    }
}

/*============================================================================
 * UA_ReferenceNode_Clear
 *===========================================================================*/
void UA_ReferenceNode_Clear(UA_ReferenceNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsInverse);
        ExpandedNodeId_Clear(&a_pValue->TargetId);
    }
}

/*============================================================================
 * UA_ReferenceNode_Encode
 *===========================================================================*/
StatusCode UA_ReferenceNode_Encode(UA_ReferenceNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsInverse, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->TargetId, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReferenceNode_Decode
 *===========================================================================*/
StatusCode UA_ReferenceNode_Decode(UA_ReferenceNode* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReferenceNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsInverse, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TargetId, msgBuf);

    if(status != STATUS_OK){
        UA_ReferenceNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReferenceNode_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReferenceNode_EncodeableType =
{
    "ReferenceNode",
    OpcUaId_ReferenceNode,
    OpcUaId_ReferenceNode_Encoding_DefaultBinary,
    OpcUaId_ReferenceNode_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReferenceNode),
    (UA_EncodeableObject_PfnInitialize*)UA_ReferenceNode_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReferenceNode_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReferenceNode_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReferenceNode_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReferenceNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Argument
/*============================================================================
 * UA_Argument_Initialize
 *===========================================================================*/
void UA_Argument_Initialize(UA_Argument* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->Name);
        NodeId_Initialize(&a_pValue->DataType);
        Int32_Initialize(&a_pValue->ValueRank);
        UA_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        LocalizedText_Initialize(&a_pValue->Description);
    }
}

/*============================================================================
 * UA_Argument_Clear
 *===========================================================================*/
void UA_Argument_Clear(UA_Argument* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->Name);
        NodeId_Clear(&a_pValue->DataType);
        Int32_Clear(&a_pValue->ValueRank);
        UA_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
        LocalizedText_Clear(&a_pValue->Description);
    }
}

/*============================================================================
 * UA_Argument_Encode
 *===========================================================================*/
StatusCode UA_Argument_Encode(UA_Argument* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->Name, msgBuf);
    status &= NodeId_Write(&a_pValue->DataType, msgBuf);
    status &= Int32_Write(&a_pValue->ValueRank, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_Argument_Decode
 *===========================================================================*/
StatusCode UA_Argument_Decode(UA_Argument* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_Argument_Initialize(a_pValue);

    status &= String_Read(&a_pValue->Name, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);

    if(status != STATUS_OK){
        UA_Argument_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_Argument_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_Argument_EncodeableType =
{
    "Argument",
    OpcUaId_Argument,
    OpcUaId_Argument_Encoding_DefaultBinary,
    OpcUaId_Argument_Encoding_DefaultXml,
    NULL,
    sizeof(UA_Argument),
    (UA_EncodeableObject_PfnInitialize*)UA_Argument_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_Argument_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_Argument_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_Argument_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_Argument_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EnumValueType
/*============================================================================
 * UA_EnumValueType_Initialize
 *===========================================================================*/
void UA_EnumValueType_Initialize(UA_EnumValueType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int64_Initialize(&a_pValue->Value);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
    }
}

/*============================================================================
 * UA_EnumValueType_Clear
 *===========================================================================*/
void UA_EnumValueType_Clear(UA_EnumValueType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int64_Clear(&a_pValue->Value);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
    }
}

/*============================================================================
 * UA_EnumValueType_Encode
 *===========================================================================*/
StatusCode UA_EnumValueType_Encode(UA_EnumValueType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Int64_Write(&a_pValue->Value, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EnumValueType_Decode
 *===========================================================================*/
StatusCode UA_EnumValueType_Decode(UA_EnumValueType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EnumValueType_Initialize(a_pValue);

    status &= Int64_Read(&a_pValue->Value, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);

    if(status != STATUS_OK){
        UA_EnumValueType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EnumValueType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EnumValueType_EncodeableType =
{
    "EnumValueType",
    OpcUaId_EnumValueType,
    OpcUaId_EnumValueType_Encoding_DefaultBinary,
    OpcUaId_EnumValueType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EnumValueType),
    (UA_EncodeableObject_PfnInitialize*)UA_EnumValueType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EnumValueType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EnumValueType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EnumValueType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EnumValueType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EnumField
/*============================================================================
 * UA_EnumField_Initialize
 *===========================================================================*/
void UA_EnumField_Initialize(UA_EnumField* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int64_Initialize(&a_pValue->Value);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        String_Initialize(&a_pValue->Name);
    }
}

/*============================================================================
 * UA_EnumField_Clear
 *===========================================================================*/
void UA_EnumField_Clear(UA_EnumField* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int64_Clear(&a_pValue->Value);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        String_Clear(&a_pValue->Name);
    }
}

/*============================================================================
 * UA_EnumField_Encode
 *===========================================================================*/
StatusCode UA_EnumField_Encode(UA_EnumField* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Int64_Write(&a_pValue->Value, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= String_Write(&a_pValue->Name, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EnumField_Decode
 *===========================================================================*/
StatusCode UA_EnumField_Decode(UA_EnumField* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EnumField_Initialize(a_pValue);

    status &= Int64_Read(&a_pValue->Value, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= String_Read(&a_pValue->Name, msgBuf);

    if(status != STATUS_OK){
        UA_EnumField_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EnumField_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EnumField_EncodeableType =
{
    "EnumField",
    OpcUaId_EnumField,
    OpcUaId_EnumField_Encoding_DefaultBinary,
    OpcUaId_EnumField_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EnumField),
    (UA_EncodeableObject_PfnInitialize*)UA_EnumField_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EnumField_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EnumField_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EnumField_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EnumField_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_OptionSet
/*============================================================================
 * UA_OptionSet_Initialize
 *===========================================================================*/
void UA_OptionSet_Initialize(UA_OptionSet* a_pValue)
{
    if (a_pValue != NULL)
    {
        ByteString_Initialize(&a_pValue->Value);
        ByteString_Initialize(&a_pValue->ValidBits);
    }
}

/*============================================================================
 * UA_OptionSet_Clear
 *===========================================================================*/
void UA_OptionSet_Clear(UA_OptionSet* a_pValue)
{
    if (a_pValue != NULL)
    {
        ByteString_Clear(&a_pValue->Value);
        ByteString_Clear(&a_pValue->ValidBits);
    }
}

/*============================================================================
 * UA_OptionSet_Encode
 *===========================================================================*/
StatusCode UA_OptionSet_Encode(UA_OptionSet* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= ByteString_Write(&a_pValue->Value, msgBuf);
    status &= ByteString_Write(&a_pValue->ValidBits, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_OptionSet_Decode
 *===========================================================================*/
StatusCode UA_OptionSet_Decode(UA_OptionSet* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_OptionSet_Initialize(a_pValue);

    status &= ByteString_Read(&a_pValue->Value, msgBuf);
    status &= ByteString_Read(&a_pValue->ValidBits, msgBuf);

    if(status != STATUS_OK){
        UA_OptionSet_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_OptionSet_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_OptionSet_EncodeableType =
{
    "OptionSet",
    OpcUaId_OptionSet,
    OpcUaId_OptionSet_Encoding_DefaultBinary,
    OpcUaId_OptionSet_Encoding_DefaultXml,
    NULL,
    sizeof(UA_OptionSet),
    (UA_EncodeableObject_PfnInitialize*)UA_OptionSet_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_OptionSet_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_OptionSet_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_OptionSet_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_OptionSet_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TimeZoneDataType
/*============================================================================
 * UA_TimeZoneDataType_Initialize
 *===========================================================================*/
void UA_TimeZoneDataType_Initialize(UA_TimeZoneDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int16_Initialize(&a_pValue->Offset);
        Boolean_Initialize(&a_pValue->DaylightSavingInOffset);
    }
}

/*============================================================================
 * UA_TimeZoneDataType_Clear
 *===========================================================================*/
void UA_TimeZoneDataType_Clear(UA_TimeZoneDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int16_Clear(&a_pValue->Offset);
        Boolean_Clear(&a_pValue->DaylightSavingInOffset);
    }
}

/*============================================================================
 * UA_TimeZoneDataType_Encode
 *===========================================================================*/
StatusCode UA_TimeZoneDataType_Encode(UA_TimeZoneDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Int16_Write(&a_pValue->Offset, msgBuf);
    status &= Boolean_Write(&a_pValue->DaylightSavingInOffset, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_TimeZoneDataType_Decode
 *===========================================================================*/
StatusCode UA_TimeZoneDataType_Decode(UA_TimeZoneDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_TimeZoneDataType_Initialize(a_pValue);

    status &= Int16_Read(&a_pValue->Offset, msgBuf);
    status &= Boolean_Read(&a_pValue->DaylightSavingInOffset, msgBuf);

    if(status != STATUS_OK){
        UA_TimeZoneDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_TimeZoneDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_TimeZoneDataType_EncodeableType =
{
    "TimeZoneDataType",
    OpcUaId_TimeZoneDataType,
    OpcUaId_TimeZoneDataType_Encoding_DefaultBinary,
    OpcUaId_TimeZoneDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_TimeZoneDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_TimeZoneDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_TimeZoneDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_TimeZoneDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_TimeZoneDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_TimeZoneDataType_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_ApplicationDescription
/*============================================================================
 * UA_ApplicationDescription_Initialize
 *===========================================================================*/
void UA_ApplicationDescription_Initialize(UA_ApplicationDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ApplicationUri);
        String_Initialize(&a_pValue->ProductUri);
        LocalizedText_Initialize(&a_pValue->ApplicationName);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->ApplicationType);
        String_Initialize(&a_pValue->GatewayServerUri);
        String_Initialize(&a_pValue->DiscoveryProfileUri);
        UA_Initialize_Array(&a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * UA_ApplicationDescription_Clear
 *===========================================================================*/
void UA_ApplicationDescription_Clear(UA_ApplicationDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ApplicationUri);
        String_Clear(&a_pValue->ProductUri);
        LocalizedText_Clear(&a_pValue->ApplicationName);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->ApplicationType);
        String_Clear(&a_pValue->GatewayServerUri);
        String_Clear(&a_pValue->DiscoveryProfileUri);
        UA_Clear_Array(&a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * UA_ApplicationDescription_Encode
 *===========================================================================*/
StatusCode UA_ApplicationDescription_Encode(UA_ApplicationDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ApplicationUri, msgBuf);
    status &= String_Write(&a_pValue->ProductUri, msgBuf);
    status &= LocalizedText_Write(&a_pValue->ApplicationName, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ApplicationType);
    status &= String_Write(&a_pValue->GatewayServerUri, msgBuf);
    status &= String_Write(&a_pValue->DiscoveryProfileUri, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ApplicationDescription_Decode
 *===========================================================================*/
StatusCode UA_ApplicationDescription_Decode(UA_ApplicationDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ApplicationDescription_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ApplicationUri, msgBuf);
    status &= String_Read(&a_pValue->ProductUri, msgBuf);
    status &= LocalizedText_Read(&a_pValue->ApplicationName, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ApplicationType);
    status &= String_Read(&a_pValue->GatewayServerUri, msgBuf);
    status &= String_Read(&a_pValue->DiscoveryProfileUri, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        UA_ApplicationDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ApplicationDescription_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ApplicationDescription_EncodeableType =
{
    "ApplicationDescription",
    OpcUaId_ApplicationDescription,
    OpcUaId_ApplicationDescription_Encoding_DefaultBinary,
    OpcUaId_ApplicationDescription_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ApplicationDescription),
    (UA_EncodeableObject_PfnInitialize*)UA_ApplicationDescription_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ApplicationDescription_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ApplicationDescription_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ApplicationDescription_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ApplicationDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RequestHeader
/*============================================================================
 * UA_RequestHeader_Initialize
 *===========================================================================*/
void UA_RequestHeader_Initialize(UA_RequestHeader* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->AuthenticationToken);
        DateTime_Initialize(&a_pValue->Timestamp);
        UInt32_Initialize(&a_pValue->RequestHandle);
        UInt32_Initialize(&a_pValue->ReturnDiagnostics);
        String_Initialize(&a_pValue->AuditEntryId);
        UInt32_Initialize(&a_pValue->TimeoutHint);
        ExtensionObject_Initialize(&a_pValue->AdditionalHeader);
    }
}

/*============================================================================
 * UA_RequestHeader_Clear
 *===========================================================================*/
void UA_RequestHeader_Clear(UA_RequestHeader* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->AuthenticationToken);
        DateTime_Clear(&a_pValue->Timestamp);
        UInt32_Clear(&a_pValue->RequestHandle);
        UInt32_Clear(&a_pValue->ReturnDiagnostics);
        String_Clear(&a_pValue->AuditEntryId);
        UInt32_Clear(&a_pValue->TimeoutHint);
        ExtensionObject_Clear(&a_pValue->AdditionalHeader);
    }
}

/*============================================================================
 * UA_RequestHeader_Encode
 *===========================================================================*/
StatusCode UA_RequestHeader_Encode(UA_RequestHeader* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->AuthenticationToken, msgBuf);
    status &= DateTime_Write(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestHandle, msgBuf);
    status &= UInt32_Write(&a_pValue->ReturnDiagnostics, msgBuf);
    status &= String_Write(&a_pValue->AuditEntryId, msgBuf);
    status &= UInt32_Write(&a_pValue->TimeoutHint, msgBuf);
    status &= ExtensionObject_Write(&a_pValue->AdditionalHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RequestHeader_Decode
 *===========================================================================*/
StatusCode UA_RequestHeader_Decode(UA_RequestHeader* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RequestHeader_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->AuthenticationToken, msgBuf);
    status &= DateTime_Read(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestHandle, msgBuf);
    status &= UInt32_Read(&a_pValue->ReturnDiagnostics, msgBuf);
    status &= String_Read(&a_pValue->AuditEntryId, msgBuf);
    status &= UInt32_Read(&a_pValue->TimeoutHint, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->AdditionalHeader, msgBuf);

    if(status != STATUS_OK){
        UA_RequestHeader_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RequestHeader_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RequestHeader_EncodeableType =
{
    "RequestHeader",
    OpcUaId_RequestHeader,
    OpcUaId_RequestHeader_Encoding_DefaultBinary,
    OpcUaId_RequestHeader_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RequestHeader),
    (UA_EncodeableObject_PfnInitialize*)UA_RequestHeader_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RequestHeader_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RequestHeader_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RequestHeader_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RequestHeader_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ResponseHeader
/*============================================================================
 * UA_ResponseHeader_Initialize
 *===========================================================================*/
void UA_ResponseHeader_Initialize(UA_ResponseHeader* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->Timestamp);
        UInt32_Initialize(&a_pValue->RequestHandle);
        StatusCode_Initialize(&a_pValue->ServiceResult);
        DiagnosticInfo_Initialize(&a_pValue->ServiceDiagnostics);
        UA_Initialize_Array(&a_pValue->NoOfStringTable, (void**) &a_pValue->StringTable, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
        ExtensionObject_Initialize(&a_pValue->AdditionalHeader);
    }
}

/*============================================================================
 * UA_ResponseHeader_Clear
 *===========================================================================*/
void UA_ResponseHeader_Clear(UA_ResponseHeader* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->Timestamp);
        UInt32_Clear(&a_pValue->RequestHandle);
        StatusCode_Clear(&a_pValue->ServiceResult);
        DiagnosticInfo_Clear(&a_pValue->ServiceDiagnostics);
        UA_Clear_Array(&a_pValue->NoOfStringTable, (void**) &a_pValue->StringTable, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
        ExtensionObject_Clear(&a_pValue->AdditionalHeader);
    }
}

/*============================================================================
 * UA_ResponseHeader_Encode
 *===========================================================================*/
StatusCode UA_ResponseHeader_Encode(UA_ResponseHeader* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestHandle, msgBuf);
    status &= StatusCode_Write(&a_pValue->ServiceResult, msgBuf);
    status &= DiagnosticInfo_Write(&a_pValue->ServiceDiagnostics, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfStringTable, (void**) &a_pValue->StringTable, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);
    status &= ExtensionObject_Write(&a_pValue->AdditionalHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ResponseHeader_Decode
 *===========================================================================*/
StatusCode UA_ResponseHeader_Decode(UA_ResponseHeader* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ResponseHeader_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestHandle, msgBuf);
    status &= StatusCode_Read(&a_pValue->ServiceResult, msgBuf);
    status &= DiagnosticInfo_Read(&a_pValue->ServiceDiagnostics, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfStringTable, (void**) &a_pValue->StringTable, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);
    status &= ExtensionObject_Read(&a_pValue->AdditionalHeader, msgBuf);

    if(status != STATUS_OK){
        UA_ResponseHeader_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ResponseHeader_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ResponseHeader_EncodeableType =
{
    "ResponseHeader",
    OpcUaId_ResponseHeader,
    OpcUaId_ResponseHeader_Encoding_DefaultBinary,
    OpcUaId_ResponseHeader_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ResponseHeader),
    (UA_EncodeableObject_PfnInitialize*)UA_ResponseHeader_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ResponseHeader_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ResponseHeader_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ResponseHeader_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ResponseHeader_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ServiceFault
/*============================================================================
 * UA_ServiceFault_Initialize
 *===========================================================================*/
void UA_ServiceFault_Initialize(UA_ServiceFault* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_ServiceFault_Clear
 *===========================================================================*/
void UA_ServiceFault_Clear(UA_ServiceFault* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_ServiceFault_Encode
 *===========================================================================*/
StatusCode UA_ServiceFault_Encode(UA_ServiceFault* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ServiceFault_Decode
 *===========================================================================*/
StatusCode UA_ServiceFault_Decode(UA_ServiceFault* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ServiceFault_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        UA_ServiceFault_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ServiceFault_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ServiceFault_EncodeableType =
{
    "ServiceFault",
    OpcUaId_ServiceFault,
    OpcUaId_ServiceFault_Encoding_DefaultBinary,
    OpcUaId_ServiceFault_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ServiceFault),
    (UA_EncodeableObject_PfnInitialize*)UA_ServiceFault_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ServiceFault_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ServiceFault_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ServiceFault_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ServiceFault_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_FindServers
#ifndef OPCUA_EXCLUDE_FindServersRequest
/*============================================================================
 * UA_FindServersRequest_Initialize
 *===========================================================================*/
void UA_FindServersRequest_Initialize(UA_FindServersRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        String_Initialize(&a_pValue->EndpointUrl);
        UA_Initialize_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfServerUris, (void**) &a_pValue->ServerUris, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * UA_FindServersRequest_Clear
 *===========================================================================*/
void UA_FindServersRequest_Clear(UA_FindServersRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        String_Clear(&a_pValue->EndpointUrl);
        UA_Clear_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
        UA_Clear_Array(&a_pValue->NoOfServerUris, (void**) &a_pValue->ServerUris, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * UA_FindServersRequest_Encode
 *===========================================================================*/
StatusCode UA_FindServersRequest_Encode(UA_FindServersRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= String_Write(&a_pValue->EndpointUrl, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfServerUris, (void**) &a_pValue->ServerUris, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_FindServersRequest_Decode
 *===========================================================================*/
StatusCode UA_FindServersRequest_Decode(UA_FindServersRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_FindServersRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfServerUris, (void**) &a_pValue->ServerUris, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        UA_FindServersRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_FindServersRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_FindServersRequest_EncodeableType =
{
    "FindServersRequest",
    OpcUaId_FindServersRequest,
    OpcUaId_FindServersRequest_Encoding_DefaultBinary,
    OpcUaId_FindServersRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_FindServersRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_FindServersRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_FindServersRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_FindServersRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_FindServersRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_FindServersRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_FindServersResponse
/*============================================================================
 * UA_FindServersResponse_Initialize
 *===========================================================================*/
void UA_FindServersResponse_Initialize(UA_FindServersResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                            sizeof(UA_ApplicationDescription), (UA_EncodeableObject_PfnInitialize*) UA_ApplicationDescription_Initialize);
    }
}

/*============================================================================
 * UA_FindServersResponse_Clear
 *===========================================================================*/
void UA_FindServersResponse_Clear(UA_FindServersResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                       sizeof(UA_ApplicationDescription), (UA_EncodeableObject_PfnClear*) UA_ApplicationDescription_Clear);
    }
}

/*============================================================================
 * UA_FindServersResponse_Encode
 *===========================================================================*/
StatusCode UA_FindServersResponse_Encode(UA_FindServersResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                   sizeof(UA_ApplicationDescription), (UA_EncodeableObject_PfnEncode*) UA_ApplicationDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_FindServersResponse_Decode
 *===========================================================================*/
StatusCode UA_FindServersResponse_Decode(UA_FindServersResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_FindServersResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                  sizeof(UA_ApplicationDescription), (UA_EncodeableObject_PfnDecode*) UA_ApplicationDescription_Decode);

    if(status != STATUS_OK){
        UA_FindServersResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_FindServersResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_FindServersResponse_EncodeableType =
{
    "FindServersResponse",
    OpcUaId_FindServersResponse,
    OpcUaId_FindServersResponse_Encoding_DefaultBinary,
    OpcUaId_FindServersResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_FindServersResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_FindServersResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_FindServersResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_FindServersResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_FindServersResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_FindServersResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_ServerOnNetwork
/*============================================================================
 * UA_ServerOnNetwork_Initialize
 *===========================================================================*/
void UA_ServerOnNetwork_Initialize(UA_ServerOnNetwork* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->RecordId);
        String_Initialize(&a_pValue->ServerName);
        String_Initialize(&a_pValue->DiscoveryUrl);
        UA_Initialize_Array(&a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * UA_ServerOnNetwork_Clear
 *===========================================================================*/
void UA_ServerOnNetwork_Clear(UA_ServerOnNetwork* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->RecordId);
        String_Clear(&a_pValue->ServerName);
        String_Clear(&a_pValue->DiscoveryUrl);
        UA_Clear_Array(&a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * UA_ServerOnNetwork_Encode
 *===========================================================================*/
StatusCode UA_ServerOnNetwork_Encode(UA_ServerOnNetwork* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->RecordId, msgBuf);
    status &= String_Write(&a_pValue->ServerName, msgBuf);
    status &= String_Write(&a_pValue->DiscoveryUrl, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ServerOnNetwork_Decode
 *===========================================================================*/
StatusCode UA_ServerOnNetwork_Decode(UA_ServerOnNetwork* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ServerOnNetwork_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->RecordId, msgBuf);
    status &= String_Read(&a_pValue->ServerName, msgBuf);
    status &= String_Read(&a_pValue->DiscoveryUrl, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        UA_ServerOnNetwork_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ServerOnNetwork_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ServerOnNetwork_EncodeableType =
{
    "ServerOnNetwork",
    OpcUaId_ServerOnNetwork,
    OpcUaId_ServerOnNetwork_Encoding_DefaultBinary,
    OpcUaId_ServerOnNetwork_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ServerOnNetwork),
    (UA_EncodeableObject_PfnInitialize*)UA_ServerOnNetwork_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ServerOnNetwork_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ServerOnNetwork_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ServerOnNetwork_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ServerOnNetwork_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
#ifndef OPCUA_EXCLUDE_FindServersOnNetworkRequest
/*============================================================================
 * UA_FindServersOnNetworkRequest_Initialize
 *===========================================================================*/
void UA_FindServersOnNetworkRequest_Initialize(UA_FindServersOnNetworkRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->StartingRecordId);
        UInt32_Initialize(&a_pValue->MaxRecordsToReturn);
        UA_Initialize_Array(&a_pValue->NoOfServerCapabilityFilter, (void**) &a_pValue->ServerCapabilityFilter, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * UA_FindServersOnNetworkRequest_Clear
 *===========================================================================*/
void UA_FindServersOnNetworkRequest_Clear(UA_FindServersOnNetworkRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->StartingRecordId);
        UInt32_Clear(&a_pValue->MaxRecordsToReturn);
        UA_Clear_Array(&a_pValue->NoOfServerCapabilityFilter, (void**) &a_pValue->ServerCapabilityFilter, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * UA_FindServersOnNetworkRequest_Encode
 *===========================================================================*/
StatusCode UA_FindServersOnNetworkRequest_Encode(UA_FindServersOnNetworkRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->StartingRecordId, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxRecordsToReturn, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfServerCapabilityFilter, (void**) &a_pValue->ServerCapabilityFilter, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_FindServersOnNetworkRequest_Decode
 *===========================================================================*/
StatusCode UA_FindServersOnNetworkRequest_Decode(UA_FindServersOnNetworkRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_FindServersOnNetworkRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->StartingRecordId, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxRecordsToReturn, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfServerCapabilityFilter, (void**) &a_pValue->ServerCapabilityFilter, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        UA_FindServersOnNetworkRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_FindServersOnNetworkRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_FindServersOnNetworkRequest_EncodeableType =
{
    "FindServersOnNetworkRequest",
    OpcUaId_FindServersOnNetworkRequest,
    OpcUaId_FindServersOnNetworkRequest_Encoding_DefaultBinary,
    OpcUaId_FindServersOnNetworkRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_FindServersOnNetworkRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_FindServersOnNetworkRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_FindServersOnNetworkRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_FindServersOnNetworkRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_FindServersOnNetworkRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_FindServersOnNetworkRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetworkResponse
/*============================================================================
 * UA_FindServersOnNetworkResponse_Initialize
 *===========================================================================*/
void UA_FindServersOnNetworkResponse_Initialize(UA_FindServersOnNetworkResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        DateTime_Initialize(&a_pValue->LastCounterResetTime);
        UA_Initialize_Array(&a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                            sizeof(UA_ServerOnNetwork), (UA_EncodeableObject_PfnInitialize*) UA_ServerOnNetwork_Initialize);
    }
}

/*============================================================================
 * UA_FindServersOnNetworkResponse_Clear
 *===========================================================================*/
void UA_FindServersOnNetworkResponse_Clear(UA_FindServersOnNetworkResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        DateTime_Clear(&a_pValue->LastCounterResetTime);
        UA_Clear_Array(&a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                       sizeof(UA_ServerOnNetwork), (UA_EncodeableObject_PfnClear*) UA_ServerOnNetwork_Clear);
    }
}

/*============================================================================
 * UA_FindServersOnNetworkResponse_Encode
 *===========================================================================*/
StatusCode UA_FindServersOnNetworkResponse_Encode(UA_FindServersOnNetworkResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= DateTime_Write(&a_pValue->LastCounterResetTime, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                   sizeof(UA_ServerOnNetwork), (UA_EncodeableObject_PfnEncode*) UA_ServerOnNetwork_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_FindServersOnNetworkResponse_Decode
 *===========================================================================*/
StatusCode UA_FindServersOnNetworkResponse_Decode(UA_FindServersOnNetworkResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_FindServersOnNetworkResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= DateTime_Read(&a_pValue->LastCounterResetTime, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                  sizeof(UA_ServerOnNetwork), (UA_EncodeableObject_PfnDecode*) UA_ServerOnNetwork_Decode);

    if(status != STATUS_OK){
        UA_FindServersOnNetworkResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_FindServersOnNetworkResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_FindServersOnNetworkResponse_EncodeableType =
{
    "FindServersOnNetworkResponse",
    OpcUaId_FindServersOnNetworkResponse,
    OpcUaId_FindServersOnNetworkResponse_Encoding_DefaultBinary,
    OpcUaId_FindServersOnNetworkResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_FindServersOnNetworkResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_FindServersOnNetworkResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_FindServersOnNetworkResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_FindServersOnNetworkResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_FindServersOnNetworkResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_FindServersOnNetworkResponse_Decode
};
#endif
#endif



#ifndef OPCUA_EXCLUDE_UserTokenPolicy
/*============================================================================
 * UA_UserTokenPolicy_Initialize
 *===========================================================================*/
void UA_UserTokenPolicy_Initialize(UA_UserTokenPolicy* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->TokenType);
        String_Initialize(&a_pValue->IssuedTokenType);
        String_Initialize(&a_pValue->IssuerEndpointUrl);
        String_Initialize(&a_pValue->SecurityPolicyUri);
    }
}

/*============================================================================
 * UA_UserTokenPolicy_Clear
 *===========================================================================*/
void UA_UserTokenPolicy_Clear(UA_UserTokenPolicy* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->TokenType);
        String_Clear(&a_pValue->IssuedTokenType);
        String_Clear(&a_pValue->IssuerEndpointUrl);
        String_Clear(&a_pValue->SecurityPolicyUri);
    }
}

/*============================================================================
 * UA_UserTokenPolicy_Encode
 *===========================================================================*/
StatusCode UA_UserTokenPolicy_Encode(UA_UserTokenPolicy* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->PolicyId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TokenType);
    status &= String_Write(&a_pValue->IssuedTokenType, msgBuf);
    status &= String_Write(&a_pValue->IssuerEndpointUrl, msgBuf);
    status &= String_Write(&a_pValue->SecurityPolicyUri, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_UserTokenPolicy_Decode
 *===========================================================================*/
StatusCode UA_UserTokenPolicy_Decode(UA_UserTokenPolicy* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_UserTokenPolicy_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TokenType);
    status &= String_Read(&a_pValue->IssuedTokenType, msgBuf);
    status &= String_Read(&a_pValue->IssuerEndpointUrl, msgBuf);
    status &= String_Read(&a_pValue->SecurityPolicyUri, msgBuf);

    if(status != STATUS_OK){
        UA_UserTokenPolicy_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_UserTokenPolicy_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_UserTokenPolicy_EncodeableType =
{
    "UserTokenPolicy",
    OpcUaId_UserTokenPolicy,
    OpcUaId_UserTokenPolicy_Encoding_DefaultBinary,
    OpcUaId_UserTokenPolicy_Encoding_DefaultXml,
    NULL,
    sizeof(UA_UserTokenPolicy),
    (UA_EncodeableObject_PfnInitialize*)UA_UserTokenPolicy_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_UserTokenPolicy_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_UserTokenPolicy_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_UserTokenPolicy_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_UserTokenPolicy_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EndpointDescription
/*============================================================================
 * UA_EndpointDescription_Initialize
 *===========================================================================*/
void UA_EndpointDescription_Initialize(UA_EndpointDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->EndpointUrl);
        UA_ApplicationDescription_Initialize(&a_pValue->Server);
        ByteString_Initialize(&a_pValue->ServerCertificate);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        String_Initialize(&a_pValue->SecurityPolicyUri);
        UA_Initialize_Array(&a_pValue->NoOfUserIdentityTokens, (void**) &a_pValue->UserIdentityTokens, 
                            sizeof(UA_UserTokenPolicy), (UA_EncodeableObject_PfnInitialize*) UA_UserTokenPolicy_Initialize);
        String_Initialize(&a_pValue->TransportProfileUri);
        Byte_Initialize(&a_pValue->SecurityLevel);
    }
}

/*============================================================================
 * UA_EndpointDescription_Clear
 *===========================================================================*/
void UA_EndpointDescription_Clear(UA_EndpointDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->EndpointUrl);
        UA_ApplicationDescription_Clear(&a_pValue->Server);
        ByteString_Clear(&a_pValue->ServerCertificate);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        String_Clear(&a_pValue->SecurityPolicyUri);
        UA_Clear_Array(&a_pValue->NoOfUserIdentityTokens, (void**) &a_pValue->UserIdentityTokens, 
                       sizeof(UA_UserTokenPolicy), (UA_EncodeableObject_PfnClear*) UA_UserTokenPolicy_Clear);
        String_Clear(&a_pValue->TransportProfileUri);
        Byte_Clear(&a_pValue->SecurityLevel);
    }
}

/*============================================================================
 * UA_EndpointDescription_Encode
 *===========================================================================*/
StatusCode UA_EndpointDescription_Encode(UA_EndpointDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->EndpointUrl, msgBuf);
    status &= UA_ApplicationDescription_Encode(&a_pValue->Server, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerCertificate, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= String_Write(&a_pValue->SecurityPolicyUri, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfUserIdentityTokens, (void**) &a_pValue->UserIdentityTokens, 
                   sizeof(UA_UserTokenPolicy), (UA_EncodeableObject_PfnEncode*) UA_UserTokenPolicy_Encode);
    status &= String_Write(&a_pValue->TransportProfileUri, msgBuf);
    status &= Byte_Write(&a_pValue->SecurityLevel, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EndpointDescription_Decode
 *===========================================================================*/
StatusCode UA_EndpointDescription_Decode(UA_EndpointDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EndpointDescription_Initialize(a_pValue);

    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= UA_ApplicationDescription_Decode(&a_pValue->Server, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerCertificate, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= String_Read(&a_pValue->SecurityPolicyUri, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfUserIdentityTokens, (void**) &a_pValue->UserIdentityTokens, 
                  sizeof(UA_UserTokenPolicy), (UA_EncodeableObject_PfnDecode*) UA_UserTokenPolicy_Decode);
    status &= String_Read(&a_pValue->TransportProfileUri, msgBuf);
    status &= Byte_Read(&a_pValue->SecurityLevel, msgBuf);

    if(status != STATUS_OK){
        UA_EndpointDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EndpointDescription_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EndpointDescription_EncodeableType =
{
    "EndpointDescription",
    OpcUaId_EndpointDescription,
    OpcUaId_EndpointDescription_Encoding_DefaultBinary,
    OpcUaId_EndpointDescription_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EndpointDescription),
    (UA_EncodeableObject_PfnInitialize*)UA_EndpointDescription_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EndpointDescription_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EndpointDescription_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EndpointDescription_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EndpointDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
#ifndef OPCUA_EXCLUDE_GetEndpointsRequest
/*============================================================================
 * UA_GetEndpointsRequest_Initialize
 *===========================================================================*/
void UA_GetEndpointsRequest_Initialize(UA_GetEndpointsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        String_Initialize(&a_pValue->EndpointUrl);
        UA_Initialize_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfProfileUris, (void**) &a_pValue->ProfileUris, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * UA_GetEndpointsRequest_Clear
 *===========================================================================*/
void UA_GetEndpointsRequest_Clear(UA_GetEndpointsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        String_Clear(&a_pValue->EndpointUrl);
        UA_Clear_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
        UA_Clear_Array(&a_pValue->NoOfProfileUris, (void**) &a_pValue->ProfileUris, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * UA_GetEndpointsRequest_Encode
 *===========================================================================*/
StatusCode UA_GetEndpointsRequest_Encode(UA_GetEndpointsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= String_Write(&a_pValue->EndpointUrl, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfProfileUris, (void**) &a_pValue->ProfileUris, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_GetEndpointsRequest_Decode
 *===========================================================================*/
StatusCode UA_GetEndpointsRequest_Decode(UA_GetEndpointsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_GetEndpointsRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfProfileUris, (void**) &a_pValue->ProfileUris, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        UA_GetEndpointsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_GetEndpointsRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_GetEndpointsRequest_EncodeableType =
{
    "GetEndpointsRequest",
    OpcUaId_GetEndpointsRequest,
    OpcUaId_GetEndpointsRequest_Encoding_DefaultBinary,
    OpcUaId_GetEndpointsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_GetEndpointsRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_GetEndpointsRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_GetEndpointsRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_GetEndpointsRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_GetEndpointsRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_GetEndpointsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_GetEndpointsResponse
/*============================================================================
 * UA_GetEndpointsResponse_Initialize
 *===========================================================================*/
void UA_GetEndpointsResponse_Initialize(UA_GetEndpointsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfEndpoints, (void**) &a_pValue->Endpoints, 
                            sizeof(UA_EndpointDescription), (UA_EncodeableObject_PfnInitialize*) UA_EndpointDescription_Initialize);
    }
}

/*============================================================================
 * UA_GetEndpointsResponse_Clear
 *===========================================================================*/
void UA_GetEndpointsResponse_Clear(UA_GetEndpointsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfEndpoints, (void**) &a_pValue->Endpoints, 
                       sizeof(UA_EndpointDescription), (UA_EncodeableObject_PfnClear*) UA_EndpointDescription_Clear);
    }
}

/*============================================================================
 * UA_GetEndpointsResponse_Encode
 *===========================================================================*/
StatusCode UA_GetEndpointsResponse_Encode(UA_GetEndpointsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfEndpoints, (void**) &a_pValue->Endpoints, 
                   sizeof(UA_EndpointDescription), (UA_EncodeableObject_PfnEncode*) UA_EndpointDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_GetEndpointsResponse_Decode
 *===========================================================================*/
StatusCode UA_GetEndpointsResponse_Decode(UA_GetEndpointsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_GetEndpointsResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfEndpoints, (void**) &a_pValue->Endpoints, 
                  sizeof(UA_EndpointDescription), (UA_EncodeableObject_PfnDecode*) UA_EndpointDescription_Decode);

    if(status != STATUS_OK){
        UA_GetEndpointsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_GetEndpointsResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_GetEndpointsResponse_EncodeableType =
{
    "GetEndpointsResponse",
    OpcUaId_GetEndpointsResponse,
    OpcUaId_GetEndpointsResponse_Encoding_DefaultBinary,
    OpcUaId_GetEndpointsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_GetEndpointsResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_GetEndpointsResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_GetEndpointsResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_GetEndpointsResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_GetEndpointsResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_GetEndpointsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_RegisteredServer
/*============================================================================
 * UA_RegisteredServer_Initialize
 *===========================================================================*/
void UA_RegisteredServer_Initialize(UA_RegisteredServer* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ServerUri);
        String_Initialize(&a_pValue->ProductUri);
        UA_Initialize_Array(&a_pValue->NoOfServerNames, (void**) &a_pValue->ServerNames, 
                            sizeof(UA_LocalizedText), (UA_EncodeableObject_PfnInitialize*) LocalizedText_Initialize);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->ServerType);
        String_Initialize(&a_pValue->GatewayServerUri);
        UA_Initialize_Array(&a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
        String_Initialize(&a_pValue->SemaphoreFilePath);
        Boolean_Initialize(&a_pValue->IsOnline);
    }
}

/*============================================================================
 * UA_RegisteredServer_Clear
 *===========================================================================*/
void UA_RegisteredServer_Clear(UA_RegisteredServer* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ServerUri);
        String_Clear(&a_pValue->ProductUri);
        UA_Clear_Array(&a_pValue->NoOfServerNames, (void**) &a_pValue->ServerNames, 
                       sizeof(UA_LocalizedText), (UA_EncodeableObject_PfnClear*) LocalizedText_Clear);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->ServerType);
        String_Clear(&a_pValue->GatewayServerUri);
        UA_Clear_Array(&a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
        String_Clear(&a_pValue->SemaphoreFilePath);
        Boolean_Clear(&a_pValue->IsOnline);
    }
}

/*============================================================================
 * UA_RegisteredServer_Encode
 *===========================================================================*/
StatusCode UA_RegisteredServer_Encode(UA_RegisteredServer* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ServerUri, msgBuf);
    status &= String_Write(&a_pValue->ProductUri, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfServerNames, (void**) &a_pValue->ServerNames, 
                   sizeof(UA_LocalizedText), (UA_EncodeableObject_PfnEncode*) LocalizedText_Write);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ServerType);
    status &= String_Write(&a_pValue->GatewayServerUri, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);
    status &= String_Write(&a_pValue->SemaphoreFilePath, msgBuf);
    status &= Boolean_Write(&a_pValue->IsOnline, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RegisteredServer_Decode
 *===========================================================================*/
StatusCode UA_RegisteredServer_Decode(UA_RegisteredServer* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RegisteredServer_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ServerUri, msgBuf);
    status &= String_Read(&a_pValue->ProductUri, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfServerNames, (void**) &a_pValue->ServerNames, 
                  sizeof(UA_LocalizedText), (UA_EncodeableObject_PfnDecode*) LocalizedText_Read);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ServerType);
    status &= String_Read(&a_pValue->GatewayServerUri, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);
    status &= String_Read(&a_pValue->SemaphoreFilePath, msgBuf);
    status &= Boolean_Read(&a_pValue->IsOnline, msgBuf);

    if(status != STATUS_OK){
        UA_RegisteredServer_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RegisteredServer_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RegisteredServer_EncodeableType =
{
    "RegisteredServer",
    OpcUaId_RegisteredServer,
    OpcUaId_RegisteredServer_Encoding_DefaultBinary,
    OpcUaId_RegisteredServer_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RegisteredServer),
    (UA_EncodeableObject_PfnInitialize*)UA_RegisteredServer_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RegisteredServer_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RegisteredServer_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RegisteredServer_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RegisteredServer_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
#ifndef OPCUA_EXCLUDE_RegisterServerRequest
/*============================================================================
 * UA_RegisterServerRequest_Initialize
 *===========================================================================*/
void UA_RegisterServerRequest_Initialize(UA_RegisterServerRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_RegisteredServer_Initialize(&a_pValue->Server);
    }
}

/*============================================================================
 * UA_RegisterServerRequest_Clear
 *===========================================================================*/
void UA_RegisterServerRequest_Clear(UA_RegisterServerRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_RegisteredServer_Clear(&a_pValue->Server);
    }
}

/*============================================================================
 * UA_RegisterServerRequest_Encode
 *===========================================================================*/
StatusCode UA_RegisterServerRequest_Encode(UA_RegisterServerRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_RegisteredServer_Encode(&a_pValue->Server, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RegisterServerRequest_Decode
 *===========================================================================*/
StatusCode UA_RegisterServerRequest_Decode(UA_RegisterServerRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RegisterServerRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_RegisteredServer_Decode(&a_pValue->Server, msgBuf);

    if(status != STATUS_OK){
        UA_RegisterServerRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RegisterServerRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RegisterServerRequest_EncodeableType =
{
    "RegisterServerRequest",
    OpcUaId_RegisterServerRequest,
    OpcUaId_RegisterServerRequest_Encoding_DefaultBinary,
    OpcUaId_RegisterServerRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RegisterServerRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_RegisterServerRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RegisterServerRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RegisterServerRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RegisterServerRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RegisterServerRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServerResponse
/*============================================================================
 * UA_RegisterServerResponse_Initialize
 *===========================================================================*/
void UA_RegisterServerResponse_Initialize(UA_RegisterServerResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_RegisterServerResponse_Clear
 *===========================================================================*/
void UA_RegisterServerResponse_Clear(UA_RegisterServerResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_RegisterServerResponse_Encode
 *===========================================================================*/
StatusCode UA_RegisterServerResponse_Encode(UA_RegisterServerResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RegisterServerResponse_Decode
 *===========================================================================*/
StatusCode UA_RegisterServerResponse_Decode(UA_RegisterServerResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RegisterServerResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        UA_RegisterServerResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RegisterServerResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RegisterServerResponse_EncodeableType =
{
    "RegisterServerResponse",
    OpcUaId_RegisterServerResponse,
    OpcUaId_RegisterServerResponse_Encoding_DefaultBinary,
    OpcUaId_RegisterServerResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RegisterServerResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_RegisterServerResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RegisterServerResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RegisterServerResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RegisterServerResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RegisterServerResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_MdnsDiscoveryConfiguration
/*============================================================================
 * UA_MdnsDiscoveryConfiguration_Initialize
 *===========================================================================*/
void UA_MdnsDiscoveryConfiguration_Initialize(UA_MdnsDiscoveryConfiguration* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->MdnsServerName);
        UA_Initialize_Array(&a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * UA_MdnsDiscoveryConfiguration_Clear
 *===========================================================================*/
void UA_MdnsDiscoveryConfiguration_Clear(UA_MdnsDiscoveryConfiguration* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->MdnsServerName);
        UA_Clear_Array(&a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * UA_MdnsDiscoveryConfiguration_Encode
 *===========================================================================*/
StatusCode UA_MdnsDiscoveryConfiguration_Encode(UA_MdnsDiscoveryConfiguration* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->MdnsServerName, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_MdnsDiscoveryConfiguration_Decode
 *===========================================================================*/
StatusCode UA_MdnsDiscoveryConfiguration_Decode(UA_MdnsDiscoveryConfiguration* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_MdnsDiscoveryConfiguration_Initialize(a_pValue);

    status &= String_Read(&a_pValue->MdnsServerName, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        UA_MdnsDiscoveryConfiguration_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_MdnsDiscoveryConfiguration_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_MdnsDiscoveryConfiguration_EncodeableType =
{
    "MdnsDiscoveryConfiguration",
    OpcUaId_MdnsDiscoveryConfiguration,
    OpcUaId_MdnsDiscoveryConfiguration_Encoding_DefaultBinary,
    OpcUaId_MdnsDiscoveryConfiguration_Encoding_DefaultXml,
    NULL,
    sizeof(UA_MdnsDiscoveryConfiguration),
    (UA_EncodeableObject_PfnInitialize*)UA_MdnsDiscoveryConfiguration_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_MdnsDiscoveryConfiguration_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_MdnsDiscoveryConfiguration_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_MdnsDiscoveryConfiguration_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_MdnsDiscoveryConfiguration_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
#ifndef OPCUA_EXCLUDE_RegisterServer2Request
/*============================================================================
 * UA_RegisterServer2Request_Initialize
 *===========================================================================*/
void UA_RegisterServer2Request_Initialize(UA_RegisterServer2Request* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_RegisteredServer_Initialize(&a_pValue->Server);
        UA_Initialize_Array(&a_pValue->NoOfDiscoveryConfiguration, (void**) &a_pValue->DiscoveryConfiguration, 
                            sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnInitialize*) ExtensionObject_Initialize);
    }
}

/*============================================================================
 * UA_RegisterServer2Request_Clear
 *===========================================================================*/
void UA_RegisterServer2Request_Clear(UA_RegisterServer2Request* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_RegisteredServer_Clear(&a_pValue->Server);
        UA_Clear_Array(&a_pValue->NoOfDiscoveryConfiguration, (void**) &a_pValue->DiscoveryConfiguration, 
                       sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnClear*) ExtensionObject_Clear);
    }
}

/*============================================================================
 * UA_RegisterServer2Request_Encode
 *===========================================================================*/
StatusCode UA_RegisterServer2Request_Encode(UA_RegisterServer2Request* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_RegisteredServer_Encode(&a_pValue->Server, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiscoveryConfiguration, (void**) &a_pValue->DiscoveryConfiguration, 
                   sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnEncode*) ExtensionObject_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RegisterServer2Request_Decode
 *===========================================================================*/
StatusCode UA_RegisterServer2Request_Decode(UA_RegisterServer2Request* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RegisterServer2Request_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_RegisteredServer_Decode(&a_pValue->Server, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiscoveryConfiguration, (void**) &a_pValue->DiscoveryConfiguration, 
                  sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnDecode*) ExtensionObject_Read);

    if(status != STATUS_OK){
        UA_RegisterServer2Request_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RegisterServer2Request_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RegisterServer2Request_EncodeableType =
{
    "RegisterServer2Request",
    OpcUaId_RegisterServer2Request,
    OpcUaId_RegisterServer2Request_Encoding_DefaultBinary,
    OpcUaId_RegisterServer2Request_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RegisterServer2Request),
    (UA_EncodeableObject_PfnInitialize*)UA_RegisterServer2Request_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RegisterServer2Request_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RegisterServer2Request_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RegisterServer2Request_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RegisterServer2Request_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2Response
/*============================================================================
 * UA_RegisterServer2Response_Initialize
 *===========================================================================*/
void UA_RegisterServer2Response_Initialize(UA_RegisterServer2Response* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfConfigurationResults, (void**) &a_pValue->ConfigurationResults, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_RegisterServer2Response_Clear
 *===========================================================================*/
void UA_RegisterServer2Response_Clear(UA_RegisterServer2Response* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfConfigurationResults, (void**) &a_pValue->ConfigurationResults, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_RegisterServer2Response_Encode
 *===========================================================================*/
StatusCode UA_RegisterServer2Response_Encode(UA_RegisterServer2Response* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfConfigurationResults, (void**) &a_pValue->ConfigurationResults, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RegisterServer2Response_Decode
 *===========================================================================*/
StatusCode UA_RegisterServer2Response_Decode(UA_RegisterServer2Response* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RegisterServer2Response_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfConfigurationResults, (void**) &a_pValue->ConfigurationResults, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_RegisterServer2Response_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RegisterServer2Response_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RegisterServer2Response_EncodeableType =
{
    "RegisterServer2Response",
    OpcUaId_RegisterServer2Response,
    OpcUaId_RegisterServer2Response_Encoding_DefaultBinary,
    OpcUaId_RegisterServer2Response_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RegisterServer2Response),
    (UA_EncodeableObject_PfnInitialize*)UA_RegisterServer2Response_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RegisterServer2Response_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RegisterServer2Response_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RegisterServer2Response_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RegisterServer2Response_Decode
};
#endif
#endif


#ifndef OPCUA_EXCLUDE_ChannelSecurityToken
/*============================================================================
 * UA_ChannelSecurityToken_Initialize
 *===========================================================================*/
void UA_ChannelSecurityToken_Initialize(UA_ChannelSecurityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->ChannelId);
        UInt32_Initialize(&a_pValue->TokenId);
        DateTime_Initialize(&a_pValue->CreatedAt);
        UInt32_Initialize(&a_pValue->RevisedLifetime);
    }
}

/*============================================================================
 * UA_ChannelSecurityToken_Clear
 *===========================================================================*/
void UA_ChannelSecurityToken_Clear(UA_ChannelSecurityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->ChannelId);
        UInt32_Clear(&a_pValue->TokenId);
        DateTime_Clear(&a_pValue->CreatedAt);
        UInt32_Clear(&a_pValue->RevisedLifetime);
    }
}

/*============================================================================
 * UA_ChannelSecurityToken_Encode
 *===========================================================================*/
StatusCode UA_ChannelSecurityToken_Encode(UA_ChannelSecurityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->ChannelId, msgBuf);
    status &= UInt32_Write(&a_pValue->TokenId, msgBuf);
    status &= DateTime_Write(&a_pValue->CreatedAt, msgBuf);
    status &= UInt32_Write(&a_pValue->RevisedLifetime, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ChannelSecurityToken_Decode
 *===========================================================================*/
StatusCode UA_ChannelSecurityToken_Decode(UA_ChannelSecurityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ChannelSecurityToken_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->ChannelId, msgBuf);
    status &= UInt32_Read(&a_pValue->TokenId, msgBuf);
    status &= DateTime_Read(&a_pValue->CreatedAt, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedLifetime, msgBuf);

    if(status != STATUS_OK){
        UA_ChannelSecurityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ChannelSecurityToken_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ChannelSecurityToken_EncodeableType =
{
    "ChannelSecurityToken",
    OpcUaId_ChannelSecurityToken,
    OpcUaId_ChannelSecurityToken_Encoding_DefaultBinary,
    OpcUaId_ChannelSecurityToken_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ChannelSecurityToken),
    (UA_EncodeableObject_PfnInitialize*)UA_ChannelSecurityToken_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ChannelSecurityToken_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ChannelSecurityToken_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ChannelSecurityToken_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ChannelSecurityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_OpenSecureChannel
#ifndef OPCUA_EXCLUDE_OpenSecureChannelRequest
/*============================================================================
 * UA_OpenSecureChannelRequest_Initialize
 *===========================================================================*/
void UA_OpenSecureChannelRequest_Initialize(UA_OpenSecureChannelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->ClientProtocolVersion);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->RequestType);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        ByteString_Initialize(&a_pValue->ClientNonce);
        UInt32_Initialize(&a_pValue->RequestedLifetime);
    }
}

/*============================================================================
 * UA_OpenSecureChannelRequest_Clear
 *===========================================================================*/
void UA_OpenSecureChannelRequest_Clear(UA_OpenSecureChannelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->ClientProtocolVersion);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->RequestType);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        ByteString_Clear(&a_pValue->ClientNonce);
        UInt32_Clear(&a_pValue->RequestedLifetime);
    }
}

/*============================================================================
 * UA_OpenSecureChannelRequest_Encode
 *===========================================================================*/
StatusCode UA_OpenSecureChannelRequest_Encode(UA_OpenSecureChannelRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->ClientProtocolVersion, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->RequestType);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= ByteString_Write(&a_pValue->ClientNonce, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestedLifetime, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_OpenSecureChannelRequest_Decode
 *===========================================================================*/
StatusCode UA_OpenSecureChannelRequest_Decode(UA_OpenSecureChannelRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_OpenSecureChannelRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->ClientProtocolVersion, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->RequestType);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= ByteString_Read(&a_pValue->ClientNonce, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedLifetime, msgBuf);

    if(status != STATUS_OK){
        UA_OpenSecureChannelRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_OpenSecureChannelRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_OpenSecureChannelRequest_EncodeableType =
{
    "OpenSecureChannelRequest",
    OpcUaId_OpenSecureChannelRequest,
    OpcUaId_OpenSecureChannelRequest_Encoding_DefaultBinary,
    OpcUaId_OpenSecureChannelRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_OpenSecureChannelRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_OpenSecureChannelRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_OpenSecureChannelRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_OpenSecureChannelRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_OpenSecureChannelRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_OpenSecureChannelRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_OpenSecureChannelResponse
/*============================================================================
 * UA_OpenSecureChannelResponse_Initialize
 *===========================================================================*/
void UA_OpenSecureChannelResponse_Initialize(UA_OpenSecureChannelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UInt32_Initialize(&a_pValue->ServerProtocolVersion);
        UA_ChannelSecurityToken_Initialize(&a_pValue->SecurityToken);
        ByteString_Initialize(&a_pValue->ServerNonce);
    }
}

/*============================================================================
 * UA_OpenSecureChannelResponse_Clear
 *===========================================================================*/
void UA_OpenSecureChannelResponse_Clear(UA_OpenSecureChannelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UInt32_Clear(&a_pValue->ServerProtocolVersion);
        UA_ChannelSecurityToken_Clear(&a_pValue->SecurityToken);
        ByteString_Clear(&a_pValue->ServerNonce);
    }
}

/*============================================================================
 * UA_OpenSecureChannelResponse_Encode
 *===========================================================================*/
StatusCode UA_OpenSecureChannelResponse_Encode(UA_OpenSecureChannelResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->ServerProtocolVersion, msgBuf);
    status &= UA_ChannelSecurityToken_Encode(&a_pValue->SecurityToken, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerNonce, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_OpenSecureChannelResponse_Decode
 *===========================================================================*/
StatusCode UA_OpenSecureChannelResponse_Decode(UA_OpenSecureChannelResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_OpenSecureChannelResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->ServerProtocolVersion, msgBuf);
    status &= UA_ChannelSecurityToken_Decode(&a_pValue->SecurityToken, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerNonce, msgBuf);

    if(status != STATUS_OK){
        UA_OpenSecureChannelResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_OpenSecureChannelResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_OpenSecureChannelResponse_EncodeableType =
{
    "OpenSecureChannelResponse",
    OpcUaId_OpenSecureChannelResponse,
    OpcUaId_OpenSecureChannelResponse_Encoding_DefaultBinary,
    OpcUaId_OpenSecureChannelResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_OpenSecureChannelResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_OpenSecureChannelResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_OpenSecureChannelResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_OpenSecureChannelResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_OpenSecureChannelResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_OpenSecureChannelResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_CloseSecureChannel
#ifndef OPCUA_EXCLUDE_CloseSecureChannelRequest
/*============================================================================
 * UA_CloseSecureChannelRequest_Initialize
 *===========================================================================*/
void UA_CloseSecureChannelRequest_Initialize(UA_CloseSecureChannelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
    }
}

/*============================================================================
 * UA_CloseSecureChannelRequest_Clear
 *===========================================================================*/
void UA_CloseSecureChannelRequest_Clear(UA_CloseSecureChannelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
    }
}

/*============================================================================
 * UA_CloseSecureChannelRequest_Encode
 *===========================================================================*/
StatusCode UA_CloseSecureChannelRequest_Encode(UA_CloseSecureChannelRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CloseSecureChannelRequest_Decode
 *===========================================================================*/
StatusCode UA_CloseSecureChannelRequest_Decode(UA_CloseSecureChannelRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CloseSecureChannelRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);

    if(status != STATUS_OK){
        UA_CloseSecureChannelRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CloseSecureChannelRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CloseSecureChannelRequest_EncodeableType =
{
    "CloseSecureChannelRequest",
    OpcUaId_CloseSecureChannelRequest,
    OpcUaId_CloseSecureChannelRequest_Encoding_DefaultBinary,
    OpcUaId_CloseSecureChannelRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CloseSecureChannelRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_CloseSecureChannelRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CloseSecureChannelRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CloseSecureChannelRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CloseSecureChannelRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CloseSecureChannelRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CloseSecureChannelResponse
/*============================================================================
 * UA_CloseSecureChannelResponse_Initialize
 *===========================================================================*/
void UA_CloseSecureChannelResponse_Initialize(UA_CloseSecureChannelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_CloseSecureChannelResponse_Clear
 *===========================================================================*/
void UA_CloseSecureChannelResponse_Clear(UA_CloseSecureChannelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_CloseSecureChannelResponse_Encode
 *===========================================================================*/
StatusCode UA_CloseSecureChannelResponse_Encode(UA_CloseSecureChannelResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CloseSecureChannelResponse_Decode
 *===========================================================================*/
StatusCode UA_CloseSecureChannelResponse_Decode(UA_CloseSecureChannelResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CloseSecureChannelResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        UA_CloseSecureChannelResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CloseSecureChannelResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CloseSecureChannelResponse_EncodeableType =
{
    "CloseSecureChannelResponse",
    OpcUaId_CloseSecureChannelResponse,
    OpcUaId_CloseSecureChannelResponse_Encoding_DefaultBinary,
    OpcUaId_CloseSecureChannelResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CloseSecureChannelResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_CloseSecureChannelResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CloseSecureChannelResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CloseSecureChannelResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CloseSecureChannelResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CloseSecureChannelResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_SignedSoftwareCertificate
/*============================================================================
 * UA_SignedSoftwareCertificate_Initialize
 *===========================================================================*/
void UA_SignedSoftwareCertificate_Initialize(UA_SignedSoftwareCertificate* a_pValue)
{
    if (a_pValue != NULL)
    {
        ByteString_Initialize(&a_pValue->CertificateData);
        ByteString_Initialize(&a_pValue->Signature);
    }
}

/*============================================================================
 * UA_SignedSoftwareCertificate_Clear
 *===========================================================================*/
void UA_SignedSoftwareCertificate_Clear(UA_SignedSoftwareCertificate* a_pValue)
{
    if (a_pValue != NULL)
    {
        ByteString_Clear(&a_pValue->CertificateData);
        ByteString_Clear(&a_pValue->Signature);
    }
}

/*============================================================================
 * UA_SignedSoftwareCertificate_Encode
 *===========================================================================*/
StatusCode UA_SignedSoftwareCertificate_Encode(UA_SignedSoftwareCertificate* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= ByteString_Write(&a_pValue->CertificateData, msgBuf);
    status &= ByteString_Write(&a_pValue->Signature, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SignedSoftwareCertificate_Decode
 *===========================================================================*/
StatusCode UA_SignedSoftwareCertificate_Decode(UA_SignedSoftwareCertificate* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SignedSoftwareCertificate_Initialize(a_pValue);

    status &= ByteString_Read(&a_pValue->CertificateData, msgBuf);
    status &= ByteString_Read(&a_pValue->Signature, msgBuf);

    if(status != STATUS_OK){
        UA_SignedSoftwareCertificate_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SignedSoftwareCertificate_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SignedSoftwareCertificate_EncodeableType =
{
    "SignedSoftwareCertificate",
    OpcUaId_SignedSoftwareCertificate,
    OpcUaId_SignedSoftwareCertificate_Encoding_DefaultBinary,
    OpcUaId_SignedSoftwareCertificate_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SignedSoftwareCertificate),
    (UA_EncodeableObject_PfnInitialize*)UA_SignedSoftwareCertificate_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SignedSoftwareCertificate_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SignedSoftwareCertificate_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SignedSoftwareCertificate_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SignedSoftwareCertificate_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SignatureData
/*============================================================================
 * UA_SignatureData_Initialize
 *===========================================================================*/
void UA_SignatureData_Initialize(UA_SignatureData* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->Algorithm);
        ByteString_Initialize(&a_pValue->Signature);
    }
}

/*============================================================================
 * UA_SignatureData_Clear
 *===========================================================================*/
void UA_SignatureData_Clear(UA_SignatureData* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->Algorithm);
        ByteString_Clear(&a_pValue->Signature);
    }
}

/*============================================================================
 * UA_SignatureData_Encode
 *===========================================================================*/
StatusCode UA_SignatureData_Encode(UA_SignatureData* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->Algorithm, msgBuf);
    status &= ByteString_Write(&a_pValue->Signature, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SignatureData_Decode
 *===========================================================================*/
StatusCode UA_SignatureData_Decode(UA_SignatureData* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SignatureData_Initialize(a_pValue);

    status &= String_Read(&a_pValue->Algorithm, msgBuf);
    status &= ByteString_Read(&a_pValue->Signature, msgBuf);

    if(status != STATUS_OK){
        UA_SignatureData_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SignatureData_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SignatureData_EncodeableType =
{
    "SignatureData",
    OpcUaId_SignatureData,
    OpcUaId_SignatureData_Encoding_DefaultBinary,
    OpcUaId_SignatureData_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SignatureData),
    (UA_EncodeableObject_PfnInitialize*)UA_SignatureData_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SignatureData_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SignatureData_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SignatureData_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SignatureData_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
#ifndef OPCUA_EXCLUDE_CreateSessionRequest
/*============================================================================
 * UA_CreateSessionRequest_Initialize
 *===========================================================================*/
void UA_CreateSessionRequest_Initialize(UA_CreateSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_ApplicationDescription_Initialize(&a_pValue->ClientDescription);
        String_Initialize(&a_pValue->ServerUri);
        String_Initialize(&a_pValue->EndpointUrl);
        String_Initialize(&a_pValue->SessionName);
        ByteString_Initialize(&a_pValue->ClientNonce);
        ByteString_Initialize(&a_pValue->ClientCertificate);
        Double_Initialize(&a_pValue->RequestedSessionTimeout);
        UInt32_Initialize(&a_pValue->MaxResponseMessageSize);
    }
}

/*============================================================================
 * UA_CreateSessionRequest_Clear
 *===========================================================================*/
void UA_CreateSessionRequest_Clear(UA_CreateSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_ApplicationDescription_Clear(&a_pValue->ClientDescription);
        String_Clear(&a_pValue->ServerUri);
        String_Clear(&a_pValue->EndpointUrl);
        String_Clear(&a_pValue->SessionName);
        ByteString_Clear(&a_pValue->ClientNonce);
        ByteString_Clear(&a_pValue->ClientCertificate);
        Double_Clear(&a_pValue->RequestedSessionTimeout);
        UInt32_Clear(&a_pValue->MaxResponseMessageSize);
    }
}

/*============================================================================
 * UA_CreateSessionRequest_Encode
 *===========================================================================*/
StatusCode UA_CreateSessionRequest_Encode(UA_CreateSessionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_ApplicationDescription_Encode(&a_pValue->ClientDescription, msgBuf);
    status &= String_Write(&a_pValue->ServerUri, msgBuf);
    status &= String_Write(&a_pValue->EndpointUrl, msgBuf);
    status &= String_Write(&a_pValue->SessionName, msgBuf);
    status &= ByteString_Write(&a_pValue->ClientNonce, msgBuf);
    status &= ByteString_Write(&a_pValue->ClientCertificate, msgBuf);
    status &= Double_Write(&a_pValue->RequestedSessionTimeout, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxResponseMessageSize, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CreateSessionRequest_Decode
 *===========================================================================*/
StatusCode UA_CreateSessionRequest_Decode(UA_CreateSessionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CreateSessionRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_ApplicationDescription_Decode(&a_pValue->ClientDescription, msgBuf);
    status &= String_Read(&a_pValue->ServerUri, msgBuf);
    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= String_Read(&a_pValue->SessionName, msgBuf);
    status &= ByteString_Read(&a_pValue->ClientNonce, msgBuf);
    status &= ByteString_Read(&a_pValue->ClientCertificate, msgBuf);
    status &= Double_Read(&a_pValue->RequestedSessionTimeout, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxResponseMessageSize, msgBuf);

    if(status != STATUS_OK){
        UA_CreateSessionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CreateSessionRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CreateSessionRequest_EncodeableType =
{
    "CreateSessionRequest",
    OpcUaId_CreateSessionRequest,
    OpcUaId_CreateSessionRequest_Encoding_DefaultBinary,
    OpcUaId_CreateSessionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CreateSessionRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_CreateSessionRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CreateSessionRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CreateSessionRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CreateSessionRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CreateSessionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateSessionResponse
/*============================================================================
 * UA_CreateSessionResponse_Initialize
 *===========================================================================*/
void UA_CreateSessionResponse_Initialize(UA_CreateSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        NodeId_Initialize(&a_pValue->SessionId);
        NodeId_Initialize(&a_pValue->AuthenticationToken);
        Double_Initialize(&a_pValue->RevisedSessionTimeout);
        ByteString_Initialize(&a_pValue->ServerNonce);
        ByteString_Initialize(&a_pValue->ServerCertificate);
        UA_Initialize_Array(&a_pValue->NoOfServerEndpoints, (void**) &a_pValue->ServerEndpoints, 
                            sizeof(UA_EndpointDescription), (UA_EncodeableObject_PfnInitialize*) UA_EndpointDescription_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfServerSoftwareCertificates, (void**) &a_pValue->ServerSoftwareCertificates, 
                            sizeof(UA_SignedSoftwareCertificate), (UA_EncodeableObject_PfnInitialize*) UA_SignedSoftwareCertificate_Initialize);
        UA_SignatureData_Initialize(&a_pValue->ServerSignature);
        UInt32_Initialize(&a_pValue->MaxRequestMessageSize);
    }
}

/*============================================================================
 * UA_CreateSessionResponse_Clear
 *===========================================================================*/
void UA_CreateSessionResponse_Clear(UA_CreateSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        NodeId_Clear(&a_pValue->SessionId);
        NodeId_Clear(&a_pValue->AuthenticationToken);
        Double_Clear(&a_pValue->RevisedSessionTimeout);
        ByteString_Clear(&a_pValue->ServerNonce);
        ByteString_Clear(&a_pValue->ServerCertificate);
        UA_Clear_Array(&a_pValue->NoOfServerEndpoints, (void**) &a_pValue->ServerEndpoints, 
                       sizeof(UA_EndpointDescription), (UA_EncodeableObject_PfnClear*) UA_EndpointDescription_Clear);
        UA_Clear_Array(&a_pValue->NoOfServerSoftwareCertificates, (void**) &a_pValue->ServerSoftwareCertificates, 
                       sizeof(UA_SignedSoftwareCertificate), (UA_EncodeableObject_PfnClear*) UA_SignedSoftwareCertificate_Clear);
        UA_SignatureData_Clear(&a_pValue->ServerSignature);
        UInt32_Clear(&a_pValue->MaxRequestMessageSize);
    }
}

/*============================================================================
 * UA_CreateSessionResponse_Encode
 *===========================================================================*/
StatusCode UA_CreateSessionResponse_Encode(UA_CreateSessionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= NodeId_Write(&a_pValue->SessionId, msgBuf);
    status &= NodeId_Write(&a_pValue->AuthenticationToken, msgBuf);
    status &= Double_Write(&a_pValue->RevisedSessionTimeout, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerNonce, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerCertificate, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfServerEndpoints, (void**) &a_pValue->ServerEndpoints, 
                   sizeof(UA_EndpointDescription), (UA_EncodeableObject_PfnEncode*) UA_EndpointDescription_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfServerSoftwareCertificates, (void**) &a_pValue->ServerSoftwareCertificates, 
                   sizeof(UA_SignedSoftwareCertificate), (UA_EncodeableObject_PfnEncode*) UA_SignedSoftwareCertificate_Encode);
    status &= UA_SignatureData_Encode(&a_pValue->ServerSignature, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxRequestMessageSize, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CreateSessionResponse_Decode
 *===========================================================================*/
StatusCode UA_CreateSessionResponse_Decode(UA_CreateSessionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CreateSessionResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= NodeId_Read(&a_pValue->SessionId, msgBuf);
    status &= NodeId_Read(&a_pValue->AuthenticationToken, msgBuf);
    status &= Double_Read(&a_pValue->RevisedSessionTimeout, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerNonce, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerCertificate, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfServerEndpoints, (void**) &a_pValue->ServerEndpoints, 
                  sizeof(UA_EndpointDescription), (UA_EncodeableObject_PfnDecode*) UA_EndpointDescription_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfServerSoftwareCertificates, (void**) &a_pValue->ServerSoftwareCertificates, 
                  sizeof(UA_SignedSoftwareCertificate), (UA_EncodeableObject_PfnDecode*) UA_SignedSoftwareCertificate_Decode);
    status &= UA_SignatureData_Decode(&a_pValue->ServerSignature, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxRequestMessageSize, msgBuf);

    if(status != STATUS_OK){
        UA_CreateSessionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CreateSessionResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CreateSessionResponse_EncodeableType =
{
    "CreateSessionResponse",
    OpcUaId_CreateSessionResponse,
    OpcUaId_CreateSessionResponse_Encoding_DefaultBinary,
    OpcUaId_CreateSessionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CreateSessionResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_CreateSessionResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CreateSessionResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CreateSessionResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CreateSessionResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CreateSessionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_UserIdentityToken
/*============================================================================
 * UA_UserIdentityToken_Initialize
 *===========================================================================*/
void UA_UserIdentityToken_Initialize(UA_UserIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
    }
}

/*============================================================================
 * UA_UserIdentityToken_Clear
 *===========================================================================*/
void UA_UserIdentityToken_Clear(UA_UserIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
    }
}

/*============================================================================
 * UA_UserIdentityToken_Encode
 *===========================================================================*/
StatusCode UA_UserIdentityToken_Encode(UA_UserIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->PolicyId, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_UserIdentityToken_Decode
 *===========================================================================*/
StatusCode UA_UserIdentityToken_Decode(UA_UserIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_UserIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);

    if(status != STATUS_OK){
        UA_UserIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_UserIdentityToken_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_UserIdentityToken_EncodeableType =
{
    "UserIdentityToken",
    OpcUaId_UserIdentityToken,
    OpcUaId_UserIdentityToken_Encoding_DefaultBinary,
    OpcUaId_UserIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(UA_UserIdentityToken),
    (UA_EncodeableObject_PfnInitialize*)UA_UserIdentityToken_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_UserIdentityToken_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_UserIdentityToken_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_UserIdentityToken_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_UserIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AnonymousIdentityToken
/*============================================================================
 * UA_AnonymousIdentityToken_Initialize
 *===========================================================================*/
void UA_AnonymousIdentityToken_Initialize(UA_AnonymousIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
    }
}

/*============================================================================
 * UA_AnonymousIdentityToken_Clear
 *===========================================================================*/
void UA_AnonymousIdentityToken_Clear(UA_AnonymousIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
    }
}

/*============================================================================
 * UA_AnonymousIdentityToken_Encode
 *===========================================================================*/
StatusCode UA_AnonymousIdentityToken_Encode(UA_AnonymousIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->PolicyId, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AnonymousIdentityToken_Decode
 *===========================================================================*/
StatusCode UA_AnonymousIdentityToken_Decode(UA_AnonymousIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AnonymousIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);

    if(status != STATUS_OK){
        UA_AnonymousIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AnonymousIdentityToken_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AnonymousIdentityToken_EncodeableType =
{
    "AnonymousIdentityToken",
    OpcUaId_AnonymousIdentityToken,
    OpcUaId_AnonymousIdentityToken_Encoding_DefaultBinary,
    OpcUaId_AnonymousIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AnonymousIdentityToken),
    (UA_EncodeableObject_PfnInitialize*)UA_AnonymousIdentityToken_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AnonymousIdentityToken_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AnonymousIdentityToken_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AnonymousIdentityToken_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AnonymousIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_UserNameIdentityToken
/*============================================================================
 * UA_UserNameIdentityToken_Initialize
 *===========================================================================*/
void UA_UserNameIdentityToken_Initialize(UA_UserNameIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
        String_Initialize(&a_pValue->UserName);
        ByteString_Initialize(&a_pValue->Password);
        String_Initialize(&a_pValue->EncryptionAlgorithm);
    }
}

/*============================================================================
 * UA_UserNameIdentityToken_Clear
 *===========================================================================*/
void UA_UserNameIdentityToken_Clear(UA_UserNameIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
        String_Clear(&a_pValue->UserName);
        ByteString_Clear(&a_pValue->Password);
        String_Clear(&a_pValue->EncryptionAlgorithm);
    }
}

/*============================================================================
 * UA_UserNameIdentityToken_Encode
 *===========================================================================*/
StatusCode UA_UserNameIdentityToken_Encode(UA_UserNameIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->PolicyId, msgBuf);
    status &= String_Write(&a_pValue->UserName, msgBuf);
    status &= ByteString_Write(&a_pValue->Password, msgBuf);
    status &= String_Write(&a_pValue->EncryptionAlgorithm, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_UserNameIdentityToken_Decode
 *===========================================================================*/
StatusCode UA_UserNameIdentityToken_Decode(UA_UserNameIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_UserNameIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= String_Read(&a_pValue->UserName, msgBuf);
    status &= ByteString_Read(&a_pValue->Password, msgBuf);
    status &= String_Read(&a_pValue->EncryptionAlgorithm, msgBuf);

    if(status != STATUS_OK){
        UA_UserNameIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_UserNameIdentityToken_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_UserNameIdentityToken_EncodeableType =
{
    "UserNameIdentityToken",
    OpcUaId_UserNameIdentityToken,
    OpcUaId_UserNameIdentityToken_Encoding_DefaultBinary,
    OpcUaId_UserNameIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(UA_UserNameIdentityToken),
    (UA_EncodeableObject_PfnInitialize*)UA_UserNameIdentityToken_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_UserNameIdentityToken_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_UserNameIdentityToken_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_UserNameIdentityToken_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_UserNameIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_X509IdentityToken
/*============================================================================
 * UA_X509IdentityToken_Initialize
 *===========================================================================*/
void UA_X509IdentityToken_Initialize(UA_X509IdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
        ByteString_Initialize(&a_pValue->CertificateData);
    }
}

/*============================================================================
 * UA_X509IdentityToken_Clear
 *===========================================================================*/
void UA_X509IdentityToken_Clear(UA_X509IdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
        ByteString_Clear(&a_pValue->CertificateData);
    }
}

/*============================================================================
 * UA_X509IdentityToken_Encode
 *===========================================================================*/
StatusCode UA_X509IdentityToken_Encode(UA_X509IdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->PolicyId, msgBuf);
    status &= ByteString_Write(&a_pValue->CertificateData, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_X509IdentityToken_Decode
 *===========================================================================*/
StatusCode UA_X509IdentityToken_Decode(UA_X509IdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_X509IdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= ByteString_Read(&a_pValue->CertificateData, msgBuf);

    if(status != STATUS_OK){
        UA_X509IdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_X509IdentityToken_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_X509IdentityToken_EncodeableType =
{
    "X509IdentityToken",
    OpcUaId_X509IdentityToken,
    OpcUaId_X509IdentityToken_Encoding_DefaultBinary,
    OpcUaId_X509IdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(UA_X509IdentityToken),
    (UA_EncodeableObject_PfnInitialize*)UA_X509IdentityToken_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_X509IdentityToken_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_X509IdentityToken_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_X509IdentityToken_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_X509IdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_KerberosIdentityToken
/*============================================================================
 * UA_KerberosIdentityToken_Initialize
 *===========================================================================*/
void UA_KerberosIdentityToken_Initialize(UA_KerberosIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
        ByteString_Initialize(&a_pValue->TicketData);
    }
}

/*============================================================================
 * UA_KerberosIdentityToken_Clear
 *===========================================================================*/
void UA_KerberosIdentityToken_Clear(UA_KerberosIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
        ByteString_Clear(&a_pValue->TicketData);
    }
}

/*============================================================================
 * UA_KerberosIdentityToken_Encode
 *===========================================================================*/
StatusCode UA_KerberosIdentityToken_Encode(UA_KerberosIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->PolicyId, msgBuf);
    status &= ByteString_Write(&a_pValue->TicketData, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_KerberosIdentityToken_Decode
 *===========================================================================*/
StatusCode UA_KerberosIdentityToken_Decode(UA_KerberosIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_KerberosIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= ByteString_Read(&a_pValue->TicketData, msgBuf);

    if(status != STATUS_OK){
        UA_KerberosIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_KerberosIdentityToken_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_KerberosIdentityToken_EncodeableType =
{
    "KerberosIdentityToken",
    OpcUaId_KerberosIdentityToken,
    OpcUaId_KerberosIdentityToken_Encoding_DefaultBinary,
    OpcUaId_KerberosIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(UA_KerberosIdentityToken),
    (UA_EncodeableObject_PfnInitialize*)UA_KerberosIdentityToken_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_KerberosIdentityToken_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_KerberosIdentityToken_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_KerberosIdentityToken_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_KerberosIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_IssuedIdentityToken
/*============================================================================
 * UA_IssuedIdentityToken_Initialize
 *===========================================================================*/
void UA_IssuedIdentityToken_Initialize(UA_IssuedIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
        ByteString_Initialize(&a_pValue->TokenData);
        String_Initialize(&a_pValue->EncryptionAlgorithm);
    }
}

/*============================================================================
 * UA_IssuedIdentityToken_Clear
 *===========================================================================*/
void UA_IssuedIdentityToken_Clear(UA_IssuedIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
        ByteString_Clear(&a_pValue->TokenData);
        String_Clear(&a_pValue->EncryptionAlgorithm);
    }
}

/*============================================================================
 * UA_IssuedIdentityToken_Encode
 *===========================================================================*/
StatusCode UA_IssuedIdentityToken_Encode(UA_IssuedIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->PolicyId, msgBuf);
    status &= ByteString_Write(&a_pValue->TokenData, msgBuf);
    status &= String_Write(&a_pValue->EncryptionAlgorithm, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_IssuedIdentityToken_Decode
 *===========================================================================*/
StatusCode UA_IssuedIdentityToken_Decode(UA_IssuedIdentityToken* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_IssuedIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= ByteString_Read(&a_pValue->TokenData, msgBuf);
    status &= String_Read(&a_pValue->EncryptionAlgorithm, msgBuf);

    if(status != STATUS_OK){
        UA_IssuedIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_IssuedIdentityToken_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_IssuedIdentityToken_EncodeableType =
{
    "IssuedIdentityToken",
    OpcUaId_IssuedIdentityToken,
    OpcUaId_IssuedIdentityToken_Encoding_DefaultBinary,
    OpcUaId_IssuedIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(UA_IssuedIdentityToken),
    (UA_EncodeableObject_PfnInitialize*)UA_IssuedIdentityToken_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_IssuedIdentityToken_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_IssuedIdentityToken_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_IssuedIdentityToken_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_IssuedIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
#ifndef OPCUA_EXCLUDE_ActivateSessionRequest
/*============================================================================
 * UA_ActivateSessionRequest_Initialize
 *===========================================================================*/
void UA_ActivateSessionRequest_Initialize(UA_ActivateSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_SignatureData_Initialize(&a_pValue->ClientSignature);
        UA_Initialize_Array(&a_pValue->NoOfClientSoftwareCertificates, (void**) &a_pValue->ClientSoftwareCertificates, 
                            sizeof(UA_SignedSoftwareCertificate), (UA_EncodeableObject_PfnInitialize*) UA_SignedSoftwareCertificate_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
        ExtensionObject_Initialize(&a_pValue->UserIdentityToken);
        UA_SignatureData_Initialize(&a_pValue->UserTokenSignature);
    }
}

/*============================================================================
 * UA_ActivateSessionRequest_Clear
 *===========================================================================*/
void UA_ActivateSessionRequest_Clear(UA_ActivateSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_SignatureData_Clear(&a_pValue->ClientSignature);
        UA_Clear_Array(&a_pValue->NoOfClientSoftwareCertificates, (void**) &a_pValue->ClientSoftwareCertificates, 
                       sizeof(UA_SignedSoftwareCertificate), (UA_EncodeableObject_PfnClear*) UA_SignedSoftwareCertificate_Clear);
        UA_Clear_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
        ExtensionObject_Clear(&a_pValue->UserIdentityToken);
        UA_SignatureData_Clear(&a_pValue->UserTokenSignature);
    }
}

/*============================================================================
 * UA_ActivateSessionRequest_Encode
 *===========================================================================*/
StatusCode UA_ActivateSessionRequest_Encode(UA_ActivateSessionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_SignatureData_Encode(&a_pValue->ClientSignature, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfClientSoftwareCertificates, (void**) &a_pValue->ClientSoftwareCertificates, 
                   sizeof(UA_SignedSoftwareCertificate), (UA_EncodeableObject_PfnEncode*) UA_SignedSoftwareCertificate_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);
    status &= ExtensionObject_Write(&a_pValue->UserIdentityToken, msgBuf);
    status &= UA_SignatureData_Encode(&a_pValue->UserTokenSignature, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ActivateSessionRequest_Decode
 *===========================================================================*/
StatusCode UA_ActivateSessionRequest_Decode(UA_ActivateSessionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ActivateSessionRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_SignatureData_Decode(&a_pValue->ClientSignature, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfClientSoftwareCertificates, (void**) &a_pValue->ClientSoftwareCertificates, 
                  sizeof(UA_SignedSoftwareCertificate), (UA_EncodeableObject_PfnDecode*) UA_SignedSoftwareCertificate_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);
    status &= ExtensionObject_Read(&a_pValue->UserIdentityToken, msgBuf);
    status &= UA_SignatureData_Decode(&a_pValue->UserTokenSignature, msgBuf);

    if(status != STATUS_OK){
        UA_ActivateSessionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ActivateSessionRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ActivateSessionRequest_EncodeableType =
{
    "ActivateSessionRequest",
    OpcUaId_ActivateSessionRequest,
    OpcUaId_ActivateSessionRequest_Encoding_DefaultBinary,
    OpcUaId_ActivateSessionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ActivateSessionRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_ActivateSessionRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ActivateSessionRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ActivateSessionRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ActivateSessionRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ActivateSessionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ActivateSessionResponse
/*============================================================================
 * UA_ActivateSessionResponse_Initialize
 *===========================================================================*/
void UA_ActivateSessionResponse_Initialize(UA_ActivateSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        ByteString_Initialize(&a_pValue->ServerNonce);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_ActivateSessionResponse_Clear
 *===========================================================================*/
void UA_ActivateSessionResponse_Clear(UA_ActivateSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        ByteString_Clear(&a_pValue->ServerNonce);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_ActivateSessionResponse_Encode
 *===========================================================================*/
StatusCode UA_ActivateSessionResponse_Encode(UA_ActivateSessionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerNonce, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ActivateSessionResponse_Decode
 *===========================================================================*/
StatusCode UA_ActivateSessionResponse_Decode(UA_ActivateSessionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ActivateSessionResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerNonce, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_ActivateSessionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ActivateSessionResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ActivateSessionResponse_EncodeableType =
{
    "ActivateSessionResponse",
    OpcUaId_ActivateSessionResponse,
    OpcUaId_ActivateSessionResponse_Encoding_DefaultBinary,
    OpcUaId_ActivateSessionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ActivateSessionResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_ActivateSessionResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ActivateSessionResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ActivateSessionResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ActivateSessionResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ActivateSessionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
#ifndef OPCUA_EXCLUDE_CloseSessionRequest
/*============================================================================
 * UA_CloseSessionRequest_Initialize
 *===========================================================================*/
void UA_CloseSessionRequest_Initialize(UA_CloseSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Boolean_Initialize(&a_pValue->DeleteSubscriptions);
    }
}

/*============================================================================
 * UA_CloseSessionRequest_Clear
 *===========================================================================*/
void UA_CloseSessionRequest_Clear(UA_CloseSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        Boolean_Clear(&a_pValue->DeleteSubscriptions);
    }
}

/*============================================================================
 * UA_CloseSessionRequest_Encode
 *===========================================================================*/
StatusCode UA_CloseSessionRequest_Encode(UA_CloseSessionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Write(&a_pValue->DeleteSubscriptions, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CloseSessionRequest_Decode
 *===========================================================================*/
StatusCode UA_CloseSessionRequest_Decode(UA_CloseSessionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CloseSessionRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Read(&a_pValue->DeleteSubscriptions, msgBuf);

    if(status != STATUS_OK){
        UA_CloseSessionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CloseSessionRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CloseSessionRequest_EncodeableType =
{
    "CloseSessionRequest",
    OpcUaId_CloseSessionRequest,
    OpcUaId_CloseSessionRequest_Encoding_DefaultBinary,
    OpcUaId_CloseSessionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CloseSessionRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_CloseSessionRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CloseSessionRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CloseSessionRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CloseSessionRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CloseSessionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CloseSessionResponse
/*============================================================================
 * UA_CloseSessionResponse_Initialize
 *===========================================================================*/
void UA_CloseSessionResponse_Initialize(UA_CloseSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_CloseSessionResponse_Clear
 *===========================================================================*/
void UA_CloseSessionResponse_Clear(UA_CloseSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_CloseSessionResponse_Encode
 *===========================================================================*/
StatusCode UA_CloseSessionResponse_Encode(UA_CloseSessionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CloseSessionResponse_Decode
 *===========================================================================*/
StatusCode UA_CloseSessionResponse_Decode(UA_CloseSessionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CloseSessionResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        UA_CloseSessionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CloseSessionResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CloseSessionResponse_EncodeableType =
{
    "CloseSessionResponse",
    OpcUaId_CloseSessionResponse,
    OpcUaId_CloseSessionResponse_Encoding_DefaultBinary,
    OpcUaId_CloseSessionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CloseSessionResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_CloseSessionResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CloseSessionResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CloseSessionResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CloseSessionResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CloseSessionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_Cancel
#ifndef OPCUA_EXCLUDE_CancelRequest
/*============================================================================
 * UA_CancelRequest_Initialize
 *===========================================================================*/
void UA_CancelRequest_Initialize(UA_CancelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->RequestHandle);
    }
}

/*============================================================================
 * UA_CancelRequest_Clear
 *===========================================================================*/
void UA_CancelRequest_Clear(UA_CancelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->RequestHandle);
    }
}

/*============================================================================
 * UA_CancelRequest_Encode
 *===========================================================================*/
StatusCode UA_CancelRequest_Encode(UA_CancelRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestHandle, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CancelRequest_Decode
 *===========================================================================*/
StatusCode UA_CancelRequest_Decode(UA_CancelRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CancelRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestHandle, msgBuf);

    if(status != STATUS_OK){
        UA_CancelRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CancelRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CancelRequest_EncodeableType =
{
    "CancelRequest",
    OpcUaId_CancelRequest,
    OpcUaId_CancelRequest_Encoding_DefaultBinary,
    OpcUaId_CancelRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CancelRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_CancelRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CancelRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CancelRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CancelRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CancelRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CancelResponse
/*============================================================================
 * UA_CancelResponse_Initialize
 *===========================================================================*/
void UA_CancelResponse_Initialize(UA_CancelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UInt32_Initialize(&a_pValue->CancelCount);
    }
}

/*============================================================================
 * UA_CancelResponse_Clear
 *===========================================================================*/
void UA_CancelResponse_Clear(UA_CancelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UInt32_Clear(&a_pValue->CancelCount);
    }
}

/*============================================================================
 * UA_CancelResponse_Encode
 *===========================================================================*/
StatusCode UA_CancelResponse_Encode(UA_CancelResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->CancelCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CancelResponse_Decode
 *===========================================================================*/
StatusCode UA_CancelResponse_Decode(UA_CancelResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CancelResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->CancelCount, msgBuf);

    if(status != STATUS_OK){
        UA_CancelResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CancelResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CancelResponse_EncodeableType =
{
    "CancelResponse",
    OpcUaId_CancelResponse,
    OpcUaId_CancelResponse_Encoding_DefaultBinary,
    OpcUaId_CancelResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CancelResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_CancelResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CancelResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CancelResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CancelResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CancelResponse_Decode
};
#endif
#endif


#ifndef OPCUA_EXCLUDE_NodeAttributes
/*============================================================================
 * UA_NodeAttributes_Initialize
 *===========================================================================*/
void UA_NodeAttributes_Initialize(UA_NodeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SpecifiedAttributes);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
    }
}

/*============================================================================
 * UA_NodeAttributes_Clear
 *===========================================================================*/
void UA_NodeAttributes_Clear(UA_NodeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SpecifiedAttributes);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
    }
}

/*============================================================================
 * UA_NodeAttributes_Encode
 *===========================================================================*/
StatusCode UA_NodeAttributes_Encode(UA_NodeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_NodeAttributes_Decode
 *===========================================================================*/
StatusCode UA_NodeAttributes_Decode(UA_NodeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_NodeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);

    if(status != STATUS_OK){
        UA_NodeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_NodeAttributes_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_NodeAttributes_EncodeableType =
{
    "NodeAttributes",
    OpcUaId_NodeAttributes,
    OpcUaId_NodeAttributes_Encoding_DefaultBinary,
    OpcUaId_NodeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(UA_NodeAttributes),
    (UA_EncodeableObject_PfnInitialize*)UA_NodeAttributes_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_NodeAttributes_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_NodeAttributes_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_NodeAttributes_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_NodeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ObjectAttributes
/*============================================================================
 * UA_ObjectAttributes_Initialize
 *===========================================================================*/
void UA_ObjectAttributes_Initialize(UA_ObjectAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SpecifiedAttributes);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        Byte_Initialize(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * UA_ObjectAttributes_Clear
 *===========================================================================*/
void UA_ObjectAttributes_Clear(UA_ObjectAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SpecifiedAttributes);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        Byte_Clear(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * UA_ObjectAttributes_Encode
 *===========================================================================*/
StatusCode UA_ObjectAttributes_Encode(UA_ObjectAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= Byte_Write(&a_pValue->EventNotifier, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ObjectAttributes_Decode
 *===========================================================================*/
StatusCode UA_ObjectAttributes_Decode(UA_ObjectAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ObjectAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Byte_Read(&a_pValue->EventNotifier, msgBuf);

    if(status != STATUS_OK){
        UA_ObjectAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ObjectAttributes_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ObjectAttributes_EncodeableType =
{
    "ObjectAttributes",
    OpcUaId_ObjectAttributes,
    OpcUaId_ObjectAttributes_Encoding_DefaultBinary,
    OpcUaId_ObjectAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ObjectAttributes),
    (UA_EncodeableObject_PfnInitialize*)UA_ObjectAttributes_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ObjectAttributes_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ObjectAttributes_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ObjectAttributes_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ObjectAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_VariableAttributes
/*============================================================================
 * UA_VariableAttributes_Initialize
 *===========================================================================*/
void UA_VariableAttributes_Initialize(UA_VariableAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SpecifiedAttributes);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        Variant_Initialize(&a_pValue->Value);
        NodeId_Initialize(&a_pValue->DataType);
        Int32_Initialize(&a_pValue->ValueRank);
        UA_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Byte_Initialize(&a_pValue->AccessLevel);
        Byte_Initialize(&a_pValue->UserAccessLevel);
        Double_Initialize(&a_pValue->MinimumSamplingInterval);
        Boolean_Initialize(&a_pValue->Historizing);
    }
}

/*============================================================================
 * UA_VariableAttributes_Clear
 *===========================================================================*/
void UA_VariableAttributes_Clear(UA_VariableAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SpecifiedAttributes);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        Variant_Clear(&a_pValue->Value);
        NodeId_Clear(&a_pValue->DataType);
        Int32_Clear(&a_pValue->ValueRank);
        UA_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
        Byte_Clear(&a_pValue->AccessLevel);
        Byte_Clear(&a_pValue->UserAccessLevel);
        Double_Clear(&a_pValue->MinimumSamplingInterval);
        Boolean_Clear(&a_pValue->Historizing);
    }
}

/*============================================================================
 * UA_VariableAttributes_Encode
 *===========================================================================*/
StatusCode UA_VariableAttributes_Encode(UA_VariableAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= Variant_Write(&a_pValue->Value, msgBuf);
    status &= NodeId_Write(&a_pValue->DataType, msgBuf);
    status &= Int32_Write(&a_pValue->ValueRank, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Byte_Write(&a_pValue->AccessLevel, msgBuf);
    status &= Byte_Write(&a_pValue->UserAccessLevel, msgBuf);
    status &= Double_Write(&a_pValue->MinimumSamplingInterval, msgBuf);
    status &= Boolean_Write(&a_pValue->Historizing, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_VariableAttributes_Decode
 *===========================================================================*/
StatusCode UA_VariableAttributes_Decode(UA_VariableAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_VariableAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Variant_Read(&a_pValue->Value, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Byte_Read(&a_pValue->AccessLevel, msgBuf);
    status &= Byte_Read(&a_pValue->UserAccessLevel, msgBuf);
    status &= Double_Read(&a_pValue->MinimumSamplingInterval, msgBuf);
    status &= Boolean_Read(&a_pValue->Historizing, msgBuf);

    if(status != STATUS_OK){
        UA_VariableAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_VariableAttributes_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_VariableAttributes_EncodeableType =
{
    "VariableAttributes",
    OpcUaId_VariableAttributes,
    OpcUaId_VariableAttributes_Encoding_DefaultBinary,
    OpcUaId_VariableAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(UA_VariableAttributes),
    (UA_EncodeableObject_PfnInitialize*)UA_VariableAttributes_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_VariableAttributes_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_VariableAttributes_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_VariableAttributes_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_VariableAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MethodAttributes
/*============================================================================
 * UA_MethodAttributes_Initialize
 *===========================================================================*/
void UA_MethodAttributes_Initialize(UA_MethodAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SpecifiedAttributes);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        Boolean_Initialize(&a_pValue->Executable);
        Boolean_Initialize(&a_pValue->UserExecutable);
    }
}

/*============================================================================
 * UA_MethodAttributes_Clear
 *===========================================================================*/
void UA_MethodAttributes_Clear(UA_MethodAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SpecifiedAttributes);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        Boolean_Clear(&a_pValue->Executable);
        Boolean_Clear(&a_pValue->UserExecutable);
    }
}

/*============================================================================
 * UA_MethodAttributes_Encode
 *===========================================================================*/
StatusCode UA_MethodAttributes_Encode(UA_MethodAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Write(&a_pValue->Executable, msgBuf);
    status &= Boolean_Write(&a_pValue->UserExecutable, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_MethodAttributes_Decode
 *===========================================================================*/
StatusCode UA_MethodAttributes_Decode(UA_MethodAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_MethodAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->Executable, msgBuf);
    status &= Boolean_Read(&a_pValue->UserExecutable, msgBuf);

    if(status != STATUS_OK){
        UA_MethodAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_MethodAttributes_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_MethodAttributes_EncodeableType =
{
    "MethodAttributes",
    OpcUaId_MethodAttributes,
    OpcUaId_MethodAttributes_Encoding_DefaultBinary,
    OpcUaId_MethodAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(UA_MethodAttributes),
    (UA_EncodeableObject_PfnInitialize*)UA_MethodAttributes_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_MethodAttributes_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_MethodAttributes_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_MethodAttributes_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_MethodAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeAttributes
/*============================================================================
 * UA_ObjectTypeAttributes_Initialize
 *===========================================================================*/
void UA_ObjectTypeAttributes_Initialize(UA_ObjectTypeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SpecifiedAttributes);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_ObjectTypeAttributes_Clear
 *===========================================================================*/
void UA_ObjectTypeAttributes_Clear(UA_ObjectTypeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SpecifiedAttributes);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_ObjectTypeAttributes_Encode
 *===========================================================================*/
StatusCode UA_ObjectTypeAttributes_Encode(UA_ObjectTypeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ObjectTypeAttributes_Decode
 *===========================================================================*/
StatusCode UA_ObjectTypeAttributes_Decode(UA_ObjectTypeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ObjectTypeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        UA_ObjectTypeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ObjectTypeAttributes_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ObjectTypeAttributes_EncodeableType =
{
    "ObjectTypeAttributes",
    OpcUaId_ObjectTypeAttributes,
    OpcUaId_ObjectTypeAttributes_Encoding_DefaultBinary,
    OpcUaId_ObjectTypeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ObjectTypeAttributes),
    (UA_EncodeableObject_PfnInitialize*)UA_ObjectTypeAttributes_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ObjectTypeAttributes_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ObjectTypeAttributes_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ObjectTypeAttributes_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ObjectTypeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeAttributes
/*============================================================================
 * UA_VariableTypeAttributes_Initialize
 *===========================================================================*/
void UA_VariableTypeAttributes_Initialize(UA_VariableTypeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SpecifiedAttributes);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        Variant_Initialize(&a_pValue->Value);
        NodeId_Initialize(&a_pValue->DataType);
        Int32_Initialize(&a_pValue->ValueRank);
        UA_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_VariableTypeAttributes_Clear
 *===========================================================================*/
void UA_VariableTypeAttributes_Clear(UA_VariableTypeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SpecifiedAttributes);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        Variant_Clear(&a_pValue->Value);
        NodeId_Clear(&a_pValue->DataType);
        Int32_Clear(&a_pValue->ValueRank);
        UA_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_VariableTypeAttributes_Encode
 *===========================================================================*/
StatusCode UA_VariableTypeAttributes_Encode(UA_VariableTypeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= Variant_Write(&a_pValue->Value, msgBuf);
    status &= NodeId_Write(&a_pValue->DataType, msgBuf);
    status &= Int32_Write(&a_pValue->ValueRank, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_VariableTypeAttributes_Decode
 *===========================================================================*/
StatusCode UA_VariableTypeAttributes_Decode(UA_VariableTypeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_VariableTypeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Variant_Read(&a_pValue->Value, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        UA_VariableTypeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_VariableTypeAttributes_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_VariableTypeAttributes_EncodeableType =
{
    "VariableTypeAttributes",
    OpcUaId_VariableTypeAttributes,
    OpcUaId_VariableTypeAttributes_Encoding_DefaultBinary,
    OpcUaId_VariableTypeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(UA_VariableTypeAttributes),
    (UA_EncodeableObject_PfnInitialize*)UA_VariableTypeAttributes_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_VariableTypeAttributes_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_VariableTypeAttributes_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_VariableTypeAttributes_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_VariableTypeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeAttributes
/*============================================================================
 * UA_ReferenceTypeAttributes_Initialize
 *===========================================================================*/
void UA_ReferenceTypeAttributes_Initialize(UA_ReferenceTypeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SpecifiedAttributes);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        Boolean_Initialize(&a_pValue->IsAbstract);
        Boolean_Initialize(&a_pValue->Symmetric);
        LocalizedText_Initialize(&a_pValue->InverseName);
    }
}

/*============================================================================
 * UA_ReferenceTypeAttributes_Clear
 *===========================================================================*/
void UA_ReferenceTypeAttributes_Clear(UA_ReferenceTypeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SpecifiedAttributes);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        Boolean_Clear(&a_pValue->IsAbstract);
        Boolean_Clear(&a_pValue->Symmetric);
        LocalizedText_Clear(&a_pValue->InverseName);
    }
}

/*============================================================================
 * UA_ReferenceTypeAttributes_Encode
 *===========================================================================*/
StatusCode UA_ReferenceTypeAttributes_Encode(UA_ReferenceTypeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);
    status &= Boolean_Write(&a_pValue->Symmetric, msgBuf);
    status &= LocalizedText_Write(&a_pValue->InverseName, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReferenceTypeAttributes_Decode
 *===========================================================================*/
StatusCode UA_ReferenceTypeAttributes_Decode(UA_ReferenceTypeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReferenceTypeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);
    status &= Boolean_Read(&a_pValue->Symmetric, msgBuf);
    status &= LocalizedText_Read(&a_pValue->InverseName, msgBuf);

    if(status != STATUS_OK){
        UA_ReferenceTypeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReferenceTypeAttributes_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReferenceTypeAttributes_EncodeableType =
{
    "ReferenceTypeAttributes",
    OpcUaId_ReferenceTypeAttributes,
    OpcUaId_ReferenceTypeAttributes_Encoding_DefaultBinary,
    OpcUaId_ReferenceTypeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReferenceTypeAttributes),
    (UA_EncodeableObject_PfnInitialize*)UA_ReferenceTypeAttributes_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReferenceTypeAttributes_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReferenceTypeAttributes_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReferenceTypeAttributes_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReferenceTypeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DataTypeAttributes
/*============================================================================
 * UA_DataTypeAttributes_Initialize
 *===========================================================================*/
void UA_DataTypeAttributes_Initialize(UA_DataTypeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SpecifiedAttributes);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_DataTypeAttributes_Clear
 *===========================================================================*/
void UA_DataTypeAttributes_Clear(UA_DataTypeAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SpecifiedAttributes);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * UA_DataTypeAttributes_Encode
 *===========================================================================*/
StatusCode UA_DataTypeAttributes_Encode(UA_DataTypeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DataTypeAttributes_Decode
 *===========================================================================*/
StatusCode UA_DataTypeAttributes_Decode(UA_DataTypeAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DataTypeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        UA_DataTypeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DataTypeAttributes_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DataTypeAttributes_EncodeableType =
{
    "DataTypeAttributes",
    OpcUaId_DataTypeAttributes,
    OpcUaId_DataTypeAttributes_Encoding_DefaultBinary,
    OpcUaId_DataTypeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DataTypeAttributes),
    (UA_EncodeableObject_PfnInitialize*)UA_DataTypeAttributes_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DataTypeAttributes_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DataTypeAttributes_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DataTypeAttributes_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DataTypeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ViewAttributes
/*============================================================================
 * UA_ViewAttributes_Initialize
 *===========================================================================*/
void UA_ViewAttributes_Initialize(UA_ViewAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SpecifiedAttributes);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        Boolean_Initialize(&a_pValue->ContainsNoLoops);
        Byte_Initialize(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * UA_ViewAttributes_Clear
 *===========================================================================*/
void UA_ViewAttributes_Clear(UA_ViewAttributes* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SpecifiedAttributes);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        Boolean_Clear(&a_pValue->ContainsNoLoops);
        Byte_Clear(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * UA_ViewAttributes_Encode
 *===========================================================================*/
StatusCode UA_ViewAttributes_Encode(UA_ViewAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Write(&a_pValue->ContainsNoLoops, msgBuf);
    status &= Byte_Write(&a_pValue->EventNotifier, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ViewAttributes_Decode
 *===========================================================================*/
StatusCode UA_ViewAttributes_Decode(UA_ViewAttributes* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ViewAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->ContainsNoLoops, msgBuf);
    status &= Byte_Read(&a_pValue->EventNotifier, msgBuf);

    if(status != STATUS_OK){
        UA_ViewAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ViewAttributes_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ViewAttributes_EncodeableType =
{
    "ViewAttributes",
    OpcUaId_ViewAttributes,
    OpcUaId_ViewAttributes_Encoding_DefaultBinary,
    OpcUaId_ViewAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ViewAttributes),
    (UA_EncodeableObject_PfnInitialize*)UA_ViewAttributes_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ViewAttributes_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ViewAttributes_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ViewAttributes_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ViewAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddNodesItem
/*============================================================================
 * UA_AddNodesItem_Initialize
 *===========================================================================*/
void UA_AddNodesItem_Initialize(UA_AddNodesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Initialize(&a_pValue->ParentNodeId);
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        ExpandedNodeId_Initialize(&a_pValue->RequestedNewNodeId);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        ExtensionObject_Initialize(&a_pValue->NodeAttributes);
        ExpandedNodeId_Initialize(&a_pValue->TypeDefinition);
    }
}

/*============================================================================
 * UA_AddNodesItem_Clear
 *===========================================================================*/
void UA_AddNodesItem_Clear(UA_AddNodesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Clear(&a_pValue->ParentNodeId);
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        ExpandedNodeId_Clear(&a_pValue->RequestedNewNodeId);
        QualifiedName_Clear(&a_pValue->BrowseName);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        ExtensionObject_Clear(&a_pValue->NodeAttributes);
        ExpandedNodeId_Clear(&a_pValue->TypeDefinition);
    }
}

/*============================================================================
 * UA_AddNodesItem_Encode
 *===========================================================================*/
StatusCode UA_AddNodesItem_Encode(UA_AddNodesItem* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= ExpandedNodeId_Write(&a_pValue->ParentNodeId, msgBuf);
    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->RequestedNewNodeId, msgBuf);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= ExtensionObject_Write(&a_pValue->NodeAttributes, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->TypeDefinition, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AddNodesItem_Decode
 *===========================================================================*/
StatusCode UA_AddNodesItem_Decode(UA_AddNodesItem* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AddNodesItem_Initialize(a_pValue);

    status &= ExpandedNodeId_Read(&a_pValue->ParentNodeId, msgBuf);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->RequestedNewNodeId, msgBuf);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= ExtensionObject_Read(&a_pValue->NodeAttributes, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TypeDefinition, msgBuf);

    if(status != STATUS_OK){
        UA_AddNodesItem_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AddNodesItem_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AddNodesItem_EncodeableType =
{
    "AddNodesItem",
    OpcUaId_AddNodesItem,
    OpcUaId_AddNodesItem_Encoding_DefaultBinary,
    OpcUaId_AddNodesItem_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AddNodesItem),
    (UA_EncodeableObject_PfnInitialize*)UA_AddNodesItem_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AddNodesItem_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AddNodesItem_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AddNodesItem_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AddNodesItem_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddNodesResult
/*============================================================================
 * UA_AddNodesResult_Initialize
 *===========================================================================*/
void UA_AddNodesResult_Initialize(UA_AddNodesResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        NodeId_Initialize(&a_pValue->AddedNodeId);
    }
}

/*============================================================================
 * UA_AddNodesResult_Clear
 *===========================================================================*/
void UA_AddNodesResult_Clear(UA_AddNodesResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        NodeId_Clear(&a_pValue->AddedNodeId);
    }
}

/*============================================================================
 * UA_AddNodesResult_Encode
 *===========================================================================*/
StatusCode UA_AddNodesResult_Encode(UA_AddNodesResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= NodeId_Write(&a_pValue->AddedNodeId, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AddNodesResult_Decode
 *===========================================================================*/
StatusCode UA_AddNodesResult_Decode(UA_AddNodesResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AddNodesResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= NodeId_Read(&a_pValue->AddedNodeId, msgBuf);

    if(status != STATUS_OK){
        UA_AddNodesResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AddNodesResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AddNodesResult_EncodeableType =
{
    "AddNodesResult",
    OpcUaId_AddNodesResult,
    OpcUaId_AddNodesResult_Encoding_DefaultBinary,
    OpcUaId_AddNodesResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AddNodesResult),
    (UA_EncodeableObject_PfnInitialize*)UA_AddNodesResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AddNodesResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AddNodesResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AddNodesResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AddNodesResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
#ifndef OPCUA_EXCLUDE_AddNodesRequest
/*============================================================================
 * UA_AddNodesRequest_Initialize
 *===========================================================================*/
void UA_AddNodesRequest_Initialize(UA_AddNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfNodesToAdd, (void**) &a_pValue->NodesToAdd, 
                            sizeof(UA_AddNodesItem), (UA_EncodeableObject_PfnInitialize*) UA_AddNodesItem_Initialize);
    }
}

/*============================================================================
 * UA_AddNodesRequest_Clear
 *===========================================================================*/
void UA_AddNodesRequest_Clear(UA_AddNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfNodesToAdd, (void**) &a_pValue->NodesToAdd, 
                       sizeof(UA_AddNodesItem), (UA_EncodeableObject_PfnClear*) UA_AddNodesItem_Clear);
    }
}

/*============================================================================
 * UA_AddNodesRequest_Encode
 *===========================================================================*/
StatusCode UA_AddNodesRequest_Encode(UA_AddNodesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNodesToAdd, (void**) &a_pValue->NodesToAdd, 
                   sizeof(UA_AddNodesItem), (UA_EncodeableObject_PfnEncode*) UA_AddNodesItem_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AddNodesRequest_Decode
 *===========================================================================*/
StatusCode UA_AddNodesRequest_Decode(UA_AddNodesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AddNodesRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNodesToAdd, (void**) &a_pValue->NodesToAdd, 
                  sizeof(UA_AddNodesItem), (UA_EncodeableObject_PfnDecode*) UA_AddNodesItem_Decode);

    if(status != STATUS_OK){
        UA_AddNodesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AddNodesRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AddNodesRequest_EncodeableType =
{
    "AddNodesRequest",
    OpcUaId_AddNodesRequest,
    OpcUaId_AddNodesRequest_Encoding_DefaultBinary,
    OpcUaId_AddNodesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AddNodesRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_AddNodesRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AddNodesRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AddNodesRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AddNodesRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AddNodesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddNodesResponse
/*============================================================================
 * UA_AddNodesResponse_Initialize
 *===========================================================================*/
void UA_AddNodesResponse_Initialize(UA_AddNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_AddNodesResult), (UA_EncodeableObject_PfnInitialize*) UA_AddNodesResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_AddNodesResponse_Clear
 *===========================================================================*/
void UA_AddNodesResponse_Clear(UA_AddNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_AddNodesResult), (UA_EncodeableObject_PfnClear*) UA_AddNodesResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_AddNodesResponse_Encode
 *===========================================================================*/
StatusCode UA_AddNodesResponse_Encode(UA_AddNodesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_AddNodesResult), (UA_EncodeableObject_PfnEncode*) UA_AddNodesResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AddNodesResponse_Decode
 *===========================================================================*/
StatusCode UA_AddNodesResponse_Decode(UA_AddNodesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AddNodesResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_AddNodesResult), (UA_EncodeableObject_PfnDecode*) UA_AddNodesResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_AddNodesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AddNodesResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AddNodesResponse_EncodeableType =
{
    "AddNodesResponse",
    OpcUaId_AddNodesResponse,
    OpcUaId_AddNodesResponse_Encoding_DefaultBinary,
    OpcUaId_AddNodesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AddNodesResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_AddNodesResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AddNodesResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AddNodesResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AddNodesResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AddNodesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesItem
/*============================================================================
 * UA_AddReferencesItem_Initialize
 *===========================================================================*/
void UA_AddReferencesItem_Initialize(UA_AddReferencesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->SourceNodeId);
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsForward);
        String_Initialize(&a_pValue->TargetServerUri);
        ExpandedNodeId_Initialize(&a_pValue->TargetNodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->TargetNodeClass);
    }
}

/*============================================================================
 * UA_AddReferencesItem_Clear
 *===========================================================================*/
void UA_AddReferencesItem_Clear(UA_AddReferencesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->SourceNodeId);
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsForward);
        String_Clear(&a_pValue->TargetServerUri);
        ExpandedNodeId_Clear(&a_pValue->TargetNodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->TargetNodeClass);
    }
}

/*============================================================================
 * UA_AddReferencesItem_Encode
 *===========================================================================*/
StatusCode UA_AddReferencesItem_Encode(UA_AddReferencesItem* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->SourceNodeId, msgBuf);
    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsForward, msgBuf);
    status &= String_Write(&a_pValue->TargetServerUri, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->TargetNodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TargetNodeClass);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AddReferencesItem_Decode
 *===========================================================================*/
StatusCode UA_AddReferencesItem_Decode(UA_AddReferencesItem* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AddReferencesItem_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->SourceNodeId, msgBuf);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsForward, msgBuf);
    status &= String_Read(&a_pValue->TargetServerUri, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TargetNodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TargetNodeClass);

    if(status != STATUS_OK){
        UA_AddReferencesItem_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AddReferencesItem_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AddReferencesItem_EncodeableType =
{
    "AddReferencesItem",
    OpcUaId_AddReferencesItem,
    OpcUaId_AddReferencesItem_Encoding_DefaultBinary,
    OpcUaId_AddReferencesItem_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AddReferencesItem),
    (UA_EncodeableObject_PfnInitialize*)UA_AddReferencesItem_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AddReferencesItem_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AddReferencesItem_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AddReferencesItem_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AddReferencesItem_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
#ifndef OPCUA_EXCLUDE_AddReferencesRequest
/*============================================================================
 * UA_AddReferencesRequest_Initialize
 *===========================================================================*/
void UA_AddReferencesRequest_Initialize(UA_AddReferencesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfReferencesToAdd, (void**) &a_pValue->ReferencesToAdd, 
                            sizeof(UA_AddReferencesItem), (UA_EncodeableObject_PfnInitialize*) UA_AddReferencesItem_Initialize);
    }
}

/*============================================================================
 * UA_AddReferencesRequest_Clear
 *===========================================================================*/
void UA_AddReferencesRequest_Clear(UA_AddReferencesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfReferencesToAdd, (void**) &a_pValue->ReferencesToAdd, 
                       sizeof(UA_AddReferencesItem), (UA_EncodeableObject_PfnClear*) UA_AddReferencesItem_Clear);
    }
}

/*============================================================================
 * UA_AddReferencesRequest_Encode
 *===========================================================================*/
StatusCode UA_AddReferencesRequest_Encode(UA_AddReferencesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferencesToAdd, (void**) &a_pValue->ReferencesToAdd, 
                   sizeof(UA_AddReferencesItem), (UA_EncodeableObject_PfnEncode*) UA_AddReferencesItem_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AddReferencesRequest_Decode
 *===========================================================================*/
StatusCode UA_AddReferencesRequest_Decode(UA_AddReferencesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AddReferencesRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferencesToAdd, (void**) &a_pValue->ReferencesToAdd, 
                  sizeof(UA_AddReferencesItem), (UA_EncodeableObject_PfnDecode*) UA_AddReferencesItem_Decode);

    if(status != STATUS_OK){
        UA_AddReferencesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AddReferencesRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AddReferencesRequest_EncodeableType =
{
    "AddReferencesRequest",
    OpcUaId_AddReferencesRequest,
    OpcUaId_AddReferencesRequest_Encoding_DefaultBinary,
    OpcUaId_AddReferencesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AddReferencesRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_AddReferencesRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AddReferencesRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AddReferencesRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AddReferencesRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AddReferencesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesResponse
/*============================================================================
 * UA_AddReferencesResponse_Initialize
 *===========================================================================*/
void UA_AddReferencesResponse_Initialize(UA_AddReferencesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_AddReferencesResponse_Clear
 *===========================================================================*/
void UA_AddReferencesResponse_Clear(UA_AddReferencesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_AddReferencesResponse_Encode
 *===========================================================================*/
StatusCode UA_AddReferencesResponse_Encode(UA_AddReferencesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AddReferencesResponse_Decode
 *===========================================================================*/
StatusCode UA_AddReferencesResponse_Decode(UA_AddReferencesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AddReferencesResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_AddReferencesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AddReferencesResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AddReferencesResponse_EncodeableType =
{
    "AddReferencesResponse",
    OpcUaId_AddReferencesResponse,
    OpcUaId_AddReferencesResponse_Encoding_DefaultBinary,
    OpcUaId_AddReferencesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AddReferencesResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_AddReferencesResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AddReferencesResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AddReferencesResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AddReferencesResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AddReferencesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesItem
/*============================================================================
 * UA_DeleteNodesItem_Initialize
 *===========================================================================*/
void UA_DeleteNodesItem_Initialize(UA_DeleteNodesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        Boolean_Initialize(&a_pValue->DeleteTargetReferences);
    }
}

/*============================================================================
 * UA_DeleteNodesItem_Clear
 *===========================================================================*/
void UA_DeleteNodesItem_Clear(UA_DeleteNodesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        Boolean_Clear(&a_pValue->DeleteTargetReferences);
    }
}

/*============================================================================
 * UA_DeleteNodesItem_Encode
 *===========================================================================*/
StatusCode UA_DeleteNodesItem_Encode(UA_DeleteNodesItem* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= Boolean_Write(&a_pValue->DeleteTargetReferences, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteNodesItem_Decode
 *===========================================================================*/
StatusCode UA_DeleteNodesItem_Decode(UA_DeleteNodesItem* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteNodesItem_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= Boolean_Read(&a_pValue->DeleteTargetReferences, msgBuf);

    if(status != STATUS_OK){
        UA_DeleteNodesItem_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteNodesItem_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteNodesItem_EncodeableType =
{
    "DeleteNodesItem",
    OpcUaId_DeleteNodesItem,
    OpcUaId_DeleteNodesItem_Encoding_DefaultBinary,
    OpcUaId_DeleteNodesItem_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteNodesItem),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteNodesItem_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteNodesItem_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteNodesItem_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteNodesItem_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteNodesItem_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
#ifndef OPCUA_EXCLUDE_DeleteNodesRequest
/*============================================================================
 * UA_DeleteNodesRequest_Initialize
 *===========================================================================*/
void UA_DeleteNodesRequest_Initialize(UA_DeleteNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfNodesToDelete, (void**) &a_pValue->NodesToDelete, 
                            sizeof(UA_DeleteNodesItem), (UA_EncodeableObject_PfnInitialize*) UA_DeleteNodesItem_Initialize);
    }
}

/*============================================================================
 * UA_DeleteNodesRequest_Clear
 *===========================================================================*/
void UA_DeleteNodesRequest_Clear(UA_DeleteNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfNodesToDelete, (void**) &a_pValue->NodesToDelete, 
                       sizeof(UA_DeleteNodesItem), (UA_EncodeableObject_PfnClear*) UA_DeleteNodesItem_Clear);
    }
}

/*============================================================================
 * UA_DeleteNodesRequest_Encode
 *===========================================================================*/
StatusCode UA_DeleteNodesRequest_Encode(UA_DeleteNodesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNodesToDelete, (void**) &a_pValue->NodesToDelete, 
                   sizeof(UA_DeleteNodesItem), (UA_EncodeableObject_PfnEncode*) UA_DeleteNodesItem_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteNodesRequest_Decode
 *===========================================================================*/
StatusCode UA_DeleteNodesRequest_Decode(UA_DeleteNodesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteNodesRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNodesToDelete, (void**) &a_pValue->NodesToDelete, 
                  sizeof(UA_DeleteNodesItem), (UA_EncodeableObject_PfnDecode*) UA_DeleteNodesItem_Decode);

    if(status != STATUS_OK){
        UA_DeleteNodesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteNodesRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteNodesRequest_EncodeableType =
{
    "DeleteNodesRequest",
    OpcUaId_DeleteNodesRequest,
    OpcUaId_DeleteNodesRequest_Encoding_DefaultBinary,
    OpcUaId_DeleteNodesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteNodesRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteNodesRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteNodesRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteNodesRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteNodesRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteNodesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesResponse
/*============================================================================
 * UA_DeleteNodesResponse_Initialize
 *===========================================================================*/
void UA_DeleteNodesResponse_Initialize(UA_DeleteNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_DeleteNodesResponse_Clear
 *===========================================================================*/
void UA_DeleteNodesResponse_Clear(UA_DeleteNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_DeleteNodesResponse_Encode
 *===========================================================================*/
StatusCode UA_DeleteNodesResponse_Encode(UA_DeleteNodesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteNodesResponse_Decode
 *===========================================================================*/
StatusCode UA_DeleteNodesResponse_Decode(UA_DeleteNodesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteNodesResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_DeleteNodesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteNodesResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteNodesResponse_EncodeableType =
{
    "DeleteNodesResponse",
    OpcUaId_DeleteNodesResponse,
    OpcUaId_DeleteNodesResponse_Encoding_DefaultBinary,
    OpcUaId_DeleteNodesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteNodesResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteNodesResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteNodesResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteNodesResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteNodesResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteNodesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesItem
/*============================================================================
 * UA_DeleteReferencesItem_Initialize
 *===========================================================================*/
void UA_DeleteReferencesItem_Initialize(UA_DeleteReferencesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->SourceNodeId);
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsForward);
        ExpandedNodeId_Initialize(&a_pValue->TargetNodeId);
        Boolean_Initialize(&a_pValue->DeleteBidirectional);
    }
}

/*============================================================================
 * UA_DeleteReferencesItem_Clear
 *===========================================================================*/
void UA_DeleteReferencesItem_Clear(UA_DeleteReferencesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->SourceNodeId);
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsForward);
        ExpandedNodeId_Clear(&a_pValue->TargetNodeId);
        Boolean_Clear(&a_pValue->DeleteBidirectional);
    }
}

/*============================================================================
 * UA_DeleteReferencesItem_Encode
 *===========================================================================*/
StatusCode UA_DeleteReferencesItem_Encode(UA_DeleteReferencesItem* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->SourceNodeId, msgBuf);
    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsForward, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->TargetNodeId, msgBuf);
    status &= Boolean_Write(&a_pValue->DeleteBidirectional, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteReferencesItem_Decode
 *===========================================================================*/
StatusCode UA_DeleteReferencesItem_Decode(UA_DeleteReferencesItem* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteReferencesItem_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->SourceNodeId, msgBuf);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsForward, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TargetNodeId, msgBuf);
    status &= Boolean_Read(&a_pValue->DeleteBidirectional, msgBuf);

    if(status != STATUS_OK){
        UA_DeleteReferencesItem_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteReferencesItem_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteReferencesItem_EncodeableType =
{
    "DeleteReferencesItem",
    OpcUaId_DeleteReferencesItem,
    OpcUaId_DeleteReferencesItem_Encoding_DefaultBinary,
    OpcUaId_DeleteReferencesItem_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteReferencesItem),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteReferencesItem_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteReferencesItem_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteReferencesItem_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteReferencesItem_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteReferencesItem_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
#ifndef OPCUA_EXCLUDE_DeleteReferencesRequest
/*============================================================================
 * UA_DeleteReferencesRequest_Initialize
 *===========================================================================*/
void UA_DeleteReferencesRequest_Initialize(UA_DeleteReferencesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfReferencesToDelete, (void**) &a_pValue->ReferencesToDelete, 
                            sizeof(UA_DeleteReferencesItem), (UA_EncodeableObject_PfnInitialize*) UA_DeleteReferencesItem_Initialize);
    }
}

/*============================================================================
 * UA_DeleteReferencesRequest_Clear
 *===========================================================================*/
void UA_DeleteReferencesRequest_Clear(UA_DeleteReferencesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfReferencesToDelete, (void**) &a_pValue->ReferencesToDelete, 
                       sizeof(UA_DeleteReferencesItem), (UA_EncodeableObject_PfnClear*) UA_DeleteReferencesItem_Clear);
    }
}

/*============================================================================
 * UA_DeleteReferencesRequest_Encode
 *===========================================================================*/
StatusCode UA_DeleteReferencesRequest_Encode(UA_DeleteReferencesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferencesToDelete, (void**) &a_pValue->ReferencesToDelete, 
                   sizeof(UA_DeleteReferencesItem), (UA_EncodeableObject_PfnEncode*) UA_DeleteReferencesItem_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteReferencesRequest_Decode
 *===========================================================================*/
StatusCode UA_DeleteReferencesRequest_Decode(UA_DeleteReferencesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteReferencesRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferencesToDelete, (void**) &a_pValue->ReferencesToDelete, 
                  sizeof(UA_DeleteReferencesItem), (UA_EncodeableObject_PfnDecode*) UA_DeleteReferencesItem_Decode);

    if(status != STATUS_OK){
        UA_DeleteReferencesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteReferencesRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteReferencesRequest_EncodeableType =
{
    "DeleteReferencesRequest",
    OpcUaId_DeleteReferencesRequest,
    OpcUaId_DeleteReferencesRequest_Encoding_DefaultBinary,
    OpcUaId_DeleteReferencesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteReferencesRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteReferencesRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteReferencesRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteReferencesRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteReferencesRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteReferencesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesResponse
/*============================================================================
 * UA_DeleteReferencesResponse_Initialize
 *===========================================================================*/
void UA_DeleteReferencesResponse_Initialize(UA_DeleteReferencesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_DeleteReferencesResponse_Clear
 *===========================================================================*/
void UA_DeleteReferencesResponse_Clear(UA_DeleteReferencesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_DeleteReferencesResponse_Encode
 *===========================================================================*/
StatusCode UA_DeleteReferencesResponse_Encode(UA_DeleteReferencesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteReferencesResponse_Decode
 *===========================================================================*/
StatusCode UA_DeleteReferencesResponse_Decode(UA_DeleteReferencesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteReferencesResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_DeleteReferencesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteReferencesResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteReferencesResponse_EncodeableType =
{
    "DeleteReferencesResponse",
    OpcUaId_DeleteReferencesResponse,
    OpcUaId_DeleteReferencesResponse_Encoding_DefaultBinary,
    OpcUaId_DeleteReferencesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteReferencesResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteReferencesResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteReferencesResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteReferencesResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteReferencesResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteReferencesResponse_Decode
};
#endif
#endif



#ifndef OPCUA_EXCLUDE_ViewDescription
/*============================================================================
 * UA_ViewDescription_Initialize
 *===========================================================================*/
void UA_ViewDescription_Initialize(UA_ViewDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->ViewId);
        DateTime_Initialize(&a_pValue->Timestamp);
        UInt32_Initialize(&a_pValue->ViewVersion);
    }
}

/*============================================================================
 * UA_ViewDescription_Clear
 *===========================================================================*/
void UA_ViewDescription_Clear(UA_ViewDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->ViewId);
        DateTime_Clear(&a_pValue->Timestamp);
        UInt32_Clear(&a_pValue->ViewVersion);
    }
}

/*============================================================================
 * UA_ViewDescription_Encode
 *===========================================================================*/
StatusCode UA_ViewDescription_Encode(UA_ViewDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->ViewId, msgBuf);
    status &= DateTime_Write(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Write(&a_pValue->ViewVersion, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ViewDescription_Decode
 *===========================================================================*/
StatusCode UA_ViewDescription_Decode(UA_ViewDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ViewDescription_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ViewId, msgBuf);
    status &= DateTime_Read(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Read(&a_pValue->ViewVersion, msgBuf);

    if(status != STATUS_OK){
        UA_ViewDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ViewDescription_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ViewDescription_EncodeableType =
{
    "ViewDescription",
    OpcUaId_ViewDescription,
    OpcUaId_ViewDescription_Encoding_DefaultBinary,
    OpcUaId_ViewDescription_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ViewDescription),
    (UA_EncodeableObject_PfnInitialize*)UA_ViewDescription_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ViewDescription_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ViewDescription_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ViewDescription_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ViewDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowseDescription
/*============================================================================
 * UA_BrowseDescription_Initialize
 *===========================================================================*/
void UA_BrowseDescription_Initialize(UA_BrowseDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->BrowseDirection);
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IncludeSubtypes);
        UInt32_Initialize(&a_pValue->NodeClassMask);
        UInt32_Initialize(&a_pValue->ResultMask);
    }
}

/*============================================================================
 * UA_BrowseDescription_Clear
 *===========================================================================*/
void UA_BrowseDescription_Clear(UA_BrowseDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->BrowseDirection);
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IncludeSubtypes);
        UInt32_Clear(&a_pValue->NodeClassMask);
        UInt32_Clear(&a_pValue->ResultMask);
    }
}

/*============================================================================
 * UA_BrowseDescription_Encode
 *===========================================================================*/
StatusCode UA_BrowseDescription_Encode(UA_BrowseDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->BrowseDirection);
    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IncludeSubtypes, msgBuf);
    status &= UInt32_Write(&a_pValue->NodeClassMask, msgBuf);
    status &= UInt32_Write(&a_pValue->ResultMask, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BrowseDescription_Decode
 *===========================================================================*/
StatusCode UA_BrowseDescription_Decode(UA_BrowseDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BrowseDescription_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->BrowseDirection);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IncludeSubtypes, msgBuf);
    status &= UInt32_Read(&a_pValue->NodeClassMask, msgBuf);
    status &= UInt32_Read(&a_pValue->ResultMask, msgBuf);

    if(status != STATUS_OK){
        UA_BrowseDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BrowseDescription_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BrowseDescription_EncodeableType =
{
    "BrowseDescription",
    OpcUaId_BrowseDescription,
    OpcUaId_BrowseDescription_Encoding_DefaultBinary,
    OpcUaId_BrowseDescription_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BrowseDescription),
    (UA_EncodeableObject_PfnInitialize*)UA_BrowseDescription_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BrowseDescription_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BrowseDescription_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BrowseDescription_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BrowseDescription_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_ReferenceDescription
/*============================================================================
 * UA_ReferenceDescription_Initialize
 *===========================================================================*/
void UA_ReferenceDescription_Initialize(UA_ReferenceDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsForward);
        ExpandedNodeId_Initialize(&a_pValue->NodeId);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        ExpandedNodeId_Initialize(&a_pValue->TypeDefinition);
    }
}

/*============================================================================
 * UA_ReferenceDescription_Clear
 *===========================================================================*/
void UA_ReferenceDescription_Clear(UA_ReferenceDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsForward);
        ExpandedNodeId_Clear(&a_pValue->NodeId);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        ExpandedNodeId_Clear(&a_pValue->TypeDefinition);
    }
}

/*============================================================================
 * UA_ReferenceDescription_Encode
 *===========================================================================*/
StatusCode UA_ReferenceDescription_Encode(UA_ReferenceDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsForward, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= ExpandedNodeId_Write(&a_pValue->TypeDefinition, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReferenceDescription_Decode
 *===========================================================================*/
StatusCode UA_ReferenceDescription_Decode(UA_ReferenceDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReferenceDescription_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsForward, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= ExpandedNodeId_Read(&a_pValue->TypeDefinition, msgBuf);

    if(status != STATUS_OK){
        UA_ReferenceDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReferenceDescription_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReferenceDescription_EncodeableType =
{
    "ReferenceDescription",
    OpcUaId_ReferenceDescription,
    OpcUaId_ReferenceDescription_Encoding_DefaultBinary,
    OpcUaId_ReferenceDescription_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReferenceDescription),
    (UA_EncodeableObject_PfnInitialize*)UA_ReferenceDescription_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReferenceDescription_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReferenceDescription_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReferenceDescription_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReferenceDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowseResult
/*============================================================================
 * UA_BrowseResult_Initialize
 *===========================================================================*/
void UA_BrowseResult_Initialize(UA_BrowseResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        ByteString_Initialize(&a_pValue->ContinuationPoint);
        UA_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(UA_ReferenceDescription), (UA_EncodeableObject_PfnInitialize*) UA_ReferenceDescription_Initialize);
    }
}

/*============================================================================
 * UA_BrowseResult_Clear
 *===========================================================================*/
void UA_BrowseResult_Clear(UA_BrowseResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        ByteString_Clear(&a_pValue->ContinuationPoint);
        UA_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(UA_ReferenceDescription), (UA_EncodeableObject_PfnClear*) UA_ReferenceDescription_Clear);
    }
}

/*============================================================================
 * UA_BrowseResult_Encode
 *===========================================================================*/
StatusCode UA_BrowseResult_Encode(UA_BrowseResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= ByteString_Write(&a_pValue->ContinuationPoint, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(UA_ReferenceDescription), (UA_EncodeableObject_PfnEncode*) UA_ReferenceDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BrowseResult_Decode
 *===========================================================================*/
StatusCode UA_BrowseResult_Decode(UA_BrowseResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BrowseResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(UA_ReferenceDescription), (UA_EncodeableObject_PfnDecode*) UA_ReferenceDescription_Decode);

    if(status != STATUS_OK){
        UA_BrowseResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BrowseResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BrowseResult_EncodeableType =
{
    "BrowseResult",
    OpcUaId_BrowseResult,
    OpcUaId_BrowseResult_Encoding_DefaultBinary,
    OpcUaId_BrowseResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BrowseResult),
    (UA_EncodeableObject_PfnInitialize*)UA_BrowseResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BrowseResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BrowseResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BrowseResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BrowseResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Browse
#ifndef OPCUA_EXCLUDE_BrowseRequest
/*============================================================================
 * UA_BrowseRequest_Initialize
 *===========================================================================*/
void UA_BrowseRequest_Initialize(UA_BrowseRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_ViewDescription_Initialize(&a_pValue->View);
        UInt32_Initialize(&a_pValue->RequestedMaxReferencesPerNode);
        UA_Initialize_Array(&a_pValue->NoOfNodesToBrowse, (void**) &a_pValue->NodesToBrowse, 
                            sizeof(UA_BrowseDescription), (UA_EncodeableObject_PfnInitialize*) UA_BrowseDescription_Initialize);
    }
}

/*============================================================================
 * UA_BrowseRequest_Clear
 *===========================================================================*/
void UA_BrowseRequest_Clear(UA_BrowseRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_ViewDescription_Clear(&a_pValue->View);
        UInt32_Clear(&a_pValue->RequestedMaxReferencesPerNode);
        UA_Clear_Array(&a_pValue->NoOfNodesToBrowse, (void**) &a_pValue->NodesToBrowse, 
                       sizeof(UA_BrowseDescription), (UA_EncodeableObject_PfnClear*) UA_BrowseDescription_Clear);
    }
}

/*============================================================================
 * UA_BrowseRequest_Encode
 *===========================================================================*/
StatusCode UA_BrowseRequest_Encode(UA_BrowseRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_ViewDescription_Encode(&a_pValue->View, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestedMaxReferencesPerNode, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNodesToBrowse, (void**) &a_pValue->NodesToBrowse, 
                   sizeof(UA_BrowseDescription), (UA_EncodeableObject_PfnEncode*) UA_BrowseDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BrowseRequest_Decode
 *===========================================================================*/
StatusCode UA_BrowseRequest_Decode(UA_BrowseRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BrowseRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_ViewDescription_Decode(&a_pValue->View, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedMaxReferencesPerNode, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNodesToBrowse, (void**) &a_pValue->NodesToBrowse, 
                  sizeof(UA_BrowseDescription), (UA_EncodeableObject_PfnDecode*) UA_BrowseDescription_Decode);

    if(status != STATUS_OK){
        UA_BrowseRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BrowseRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BrowseRequest_EncodeableType =
{
    "BrowseRequest",
    OpcUaId_BrowseRequest,
    OpcUaId_BrowseRequest_Encoding_DefaultBinary,
    OpcUaId_BrowseRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BrowseRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_BrowseRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BrowseRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BrowseRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BrowseRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BrowseRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowseResponse
/*============================================================================
 * UA_BrowseResponse_Initialize
 *===========================================================================*/
void UA_BrowseResponse_Initialize(UA_BrowseResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_BrowseResult), (UA_EncodeableObject_PfnInitialize*) UA_BrowseResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_BrowseResponse_Clear
 *===========================================================================*/
void UA_BrowseResponse_Clear(UA_BrowseResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_BrowseResult), (UA_EncodeableObject_PfnClear*) UA_BrowseResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_BrowseResponse_Encode
 *===========================================================================*/
StatusCode UA_BrowseResponse_Encode(UA_BrowseResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_BrowseResult), (UA_EncodeableObject_PfnEncode*) UA_BrowseResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BrowseResponse_Decode
 *===========================================================================*/
StatusCode UA_BrowseResponse_Decode(UA_BrowseResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BrowseResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_BrowseResult), (UA_EncodeableObject_PfnDecode*) UA_BrowseResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_BrowseResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BrowseResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BrowseResponse_EncodeableType =
{
    "BrowseResponse",
    OpcUaId_BrowseResponse,
    OpcUaId_BrowseResponse_Encoding_DefaultBinary,
    OpcUaId_BrowseResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BrowseResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_BrowseResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BrowseResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BrowseResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BrowseResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BrowseResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
#ifndef OPCUA_EXCLUDE_BrowseNextRequest
/*============================================================================
 * UA_BrowseNextRequest_Initialize
 *===========================================================================*/
void UA_BrowseNextRequest_Initialize(UA_BrowseNextRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Boolean_Initialize(&a_pValue->ReleaseContinuationPoints);
        UA_Initialize_Array(&a_pValue->NoOfContinuationPoints, (void**) &a_pValue->ContinuationPoints, 
                            sizeof(UA_ByteString), (UA_EncodeableObject_PfnInitialize*) ByteString_Initialize);
    }
}

/*============================================================================
 * UA_BrowseNextRequest_Clear
 *===========================================================================*/
void UA_BrowseNextRequest_Clear(UA_BrowseNextRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        Boolean_Clear(&a_pValue->ReleaseContinuationPoints);
        UA_Clear_Array(&a_pValue->NoOfContinuationPoints, (void**) &a_pValue->ContinuationPoints, 
                       sizeof(UA_ByteString), (UA_EncodeableObject_PfnClear*) ByteString_Clear);
    }
}

/*============================================================================
 * UA_BrowseNextRequest_Encode
 *===========================================================================*/
StatusCode UA_BrowseNextRequest_Encode(UA_BrowseNextRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Write(&a_pValue->ReleaseContinuationPoints, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfContinuationPoints, (void**) &a_pValue->ContinuationPoints, 
                   sizeof(UA_ByteString), (UA_EncodeableObject_PfnEncode*) ByteString_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BrowseNextRequest_Decode
 *===========================================================================*/
StatusCode UA_BrowseNextRequest_Decode(UA_BrowseNextRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BrowseNextRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Read(&a_pValue->ReleaseContinuationPoints, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfContinuationPoints, (void**) &a_pValue->ContinuationPoints, 
                  sizeof(UA_ByteString), (UA_EncodeableObject_PfnDecode*) ByteString_Read);

    if(status != STATUS_OK){
        UA_BrowseNextRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BrowseNextRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BrowseNextRequest_EncodeableType =
{
    "BrowseNextRequest",
    OpcUaId_BrowseNextRequest,
    OpcUaId_BrowseNextRequest_Encoding_DefaultBinary,
    OpcUaId_BrowseNextRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BrowseNextRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_BrowseNextRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BrowseNextRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BrowseNextRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BrowseNextRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BrowseNextRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowseNextResponse
/*============================================================================
 * UA_BrowseNextResponse_Initialize
 *===========================================================================*/
void UA_BrowseNextResponse_Initialize(UA_BrowseNextResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_BrowseResult), (UA_EncodeableObject_PfnInitialize*) UA_BrowseResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_BrowseNextResponse_Clear
 *===========================================================================*/
void UA_BrowseNextResponse_Clear(UA_BrowseNextResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_BrowseResult), (UA_EncodeableObject_PfnClear*) UA_BrowseResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_BrowseNextResponse_Encode
 *===========================================================================*/
StatusCode UA_BrowseNextResponse_Encode(UA_BrowseNextResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_BrowseResult), (UA_EncodeableObject_PfnEncode*) UA_BrowseResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BrowseNextResponse_Decode
 *===========================================================================*/
StatusCode UA_BrowseNextResponse_Decode(UA_BrowseNextResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BrowseNextResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_BrowseResult), (UA_EncodeableObject_PfnDecode*) UA_BrowseResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_BrowseNextResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BrowseNextResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BrowseNextResponse_EncodeableType =
{
    "BrowseNextResponse",
    OpcUaId_BrowseNextResponse,
    OpcUaId_BrowseNextResponse_Encoding_DefaultBinary,
    OpcUaId_BrowseNextResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BrowseNextResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_BrowseNextResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BrowseNextResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BrowseNextResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BrowseNextResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BrowseNextResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_RelativePathElement
/*============================================================================
 * UA_RelativePathElement_Initialize
 *===========================================================================*/
void UA_RelativePathElement_Initialize(UA_RelativePathElement* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsInverse);
        Boolean_Initialize(&a_pValue->IncludeSubtypes);
        QualifiedName_Initialize(&a_pValue->TargetName);
    }
}

/*============================================================================
 * UA_RelativePathElement_Clear
 *===========================================================================*/
void UA_RelativePathElement_Clear(UA_RelativePathElement* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsInverse);
        Boolean_Clear(&a_pValue->IncludeSubtypes);
        QualifiedName_Clear(&a_pValue->TargetName);
    }
}

/*============================================================================
 * UA_RelativePathElement_Encode
 *===========================================================================*/
StatusCode UA_RelativePathElement_Encode(UA_RelativePathElement* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsInverse, msgBuf);
    status &= Boolean_Write(&a_pValue->IncludeSubtypes, msgBuf);
    status &= QualifiedName_Write(&a_pValue->TargetName, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RelativePathElement_Decode
 *===========================================================================*/
StatusCode UA_RelativePathElement_Decode(UA_RelativePathElement* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RelativePathElement_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsInverse, msgBuf);
    status &= Boolean_Read(&a_pValue->IncludeSubtypes, msgBuf);
    status &= QualifiedName_Read(&a_pValue->TargetName, msgBuf);

    if(status != STATUS_OK){
        UA_RelativePathElement_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RelativePathElement_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RelativePathElement_EncodeableType =
{
    "RelativePathElement",
    OpcUaId_RelativePathElement,
    OpcUaId_RelativePathElement_Encoding_DefaultBinary,
    OpcUaId_RelativePathElement_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RelativePathElement),
    (UA_EncodeableObject_PfnInitialize*)UA_RelativePathElement_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RelativePathElement_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RelativePathElement_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RelativePathElement_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RelativePathElement_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RelativePath
/*============================================================================
 * UA_RelativePath_Initialize
 *===========================================================================*/
void UA_RelativePath_Initialize(UA_RelativePath* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                            sizeof(UA_RelativePathElement), (UA_EncodeableObject_PfnInitialize*) UA_RelativePathElement_Initialize);
    }
}

/*============================================================================
 * UA_RelativePath_Clear
 *===========================================================================*/
void UA_RelativePath_Clear(UA_RelativePath* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                       sizeof(UA_RelativePathElement), (UA_EncodeableObject_PfnClear*) UA_RelativePathElement_Clear);
    }
}

/*============================================================================
 * UA_RelativePath_Encode
 *===========================================================================*/
StatusCode UA_RelativePath_Encode(UA_RelativePath* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                   sizeof(UA_RelativePathElement), (UA_EncodeableObject_PfnEncode*) UA_RelativePathElement_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RelativePath_Decode
 *===========================================================================*/
StatusCode UA_RelativePath_Decode(UA_RelativePath* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RelativePath_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                  sizeof(UA_RelativePathElement), (UA_EncodeableObject_PfnDecode*) UA_RelativePathElement_Decode);

    if(status != STATUS_OK){
        UA_RelativePath_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RelativePath_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RelativePath_EncodeableType =
{
    "RelativePath",
    OpcUaId_RelativePath,
    OpcUaId_RelativePath_Encoding_DefaultBinary,
    OpcUaId_RelativePath_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RelativePath),
    (UA_EncodeableObject_PfnInitialize*)UA_RelativePath_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RelativePath_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RelativePath_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RelativePath_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RelativePath_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowsePath
/*============================================================================
 * UA_BrowsePath_Initialize
 *===========================================================================*/
void UA_BrowsePath_Initialize(UA_BrowsePath* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->StartingNode);
        UA_RelativePath_Initialize(&a_pValue->RelativePath);
    }
}

/*============================================================================
 * UA_BrowsePath_Clear
 *===========================================================================*/
void UA_BrowsePath_Clear(UA_BrowsePath* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->StartingNode);
        UA_RelativePath_Clear(&a_pValue->RelativePath);
    }
}

/*============================================================================
 * UA_BrowsePath_Encode
 *===========================================================================*/
StatusCode UA_BrowsePath_Encode(UA_BrowsePath* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->StartingNode, msgBuf);
    status &= UA_RelativePath_Encode(&a_pValue->RelativePath, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BrowsePath_Decode
 *===========================================================================*/
StatusCode UA_BrowsePath_Decode(UA_BrowsePath* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BrowsePath_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->StartingNode, msgBuf);
    status &= UA_RelativePath_Decode(&a_pValue->RelativePath, msgBuf);

    if(status != STATUS_OK){
        UA_BrowsePath_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BrowsePath_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BrowsePath_EncodeableType =
{
    "BrowsePath",
    OpcUaId_BrowsePath,
    OpcUaId_BrowsePath_Encoding_DefaultBinary,
    OpcUaId_BrowsePath_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BrowsePath),
    (UA_EncodeableObject_PfnInitialize*)UA_BrowsePath_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BrowsePath_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BrowsePath_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BrowsePath_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BrowsePath_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathTarget
/*============================================================================
 * UA_BrowsePathTarget_Initialize
 *===========================================================================*/
void UA_BrowsePathTarget_Initialize(UA_BrowsePathTarget* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Initialize(&a_pValue->TargetId);
        UInt32_Initialize(&a_pValue->RemainingPathIndex);
    }
}

/*============================================================================
 * UA_BrowsePathTarget_Clear
 *===========================================================================*/
void UA_BrowsePathTarget_Clear(UA_BrowsePathTarget* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Clear(&a_pValue->TargetId);
        UInt32_Clear(&a_pValue->RemainingPathIndex);
    }
}

/*============================================================================
 * UA_BrowsePathTarget_Encode
 *===========================================================================*/
StatusCode UA_BrowsePathTarget_Encode(UA_BrowsePathTarget* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= ExpandedNodeId_Write(&a_pValue->TargetId, msgBuf);
    status &= UInt32_Write(&a_pValue->RemainingPathIndex, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BrowsePathTarget_Decode
 *===========================================================================*/
StatusCode UA_BrowsePathTarget_Decode(UA_BrowsePathTarget* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BrowsePathTarget_Initialize(a_pValue);

    status &= ExpandedNodeId_Read(&a_pValue->TargetId, msgBuf);
    status &= UInt32_Read(&a_pValue->RemainingPathIndex, msgBuf);

    if(status != STATUS_OK){
        UA_BrowsePathTarget_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BrowsePathTarget_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BrowsePathTarget_EncodeableType =
{
    "BrowsePathTarget",
    OpcUaId_BrowsePathTarget,
    OpcUaId_BrowsePathTarget_Encoding_DefaultBinary,
    OpcUaId_BrowsePathTarget_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BrowsePathTarget),
    (UA_EncodeableObject_PfnInitialize*)UA_BrowsePathTarget_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BrowsePathTarget_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BrowsePathTarget_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BrowsePathTarget_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BrowsePathTarget_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathResult
/*============================================================================
 * UA_BrowsePathResult_Initialize
 *===========================================================================*/
void UA_BrowsePathResult_Initialize(UA_BrowsePathResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        UA_Initialize_Array(&a_pValue->NoOfTargets, (void**) &a_pValue->Targets, 
                            sizeof(UA_BrowsePathTarget), (UA_EncodeableObject_PfnInitialize*) UA_BrowsePathTarget_Initialize);
    }
}

/*============================================================================
 * UA_BrowsePathResult_Clear
 *===========================================================================*/
void UA_BrowsePathResult_Clear(UA_BrowsePathResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        UA_Clear_Array(&a_pValue->NoOfTargets, (void**) &a_pValue->Targets, 
                       sizeof(UA_BrowsePathTarget), (UA_EncodeableObject_PfnClear*) UA_BrowsePathTarget_Clear);
    }
}

/*============================================================================
 * UA_BrowsePathResult_Encode
 *===========================================================================*/
StatusCode UA_BrowsePathResult_Encode(UA_BrowsePathResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfTargets, (void**) &a_pValue->Targets, 
                   sizeof(UA_BrowsePathTarget), (UA_EncodeableObject_PfnEncode*) UA_BrowsePathTarget_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BrowsePathResult_Decode
 *===========================================================================*/
StatusCode UA_BrowsePathResult_Decode(UA_BrowsePathResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BrowsePathResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfTargets, (void**) &a_pValue->Targets, 
                  sizeof(UA_BrowsePathTarget), (UA_EncodeableObject_PfnDecode*) UA_BrowsePathTarget_Decode);

    if(status != STATUS_OK){
        UA_BrowsePathResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BrowsePathResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BrowsePathResult_EncodeableType =
{
    "BrowsePathResult",
    OpcUaId_BrowsePathResult,
    OpcUaId_BrowsePathResult_Encoding_DefaultBinary,
    OpcUaId_BrowsePathResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BrowsePathResult),
    (UA_EncodeableObject_PfnInitialize*)UA_BrowsePathResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BrowsePathResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BrowsePathResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BrowsePathResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BrowsePathResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIdsRequest
/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsRequest_Initialize
 *===========================================================================*/
void UA_TranslateBrowsePathsToNodeIdsRequest_Initialize(UA_TranslateBrowsePathsToNodeIdsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfBrowsePaths, (void**) &a_pValue->BrowsePaths, 
                            sizeof(UA_BrowsePath), (UA_EncodeableObject_PfnInitialize*) UA_BrowsePath_Initialize);
    }
}

/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsRequest_Clear
 *===========================================================================*/
void UA_TranslateBrowsePathsToNodeIdsRequest_Clear(UA_TranslateBrowsePathsToNodeIdsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfBrowsePaths, (void**) &a_pValue->BrowsePaths, 
                       sizeof(UA_BrowsePath), (UA_EncodeableObject_PfnClear*) UA_BrowsePath_Clear);
    }
}

/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsRequest_Encode
 *===========================================================================*/
StatusCode UA_TranslateBrowsePathsToNodeIdsRequest_Encode(UA_TranslateBrowsePathsToNodeIdsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfBrowsePaths, (void**) &a_pValue->BrowsePaths, 
                   sizeof(UA_BrowsePath), (UA_EncodeableObject_PfnEncode*) UA_BrowsePath_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsRequest_Decode
 *===========================================================================*/
StatusCode UA_TranslateBrowsePathsToNodeIdsRequest_Decode(UA_TranslateBrowsePathsToNodeIdsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_TranslateBrowsePathsToNodeIdsRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfBrowsePaths, (void**) &a_pValue->BrowsePaths, 
                  sizeof(UA_BrowsePath), (UA_EncodeableObject_PfnDecode*) UA_BrowsePath_Decode);

    if(status != STATUS_OK){
        UA_TranslateBrowsePathsToNodeIdsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_TranslateBrowsePathsToNodeIdsRequest_EncodeableType =
{
    "TranslateBrowsePathsToNodeIdsRequest",
    OpcUaId_TranslateBrowsePathsToNodeIdsRequest,
    OpcUaId_TranslateBrowsePathsToNodeIdsRequest_Encoding_DefaultBinary,
    OpcUaId_TranslateBrowsePathsToNodeIdsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_TranslateBrowsePathsToNodeIdsRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_TranslateBrowsePathsToNodeIdsRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_TranslateBrowsePathsToNodeIdsRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_TranslateBrowsePathsToNodeIdsRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_TranslateBrowsePathsToNodeIdsRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_TranslateBrowsePathsToNodeIdsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIdsResponse
/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsResponse_Initialize
 *===========================================================================*/
void UA_TranslateBrowsePathsToNodeIdsResponse_Initialize(UA_TranslateBrowsePathsToNodeIdsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_BrowsePathResult), (UA_EncodeableObject_PfnInitialize*) UA_BrowsePathResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsResponse_Clear
 *===========================================================================*/
void UA_TranslateBrowsePathsToNodeIdsResponse_Clear(UA_TranslateBrowsePathsToNodeIdsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_BrowsePathResult), (UA_EncodeableObject_PfnClear*) UA_BrowsePathResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsResponse_Encode
 *===========================================================================*/
StatusCode UA_TranslateBrowsePathsToNodeIdsResponse_Encode(UA_TranslateBrowsePathsToNodeIdsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_BrowsePathResult), (UA_EncodeableObject_PfnEncode*) UA_BrowsePathResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsResponse_Decode
 *===========================================================================*/
StatusCode UA_TranslateBrowsePathsToNodeIdsResponse_Decode(UA_TranslateBrowsePathsToNodeIdsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_TranslateBrowsePathsToNodeIdsResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_BrowsePathResult), (UA_EncodeableObject_PfnDecode*) UA_BrowsePathResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_TranslateBrowsePathsToNodeIdsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_TranslateBrowsePathsToNodeIdsResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_TranslateBrowsePathsToNodeIdsResponse_EncodeableType =
{
    "TranslateBrowsePathsToNodeIdsResponse",
    OpcUaId_TranslateBrowsePathsToNodeIdsResponse,
    OpcUaId_TranslateBrowsePathsToNodeIdsResponse_Encoding_DefaultBinary,
    OpcUaId_TranslateBrowsePathsToNodeIdsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_TranslateBrowsePathsToNodeIdsResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_TranslateBrowsePathsToNodeIdsResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_TranslateBrowsePathsToNodeIdsResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_TranslateBrowsePathsToNodeIdsResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_TranslateBrowsePathsToNodeIdsResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_TranslateBrowsePathsToNodeIdsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
#ifndef OPCUA_EXCLUDE_RegisterNodesRequest
/*============================================================================
 * UA_RegisterNodesRequest_Initialize
 *===========================================================================*/
void UA_RegisterNodesRequest_Initialize(UA_RegisterNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfNodesToRegister, (void**) &a_pValue->NodesToRegister, 
                            sizeof(UA_NodeId), (UA_EncodeableObject_PfnInitialize*) NodeId_Initialize);
    }
}

/*============================================================================
 * UA_RegisterNodesRequest_Clear
 *===========================================================================*/
void UA_RegisterNodesRequest_Clear(UA_RegisterNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfNodesToRegister, (void**) &a_pValue->NodesToRegister, 
                       sizeof(UA_NodeId), (UA_EncodeableObject_PfnClear*) NodeId_Clear);
    }
}

/*============================================================================
 * UA_RegisterNodesRequest_Encode
 *===========================================================================*/
StatusCode UA_RegisterNodesRequest_Encode(UA_RegisterNodesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNodesToRegister, (void**) &a_pValue->NodesToRegister, 
                   sizeof(UA_NodeId), (UA_EncodeableObject_PfnEncode*) NodeId_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RegisterNodesRequest_Decode
 *===========================================================================*/
StatusCode UA_RegisterNodesRequest_Decode(UA_RegisterNodesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RegisterNodesRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNodesToRegister, (void**) &a_pValue->NodesToRegister, 
                  sizeof(UA_NodeId), (UA_EncodeableObject_PfnDecode*) NodeId_Read);

    if(status != STATUS_OK){
        UA_RegisterNodesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RegisterNodesRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RegisterNodesRequest_EncodeableType =
{
    "RegisterNodesRequest",
    OpcUaId_RegisterNodesRequest,
    OpcUaId_RegisterNodesRequest_Encoding_DefaultBinary,
    OpcUaId_RegisterNodesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RegisterNodesRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_RegisterNodesRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RegisterNodesRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RegisterNodesRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RegisterNodesRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RegisterNodesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodesResponse
/*============================================================================
 * UA_RegisterNodesResponse_Initialize
 *===========================================================================*/
void UA_RegisterNodesResponse_Initialize(UA_RegisterNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfRegisteredNodeIds, (void**) &a_pValue->RegisteredNodeIds, 
                            sizeof(UA_NodeId), (UA_EncodeableObject_PfnInitialize*) NodeId_Initialize);
    }
}

/*============================================================================
 * UA_RegisterNodesResponse_Clear
 *===========================================================================*/
void UA_RegisterNodesResponse_Clear(UA_RegisterNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfRegisteredNodeIds, (void**) &a_pValue->RegisteredNodeIds, 
                       sizeof(UA_NodeId), (UA_EncodeableObject_PfnClear*) NodeId_Clear);
    }
}

/*============================================================================
 * UA_RegisterNodesResponse_Encode
 *===========================================================================*/
StatusCode UA_RegisterNodesResponse_Encode(UA_RegisterNodesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfRegisteredNodeIds, (void**) &a_pValue->RegisteredNodeIds, 
                   sizeof(UA_NodeId), (UA_EncodeableObject_PfnEncode*) NodeId_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RegisterNodesResponse_Decode
 *===========================================================================*/
StatusCode UA_RegisterNodesResponse_Decode(UA_RegisterNodesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RegisterNodesResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfRegisteredNodeIds, (void**) &a_pValue->RegisteredNodeIds, 
                  sizeof(UA_NodeId), (UA_EncodeableObject_PfnDecode*) NodeId_Read);

    if(status != STATUS_OK){
        UA_RegisterNodesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RegisterNodesResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RegisterNodesResponse_EncodeableType =
{
    "RegisterNodesResponse",
    OpcUaId_RegisterNodesResponse,
    OpcUaId_RegisterNodesResponse_Encoding_DefaultBinary,
    OpcUaId_RegisterNodesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RegisterNodesResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_RegisterNodesResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RegisterNodesResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RegisterNodesResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RegisterNodesResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RegisterNodesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
#ifndef OPCUA_EXCLUDE_UnregisterNodesRequest
/*============================================================================
 * UA_UnregisterNodesRequest_Initialize
 *===========================================================================*/
void UA_UnregisterNodesRequest_Initialize(UA_UnregisterNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfNodesToUnregister, (void**) &a_pValue->NodesToUnregister, 
                            sizeof(UA_NodeId), (UA_EncodeableObject_PfnInitialize*) NodeId_Initialize);
    }
}

/*============================================================================
 * UA_UnregisterNodesRequest_Clear
 *===========================================================================*/
void UA_UnregisterNodesRequest_Clear(UA_UnregisterNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfNodesToUnregister, (void**) &a_pValue->NodesToUnregister, 
                       sizeof(UA_NodeId), (UA_EncodeableObject_PfnClear*) NodeId_Clear);
    }
}

/*============================================================================
 * UA_UnregisterNodesRequest_Encode
 *===========================================================================*/
StatusCode UA_UnregisterNodesRequest_Encode(UA_UnregisterNodesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNodesToUnregister, (void**) &a_pValue->NodesToUnregister, 
                   sizeof(UA_NodeId), (UA_EncodeableObject_PfnEncode*) NodeId_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_UnregisterNodesRequest_Decode
 *===========================================================================*/
StatusCode UA_UnregisterNodesRequest_Decode(UA_UnregisterNodesRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_UnregisterNodesRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNodesToUnregister, (void**) &a_pValue->NodesToUnregister, 
                  sizeof(UA_NodeId), (UA_EncodeableObject_PfnDecode*) NodeId_Read);

    if(status != STATUS_OK){
        UA_UnregisterNodesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_UnregisterNodesRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_UnregisterNodesRequest_EncodeableType =
{
    "UnregisterNodesRequest",
    OpcUaId_UnregisterNodesRequest,
    OpcUaId_UnregisterNodesRequest_Encoding_DefaultBinary,
    OpcUaId_UnregisterNodesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_UnregisterNodesRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_UnregisterNodesRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_UnregisterNodesRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_UnregisterNodesRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_UnregisterNodesRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_UnregisterNodesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodesResponse
/*============================================================================
 * UA_UnregisterNodesResponse_Initialize
 *===========================================================================*/
void UA_UnregisterNodesResponse_Initialize(UA_UnregisterNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_UnregisterNodesResponse_Clear
 *===========================================================================*/
void UA_UnregisterNodesResponse_Clear(UA_UnregisterNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * UA_UnregisterNodesResponse_Encode
 *===========================================================================*/
StatusCode UA_UnregisterNodesResponse_Encode(UA_UnregisterNodesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_UnregisterNodesResponse_Decode
 *===========================================================================*/
StatusCode UA_UnregisterNodesResponse_Decode(UA_UnregisterNodesResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_UnregisterNodesResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        UA_UnregisterNodesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_UnregisterNodesResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_UnregisterNodesResponse_EncodeableType =
{
    "UnregisterNodesResponse",
    OpcUaId_UnregisterNodesResponse,
    OpcUaId_UnregisterNodesResponse_Encoding_DefaultBinary,
    OpcUaId_UnregisterNodesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_UnregisterNodesResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_UnregisterNodesResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_UnregisterNodesResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_UnregisterNodesResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_UnregisterNodesResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_UnregisterNodesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_EndpointConfiguration
/*============================================================================
 * UA_EndpointConfiguration_Initialize
 *===========================================================================*/
void UA_EndpointConfiguration_Initialize(UA_EndpointConfiguration* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int32_Initialize(&a_pValue->OperationTimeout);
        Boolean_Initialize(&a_pValue->UseBinaryEncoding);
        Int32_Initialize(&a_pValue->MaxStringLength);
        Int32_Initialize(&a_pValue->MaxByteStringLength);
        Int32_Initialize(&a_pValue->MaxArrayLength);
        Int32_Initialize(&a_pValue->MaxMessageSize);
        Int32_Initialize(&a_pValue->MaxBufferSize);
        Int32_Initialize(&a_pValue->ChannelLifetime);
        Int32_Initialize(&a_pValue->SecurityTokenLifetime);
    }
}

/*============================================================================
 * UA_EndpointConfiguration_Clear
 *===========================================================================*/
void UA_EndpointConfiguration_Clear(UA_EndpointConfiguration* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int32_Clear(&a_pValue->OperationTimeout);
        Boolean_Clear(&a_pValue->UseBinaryEncoding);
        Int32_Clear(&a_pValue->MaxStringLength);
        Int32_Clear(&a_pValue->MaxByteStringLength);
        Int32_Clear(&a_pValue->MaxArrayLength);
        Int32_Clear(&a_pValue->MaxMessageSize);
        Int32_Clear(&a_pValue->MaxBufferSize);
        Int32_Clear(&a_pValue->ChannelLifetime);
        Int32_Clear(&a_pValue->SecurityTokenLifetime);
    }
}

/*============================================================================
 * UA_EndpointConfiguration_Encode
 *===========================================================================*/
StatusCode UA_EndpointConfiguration_Encode(UA_EndpointConfiguration* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Int32_Write(&a_pValue->OperationTimeout, msgBuf);
    status &= Boolean_Write(&a_pValue->UseBinaryEncoding, msgBuf);
    status &= Int32_Write(&a_pValue->MaxStringLength, msgBuf);
    status &= Int32_Write(&a_pValue->MaxByteStringLength, msgBuf);
    status &= Int32_Write(&a_pValue->MaxArrayLength, msgBuf);
    status &= Int32_Write(&a_pValue->MaxMessageSize, msgBuf);
    status &= Int32_Write(&a_pValue->MaxBufferSize, msgBuf);
    status &= Int32_Write(&a_pValue->ChannelLifetime, msgBuf);
    status &= Int32_Write(&a_pValue->SecurityTokenLifetime, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EndpointConfiguration_Decode
 *===========================================================================*/
StatusCode UA_EndpointConfiguration_Decode(UA_EndpointConfiguration* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EndpointConfiguration_Initialize(a_pValue);

    status &= Int32_Read(&a_pValue->OperationTimeout, msgBuf);
    status &= Boolean_Read(&a_pValue->UseBinaryEncoding, msgBuf);
    status &= Int32_Read(&a_pValue->MaxStringLength, msgBuf);
    status &= Int32_Read(&a_pValue->MaxByteStringLength, msgBuf);
    status &= Int32_Read(&a_pValue->MaxArrayLength, msgBuf);
    status &= Int32_Read(&a_pValue->MaxMessageSize, msgBuf);
    status &= Int32_Read(&a_pValue->MaxBufferSize, msgBuf);
    status &= Int32_Read(&a_pValue->ChannelLifetime, msgBuf);
    status &= Int32_Read(&a_pValue->SecurityTokenLifetime, msgBuf);

    if(status != STATUS_OK){
        UA_EndpointConfiguration_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EndpointConfiguration_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EndpointConfiguration_EncodeableType =
{
    "EndpointConfiguration",
    OpcUaId_EndpointConfiguration,
    OpcUaId_EndpointConfiguration_Encoding_DefaultBinary,
    OpcUaId_EndpointConfiguration_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EndpointConfiguration),
    (UA_EncodeableObject_PfnInitialize*)UA_EndpointConfiguration_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EndpointConfiguration_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EndpointConfiguration_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EndpointConfiguration_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EndpointConfiguration_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_SupportedProfile
/*============================================================================
 * UA_SupportedProfile_Initialize
 *===========================================================================*/
void UA_SupportedProfile_Initialize(UA_SupportedProfile* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->OrganizationUri);
        String_Initialize(&a_pValue->ProfileId);
        String_Initialize(&a_pValue->ComplianceTool);
        DateTime_Initialize(&a_pValue->ComplianceDate);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->ComplianceLevel);
        UA_Initialize_Array(&a_pValue->NoOfUnsupportedUnitIds, (void**) &a_pValue->UnsupportedUnitIds, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * UA_SupportedProfile_Clear
 *===========================================================================*/
void UA_SupportedProfile_Clear(UA_SupportedProfile* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->OrganizationUri);
        String_Clear(&a_pValue->ProfileId);
        String_Clear(&a_pValue->ComplianceTool);
        DateTime_Clear(&a_pValue->ComplianceDate);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->ComplianceLevel);
        UA_Clear_Array(&a_pValue->NoOfUnsupportedUnitIds, (void**) &a_pValue->UnsupportedUnitIds, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * UA_SupportedProfile_Encode
 *===========================================================================*/
StatusCode UA_SupportedProfile_Encode(UA_SupportedProfile* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->OrganizationUri, msgBuf);
    status &= String_Write(&a_pValue->ProfileId, msgBuf);
    status &= String_Write(&a_pValue->ComplianceTool, msgBuf);
    status &= DateTime_Write(&a_pValue->ComplianceDate, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ComplianceLevel);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfUnsupportedUnitIds, (void**) &a_pValue->UnsupportedUnitIds, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SupportedProfile_Decode
 *===========================================================================*/
StatusCode UA_SupportedProfile_Decode(UA_SupportedProfile* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SupportedProfile_Initialize(a_pValue);

    status &= String_Read(&a_pValue->OrganizationUri, msgBuf);
    status &= String_Read(&a_pValue->ProfileId, msgBuf);
    status &= String_Read(&a_pValue->ComplianceTool, msgBuf);
    status &= DateTime_Read(&a_pValue->ComplianceDate, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ComplianceLevel);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfUnsupportedUnitIds, (void**) &a_pValue->UnsupportedUnitIds, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        UA_SupportedProfile_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SupportedProfile_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SupportedProfile_EncodeableType =
{
    "SupportedProfile",
    OpcUaId_SupportedProfile,
    OpcUaId_SupportedProfile_Encoding_DefaultBinary,
    OpcUaId_SupportedProfile_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SupportedProfile),
    (UA_EncodeableObject_PfnInitialize*)UA_SupportedProfile_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SupportedProfile_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SupportedProfile_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SupportedProfile_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SupportedProfile_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SoftwareCertificate
/*============================================================================
 * UA_SoftwareCertificate_Initialize
 *===========================================================================*/
void UA_SoftwareCertificate_Initialize(UA_SoftwareCertificate* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ProductName);
        String_Initialize(&a_pValue->ProductUri);
        String_Initialize(&a_pValue->VendorName);
        ByteString_Initialize(&a_pValue->VendorProductCertificate);
        String_Initialize(&a_pValue->SoftwareVersion);
        String_Initialize(&a_pValue->BuildNumber);
        DateTime_Initialize(&a_pValue->BuildDate);
        String_Initialize(&a_pValue->IssuedBy);
        DateTime_Initialize(&a_pValue->IssueDate);
        UA_Initialize_Array(&a_pValue->NoOfSupportedProfiles, (void**) &a_pValue->SupportedProfiles, 
                            sizeof(UA_SupportedProfile), (UA_EncodeableObject_PfnInitialize*) UA_SupportedProfile_Initialize);
    }
}

/*============================================================================
 * UA_SoftwareCertificate_Clear
 *===========================================================================*/
void UA_SoftwareCertificate_Clear(UA_SoftwareCertificate* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ProductName);
        String_Clear(&a_pValue->ProductUri);
        String_Clear(&a_pValue->VendorName);
        ByteString_Clear(&a_pValue->VendorProductCertificate);
        String_Clear(&a_pValue->SoftwareVersion);
        String_Clear(&a_pValue->BuildNumber);
        DateTime_Clear(&a_pValue->BuildDate);
        String_Clear(&a_pValue->IssuedBy);
        DateTime_Clear(&a_pValue->IssueDate);
        UA_Clear_Array(&a_pValue->NoOfSupportedProfiles, (void**) &a_pValue->SupportedProfiles, 
                       sizeof(UA_SupportedProfile), (UA_EncodeableObject_PfnClear*) UA_SupportedProfile_Clear);
    }
}

/*============================================================================
 * UA_SoftwareCertificate_Encode
 *===========================================================================*/
StatusCode UA_SoftwareCertificate_Encode(UA_SoftwareCertificate* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ProductName, msgBuf);
    status &= String_Write(&a_pValue->ProductUri, msgBuf);
    status &= String_Write(&a_pValue->VendorName, msgBuf);
    status &= ByteString_Write(&a_pValue->VendorProductCertificate, msgBuf);
    status &= String_Write(&a_pValue->SoftwareVersion, msgBuf);
    status &= String_Write(&a_pValue->BuildNumber, msgBuf);
    status &= DateTime_Write(&a_pValue->BuildDate, msgBuf);
    status &= String_Write(&a_pValue->IssuedBy, msgBuf);
    status &= DateTime_Write(&a_pValue->IssueDate, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfSupportedProfiles, (void**) &a_pValue->SupportedProfiles, 
                   sizeof(UA_SupportedProfile), (UA_EncodeableObject_PfnEncode*) UA_SupportedProfile_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SoftwareCertificate_Decode
 *===========================================================================*/
StatusCode UA_SoftwareCertificate_Decode(UA_SoftwareCertificate* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SoftwareCertificate_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ProductName, msgBuf);
    status &= String_Read(&a_pValue->ProductUri, msgBuf);
    status &= String_Read(&a_pValue->VendorName, msgBuf);
    status &= ByteString_Read(&a_pValue->VendorProductCertificate, msgBuf);
    status &= String_Read(&a_pValue->SoftwareVersion, msgBuf);
    status &= String_Read(&a_pValue->BuildNumber, msgBuf);
    status &= DateTime_Read(&a_pValue->BuildDate, msgBuf);
    status &= String_Read(&a_pValue->IssuedBy, msgBuf);
    status &= DateTime_Read(&a_pValue->IssueDate, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfSupportedProfiles, (void**) &a_pValue->SupportedProfiles, 
                  sizeof(UA_SupportedProfile), (UA_EncodeableObject_PfnDecode*) UA_SupportedProfile_Decode);

    if(status != STATUS_OK){
        UA_SoftwareCertificate_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SoftwareCertificate_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SoftwareCertificate_EncodeableType =
{
    "SoftwareCertificate",
    OpcUaId_SoftwareCertificate,
    OpcUaId_SoftwareCertificate_Encoding_DefaultBinary,
    OpcUaId_SoftwareCertificate_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SoftwareCertificate),
    (UA_EncodeableObject_PfnInitialize*)UA_SoftwareCertificate_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SoftwareCertificate_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SoftwareCertificate_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SoftwareCertificate_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SoftwareCertificate_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_QueryDataDescription
/*============================================================================
 * UA_QueryDataDescription_Initialize
 *===========================================================================*/
void UA_QueryDataDescription_Initialize(UA_QueryDataDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RelativePath_Initialize(&a_pValue->RelativePath);
        UInt32_Initialize(&a_pValue->AttributeId);
        String_Initialize(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * UA_QueryDataDescription_Clear
 *===========================================================================*/
void UA_QueryDataDescription_Clear(UA_QueryDataDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RelativePath_Clear(&a_pValue->RelativePath);
        UInt32_Clear(&a_pValue->AttributeId);
        String_Clear(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * UA_QueryDataDescription_Encode
 *===========================================================================*/
StatusCode UA_QueryDataDescription_Encode(UA_QueryDataDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RelativePath_Encode(&a_pValue->RelativePath, msgBuf);
    status &= UInt32_Write(&a_pValue->AttributeId, msgBuf);
    status &= String_Write(&a_pValue->IndexRange, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_QueryDataDescription_Decode
 *===========================================================================*/
StatusCode UA_QueryDataDescription_Decode(UA_QueryDataDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_QueryDataDescription_Initialize(a_pValue);

    status &= UA_RelativePath_Decode(&a_pValue->RelativePath, msgBuf);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);

    if(status != STATUS_OK){
        UA_QueryDataDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_QueryDataDescription_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_QueryDataDescription_EncodeableType =
{
    "QueryDataDescription",
    OpcUaId_QueryDataDescription,
    OpcUaId_QueryDataDescription_Encoding_DefaultBinary,
    OpcUaId_QueryDataDescription_Encoding_DefaultXml,
    NULL,
    sizeof(UA_QueryDataDescription),
    (UA_EncodeableObject_PfnInitialize*)UA_QueryDataDescription_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_QueryDataDescription_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_QueryDataDescription_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_QueryDataDescription_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_QueryDataDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_NodeTypeDescription
/*============================================================================
 * UA_NodeTypeDescription_Initialize
 *===========================================================================*/
void UA_NodeTypeDescription_Initialize(UA_NodeTypeDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Initialize(&a_pValue->TypeDefinitionNode);
        Boolean_Initialize(&a_pValue->IncludeSubTypes);
        UA_Initialize_Array(&a_pValue->NoOfDataToReturn, (void**) &a_pValue->DataToReturn, 
                            sizeof(UA_QueryDataDescription), (UA_EncodeableObject_PfnInitialize*) UA_QueryDataDescription_Initialize);
    }
}

/*============================================================================
 * UA_NodeTypeDescription_Clear
 *===========================================================================*/
void UA_NodeTypeDescription_Clear(UA_NodeTypeDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Clear(&a_pValue->TypeDefinitionNode);
        Boolean_Clear(&a_pValue->IncludeSubTypes);
        UA_Clear_Array(&a_pValue->NoOfDataToReturn, (void**) &a_pValue->DataToReturn, 
                       sizeof(UA_QueryDataDescription), (UA_EncodeableObject_PfnClear*) UA_QueryDataDescription_Clear);
    }
}

/*============================================================================
 * UA_NodeTypeDescription_Encode
 *===========================================================================*/
StatusCode UA_NodeTypeDescription_Encode(UA_NodeTypeDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= ExpandedNodeId_Write(&a_pValue->TypeDefinitionNode, msgBuf);
    status &= Boolean_Write(&a_pValue->IncludeSubTypes, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDataToReturn, (void**) &a_pValue->DataToReturn, 
                   sizeof(UA_QueryDataDescription), (UA_EncodeableObject_PfnEncode*) UA_QueryDataDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_NodeTypeDescription_Decode
 *===========================================================================*/
StatusCode UA_NodeTypeDescription_Decode(UA_NodeTypeDescription* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_NodeTypeDescription_Initialize(a_pValue);

    status &= ExpandedNodeId_Read(&a_pValue->TypeDefinitionNode, msgBuf);
    status &= Boolean_Read(&a_pValue->IncludeSubTypes, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDataToReturn, (void**) &a_pValue->DataToReturn, 
                  sizeof(UA_QueryDataDescription), (UA_EncodeableObject_PfnDecode*) UA_QueryDataDescription_Decode);

    if(status != STATUS_OK){
        UA_NodeTypeDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_NodeTypeDescription_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_NodeTypeDescription_EncodeableType =
{
    "NodeTypeDescription",
    OpcUaId_NodeTypeDescription,
    OpcUaId_NodeTypeDescription_Encoding_DefaultBinary,
    OpcUaId_NodeTypeDescription_Encoding_DefaultXml,
    NULL,
    sizeof(UA_NodeTypeDescription),
    (UA_EncodeableObject_PfnInitialize*)UA_NodeTypeDescription_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_NodeTypeDescription_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_NodeTypeDescription_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_NodeTypeDescription_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_NodeTypeDescription_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_QueryDataSet
/*============================================================================
 * UA_QueryDataSet_Initialize
 *===========================================================================*/
void UA_QueryDataSet_Initialize(UA_QueryDataSet* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Initialize(&a_pValue->NodeId);
        ExpandedNodeId_Initialize(&a_pValue->TypeDefinitionNode);
        UA_Initialize_Array(&a_pValue->NoOfValues, (void**) &a_pValue->Values, 
                            sizeof(UA_Variant), (UA_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * UA_QueryDataSet_Clear
 *===========================================================================*/
void UA_QueryDataSet_Clear(UA_QueryDataSet* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Clear(&a_pValue->NodeId);
        ExpandedNodeId_Clear(&a_pValue->TypeDefinitionNode);
        UA_Clear_Array(&a_pValue->NoOfValues, (void**) &a_pValue->Values, 
                       sizeof(UA_Variant), (UA_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * UA_QueryDataSet_Encode
 *===========================================================================*/
StatusCode UA_QueryDataSet_Encode(UA_QueryDataSet* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= ExpandedNodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->TypeDefinitionNode, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfValues, (void**) &a_pValue->Values, 
                   sizeof(UA_Variant), (UA_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_QueryDataSet_Decode
 *===========================================================================*/
StatusCode UA_QueryDataSet_Decode(UA_QueryDataSet* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_QueryDataSet_Initialize(a_pValue);

    status &= ExpandedNodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TypeDefinitionNode, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfValues, (void**) &a_pValue->Values, 
                  sizeof(UA_Variant), (UA_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        UA_QueryDataSet_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_QueryDataSet_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_QueryDataSet_EncodeableType =
{
    "QueryDataSet",
    OpcUaId_QueryDataSet,
    OpcUaId_QueryDataSet_Encoding_DefaultBinary,
    OpcUaId_QueryDataSet_Encoding_DefaultXml,
    NULL,
    sizeof(UA_QueryDataSet),
    (UA_EncodeableObject_PfnInitialize*)UA_QueryDataSet_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_QueryDataSet_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_QueryDataSet_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_QueryDataSet_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_QueryDataSet_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_NodeReference
/*============================================================================
 * UA_NodeReference_Initialize
 *===========================================================================*/
void UA_NodeReference_Initialize(UA_NodeReference* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsForward);
        UA_Initialize_Array(&a_pValue->NoOfReferencedNodeIds, (void**) &a_pValue->ReferencedNodeIds, 
                            sizeof(UA_NodeId), (UA_EncodeableObject_PfnInitialize*) NodeId_Initialize);
    }
}

/*============================================================================
 * UA_NodeReference_Clear
 *===========================================================================*/
void UA_NodeReference_Clear(UA_NodeReference* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsForward);
        UA_Clear_Array(&a_pValue->NoOfReferencedNodeIds, (void**) &a_pValue->ReferencedNodeIds, 
                       sizeof(UA_NodeId), (UA_EncodeableObject_PfnClear*) NodeId_Clear);
    }
}

/*============================================================================
 * UA_NodeReference_Encode
 *===========================================================================*/
StatusCode UA_NodeReference_Encode(UA_NodeReference* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsForward, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReferencedNodeIds, (void**) &a_pValue->ReferencedNodeIds, 
                   sizeof(UA_NodeId), (UA_EncodeableObject_PfnEncode*) NodeId_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_NodeReference_Decode
 *===========================================================================*/
StatusCode UA_NodeReference_Decode(UA_NodeReference* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_NodeReference_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsForward, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReferencedNodeIds, (void**) &a_pValue->ReferencedNodeIds, 
                  sizeof(UA_NodeId), (UA_EncodeableObject_PfnDecode*) NodeId_Read);

    if(status != STATUS_OK){
        UA_NodeReference_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_NodeReference_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_NodeReference_EncodeableType =
{
    "NodeReference",
    OpcUaId_NodeReference,
    OpcUaId_NodeReference_Encoding_DefaultBinary,
    OpcUaId_NodeReference_Encoding_DefaultXml,
    NULL,
    sizeof(UA_NodeReference),
    (UA_EncodeableObject_PfnInitialize*)UA_NodeReference_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_NodeReference_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_NodeReference_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_NodeReference_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_NodeReference_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElement
/*============================================================================
 * UA_ContentFilterElement_Initialize
 *===========================================================================*/
void UA_ContentFilterElement_Initialize(UA_ContentFilterElement* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->FilterOperator);
        UA_Initialize_Array(&a_pValue->NoOfFilterOperands, (void**) &a_pValue->FilterOperands, 
                            sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnInitialize*) ExtensionObject_Initialize);
    }
}

/*============================================================================
 * UA_ContentFilterElement_Clear
 *===========================================================================*/
void UA_ContentFilterElement_Clear(UA_ContentFilterElement* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->FilterOperator);
        UA_Clear_Array(&a_pValue->NoOfFilterOperands, (void**) &a_pValue->FilterOperands, 
                       sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnClear*) ExtensionObject_Clear);
    }
}

/*============================================================================
 * UA_ContentFilterElement_Encode
 *===========================================================================*/
StatusCode UA_ContentFilterElement_Encode(UA_ContentFilterElement* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->FilterOperator);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfFilterOperands, (void**) &a_pValue->FilterOperands, 
                   sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnEncode*) ExtensionObject_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ContentFilterElement_Decode
 *===========================================================================*/
StatusCode UA_ContentFilterElement_Decode(UA_ContentFilterElement* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ContentFilterElement_Initialize(a_pValue);

    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->FilterOperator);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfFilterOperands, (void**) &a_pValue->FilterOperands, 
                  sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnDecode*) ExtensionObject_Read);

    if(status != STATUS_OK){
        UA_ContentFilterElement_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ContentFilterElement_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ContentFilterElement_EncodeableType =
{
    "ContentFilterElement",
    OpcUaId_ContentFilterElement,
    OpcUaId_ContentFilterElement_Encoding_DefaultBinary,
    OpcUaId_ContentFilterElement_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ContentFilterElement),
    (UA_EncodeableObject_PfnInitialize*)UA_ContentFilterElement_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ContentFilterElement_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ContentFilterElement_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ContentFilterElement_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ContentFilterElement_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ContentFilter
/*============================================================================
 * UA_ContentFilter_Initialize
 *===========================================================================*/
void UA_ContentFilter_Initialize(UA_ContentFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                            sizeof(UA_ContentFilterElement), (UA_EncodeableObject_PfnInitialize*) UA_ContentFilterElement_Initialize);
    }
}

/*============================================================================
 * UA_ContentFilter_Clear
 *===========================================================================*/
void UA_ContentFilter_Clear(UA_ContentFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                       sizeof(UA_ContentFilterElement), (UA_EncodeableObject_PfnClear*) UA_ContentFilterElement_Clear);
    }
}

/*============================================================================
 * UA_ContentFilter_Encode
 *===========================================================================*/
StatusCode UA_ContentFilter_Encode(UA_ContentFilter* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                   sizeof(UA_ContentFilterElement), (UA_EncodeableObject_PfnEncode*) UA_ContentFilterElement_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ContentFilter_Decode
 *===========================================================================*/
StatusCode UA_ContentFilter_Decode(UA_ContentFilter* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ContentFilter_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                  sizeof(UA_ContentFilterElement), (UA_EncodeableObject_PfnDecode*) UA_ContentFilterElement_Decode);

    if(status != STATUS_OK){
        UA_ContentFilter_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ContentFilter_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ContentFilter_EncodeableType =
{
    "ContentFilter",
    OpcUaId_ContentFilter,
    OpcUaId_ContentFilter_Encoding_DefaultBinary,
    OpcUaId_ContentFilter_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ContentFilter),
    (UA_EncodeableObject_PfnInitialize*)UA_ContentFilter_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ContentFilter_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ContentFilter_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ContentFilter_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ContentFilter_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ElementOperand
/*============================================================================
 * UA_ElementOperand_Initialize
 *===========================================================================*/
void UA_ElementOperand_Initialize(UA_ElementOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->Index);
    }
}

/*============================================================================
 * UA_ElementOperand_Clear
 *===========================================================================*/
void UA_ElementOperand_Clear(UA_ElementOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->Index);
    }
}

/*============================================================================
 * UA_ElementOperand_Encode
 *===========================================================================*/
StatusCode UA_ElementOperand_Encode(UA_ElementOperand* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->Index, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ElementOperand_Decode
 *===========================================================================*/
StatusCode UA_ElementOperand_Decode(UA_ElementOperand* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ElementOperand_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->Index, msgBuf);

    if(status != STATUS_OK){
        UA_ElementOperand_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ElementOperand_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ElementOperand_EncodeableType =
{
    "ElementOperand",
    OpcUaId_ElementOperand,
    OpcUaId_ElementOperand_Encoding_DefaultBinary,
    OpcUaId_ElementOperand_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ElementOperand),
    (UA_EncodeableObject_PfnInitialize*)UA_ElementOperand_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ElementOperand_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ElementOperand_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ElementOperand_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ElementOperand_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_LiteralOperand
/*============================================================================
 * UA_LiteralOperand_Initialize
 *===========================================================================*/
void UA_LiteralOperand_Initialize(UA_LiteralOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        Variant_Initialize(&a_pValue->Value);
    }
}

/*============================================================================
 * UA_LiteralOperand_Clear
 *===========================================================================*/
void UA_LiteralOperand_Clear(UA_LiteralOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        Variant_Clear(&a_pValue->Value);
    }
}

/*============================================================================
 * UA_LiteralOperand_Encode
 *===========================================================================*/
StatusCode UA_LiteralOperand_Encode(UA_LiteralOperand* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Variant_Write(&a_pValue->Value, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_LiteralOperand_Decode
 *===========================================================================*/
StatusCode UA_LiteralOperand_Decode(UA_LiteralOperand* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_LiteralOperand_Initialize(a_pValue);

    status &= Variant_Read(&a_pValue->Value, msgBuf);

    if(status != STATUS_OK){
        UA_LiteralOperand_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_LiteralOperand_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_LiteralOperand_EncodeableType =
{
    "LiteralOperand",
    OpcUaId_LiteralOperand,
    OpcUaId_LiteralOperand_Encoding_DefaultBinary,
    OpcUaId_LiteralOperand_Encoding_DefaultXml,
    NULL,
    sizeof(UA_LiteralOperand),
    (UA_EncodeableObject_PfnInitialize*)UA_LiteralOperand_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_LiteralOperand_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_LiteralOperand_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_LiteralOperand_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_LiteralOperand_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AttributeOperand
/*============================================================================
 * UA_AttributeOperand_Initialize
 *===========================================================================*/
void UA_AttributeOperand_Initialize(UA_AttributeOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        String_Initialize(&a_pValue->Alias);
        UA_RelativePath_Initialize(&a_pValue->BrowsePath);
        UInt32_Initialize(&a_pValue->AttributeId);
        String_Initialize(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * UA_AttributeOperand_Clear
 *===========================================================================*/
void UA_AttributeOperand_Clear(UA_AttributeOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        String_Clear(&a_pValue->Alias);
        UA_RelativePath_Clear(&a_pValue->BrowsePath);
        UInt32_Clear(&a_pValue->AttributeId);
        String_Clear(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * UA_AttributeOperand_Encode
 *===========================================================================*/
StatusCode UA_AttributeOperand_Encode(UA_AttributeOperand* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= String_Write(&a_pValue->Alias, msgBuf);
    status &= UA_RelativePath_Encode(&a_pValue->BrowsePath, msgBuf);
    status &= UInt32_Write(&a_pValue->AttributeId, msgBuf);
    status &= String_Write(&a_pValue->IndexRange, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AttributeOperand_Decode
 *===========================================================================*/
StatusCode UA_AttributeOperand_Decode(UA_AttributeOperand* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AttributeOperand_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= String_Read(&a_pValue->Alias, msgBuf);
    status &= UA_RelativePath_Decode(&a_pValue->BrowsePath, msgBuf);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);

    if(status != STATUS_OK){
        UA_AttributeOperand_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AttributeOperand_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AttributeOperand_EncodeableType =
{
    "AttributeOperand",
    OpcUaId_AttributeOperand,
    OpcUaId_AttributeOperand_Encoding_DefaultBinary,
    OpcUaId_AttributeOperand_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AttributeOperand),
    (UA_EncodeableObject_PfnInitialize*)UA_AttributeOperand_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AttributeOperand_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AttributeOperand_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AttributeOperand_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AttributeOperand_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SimpleAttributeOperand
/*============================================================================
 * UA_SimpleAttributeOperand_Initialize
 *===========================================================================*/
void UA_SimpleAttributeOperand_Initialize(UA_SimpleAttributeOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->TypeDefinitionId);
        UA_Initialize_Array(&a_pValue->NoOfBrowsePath, (void**) &a_pValue->BrowsePath, 
                            sizeof(UA_QualifiedName), (UA_EncodeableObject_PfnInitialize*) QualifiedName_Initialize);
        UInt32_Initialize(&a_pValue->AttributeId);
        String_Initialize(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * UA_SimpleAttributeOperand_Clear
 *===========================================================================*/
void UA_SimpleAttributeOperand_Clear(UA_SimpleAttributeOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->TypeDefinitionId);
        UA_Clear_Array(&a_pValue->NoOfBrowsePath, (void**) &a_pValue->BrowsePath, 
                       sizeof(UA_QualifiedName), (UA_EncodeableObject_PfnClear*) QualifiedName_Clear);
        UInt32_Clear(&a_pValue->AttributeId);
        String_Clear(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * UA_SimpleAttributeOperand_Encode
 *===========================================================================*/
StatusCode UA_SimpleAttributeOperand_Encode(UA_SimpleAttributeOperand* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->TypeDefinitionId, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfBrowsePath, (void**) &a_pValue->BrowsePath, 
                   sizeof(UA_QualifiedName), (UA_EncodeableObject_PfnEncode*) QualifiedName_Write);
    status &= UInt32_Write(&a_pValue->AttributeId, msgBuf);
    status &= String_Write(&a_pValue->IndexRange, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SimpleAttributeOperand_Decode
 *===========================================================================*/
StatusCode UA_SimpleAttributeOperand_Decode(UA_SimpleAttributeOperand* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SimpleAttributeOperand_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->TypeDefinitionId, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfBrowsePath, (void**) &a_pValue->BrowsePath, 
                  sizeof(UA_QualifiedName), (UA_EncodeableObject_PfnDecode*) QualifiedName_Read);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);

    if(status != STATUS_OK){
        UA_SimpleAttributeOperand_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SimpleAttributeOperand_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SimpleAttributeOperand_EncodeableType =
{
    "SimpleAttributeOperand",
    OpcUaId_SimpleAttributeOperand,
    OpcUaId_SimpleAttributeOperand_Encoding_DefaultBinary,
    OpcUaId_SimpleAttributeOperand_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SimpleAttributeOperand),
    (UA_EncodeableObject_PfnInitialize*)UA_SimpleAttributeOperand_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SimpleAttributeOperand_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SimpleAttributeOperand_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SimpleAttributeOperand_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SimpleAttributeOperand_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElementResult
/*============================================================================
 * UA_ContentFilterElementResult_Initialize
 *===========================================================================*/
void UA_ContentFilterElementResult_Initialize(UA_ContentFilterElementResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        UA_Initialize_Array(&a_pValue->NoOfOperandStatusCodes, (void**) &a_pValue->OperandStatusCodes, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfOperandDiagnosticInfos, (void**) &a_pValue->OperandDiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_ContentFilterElementResult_Clear
 *===========================================================================*/
void UA_ContentFilterElementResult_Clear(UA_ContentFilterElementResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        UA_Clear_Array(&a_pValue->NoOfOperandStatusCodes, (void**) &a_pValue->OperandStatusCodes, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfOperandDiagnosticInfos, (void**) &a_pValue->OperandDiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_ContentFilterElementResult_Encode
 *===========================================================================*/
StatusCode UA_ContentFilterElementResult_Encode(UA_ContentFilterElementResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfOperandStatusCodes, (void**) &a_pValue->OperandStatusCodes, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfOperandDiagnosticInfos, (void**) &a_pValue->OperandDiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ContentFilterElementResult_Decode
 *===========================================================================*/
StatusCode UA_ContentFilterElementResult_Decode(UA_ContentFilterElementResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ContentFilterElementResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfOperandStatusCodes, (void**) &a_pValue->OperandStatusCodes, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfOperandDiagnosticInfos, (void**) &a_pValue->OperandDiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_ContentFilterElementResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ContentFilterElementResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ContentFilterElementResult_EncodeableType =
{
    "ContentFilterElementResult",
    OpcUaId_ContentFilterElementResult,
    OpcUaId_ContentFilterElementResult_Encoding_DefaultBinary,
    OpcUaId_ContentFilterElementResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ContentFilterElementResult),
    (UA_EncodeableObject_PfnInitialize*)UA_ContentFilterElementResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ContentFilterElementResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ContentFilterElementResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ContentFilterElementResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ContentFilterElementResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterResult
/*============================================================================
 * UA_ContentFilterResult_Initialize
 *===========================================================================*/
void UA_ContentFilterResult_Initialize(UA_ContentFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfElementResults, (void**) &a_pValue->ElementResults, 
                            sizeof(UA_ContentFilterElementResult), (UA_EncodeableObject_PfnInitialize*) UA_ContentFilterElementResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfElementDiagnosticInfos, (void**) &a_pValue->ElementDiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_ContentFilterResult_Clear
 *===========================================================================*/
void UA_ContentFilterResult_Clear(UA_ContentFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfElementResults, (void**) &a_pValue->ElementResults, 
                       sizeof(UA_ContentFilterElementResult), (UA_EncodeableObject_PfnClear*) UA_ContentFilterElementResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfElementDiagnosticInfos, (void**) &a_pValue->ElementDiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_ContentFilterResult_Encode
 *===========================================================================*/
StatusCode UA_ContentFilterResult_Encode(UA_ContentFilterResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfElementResults, (void**) &a_pValue->ElementResults, 
                   sizeof(UA_ContentFilterElementResult), (UA_EncodeableObject_PfnEncode*) UA_ContentFilterElementResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfElementDiagnosticInfos, (void**) &a_pValue->ElementDiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ContentFilterResult_Decode
 *===========================================================================*/
StatusCode UA_ContentFilterResult_Decode(UA_ContentFilterResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ContentFilterResult_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfElementResults, (void**) &a_pValue->ElementResults, 
                  sizeof(UA_ContentFilterElementResult), (UA_EncodeableObject_PfnDecode*) UA_ContentFilterElementResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfElementDiagnosticInfos, (void**) &a_pValue->ElementDiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_ContentFilterResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ContentFilterResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ContentFilterResult_EncodeableType =
{
    "ContentFilterResult",
    OpcUaId_ContentFilterResult,
    OpcUaId_ContentFilterResult_Encoding_DefaultBinary,
    OpcUaId_ContentFilterResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ContentFilterResult),
    (UA_EncodeableObject_PfnInitialize*)UA_ContentFilterResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ContentFilterResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ContentFilterResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ContentFilterResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ContentFilterResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ParsingResult
/*============================================================================
 * UA_ParsingResult_Initialize
 *===========================================================================*/
void UA_ParsingResult_Initialize(UA_ParsingResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        UA_Initialize_Array(&a_pValue->NoOfDataStatusCodes, (void**) &a_pValue->DataStatusCodes, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDataDiagnosticInfos, (void**) &a_pValue->DataDiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_ParsingResult_Clear
 *===========================================================================*/
void UA_ParsingResult_Clear(UA_ParsingResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        UA_Clear_Array(&a_pValue->NoOfDataStatusCodes, (void**) &a_pValue->DataStatusCodes, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDataDiagnosticInfos, (void**) &a_pValue->DataDiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_ParsingResult_Encode
 *===========================================================================*/
StatusCode UA_ParsingResult_Encode(UA_ParsingResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDataStatusCodes, (void**) &a_pValue->DataStatusCodes, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDataDiagnosticInfos, (void**) &a_pValue->DataDiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ParsingResult_Decode
 *===========================================================================*/
StatusCode UA_ParsingResult_Decode(UA_ParsingResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ParsingResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDataStatusCodes, (void**) &a_pValue->DataStatusCodes, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDataDiagnosticInfos, (void**) &a_pValue->DataDiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_ParsingResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ParsingResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ParsingResult_EncodeableType =
{
    "ParsingResult",
    OpcUaId_ParsingResult,
    OpcUaId_ParsingResult_Encoding_DefaultBinary,
    OpcUaId_ParsingResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ParsingResult),
    (UA_EncodeableObject_PfnInitialize*)UA_ParsingResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ParsingResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ParsingResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ParsingResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ParsingResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
#ifndef OPCUA_EXCLUDE_QueryFirstRequest
/*============================================================================
 * UA_QueryFirstRequest_Initialize
 *===========================================================================*/
void UA_QueryFirstRequest_Initialize(UA_QueryFirstRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_ViewDescription_Initialize(&a_pValue->View);
        UA_Initialize_Array(&a_pValue->NoOfNodeTypes, (void**) &a_pValue->NodeTypes, 
                            sizeof(UA_NodeTypeDescription), (UA_EncodeableObject_PfnInitialize*) UA_NodeTypeDescription_Initialize);
        UA_ContentFilter_Initialize(&a_pValue->Filter);
        UInt32_Initialize(&a_pValue->MaxDataSetsToReturn);
        UInt32_Initialize(&a_pValue->MaxReferencesToReturn);
    }
}

/*============================================================================
 * UA_QueryFirstRequest_Clear
 *===========================================================================*/
void UA_QueryFirstRequest_Clear(UA_QueryFirstRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_ViewDescription_Clear(&a_pValue->View);
        UA_Clear_Array(&a_pValue->NoOfNodeTypes, (void**) &a_pValue->NodeTypes, 
                       sizeof(UA_NodeTypeDescription), (UA_EncodeableObject_PfnClear*) UA_NodeTypeDescription_Clear);
        UA_ContentFilter_Clear(&a_pValue->Filter);
        UInt32_Clear(&a_pValue->MaxDataSetsToReturn);
        UInt32_Clear(&a_pValue->MaxReferencesToReturn);
    }
}

/*============================================================================
 * UA_QueryFirstRequest_Encode
 *===========================================================================*/
StatusCode UA_QueryFirstRequest_Encode(UA_QueryFirstRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_ViewDescription_Encode(&a_pValue->View, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNodeTypes, (void**) &a_pValue->NodeTypes, 
                   sizeof(UA_NodeTypeDescription), (UA_EncodeableObject_PfnEncode*) UA_NodeTypeDescription_Encode);
    status &= UA_ContentFilter_Encode(&a_pValue->Filter, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxDataSetsToReturn, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxReferencesToReturn, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_QueryFirstRequest_Decode
 *===========================================================================*/
StatusCode UA_QueryFirstRequest_Decode(UA_QueryFirstRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_QueryFirstRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_ViewDescription_Decode(&a_pValue->View, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNodeTypes, (void**) &a_pValue->NodeTypes, 
                  sizeof(UA_NodeTypeDescription), (UA_EncodeableObject_PfnDecode*) UA_NodeTypeDescription_Decode);
    status &= UA_ContentFilter_Decode(&a_pValue->Filter, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxDataSetsToReturn, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxReferencesToReturn, msgBuf);

    if(status != STATUS_OK){
        UA_QueryFirstRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_QueryFirstRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_QueryFirstRequest_EncodeableType =
{
    "QueryFirstRequest",
    OpcUaId_QueryFirstRequest,
    OpcUaId_QueryFirstRequest_Encoding_DefaultBinary,
    OpcUaId_QueryFirstRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_QueryFirstRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_QueryFirstRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_QueryFirstRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_QueryFirstRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_QueryFirstRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_QueryFirstRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_QueryFirstResponse
/*============================================================================
 * UA_QueryFirstResponse_Initialize
 *===========================================================================*/
void UA_QueryFirstResponse_Initialize(UA_QueryFirstResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                            sizeof(UA_QueryDataSet), (UA_EncodeableObject_PfnInitialize*) UA_QueryDataSet_Initialize);
        ByteString_Initialize(&a_pValue->ContinuationPoint);
        UA_Initialize_Array(&a_pValue->NoOfParsingResults, (void**) &a_pValue->ParsingResults, 
                            sizeof(UA_ParsingResult), (UA_EncodeableObject_PfnInitialize*) UA_ParsingResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
        UA_ContentFilterResult_Initialize(&a_pValue->FilterResult);
    }
}

/*============================================================================
 * UA_QueryFirstResponse_Clear
 *===========================================================================*/
void UA_QueryFirstResponse_Clear(UA_QueryFirstResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                       sizeof(UA_QueryDataSet), (UA_EncodeableObject_PfnClear*) UA_QueryDataSet_Clear);
        ByteString_Clear(&a_pValue->ContinuationPoint);
        UA_Clear_Array(&a_pValue->NoOfParsingResults, (void**) &a_pValue->ParsingResults, 
                       sizeof(UA_ParsingResult), (UA_EncodeableObject_PfnClear*) UA_ParsingResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
        UA_ContentFilterResult_Clear(&a_pValue->FilterResult);
    }
}

/*============================================================================
 * UA_QueryFirstResponse_Encode
 *===========================================================================*/
StatusCode UA_QueryFirstResponse_Encode(UA_QueryFirstResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                   sizeof(UA_QueryDataSet), (UA_EncodeableObject_PfnEncode*) UA_QueryDataSet_Encode);
    status &= ByteString_Write(&a_pValue->ContinuationPoint, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfParsingResults, (void**) &a_pValue->ParsingResults, 
                   sizeof(UA_ParsingResult), (UA_EncodeableObject_PfnEncode*) UA_ParsingResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);
    status &= UA_ContentFilterResult_Encode(&a_pValue->FilterResult, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_QueryFirstResponse_Decode
 *===========================================================================*/
StatusCode UA_QueryFirstResponse_Decode(UA_QueryFirstResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_QueryFirstResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                  sizeof(UA_QueryDataSet), (UA_EncodeableObject_PfnDecode*) UA_QueryDataSet_Decode);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfParsingResults, (void**) &a_pValue->ParsingResults, 
                  sizeof(UA_ParsingResult), (UA_EncodeableObject_PfnDecode*) UA_ParsingResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);
    status &= UA_ContentFilterResult_Decode(&a_pValue->FilterResult, msgBuf);

    if(status != STATUS_OK){
        UA_QueryFirstResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_QueryFirstResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_QueryFirstResponse_EncodeableType =
{
    "QueryFirstResponse",
    OpcUaId_QueryFirstResponse,
    OpcUaId_QueryFirstResponse_Encoding_DefaultBinary,
    OpcUaId_QueryFirstResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_QueryFirstResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_QueryFirstResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_QueryFirstResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_QueryFirstResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_QueryFirstResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_QueryFirstResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
#ifndef OPCUA_EXCLUDE_QueryNextRequest
/*============================================================================
 * UA_QueryNextRequest_Initialize
 *===========================================================================*/
void UA_QueryNextRequest_Initialize(UA_QueryNextRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Boolean_Initialize(&a_pValue->ReleaseContinuationPoint);
        ByteString_Initialize(&a_pValue->ContinuationPoint);
    }
}

/*============================================================================
 * UA_QueryNextRequest_Clear
 *===========================================================================*/
void UA_QueryNextRequest_Clear(UA_QueryNextRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        Boolean_Clear(&a_pValue->ReleaseContinuationPoint);
        ByteString_Clear(&a_pValue->ContinuationPoint);
    }
}

/*============================================================================
 * UA_QueryNextRequest_Encode
 *===========================================================================*/
StatusCode UA_QueryNextRequest_Encode(UA_QueryNextRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Write(&a_pValue->ReleaseContinuationPoint, msgBuf);
    status &= ByteString_Write(&a_pValue->ContinuationPoint, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_QueryNextRequest_Decode
 *===========================================================================*/
StatusCode UA_QueryNextRequest_Decode(UA_QueryNextRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_QueryNextRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Read(&a_pValue->ReleaseContinuationPoint, msgBuf);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);

    if(status != STATUS_OK){
        UA_QueryNextRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_QueryNextRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_QueryNextRequest_EncodeableType =
{
    "QueryNextRequest",
    OpcUaId_QueryNextRequest,
    OpcUaId_QueryNextRequest_Encoding_DefaultBinary,
    OpcUaId_QueryNextRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_QueryNextRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_QueryNextRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_QueryNextRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_QueryNextRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_QueryNextRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_QueryNextRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_QueryNextResponse
/*============================================================================
 * UA_QueryNextResponse_Initialize
 *===========================================================================*/
void UA_QueryNextResponse_Initialize(UA_QueryNextResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                            sizeof(UA_QueryDataSet), (UA_EncodeableObject_PfnInitialize*) UA_QueryDataSet_Initialize);
        ByteString_Initialize(&a_pValue->RevisedContinuationPoint);
    }
}

/*============================================================================
 * UA_QueryNextResponse_Clear
 *===========================================================================*/
void UA_QueryNextResponse_Clear(UA_QueryNextResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                       sizeof(UA_QueryDataSet), (UA_EncodeableObject_PfnClear*) UA_QueryDataSet_Clear);
        ByteString_Clear(&a_pValue->RevisedContinuationPoint);
    }
}

/*============================================================================
 * UA_QueryNextResponse_Encode
 *===========================================================================*/
StatusCode UA_QueryNextResponse_Encode(UA_QueryNextResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                   sizeof(UA_QueryDataSet), (UA_EncodeableObject_PfnEncode*) UA_QueryDataSet_Encode);
    status &= ByteString_Write(&a_pValue->RevisedContinuationPoint, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_QueryNextResponse_Decode
 *===========================================================================*/
StatusCode UA_QueryNextResponse_Decode(UA_QueryNextResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_QueryNextResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                  sizeof(UA_QueryDataSet), (UA_EncodeableObject_PfnDecode*) UA_QueryDataSet_Decode);
    status &= ByteString_Read(&a_pValue->RevisedContinuationPoint, msgBuf);

    if(status != STATUS_OK){
        UA_QueryNextResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_QueryNextResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_QueryNextResponse_EncodeableType =
{
    "QueryNextResponse",
    OpcUaId_QueryNextResponse,
    OpcUaId_QueryNextResponse_Encoding_DefaultBinary,
    OpcUaId_QueryNextResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_QueryNextResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_QueryNextResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_QueryNextResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_QueryNextResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_QueryNextResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_QueryNextResponse_Decode
};
#endif
#endif


#ifndef OPCUA_EXCLUDE_ReadValueId
/*============================================================================
 * UA_ReadValueId_Initialize
 *===========================================================================*/
void UA_ReadValueId_Initialize(UA_ReadValueId* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UInt32_Initialize(&a_pValue->AttributeId);
        String_Initialize(&a_pValue->IndexRange);
        QualifiedName_Initialize(&a_pValue->DataEncoding);
    }
}

/*============================================================================
 * UA_ReadValueId_Clear
 *===========================================================================*/
void UA_ReadValueId_Clear(UA_ReadValueId* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UInt32_Clear(&a_pValue->AttributeId);
        String_Clear(&a_pValue->IndexRange);
        QualifiedName_Clear(&a_pValue->DataEncoding);
    }
}

/*============================================================================
 * UA_ReadValueId_Encode
 *===========================================================================*/
StatusCode UA_ReadValueId_Encode(UA_ReadValueId* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UInt32_Write(&a_pValue->AttributeId, msgBuf);
    status &= String_Write(&a_pValue->IndexRange, msgBuf);
    status &= QualifiedName_Write(&a_pValue->DataEncoding, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReadValueId_Decode
 *===========================================================================*/
StatusCode UA_ReadValueId_Decode(UA_ReadValueId* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReadValueId_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);
    status &= QualifiedName_Read(&a_pValue->DataEncoding, msgBuf);

    if(status != STATUS_OK){
        UA_ReadValueId_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReadValueId_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReadValueId_EncodeableType =
{
    "ReadValueId",
    OpcUaId_ReadValueId,
    OpcUaId_ReadValueId_Encoding_DefaultBinary,
    OpcUaId_ReadValueId_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReadValueId),
    (UA_EncodeableObject_PfnInitialize*)UA_ReadValueId_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReadValueId_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReadValueId_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReadValueId_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReadValueId_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Read
#ifndef OPCUA_EXCLUDE_ReadRequest
/*============================================================================
 * UA_ReadRequest_Initialize
 *===========================================================================*/
void UA_ReadRequest_Initialize(UA_ReadRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Double_Initialize(&a_pValue->MaxAge);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        UA_Initialize_Array(&a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                            sizeof(UA_ReadValueId), (UA_EncodeableObject_PfnInitialize*) UA_ReadValueId_Initialize);
    }
}

/*============================================================================
 * UA_ReadRequest_Clear
 *===========================================================================*/
void UA_ReadRequest_Clear(UA_ReadRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        Double_Clear(&a_pValue->MaxAge);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        UA_Clear_Array(&a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                       sizeof(UA_ReadValueId), (UA_EncodeableObject_PfnClear*) UA_ReadValueId_Clear);
    }
}

/*============================================================================
 * UA_ReadRequest_Encode
 *===========================================================================*/
StatusCode UA_ReadRequest_Encode(UA_ReadRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Double_Write(&a_pValue->MaxAge, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                   sizeof(UA_ReadValueId), (UA_EncodeableObject_PfnEncode*) UA_ReadValueId_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReadRequest_Decode
 *===========================================================================*/
StatusCode UA_ReadRequest_Decode(UA_ReadRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReadRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Double_Read(&a_pValue->MaxAge, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                  sizeof(UA_ReadValueId), (UA_EncodeableObject_PfnDecode*) UA_ReadValueId_Decode);

    if(status != STATUS_OK){
        UA_ReadRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReadRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReadRequest_EncodeableType =
{
    "ReadRequest",
    OpcUaId_ReadRequest,
    OpcUaId_ReadRequest_Encoding_DefaultBinary,
    OpcUaId_ReadRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReadRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_ReadRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReadRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReadRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReadRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReadRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadResponse
/*============================================================================
 * UA_ReadResponse_Initialize
 *===========================================================================*/
void UA_ReadResponse_Initialize(UA_ReadResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_DataValue), (UA_EncodeableObject_PfnInitialize*) DataValue_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_ReadResponse_Clear
 *===========================================================================*/
void UA_ReadResponse_Clear(UA_ReadResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_DataValue), (UA_EncodeableObject_PfnClear*) DataValue_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_ReadResponse_Encode
 *===========================================================================*/
StatusCode UA_ReadResponse_Encode(UA_ReadResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_DataValue), (UA_EncodeableObject_PfnEncode*) DataValue_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReadResponse_Decode
 *===========================================================================*/
StatusCode UA_ReadResponse_Decode(UA_ReadResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReadResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_DataValue), (UA_EncodeableObject_PfnDecode*) DataValue_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_ReadResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReadResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReadResponse_EncodeableType =
{
    "ReadResponse",
    OpcUaId_ReadResponse,
    OpcUaId_ReadResponse_Encoding_DefaultBinary,
    OpcUaId_ReadResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReadResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_ReadResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReadResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReadResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReadResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReadResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadValueId
/*============================================================================
 * UA_HistoryReadValueId_Initialize
 *===========================================================================*/
void UA_HistoryReadValueId_Initialize(UA_HistoryReadValueId* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        String_Initialize(&a_pValue->IndexRange);
        QualifiedName_Initialize(&a_pValue->DataEncoding);
        ByteString_Initialize(&a_pValue->ContinuationPoint);
    }
}

/*============================================================================
 * UA_HistoryReadValueId_Clear
 *===========================================================================*/
void UA_HistoryReadValueId_Clear(UA_HistoryReadValueId* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        String_Clear(&a_pValue->IndexRange);
        QualifiedName_Clear(&a_pValue->DataEncoding);
        ByteString_Clear(&a_pValue->ContinuationPoint);
    }
}

/*============================================================================
 * UA_HistoryReadValueId_Encode
 *===========================================================================*/
StatusCode UA_HistoryReadValueId_Encode(UA_HistoryReadValueId* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= String_Write(&a_pValue->IndexRange, msgBuf);
    status &= QualifiedName_Write(&a_pValue->DataEncoding, msgBuf);
    status &= ByteString_Write(&a_pValue->ContinuationPoint, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryReadValueId_Decode
 *===========================================================================*/
StatusCode UA_HistoryReadValueId_Decode(UA_HistoryReadValueId* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryReadValueId_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);
    status &= QualifiedName_Read(&a_pValue->DataEncoding, msgBuf);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);

    if(status != STATUS_OK){
        UA_HistoryReadValueId_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryReadValueId_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryReadValueId_EncodeableType =
{
    "HistoryReadValueId",
    OpcUaId_HistoryReadValueId,
    OpcUaId_HistoryReadValueId_Encoding_DefaultBinary,
    OpcUaId_HistoryReadValueId_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryReadValueId),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryReadValueId_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryReadValueId_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryReadValueId_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryReadValueId_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryReadValueId_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadResult
/*============================================================================
 * UA_HistoryReadResult_Initialize
 *===========================================================================*/
void UA_HistoryReadResult_Initialize(UA_HistoryReadResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        ByteString_Initialize(&a_pValue->ContinuationPoint);
        ExtensionObject_Initialize(&a_pValue->HistoryData);
    }
}

/*============================================================================
 * UA_HistoryReadResult_Clear
 *===========================================================================*/
void UA_HistoryReadResult_Clear(UA_HistoryReadResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        ByteString_Clear(&a_pValue->ContinuationPoint);
        ExtensionObject_Clear(&a_pValue->HistoryData);
    }
}

/*============================================================================
 * UA_HistoryReadResult_Encode
 *===========================================================================*/
StatusCode UA_HistoryReadResult_Encode(UA_HistoryReadResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= ByteString_Write(&a_pValue->ContinuationPoint, msgBuf);
    status &= ExtensionObject_Write(&a_pValue->HistoryData, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryReadResult_Decode
 *===========================================================================*/
StatusCode UA_HistoryReadResult_Decode(UA_HistoryReadResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryReadResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->HistoryData, msgBuf);

    if(status != STATUS_OK){
        UA_HistoryReadResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryReadResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryReadResult_EncodeableType =
{
    "HistoryReadResult",
    OpcUaId_HistoryReadResult,
    OpcUaId_HistoryReadResult_Encoding_DefaultBinary,
    OpcUaId_HistoryReadResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryReadResult),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryReadResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryReadResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryReadResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryReadResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryReadResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadEventDetails
/*============================================================================
 * UA_ReadEventDetails_Initialize
 *===========================================================================*/
void UA_ReadEventDetails_Initialize(UA_ReadEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->NumValuesPerNode);
        DateTime_Initialize(&a_pValue->StartTime);
        DateTime_Initialize(&a_pValue->EndTime);
        UA_EventFilter_Initialize(&a_pValue->Filter);
    }
}

/*============================================================================
 * UA_ReadEventDetails_Clear
 *===========================================================================*/
void UA_ReadEventDetails_Clear(UA_ReadEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->NumValuesPerNode);
        DateTime_Clear(&a_pValue->StartTime);
        DateTime_Clear(&a_pValue->EndTime);
        UA_EventFilter_Clear(&a_pValue->Filter);
    }
}

/*============================================================================
 * UA_ReadEventDetails_Encode
 *===========================================================================*/
StatusCode UA_ReadEventDetails_Encode(UA_ReadEventDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->NumValuesPerNode, msgBuf);
    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Write(&a_pValue->EndTime, msgBuf);
    status &= UA_EventFilter_Encode(&a_pValue->Filter, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReadEventDetails_Decode
 *===========================================================================*/
StatusCode UA_ReadEventDetails_Decode(UA_ReadEventDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReadEventDetails_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->NumValuesPerNode, msgBuf);
    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->EndTime, msgBuf);
    status &= UA_EventFilter_Decode(&a_pValue->Filter, msgBuf);

    if(status != STATUS_OK){
        UA_ReadEventDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReadEventDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReadEventDetails_EncodeableType =
{
    "ReadEventDetails",
    OpcUaId_ReadEventDetails,
    OpcUaId_ReadEventDetails_Encoding_DefaultBinary,
    OpcUaId_ReadEventDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReadEventDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_ReadEventDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReadEventDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReadEventDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReadEventDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReadEventDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadRawModifiedDetails
/*============================================================================
 * UA_ReadRawModifiedDetails_Initialize
 *===========================================================================*/
void UA_ReadRawModifiedDetails_Initialize(UA_ReadRawModifiedDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        Boolean_Initialize(&a_pValue->IsReadModified);
        DateTime_Initialize(&a_pValue->StartTime);
        DateTime_Initialize(&a_pValue->EndTime);
        UInt32_Initialize(&a_pValue->NumValuesPerNode);
        Boolean_Initialize(&a_pValue->ReturnBounds);
    }
}

/*============================================================================
 * UA_ReadRawModifiedDetails_Clear
 *===========================================================================*/
void UA_ReadRawModifiedDetails_Clear(UA_ReadRawModifiedDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        Boolean_Clear(&a_pValue->IsReadModified);
        DateTime_Clear(&a_pValue->StartTime);
        DateTime_Clear(&a_pValue->EndTime);
        UInt32_Clear(&a_pValue->NumValuesPerNode);
        Boolean_Clear(&a_pValue->ReturnBounds);
    }
}

/*============================================================================
 * UA_ReadRawModifiedDetails_Encode
 *===========================================================================*/
StatusCode UA_ReadRawModifiedDetails_Encode(UA_ReadRawModifiedDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Boolean_Write(&a_pValue->IsReadModified, msgBuf);
    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Write(&a_pValue->EndTime, msgBuf);
    status &= UInt32_Write(&a_pValue->NumValuesPerNode, msgBuf);
    status &= Boolean_Write(&a_pValue->ReturnBounds, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReadRawModifiedDetails_Decode
 *===========================================================================*/
StatusCode UA_ReadRawModifiedDetails_Decode(UA_ReadRawModifiedDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReadRawModifiedDetails_Initialize(a_pValue);

    status &= Boolean_Read(&a_pValue->IsReadModified, msgBuf);
    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->EndTime, msgBuf);
    status &= UInt32_Read(&a_pValue->NumValuesPerNode, msgBuf);
    status &= Boolean_Read(&a_pValue->ReturnBounds, msgBuf);

    if(status != STATUS_OK){
        UA_ReadRawModifiedDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReadRawModifiedDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReadRawModifiedDetails_EncodeableType =
{
    "ReadRawModifiedDetails",
    OpcUaId_ReadRawModifiedDetails,
    OpcUaId_ReadRawModifiedDetails_Encoding_DefaultBinary,
    OpcUaId_ReadRawModifiedDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReadRawModifiedDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_ReadRawModifiedDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReadRawModifiedDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReadRawModifiedDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReadRawModifiedDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReadRawModifiedDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadProcessedDetails
/*============================================================================
 * UA_ReadProcessedDetails_Initialize
 *===========================================================================*/
void UA_ReadProcessedDetails_Initialize(UA_ReadProcessedDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->StartTime);
        DateTime_Initialize(&a_pValue->EndTime);
        Double_Initialize(&a_pValue->ProcessingInterval);
        UA_Initialize_Array(&a_pValue->NoOfAggregateType, (void**) &a_pValue->AggregateType, 
                            sizeof(UA_NodeId), (UA_EncodeableObject_PfnInitialize*) NodeId_Initialize);
        UA_AggregateConfiguration_Initialize(&a_pValue->AggregateConfiguration);
    }
}

/*============================================================================
 * UA_ReadProcessedDetails_Clear
 *===========================================================================*/
void UA_ReadProcessedDetails_Clear(UA_ReadProcessedDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->StartTime);
        DateTime_Clear(&a_pValue->EndTime);
        Double_Clear(&a_pValue->ProcessingInterval);
        UA_Clear_Array(&a_pValue->NoOfAggregateType, (void**) &a_pValue->AggregateType, 
                       sizeof(UA_NodeId), (UA_EncodeableObject_PfnClear*) NodeId_Clear);
        UA_AggregateConfiguration_Clear(&a_pValue->AggregateConfiguration);
    }
}

/*============================================================================
 * UA_ReadProcessedDetails_Encode
 *===========================================================================*/
StatusCode UA_ReadProcessedDetails_Encode(UA_ReadProcessedDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Write(&a_pValue->EndTime, msgBuf);
    status &= Double_Write(&a_pValue->ProcessingInterval, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfAggregateType, (void**) &a_pValue->AggregateType, 
                   sizeof(UA_NodeId), (UA_EncodeableObject_PfnEncode*) NodeId_Write);
    status &= UA_AggregateConfiguration_Encode(&a_pValue->AggregateConfiguration, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReadProcessedDetails_Decode
 *===========================================================================*/
StatusCode UA_ReadProcessedDetails_Decode(UA_ReadProcessedDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReadProcessedDetails_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->EndTime, msgBuf);
    status &= Double_Read(&a_pValue->ProcessingInterval, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfAggregateType, (void**) &a_pValue->AggregateType, 
                  sizeof(UA_NodeId), (UA_EncodeableObject_PfnDecode*) NodeId_Read);
    status &= UA_AggregateConfiguration_Decode(&a_pValue->AggregateConfiguration, msgBuf);

    if(status != STATUS_OK){
        UA_ReadProcessedDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReadProcessedDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReadProcessedDetails_EncodeableType =
{
    "ReadProcessedDetails",
    OpcUaId_ReadProcessedDetails,
    OpcUaId_ReadProcessedDetails_Encoding_DefaultBinary,
    OpcUaId_ReadProcessedDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReadProcessedDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_ReadProcessedDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReadProcessedDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReadProcessedDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReadProcessedDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReadProcessedDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadAtTimeDetails
/*============================================================================
 * UA_ReadAtTimeDetails_Initialize
 *===========================================================================*/
void UA_ReadAtTimeDetails_Initialize(UA_ReadAtTimeDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                            sizeof(UA_DateTime), (UA_EncodeableObject_PfnInitialize*) DateTime_Initialize);
        Boolean_Initialize(&a_pValue->UseSimpleBounds);
    }
}

/*============================================================================
 * UA_ReadAtTimeDetails_Clear
 *===========================================================================*/
void UA_ReadAtTimeDetails_Clear(UA_ReadAtTimeDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                       sizeof(UA_DateTime), (UA_EncodeableObject_PfnClear*) DateTime_Clear);
        Boolean_Clear(&a_pValue->UseSimpleBounds);
    }
}

/*============================================================================
 * UA_ReadAtTimeDetails_Encode
 *===========================================================================*/
StatusCode UA_ReadAtTimeDetails_Encode(UA_ReadAtTimeDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                   sizeof(UA_DateTime), (UA_EncodeableObject_PfnEncode*) DateTime_Write);
    status &= Boolean_Write(&a_pValue->UseSimpleBounds, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ReadAtTimeDetails_Decode
 *===========================================================================*/
StatusCode UA_ReadAtTimeDetails_Decode(UA_ReadAtTimeDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ReadAtTimeDetails_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                  sizeof(UA_DateTime), (UA_EncodeableObject_PfnDecode*) DateTime_Read);
    status &= Boolean_Read(&a_pValue->UseSimpleBounds, msgBuf);

    if(status != STATUS_OK){
        UA_ReadAtTimeDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ReadAtTimeDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ReadAtTimeDetails_EncodeableType =
{
    "ReadAtTimeDetails",
    OpcUaId_ReadAtTimeDetails,
    OpcUaId_ReadAtTimeDetails_Encoding_DefaultBinary,
    OpcUaId_ReadAtTimeDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ReadAtTimeDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_ReadAtTimeDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ReadAtTimeDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ReadAtTimeDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ReadAtTimeDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ReadAtTimeDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryData
/*============================================================================
 * UA_HistoryData_Initialize
 *===========================================================================*/
void UA_HistoryData_Initialize(UA_HistoryData* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                            sizeof(UA_DataValue), (UA_EncodeableObject_PfnInitialize*) DataValue_Initialize);
    }
}

/*============================================================================
 * UA_HistoryData_Clear
 *===========================================================================*/
void UA_HistoryData_Clear(UA_HistoryData* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                       sizeof(UA_DataValue), (UA_EncodeableObject_PfnClear*) DataValue_Clear);
    }
}

/*============================================================================
 * UA_HistoryData_Encode
 *===========================================================================*/
StatusCode UA_HistoryData_Encode(UA_HistoryData* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                   sizeof(UA_DataValue), (UA_EncodeableObject_PfnEncode*) DataValue_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryData_Decode
 *===========================================================================*/
StatusCode UA_HistoryData_Decode(UA_HistoryData* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryData_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                  sizeof(UA_DataValue), (UA_EncodeableObject_PfnDecode*) DataValue_Read);

    if(status != STATUS_OK){
        UA_HistoryData_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryData_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryData_EncodeableType =
{
    "HistoryData",
    OpcUaId_HistoryData,
    OpcUaId_HistoryData_Encoding_DefaultBinary,
    OpcUaId_HistoryData_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryData),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryData_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryData_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryData_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryData_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryData_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ModificationInfo
/*============================================================================
 * UA_ModificationInfo_Initialize
 *===========================================================================*/
void UA_ModificationInfo_Initialize(UA_ModificationInfo* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->ModificationTime);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->UpdateType);
        String_Initialize(&a_pValue->UserName);
    }
}

/*============================================================================
 * UA_ModificationInfo_Clear
 *===========================================================================*/
void UA_ModificationInfo_Clear(UA_ModificationInfo* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->ModificationTime);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->UpdateType);
        String_Clear(&a_pValue->UserName);
    }
}

/*============================================================================
 * UA_ModificationInfo_Encode
 *===========================================================================*/
StatusCode UA_ModificationInfo_Encode(UA_ModificationInfo* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->ModificationTime, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->UpdateType);
    status &= String_Write(&a_pValue->UserName, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ModificationInfo_Decode
 *===========================================================================*/
StatusCode UA_ModificationInfo_Decode(UA_ModificationInfo* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ModificationInfo_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->ModificationTime, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->UpdateType);
    status &= String_Read(&a_pValue->UserName, msgBuf);

    if(status != STATUS_OK){
        UA_ModificationInfo_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ModificationInfo_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ModificationInfo_EncodeableType =
{
    "ModificationInfo",
    OpcUaId_ModificationInfo,
    OpcUaId_ModificationInfo_Encoding_DefaultBinary,
    OpcUaId_ModificationInfo_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ModificationInfo),
    (UA_EncodeableObject_PfnInitialize*)UA_ModificationInfo_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ModificationInfo_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ModificationInfo_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ModificationInfo_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ModificationInfo_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryModifiedData
/*============================================================================
 * UA_HistoryModifiedData_Initialize
 *===========================================================================*/
void UA_HistoryModifiedData_Initialize(UA_HistoryModifiedData* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                            sizeof(UA_DataValue), (UA_EncodeableObject_PfnInitialize*) DataValue_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfModificationInfos, (void**) &a_pValue->ModificationInfos, 
                            sizeof(UA_ModificationInfo), (UA_EncodeableObject_PfnInitialize*) UA_ModificationInfo_Initialize);
    }
}

/*============================================================================
 * UA_HistoryModifiedData_Clear
 *===========================================================================*/
void UA_HistoryModifiedData_Clear(UA_HistoryModifiedData* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                       sizeof(UA_DataValue), (UA_EncodeableObject_PfnClear*) DataValue_Clear);
        UA_Clear_Array(&a_pValue->NoOfModificationInfos, (void**) &a_pValue->ModificationInfos, 
                       sizeof(UA_ModificationInfo), (UA_EncodeableObject_PfnClear*) UA_ModificationInfo_Clear);
    }
}

/*============================================================================
 * UA_HistoryModifiedData_Encode
 *===========================================================================*/
StatusCode UA_HistoryModifiedData_Encode(UA_HistoryModifiedData* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                   sizeof(UA_DataValue), (UA_EncodeableObject_PfnEncode*) DataValue_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfModificationInfos, (void**) &a_pValue->ModificationInfos, 
                   sizeof(UA_ModificationInfo), (UA_EncodeableObject_PfnEncode*) UA_ModificationInfo_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryModifiedData_Decode
 *===========================================================================*/
StatusCode UA_HistoryModifiedData_Decode(UA_HistoryModifiedData* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryModifiedData_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                  sizeof(UA_DataValue), (UA_EncodeableObject_PfnDecode*) DataValue_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfModificationInfos, (void**) &a_pValue->ModificationInfos, 
                  sizeof(UA_ModificationInfo), (UA_EncodeableObject_PfnDecode*) UA_ModificationInfo_Decode);

    if(status != STATUS_OK){
        UA_HistoryModifiedData_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryModifiedData_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryModifiedData_EncodeableType =
{
    "HistoryModifiedData",
    OpcUaId_HistoryModifiedData,
    OpcUaId_HistoryModifiedData_Encoding_DefaultBinary,
    OpcUaId_HistoryModifiedData_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryModifiedData),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryModifiedData_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryModifiedData_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryModifiedData_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryModifiedData_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryModifiedData_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryEvent
/*============================================================================
 * UA_HistoryEvent_Initialize
 *===========================================================================*/
void UA_HistoryEvent_Initialize(UA_HistoryEvent* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                            sizeof(UA_HistoryEventFieldList), (UA_EncodeableObject_PfnInitialize*) UA_HistoryEventFieldList_Initialize);
    }
}

/*============================================================================
 * UA_HistoryEvent_Clear
 *===========================================================================*/
void UA_HistoryEvent_Clear(UA_HistoryEvent* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                       sizeof(UA_HistoryEventFieldList), (UA_EncodeableObject_PfnClear*) UA_HistoryEventFieldList_Clear);
    }
}

/*============================================================================
 * UA_HistoryEvent_Encode
 *===========================================================================*/
StatusCode UA_HistoryEvent_Encode(UA_HistoryEvent* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                   sizeof(UA_HistoryEventFieldList), (UA_EncodeableObject_PfnEncode*) UA_HistoryEventFieldList_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryEvent_Decode
 *===========================================================================*/
StatusCode UA_HistoryEvent_Decode(UA_HistoryEvent* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryEvent_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                  sizeof(UA_HistoryEventFieldList), (UA_EncodeableObject_PfnDecode*) UA_HistoryEventFieldList_Decode);

    if(status != STATUS_OK){
        UA_HistoryEvent_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryEvent_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryEvent_EncodeableType =
{
    "HistoryEvent",
    OpcUaId_HistoryEvent,
    OpcUaId_HistoryEvent_Encoding_DefaultBinary,
    OpcUaId_HistoryEvent_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryEvent),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryEvent_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryEvent_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryEvent_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryEvent_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryEvent_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
#ifndef OPCUA_EXCLUDE_HistoryReadRequest
/*============================================================================
 * UA_HistoryReadRequest_Initialize
 *===========================================================================*/
void UA_HistoryReadRequest_Initialize(UA_HistoryReadRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        ExtensionObject_Initialize(&a_pValue->HistoryReadDetails);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        Boolean_Initialize(&a_pValue->ReleaseContinuationPoints);
        UA_Initialize_Array(&a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                            sizeof(UA_HistoryReadValueId), (UA_EncodeableObject_PfnInitialize*) UA_HistoryReadValueId_Initialize);
    }
}

/*============================================================================
 * UA_HistoryReadRequest_Clear
 *===========================================================================*/
void UA_HistoryReadRequest_Clear(UA_HistoryReadRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        ExtensionObject_Clear(&a_pValue->HistoryReadDetails);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        Boolean_Clear(&a_pValue->ReleaseContinuationPoints);
        UA_Clear_Array(&a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                       sizeof(UA_HistoryReadValueId), (UA_EncodeableObject_PfnClear*) UA_HistoryReadValueId_Clear);
    }
}

/*============================================================================
 * UA_HistoryReadRequest_Encode
 *===========================================================================*/
StatusCode UA_HistoryReadRequest_Encode(UA_HistoryReadRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= ExtensionObject_Write(&a_pValue->HistoryReadDetails, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= Boolean_Write(&a_pValue->ReleaseContinuationPoints, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                   sizeof(UA_HistoryReadValueId), (UA_EncodeableObject_PfnEncode*) UA_HistoryReadValueId_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryReadRequest_Decode
 *===========================================================================*/
StatusCode UA_HistoryReadRequest_Decode(UA_HistoryReadRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryReadRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->HistoryReadDetails, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= Boolean_Read(&a_pValue->ReleaseContinuationPoints, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                  sizeof(UA_HistoryReadValueId), (UA_EncodeableObject_PfnDecode*) UA_HistoryReadValueId_Decode);

    if(status != STATUS_OK){
        UA_HistoryReadRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryReadRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryReadRequest_EncodeableType =
{
    "HistoryReadRequest",
    OpcUaId_HistoryReadRequest,
    OpcUaId_HistoryReadRequest_Encoding_DefaultBinary,
    OpcUaId_HistoryReadRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryReadRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryReadRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryReadRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryReadRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryReadRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryReadRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadResponse
/*============================================================================
 * UA_HistoryReadResponse_Initialize
 *===========================================================================*/
void UA_HistoryReadResponse_Initialize(UA_HistoryReadResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_HistoryReadResult), (UA_EncodeableObject_PfnInitialize*) UA_HistoryReadResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_HistoryReadResponse_Clear
 *===========================================================================*/
void UA_HistoryReadResponse_Clear(UA_HistoryReadResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_HistoryReadResult), (UA_EncodeableObject_PfnClear*) UA_HistoryReadResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_HistoryReadResponse_Encode
 *===========================================================================*/
StatusCode UA_HistoryReadResponse_Encode(UA_HistoryReadResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_HistoryReadResult), (UA_EncodeableObject_PfnEncode*) UA_HistoryReadResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryReadResponse_Decode
 *===========================================================================*/
StatusCode UA_HistoryReadResponse_Decode(UA_HistoryReadResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryReadResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_HistoryReadResult), (UA_EncodeableObject_PfnDecode*) UA_HistoryReadResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_HistoryReadResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryReadResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryReadResponse_EncodeableType =
{
    "HistoryReadResponse",
    OpcUaId_HistoryReadResponse,
    OpcUaId_HistoryReadResponse_Encoding_DefaultBinary,
    OpcUaId_HistoryReadResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryReadResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryReadResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryReadResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryReadResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryReadResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryReadResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_WriteValue
/*============================================================================
 * UA_WriteValue_Initialize
 *===========================================================================*/
void UA_WriteValue_Initialize(UA_WriteValue* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UInt32_Initialize(&a_pValue->AttributeId);
        String_Initialize(&a_pValue->IndexRange);
        DataValue_Initialize(&a_pValue->Value);
    }
}

/*============================================================================
 * UA_WriteValue_Clear
 *===========================================================================*/
void UA_WriteValue_Clear(UA_WriteValue* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UInt32_Clear(&a_pValue->AttributeId);
        String_Clear(&a_pValue->IndexRange);
        DataValue_Clear(&a_pValue->Value);
    }
}

/*============================================================================
 * UA_WriteValue_Encode
 *===========================================================================*/
StatusCode UA_WriteValue_Encode(UA_WriteValue* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UInt32_Write(&a_pValue->AttributeId, msgBuf);
    status &= String_Write(&a_pValue->IndexRange, msgBuf);
    status &= DataValue_Write(&a_pValue->Value, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_WriteValue_Decode
 *===========================================================================*/
StatusCode UA_WriteValue_Decode(UA_WriteValue* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_WriteValue_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);
    status &= DataValue_Read(&a_pValue->Value, msgBuf);

    if(status != STATUS_OK){
        UA_WriteValue_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_WriteValue_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_WriteValue_EncodeableType =
{
    "WriteValue",
    OpcUaId_WriteValue,
    OpcUaId_WriteValue_Encoding_DefaultBinary,
    OpcUaId_WriteValue_Encoding_DefaultXml,
    NULL,
    sizeof(UA_WriteValue),
    (UA_EncodeableObject_PfnInitialize*)UA_WriteValue_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_WriteValue_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_WriteValue_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_WriteValue_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_WriteValue_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Write
#ifndef OPCUA_EXCLUDE_WriteRequest
/*============================================================================
 * UA_WriteRequest_Initialize
 *===========================================================================*/
void UA_WriteRequest_Initialize(UA_WriteRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfNodesToWrite, (void**) &a_pValue->NodesToWrite, 
                            sizeof(UA_WriteValue), (UA_EncodeableObject_PfnInitialize*) UA_WriteValue_Initialize);
    }
}

/*============================================================================
 * UA_WriteRequest_Clear
 *===========================================================================*/
void UA_WriteRequest_Clear(UA_WriteRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfNodesToWrite, (void**) &a_pValue->NodesToWrite, 
                       sizeof(UA_WriteValue), (UA_EncodeableObject_PfnClear*) UA_WriteValue_Clear);
    }
}

/*============================================================================
 * UA_WriteRequest_Encode
 *===========================================================================*/
StatusCode UA_WriteRequest_Encode(UA_WriteRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNodesToWrite, (void**) &a_pValue->NodesToWrite, 
                   sizeof(UA_WriteValue), (UA_EncodeableObject_PfnEncode*) UA_WriteValue_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_WriteRequest_Decode
 *===========================================================================*/
StatusCode UA_WriteRequest_Decode(UA_WriteRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_WriteRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNodesToWrite, (void**) &a_pValue->NodesToWrite, 
                  sizeof(UA_WriteValue), (UA_EncodeableObject_PfnDecode*) UA_WriteValue_Decode);

    if(status != STATUS_OK){
        UA_WriteRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_WriteRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_WriteRequest_EncodeableType =
{
    "WriteRequest",
    OpcUaId_WriteRequest,
    OpcUaId_WriteRequest_Encoding_DefaultBinary,
    OpcUaId_WriteRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_WriteRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_WriteRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_WriteRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_WriteRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_WriteRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_WriteRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_WriteResponse
/*============================================================================
 * UA_WriteResponse_Initialize
 *===========================================================================*/
void UA_WriteResponse_Initialize(UA_WriteResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_WriteResponse_Clear
 *===========================================================================*/
void UA_WriteResponse_Clear(UA_WriteResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_WriteResponse_Encode
 *===========================================================================*/
StatusCode UA_WriteResponse_Encode(UA_WriteResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_WriteResponse_Decode
 *===========================================================================*/
StatusCode UA_WriteResponse_Decode(UA_WriteResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_WriteResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_WriteResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_WriteResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_WriteResponse_EncodeableType =
{
    "WriteResponse",
    OpcUaId_WriteResponse,
    OpcUaId_WriteResponse_Encoding_DefaultBinary,
    OpcUaId_WriteResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_WriteResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_WriteResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_WriteResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_WriteResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_WriteResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_WriteResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateDetails
/*============================================================================
 * UA_HistoryUpdateDetails_Initialize
 *===========================================================================*/
void UA_HistoryUpdateDetails_Initialize(UA_HistoryUpdateDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
    }
}

/*============================================================================
 * UA_HistoryUpdateDetails_Clear
 *===========================================================================*/
void UA_HistoryUpdateDetails_Clear(UA_HistoryUpdateDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
    }
}

/*============================================================================
 * UA_HistoryUpdateDetails_Encode
 *===========================================================================*/
StatusCode UA_HistoryUpdateDetails_Encode(UA_HistoryUpdateDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryUpdateDetails_Decode
 *===========================================================================*/
StatusCode UA_HistoryUpdateDetails_Decode(UA_HistoryUpdateDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryUpdateDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);

    if(status != STATUS_OK){
        UA_HistoryUpdateDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryUpdateDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryUpdateDetails_EncodeableType =
{
    "HistoryUpdateDetails",
    OpcUaId_HistoryUpdateDetails,
    OpcUaId_HistoryUpdateDetails_Encoding_DefaultBinary,
    OpcUaId_HistoryUpdateDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryUpdateDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryUpdateDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryUpdateDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryUpdateDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryUpdateDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryUpdateDetails_Decode
};
#endif



#ifndef OPCUA_EXCLUDE_UpdateDataDetails
/*============================================================================
 * UA_UpdateDataDetails_Initialize
 *===========================================================================*/
void UA_UpdateDataDetails_Initialize(UA_UpdateDataDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        UA_Initialize_Array(&a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                            sizeof(UA_DataValue), (UA_EncodeableObject_PfnInitialize*) DataValue_Initialize);
    }
}

/*============================================================================
 * UA_UpdateDataDetails_Clear
 *===========================================================================*/
void UA_UpdateDataDetails_Clear(UA_UpdateDataDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        UA_Clear_Array(&a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                       sizeof(UA_DataValue), (UA_EncodeableObject_PfnClear*) DataValue_Clear);
    }
}

/*============================================================================
 * UA_UpdateDataDetails_Encode
 *===========================================================================*/
StatusCode UA_UpdateDataDetails_Encode(UA_UpdateDataDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                   sizeof(UA_DataValue), (UA_EncodeableObject_PfnEncode*) DataValue_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_UpdateDataDetails_Decode
 *===========================================================================*/
StatusCode UA_UpdateDataDetails_Decode(UA_UpdateDataDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_UpdateDataDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                  sizeof(UA_DataValue), (UA_EncodeableObject_PfnDecode*) DataValue_Read);

    if(status != STATUS_OK){
        UA_UpdateDataDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_UpdateDataDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_UpdateDataDetails_EncodeableType =
{
    "UpdateDataDetails",
    OpcUaId_UpdateDataDetails,
    OpcUaId_UpdateDataDetails_Encoding_DefaultBinary,
    OpcUaId_UpdateDataDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_UpdateDataDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_UpdateDataDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_UpdateDataDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_UpdateDataDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_UpdateDataDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_UpdateDataDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_UpdateStructureDataDetails
/*============================================================================
 * UA_UpdateStructureDataDetails_Initialize
 *===========================================================================*/
void UA_UpdateStructureDataDetails_Initialize(UA_UpdateStructureDataDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        UA_Initialize_Array(&a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                            sizeof(UA_DataValue), (UA_EncodeableObject_PfnInitialize*) DataValue_Initialize);
    }
}

/*============================================================================
 * UA_UpdateStructureDataDetails_Clear
 *===========================================================================*/
void UA_UpdateStructureDataDetails_Clear(UA_UpdateStructureDataDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        UA_Clear_Array(&a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                       sizeof(UA_DataValue), (UA_EncodeableObject_PfnClear*) DataValue_Clear);
    }
}

/*============================================================================
 * UA_UpdateStructureDataDetails_Encode
 *===========================================================================*/
StatusCode UA_UpdateStructureDataDetails_Encode(UA_UpdateStructureDataDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                   sizeof(UA_DataValue), (UA_EncodeableObject_PfnEncode*) DataValue_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_UpdateStructureDataDetails_Decode
 *===========================================================================*/
StatusCode UA_UpdateStructureDataDetails_Decode(UA_UpdateStructureDataDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_UpdateStructureDataDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                  sizeof(UA_DataValue), (UA_EncodeableObject_PfnDecode*) DataValue_Read);

    if(status != STATUS_OK){
        UA_UpdateStructureDataDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_UpdateStructureDataDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_UpdateStructureDataDetails_EncodeableType =
{
    "UpdateStructureDataDetails",
    OpcUaId_UpdateStructureDataDetails,
    OpcUaId_UpdateStructureDataDetails_Encoding_DefaultBinary,
    OpcUaId_UpdateStructureDataDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_UpdateStructureDataDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_UpdateStructureDataDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_UpdateStructureDataDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_UpdateStructureDataDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_UpdateStructureDataDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_UpdateStructureDataDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_UpdateEventDetails
/*============================================================================
 * UA_UpdateEventDetails_Initialize
 *===========================================================================*/
void UA_UpdateEventDetails_Initialize(UA_UpdateEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        UA_EventFilter_Initialize(&a_pValue->Filter);
        UA_Initialize_Array(&a_pValue->NoOfEventData, (void**) &a_pValue->EventData, 
                            sizeof(UA_HistoryEventFieldList), (UA_EncodeableObject_PfnInitialize*) UA_HistoryEventFieldList_Initialize);
    }
}

/*============================================================================
 * UA_UpdateEventDetails_Clear
 *===========================================================================*/
void UA_UpdateEventDetails_Clear(UA_UpdateEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        UA_EventFilter_Clear(&a_pValue->Filter);
        UA_Clear_Array(&a_pValue->NoOfEventData, (void**) &a_pValue->EventData, 
                       sizeof(UA_HistoryEventFieldList), (UA_EncodeableObject_PfnClear*) UA_HistoryEventFieldList_Clear);
    }
}

/*============================================================================
 * UA_UpdateEventDetails_Encode
 *===========================================================================*/
StatusCode UA_UpdateEventDetails_Encode(UA_UpdateEventDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= UA_EventFilter_Encode(&a_pValue->Filter, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfEventData, (void**) &a_pValue->EventData, 
                   sizeof(UA_HistoryEventFieldList), (UA_EncodeableObject_PfnEncode*) UA_HistoryEventFieldList_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_UpdateEventDetails_Decode
 *===========================================================================*/
StatusCode UA_UpdateEventDetails_Decode(UA_UpdateEventDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_UpdateEventDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= UA_EventFilter_Decode(&a_pValue->Filter, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfEventData, (void**) &a_pValue->EventData, 
                  sizeof(UA_HistoryEventFieldList), (UA_EncodeableObject_PfnDecode*) UA_HistoryEventFieldList_Decode);

    if(status != STATUS_OK){
        UA_UpdateEventDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_UpdateEventDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_UpdateEventDetails_EncodeableType =
{
    "UpdateEventDetails",
    OpcUaId_UpdateEventDetails,
    OpcUaId_UpdateEventDetails_Encoding_DefaultBinary,
    OpcUaId_UpdateEventDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_UpdateEventDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_UpdateEventDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_UpdateEventDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_UpdateEventDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_UpdateEventDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_UpdateEventDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteRawModifiedDetails
/*============================================================================
 * UA_DeleteRawModifiedDetails_Initialize
 *===========================================================================*/
void UA_DeleteRawModifiedDetails_Initialize(UA_DeleteRawModifiedDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        Boolean_Initialize(&a_pValue->IsDeleteModified);
        DateTime_Initialize(&a_pValue->StartTime);
        DateTime_Initialize(&a_pValue->EndTime);
    }
}

/*============================================================================
 * UA_DeleteRawModifiedDetails_Clear
 *===========================================================================*/
void UA_DeleteRawModifiedDetails_Clear(UA_DeleteRawModifiedDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        Boolean_Clear(&a_pValue->IsDeleteModified);
        DateTime_Clear(&a_pValue->StartTime);
        DateTime_Clear(&a_pValue->EndTime);
    }
}

/*============================================================================
 * UA_DeleteRawModifiedDetails_Encode
 *===========================================================================*/
StatusCode UA_DeleteRawModifiedDetails_Encode(UA_DeleteRawModifiedDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsDeleteModified, msgBuf);
    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Write(&a_pValue->EndTime, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteRawModifiedDetails_Decode
 *===========================================================================*/
StatusCode UA_DeleteRawModifiedDetails_Decode(UA_DeleteRawModifiedDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteRawModifiedDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsDeleteModified, msgBuf);
    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->EndTime, msgBuf);

    if(status != STATUS_OK){
        UA_DeleteRawModifiedDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteRawModifiedDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteRawModifiedDetails_EncodeableType =
{
    "DeleteRawModifiedDetails",
    OpcUaId_DeleteRawModifiedDetails,
    OpcUaId_DeleteRawModifiedDetails_Encoding_DefaultBinary,
    OpcUaId_DeleteRawModifiedDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteRawModifiedDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteRawModifiedDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteRawModifiedDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteRawModifiedDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteRawModifiedDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteRawModifiedDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteAtTimeDetails
/*============================================================================
 * UA_DeleteAtTimeDetails_Initialize
 *===========================================================================*/
void UA_DeleteAtTimeDetails_Initialize(UA_DeleteAtTimeDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_Array(&a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                            sizeof(UA_DateTime), (UA_EncodeableObject_PfnInitialize*) DateTime_Initialize);
    }
}

/*============================================================================
 * UA_DeleteAtTimeDetails_Clear
 *===========================================================================*/
void UA_DeleteAtTimeDetails_Clear(UA_DeleteAtTimeDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_Array(&a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                       sizeof(UA_DateTime), (UA_EncodeableObject_PfnClear*) DateTime_Clear);
    }
}

/*============================================================================
 * UA_DeleteAtTimeDetails_Encode
 *===========================================================================*/
StatusCode UA_DeleteAtTimeDetails_Encode(UA_DeleteAtTimeDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                   sizeof(UA_DateTime), (UA_EncodeableObject_PfnEncode*) DateTime_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteAtTimeDetails_Decode
 *===========================================================================*/
StatusCode UA_DeleteAtTimeDetails_Decode(UA_DeleteAtTimeDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteAtTimeDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                  sizeof(UA_DateTime), (UA_EncodeableObject_PfnDecode*) DateTime_Read);

    if(status != STATUS_OK){
        UA_DeleteAtTimeDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteAtTimeDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteAtTimeDetails_EncodeableType =
{
    "DeleteAtTimeDetails",
    OpcUaId_DeleteAtTimeDetails,
    OpcUaId_DeleteAtTimeDetails_Encoding_DefaultBinary,
    OpcUaId_DeleteAtTimeDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteAtTimeDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteAtTimeDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteAtTimeDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteAtTimeDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteAtTimeDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteAtTimeDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteEventDetails
/*============================================================================
 * UA_DeleteEventDetails_Initialize
 *===========================================================================*/
void UA_DeleteEventDetails_Initialize(UA_DeleteEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        UA_Initialize_Array(&a_pValue->NoOfEventIds, (void**) &a_pValue->EventIds, 
                            sizeof(UA_ByteString), (UA_EncodeableObject_PfnInitialize*) ByteString_Initialize);
    }
}

/*============================================================================
 * UA_DeleteEventDetails_Clear
 *===========================================================================*/
void UA_DeleteEventDetails_Clear(UA_DeleteEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        UA_Clear_Array(&a_pValue->NoOfEventIds, (void**) &a_pValue->EventIds, 
                       sizeof(UA_ByteString), (UA_EncodeableObject_PfnClear*) ByteString_Clear);
    }
}

/*============================================================================
 * UA_DeleteEventDetails_Encode
 *===========================================================================*/
StatusCode UA_DeleteEventDetails_Encode(UA_DeleteEventDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfEventIds, (void**) &a_pValue->EventIds, 
                   sizeof(UA_ByteString), (UA_EncodeableObject_PfnEncode*) ByteString_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteEventDetails_Decode
 *===========================================================================*/
StatusCode UA_DeleteEventDetails_Decode(UA_DeleteEventDetails* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteEventDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfEventIds, (void**) &a_pValue->EventIds, 
                  sizeof(UA_ByteString), (UA_EncodeableObject_PfnDecode*) ByteString_Read);

    if(status != STATUS_OK){
        UA_DeleteEventDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteEventDetails_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteEventDetails_EncodeableType =
{
    "DeleteEventDetails",
    OpcUaId_DeleteEventDetails,
    OpcUaId_DeleteEventDetails_Encoding_DefaultBinary,
    OpcUaId_DeleteEventDetails_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteEventDetails),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteEventDetails_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteEventDetails_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteEventDetails_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteEventDetails_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteEventDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateResult
/*============================================================================
 * UA_HistoryUpdateResult_Initialize
 *===========================================================================*/
void UA_HistoryUpdateResult_Initialize(UA_HistoryUpdateResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        UA_Initialize_Array(&a_pValue->NoOfOperationResults, (void**) &a_pValue->OperationResults, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_HistoryUpdateResult_Clear
 *===========================================================================*/
void UA_HistoryUpdateResult_Clear(UA_HistoryUpdateResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        UA_Clear_Array(&a_pValue->NoOfOperationResults, (void**) &a_pValue->OperationResults, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_HistoryUpdateResult_Encode
 *===========================================================================*/
StatusCode UA_HistoryUpdateResult_Encode(UA_HistoryUpdateResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfOperationResults, (void**) &a_pValue->OperationResults, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryUpdateResult_Decode
 *===========================================================================*/
StatusCode UA_HistoryUpdateResult_Decode(UA_HistoryUpdateResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryUpdateResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfOperationResults, (void**) &a_pValue->OperationResults, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_HistoryUpdateResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryUpdateResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryUpdateResult_EncodeableType =
{
    "HistoryUpdateResult",
    OpcUaId_HistoryUpdateResult,
    OpcUaId_HistoryUpdateResult_Encoding_DefaultBinary,
    OpcUaId_HistoryUpdateResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryUpdateResult),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryUpdateResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryUpdateResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryUpdateResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryUpdateResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryUpdateResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
#ifndef OPCUA_EXCLUDE_HistoryUpdateRequest
/*============================================================================
 * UA_HistoryUpdateRequest_Initialize
 *===========================================================================*/
void UA_HistoryUpdateRequest_Initialize(UA_HistoryUpdateRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfHistoryUpdateDetails, (void**) &a_pValue->HistoryUpdateDetails, 
                            sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnInitialize*) ExtensionObject_Initialize);
    }
}

/*============================================================================
 * UA_HistoryUpdateRequest_Clear
 *===========================================================================*/
void UA_HistoryUpdateRequest_Clear(UA_HistoryUpdateRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfHistoryUpdateDetails, (void**) &a_pValue->HistoryUpdateDetails, 
                       sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnClear*) ExtensionObject_Clear);
    }
}

/*============================================================================
 * UA_HistoryUpdateRequest_Encode
 *===========================================================================*/
StatusCode UA_HistoryUpdateRequest_Encode(UA_HistoryUpdateRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfHistoryUpdateDetails, (void**) &a_pValue->HistoryUpdateDetails, 
                   sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnEncode*) ExtensionObject_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryUpdateRequest_Decode
 *===========================================================================*/
StatusCode UA_HistoryUpdateRequest_Decode(UA_HistoryUpdateRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryUpdateRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfHistoryUpdateDetails, (void**) &a_pValue->HistoryUpdateDetails, 
                  sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnDecode*) ExtensionObject_Read);

    if(status != STATUS_OK){
        UA_HistoryUpdateRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryUpdateRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryUpdateRequest_EncodeableType =
{
    "HistoryUpdateRequest",
    OpcUaId_HistoryUpdateRequest,
    OpcUaId_HistoryUpdateRequest_Encoding_DefaultBinary,
    OpcUaId_HistoryUpdateRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryUpdateRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryUpdateRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryUpdateRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryUpdateRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryUpdateRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryUpdateRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateResponse
/*============================================================================
 * UA_HistoryUpdateResponse_Initialize
 *===========================================================================*/
void UA_HistoryUpdateResponse_Initialize(UA_HistoryUpdateResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_HistoryUpdateResult), (UA_EncodeableObject_PfnInitialize*) UA_HistoryUpdateResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_HistoryUpdateResponse_Clear
 *===========================================================================*/
void UA_HistoryUpdateResponse_Clear(UA_HistoryUpdateResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_HistoryUpdateResult), (UA_EncodeableObject_PfnClear*) UA_HistoryUpdateResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_HistoryUpdateResponse_Encode
 *===========================================================================*/
StatusCode UA_HistoryUpdateResponse_Encode(UA_HistoryUpdateResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_HistoryUpdateResult), (UA_EncodeableObject_PfnEncode*) UA_HistoryUpdateResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryUpdateResponse_Decode
 *===========================================================================*/
StatusCode UA_HistoryUpdateResponse_Decode(UA_HistoryUpdateResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryUpdateResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_HistoryUpdateResult), (UA_EncodeableObject_PfnDecode*) UA_HistoryUpdateResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_HistoryUpdateResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryUpdateResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryUpdateResponse_EncodeableType =
{
    "HistoryUpdateResponse",
    OpcUaId_HistoryUpdateResponse,
    OpcUaId_HistoryUpdateResponse_Encoding_DefaultBinary,
    OpcUaId_HistoryUpdateResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryUpdateResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryUpdateResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryUpdateResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryUpdateResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryUpdateResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryUpdateResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_CallMethodRequest
/*============================================================================
 * UA_CallMethodRequest_Initialize
 *===========================================================================*/
void UA_CallMethodRequest_Initialize(UA_CallMethodRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->ObjectId);
        NodeId_Initialize(&a_pValue->MethodId);
        UA_Initialize_Array(&a_pValue->NoOfInputArguments, (void**) &a_pValue->InputArguments, 
                            sizeof(UA_Variant), (UA_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * UA_CallMethodRequest_Clear
 *===========================================================================*/
void UA_CallMethodRequest_Clear(UA_CallMethodRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->ObjectId);
        NodeId_Clear(&a_pValue->MethodId);
        UA_Clear_Array(&a_pValue->NoOfInputArguments, (void**) &a_pValue->InputArguments, 
                       sizeof(UA_Variant), (UA_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * UA_CallMethodRequest_Encode
 *===========================================================================*/
StatusCode UA_CallMethodRequest_Encode(UA_CallMethodRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->ObjectId, msgBuf);
    status &= NodeId_Write(&a_pValue->MethodId, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfInputArguments, (void**) &a_pValue->InputArguments, 
                   sizeof(UA_Variant), (UA_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CallMethodRequest_Decode
 *===========================================================================*/
StatusCode UA_CallMethodRequest_Decode(UA_CallMethodRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CallMethodRequest_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ObjectId, msgBuf);
    status &= NodeId_Read(&a_pValue->MethodId, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfInputArguments, (void**) &a_pValue->InputArguments, 
                  sizeof(UA_Variant), (UA_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        UA_CallMethodRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CallMethodRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CallMethodRequest_EncodeableType =
{
    "CallMethodRequest",
    OpcUaId_CallMethodRequest,
    OpcUaId_CallMethodRequest_Encoding_DefaultBinary,
    OpcUaId_CallMethodRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CallMethodRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_CallMethodRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CallMethodRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CallMethodRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CallMethodRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CallMethodRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CallMethodResult
/*============================================================================
 * UA_CallMethodResult_Initialize
 *===========================================================================*/
void UA_CallMethodResult_Initialize(UA_CallMethodResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        UA_Initialize_Array(&a_pValue->NoOfInputArgumentResults, (void**) &a_pValue->InputArgumentResults, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfInputArgumentDiagnosticInfos, (void**) &a_pValue->InputArgumentDiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfOutputArguments, (void**) &a_pValue->OutputArguments, 
                            sizeof(UA_Variant), (UA_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * UA_CallMethodResult_Clear
 *===========================================================================*/
void UA_CallMethodResult_Clear(UA_CallMethodResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        UA_Clear_Array(&a_pValue->NoOfInputArgumentResults, (void**) &a_pValue->InputArgumentResults, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfInputArgumentDiagnosticInfos, (void**) &a_pValue->InputArgumentDiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
        UA_Clear_Array(&a_pValue->NoOfOutputArguments, (void**) &a_pValue->OutputArguments, 
                       sizeof(UA_Variant), (UA_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * UA_CallMethodResult_Encode
 *===========================================================================*/
StatusCode UA_CallMethodResult_Encode(UA_CallMethodResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfInputArgumentResults, (void**) &a_pValue->InputArgumentResults, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfInputArgumentDiagnosticInfos, (void**) &a_pValue->InputArgumentDiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfOutputArguments, (void**) &a_pValue->OutputArguments, 
                   sizeof(UA_Variant), (UA_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CallMethodResult_Decode
 *===========================================================================*/
StatusCode UA_CallMethodResult_Decode(UA_CallMethodResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CallMethodResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfInputArgumentResults, (void**) &a_pValue->InputArgumentResults, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfInputArgumentDiagnosticInfos, (void**) &a_pValue->InputArgumentDiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfOutputArguments, (void**) &a_pValue->OutputArguments, 
                  sizeof(UA_Variant), (UA_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        UA_CallMethodResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CallMethodResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CallMethodResult_EncodeableType =
{
    "CallMethodResult",
    OpcUaId_CallMethodResult,
    OpcUaId_CallMethodResult_Encoding_DefaultBinary,
    OpcUaId_CallMethodResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CallMethodResult),
    (UA_EncodeableObject_PfnInitialize*)UA_CallMethodResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CallMethodResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CallMethodResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CallMethodResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CallMethodResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Call
#ifndef OPCUA_EXCLUDE_CallRequest
/*============================================================================
 * UA_CallRequest_Initialize
 *===========================================================================*/
void UA_CallRequest_Initialize(UA_CallRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfMethodsToCall, (void**) &a_pValue->MethodsToCall, 
                            sizeof(UA_CallMethodRequest), (UA_EncodeableObject_PfnInitialize*) UA_CallMethodRequest_Initialize);
    }
}

/*============================================================================
 * UA_CallRequest_Clear
 *===========================================================================*/
void UA_CallRequest_Clear(UA_CallRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfMethodsToCall, (void**) &a_pValue->MethodsToCall, 
                       sizeof(UA_CallMethodRequest), (UA_EncodeableObject_PfnClear*) UA_CallMethodRequest_Clear);
    }
}

/*============================================================================
 * UA_CallRequest_Encode
 *===========================================================================*/
StatusCode UA_CallRequest_Encode(UA_CallRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfMethodsToCall, (void**) &a_pValue->MethodsToCall, 
                   sizeof(UA_CallMethodRequest), (UA_EncodeableObject_PfnEncode*) UA_CallMethodRequest_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CallRequest_Decode
 *===========================================================================*/
StatusCode UA_CallRequest_Decode(UA_CallRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CallRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfMethodsToCall, (void**) &a_pValue->MethodsToCall, 
                  sizeof(UA_CallMethodRequest), (UA_EncodeableObject_PfnDecode*) UA_CallMethodRequest_Decode);

    if(status != STATUS_OK){
        UA_CallRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CallRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CallRequest_EncodeableType =
{
    "CallRequest",
    OpcUaId_CallRequest,
    OpcUaId_CallRequest_Encoding_DefaultBinary,
    OpcUaId_CallRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CallRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_CallRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CallRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CallRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CallRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CallRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CallResponse
/*============================================================================
 * UA_CallResponse_Initialize
 *===========================================================================*/
void UA_CallResponse_Initialize(UA_CallResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_CallMethodResult), (UA_EncodeableObject_PfnInitialize*) UA_CallMethodResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_CallResponse_Clear
 *===========================================================================*/
void UA_CallResponse_Clear(UA_CallResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_CallMethodResult), (UA_EncodeableObject_PfnClear*) UA_CallMethodResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_CallResponse_Encode
 *===========================================================================*/
StatusCode UA_CallResponse_Encode(UA_CallResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_CallMethodResult), (UA_EncodeableObject_PfnEncode*) UA_CallMethodResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CallResponse_Decode
 *===========================================================================*/
StatusCode UA_CallResponse_Decode(UA_CallResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CallResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_CallMethodResult), (UA_EncodeableObject_PfnDecode*) UA_CallMethodResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_CallResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CallResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CallResponse_EncodeableType =
{
    "CallResponse",
    OpcUaId_CallResponse,
    OpcUaId_CallResponse_Encoding_DefaultBinary,
    OpcUaId_CallResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CallResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_CallResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CallResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CallResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CallResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CallResponse_Decode
};
#endif
#endif




#ifndef OPCUA_EXCLUDE_DataChangeFilter
/*============================================================================
 * UA_DataChangeFilter_Initialize
 *===========================================================================*/
void UA_DataChangeFilter_Initialize(UA_DataChangeFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->Trigger);
        UInt32_Initialize(&a_pValue->DeadbandType);
        Double_Initialize(&a_pValue->DeadbandValue);
    }
}

/*============================================================================
 * UA_DataChangeFilter_Clear
 *===========================================================================*/
void UA_DataChangeFilter_Clear(UA_DataChangeFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->Trigger);
        UInt32_Clear(&a_pValue->DeadbandType);
        Double_Clear(&a_pValue->DeadbandValue);
    }
}

/*============================================================================
 * UA_DataChangeFilter_Encode
 *===========================================================================*/
StatusCode UA_DataChangeFilter_Encode(UA_DataChangeFilter* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->Trigger);
    status &= UInt32_Write(&a_pValue->DeadbandType, msgBuf);
    status &= Double_Write(&a_pValue->DeadbandValue, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DataChangeFilter_Decode
 *===========================================================================*/
StatusCode UA_DataChangeFilter_Decode(UA_DataChangeFilter* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DataChangeFilter_Initialize(a_pValue);

    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->Trigger);
    status &= UInt32_Read(&a_pValue->DeadbandType, msgBuf);
    status &= Double_Read(&a_pValue->DeadbandValue, msgBuf);

    if(status != STATUS_OK){
        UA_DataChangeFilter_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DataChangeFilter_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DataChangeFilter_EncodeableType =
{
    "DataChangeFilter",
    OpcUaId_DataChangeFilter,
    OpcUaId_DataChangeFilter_Encoding_DefaultBinary,
    OpcUaId_DataChangeFilter_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DataChangeFilter),
    (UA_EncodeableObject_PfnInitialize*)UA_DataChangeFilter_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DataChangeFilter_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DataChangeFilter_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DataChangeFilter_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DataChangeFilter_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EventFilter
/*============================================================================
 * UA_EventFilter_Initialize
 *===========================================================================*/
void UA_EventFilter_Initialize(UA_EventFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfSelectClauses, (void**) &a_pValue->SelectClauses, 
                            sizeof(UA_SimpleAttributeOperand), (UA_EncodeableObject_PfnInitialize*) UA_SimpleAttributeOperand_Initialize);
        UA_ContentFilter_Initialize(&a_pValue->WhereClause);
    }
}

/*============================================================================
 * UA_EventFilter_Clear
 *===========================================================================*/
void UA_EventFilter_Clear(UA_EventFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfSelectClauses, (void**) &a_pValue->SelectClauses, 
                       sizeof(UA_SimpleAttributeOperand), (UA_EncodeableObject_PfnClear*) UA_SimpleAttributeOperand_Clear);
        UA_ContentFilter_Clear(&a_pValue->WhereClause);
    }
}

/*============================================================================
 * UA_EventFilter_Encode
 *===========================================================================*/
StatusCode UA_EventFilter_Encode(UA_EventFilter* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfSelectClauses, (void**) &a_pValue->SelectClauses, 
                   sizeof(UA_SimpleAttributeOperand), (UA_EncodeableObject_PfnEncode*) UA_SimpleAttributeOperand_Encode);
    status &= UA_ContentFilter_Encode(&a_pValue->WhereClause, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EventFilter_Decode
 *===========================================================================*/
StatusCode UA_EventFilter_Decode(UA_EventFilter* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EventFilter_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfSelectClauses, (void**) &a_pValue->SelectClauses, 
                  sizeof(UA_SimpleAttributeOperand), (UA_EncodeableObject_PfnDecode*) UA_SimpleAttributeOperand_Decode);
    status &= UA_ContentFilter_Decode(&a_pValue->WhereClause, msgBuf);

    if(status != STATUS_OK){
        UA_EventFilter_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EventFilter_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EventFilter_EncodeableType =
{
    "EventFilter",
    OpcUaId_EventFilter,
    OpcUaId_EventFilter_Encoding_DefaultBinary,
    OpcUaId_EventFilter_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EventFilter),
    (UA_EncodeableObject_PfnInitialize*)UA_EventFilter_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EventFilter_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EventFilter_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EventFilter_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EventFilter_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AggregateConfiguration
/*============================================================================
 * UA_AggregateConfiguration_Initialize
 *===========================================================================*/
void UA_AggregateConfiguration_Initialize(UA_AggregateConfiguration* a_pValue)
{
    if (a_pValue != NULL)
    {
        Boolean_Initialize(&a_pValue->UseServerCapabilitiesDefaults);
        Boolean_Initialize(&a_pValue->TreatUncertainAsBad);
        Byte_Initialize(&a_pValue->PercentDataBad);
        Byte_Initialize(&a_pValue->PercentDataGood);
        Boolean_Initialize(&a_pValue->UseSlopedExtrapolation);
    }
}

/*============================================================================
 * UA_AggregateConfiguration_Clear
 *===========================================================================*/
void UA_AggregateConfiguration_Clear(UA_AggregateConfiguration* a_pValue)
{
    if (a_pValue != NULL)
    {
        Boolean_Clear(&a_pValue->UseServerCapabilitiesDefaults);
        Boolean_Clear(&a_pValue->TreatUncertainAsBad);
        Byte_Clear(&a_pValue->PercentDataBad);
        Byte_Clear(&a_pValue->PercentDataGood);
        Boolean_Clear(&a_pValue->UseSlopedExtrapolation);
    }
}

/*============================================================================
 * UA_AggregateConfiguration_Encode
 *===========================================================================*/
StatusCode UA_AggregateConfiguration_Encode(UA_AggregateConfiguration* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Boolean_Write(&a_pValue->UseServerCapabilitiesDefaults, msgBuf);
    status &= Boolean_Write(&a_pValue->TreatUncertainAsBad, msgBuf);
    status &= Byte_Write(&a_pValue->PercentDataBad, msgBuf);
    status &= Byte_Write(&a_pValue->PercentDataGood, msgBuf);
    status &= Boolean_Write(&a_pValue->UseSlopedExtrapolation, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AggregateConfiguration_Decode
 *===========================================================================*/
StatusCode UA_AggregateConfiguration_Decode(UA_AggregateConfiguration* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AggregateConfiguration_Initialize(a_pValue);

    status &= Boolean_Read(&a_pValue->UseServerCapabilitiesDefaults, msgBuf);
    status &= Boolean_Read(&a_pValue->TreatUncertainAsBad, msgBuf);
    status &= Byte_Read(&a_pValue->PercentDataBad, msgBuf);
    status &= Byte_Read(&a_pValue->PercentDataGood, msgBuf);
    status &= Boolean_Read(&a_pValue->UseSlopedExtrapolation, msgBuf);

    if(status != STATUS_OK){
        UA_AggregateConfiguration_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AggregateConfiguration_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AggregateConfiguration_EncodeableType =
{
    "AggregateConfiguration",
    OpcUaId_AggregateConfiguration,
    OpcUaId_AggregateConfiguration_Encoding_DefaultBinary,
    OpcUaId_AggregateConfiguration_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AggregateConfiguration),
    (UA_EncodeableObject_PfnInitialize*)UA_AggregateConfiguration_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AggregateConfiguration_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AggregateConfiguration_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AggregateConfiguration_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AggregateConfiguration_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilter
/*============================================================================
 * UA_AggregateFilter_Initialize
 *===========================================================================*/
void UA_AggregateFilter_Initialize(UA_AggregateFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->StartTime);
        NodeId_Initialize(&a_pValue->AggregateType);
        Double_Initialize(&a_pValue->ProcessingInterval);
        UA_AggregateConfiguration_Initialize(&a_pValue->AggregateConfiguration);
    }
}

/*============================================================================
 * UA_AggregateFilter_Clear
 *===========================================================================*/
void UA_AggregateFilter_Clear(UA_AggregateFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->StartTime);
        NodeId_Clear(&a_pValue->AggregateType);
        Double_Clear(&a_pValue->ProcessingInterval);
        UA_AggregateConfiguration_Clear(&a_pValue->AggregateConfiguration);
    }
}

/*============================================================================
 * UA_AggregateFilter_Encode
 *===========================================================================*/
StatusCode UA_AggregateFilter_Encode(UA_AggregateFilter* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= NodeId_Write(&a_pValue->AggregateType, msgBuf);
    status &= Double_Write(&a_pValue->ProcessingInterval, msgBuf);
    status &= UA_AggregateConfiguration_Encode(&a_pValue->AggregateConfiguration, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AggregateFilter_Decode
 *===========================================================================*/
StatusCode UA_AggregateFilter_Decode(UA_AggregateFilter* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AggregateFilter_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= NodeId_Read(&a_pValue->AggregateType, msgBuf);
    status &= Double_Read(&a_pValue->ProcessingInterval, msgBuf);
    status &= UA_AggregateConfiguration_Decode(&a_pValue->AggregateConfiguration, msgBuf);

    if(status != STATUS_OK){
        UA_AggregateFilter_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AggregateFilter_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AggregateFilter_EncodeableType =
{
    "AggregateFilter",
    OpcUaId_AggregateFilter,
    OpcUaId_AggregateFilter_Encoding_DefaultBinary,
    OpcUaId_AggregateFilter_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AggregateFilter),
    (UA_EncodeableObject_PfnInitialize*)UA_AggregateFilter_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AggregateFilter_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AggregateFilter_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AggregateFilter_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AggregateFilter_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EventFilterResult
/*============================================================================
 * UA_EventFilterResult_Initialize
 *===========================================================================*/
void UA_EventFilterResult_Initialize(UA_EventFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfSelectClauseResults, (void**) &a_pValue->SelectClauseResults, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfSelectClauseDiagnosticInfos, (void**) &a_pValue->SelectClauseDiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
        UA_ContentFilterResult_Initialize(&a_pValue->WhereClauseResult);
    }
}

/*============================================================================
 * UA_EventFilterResult_Clear
 *===========================================================================*/
void UA_EventFilterResult_Clear(UA_EventFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfSelectClauseResults, (void**) &a_pValue->SelectClauseResults, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfSelectClauseDiagnosticInfos, (void**) &a_pValue->SelectClauseDiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
        UA_ContentFilterResult_Clear(&a_pValue->WhereClauseResult);
    }
}

/*============================================================================
 * UA_EventFilterResult_Encode
 *===========================================================================*/
StatusCode UA_EventFilterResult_Encode(UA_EventFilterResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfSelectClauseResults, (void**) &a_pValue->SelectClauseResults, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfSelectClauseDiagnosticInfos, (void**) &a_pValue->SelectClauseDiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);
    status &= UA_ContentFilterResult_Encode(&a_pValue->WhereClauseResult, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EventFilterResult_Decode
 *===========================================================================*/
StatusCode UA_EventFilterResult_Decode(UA_EventFilterResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EventFilterResult_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfSelectClauseResults, (void**) &a_pValue->SelectClauseResults, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfSelectClauseDiagnosticInfos, (void**) &a_pValue->SelectClauseDiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);
    status &= UA_ContentFilterResult_Decode(&a_pValue->WhereClauseResult, msgBuf);

    if(status != STATUS_OK){
        UA_EventFilterResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EventFilterResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EventFilterResult_EncodeableType =
{
    "EventFilterResult",
    OpcUaId_EventFilterResult,
    OpcUaId_EventFilterResult_Encoding_DefaultBinary,
    OpcUaId_EventFilterResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EventFilterResult),
    (UA_EncodeableObject_PfnInitialize*)UA_EventFilterResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EventFilterResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EventFilterResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EventFilterResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EventFilterResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilterResult
/*============================================================================
 * UA_AggregateFilterResult_Initialize
 *===========================================================================*/
void UA_AggregateFilterResult_Initialize(UA_AggregateFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->RevisedStartTime);
        Double_Initialize(&a_pValue->RevisedProcessingInterval);
        UA_AggregateConfiguration_Initialize(&a_pValue->RevisedAggregateConfiguration);
    }
}

/*============================================================================
 * UA_AggregateFilterResult_Clear
 *===========================================================================*/
void UA_AggregateFilterResult_Clear(UA_AggregateFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->RevisedStartTime);
        Double_Clear(&a_pValue->RevisedProcessingInterval);
        UA_AggregateConfiguration_Clear(&a_pValue->RevisedAggregateConfiguration);
    }
}

/*============================================================================
 * UA_AggregateFilterResult_Encode
 *===========================================================================*/
StatusCode UA_AggregateFilterResult_Encode(UA_AggregateFilterResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->RevisedStartTime, msgBuf);
    status &= Double_Write(&a_pValue->RevisedProcessingInterval, msgBuf);
    status &= UA_AggregateConfiguration_Encode(&a_pValue->RevisedAggregateConfiguration, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AggregateFilterResult_Decode
 *===========================================================================*/
StatusCode UA_AggregateFilterResult_Decode(UA_AggregateFilterResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AggregateFilterResult_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->RevisedStartTime, msgBuf);
    status &= Double_Read(&a_pValue->RevisedProcessingInterval, msgBuf);
    status &= UA_AggregateConfiguration_Decode(&a_pValue->RevisedAggregateConfiguration, msgBuf);

    if(status != STATUS_OK){
        UA_AggregateFilterResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AggregateFilterResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AggregateFilterResult_EncodeableType =
{
    "AggregateFilterResult",
    OpcUaId_AggregateFilterResult,
    OpcUaId_AggregateFilterResult_Encoding_DefaultBinary,
    OpcUaId_AggregateFilterResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AggregateFilterResult),
    (UA_EncodeableObject_PfnInitialize*)UA_AggregateFilterResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AggregateFilterResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AggregateFilterResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AggregateFilterResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AggregateFilterResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoringParameters
/*============================================================================
 * UA_MonitoringParameters_Initialize
 *===========================================================================*/
void UA_MonitoringParameters_Initialize(UA_MonitoringParameters* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->ClientHandle);
        Double_Initialize(&a_pValue->SamplingInterval);
        ExtensionObject_Initialize(&a_pValue->Filter);
        UInt32_Initialize(&a_pValue->QueueSize);
        Boolean_Initialize(&a_pValue->DiscardOldest);
    }
}

/*============================================================================
 * UA_MonitoringParameters_Clear
 *===========================================================================*/
void UA_MonitoringParameters_Clear(UA_MonitoringParameters* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->ClientHandle);
        Double_Clear(&a_pValue->SamplingInterval);
        ExtensionObject_Clear(&a_pValue->Filter);
        UInt32_Clear(&a_pValue->QueueSize);
        Boolean_Clear(&a_pValue->DiscardOldest);
    }
}

/*============================================================================
 * UA_MonitoringParameters_Encode
 *===========================================================================*/
StatusCode UA_MonitoringParameters_Encode(UA_MonitoringParameters* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->ClientHandle, msgBuf);
    status &= Double_Write(&a_pValue->SamplingInterval, msgBuf);
    status &= ExtensionObject_Write(&a_pValue->Filter, msgBuf);
    status &= UInt32_Write(&a_pValue->QueueSize, msgBuf);
    status &= Boolean_Write(&a_pValue->DiscardOldest, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_MonitoringParameters_Decode
 *===========================================================================*/
StatusCode UA_MonitoringParameters_Decode(UA_MonitoringParameters* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_MonitoringParameters_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->ClientHandle, msgBuf);
    status &= Double_Read(&a_pValue->SamplingInterval, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->Filter, msgBuf);
    status &= UInt32_Read(&a_pValue->QueueSize, msgBuf);
    status &= Boolean_Read(&a_pValue->DiscardOldest, msgBuf);

    if(status != STATUS_OK){
        UA_MonitoringParameters_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_MonitoringParameters_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_MonitoringParameters_EncodeableType =
{
    "MonitoringParameters",
    OpcUaId_MonitoringParameters,
    OpcUaId_MonitoringParameters_Encoding_DefaultBinary,
    OpcUaId_MonitoringParameters_Encoding_DefaultXml,
    NULL,
    sizeof(UA_MonitoringParameters),
    (UA_EncodeableObject_PfnInitialize*)UA_MonitoringParameters_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_MonitoringParameters_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_MonitoringParameters_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_MonitoringParameters_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_MonitoringParameters_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemCreateRequest
/*============================================================================
 * UA_MonitoredItemCreateRequest_Initialize
 *===========================================================================*/
void UA_MonitoredItemCreateRequest_Initialize(UA_MonitoredItemCreateRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ReadValueId_Initialize(&a_pValue->ItemToMonitor);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->MonitoringMode);
        UA_MonitoringParameters_Initialize(&a_pValue->RequestedParameters);
    }
}

/*============================================================================
 * UA_MonitoredItemCreateRequest_Clear
 *===========================================================================*/
void UA_MonitoredItemCreateRequest_Clear(UA_MonitoredItemCreateRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ReadValueId_Clear(&a_pValue->ItemToMonitor);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->MonitoringMode);
        UA_MonitoringParameters_Clear(&a_pValue->RequestedParameters);
    }
}

/*============================================================================
 * UA_MonitoredItemCreateRequest_Encode
 *===========================================================================*/
StatusCode UA_MonitoredItemCreateRequest_Encode(UA_MonitoredItemCreateRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ReadValueId_Encode(&a_pValue->ItemToMonitor, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->MonitoringMode);
    status &= UA_MonitoringParameters_Encode(&a_pValue->RequestedParameters, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemCreateRequest_Decode
 *===========================================================================*/
StatusCode UA_MonitoredItemCreateRequest_Decode(UA_MonitoredItemCreateRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_MonitoredItemCreateRequest_Initialize(a_pValue);

    status &= UA_ReadValueId_Decode(&a_pValue->ItemToMonitor, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->MonitoringMode);
    status &= UA_MonitoringParameters_Decode(&a_pValue->RequestedParameters, msgBuf);

    if(status != STATUS_OK){
        UA_MonitoredItemCreateRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemCreateRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_MonitoredItemCreateRequest_EncodeableType =
{
    "MonitoredItemCreateRequest",
    OpcUaId_MonitoredItemCreateRequest,
    OpcUaId_MonitoredItemCreateRequest_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemCreateRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_MonitoredItemCreateRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_MonitoredItemCreateRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_MonitoredItemCreateRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_MonitoredItemCreateRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_MonitoredItemCreateRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_MonitoredItemCreateRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemCreateResult
/*============================================================================
 * UA_MonitoredItemCreateResult_Initialize
 *===========================================================================*/
void UA_MonitoredItemCreateResult_Initialize(UA_MonitoredItemCreateResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        UInt32_Initialize(&a_pValue->MonitoredItemId);
        Double_Initialize(&a_pValue->RevisedSamplingInterval);
        UInt32_Initialize(&a_pValue->RevisedQueueSize);
        ExtensionObject_Initialize(&a_pValue->FilterResult);
    }
}

/*============================================================================
 * UA_MonitoredItemCreateResult_Clear
 *===========================================================================*/
void UA_MonitoredItemCreateResult_Clear(UA_MonitoredItemCreateResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        UInt32_Clear(&a_pValue->MonitoredItemId);
        Double_Clear(&a_pValue->RevisedSamplingInterval);
        UInt32_Clear(&a_pValue->RevisedQueueSize);
        ExtensionObject_Clear(&a_pValue->FilterResult);
    }
}

/*============================================================================
 * UA_MonitoredItemCreateResult_Encode
 *===========================================================================*/
StatusCode UA_MonitoredItemCreateResult_Encode(UA_MonitoredItemCreateResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= UInt32_Write(&a_pValue->MonitoredItemId, msgBuf);
    status &= Double_Write(&a_pValue->RevisedSamplingInterval, msgBuf);
    status &= UInt32_Write(&a_pValue->RevisedQueueSize, msgBuf);
    status &= ExtensionObject_Write(&a_pValue->FilterResult, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemCreateResult_Decode
 *===========================================================================*/
StatusCode UA_MonitoredItemCreateResult_Decode(UA_MonitoredItemCreateResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_MonitoredItemCreateResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= UInt32_Read(&a_pValue->MonitoredItemId, msgBuf);
    status &= Double_Read(&a_pValue->RevisedSamplingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedQueueSize, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->FilterResult, msgBuf);

    if(status != STATUS_OK){
        UA_MonitoredItemCreateResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemCreateResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_MonitoredItemCreateResult_EncodeableType =
{
    "MonitoredItemCreateResult",
    OpcUaId_MonitoredItemCreateResult,
    OpcUaId_MonitoredItemCreateResult_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemCreateResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_MonitoredItemCreateResult),
    (UA_EncodeableObject_PfnInitialize*)UA_MonitoredItemCreateResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_MonitoredItemCreateResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_MonitoredItemCreateResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_MonitoredItemCreateResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_MonitoredItemCreateResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
#ifndef OPCUA_EXCLUDE_CreateMonitoredItemsRequest
/*============================================================================
 * UA_CreateMonitoredItemsRequest_Initialize
 *===========================================================================*/
void UA_CreateMonitoredItemsRequest_Initialize(UA_CreateMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        UA_Initialize_Array(&a_pValue->NoOfItemsToCreate, (void**) &a_pValue->ItemsToCreate, 
                            sizeof(UA_MonitoredItemCreateRequest), (UA_EncodeableObject_PfnInitialize*) UA_MonitoredItemCreateRequest_Initialize);
    }
}

/*============================================================================
 * UA_CreateMonitoredItemsRequest_Clear
 *===========================================================================*/
void UA_CreateMonitoredItemsRequest_Clear(UA_CreateMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        UA_Clear_Array(&a_pValue->NoOfItemsToCreate, (void**) &a_pValue->ItemsToCreate, 
                       sizeof(UA_MonitoredItemCreateRequest), (UA_EncodeableObject_PfnClear*) UA_MonitoredItemCreateRequest_Clear);
    }
}

/*============================================================================
 * UA_CreateMonitoredItemsRequest_Encode
 *===========================================================================*/
StatusCode UA_CreateMonitoredItemsRequest_Encode(UA_CreateMonitoredItemsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfItemsToCreate, (void**) &a_pValue->ItemsToCreate, 
                   sizeof(UA_MonitoredItemCreateRequest), (UA_EncodeableObject_PfnEncode*) UA_MonitoredItemCreateRequest_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CreateMonitoredItemsRequest_Decode
 *===========================================================================*/
StatusCode UA_CreateMonitoredItemsRequest_Decode(UA_CreateMonitoredItemsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CreateMonitoredItemsRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfItemsToCreate, (void**) &a_pValue->ItemsToCreate, 
                  sizeof(UA_MonitoredItemCreateRequest), (UA_EncodeableObject_PfnDecode*) UA_MonitoredItemCreateRequest_Decode);

    if(status != STATUS_OK){
        UA_CreateMonitoredItemsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CreateMonitoredItemsRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CreateMonitoredItemsRequest_EncodeableType =
{
    "CreateMonitoredItemsRequest",
    OpcUaId_CreateMonitoredItemsRequest,
    OpcUaId_CreateMonitoredItemsRequest_Encoding_DefaultBinary,
    OpcUaId_CreateMonitoredItemsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CreateMonitoredItemsRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_CreateMonitoredItemsRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CreateMonitoredItemsRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CreateMonitoredItemsRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CreateMonitoredItemsRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CreateMonitoredItemsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItemsResponse
/*============================================================================
 * UA_CreateMonitoredItemsResponse_Initialize
 *===========================================================================*/
void UA_CreateMonitoredItemsResponse_Initialize(UA_CreateMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_MonitoredItemCreateResult), (UA_EncodeableObject_PfnInitialize*) UA_MonitoredItemCreateResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_CreateMonitoredItemsResponse_Clear
 *===========================================================================*/
void UA_CreateMonitoredItemsResponse_Clear(UA_CreateMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_MonitoredItemCreateResult), (UA_EncodeableObject_PfnClear*) UA_MonitoredItemCreateResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_CreateMonitoredItemsResponse_Encode
 *===========================================================================*/
StatusCode UA_CreateMonitoredItemsResponse_Encode(UA_CreateMonitoredItemsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_MonitoredItemCreateResult), (UA_EncodeableObject_PfnEncode*) UA_MonitoredItemCreateResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CreateMonitoredItemsResponse_Decode
 *===========================================================================*/
StatusCode UA_CreateMonitoredItemsResponse_Decode(UA_CreateMonitoredItemsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CreateMonitoredItemsResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_MonitoredItemCreateResult), (UA_EncodeableObject_PfnDecode*) UA_MonitoredItemCreateResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_CreateMonitoredItemsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CreateMonitoredItemsResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CreateMonitoredItemsResponse_EncodeableType =
{
    "CreateMonitoredItemsResponse",
    OpcUaId_CreateMonitoredItemsResponse,
    OpcUaId_CreateMonitoredItemsResponse_Encoding_DefaultBinary,
    OpcUaId_CreateMonitoredItemsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CreateMonitoredItemsResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_CreateMonitoredItemsResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CreateMonitoredItemsResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CreateMonitoredItemsResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CreateMonitoredItemsResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CreateMonitoredItemsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemModifyRequest
/*============================================================================
 * UA_MonitoredItemModifyRequest_Initialize
 *===========================================================================*/
void UA_MonitoredItemModifyRequest_Initialize(UA_MonitoredItemModifyRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->MonitoredItemId);
        UA_MonitoringParameters_Initialize(&a_pValue->RequestedParameters);
    }
}

/*============================================================================
 * UA_MonitoredItemModifyRequest_Clear
 *===========================================================================*/
void UA_MonitoredItemModifyRequest_Clear(UA_MonitoredItemModifyRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->MonitoredItemId);
        UA_MonitoringParameters_Clear(&a_pValue->RequestedParameters);
    }
}

/*============================================================================
 * UA_MonitoredItemModifyRequest_Encode
 *===========================================================================*/
StatusCode UA_MonitoredItemModifyRequest_Encode(UA_MonitoredItemModifyRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->MonitoredItemId, msgBuf);
    status &= UA_MonitoringParameters_Encode(&a_pValue->RequestedParameters, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemModifyRequest_Decode
 *===========================================================================*/
StatusCode UA_MonitoredItemModifyRequest_Decode(UA_MonitoredItemModifyRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_MonitoredItemModifyRequest_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->MonitoredItemId, msgBuf);
    status &= UA_MonitoringParameters_Decode(&a_pValue->RequestedParameters, msgBuf);

    if(status != STATUS_OK){
        UA_MonitoredItemModifyRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemModifyRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_MonitoredItemModifyRequest_EncodeableType =
{
    "MonitoredItemModifyRequest",
    OpcUaId_MonitoredItemModifyRequest,
    OpcUaId_MonitoredItemModifyRequest_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemModifyRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_MonitoredItemModifyRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_MonitoredItemModifyRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_MonitoredItemModifyRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_MonitoredItemModifyRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_MonitoredItemModifyRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_MonitoredItemModifyRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemModifyResult
/*============================================================================
 * UA_MonitoredItemModifyResult_Initialize
 *===========================================================================*/
void UA_MonitoredItemModifyResult_Initialize(UA_MonitoredItemModifyResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        Double_Initialize(&a_pValue->RevisedSamplingInterval);
        UInt32_Initialize(&a_pValue->RevisedQueueSize);
        ExtensionObject_Initialize(&a_pValue->FilterResult);
    }
}

/*============================================================================
 * UA_MonitoredItemModifyResult_Clear
 *===========================================================================*/
void UA_MonitoredItemModifyResult_Clear(UA_MonitoredItemModifyResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        Double_Clear(&a_pValue->RevisedSamplingInterval);
        UInt32_Clear(&a_pValue->RevisedQueueSize);
        ExtensionObject_Clear(&a_pValue->FilterResult);
    }
}

/*============================================================================
 * UA_MonitoredItemModifyResult_Encode
 *===========================================================================*/
StatusCode UA_MonitoredItemModifyResult_Encode(UA_MonitoredItemModifyResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= Double_Write(&a_pValue->RevisedSamplingInterval, msgBuf);
    status &= UInt32_Write(&a_pValue->RevisedQueueSize, msgBuf);
    status &= ExtensionObject_Write(&a_pValue->FilterResult, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemModifyResult_Decode
 *===========================================================================*/
StatusCode UA_MonitoredItemModifyResult_Decode(UA_MonitoredItemModifyResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_MonitoredItemModifyResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= Double_Read(&a_pValue->RevisedSamplingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedQueueSize, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->FilterResult, msgBuf);

    if(status != STATUS_OK){
        UA_MonitoredItemModifyResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemModifyResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_MonitoredItemModifyResult_EncodeableType =
{
    "MonitoredItemModifyResult",
    OpcUaId_MonitoredItemModifyResult,
    OpcUaId_MonitoredItemModifyResult_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemModifyResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_MonitoredItemModifyResult),
    (UA_EncodeableObject_PfnInitialize*)UA_MonitoredItemModifyResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_MonitoredItemModifyResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_MonitoredItemModifyResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_MonitoredItemModifyResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_MonitoredItemModifyResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
#ifndef OPCUA_EXCLUDE_ModifyMonitoredItemsRequest
/*============================================================================
 * UA_ModifyMonitoredItemsRequest_Initialize
 *===========================================================================*/
void UA_ModifyMonitoredItemsRequest_Initialize(UA_ModifyMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        UA_Initialize_Array(&a_pValue->NoOfItemsToModify, (void**) &a_pValue->ItemsToModify, 
                            sizeof(UA_MonitoredItemModifyRequest), (UA_EncodeableObject_PfnInitialize*) UA_MonitoredItemModifyRequest_Initialize);
    }
}

/*============================================================================
 * UA_ModifyMonitoredItemsRequest_Clear
 *===========================================================================*/
void UA_ModifyMonitoredItemsRequest_Clear(UA_ModifyMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        UA_Clear_Array(&a_pValue->NoOfItemsToModify, (void**) &a_pValue->ItemsToModify, 
                       sizeof(UA_MonitoredItemModifyRequest), (UA_EncodeableObject_PfnClear*) UA_MonitoredItemModifyRequest_Clear);
    }
}

/*============================================================================
 * UA_ModifyMonitoredItemsRequest_Encode
 *===========================================================================*/
StatusCode UA_ModifyMonitoredItemsRequest_Encode(UA_ModifyMonitoredItemsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfItemsToModify, (void**) &a_pValue->ItemsToModify, 
                   sizeof(UA_MonitoredItemModifyRequest), (UA_EncodeableObject_PfnEncode*) UA_MonitoredItemModifyRequest_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ModifyMonitoredItemsRequest_Decode
 *===========================================================================*/
StatusCode UA_ModifyMonitoredItemsRequest_Decode(UA_ModifyMonitoredItemsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ModifyMonitoredItemsRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfItemsToModify, (void**) &a_pValue->ItemsToModify, 
                  sizeof(UA_MonitoredItemModifyRequest), (UA_EncodeableObject_PfnDecode*) UA_MonitoredItemModifyRequest_Decode);

    if(status != STATUS_OK){
        UA_ModifyMonitoredItemsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ModifyMonitoredItemsRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ModifyMonitoredItemsRequest_EncodeableType =
{
    "ModifyMonitoredItemsRequest",
    OpcUaId_ModifyMonitoredItemsRequest,
    OpcUaId_ModifyMonitoredItemsRequest_Encoding_DefaultBinary,
    OpcUaId_ModifyMonitoredItemsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ModifyMonitoredItemsRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_ModifyMonitoredItemsRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ModifyMonitoredItemsRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ModifyMonitoredItemsRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ModifyMonitoredItemsRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ModifyMonitoredItemsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItemsResponse
/*============================================================================
 * UA_ModifyMonitoredItemsResponse_Initialize
 *===========================================================================*/
void UA_ModifyMonitoredItemsResponse_Initialize(UA_ModifyMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_MonitoredItemModifyResult), (UA_EncodeableObject_PfnInitialize*) UA_MonitoredItemModifyResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_ModifyMonitoredItemsResponse_Clear
 *===========================================================================*/
void UA_ModifyMonitoredItemsResponse_Clear(UA_ModifyMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_MonitoredItemModifyResult), (UA_EncodeableObject_PfnClear*) UA_MonitoredItemModifyResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_ModifyMonitoredItemsResponse_Encode
 *===========================================================================*/
StatusCode UA_ModifyMonitoredItemsResponse_Encode(UA_ModifyMonitoredItemsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_MonitoredItemModifyResult), (UA_EncodeableObject_PfnEncode*) UA_MonitoredItemModifyResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ModifyMonitoredItemsResponse_Decode
 *===========================================================================*/
StatusCode UA_ModifyMonitoredItemsResponse_Decode(UA_ModifyMonitoredItemsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ModifyMonitoredItemsResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_MonitoredItemModifyResult), (UA_EncodeableObject_PfnDecode*) UA_MonitoredItemModifyResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_ModifyMonitoredItemsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ModifyMonitoredItemsResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ModifyMonitoredItemsResponse_EncodeableType =
{
    "ModifyMonitoredItemsResponse",
    OpcUaId_ModifyMonitoredItemsResponse,
    OpcUaId_ModifyMonitoredItemsResponse_Encoding_DefaultBinary,
    OpcUaId_ModifyMonitoredItemsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ModifyMonitoredItemsResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_ModifyMonitoredItemsResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ModifyMonitoredItemsResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ModifyMonitoredItemsResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ModifyMonitoredItemsResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ModifyMonitoredItemsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
#ifndef OPCUA_EXCLUDE_SetMonitoringModeRequest
/*============================================================================
 * UA_SetMonitoringModeRequest_Initialize
 *===========================================================================*/
void UA_SetMonitoringModeRequest_Initialize(UA_SetMonitoringModeRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->MonitoringMode);
        UA_Initialize_Array(&a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * UA_SetMonitoringModeRequest_Clear
 *===========================================================================*/
void UA_SetMonitoringModeRequest_Clear(UA_SetMonitoringModeRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->MonitoringMode);
        UA_Clear_Array(&a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * UA_SetMonitoringModeRequest_Encode
 *===========================================================================*/
StatusCode UA_SetMonitoringModeRequest_Encode(UA_SetMonitoringModeRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->MonitoringMode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SetMonitoringModeRequest_Decode
 *===========================================================================*/
StatusCode UA_SetMonitoringModeRequest_Decode(UA_SetMonitoringModeRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SetMonitoringModeRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->MonitoringMode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        UA_SetMonitoringModeRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SetMonitoringModeRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SetMonitoringModeRequest_EncodeableType =
{
    "SetMonitoringModeRequest",
    OpcUaId_SetMonitoringModeRequest,
    OpcUaId_SetMonitoringModeRequest_Encoding_DefaultBinary,
    OpcUaId_SetMonitoringModeRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SetMonitoringModeRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_SetMonitoringModeRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SetMonitoringModeRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SetMonitoringModeRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SetMonitoringModeRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SetMonitoringModeRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringModeResponse
/*============================================================================
 * UA_SetMonitoringModeResponse_Initialize
 *===========================================================================*/
void UA_SetMonitoringModeResponse_Initialize(UA_SetMonitoringModeResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_SetMonitoringModeResponse_Clear
 *===========================================================================*/
void UA_SetMonitoringModeResponse_Clear(UA_SetMonitoringModeResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_SetMonitoringModeResponse_Encode
 *===========================================================================*/
StatusCode UA_SetMonitoringModeResponse_Encode(UA_SetMonitoringModeResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SetMonitoringModeResponse_Decode
 *===========================================================================*/
StatusCode UA_SetMonitoringModeResponse_Decode(UA_SetMonitoringModeResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SetMonitoringModeResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_SetMonitoringModeResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SetMonitoringModeResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SetMonitoringModeResponse_EncodeableType =
{
    "SetMonitoringModeResponse",
    OpcUaId_SetMonitoringModeResponse,
    OpcUaId_SetMonitoringModeResponse_Encoding_DefaultBinary,
    OpcUaId_SetMonitoringModeResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SetMonitoringModeResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_SetMonitoringModeResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SetMonitoringModeResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SetMonitoringModeResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SetMonitoringModeResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SetMonitoringModeResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
#ifndef OPCUA_EXCLUDE_SetTriggeringRequest
/*============================================================================
 * UA_SetTriggeringRequest_Initialize
 *===========================================================================*/
void UA_SetTriggeringRequest_Initialize(UA_SetTriggeringRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UInt32_Initialize(&a_pValue->TriggeringItemId);
        UA_Initialize_Array(&a_pValue->NoOfLinksToAdd, (void**) &a_pValue->LinksToAdd, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfLinksToRemove, (void**) &a_pValue->LinksToRemove, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * UA_SetTriggeringRequest_Clear
 *===========================================================================*/
void UA_SetTriggeringRequest_Clear(UA_SetTriggeringRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        UInt32_Clear(&a_pValue->TriggeringItemId);
        UA_Clear_Array(&a_pValue->NoOfLinksToAdd, (void**) &a_pValue->LinksToAdd, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
        UA_Clear_Array(&a_pValue->NoOfLinksToRemove, (void**) &a_pValue->LinksToRemove, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * UA_SetTriggeringRequest_Encode
 *===========================================================================*/
StatusCode UA_SetTriggeringRequest_Encode(UA_SetTriggeringRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Write(&a_pValue->TriggeringItemId, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfLinksToAdd, (void**) &a_pValue->LinksToAdd, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfLinksToRemove, (void**) &a_pValue->LinksToRemove, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SetTriggeringRequest_Decode
 *===========================================================================*/
StatusCode UA_SetTriggeringRequest_Decode(UA_SetTriggeringRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SetTriggeringRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Read(&a_pValue->TriggeringItemId, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfLinksToAdd, (void**) &a_pValue->LinksToAdd, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfLinksToRemove, (void**) &a_pValue->LinksToRemove, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        UA_SetTriggeringRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SetTriggeringRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SetTriggeringRequest_EncodeableType =
{
    "SetTriggeringRequest",
    OpcUaId_SetTriggeringRequest,
    OpcUaId_SetTriggeringRequest_Encoding_DefaultBinary,
    OpcUaId_SetTriggeringRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SetTriggeringRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_SetTriggeringRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SetTriggeringRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SetTriggeringRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SetTriggeringRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SetTriggeringRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SetTriggeringResponse
/*============================================================================
 * UA_SetTriggeringResponse_Initialize
 *===========================================================================*/
void UA_SetTriggeringResponse_Initialize(UA_SetTriggeringResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfAddResults, (void**) &a_pValue->AddResults, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfAddDiagnosticInfos, (void**) &a_pValue->AddDiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfRemoveResults, (void**) &a_pValue->RemoveResults, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfRemoveDiagnosticInfos, (void**) &a_pValue->RemoveDiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_SetTriggeringResponse_Clear
 *===========================================================================*/
void UA_SetTriggeringResponse_Clear(UA_SetTriggeringResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfAddResults, (void**) &a_pValue->AddResults, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfAddDiagnosticInfos, (void**) &a_pValue->AddDiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
        UA_Clear_Array(&a_pValue->NoOfRemoveResults, (void**) &a_pValue->RemoveResults, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfRemoveDiagnosticInfos, (void**) &a_pValue->RemoveDiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_SetTriggeringResponse_Encode
 *===========================================================================*/
StatusCode UA_SetTriggeringResponse_Encode(UA_SetTriggeringResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfAddResults, (void**) &a_pValue->AddResults, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfAddDiagnosticInfos, (void**) &a_pValue->AddDiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfRemoveResults, (void**) &a_pValue->RemoveResults, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfRemoveDiagnosticInfos, (void**) &a_pValue->RemoveDiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SetTriggeringResponse_Decode
 *===========================================================================*/
StatusCode UA_SetTriggeringResponse_Decode(UA_SetTriggeringResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SetTriggeringResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfAddResults, (void**) &a_pValue->AddResults, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfAddDiagnosticInfos, (void**) &a_pValue->AddDiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfRemoveResults, (void**) &a_pValue->RemoveResults, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfRemoveDiagnosticInfos, (void**) &a_pValue->RemoveDiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_SetTriggeringResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SetTriggeringResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SetTriggeringResponse_EncodeableType =
{
    "SetTriggeringResponse",
    OpcUaId_SetTriggeringResponse,
    OpcUaId_SetTriggeringResponse_Encoding_DefaultBinary,
    OpcUaId_SetTriggeringResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SetTriggeringResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_SetTriggeringResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SetTriggeringResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SetTriggeringResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SetTriggeringResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SetTriggeringResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
#ifndef OPCUA_EXCLUDE_DeleteMonitoredItemsRequest
/*============================================================================
 * UA_DeleteMonitoredItemsRequest_Initialize
 *===========================================================================*/
void UA_DeleteMonitoredItemsRequest_Initialize(UA_DeleteMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UA_Initialize_Array(&a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * UA_DeleteMonitoredItemsRequest_Clear
 *===========================================================================*/
void UA_DeleteMonitoredItemsRequest_Clear(UA_DeleteMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        UA_Clear_Array(&a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * UA_DeleteMonitoredItemsRequest_Encode
 *===========================================================================*/
StatusCode UA_DeleteMonitoredItemsRequest_Encode(UA_DeleteMonitoredItemsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteMonitoredItemsRequest_Decode
 *===========================================================================*/
StatusCode UA_DeleteMonitoredItemsRequest_Decode(UA_DeleteMonitoredItemsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteMonitoredItemsRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        UA_DeleteMonitoredItemsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteMonitoredItemsRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteMonitoredItemsRequest_EncodeableType =
{
    "DeleteMonitoredItemsRequest",
    OpcUaId_DeleteMonitoredItemsRequest,
    OpcUaId_DeleteMonitoredItemsRequest_Encoding_DefaultBinary,
    OpcUaId_DeleteMonitoredItemsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteMonitoredItemsRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteMonitoredItemsRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteMonitoredItemsRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteMonitoredItemsRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteMonitoredItemsRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteMonitoredItemsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItemsResponse
/*============================================================================
 * UA_DeleteMonitoredItemsResponse_Initialize
 *===========================================================================*/
void UA_DeleteMonitoredItemsResponse_Initialize(UA_DeleteMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_DeleteMonitoredItemsResponse_Clear
 *===========================================================================*/
void UA_DeleteMonitoredItemsResponse_Clear(UA_DeleteMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_DeleteMonitoredItemsResponse_Encode
 *===========================================================================*/
StatusCode UA_DeleteMonitoredItemsResponse_Encode(UA_DeleteMonitoredItemsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteMonitoredItemsResponse_Decode
 *===========================================================================*/
StatusCode UA_DeleteMonitoredItemsResponse_Decode(UA_DeleteMonitoredItemsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteMonitoredItemsResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_DeleteMonitoredItemsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteMonitoredItemsResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteMonitoredItemsResponse_EncodeableType =
{
    "DeleteMonitoredItemsResponse",
    OpcUaId_DeleteMonitoredItemsResponse,
    OpcUaId_DeleteMonitoredItemsResponse_Encoding_DefaultBinary,
    OpcUaId_DeleteMonitoredItemsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteMonitoredItemsResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteMonitoredItemsResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteMonitoredItemsResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteMonitoredItemsResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteMonitoredItemsResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteMonitoredItemsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
#ifndef OPCUA_EXCLUDE_CreateSubscriptionRequest
/*============================================================================
 * UA_CreateSubscriptionRequest_Initialize
 *===========================================================================*/
void UA_CreateSubscriptionRequest_Initialize(UA_CreateSubscriptionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Double_Initialize(&a_pValue->RequestedPublishingInterval);
        UInt32_Initialize(&a_pValue->RequestedLifetimeCount);
        UInt32_Initialize(&a_pValue->RequestedMaxKeepAliveCount);
        UInt32_Initialize(&a_pValue->MaxNotificationsPerPublish);
        Boolean_Initialize(&a_pValue->PublishingEnabled);
        Byte_Initialize(&a_pValue->Priority);
    }
}

/*============================================================================
 * UA_CreateSubscriptionRequest_Clear
 *===========================================================================*/
void UA_CreateSubscriptionRequest_Clear(UA_CreateSubscriptionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        Double_Clear(&a_pValue->RequestedPublishingInterval);
        UInt32_Clear(&a_pValue->RequestedLifetimeCount);
        UInt32_Clear(&a_pValue->RequestedMaxKeepAliveCount);
        UInt32_Clear(&a_pValue->MaxNotificationsPerPublish);
        Boolean_Clear(&a_pValue->PublishingEnabled);
        Byte_Clear(&a_pValue->Priority);
    }
}

/*============================================================================
 * UA_CreateSubscriptionRequest_Encode
 *===========================================================================*/
StatusCode UA_CreateSubscriptionRequest_Encode(UA_CreateSubscriptionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Double_Write(&a_pValue->RequestedPublishingInterval, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestedLifetimeCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestedMaxKeepAliveCount, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxNotificationsPerPublish, msgBuf);
    status &= Boolean_Write(&a_pValue->PublishingEnabled, msgBuf);
    status &= Byte_Write(&a_pValue->Priority, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CreateSubscriptionRequest_Decode
 *===========================================================================*/
StatusCode UA_CreateSubscriptionRequest_Decode(UA_CreateSubscriptionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CreateSubscriptionRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Double_Read(&a_pValue->RequestedPublishingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedMaxKeepAliveCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxNotificationsPerPublish, msgBuf);
    status &= Boolean_Read(&a_pValue->PublishingEnabled, msgBuf);
    status &= Byte_Read(&a_pValue->Priority, msgBuf);

    if(status != STATUS_OK){
        UA_CreateSubscriptionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CreateSubscriptionRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CreateSubscriptionRequest_EncodeableType =
{
    "CreateSubscriptionRequest",
    OpcUaId_CreateSubscriptionRequest,
    OpcUaId_CreateSubscriptionRequest_Encoding_DefaultBinary,
    OpcUaId_CreateSubscriptionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CreateSubscriptionRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_CreateSubscriptionRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CreateSubscriptionRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CreateSubscriptionRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CreateSubscriptionRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CreateSubscriptionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscriptionResponse
/*============================================================================
 * UA_CreateSubscriptionResponse_Initialize
 *===========================================================================*/
void UA_CreateSubscriptionResponse_Initialize(UA_CreateSubscriptionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        Double_Initialize(&a_pValue->RevisedPublishingInterval);
        UInt32_Initialize(&a_pValue->RevisedLifetimeCount);
        UInt32_Initialize(&a_pValue->RevisedMaxKeepAliveCount);
    }
}

/*============================================================================
 * UA_CreateSubscriptionResponse_Clear
 *===========================================================================*/
void UA_CreateSubscriptionResponse_Clear(UA_CreateSubscriptionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        Double_Clear(&a_pValue->RevisedPublishingInterval);
        UInt32_Clear(&a_pValue->RevisedLifetimeCount);
        UInt32_Clear(&a_pValue->RevisedMaxKeepAliveCount);
    }
}

/*============================================================================
 * UA_CreateSubscriptionResponse_Encode
 *===========================================================================*/
StatusCode UA_CreateSubscriptionResponse_Encode(UA_CreateSubscriptionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= Double_Write(&a_pValue->RevisedPublishingInterval, msgBuf);
    status &= UInt32_Write(&a_pValue->RevisedLifetimeCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RevisedMaxKeepAliveCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_CreateSubscriptionResponse_Decode
 *===========================================================================*/
StatusCode UA_CreateSubscriptionResponse_Decode(UA_CreateSubscriptionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_CreateSubscriptionResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= Double_Read(&a_pValue->RevisedPublishingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedMaxKeepAliveCount, msgBuf);

    if(status != STATUS_OK){
        UA_CreateSubscriptionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_CreateSubscriptionResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_CreateSubscriptionResponse_EncodeableType =
{
    "CreateSubscriptionResponse",
    OpcUaId_CreateSubscriptionResponse,
    OpcUaId_CreateSubscriptionResponse_Encoding_DefaultBinary,
    OpcUaId_CreateSubscriptionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_CreateSubscriptionResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_CreateSubscriptionResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_CreateSubscriptionResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_CreateSubscriptionResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_CreateSubscriptionResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_CreateSubscriptionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
#ifndef OPCUA_EXCLUDE_ModifySubscriptionRequest
/*============================================================================
 * UA_ModifySubscriptionRequest_Initialize
 *===========================================================================*/
void UA_ModifySubscriptionRequest_Initialize(UA_ModifySubscriptionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        Double_Initialize(&a_pValue->RequestedPublishingInterval);
        UInt32_Initialize(&a_pValue->RequestedLifetimeCount);
        UInt32_Initialize(&a_pValue->RequestedMaxKeepAliveCount);
        UInt32_Initialize(&a_pValue->MaxNotificationsPerPublish);
        Byte_Initialize(&a_pValue->Priority);
    }
}

/*============================================================================
 * UA_ModifySubscriptionRequest_Clear
 *===========================================================================*/
void UA_ModifySubscriptionRequest_Clear(UA_ModifySubscriptionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        Double_Clear(&a_pValue->RequestedPublishingInterval);
        UInt32_Clear(&a_pValue->RequestedLifetimeCount);
        UInt32_Clear(&a_pValue->RequestedMaxKeepAliveCount);
        UInt32_Clear(&a_pValue->MaxNotificationsPerPublish);
        Byte_Clear(&a_pValue->Priority);
    }
}

/*============================================================================
 * UA_ModifySubscriptionRequest_Encode
 *===========================================================================*/
StatusCode UA_ModifySubscriptionRequest_Encode(UA_ModifySubscriptionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= Double_Write(&a_pValue->RequestedPublishingInterval, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestedLifetimeCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestedMaxKeepAliveCount, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxNotificationsPerPublish, msgBuf);
    status &= Byte_Write(&a_pValue->Priority, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ModifySubscriptionRequest_Decode
 *===========================================================================*/
StatusCode UA_ModifySubscriptionRequest_Decode(UA_ModifySubscriptionRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ModifySubscriptionRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= Double_Read(&a_pValue->RequestedPublishingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedMaxKeepAliveCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxNotificationsPerPublish, msgBuf);
    status &= Byte_Read(&a_pValue->Priority, msgBuf);

    if(status != STATUS_OK){
        UA_ModifySubscriptionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ModifySubscriptionRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ModifySubscriptionRequest_EncodeableType =
{
    "ModifySubscriptionRequest",
    OpcUaId_ModifySubscriptionRequest,
    OpcUaId_ModifySubscriptionRequest_Encoding_DefaultBinary,
    OpcUaId_ModifySubscriptionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ModifySubscriptionRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_ModifySubscriptionRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ModifySubscriptionRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ModifySubscriptionRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ModifySubscriptionRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ModifySubscriptionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscriptionResponse
/*============================================================================
 * UA_ModifySubscriptionResponse_Initialize
 *===========================================================================*/
void UA_ModifySubscriptionResponse_Initialize(UA_ModifySubscriptionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        Double_Initialize(&a_pValue->RevisedPublishingInterval);
        UInt32_Initialize(&a_pValue->RevisedLifetimeCount);
        UInt32_Initialize(&a_pValue->RevisedMaxKeepAliveCount);
    }
}

/*============================================================================
 * UA_ModifySubscriptionResponse_Clear
 *===========================================================================*/
void UA_ModifySubscriptionResponse_Clear(UA_ModifySubscriptionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        Double_Clear(&a_pValue->RevisedPublishingInterval);
        UInt32_Clear(&a_pValue->RevisedLifetimeCount);
        UInt32_Clear(&a_pValue->RevisedMaxKeepAliveCount);
    }
}

/*============================================================================
 * UA_ModifySubscriptionResponse_Encode
 *===========================================================================*/
StatusCode UA_ModifySubscriptionResponse_Encode(UA_ModifySubscriptionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= Double_Write(&a_pValue->RevisedPublishingInterval, msgBuf);
    status &= UInt32_Write(&a_pValue->RevisedLifetimeCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RevisedMaxKeepAliveCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ModifySubscriptionResponse_Decode
 *===========================================================================*/
StatusCode UA_ModifySubscriptionResponse_Decode(UA_ModifySubscriptionResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ModifySubscriptionResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= Double_Read(&a_pValue->RevisedPublishingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedMaxKeepAliveCount, msgBuf);

    if(status != STATUS_OK){
        UA_ModifySubscriptionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ModifySubscriptionResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ModifySubscriptionResponse_EncodeableType =
{
    "ModifySubscriptionResponse",
    OpcUaId_ModifySubscriptionResponse,
    OpcUaId_ModifySubscriptionResponse_Encoding_DefaultBinary,
    OpcUaId_ModifySubscriptionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ModifySubscriptionResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_ModifySubscriptionResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ModifySubscriptionResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ModifySubscriptionResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ModifySubscriptionResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ModifySubscriptionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
#ifndef OPCUA_EXCLUDE_SetPublishingModeRequest
/*============================================================================
 * UA_SetPublishingModeRequest_Initialize
 *===========================================================================*/
void UA_SetPublishingModeRequest_Initialize(UA_SetPublishingModeRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Boolean_Initialize(&a_pValue->PublishingEnabled);
        UA_Initialize_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * UA_SetPublishingModeRequest_Clear
 *===========================================================================*/
void UA_SetPublishingModeRequest_Clear(UA_SetPublishingModeRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        Boolean_Clear(&a_pValue->PublishingEnabled);
        UA_Clear_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * UA_SetPublishingModeRequest_Encode
 *===========================================================================*/
StatusCode UA_SetPublishingModeRequest_Encode(UA_SetPublishingModeRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Write(&a_pValue->PublishingEnabled, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SetPublishingModeRequest_Decode
 *===========================================================================*/
StatusCode UA_SetPublishingModeRequest_Decode(UA_SetPublishingModeRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SetPublishingModeRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Read(&a_pValue->PublishingEnabled, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        UA_SetPublishingModeRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SetPublishingModeRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SetPublishingModeRequest_EncodeableType =
{
    "SetPublishingModeRequest",
    OpcUaId_SetPublishingModeRequest,
    OpcUaId_SetPublishingModeRequest_Encoding_DefaultBinary,
    OpcUaId_SetPublishingModeRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SetPublishingModeRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_SetPublishingModeRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SetPublishingModeRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SetPublishingModeRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SetPublishingModeRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SetPublishingModeRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingModeResponse
/*============================================================================
 * UA_SetPublishingModeResponse_Initialize
 *===========================================================================*/
void UA_SetPublishingModeResponse_Initialize(UA_SetPublishingModeResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_SetPublishingModeResponse_Clear
 *===========================================================================*/
void UA_SetPublishingModeResponse_Clear(UA_SetPublishingModeResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_SetPublishingModeResponse_Encode
 *===========================================================================*/
StatusCode UA_SetPublishingModeResponse_Encode(UA_SetPublishingModeResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SetPublishingModeResponse_Decode
 *===========================================================================*/
StatusCode UA_SetPublishingModeResponse_Decode(UA_SetPublishingModeResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SetPublishingModeResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_SetPublishingModeResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SetPublishingModeResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SetPublishingModeResponse_EncodeableType =
{
    "SetPublishingModeResponse",
    OpcUaId_SetPublishingModeResponse,
    OpcUaId_SetPublishingModeResponse_Encoding_DefaultBinary,
    OpcUaId_SetPublishingModeResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SetPublishingModeResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_SetPublishingModeResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SetPublishingModeResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SetPublishingModeResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SetPublishingModeResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SetPublishingModeResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_NotificationMessage
/*============================================================================
 * UA_NotificationMessage_Initialize
 *===========================================================================*/
void UA_NotificationMessage_Initialize(UA_NotificationMessage* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SequenceNumber);
        DateTime_Initialize(&a_pValue->PublishTime);
        UA_Initialize_Array(&a_pValue->NoOfNotificationData, (void**) &a_pValue->NotificationData, 
                            sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnInitialize*) ExtensionObject_Initialize);
    }
}

/*============================================================================
 * UA_NotificationMessage_Clear
 *===========================================================================*/
void UA_NotificationMessage_Clear(UA_NotificationMessage* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SequenceNumber);
        DateTime_Clear(&a_pValue->PublishTime);
        UA_Clear_Array(&a_pValue->NoOfNotificationData, (void**) &a_pValue->NotificationData, 
                       sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnClear*) ExtensionObject_Clear);
    }
}

/*============================================================================
 * UA_NotificationMessage_Encode
 *===========================================================================*/
StatusCode UA_NotificationMessage_Encode(UA_NotificationMessage* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SequenceNumber, msgBuf);
    status &= DateTime_Write(&a_pValue->PublishTime, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNotificationData, (void**) &a_pValue->NotificationData, 
                   sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnEncode*) ExtensionObject_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_NotificationMessage_Decode
 *===========================================================================*/
StatusCode UA_NotificationMessage_Decode(UA_NotificationMessage* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_NotificationMessage_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SequenceNumber, msgBuf);
    status &= DateTime_Read(&a_pValue->PublishTime, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNotificationData, (void**) &a_pValue->NotificationData, 
                  sizeof(UA_ExtensionObject), (UA_EncodeableObject_PfnDecode*) ExtensionObject_Read);

    if(status != STATUS_OK){
        UA_NotificationMessage_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_NotificationMessage_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_NotificationMessage_EncodeableType =
{
    "NotificationMessage",
    OpcUaId_NotificationMessage,
    OpcUaId_NotificationMessage_Encoding_DefaultBinary,
    OpcUaId_NotificationMessage_Encoding_DefaultXml,
    NULL,
    sizeof(UA_NotificationMessage),
    (UA_EncodeableObject_PfnInitialize*)UA_NotificationMessage_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_NotificationMessage_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_NotificationMessage_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_NotificationMessage_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_NotificationMessage_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DataChangeNotification
/*============================================================================
 * UA_DataChangeNotification_Initialize
 *===========================================================================*/
void UA_DataChangeNotification_Initialize(UA_DataChangeNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfMonitoredItems, (void**) &a_pValue->MonitoredItems, 
                            sizeof(UA_MonitoredItemNotification), (UA_EncodeableObject_PfnInitialize*) UA_MonitoredItemNotification_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_DataChangeNotification_Clear
 *===========================================================================*/
void UA_DataChangeNotification_Clear(UA_DataChangeNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfMonitoredItems, (void**) &a_pValue->MonitoredItems, 
                       sizeof(UA_MonitoredItemNotification), (UA_EncodeableObject_PfnClear*) UA_MonitoredItemNotification_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_DataChangeNotification_Encode
 *===========================================================================*/
StatusCode UA_DataChangeNotification_Encode(UA_DataChangeNotification* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfMonitoredItems, (void**) &a_pValue->MonitoredItems, 
                   sizeof(UA_MonitoredItemNotification), (UA_EncodeableObject_PfnEncode*) UA_MonitoredItemNotification_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DataChangeNotification_Decode
 *===========================================================================*/
StatusCode UA_DataChangeNotification_Decode(UA_DataChangeNotification* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DataChangeNotification_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfMonitoredItems, (void**) &a_pValue->MonitoredItems, 
                  sizeof(UA_MonitoredItemNotification), (UA_EncodeableObject_PfnDecode*) UA_MonitoredItemNotification_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_DataChangeNotification_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DataChangeNotification_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DataChangeNotification_EncodeableType =
{
    "DataChangeNotification",
    OpcUaId_DataChangeNotification,
    OpcUaId_DataChangeNotification_Encoding_DefaultBinary,
    OpcUaId_DataChangeNotification_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DataChangeNotification),
    (UA_EncodeableObject_PfnInitialize*)UA_DataChangeNotification_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DataChangeNotification_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DataChangeNotification_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DataChangeNotification_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DataChangeNotification_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemNotification
/*============================================================================
 * UA_MonitoredItemNotification_Initialize
 *===========================================================================*/
void UA_MonitoredItemNotification_Initialize(UA_MonitoredItemNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->ClientHandle);
        DataValue_Initialize(&a_pValue->Value);
    }
}

/*============================================================================
 * UA_MonitoredItemNotification_Clear
 *===========================================================================*/
void UA_MonitoredItemNotification_Clear(UA_MonitoredItemNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->ClientHandle);
        DataValue_Clear(&a_pValue->Value);
    }
}

/*============================================================================
 * UA_MonitoredItemNotification_Encode
 *===========================================================================*/
StatusCode UA_MonitoredItemNotification_Encode(UA_MonitoredItemNotification* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->ClientHandle, msgBuf);
    status &= DataValue_Write(&a_pValue->Value, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemNotification_Decode
 *===========================================================================*/
StatusCode UA_MonitoredItemNotification_Decode(UA_MonitoredItemNotification* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_MonitoredItemNotification_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->ClientHandle, msgBuf);
    status &= DataValue_Read(&a_pValue->Value, msgBuf);

    if(status != STATUS_OK){
        UA_MonitoredItemNotification_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_MonitoredItemNotification_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_MonitoredItemNotification_EncodeableType =
{
    "MonitoredItemNotification",
    OpcUaId_MonitoredItemNotification,
    OpcUaId_MonitoredItemNotification_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemNotification_Encoding_DefaultXml,
    NULL,
    sizeof(UA_MonitoredItemNotification),
    (UA_EncodeableObject_PfnInitialize*)UA_MonitoredItemNotification_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_MonitoredItemNotification_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_MonitoredItemNotification_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_MonitoredItemNotification_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_MonitoredItemNotification_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EventNotificationList
/*============================================================================
 * UA_EventNotificationList_Initialize
 *===========================================================================*/
void UA_EventNotificationList_Initialize(UA_EventNotificationList* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                            sizeof(UA_EventFieldList), (UA_EncodeableObject_PfnInitialize*) UA_EventFieldList_Initialize);
    }
}

/*============================================================================
 * UA_EventNotificationList_Clear
 *===========================================================================*/
void UA_EventNotificationList_Clear(UA_EventNotificationList* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                       sizeof(UA_EventFieldList), (UA_EncodeableObject_PfnClear*) UA_EventFieldList_Clear);
    }
}

/*============================================================================
 * UA_EventNotificationList_Encode
 *===========================================================================*/
StatusCode UA_EventNotificationList_Encode(UA_EventNotificationList* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                   sizeof(UA_EventFieldList), (UA_EncodeableObject_PfnEncode*) UA_EventFieldList_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EventNotificationList_Decode
 *===========================================================================*/
StatusCode UA_EventNotificationList_Decode(UA_EventNotificationList* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EventNotificationList_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                  sizeof(UA_EventFieldList), (UA_EncodeableObject_PfnDecode*) UA_EventFieldList_Decode);

    if(status != STATUS_OK){
        UA_EventNotificationList_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EventNotificationList_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EventNotificationList_EncodeableType =
{
    "EventNotificationList",
    OpcUaId_EventNotificationList,
    OpcUaId_EventNotificationList_Encoding_DefaultBinary,
    OpcUaId_EventNotificationList_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EventNotificationList),
    (UA_EncodeableObject_PfnInitialize*)UA_EventNotificationList_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EventNotificationList_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EventNotificationList_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EventNotificationList_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EventNotificationList_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EventFieldList
/*============================================================================
 * UA_EventFieldList_Initialize
 *===========================================================================*/
void UA_EventFieldList_Initialize(UA_EventFieldList* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->ClientHandle);
        UA_Initialize_Array(&a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                            sizeof(UA_Variant), (UA_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * UA_EventFieldList_Clear
 *===========================================================================*/
void UA_EventFieldList_Clear(UA_EventFieldList* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->ClientHandle);
        UA_Clear_Array(&a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                       sizeof(UA_Variant), (UA_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * UA_EventFieldList_Encode
 *===========================================================================*/
StatusCode UA_EventFieldList_Encode(UA_EventFieldList* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->ClientHandle, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                   sizeof(UA_Variant), (UA_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EventFieldList_Decode
 *===========================================================================*/
StatusCode UA_EventFieldList_Decode(UA_EventFieldList* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EventFieldList_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->ClientHandle, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                  sizeof(UA_Variant), (UA_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        UA_EventFieldList_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EventFieldList_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EventFieldList_EncodeableType =
{
    "EventFieldList",
    OpcUaId_EventFieldList,
    OpcUaId_EventFieldList_Encoding_DefaultBinary,
    OpcUaId_EventFieldList_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EventFieldList),
    (UA_EncodeableObject_PfnInitialize*)UA_EventFieldList_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EventFieldList_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EventFieldList_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EventFieldList_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EventFieldList_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryEventFieldList
/*============================================================================
 * UA_HistoryEventFieldList_Initialize
 *===========================================================================*/
void UA_HistoryEventFieldList_Initialize(UA_HistoryEventFieldList* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                            sizeof(UA_Variant), (UA_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * UA_HistoryEventFieldList_Clear
 *===========================================================================*/
void UA_HistoryEventFieldList_Clear(UA_HistoryEventFieldList* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                       sizeof(UA_Variant), (UA_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * UA_HistoryEventFieldList_Encode
 *===========================================================================*/
StatusCode UA_HistoryEventFieldList_Encode(UA_HistoryEventFieldList* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                   sizeof(UA_Variant), (UA_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_HistoryEventFieldList_Decode
 *===========================================================================*/
StatusCode UA_HistoryEventFieldList_Decode(UA_HistoryEventFieldList* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_HistoryEventFieldList_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                  sizeof(UA_Variant), (UA_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        UA_HistoryEventFieldList_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_HistoryEventFieldList_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_HistoryEventFieldList_EncodeableType =
{
    "HistoryEventFieldList",
    OpcUaId_HistoryEventFieldList,
    OpcUaId_HistoryEventFieldList_Encoding_DefaultBinary,
    OpcUaId_HistoryEventFieldList_Encoding_DefaultXml,
    NULL,
    sizeof(UA_HistoryEventFieldList),
    (UA_EncodeableObject_PfnInitialize*)UA_HistoryEventFieldList_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_HistoryEventFieldList_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_HistoryEventFieldList_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_HistoryEventFieldList_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_HistoryEventFieldList_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_StatusChangeNotification
/*============================================================================
 * UA_StatusChangeNotification_Initialize
 *===========================================================================*/
void UA_StatusChangeNotification_Initialize(UA_StatusChangeNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->Status);
        DiagnosticInfo_Initialize(&a_pValue->DiagnosticInfo);
    }
}

/*============================================================================
 * UA_StatusChangeNotification_Clear
 *===========================================================================*/
void UA_StatusChangeNotification_Clear(UA_StatusChangeNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->Status);
        DiagnosticInfo_Clear(&a_pValue->DiagnosticInfo);
    }
}

/*============================================================================
 * UA_StatusChangeNotification_Encode
 *===========================================================================*/
StatusCode UA_StatusChangeNotification_Encode(UA_StatusChangeNotification* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->Status, msgBuf);
    status &= DiagnosticInfo_Write(&a_pValue->DiagnosticInfo, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_StatusChangeNotification_Decode
 *===========================================================================*/
StatusCode UA_StatusChangeNotification_Decode(UA_StatusChangeNotification* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_StatusChangeNotification_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->Status, msgBuf);
    status &= DiagnosticInfo_Read(&a_pValue->DiagnosticInfo, msgBuf);

    if(status != STATUS_OK){
        UA_StatusChangeNotification_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_StatusChangeNotification_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_StatusChangeNotification_EncodeableType =
{
    "StatusChangeNotification",
    OpcUaId_StatusChangeNotification,
    OpcUaId_StatusChangeNotification_Encoding_DefaultBinary,
    OpcUaId_StatusChangeNotification_Encoding_DefaultXml,
    NULL,
    sizeof(UA_StatusChangeNotification),
    (UA_EncodeableObject_PfnInitialize*)UA_StatusChangeNotification_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_StatusChangeNotification_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_StatusChangeNotification_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_StatusChangeNotification_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_StatusChangeNotification_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SubscriptionAcknowledgement
/*============================================================================
 * UA_SubscriptionAcknowledgement_Initialize
 *===========================================================================*/
void UA_SubscriptionAcknowledgement_Initialize(UA_SubscriptionAcknowledgement* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UInt32_Initialize(&a_pValue->SequenceNumber);
    }
}

/*============================================================================
 * UA_SubscriptionAcknowledgement_Clear
 *===========================================================================*/
void UA_SubscriptionAcknowledgement_Clear(UA_SubscriptionAcknowledgement* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SubscriptionId);
        UInt32_Clear(&a_pValue->SequenceNumber);
    }
}

/*============================================================================
 * UA_SubscriptionAcknowledgement_Encode
 *===========================================================================*/
StatusCode UA_SubscriptionAcknowledgement_Encode(UA_SubscriptionAcknowledgement* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Write(&a_pValue->SequenceNumber, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SubscriptionAcknowledgement_Decode
 *===========================================================================*/
StatusCode UA_SubscriptionAcknowledgement_Decode(UA_SubscriptionAcknowledgement* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SubscriptionAcknowledgement_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Read(&a_pValue->SequenceNumber, msgBuf);

    if(status != STATUS_OK){
        UA_SubscriptionAcknowledgement_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SubscriptionAcknowledgement_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SubscriptionAcknowledgement_EncodeableType =
{
    "SubscriptionAcknowledgement",
    OpcUaId_SubscriptionAcknowledgement,
    OpcUaId_SubscriptionAcknowledgement_Encoding_DefaultBinary,
    OpcUaId_SubscriptionAcknowledgement_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SubscriptionAcknowledgement),
    (UA_EncodeableObject_PfnInitialize*)UA_SubscriptionAcknowledgement_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SubscriptionAcknowledgement_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SubscriptionAcknowledgement_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SubscriptionAcknowledgement_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SubscriptionAcknowledgement_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Publish
#ifndef OPCUA_EXCLUDE_PublishRequest
/*============================================================================
 * UA_PublishRequest_Initialize
 *===========================================================================*/
void UA_PublishRequest_Initialize(UA_PublishRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfSubscriptionAcknowledgements, (void**) &a_pValue->SubscriptionAcknowledgements, 
                            sizeof(UA_SubscriptionAcknowledgement), (UA_EncodeableObject_PfnInitialize*) UA_SubscriptionAcknowledgement_Initialize);
    }
}

/*============================================================================
 * UA_PublishRequest_Clear
 *===========================================================================*/
void UA_PublishRequest_Clear(UA_PublishRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfSubscriptionAcknowledgements, (void**) &a_pValue->SubscriptionAcknowledgements, 
                       sizeof(UA_SubscriptionAcknowledgement), (UA_EncodeableObject_PfnClear*) UA_SubscriptionAcknowledgement_Clear);
    }
}

/*============================================================================
 * UA_PublishRequest_Encode
 *===========================================================================*/
StatusCode UA_PublishRequest_Encode(UA_PublishRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfSubscriptionAcknowledgements, (void**) &a_pValue->SubscriptionAcknowledgements, 
                   sizeof(UA_SubscriptionAcknowledgement), (UA_EncodeableObject_PfnEncode*) UA_SubscriptionAcknowledgement_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_PublishRequest_Decode
 *===========================================================================*/
StatusCode UA_PublishRequest_Decode(UA_PublishRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_PublishRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfSubscriptionAcknowledgements, (void**) &a_pValue->SubscriptionAcknowledgements, 
                  sizeof(UA_SubscriptionAcknowledgement), (UA_EncodeableObject_PfnDecode*) UA_SubscriptionAcknowledgement_Decode);

    if(status != STATUS_OK){
        UA_PublishRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_PublishRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_PublishRequest_EncodeableType =
{
    "PublishRequest",
    OpcUaId_PublishRequest,
    OpcUaId_PublishRequest_Encoding_DefaultBinary,
    OpcUaId_PublishRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_PublishRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_PublishRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_PublishRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_PublishRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_PublishRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_PublishRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_PublishResponse
/*============================================================================
 * UA_PublishResponse_Initialize
 *===========================================================================*/
void UA_PublishResponse_Initialize(UA_PublishResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UA_Initialize_Array(&a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Boolean_Initialize(&a_pValue->MoreNotifications);
        UA_NotificationMessage_Initialize(&a_pValue->NotificationMessage);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_PublishResponse_Clear
 *===========================================================================*/
void UA_PublishResponse_Clear(UA_PublishResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        UA_Clear_Array(&a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
        Boolean_Clear(&a_pValue->MoreNotifications);
        UA_NotificationMessage_Clear(&a_pValue->NotificationMessage);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_PublishResponse_Encode
 *===========================================================================*/
StatusCode UA_PublishResponse_Encode(UA_PublishResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Boolean_Write(&a_pValue->MoreNotifications, msgBuf);
    status &= UA_NotificationMessage_Encode(&a_pValue->NotificationMessage, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_PublishResponse_Decode
 *===========================================================================*/
StatusCode UA_PublishResponse_Decode(UA_PublishResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_PublishResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Boolean_Read(&a_pValue->MoreNotifications, msgBuf);
    status &= UA_NotificationMessage_Decode(&a_pValue->NotificationMessage, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_PublishResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_PublishResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_PublishResponse_EncodeableType =
{
    "PublishResponse",
    OpcUaId_PublishResponse,
    OpcUaId_PublishResponse_Encoding_DefaultBinary,
    OpcUaId_PublishResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_PublishResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_PublishResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_PublishResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_PublishResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_PublishResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_PublishResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_Republish
#ifndef OPCUA_EXCLUDE_RepublishRequest
/*============================================================================
 * UA_RepublishRequest_Initialize
 *===========================================================================*/
void UA_RepublishRequest_Initialize(UA_RepublishRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UInt32_Initialize(&a_pValue->RetransmitSequenceNumber);
    }
}

/*============================================================================
 * UA_RepublishRequest_Clear
 *===========================================================================*/
void UA_RepublishRequest_Clear(UA_RepublishRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        UInt32_Clear(&a_pValue->RetransmitSequenceNumber);
    }
}

/*============================================================================
 * UA_RepublishRequest_Encode
 *===========================================================================*/
StatusCode UA_RepublishRequest_Encode(UA_RepublishRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Write(&a_pValue->RetransmitSequenceNumber, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RepublishRequest_Decode
 *===========================================================================*/
StatusCode UA_RepublishRequest_Decode(UA_RepublishRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RepublishRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Read(&a_pValue->RetransmitSequenceNumber, msgBuf);

    if(status != STATUS_OK){
        UA_RepublishRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RepublishRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RepublishRequest_EncodeableType =
{
    "RepublishRequest",
    OpcUaId_RepublishRequest,
    OpcUaId_RepublishRequest_Encoding_DefaultBinary,
    OpcUaId_RepublishRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RepublishRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_RepublishRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RepublishRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RepublishRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RepublishRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RepublishRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RepublishResponse
/*============================================================================
 * UA_RepublishResponse_Initialize
 *===========================================================================*/
void UA_RepublishResponse_Initialize(UA_RepublishResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_NotificationMessage_Initialize(&a_pValue->NotificationMessage);
    }
}

/*============================================================================
 * UA_RepublishResponse_Clear
 *===========================================================================*/
void UA_RepublishResponse_Clear(UA_RepublishResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_NotificationMessage_Clear(&a_pValue->NotificationMessage);
    }
}

/*============================================================================
 * UA_RepublishResponse_Encode
 *===========================================================================*/
StatusCode UA_RepublishResponse_Encode(UA_RepublishResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_NotificationMessage_Encode(&a_pValue->NotificationMessage, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RepublishResponse_Decode
 *===========================================================================*/
StatusCode UA_RepublishResponse_Decode(UA_RepublishResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RepublishResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_NotificationMessage_Decode(&a_pValue->NotificationMessage, msgBuf);

    if(status != STATUS_OK){
        UA_RepublishResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RepublishResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RepublishResponse_EncodeableType =
{
    "RepublishResponse",
    OpcUaId_RepublishResponse,
    OpcUaId_RepublishResponse_Encoding_DefaultBinary,
    OpcUaId_RepublishResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RepublishResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_RepublishResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RepublishResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RepublishResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RepublishResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RepublishResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_TransferResult
/*============================================================================
 * UA_TransferResult_Initialize
 *===========================================================================*/
void UA_TransferResult_Initialize(UA_TransferResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        UA_Initialize_Array(&a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * UA_TransferResult_Clear
 *===========================================================================*/
void UA_TransferResult_Clear(UA_TransferResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        UA_Clear_Array(&a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * UA_TransferResult_Encode
 *===========================================================================*/
StatusCode UA_TransferResult_Encode(UA_TransferResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_TransferResult_Decode
 *===========================================================================*/
StatusCode UA_TransferResult_Decode(UA_TransferResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_TransferResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        UA_TransferResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_TransferResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_TransferResult_EncodeableType =
{
    "TransferResult",
    OpcUaId_TransferResult,
    OpcUaId_TransferResult_Encoding_DefaultBinary,
    OpcUaId_TransferResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_TransferResult),
    (UA_EncodeableObject_PfnInitialize*)UA_TransferResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_TransferResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_TransferResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_TransferResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_TransferResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
#ifndef OPCUA_EXCLUDE_TransferSubscriptionsRequest
/*============================================================================
 * UA_TransferSubscriptionsRequest_Initialize
 *===========================================================================*/
void UA_TransferSubscriptionsRequest_Initialize(UA_TransferSubscriptionsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Boolean_Initialize(&a_pValue->SendInitialValues);
    }
}

/*============================================================================
 * UA_TransferSubscriptionsRequest_Clear
 *===========================================================================*/
void UA_TransferSubscriptionsRequest_Clear(UA_TransferSubscriptionsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
        Boolean_Clear(&a_pValue->SendInitialValues);
    }
}

/*============================================================================
 * UA_TransferSubscriptionsRequest_Encode
 *===========================================================================*/
StatusCode UA_TransferSubscriptionsRequest_Encode(UA_TransferSubscriptionsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Boolean_Write(&a_pValue->SendInitialValues, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_TransferSubscriptionsRequest_Decode
 *===========================================================================*/
StatusCode UA_TransferSubscriptionsRequest_Decode(UA_TransferSubscriptionsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_TransferSubscriptionsRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Boolean_Read(&a_pValue->SendInitialValues, msgBuf);

    if(status != STATUS_OK){
        UA_TransferSubscriptionsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_TransferSubscriptionsRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_TransferSubscriptionsRequest_EncodeableType =
{
    "TransferSubscriptionsRequest",
    OpcUaId_TransferSubscriptionsRequest,
    OpcUaId_TransferSubscriptionsRequest_Encoding_DefaultBinary,
    OpcUaId_TransferSubscriptionsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_TransferSubscriptionsRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_TransferSubscriptionsRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_TransferSubscriptionsRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_TransferSubscriptionsRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_TransferSubscriptionsRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_TransferSubscriptionsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptionsResponse
/*============================================================================
 * UA_TransferSubscriptionsResponse_Initialize
 *===========================================================================*/
void UA_TransferSubscriptionsResponse_Initialize(UA_TransferSubscriptionsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(UA_TransferResult), (UA_EncodeableObject_PfnInitialize*) UA_TransferResult_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_TransferSubscriptionsResponse_Clear
 *===========================================================================*/
void UA_TransferSubscriptionsResponse_Clear(UA_TransferSubscriptionsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(UA_TransferResult), (UA_EncodeableObject_PfnClear*) UA_TransferResult_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_TransferSubscriptionsResponse_Encode
 *===========================================================================*/
StatusCode UA_TransferSubscriptionsResponse_Encode(UA_TransferSubscriptionsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(UA_TransferResult), (UA_EncodeableObject_PfnEncode*) UA_TransferResult_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_TransferSubscriptionsResponse_Decode
 *===========================================================================*/
StatusCode UA_TransferSubscriptionsResponse_Decode(UA_TransferSubscriptionsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_TransferSubscriptionsResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(UA_TransferResult), (UA_EncodeableObject_PfnDecode*) UA_TransferResult_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_TransferSubscriptionsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_TransferSubscriptionsResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_TransferSubscriptionsResponse_EncodeableType =
{
    "TransferSubscriptionsResponse",
    OpcUaId_TransferSubscriptionsResponse,
    OpcUaId_TransferSubscriptionsResponse_Encoding_DefaultBinary,
    OpcUaId_TransferSubscriptionsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_TransferSubscriptionsResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_TransferSubscriptionsResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_TransferSubscriptionsResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_TransferSubscriptionsResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_TransferSubscriptionsResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_TransferSubscriptionsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
#ifndef OPCUA_EXCLUDE_DeleteSubscriptionsRequest
/*============================================================================
 * UA_DeleteSubscriptionsRequest_Initialize
 *===========================================================================*/
void UA_DeleteSubscriptionsRequest_Initialize(UA_DeleteSubscriptionsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UA_Initialize_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                            sizeof(uint32_t), (UA_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * UA_DeleteSubscriptionsRequest_Clear
 *===========================================================================*/
void UA_DeleteSubscriptionsRequest_Clear(UA_DeleteSubscriptionsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_RequestHeader_Clear(&a_pValue->RequestHeader);
        UA_Clear_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                       sizeof(uint32_t), (UA_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * UA_DeleteSubscriptionsRequest_Encode
 *===========================================================================*/
StatusCode UA_DeleteSubscriptionsRequest_Encode(UA_DeleteSubscriptionsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                   sizeof(uint32_t), (UA_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteSubscriptionsRequest_Decode
 *===========================================================================*/
StatusCode UA_DeleteSubscriptionsRequest_Decode(UA_DeleteSubscriptionsRequest* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteSubscriptionsRequest_Initialize(a_pValue);

    status &= UA_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                  sizeof(uint32_t), (UA_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        UA_DeleteSubscriptionsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteSubscriptionsRequest_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteSubscriptionsRequest_EncodeableType =
{
    "DeleteSubscriptionsRequest",
    OpcUaId_DeleteSubscriptionsRequest,
    OpcUaId_DeleteSubscriptionsRequest_Encoding_DefaultBinary,
    OpcUaId_DeleteSubscriptionsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteSubscriptionsRequest),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteSubscriptionsRequest_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteSubscriptionsRequest_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteSubscriptionsRequest_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteSubscriptionsRequest_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteSubscriptionsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptionsResponse
/*============================================================================
 * UA_DeleteSubscriptionsResponse_Initialize
 *===========================================================================*/
void UA_DeleteSubscriptionsResponse_Initialize(UA_DeleteSubscriptionsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UA_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(StatusCode), (UA_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * UA_DeleteSubscriptionsResponse_Clear
 *===========================================================================*/
void UA_DeleteSubscriptionsResponse_Clear(UA_DeleteSubscriptionsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UA_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(StatusCode), (UA_EncodeableObject_PfnClear*) StatusCode_Clear);
        UA_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * UA_DeleteSubscriptionsResponse_Encode
 *===========================================================================*/
StatusCode UA_DeleteSubscriptionsResponse_Encode(UA_DeleteSubscriptionsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(StatusCode), (UA_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DeleteSubscriptionsResponse_Decode
 *===========================================================================*/
StatusCode UA_DeleteSubscriptionsResponse_Decode(UA_DeleteSubscriptionsResponse* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DeleteSubscriptionsResponse_Initialize(a_pValue);

    status &= UA_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(StatusCode), (UA_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(UA_DiagnosticInfo), (UA_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        UA_DeleteSubscriptionsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DeleteSubscriptionsResponse_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DeleteSubscriptionsResponse_EncodeableType =
{
    "DeleteSubscriptionsResponse",
    OpcUaId_DeleteSubscriptionsResponse,
    OpcUaId_DeleteSubscriptionsResponse_Encoding_DefaultBinary,
    OpcUaId_DeleteSubscriptionsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DeleteSubscriptionsResponse),
    (UA_EncodeableObject_PfnInitialize*)UA_DeleteSubscriptionsResponse_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DeleteSubscriptionsResponse_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DeleteSubscriptionsResponse_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DeleteSubscriptionsResponse_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DeleteSubscriptionsResponse_Decode
};
#endif
#endif


#ifndef OPCUA_EXCLUDE_BuildInfo
/*============================================================================
 * UA_BuildInfo_Initialize
 *===========================================================================*/
void UA_BuildInfo_Initialize(UA_BuildInfo* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ProductUri);
        String_Initialize(&a_pValue->ManufacturerName);
        String_Initialize(&a_pValue->ProductName);
        String_Initialize(&a_pValue->SoftwareVersion);
        String_Initialize(&a_pValue->BuildNumber);
        DateTime_Initialize(&a_pValue->BuildDate);
    }
}

/*============================================================================
 * UA_BuildInfo_Clear
 *===========================================================================*/
void UA_BuildInfo_Clear(UA_BuildInfo* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ProductUri);
        String_Clear(&a_pValue->ManufacturerName);
        String_Clear(&a_pValue->ProductName);
        String_Clear(&a_pValue->SoftwareVersion);
        String_Clear(&a_pValue->BuildNumber);
        DateTime_Clear(&a_pValue->BuildDate);
    }
}

/*============================================================================
 * UA_BuildInfo_Encode
 *===========================================================================*/
StatusCode UA_BuildInfo_Encode(UA_BuildInfo* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ProductUri, msgBuf);
    status &= String_Write(&a_pValue->ManufacturerName, msgBuf);
    status &= String_Write(&a_pValue->ProductName, msgBuf);
    status &= String_Write(&a_pValue->SoftwareVersion, msgBuf);
    status &= String_Write(&a_pValue->BuildNumber, msgBuf);
    status &= DateTime_Write(&a_pValue->BuildDate, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_BuildInfo_Decode
 *===========================================================================*/
StatusCode UA_BuildInfo_Decode(UA_BuildInfo* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_BuildInfo_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ProductUri, msgBuf);
    status &= String_Read(&a_pValue->ManufacturerName, msgBuf);
    status &= String_Read(&a_pValue->ProductName, msgBuf);
    status &= String_Read(&a_pValue->SoftwareVersion, msgBuf);
    status &= String_Read(&a_pValue->BuildNumber, msgBuf);
    status &= DateTime_Read(&a_pValue->BuildDate, msgBuf);

    if(status != STATUS_OK){
        UA_BuildInfo_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_BuildInfo_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_BuildInfo_EncodeableType =
{
    "BuildInfo",
    OpcUaId_BuildInfo,
    OpcUaId_BuildInfo_Encoding_DefaultBinary,
    OpcUaId_BuildInfo_Encoding_DefaultXml,
    NULL,
    sizeof(UA_BuildInfo),
    (UA_EncodeableObject_PfnInitialize*)UA_BuildInfo_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_BuildInfo_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_BuildInfo_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_BuildInfo_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_BuildInfo_Decode
};
#endif



#ifndef OPCUA_EXCLUDE_RedundantServerDataType
/*============================================================================
 * UA_RedundantServerDataType_Initialize
 *===========================================================================*/
void UA_RedundantServerDataType_Initialize(UA_RedundantServerDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ServerId);
        Byte_Initialize(&a_pValue->ServiceLevel);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->ServerState);
    }
}

/*============================================================================
 * UA_RedundantServerDataType_Clear
 *===========================================================================*/
void UA_RedundantServerDataType_Clear(UA_RedundantServerDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ServerId);
        Byte_Clear(&a_pValue->ServiceLevel);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->ServerState);
    }
}

/*============================================================================
 * UA_RedundantServerDataType_Encode
 *===========================================================================*/
StatusCode UA_RedundantServerDataType_Encode(UA_RedundantServerDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ServerId, msgBuf);
    status &= Byte_Write(&a_pValue->ServiceLevel, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ServerState);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_RedundantServerDataType_Decode
 *===========================================================================*/
StatusCode UA_RedundantServerDataType_Decode(UA_RedundantServerDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_RedundantServerDataType_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ServerId, msgBuf);
    status &= Byte_Read(&a_pValue->ServiceLevel, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ServerState);

    if(status != STATUS_OK){
        UA_RedundantServerDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_RedundantServerDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_RedundantServerDataType_EncodeableType =
{
    "RedundantServerDataType",
    OpcUaId_RedundantServerDataType,
    OpcUaId_RedundantServerDataType_Encoding_DefaultBinary,
    OpcUaId_RedundantServerDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_RedundantServerDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_RedundantServerDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_RedundantServerDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_RedundantServerDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_RedundantServerDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_RedundantServerDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EndpointUrlListDataType
/*============================================================================
 * UA_EndpointUrlListDataType_Initialize
 *===========================================================================*/
void UA_EndpointUrlListDataType_Initialize(UA_EndpointUrlListDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Initialize_Array(&a_pValue->NoOfEndpointUrlList, (void**) &a_pValue->EndpointUrlList, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * UA_EndpointUrlListDataType_Clear
 *===========================================================================*/
void UA_EndpointUrlListDataType_Clear(UA_EndpointUrlListDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_Clear_Array(&a_pValue->NoOfEndpointUrlList, (void**) &a_pValue->EndpointUrlList, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * UA_EndpointUrlListDataType_Encode
 *===========================================================================*/
StatusCode UA_EndpointUrlListDataType_Encode(UA_EndpointUrlListDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfEndpointUrlList, (void**) &a_pValue->EndpointUrlList, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EndpointUrlListDataType_Decode
 *===========================================================================*/
StatusCode UA_EndpointUrlListDataType_Decode(UA_EndpointUrlListDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EndpointUrlListDataType_Initialize(a_pValue);

    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfEndpointUrlList, (void**) &a_pValue->EndpointUrlList, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        UA_EndpointUrlListDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EndpointUrlListDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EndpointUrlListDataType_EncodeableType =
{
    "EndpointUrlListDataType",
    OpcUaId_EndpointUrlListDataType,
    OpcUaId_EndpointUrlListDataType_Encoding_DefaultBinary,
    OpcUaId_EndpointUrlListDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EndpointUrlListDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_EndpointUrlListDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EndpointUrlListDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EndpointUrlListDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EndpointUrlListDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EndpointUrlListDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_NetworkGroupDataType
/*============================================================================
 * UA_NetworkGroupDataType_Initialize
 *===========================================================================*/
void UA_NetworkGroupDataType_Initialize(UA_NetworkGroupDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ServerUri);
        UA_Initialize_Array(&a_pValue->NoOfNetworkPaths, (void**) &a_pValue->NetworkPaths, 
                            sizeof(UA_EndpointUrlListDataType), (UA_EncodeableObject_PfnInitialize*) UA_EndpointUrlListDataType_Initialize);
    }
}

/*============================================================================
 * UA_NetworkGroupDataType_Clear
 *===========================================================================*/
void UA_NetworkGroupDataType_Clear(UA_NetworkGroupDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ServerUri);
        UA_Clear_Array(&a_pValue->NoOfNetworkPaths, (void**) &a_pValue->NetworkPaths, 
                       sizeof(UA_EndpointUrlListDataType), (UA_EncodeableObject_PfnClear*) UA_EndpointUrlListDataType_Clear);
    }
}

/*============================================================================
 * UA_NetworkGroupDataType_Encode
 *===========================================================================*/
StatusCode UA_NetworkGroupDataType_Encode(UA_NetworkGroupDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ServerUri, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfNetworkPaths, (void**) &a_pValue->NetworkPaths, 
                   sizeof(UA_EndpointUrlListDataType), (UA_EncodeableObject_PfnEncode*) UA_EndpointUrlListDataType_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_NetworkGroupDataType_Decode
 *===========================================================================*/
StatusCode UA_NetworkGroupDataType_Decode(UA_NetworkGroupDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_NetworkGroupDataType_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ServerUri, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfNetworkPaths, (void**) &a_pValue->NetworkPaths, 
                  sizeof(UA_EndpointUrlListDataType), (UA_EncodeableObject_PfnDecode*) UA_EndpointUrlListDataType_Decode);

    if(status != STATUS_OK){
        UA_NetworkGroupDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_NetworkGroupDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_NetworkGroupDataType_EncodeableType =
{
    "NetworkGroupDataType",
    OpcUaId_NetworkGroupDataType,
    OpcUaId_NetworkGroupDataType_Encoding_DefaultBinary,
    OpcUaId_NetworkGroupDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_NetworkGroupDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_NetworkGroupDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_NetworkGroupDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_NetworkGroupDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_NetworkGroupDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_NetworkGroupDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SamplingIntervalDiagnosticsDataType
/*============================================================================
 * UA_SamplingIntervalDiagnosticsDataType_Initialize
 *===========================================================================*/
void UA_SamplingIntervalDiagnosticsDataType_Initialize(UA_SamplingIntervalDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Initialize(&a_pValue->SamplingInterval);
        UInt32_Initialize(&a_pValue->MonitoredItemCount);
        UInt32_Initialize(&a_pValue->MaxMonitoredItemCount);
        UInt32_Initialize(&a_pValue->DisabledMonitoredItemCount);
    }
}

/*============================================================================
 * UA_SamplingIntervalDiagnosticsDataType_Clear
 *===========================================================================*/
void UA_SamplingIntervalDiagnosticsDataType_Clear(UA_SamplingIntervalDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Clear(&a_pValue->SamplingInterval);
        UInt32_Clear(&a_pValue->MonitoredItemCount);
        UInt32_Clear(&a_pValue->MaxMonitoredItemCount);
        UInt32_Clear(&a_pValue->DisabledMonitoredItemCount);
    }
}

/*============================================================================
 * UA_SamplingIntervalDiagnosticsDataType_Encode
 *===========================================================================*/
StatusCode UA_SamplingIntervalDiagnosticsDataType_Encode(UA_SamplingIntervalDiagnosticsDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Double_Write(&a_pValue->SamplingInterval, msgBuf);
    status &= UInt32_Write(&a_pValue->MonitoredItemCount, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxMonitoredItemCount, msgBuf);
    status &= UInt32_Write(&a_pValue->DisabledMonitoredItemCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SamplingIntervalDiagnosticsDataType_Decode
 *===========================================================================*/
StatusCode UA_SamplingIntervalDiagnosticsDataType_Decode(UA_SamplingIntervalDiagnosticsDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SamplingIntervalDiagnosticsDataType_Initialize(a_pValue);

    status &= Double_Read(&a_pValue->SamplingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->MonitoredItemCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxMonitoredItemCount, msgBuf);
    status &= UInt32_Read(&a_pValue->DisabledMonitoredItemCount, msgBuf);

    if(status != STATUS_OK){
        UA_SamplingIntervalDiagnosticsDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SamplingIntervalDiagnosticsDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SamplingIntervalDiagnosticsDataType_EncodeableType =
{
    "SamplingIntervalDiagnosticsDataType",
    OpcUaId_SamplingIntervalDiagnosticsDataType,
    OpcUaId_SamplingIntervalDiagnosticsDataType_Encoding_DefaultBinary,
    OpcUaId_SamplingIntervalDiagnosticsDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SamplingIntervalDiagnosticsDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_SamplingIntervalDiagnosticsDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SamplingIntervalDiagnosticsDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SamplingIntervalDiagnosticsDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SamplingIntervalDiagnosticsDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SamplingIntervalDiagnosticsDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ServerDiagnosticsSummaryDataType
/*============================================================================
 * UA_ServerDiagnosticsSummaryDataType_Initialize
 *===========================================================================*/
void UA_ServerDiagnosticsSummaryDataType_Initialize(UA_ServerDiagnosticsSummaryDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->ServerViewCount);
        UInt32_Initialize(&a_pValue->CurrentSessionCount);
        UInt32_Initialize(&a_pValue->CumulatedSessionCount);
        UInt32_Initialize(&a_pValue->SecurityRejectedSessionCount);
        UInt32_Initialize(&a_pValue->RejectedSessionCount);
        UInt32_Initialize(&a_pValue->SessionTimeoutCount);
        UInt32_Initialize(&a_pValue->SessionAbortCount);
        UInt32_Initialize(&a_pValue->CurrentSubscriptionCount);
        UInt32_Initialize(&a_pValue->CumulatedSubscriptionCount);
        UInt32_Initialize(&a_pValue->PublishingIntervalCount);
        UInt32_Initialize(&a_pValue->SecurityRejectedRequestsCount);
        UInt32_Initialize(&a_pValue->RejectedRequestsCount);
    }
}

/*============================================================================
 * UA_ServerDiagnosticsSummaryDataType_Clear
 *===========================================================================*/
void UA_ServerDiagnosticsSummaryDataType_Clear(UA_ServerDiagnosticsSummaryDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->ServerViewCount);
        UInt32_Clear(&a_pValue->CurrentSessionCount);
        UInt32_Clear(&a_pValue->CumulatedSessionCount);
        UInt32_Clear(&a_pValue->SecurityRejectedSessionCount);
        UInt32_Clear(&a_pValue->RejectedSessionCount);
        UInt32_Clear(&a_pValue->SessionTimeoutCount);
        UInt32_Clear(&a_pValue->SessionAbortCount);
        UInt32_Clear(&a_pValue->CurrentSubscriptionCount);
        UInt32_Clear(&a_pValue->CumulatedSubscriptionCount);
        UInt32_Clear(&a_pValue->PublishingIntervalCount);
        UInt32_Clear(&a_pValue->SecurityRejectedRequestsCount);
        UInt32_Clear(&a_pValue->RejectedRequestsCount);
    }
}

/*============================================================================
 * UA_ServerDiagnosticsSummaryDataType_Encode
 *===========================================================================*/
StatusCode UA_ServerDiagnosticsSummaryDataType_Encode(UA_ServerDiagnosticsSummaryDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->ServerViewCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentSessionCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CumulatedSessionCount, msgBuf);
    status &= UInt32_Write(&a_pValue->SecurityRejectedSessionCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RejectedSessionCount, msgBuf);
    status &= UInt32_Write(&a_pValue->SessionTimeoutCount, msgBuf);
    status &= UInt32_Write(&a_pValue->SessionAbortCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentSubscriptionCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CumulatedSubscriptionCount, msgBuf);
    status &= UInt32_Write(&a_pValue->PublishingIntervalCount, msgBuf);
    status &= UInt32_Write(&a_pValue->SecurityRejectedRequestsCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RejectedRequestsCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ServerDiagnosticsSummaryDataType_Decode
 *===========================================================================*/
StatusCode UA_ServerDiagnosticsSummaryDataType_Decode(UA_ServerDiagnosticsSummaryDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ServerDiagnosticsSummaryDataType_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->ServerViewCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentSessionCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CumulatedSessionCount, msgBuf);
    status &= UInt32_Read(&a_pValue->SecurityRejectedSessionCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RejectedSessionCount, msgBuf);
    status &= UInt32_Read(&a_pValue->SessionTimeoutCount, msgBuf);
    status &= UInt32_Read(&a_pValue->SessionAbortCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentSubscriptionCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CumulatedSubscriptionCount, msgBuf);
    status &= UInt32_Read(&a_pValue->PublishingIntervalCount, msgBuf);
    status &= UInt32_Read(&a_pValue->SecurityRejectedRequestsCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RejectedRequestsCount, msgBuf);

    if(status != STATUS_OK){
        UA_ServerDiagnosticsSummaryDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ServerDiagnosticsSummaryDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ServerDiagnosticsSummaryDataType_EncodeableType =
{
    "ServerDiagnosticsSummaryDataType",
    OpcUaId_ServerDiagnosticsSummaryDataType,
    OpcUaId_ServerDiagnosticsSummaryDataType_Encoding_DefaultBinary,
    OpcUaId_ServerDiagnosticsSummaryDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ServerDiagnosticsSummaryDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_ServerDiagnosticsSummaryDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ServerDiagnosticsSummaryDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ServerDiagnosticsSummaryDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ServerDiagnosticsSummaryDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ServerDiagnosticsSummaryDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ServerStatusDataType
/*============================================================================
 * UA_ServerStatusDataType_Initialize
 *===========================================================================*/
void UA_ServerStatusDataType_Initialize(UA_ServerStatusDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->StartTime);
        DateTime_Initialize(&a_pValue->CurrentTime);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->State);
        UA_BuildInfo_Initialize(&a_pValue->BuildInfo);
        UInt32_Initialize(&a_pValue->SecondsTillShutdown);
        LocalizedText_Initialize(&a_pValue->ShutdownReason);
    }
}

/*============================================================================
 * UA_ServerStatusDataType_Clear
 *===========================================================================*/
void UA_ServerStatusDataType_Clear(UA_ServerStatusDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->StartTime);
        DateTime_Clear(&a_pValue->CurrentTime);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->State);
        UA_BuildInfo_Clear(&a_pValue->BuildInfo);
        UInt32_Clear(&a_pValue->SecondsTillShutdown);
        LocalizedText_Clear(&a_pValue->ShutdownReason);
    }
}

/*============================================================================
 * UA_ServerStatusDataType_Encode
 *===========================================================================*/
StatusCode UA_ServerStatusDataType_Encode(UA_ServerStatusDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Write(&a_pValue->CurrentTime, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->State);
    status &= UA_BuildInfo_Encode(&a_pValue->BuildInfo, msgBuf);
    status &= UInt32_Write(&a_pValue->SecondsTillShutdown, msgBuf);
    status &= LocalizedText_Write(&a_pValue->ShutdownReason, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ServerStatusDataType_Decode
 *===========================================================================*/
StatusCode UA_ServerStatusDataType_Decode(UA_ServerStatusDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ServerStatusDataType_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->CurrentTime, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->State);
    status &= UA_BuildInfo_Decode(&a_pValue->BuildInfo, msgBuf);
    status &= UInt32_Read(&a_pValue->SecondsTillShutdown, msgBuf);
    status &= LocalizedText_Read(&a_pValue->ShutdownReason, msgBuf);

    if(status != STATUS_OK){
        UA_ServerStatusDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ServerStatusDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ServerStatusDataType_EncodeableType =
{
    "ServerStatusDataType",
    OpcUaId_ServerStatusDataType,
    OpcUaId_ServerStatusDataType_Encoding_DefaultBinary,
    OpcUaId_ServerStatusDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ServerStatusDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_ServerStatusDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ServerStatusDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ServerStatusDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ServerStatusDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ServerStatusDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SessionDiagnosticsDataType
/*============================================================================
 * UA_SessionDiagnosticsDataType_Initialize
 *===========================================================================*/
void UA_SessionDiagnosticsDataType_Initialize(UA_SessionDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->SessionId);
        String_Initialize(&a_pValue->SessionName);
        UA_ApplicationDescription_Initialize(&a_pValue->ClientDescription);
        String_Initialize(&a_pValue->ServerUri);
        String_Initialize(&a_pValue->EndpointUrl);
        UA_Initialize_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
        Double_Initialize(&a_pValue->ActualSessionTimeout);
        UInt32_Initialize(&a_pValue->MaxResponseMessageSize);
        DateTime_Initialize(&a_pValue->ClientConnectionTime);
        DateTime_Initialize(&a_pValue->ClientLastContactTime);
        UInt32_Initialize(&a_pValue->CurrentSubscriptionsCount);
        UInt32_Initialize(&a_pValue->CurrentMonitoredItemsCount);
        UInt32_Initialize(&a_pValue->CurrentPublishRequestsInQueue);
        UA_ServiceCounterDataType_Initialize(&a_pValue->TotalRequestCount);
        UInt32_Initialize(&a_pValue->UnauthorizedRequestCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->ReadCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->HistoryReadCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->WriteCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->HistoryUpdateCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->CallCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->CreateMonitoredItemsCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->ModifyMonitoredItemsCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->SetMonitoringModeCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->SetTriggeringCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->DeleteMonitoredItemsCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->CreateSubscriptionCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->ModifySubscriptionCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->SetPublishingModeCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->PublishCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->RepublishCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->TransferSubscriptionsCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->DeleteSubscriptionsCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->AddNodesCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->AddReferencesCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->DeleteNodesCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->DeleteReferencesCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->BrowseCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->BrowseNextCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->TranslateBrowsePathsToNodeIdsCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->QueryFirstCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->QueryNextCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->RegisterNodesCount);
        UA_ServiceCounterDataType_Initialize(&a_pValue->UnregisterNodesCount);
    }
}

/*============================================================================
 * UA_SessionDiagnosticsDataType_Clear
 *===========================================================================*/
void UA_SessionDiagnosticsDataType_Clear(UA_SessionDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->SessionId);
        String_Clear(&a_pValue->SessionName);
        UA_ApplicationDescription_Clear(&a_pValue->ClientDescription);
        String_Clear(&a_pValue->ServerUri);
        String_Clear(&a_pValue->EndpointUrl);
        UA_Clear_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
        Double_Clear(&a_pValue->ActualSessionTimeout);
        UInt32_Clear(&a_pValue->MaxResponseMessageSize);
        DateTime_Clear(&a_pValue->ClientConnectionTime);
        DateTime_Clear(&a_pValue->ClientLastContactTime);
        UInt32_Clear(&a_pValue->CurrentSubscriptionsCount);
        UInt32_Clear(&a_pValue->CurrentMonitoredItemsCount);
        UInt32_Clear(&a_pValue->CurrentPublishRequestsInQueue);
        UA_ServiceCounterDataType_Clear(&a_pValue->TotalRequestCount);
        UInt32_Clear(&a_pValue->UnauthorizedRequestCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->ReadCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->HistoryReadCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->WriteCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->HistoryUpdateCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->CallCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->CreateMonitoredItemsCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->ModifyMonitoredItemsCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->SetMonitoringModeCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->SetTriggeringCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->DeleteMonitoredItemsCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->CreateSubscriptionCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->ModifySubscriptionCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->SetPublishingModeCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->PublishCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->RepublishCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->TransferSubscriptionsCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->DeleteSubscriptionsCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->AddNodesCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->AddReferencesCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->DeleteNodesCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->DeleteReferencesCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->BrowseCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->BrowseNextCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->TranslateBrowsePathsToNodeIdsCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->QueryFirstCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->QueryNextCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->RegisterNodesCount);
        UA_ServiceCounterDataType_Clear(&a_pValue->UnregisterNodesCount);
    }
}

/*============================================================================
 * UA_SessionDiagnosticsDataType_Encode
 *===========================================================================*/
StatusCode UA_SessionDiagnosticsDataType_Encode(UA_SessionDiagnosticsDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->SessionId, msgBuf);
    status &= String_Write(&a_pValue->SessionName, msgBuf);
    status &= UA_ApplicationDescription_Encode(&a_pValue->ClientDescription, msgBuf);
    status &= String_Write(&a_pValue->ServerUri, msgBuf);
    status &= String_Write(&a_pValue->EndpointUrl, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);
    status &= Double_Write(&a_pValue->ActualSessionTimeout, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxResponseMessageSize, msgBuf);
    status &= DateTime_Write(&a_pValue->ClientConnectionTime, msgBuf);
    status &= DateTime_Write(&a_pValue->ClientLastContactTime, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentSubscriptionsCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentMonitoredItemsCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentPublishRequestsInQueue, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->TotalRequestCount, msgBuf);
    status &= UInt32_Write(&a_pValue->UnauthorizedRequestCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->ReadCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->HistoryReadCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->WriteCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->HistoryUpdateCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->CallCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->CreateMonitoredItemsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->ModifyMonitoredItemsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->SetMonitoringModeCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->SetTriggeringCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->DeleteMonitoredItemsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->CreateSubscriptionCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->ModifySubscriptionCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->SetPublishingModeCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->PublishCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->RepublishCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->TransferSubscriptionsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->DeleteSubscriptionsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->AddNodesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->AddReferencesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->DeleteNodesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->DeleteReferencesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->BrowseCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->BrowseNextCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->TranslateBrowsePathsToNodeIdsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->QueryFirstCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->QueryNextCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->RegisterNodesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Encode(&a_pValue->UnregisterNodesCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SessionDiagnosticsDataType_Decode
 *===========================================================================*/
StatusCode UA_SessionDiagnosticsDataType_Decode(UA_SessionDiagnosticsDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SessionDiagnosticsDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->SessionId, msgBuf);
    status &= String_Read(&a_pValue->SessionName, msgBuf);
    status &= UA_ApplicationDescription_Decode(&a_pValue->ClientDescription, msgBuf);
    status &= String_Read(&a_pValue->ServerUri, msgBuf);
    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);
    status &= Double_Read(&a_pValue->ActualSessionTimeout, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxResponseMessageSize, msgBuf);
    status &= DateTime_Read(&a_pValue->ClientConnectionTime, msgBuf);
    status &= DateTime_Read(&a_pValue->ClientLastContactTime, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentSubscriptionsCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentMonitoredItemsCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentPublishRequestsInQueue, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->TotalRequestCount, msgBuf);
    status &= UInt32_Read(&a_pValue->UnauthorizedRequestCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->ReadCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->HistoryReadCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->WriteCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->HistoryUpdateCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->CallCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->CreateMonitoredItemsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->ModifyMonitoredItemsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->SetMonitoringModeCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->SetTriggeringCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->DeleteMonitoredItemsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->CreateSubscriptionCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->ModifySubscriptionCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->SetPublishingModeCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->PublishCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->RepublishCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->TransferSubscriptionsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->DeleteSubscriptionsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->AddNodesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->AddReferencesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->DeleteNodesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->DeleteReferencesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->BrowseCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->BrowseNextCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->TranslateBrowsePathsToNodeIdsCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->QueryFirstCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->QueryNextCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->RegisterNodesCount, msgBuf);
    status &= UA_ServiceCounterDataType_Decode(&a_pValue->UnregisterNodesCount, msgBuf);

    if(status != STATUS_OK){
        UA_SessionDiagnosticsDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SessionDiagnosticsDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SessionDiagnosticsDataType_EncodeableType =
{
    "SessionDiagnosticsDataType",
    OpcUaId_SessionDiagnosticsDataType,
    OpcUaId_SessionDiagnosticsDataType_Encoding_DefaultBinary,
    OpcUaId_SessionDiagnosticsDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SessionDiagnosticsDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_SessionDiagnosticsDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SessionDiagnosticsDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SessionDiagnosticsDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SessionDiagnosticsDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SessionDiagnosticsDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SessionSecurityDiagnosticsDataType
/*============================================================================
 * UA_SessionSecurityDiagnosticsDataType_Initialize
 *===========================================================================*/
void UA_SessionSecurityDiagnosticsDataType_Initialize(UA_SessionSecurityDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->SessionId);
        String_Initialize(&a_pValue->ClientUserIdOfSession);
        UA_Initialize_Array(&a_pValue->NoOfClientUserIdHistory, (void**) &a_pValue->ClientUserIdHistory, 
                            sizeof(UA_String), (UA_EncodeableObject_PfnInitialize*) String_Initialize);
        String_Initialize(&a_pValue->AuthenticationMechanism);
        String_Initialize(&a_pValue->Encoding);
        String_Initialize(&a_pValue->TransportProtocol);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        String_Initialize(&a_pValue->SecurityPolicyUri);
        ByteString_Initialize(&a_pValue->ClientCertificate);
    }
}

/*============================================================================
 * UA_SessionSecurityDiagnosticsDataType_Clear
 *===========================================================================*/
void UA_SessionSecurityDiagnosticsDataType_Clear(UA_SessionSecurityDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->SessionId);
        String_Clear(&a_pValue->ClientUserIdOfSession);
        UA_Clear_Array(&a_pValue->NoOfClientUserIdHistory, (void**) &a_pValue->ClientUserIdHistory, 
                       sizeof(UA_String), (UA_EncodeableObject_PfnClear*) String_Clear);
        String_Clear(&a_pValue->AuthenticationMechanism);
        String_Clear(&a_pValue->Encoding);
        String_Clear(&a_pValue->TransportProtocol);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        String_Clear(&a_pValue->SecurityPolicyUri);
        ByteString_Clear(&a_pValue->ClientCertificate);
    }
}

/*============================================================================
 * UA_SessionSecurityDiagnosticsDataType_Encode
 *===========================================================================*/
StatusCode UA_SessionSecurityDiagnosticsDataType_Encode(UA_SessionSecurityDiagnosticsDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->SessionId, msgBuf);
    status &= String_Write(&a_pValue->ClientUserIdOfSession, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfClientUserIdHistory, (void**) &a_pValue->ClientUserIdHistory, 
                   sizeof(UA_String), (UA_EncodeableObject_PfnEncode*) String_Write);
    status &= String_Write(&a_pValue->AuthenticationMechanism, msgBuf);
    status &= String_Write(&a_pValue->Encoding, msgBuf);
    status &= String_Write(&a_pValue->TransportProtocol, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= String_Write(&a_pValue->SecurityPolicyUri, msgBuf);
    status &= ByteString_Write(&a_pValue->ClientCertificate, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SessionSecurityDiagnosticsDataType_Decode
 *===========================================================================*/
StatusCode UA_SessionSecurityDiagnosticsDataType_Decode(UA_SessionSecurityDiagnosticsDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SessionSecurityDiagnosticsDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->SessionId, msgBuf);
    status &= String_Read(&a_pValue->ClientUserIdOfSession, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfClientUserIdHistory, (void**) &a_pValue->ClientUserIdHistory, 
                  sizeof(UA_String), (UA_EncodeableObject_PfnDecode*) String_Read);
    status &= String_Read(&a_pValue->AuthenticationMechanism, msgBuf);
    status &= String_Read(&a_pValue->Encoding, msgBuf);
    status &= String_Read(&a_pValue->TransportProtocol, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= String_Read(&a_pValue->SecurityPolicyUri, msgBuf);
    status &= ByteString_Read(&a_pValue->ClientCertificate, msgBuf);

    if(status != STATUS_OK){
        UA_SessionSecurityDiagnosticsDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SessionSecurityDiagnosticsDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SessionSecurityDiagnosticsDataType_EncodeableType =
{
    "SessionSecurityDiagnosticsDataType",
    OpcUaId_SessionSecurityDiagnosticsDataType,
    OpcUaId_SessionSecurityDiagnosticsDataType_Encoding_DefaultBinary,
    OpcUaId_SessionSecurityDiagnosticsDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SessionSecurityDiagnosticsDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_SessionSecurityDiagnosticsDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SessionSecurityDiagnosticsDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SessionSecurityDiagnosticsDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SessionSecurityDiagnosticsDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SessionSecurityDiagnosticsDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ServiceCounterDataType
/*============================================================================
 * UA_ServiceCounterDataType_Initialize
 *===========================================================================*/
void UA_ServiceCounterDataType_Initialize(UA_ServiceCounterDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->TotalCount);
        UInt32_Initialize(&a_pValue->ErrorCount);
    }
}

/*============================================================================
 * UA_ServiceCounterDataType_Clear
 *===========================================================================*/
void UA_ServiceCounterDataType_Clear(UA_ServiceCounterDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->TotalCount);
        UInt32_Clear(&a_pValue->ErrorCount);
    }
}

/*============================================================================
 * UA_ServiceCounterDataType_Encode
 *===========================================================================*/
StatusCode UA_ServiceCounterDataType_Encode(UA_ServiceCounterDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->TotalCount, msgBuf);
    status &= UInt32_Write(&a_pValue->ErrorCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ServiceCounterDataType_Decode
 *===========================================================================*/
StatusCode UA_ServiceCounterDataType_Decode(UA_ServiceCounterDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ServiceCounterDataType_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->TotalCount, msgBuf);
    status &= UInt32_Read(&a_pValue->ErrorCount, msgBuf);

    if(status != STATUS_OK){
        UA_ServiceCounterDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ServiceCounterDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ServiceCounterDataType_EncodeableType =
{
    "ServiceCounterDataType",
    OpcUaId_ServiceCounterDataType,
    OpcUaId_ServiceCounterDataType_Encoding_DefaultBinary,
    OpcUaId_ServiceCounterDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ServiceCounterDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_ServiceCounterDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ServiceCounterDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ServiceCounterDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ServiceCounterDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ServiceCounterDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_StatusResult
/*============================================================================
 * UA_StatusResult_Initialize
 *===========================================================================*/
void UA_StatusResult_Initialize(UA_StatusResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        DiagnosticInfo_Initialize(&a_pValue->DiagnosticInfo);
    }
}

/*============================================================================
 * UA_StatusResult_Clear
 *===========================================================================*/
void UA_StatusResult_Clear(UA_StatusResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        DiagnosticInfo_Clear(&a_pValue->DiagnosticInfo);
    }
}

/*============================================================================
 * UA_StatusResult_Encode
 *===========================================================================*/
StatusCode UA_StatusResult_Encode(UA_StatusResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= DiagnosticInfo_Write(&a_pValue->DiagnosticInfo, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_StatusResult_Decode
 *===========================================================================*/
StatusCode UA_StatusResult_Decode(UA_StatusResult* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_StatusResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= DiagnosticInfo_Read(&a_pValue->DiagnosticInfo, msgBuf);

    if(status != STATUS_OK){
        UA_StatusResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_StatusResult_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_StatusResult_EncodeableType =
{
    "StatusResult",
    OpcUaId_StatusResult,
    OpcUaId_StatusResult_Encoding_DefaultBinary,
    OpcUaId_StatusResult_Encoding_DefaultXml,
    NULL,
    sizeof(UA_StatusResult),
    (UA_EncodeableObject_PfnInitialize*)UA_StatusResult_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_StatusResult_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_StatusResult_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_StatusResult_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_StatusResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SubscriptionDiagnosticsDataType
/*============================================================================
 * UA_SubscriptionDiagnosticsDataType_Initialize
 *===========================================================================*/
void UA_SubscriptionDiagnosticsDataType_Initialize(UA_SubscriptionDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->SessionId);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        Byte_Initialize(&a_pValue->Priority);
        Double_Initialize(&a_pValue->PublishingInterval);
        UInt32_Initialize(&a_pValue->MaxKeepAliveCount);
        UInt32_Initialize(&a_pValue->MaxLifetimeCount);
        UInt32_Initialize(&a_pValue->MaxNotificationsPerPublish);
        Boolean_Initialize(&a_pValue->PublishingEnabled);
        UInt32_Initialize(&a_pValue->ModifyCount);
        UInt32_Initialize(&a_pValue->EnableCount);
        UInt32_Initialize(&a_pValue->DisableCount);
        UInt32_Initialize(&a_pValue->RepublishRequestCount);
        UInt32_Initialize(&a_pValue->RepublishMessageRequestCount);
        UInt32_Initialize(&a_pValue->RepublishMessageCount);
        UInt32_Initialize(&a_pValue->TransferRequestCount);
        UInt32_Initialize(&a_pValue->TransferredToAltClientCount);
        UInt32_Initialize(&a_pValue->TransferredToSameClientCount);
        UInt32_Initialize(&a_pValue->PublishRequestCount);
        UInt32_Initialize(&a_pValue->DataChangeNotificationsCount);
        UInt32_Initialize(&a_pValue->EventNotificationsCount);
        UInt32_Initialize(&a_pValue->NotificationsCount);
        UInt32_Initialize(&a_pValue->LatePublishRequestCount);
        UInt32_Initialize(&a_pValue->CurrentKeepAliveCount);
        UInt32_Initialize(&a_pValue->CurrentLifetimeCount);
        UInt32_Initialize(&a_pValue->UnacknowledgedMessageCount);
        UInt32_Initialize(&a_pValue->DiscardedMessageCount);
        UInt32_Initialize(&a_pValue->MonitoredItemCount);
        UInt32_Initialize(&a_pValue->DisabledMonitoredItemCount);
        UInt32_Initialize(&a_pValue->MonitoringQueueOverflowCount);
        UInt32_Initialize(&a_pValue->NextSequenceNumber);
        UInt32_Initialize(&a_pValue->EventQueueOverFlowCount);
    }
}

/*============================================================================
 * UA_SubscriptionDiagnosticsDataType_Clear
 *===========================================================================*/
void UA_SubscriptionDiagnosticsDataType_Clear(UA_SubscriptionDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->SessionId);
        UInt32_Clear(&a_pValue->SubscriptionId);
        Byte_Clear(&a_pValue->Priority);
        Double_Clear(&a_pValue->PublishingInterval);
        UInt32_Clear(&a_pValue->MaxKeepAliveCount);
        UInt32_Clear(&a_pValue->MaxLifetimeCount);
        UInt32_Clear(&a_pValue->MaxNotificationsPerPublish);
        Boolean_Clear(&a_pValue->PublishingEnabled);
        UInt32_Clear(&a_pValue->ModifyCount);
        UInt32_Clear(&a_pValue->EnableCount);
        UInt32_Clear(&a_pValue->DisableCount);
        UInt32_Clear(&a_pValue->RepublishRequestCount);
        UInt32_Clear(&a_pValue->RepublishMessageRequestCount);
        UInt32_Clear(&a_pValue->RepublishMessageCount);
        UInt32_Clear(&a_pValue->TransferRequestCount);
        UInt32_Clear(&a_pValue->TransferredToAltClientCount);
        UInt32_Clear(&a_pValue->TransferredToSameClientCount);
        UInt32_Clear(&a_pValue->PublishRequestCount);
        UInt32_Clear(&a_pValue->DataChangeNotificationsCount);
        UInt32_Clear(&a_pValue->EventNotificationsCount);
        UInt32_Clear(&a_pValue->NotificationsCount);
        UInt32_Clear(&a_pValue->LatePublishRequestCount);
        UInt32_Clear(&a_pValue->CurrentKeepAliveCount);
        UInt32_Clear(&a_pValue->CurrentLifetimeCount);
        UInt32_Clear(&a_pValue->UnacknowledgedMessageCount);
        UInt32_Clear(&a_pValue->DiscardedMessageCount);
        UInt32_Clear(&a_pValue->MonitoredItemCount);
        UInt32_Clear(&a_pValue->DisabledMonitoredItemCount);
        UInt32_Clear(&a_pValue->MonitoringQueueOverflowCount);
        UInt32_Clear(&a_pValue->NextSequenceNumber);
        UInt32_Clear(&a_pValue->EventQueueOverFlowCount);
    }
}

/*============================================================================
 * UA_SubscriptionDiagnosticsDataType_Encode
 *===========================================================================*/
StatusCode UA_SubscriptionDiagnosticsDataType_Encode(UA_SubscriptionDiagnosticsDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->SessionId, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= Byte_Write(&a_pValue->Priority, msgBuf);
    status &= Double_Write(&a_pValue->PublishingInterval, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxKeepAliveCount, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxLifetimeCount, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxNotificationsPerPublish, msgBuf);
    status &= Boolean_Write(&a_pValue->PublishingEnabled, msgBuf);
    status &= UInt32_Write(&a_pValue->ModifyCount, msgBuf);
    status &= UInt32_Write(&a_pValue->EnableCount, msgBuf);
    status &= UInt32_Write(&a_pValue->DisableCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RepublishRequestCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RepublishMessageRequestCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RepublishMessageCount, msgBuf);
    status &= UInt32_Write(&a_pValue->TransferRequestCount, msgBuf);
    status &= UInt32_Write(&a_pValue->TransferredToAltClientCount, msgBuf);
    status &= UInt32_Write(&a_pValue->TransferredToSameClientCount, msgBuf);
    status &= UInt32_Write(&a_pValue->PublishRequestCount, msgBuf);
    status &= UInt32_Write(&a_pValue->DataChangeNotificationsCount, msgBuf);
    status &= UInt32_Write(&a_pValue->EventNotificationsCount, msgBuf);
    status &= UInt32_Write(&a_pValue->NotificationsCount, msgBuf);
    status &= UInt32_Write(&a_pValue->LatePublishRequestCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentKeepAliveCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentLifetimeCount, msgBuf);
    status &= UInt32_Write(&a_pValue->UnacknowledgedMessageCount, msgBuf);
    status &= UInt32_Write(&a_pValue->DiscardedMessageCount, msgBuf);
    status &= UInt32_Write(&a_pValue->MonitoredItemCount, msgBuf);
    status &= UInt32_Write(&a_pValue->DisabledMonitoredItemCount, msgBuf);
    status &= UInt32_Write(&a_pValue->MonitoringQueueOverflowCount, msgBuf);
    status &= UInt32_Write(&a_pValue->NextSequenceNumber, msgBuf);
    status &= UInt32_Write(&a_pValue->EventQueueOverFlowCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SubscriptionDiagnosticsDataType_Decode
 *===========================================================================*/
StatusCode UA_SubscriptionDiagnosticsDataType_Decode(UA_SubscriptionDiagnosticsDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SubscriptionDiagnosticsDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->SessionId, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= Byte_Read(&a_pValue->Priority, msgBuf);
    status &= Double_Read(&a_pValue->PublishingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxKeepAliveCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxNotificationsPerPublish, msgBuf);
    status &= Boolean_Read(&a_pValue->PublishingEnabled, msgBuf);
    status &= UInt32_Read(&a_pValue->ModifyCount, msgBuf);
    status &= UInt32_Read(&a_pValue->EnableCount, msgBuf);
    status &= UInt32_Read(&a_pValue->DisableCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RepublishRequestCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RepublishMessageRequestCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RepublishMessageCount, msgBuf);
    status &= UInt32_Read(&a_pValue->TransferRequestCount, msgBuf);
    status &= UInt32_Read(&a_pValue->TransferredToAltClientCount, msgBuf);
    status &= UInt32_Read(&a_pValue->TransferredToSameClientCount, msgBuf);
    status &= UInt32_Read(&a_pValue->PublishRequestCount, msgBuf);
    status &= UInt32_Read(&a_pValue->DataChangeNotificationsCount, msgBuf);
    status &= UInt32_Read(&a_pValue->EventNotificationsCount, msgBuf);
    status &= UInt32_Read(&a_pValue->NotificationsCount, msgBuf);
    status &= UInt32_Read(&a_pValue->LatePublishRequestCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentKeepAliveCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->UnacknowledgedMessageCount, msgBuf);
    status &= UInt32_Read(&a_pValue->DiscardedMessageCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MonitoredItemCount, msgBuf);
    status &= UInt32_Read(&a_pValue->DisabledMonitoredItemCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MonitoringQueueOverflowCount, msgBuf);
    status &= UInt32_Read(&a_pValue->NextSequenceNumber, msgBuf);
    status &= UInt32_Read(&a_pValue->EventQueueOverFlowCount, msgBuf);

    if(status != STATUS_OK){
        UA_SubscriptionDiagnosticsDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SubscriptionDiagnosticsDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SubscriptionDiagnosticsDataType_EncodeableType =
{
    "SubscriptionDiagnosticsDataType",
    OpcUaId_SubscriptionDiagnosticsDataType,
    OpcUaId_SubscriptionDiagnosticsDataType_Encoding_DefaultBinary,
    OpcUaId_SubscriptionDiagnosticsDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SubscriptionDiagnosticsDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_SubscriptionDiagnosticsDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SubscriptionDiagnosticsDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SubscriptionDiagnosticsDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SubscriptionDiagnosticsDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SubscriptionDiagnosticsDataType_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_ModelChangeStructureDataType
/*============================================================================
 * UA_ModelChangeStructureDataType_Initialize
 *===========================================================================*/
void UA_ModelChangeStructureDataType_Initialize(UA_ModelChangeStructureDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->Affected);
        NodeId_Initialize(&a_pValue->AffectedType);
        Byte_Initialize(&a_pValue->Verb);
    }
}

/*============================================================================
 * UA_ModelChangeStructureDataType_Clear
 *===========================================================================*/
void UA_ModelChangeStructureDataType_Clear(UA_ModelChangeStructureDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->Affected);
        NodeId_Clear(&a_pValue->AffectedType);
        Byte_Clear(&a_pValue->Verb);
    }
}

/*============================================================================
 * UA_ModelChangeStructureDataType_Encode
 *===========================================================================*/
StatusCode UA_ModelChangeStructureDataType_Encode(UA_ModelChangeStructureDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->Affected, msgBuf);
    status &= NodeId_Write(&a_pValue->AffectedType, msgBuf);
    status &= Byte_Write(&a_pValue->Verb, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ModelChangeStructureDataType_Decode
 *===========================================================================*/
StatusCode UA_ModelChangeStructureDataType_Decode(UA_ModelChangeStructureDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ModelChangeStructureDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->Affected, msgBuf);
    status &= NodeId_Read(&a_pValue->AffectedType, msgBuf);
    status &= Byte_Read(&a_pValue->Verb, msgBuf);

    if(status != STATUS_OK){
        UA_ModelChangeStructureDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ModelChangeStructureDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ModelChangeStructureDataType_EncodeableType =
{
    "ModelChangeStructureDataType",
    OpcUaId_ModelChangeStructureDataType,
    OpcUaId_ModelChangeStructureDataType_Encoding_DefaultBinary,
    OpcUaId_ModelChangeStructureDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ModelChangeStructureDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_ModelChangeStructureDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ModelChangeStructureDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ModelChangeStructureDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ModelChangeStructureDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ModelChangeStructureDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SemanticChangeStructureDataType
/*============================================================================
 * UA_SemanticChangeStructureDataType_Initialize
 *===========================================================================*/
void UA_SemanticChangeStructureDataType_Initialize(UA_SemanticChangeStructureDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->Affected);
        NodeId_Initialize(&a_pValue->AffectedType);
    }
}

/*============================================================================
 * UA_SemanticChangeStructureDataType_Clear
 *===========================================================================*/
void UA_SemanticChangeStructureDataType_Clear(UA_SemanticChangeStructureDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->Affected);
        NodeId_Clear(&a_pValue->AffectedType);
    }
}

/*============================================================================
 * UA_SemanticChangeStructureDataType_Encode
 *===========================================================================*/
StatusCode UA_SemanticChangeStructureDataType_Encode(UA_SemanticChangeStructureDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->Affected, msgBuf);
    status &= NodeId_Write(&a_pValue->AffectedType, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_SemanticChangeStructureDataType_Decode
 *===========================================================================*/
StatusCode UA_SemanticChangeStructureDataType_Decode(UA_SemanticChangeStructureDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_SemanticChangeStructureDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->Affected, msgBuf);
    status &= NodeId_Read(&a_pValue->AffectedType, msgBuf);

    if(status != STATUS_OK){
        UA_SemanticChangeStructureDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_SemanticChangeStructureDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_SemanticChangeStructureDataType_EncodeableType =
{
    "SemanticChangeStructureDataType",
    OpcUaId_SemanticChangeStructureDataType,
    OpcUaId_SemanticChangeStructureDataType_Encoding_DefaultBinary,
    OpcUaId_SemanticChangeStructureDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_SemanticChangeStructureDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_SemanticChangeStructureDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_SemanticChangeStructureDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_SemanticChangeStructureDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_SemanticChangeStructureDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_SemanticChangeStructureDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Range
/*============================================================================
 * UA_Range_Initialize
 *===========================================================================*/
void UA_Range_Initialize(UA_Range* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Initialize(&a_pValue->Low);
        Double_Initialize(&a_pValue->High);
    }
}

/*============================================================================
 * UA_Range_Clear
 *===========================================================================*/
void UA_Range_Clear(UA_Range* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Clear(&a_pValue->Low);
        Double_Clear(&a_pValue->High);
    }
}

/*============================================================================
 * UA_Range_Encode
 *===========================================================================*/
StatusCode UA_Range_Encode(UA_Range* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Double_Write(&a_pValue->Low, msgBuf);
    status &= Double_Write(&a_pValue->High, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_Range_Decode
 *===========================================================================*/
StatusCode UA_Range_Decode(UA_Range* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_Range_Initialize(a_pValue);

    status &= Double_Read(&a_pValue->Low, msgBuf);
    status &= Double_Read(&a_pValue->High, msgBuf);

    if(status != STATUS_OK){
        UA_Range_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_Range_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_Range_EncodeableType =
{
    "Range",
    OpcUaId_Range,
    OpcUaId_Range_Encoding_DefaultBinary,
    OpcUaId_Range_Encoding_DefaultXml,
    NULL,
    sizeof(UA_Range),
    (UA_EncodeableObject_PfnInitialize*)UA_Range_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_Range_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_Range_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_Range_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_Range_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EUInformation
/*============================================================================
 * UA_EUInformation_Initialize
 *===========================================================================*/
void UA_EUInformation_Initialize(UA_EUInformation* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->NamespaceUri);
        Int32_Initialize(&a_pValue->UnitId);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
    }
}

/*============================================================================
 * UA_EUInformation_Clear
 *===========================================================================*/
void UA_EUInformation_Clear(UA_EUInformation* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->NamespaceUri);
        Int32_Clear(&a_pValue->UnitId);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
    }
}

/*============================================================================
 * UA_EUInformation_Encode
 *===========================================================================*/
StatusCode UA_EUInformation_Encode(UA_EUInformation* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->NamespaceUri, msgBuf);
    status &= Int32_Write(&a_pValue->UnitId, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_EUInformation_Decode
 *===========================================================================*/
StatusCode UA_EUInformation_Decode(UA_EUInformation* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_EUInformation_Initialize(a_pValue);

    status &= String_Read(&a_pValue->NamespaceUri, msgBuf);
    status &= Int32_Read(&a_pValue->UnitId, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);

    if(status != STATUS_OK){
        UA_EUInformation_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_EUInformation_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_EUInformation_EncodeableType =
{
    "EUInformation",
    OpcUaId_EUInformation,
    OpcUaId_EUInformation_Encoding_DefaultBinary,
    OpcUaId_EUInformation_Encoding_DefaultXml,
    NULL,
    sizeof(UA_EUInformation),
    (UA_EncodeableObject_PfnInitialize*)UA_EUInformation_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_EUInformation_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_EUInformation_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_EUInformation_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_EUInformation_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_ComplexNumberType
/*============================================================================
 * UA_ComplexNumberType_Initialize
 *===========================================================================*/
void UA_ComplexNumberType_Initialize(UA_ComplexNumberType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Float_Initialize(&a_pValue->Real);
        Float_Initialize(&a_pValue->Imaginary);
    }
}

/*============================================================================
 * UA_ComplexNumberType_Clear
 *===========================================================================*/
void UA_ComplexNumberType_Clear(UA_ComplexNumberType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Float_Clear(&a_pValue->Real);
        Float_Clear(&a_pValue->Imaginary);
    }
}

/*============================================================================
 * UA_ComplexNumberType_Encode
 *===========================================================================*/
StatusCode UA_ComplexNumberType_Encode(UA_ComplexNumberType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Float_Write(&a_pValue->Real, msgBuf);
    status &= Float_Write(&a_pValue->Imaginary, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ComplexNumberType_Decode
 *===========================================================================*/
StatusCode UA_ComplexNumberType_Decode(UA_ComplexNumberType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ComplexNumberType_Initialize(a_pValue);

    status &= Float_Read(&a_pValue->Real, msgBuf);
    status &= Float_Read(&a_pValue->Imaginary, msgBuf);

    if(status != STATUS_OK){
        UA_ComplexNumberType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ComplexNumberType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ComplexNumberType_EncodeableType =
{
    "ComplexNumberType",
    OpcUaId_ComplexNumberType,
    OpcUaId_ComplexNumberType_Encoding_DefaultBinary,
    OpcUaId_ComplexNumberType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ComplexNumberType),
    (UA_EncodeableObject_PfnInitialize*)UA_ComplexNumberType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ComplexNumberType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ComplexNumberType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ComplexNumberType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ComplexNumberType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DoubleComplexNumberType
/*============================================================================
 * UA_DoubleComplexNumberType_Initialize
 *===========================================================================*/
void UA_DoubleComplexNumberType_Initialize(UA_DoubleComplexNumberType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Initialize(&a_pValue->Real);
        Double_Initialize(&a_pValue->Imaginary);
    }
}

/*============================================================================
 * UA_DoubleComplexNumberType_Clear
 *===========================================================================*/
void UA_DoubleComplexNumberType_Clear(UA_DoubleComplexNumberType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Clear(&a_pValue->Real);
        Double_Clear(&a_pValue->Imaginary);
    }
}

/*============================================================================
 * UA_DoubleComplexNumberType_Encode
 *===========================================================================*/
StatusCode UA_DoubleComplexNumberType_Encode(UA_DoubleComplexNumberType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Double_Write(&a_pValue->Real, msgBuf);
    status &= Double_Write(&a_pValue->Imaginary, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_DoubleComplexNumberType_Decode
 *===========================================================================*/
StatusCode UA_DoubleComplexNumberType_Decode(UA_DoubleComplexNumberType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_DoubleComplexNumberType_Initialize(a_pValue);

    status &= Double_Read(&a_pValue->Real, msgBuf);
    status &= Double_Read(&a_pValue->Imaginary, msgBuf);

    if(status != STATUS_OK){
        UA_DoubleComplexNumberType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_DoubleComplexNumberType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_DoubleComplexNumberType_EncodeableType =
{
    "DoubleComplexNumberType",
    OpcUaId_DoubleComplexNumberType,
    OpcUaId_DoubleComplexNumberType_Encoding_DefaultBinary,
    OpcUaId_DoubleComplexNumberType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_DoubleComplexNumberType),
    (UA_EncodeableObject_PfnInitialize*)UA_DoubleComplexNumberType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_DoubleComplexNumberType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_DoubleComplexNumberType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_DoubleComplexNumberType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_DoubleComplexNumberType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AxisInformation
/*============================================================================
 * UA_AxisInformation_Initialize
 *===========================================================================*/
void UA_AxisInformation_Initialize(UA_AxisInformation* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_EUInformation_Initialize(&a_pValue->EngineeringUnits);
        UA_Range_Initialize(&a_pValue->EURange);
        LocalizedText_Initialize(&a_pValue->Title);
        UA_Initialize_EnumeratedType((int32_t*) &a_pValue->AxisScaleType);
        UA_Initialize_Array(&a_pValue->NoOfAxisSteps, (void**) &a_pValue->AxisSteps, 
                            sizeof(double), (UA_EncodeableObject_PfnInitialize*) Double_Initialize);
    }
}

/*============================================================================
 * UA_AxisInformation_Clear
 *===========================================================================*/
void UA_AxisInformation_Clear(UA_AxisInformation* a_pValue)
{
    if (a_pValue != NULL)
    {
        UA_EUInformation_Clear(&a_pValue->EngineeringUnits);
        UA_Range_Clear(&a_pValue->EURange);
        LocalizedText_Clear(&a_pValue->Title);
        UA_Clear_EnumeratedType((int32_t*) &a_pValue->AxisScaleType);
        UA_Clear_Array(&a_pValue->NoOfAxisSteps, (void**) &a_pValue->AxisSteps, 
                       sizeof(double), (UA_EncodeableObject_PfnClear*) Double_Clear);
    }
}

/*============================================================================
 * UA_AxisInformation_Encode
 *===========================================================================*/
StatusCode UA_AxisInformation_Encode(UA_AxisInformation* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UA_EUInformation_Encode(&a_pValue->EngineeringUnits, msgBuf);
    status &= UA_Range_Encode(&a_pValue->EURange, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Title, msgBuf);
    status &= UA_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->AxisScaleType);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfAxisSteps, (void**) &a_pValue->AxisSteps, 
                   sizeof(double), (UA_EncodeableObject_PfnEncode*) Double_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_AxisInformation_Decode
 *===========================================================================*/
StatusCode UA_AxisInformation_Decode(UA_AxisInformation* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_AxisInformation_Initialize(a_pValue);

    status &= UA_EUInformation_Decode(&a_pValue->EngineeringUnits, msgBuf);
    status &= UA_Range_Decode(&a_pValue->EURange, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Title, msgBuf);
    status &= UA_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->AxisScaleType);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfAxisSteps, (void**) &a_pValue->AxisSteps, 
                  sizeof(double), (UA_EncodeableObject_PfnDecode*) Double_Read);

    if(status != STATUS_OK){
        UA_AxisInformation_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_AxisInformation_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_AxisInformation_EncodeableType =
{
    "AxisInformation",
    OpcUaId_AxisInformation,
    OpcUaId_AxisInformation_Encoding_DefaultBinary,
    OpcUaId_AxisInformation_Encoding_DefaultXml,
    NULL,
    sizeof(UA_AxisInformation),
    (UA_EncodeableObject_PfnInitialize*)UA_AxisInformation_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_AxisInformation_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_AxisInformation_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_AxisInformation_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_AxisInformation_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_XVType
/*============================================================================
 * UA_XVType_Initialize
 *===========================================================================*/
void UA_XVType_Initialize(UA_XVType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Initialize(&a_pValue->X);
        Float_Initialize(&a_pValue->Value);
    }
}

/*============================================================================
 * UA_XVType_Clear
 *===========================================================================*/
void UA_XVType_Clear(UA_XVType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Clear(&a_pValue->X);
        Float_Clear(&a_pValue->Value);
    }
}

/*============================================================================
 * UA_XVType_Encode
 *===========================================================================*/
StatusCode UA_XVType_Encode(UA_XVType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= Double_Write(&a_pValue->X, msgBuf);
    status &= Float_Write(&a_pValue->Value, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_XVType_Decode
 *===========================================================================*/
StatusCode UA_XVType_Decode(UA_XVType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_XVType_Initialize(a_pValue);

    status &= Double_Read(&a_pValue->X, msgBuf);
    status &= Float_Read(&a_pValue->Value, msgBuf);

    if(status != STATUS_OK){
        UA_XVType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_XVType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_XVType_EncodeableType =
{
    "XVType",
    OpcUaId_XVType,
    OpcUaId_XVType_Encoding_DefaultBinary,
    OpcUaId_XVType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_XVType),
    (UA_EncodeableObject_PfnInitialize*)UA_XVType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_XVType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_XVType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_XVType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_XVType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ProgramDiagnosticDataType
/*============================================================================
 * UA_ProgramDiagnosticDataType_Initialize
 *===========================================================================*/
void UA_ProgramDiagnosticDataType_Initialize(UA_ProgramDiagnosticDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->CreateSessionId);
        String_Initialize(&a_pValue->CreateClientName);
        DateTime_Initialize(&a_pValue->InvocationCreationTime);
        DateTime_Initialize(&a_pValue->LastTransitionTime);
        String_Initialize(&a_pValue->LastMethodCall);
        NodeId_Initialize(&a_pValue->LastMethodSessionId);
        UA_Initialize_Array(&a_pValue->NoOfLastMethodInputArguments, (void**) &a_pValue->LastMethodInputArguments, 
                            sizeof(UA_Argument), (UA_EncodeableObject_PfnInitialize*) UA_Argument_Initialize);
        UA_Initialize_Array(&a_pValue->NoOfLastMethodOutputArguments, (void**) &a_pValue->LastMethodOutputArguments, 
                            sizeof(UA_Argument), (UA_EncodeableObject_PfnInitialize*) UA_Argument_Initialize);
        DateTime_Initialize(&a_pValue->LastMethodCallTime);
        UA_StatusResult_Initialize(&a_pValue->LastMethodReturnStatus);
    }
}

/*============================================================================
 * UA_ProgramDiagnosticDataType_Clear
 *===========================================================================*/
void UA_ProgramDiagnosticDataType_Clear(UA_ProgramDiagnosticDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->CreateSessionId);
        String_Clear(&a_pValue->CreateClientName);
        DateTime_Clear(&a_pValue->InvocationCreationTime);
        DateTime_Clear(&a_pValue->LastTransitionTime);
        String_Clear(&a_pValue->LastMethodCall);
        NodeId_Clear(&a_pValue->LastMethodSessionId);
        UA_Clear_Array(&a_pValue->NoOfLastMethodInputArguments, (void**) &a_pValue->LastMethodInputArguments, 
                       sizeof(UA_Argument), (UA_EncodeableObject_PfnClear*) UA_Argument_Clear);
        UA_Clear_Array(&a_pValue->NoOfLastMethodOutputArguments, (void**) &a_pValue->LastMethodOutputArguments, 
                       sizeof(UA_Argument), (UA_EncodeableObject_PfnClear*) UA_Argument_Clear);
        DateTime_Clear(&a_pValue->LastMethodCallTime);
        UA_StatusResult_Clear(&a_pValue->LastMethodReturnStatus);
    }
}

/*============================================================================
 * UA_ProgramDiagnosticDataType_Encode
 *===========================================================================*/
StatusCode UA_ProgramDiagnosticDataType_Encode(UA_ProgramDiagnosticDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->CreateSessionId, msgBuf);
    status &= String_Write(&a_pValue->CreateClientName, msgBuf);
    status &= DateTime_Write(&a_pValue->InvocationCreationTime, msgBuf);
    status &= DateTime_Write(&a_pValue->LastTransitionTime, msgBuf);
    status &= String_Write(&a_pValue->LastMethodCall, msgBuf);
    status &= NodeId_Write(&a_pValue->LastMethodSessionId, msgBuf);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfLastMethodInputArguments, (void**) &a_pValue->LastMethodInputArguments, 
                   sizeof(UA_Argument), (UA_EncodeableObject_PfnEncode*) UA_Argument_Encode);
    status &= UA_Write_Array(msgBuf, &a_pValue->NoOfLastMethodOutputArguments, (void**) &a_pValue->LastMethodOutputArguments, 
                   sizeof(UA_Argument), (UA_EncodeableObject_PfnEncode*) UA_Argument_Encode);
    status &= DateTime_Write(&a_pValue->LastMethodCallTime, msgBuf);
    status &= UA_StatusResult_Encode(&a_pValue->LastMethodReturnStatus, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_ProgramDiagnosticDataType_Decode
 *===========================================================================*/
StatusCode UA_ProgramDiagnosticDataType_Decode(UA_ProgramDiagnosticDataType* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_ProgramDiagnosticDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->CreateSessionId, msgBuf);
    status &= String_Read(&a_pValue->CreateClientName, msgBuf);
    status &= DateTime_Read(&a_pValue->InvocationCreationTime, msgBuf);
    status &= DateTime_Read(&a_pValue->LastTransitionTime, msgBuf);
    status &= String_Read(&a_pValue->LastMethodCall, msgBuf);
    status &= NodeId_Read(&a_pValue->LastMethodSessionId, msgBuf);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfLastMethodInputArguments, (void**) &a_pValue->LastMethodInputArguments, 
                  sizeof(UA_Argument), (UA_EncodeableObject_PfnDecode*) UA_Argument_Decode);
    status &= UA_Read_Array(msgBuf, &a_pValue->NoOfLastMethodOutputArguments, (void**) &a_pValue->LastMethodOutputArguments, 
                  sizeof(UA_Argument), (UA_EncodeableObject_PfnDecode*) UA_Argument_Decode);
    status &= DateTime_Read(&a_pValue->LastMethodCallTime, msgBuf);
    status &= UA_StatusResult_Decode(&a_pValue->LastMethodReturnStatus, msgBuf);

    if(status != STATUS_OK){
        UA_ProgramDiagnosticDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_ProgramDiagnosticDataType_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_ProgramDiagnosticDataType_EncodeableType =
{
    "ProgramDiagnosticDataType",
    OpcUaId_ProgramDiagnosticDataType,
    OpcUaId_ProgramDiagnosticDataType_Encoding_DefaultBinary,
    OpcUaId_ProgramDiagnosticDataType_Encoding_DefaultXml,
    NULL,
    sizeof(UA_ProgramDiagnosticDataType),
    (UA_EncodeableObject_PfnInitialize*)UA_ProgramDiagnosticDataType_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_ProgramDiagnosticDataType_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_ProgramDiagnosticDataType_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_ProgramDiagnosticDataType_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_ProgramDiagnosticDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Annotation
/*============================================================================
 * UA_Annotation_Initialize
 *===========================================================================*/
void UA_Annotation_Initialize(UA_Annotation* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->Message);
        String_Initialize(&a_pValue->UserName);
        DateTime_Initialize(&a_pValue->AnnotationTime);
    }
}

/*============================================================================
 * UA_Annotation_Clear
 *===========================================================================*/
void UA_Annotation_Clear(UA_Annotation* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->Message);
        String_Clear(&a_pValue->UserName);
        DateTime_Clear(&a_pValue->AnnotationTime);
    }
}

/*============================================================================
 * UA_Annotation_Encode
 *===========================================================================*/
StatusCode UA_Annotation_Encode(UA_Annotation* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->Message, msgBuf);
    status &= String_Write(&a_pValue->UserName, msgBuf);
    status &= DateTime_Write(&a_pValue->AnnotationTime, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * UA_Annotation_Decode
 *===========================================================================*/
StatusCode UA_Annotation_Decode(UA_Annotation* a_pValue, UA_MsgBuffer* msgBuf)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    UA_Annotation_Initialize(a_pValue);

    status &= String_Read(&a_pValue->Message, msgBuf);
    status &= String_Read(&a_pValue->UserName, msgBuf);
    status &= DateTime_Read(&a_pValue->AnnotationTime, msgBuf);

    if(status != STATUS_OK){
        UA_Annotation_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * UA_Annotation_EncodeableType
 *===========================================================================*/
struct UA_EncodeableType UA_Annotation_EncodeableType =
{
    "Annotation",
    OpcUaId_Annotation,
    OpcUaId_Annotation_Encoding_DefaultBinary,
    OpcUaId_Annotation_Encoding_DefaultXml,
    NULL,
    sizeof(UA_Annotation),
    (UA_EncodeableObject_PfnInitialize*)UA_Annotation_Initialize,
    (UA_EncodeableObject_PfnClear*)UA_Annotation_Clear,
    (UA_EncodeableObject_PfnGetSize*)NULL,
//    (UA_EncodeableObject_PfnGetSize*)UA_Annotation_GetSize,
    (UA_EncodeableObject_PfnEncode*)UA_Annotation_Encode,
    (UA_EncodeableObject_PfnDecode*)UA_Annotation_Decode
};
#endif


void UA_Initialize_EnumeratedType(int32_t* enumerationValue)
{
    *enumerationValue = 0;
}

void UA_Initialize_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                         UA_EncodeableObject_PfnInitialize* initFct)
{
    (void) initFct;
    (void) sizeOfElt;
    *noOfElts = 0;
    *eltsArray = NULL;
}

void UA_Clear_EnumeratedType(int32_t* enumerationValue){
    *enumerationValue = 0;
}

void UA_Clear_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                    UA_EncodeableObject_PfnClear* clearFct)
{
    int32_t idx = 0;
    uint32_t pos = 0;
    UA_Byte* byteArray = *eltsArray;
    for (idx = 0; idx < *noOfElts; idx ++){
        pos = idx * sizeOfElt;
        clearFct(&(byteArray[pos]));
    }
    
    free(*eltsArray);
    
    *noOfElts = 0;
    *eltsArray = NULL;
}

StatusCode UA_Read_EnumeratedType(UA_MsgBuffer* msgBuffer, int32_t* enumerationValue){
    return Int32_Read(enumerationValue, msgBuffer);
}

StatusCode UA_Read_Array(UA_MsgBuffer* msgBuffer, int32_t* noOfElts, void** eltsArray,
                         size_t sizeOfElt, UA_EncodeableObject_PfnDecode* decodeFct)
{
    assert(msgBuffer != NULL && *eltsArray == NULL && noOfElts != NULL);
    StatusCode status = STATUS_OK;
    UA_Byte* byteArray = *eltsArray;
    status = Int32_Read(noOfElts, msgBuffer);
    if(status == STATUS_OK && *noOfElts > 0){
        *eltsArray = malloc (sizeOfElt * *noOfElts);
    }
    
    if(eltsArray != NULL){
        int32_t idx = 0;
        uint32_t pos = 0;
        for (idx = 0; status == STATUS_OK && idx < *noOfElts; idx ++){
            pos = idx * sizeOfElt;
            status = decodeFct(&(byteArray[pos]), msgBuffer);
        }
        
        if(status != STATUS_OK){
            free(*eltsArray);
            *eltsArray = NULL;
        }
        
    }else{
        status = STATUS_NOK;
    }
    
    return status;
}
                    
StatusCode UA_Write_EnumeratedType(UA_MsgBuffer* msgBuffer, int32_t* enumerationValue){
    return Int32_Write(enumerationValue, msgBuffer);
}

StatusCode UA_Write_Array(UA_MsgBuffer* msgBuffer, int32_t* noOfElts, void** eltsArray,
                          size_t sizeOfElt, UA_EncodeableObject_PfnEncode* encodeFct){
    assert(msgBuffer != NULL && *eltsArray != NULL && noOfElts != NULL);
    StatusCode status = STATUS_OK;
    UA_Byte* byteArray = *eltsArray;
    
    status = Int32_Write(noOfElts, msgBuffer);
    if(status == STATUS_OK && *noOfElts > 0){
        int32_t idx = 0;
        uint32_t pos = 0;
        for (idx = 0; status == STATUS_OK && idx < *noOfElts; idx ++){
            pos = idx * sizeOfElt;
            status = encodeFct(&(byteArray[pos]), msgBuffer);
        }
    }
    return status;
}

/*============================================================================
 * Table of known types.
 *===========================================================================*/
static UA_EncodeableType* g_KnownEncodeableTypes[] =
{
    #ifndef OPCUA_EXCLUDE_Node
    &UA_Node_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_InstanceNode
    &UA_InstanceNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TypeNode
    &UA_TypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ObjectNode
    &UA_ObjectNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ObjectTypeNode
    &UA_ObjectTypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_VariableNode
    &UA_VariableNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_VariableTypeNode
    &UA_VariableTypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReferenceTypeNode
    &UA_ReferenceTypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MethodNode
    &UA_MethodNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ViewNode
    &UA_ViewNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DataTypeNode
    &UA_DataTypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReferenceNode
    &UA_ReferenceNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Argument
    &UA_Argument_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EnumValueType
    &UA_EnumValueType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EnumField
    &UA_EnumField_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_OptionSet
    &UA_OptionSet_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TimeZoneDataType
    &UA_TimeZoneDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ApplicationDescription
    &UA_ApplicationDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RequestHeader
    &UA_RequestHeader_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ResponseHeader
    &UA_ResponseHeader_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServiceFault
    &UA_ServiceFault_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_FindServers
    &UA_FindServersRequest_EncodeableType,
    &UA_FindServersResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServerOnNetwork
    &UA_ServerOnNetwork_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_FindServersOnNetwork
    &UA_FindServersOnNetworkRequest_EncodeableType,
    &UA_FindServersOnNetworkResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UserTokenPolicy
    &UA_UserTokenPolicy_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EndpointDescription
    &UA_EndpointDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_GetEndpoints
    &UA_GetEndpointsRequest_EncodeableType,
    &UA_GetEndpointsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisteredServer
    &UA_RegisteredServer_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisterServer
    &UA_RegisterServerRequest_EncodeableType,
    &UA_RegisterServerResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MdnsDiscoveryConfiguration
    &UA_MdnsDiscoveryConfiguration_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisterServer2
    &UA_RegisterServer2Request_EncodeableType,
    &UA_RegisterServer2Response_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ChannelSecurityToken
    &UA_ChannelSecurityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_OpenSecureChannel
    &UA_OpenSecureChannelRequest_EncodeableType,
    &UA_OpenSecureChannelResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CloseSecureChannel
    &UA_CloseSecureChannelRequest_EncodeableType,
    &UA_CloseSecureChannelResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SignedSoftwareCertificate
    &UA_SignedSoftwareCertificate_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SignatureData
    &UA_SignatureData_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CreateSession
    &UA_CreateSessionRequest_EncodeableType,
    &UA_CreateSessionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UserIdentityToken
    &UA_UserIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AnonymousIdentityToken
    &UA_AnonymousIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UserNameIdentityToken
    &UA_UserNameIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_X509IdentityToken
    &UA_X509IdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_KerberosIdentityToken
    &UA_KerberosIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_IssuedIdentityToken
    &UA_IssuedIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ActivateSession
    &UA_ActivateSessionRequest_EncodeableType,
    &UA_ActivateSessionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CloseSession
    &UA_CloseSessionRequest_EncodeableType,
    &UA_CloseSessionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Cancel
    &UA_CancelRequest_EncodeableType,
    &UA_CancelResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NodeAttributes
    &UA_NodeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ObjectAttributes
    &UA_ObjectAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_VariableAttributes
    &UA_VariableAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MethodAttributes
    &UA_MethodAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ObjectTypeAttributes
    &UA_ObjectTypeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_VariableTypeAttributes
    &UA_VariableTypeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReferenceTypeAttributes
    &UA_ReferenceTypeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DataTypeAttributes
    &UA_DataTypeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ViewAttributes
    &UA_ViewAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddNodesItem
    &UA_AddNodesItem_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddNodesResult
    &UA_AddNodesResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddNodes
    &UA_AddNodesRequest_EncodeableType,
    &UA_AddNodesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddReferencesItem
    &UA_AddReferencesItem_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddReferences
    &UA_AddReferencesRequest_EncodeableType,
    &UA_AddReferencesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteNodesItem
    &UA_DeleteNodesItem_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteNodes
    &UA_DeleteNodesRequest_EncodeableType,
    &UA_DeleteNodesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteReferencesItem
    &UA_DeleteReferencesItem_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteReferences
    &UA_DeleteReferencesRequest_EncodeableType,
    &UA_DeleteReferencesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ViewDescription
    &UA_ViewDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowseDescription
    &UA_BrowseDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReferenceDescription
    &UA_ReferenceDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowseResult
    &UA_BrowseResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Browse
    &UA_BrowseRequest_EncodeableType,
    &UA_BrowseResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowseNext
    &UA_BrowseNextRequest_EncodeableType,
    &UA_BrowseNextResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RelativePathElement
    &UA_RelativePathElement_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RelativePath
    &UA_RelativePath_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowsePath
    &UA_BrowsePath_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowsePathTarget
    &UA_BrowsePathTarget_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowsePathResult
    &UA_BrowsePathResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
    &UA_TranslateBrowsePathsToNodeIdsRequest_EncodeableType,
    &UA_TranslateBrowsePathsToNodeIdsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisterNodes
    &UA_RegisterNodesRequest_EncodeableType,
    &UA_RegisterNodesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UnregisterNodes
    &UA_UnregisterNodesRequest_EncodeableType,
    &UA_UnregisterNodesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EndpointConfiguration
    &UA_EndpointConfiguration_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SupportedProfile
    &UA_SupportedProfile_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SoftwareCertificate
    &UA_SoftwareCertificate_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryDataDescription
    &UA_QueryDataDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NodeTypeDescription
    &UA_NodeTypeDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryDataSet
    &UA_QueryDataSet_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NodeReference
    &UA_NodeReference_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ContentFilterElement
    &UA_ContentFilterElement_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ContentFilter
    &UA_ContentFilter_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ElementOperand
    &UA_ElementOperand_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_LiteralOperand
    &UA_LiteralOperand_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AttributeOperand
    &UA_AttributeOperand_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SimpleAttributeOperand
    &UA_SimpleAttributeOperand_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ContentFilterElementResult
    &UA_ContentFilterElementResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ContentFilterResult
    &UA_ContentFilterResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ParsingResult
    &UA_ParsingResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryFirst
    &UA_QueryFirstRequest_EncodeableType,
    &UA_QueryFirstResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryNext
    &UA_QueryNextRequest_EncodeableType,
    &UA_QueryNextResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadValueId
    &UA_ReadValueId_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Read
    &UA_ReadRequest_EncodeableType,
    &UA_ReadResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryReadValueId
    &UA_HistoryReadValueId_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryReadResult
    &UA_HistoryReadResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadEventDetails
    &UA_ReadEventDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadRawModifiedDetails
    &UA_ReadRawModifiedDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadProcessedDetails
    &UA_ReadProcessedDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadAtTimeDetails
    &UA_ReadAtTimeDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryData
    &UA_HistoryData_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModificationInfo
    &UA_ModificationInfo_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryModifiedData
    &UA_HistoryModifiedData_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryEvent
    &UA_HistoryEvent_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryRead
    &UA_HistoryReadRequest_EncodeableType,
    &UA_HistoryReadResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_WriteValue
    &UA_WriteValue_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Write
    &UA_WriteRequest_EncodeableType,
    &UA_WriteResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryUpdateDetails
    &UA_HistoryUpdateDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UpdateDataDetails
    &UA_UpdateDataDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UpdateStructureDataDetails
    &UA_UpdateStructureDataDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UpdateEventDetails
    &UA_UpdateEventDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteRawModifiedDetails
    &UA_DeleteRawModifiedDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteAtTimeDetails
    &UA_DeleteAtTimeDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteEventDetails
    &UA_DeleteEventDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryUpdateResult
    &UA_HistoryUpdateResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryUpdate
    &UA_HistoryUpdateRequest_EncodeableType,
    &UA_HistoryUpdateResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CallMethodRequest
    &UA_CallMethodRequest_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CallMethodResult
    &UA_CallMethodResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Call
    &UA_CallRequest_EncodeableType,
    &UA_CallResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DataChangeFilter
    &UA_DataChangeFilter_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EventFilter
    &UA_EventFilter_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AggregateConfiguration
    &UA_AggregateConfiguration_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AggregateFilter
    &UA_AggregateFilter_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EventFilterResult
    &UA_EventFilterResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AggregateFilterResult
    &UA_AggregateFilterResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoringParameters
    &UA_MonitoringParameters_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemCreateRequest
    &UA_MonitoredItemCreateRequest_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemCreateResult
    &UA_MonitoredItemCreateResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CreateMonitoredItems
    &UA_CreateMonitoredItemsRequest_EncodeableType,
    &UA_CreateMonitoredItemsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemModifyRequest
    &UA_MonitoredItemModifyRequest_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemModifyResult
    &UA_MonitoredItemModifyResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
    &UA_ModifyMonitoredItemsRequest_EncodeableType,
    &UA_ModifyMonitoredItemsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SetMonitoringMode
    &UA_SetMonitoringModeRequest_EncodeableType,
    &UA_SetMonitoringModeResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SetTriggering
    &UA_SetTriggeringRequest_EncodeableType,
    &UA_SetTriggeringResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
    &UA_DeleteMonitoredItemsRequest_EncodeableType,
    &UA_DeleteMonitoredItemsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CreateSubscription
    &UA_CreateSubscriptionRequest_EncodeableType,
    &UA_CreateSubscriptionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModifySubscription
    &UA_ModifySubscriptionRequest_EncodeableType,
    &UA_ModifySubscriptionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SetPublishingMode
    &UA_SetPublishingModeRequest_EncodeableType,
    &UA_SetPublishingModeResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NotificationMessage
    &UA_NotificationMessage_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DataChangeNotification
    &UA_DataChangeNotification_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemNotification
    &UA_MonitoredItemNotification_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EventNotificationList
    &UA_EventNotificationList_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EventFieldList
    &UA_EventFieldList_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryEventFieldList
    &UA_HistoryEventFieldList_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_StatusChangeNotification
    &UA_StatusChangeNotification_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SubscriptionAcknowledgement
    &UA_SubscriptionAcknowledgement_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Publish
    &UA_PublishRequest_EncodeableType,
    &UA_PublishResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Republish
    &UA_RepublishRequest_EncodeableType,
    &UA_RepublishResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TransferResult
    &UA_TransferResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TransferSubscriptions
    &UA_TransferSubscriptionsRequest_EncodeableType,
    &UA_TransferSubscriptionsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteSubscriptions
    &UA_DeleteSubscriptionsRequest_EncodeableType,
    &UA_DeleteSubscriptionsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BuildInfo
    &UA_BuildInfo_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RedundantServerDataType
    &UA_RedundantServerDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EndpointUrlListDataType
    &UA_EndpointUrlListDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NetworkGroupDataType
    &UA_NetworkGroupDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SamplingIntervalDiagnosticsDataType
    &UA_SamplingIntervalDiagnosticsDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServerDiagnosticsSummaryDataType
    &UA_ServerDiagnosticsSummaryDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServerStatusDataType
    &UA_ServerStatusDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SessionDiagnosticsDataType
    &UA_SessionDiagnosticsDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SessionSecurityDiagnosticsDataType
    &UA_SessionSecurityDiagnosticsDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServiceCounterDataType
    &UA_ServiceCounterDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_StatusResult
    &UA_StatusResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SubscriptionDiagnosticsDataType
    &UA_SubscriptionDiagnosticsDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModelChangeStructureDataType
    &UA_ModelChangeStructureDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SemanticChangeStructureDataType
    &UA_SemanticChangeStructureDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Range
    &UA_Range_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EUInformation
    &UA_EUInformation_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ComplexNumberType
    &UA_ComplexNumberType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DoubleComplexNumberType
    &UA_DoubleComplexNumberType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AxisInformation
    &UA_AxisInformation_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_XVType
    &UA_XVType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ProgramDiagnosticDataType
    &UA_ProgramDiagnosticDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Annotation
    &UA_Annotation_EncodeableType,
    #endif
    NULL
};

UA_EncodeableType** UA_KnownEncodeableTypes = g_KnownEncodeableTypes;
/* This is the last line of an autogenerated file. */
