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

/* self */
#include "sopc_types.h"

/* stack */
#include <sopc_encoder.h>

/* types */
#include <opcua_identifiers.h>


#ifndef OPCUA_EXCLUDE_Node
/*============================================================================
 * OpcUa_Node_Initialize
 *===========================================================================*/
void OpcUa_Node_Initialize(OpcUa_Node* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
    }
}

/*============================================================================
 * OpcUa_Node_Clear
 *===========================================================================*/
void OpcUa_Node_Clear(OpcUa_Node* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
    }
}

/*============================================================================
 * OpcUa_Node_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_Node_Encode(OpcUa_Node* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_Node_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_Node_Decode(OpcUa_Node* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_Node_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);

    if(status != STATUS_OK){
        OpcUa_Node_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_Node_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_Node_EncodeableType =
{
    "Node",
    OpcUaId_Node,
    OpcUaId_Node_Encoding_DefaultBinary,
    OpcUaId_Node_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_Node),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_Node_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_Node_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_Node_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_Node_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_Node_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_InstanceNode
/*============================================================================
 * OpcUa_InstanceNode_Initialize
 *===========================================================================*/
void OpcUa_InstanceNode_Initialize(OpcUa_InstanceNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
    }
}

/*============================================================================
 * OpcUa_InstanceNode_Clear
 *===========================================================================*/
void OpcUa_InstanceNode_Clear(OpcUa_InstanceNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
    }
}

/*============================================================================
 * OpcUa_InstanceNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_InstanceNode_Encode(OpcUa_InstanceNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_InstanceNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_InstanceNode_Decode(OpcUa_InstanceNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_InstanceNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);

    if(status != STATUS_OK){
        OpcUa_InstanceNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_InstanceNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_InstanceNode_EncodeableType =
{
    "InstanceNode",
    OpcUaId_InstanceNode,
    OpcUaId_InstanceNode_Encoding_DefaultBinary,
    OpcUaId_InstanceNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_InstanceNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_InstanceNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_InstanceNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_InstanceNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_InstanceNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_InstanceNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TypeNode
/*============================================================================
 * OpcUa_TypeNode_Initialize
 *===========================================================================*/
void OpcUa_TypeNode_Initialize(OpcUa_TypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
    }
}

/*============================================================================
 * OpcUa_TypeNode_Clear
 *===========================================================================*/
void OpcUa_TypeNode_Clear(OpcUa_TypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
    }
}

/*============================================================================
 * OpcUa_TypeNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TypeNode_Encode(OpcUa_TypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_TypeNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TypeNode_Decode(OpcUa_TypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_TypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);

    if(status != STATUS_OK){
        OpcUa_TypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_TypeNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_TypeNode_EncodeableType =
{
    "TypeNode",
    OpcUaId_TypeNode,
    OpcUaId_TypeNode_Encoding_DefaultBinary,
    OpcUaId_TypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_TypeNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_TypeNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_TypeNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_TypeNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_TypeNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_TypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ObjectNode
/*============================================================================
 * OpcUa_ObjectNode_Initialize
 *===========================================================================*/
void OpcUa_ObjectNode_Initialize(OpcUa_ObjectNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
        Byte_Initialize(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * OpcUa_ObjectNode_Clear
 *===========================================================================*/
void OpcUa_ObjectNode_Clear(OpcUa_ObjectNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
        Byte_Clear(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * OpcUa_ObjectNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ObjectNode_Encode(OpcUa_ObjectNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);
    status &= Byte_Write(&a_pValue->EventNotifier, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ObjectNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ObjectNode_Decode(OpcUa_ObjectNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ObjectNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);
    status &= Byte_Read(&a_pValue->EventNotifier, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ObjectNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ObjectNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ObjectNode_EncodeableType =
{
    "ObjectNode",
    OpcUaId_ObjectNode,
    OpcUaId_ObjectNode_Encoding_DefaultBinary,
    OpcUaId_ObjectNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ObjectNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ObjectNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ObjectNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ObjectNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ObjectNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ObjectNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeNode
/*============================================================================
 * OpcUa_ObjectTypeNode_Initialize
 *===========================================================================*/
void OpcUa_ObjectTypeNode_Initialize(OpcUa_ObjectTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * OpcUa_ObjectTypeNode_Clear
 *===========================================================================*/
void OpcUa_ObjectTypeNode_Clear(OpcUa_ObjectTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * OpcUa_ObjectTypeNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ObjectTypeNode_Encode(OpcUa_ObjectTypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ObjectTypeNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ObjectTypeNode_Decode(OpcUa_ObjectTypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ObjectTypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ObjectTypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ObjectTypeNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ObjectTypeNode_EncodeableType =
{
    "ObjectTypeNode",
    OpcUaId_ObjectTypeNode,
    OpcUaId_ObjectTypeNode_Encoding_DefaultBinary,
    OpcUaId_ObjectTypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ObjectTypeNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ObjectTypeNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ObjectTypeNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ObjectTypeNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ObjectTypeNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ObjectTypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_VariableNode
/*============================================================================
 * OpcUa_VariableNode_Initialize
 *===========================================================================*/
void OpcUa_VariableNode_Initialize(OpcUa_VariableNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
        Variant_Initialize(&a_pValue->Value);
        NodeId_Initialize(&a_pValue->DataType);
        Int32_Initialize(&a_pValue->ValueRank);
        SOPC_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Byte_Initialize(&a_pValue->AccessLevel);
        Byte_Initialize(&a_pValue->UserAccessLevel);
        Double_Initialize(&a_pValue->MinimumSamplingInterval);
        Boolean_Initialize(&a_pValue->Historizing);
    }
}

/*============================================================================
 * OpcUa_VariableNode_Clear
 *===========================================================================*/
void OpcUa_VariableNode_Clear(OpcUa_VariableNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
        Variant_Clear(&a_pValue->Value);
        NodeId_Clear(&a_pValue->DataType);
        Int32_Clear(&a_pValue->ValueRank);
        SOPC_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
        Byte_Clear(&a_pValue->AccessLevel);
        Byte_Clear(&a_pValue->UserAccessLevel);
        Double_Clear(&a_pValue->MinimumSamplingInterval);
        Boolean_Clear(&a_pValue->Historizing);
    }
}

/*============================================================================
 * OpcUa_VariableNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_VariableNode_Encode(OpcUa_VariableNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);
    status &= Variant_Write(&a_pValue->Value, msgBuf);
    status &= NodeId_Write(&a_pValue->DataType, msgBuf);
    status &= Int32_Write(&a_pValue->ValueRank, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);
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
 * OpcUa_VariableNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_VariableNode_Decode(OpcUa_VariableNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_VariableNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);
    status &= Variant_Read(&a_pValue->Value, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Byte_Read(&a_pValue->AccessLevel, msgBuf);
    status &= Byte_Read(&a_pValue->UserAccessLevel, msgBuf);
    status &= Double_Read(&a_pValue->MinimumSamplingInterval, msgBuf);
    status &= Boolean_Read(&a_pValue->Historizing, msgBuf);

    if(status != STATUS_OK){
        OpcUa_VariableNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_VariableNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_VariableNode_EncodeableType =
{
    "VariableNode",
    OpcUaId_VariableNode,
    OpcUaId_VariableNode_Encoding_DefaultBinary,
    OpcUaId_VariableNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_VariableNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_VariableNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_VariableNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_VariableNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_VariableNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_VariableNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeNode
/*============================================================================
 * OpcUa_VariableTypeNode_Initialize
 *===========================================================================*/
void OpcUa_VariableTypeNode_Initialize(OpcUa_VariableTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
        Variant_Initialize(&a_pValue->Value);
        NodeId_Initialize(&a_pValue->DataType);
        Int32_Initialize(&a_pValue->ValueRank);
        SOPC_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * OpcUa_VariableTypeNode_Clear
 *===========================================================================*/
void OpcUa_VariableTypeNode_Clear(OpcUa_VariableTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
        Variant_Clear(&a_pValue->Value);
        NodeId_Clear(&a_pValue->DataType);
        Int32_Clear(&a_pValue->ValueRank);
        SOPC_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * OpcUa_VariableTypeNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_VariableTypeNode_Encode(OpcUa_VariableTypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);
    status &= Variant_Write(&a_pValue->Value, msgBuf);
    status &= NodeId_Write(&a_pValue->DataType, msgBuf);
    status &= Int32_Write(&a_pValue->ValueRank, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_VariableTypeNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_VariableTypeNode_Decode(OpcUa_VariableTypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_VariableTypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);
    status &= Variant_Read(&a_pValue->Value, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        OpcUa_VariableTypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_VariableTypeNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_VariableTypeNode_EncodeableType =
{
    "VariableTypeNode",
    OpcUaId_VariableTypeNode,
    OpcUaId_VariableTypeNode_Encoding_DefaultBinary,
    OpcUaId_VariableTypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_VariableTypeNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_VariableTypeNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_VariableTypeNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_VariableTypeNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_VariableTypeNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_VariableTypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeNode
/*============================================================================
 * OpcUa_ReferenceTypeNode_Initialize
 *===========================================================================*/
void OpcUa_ReferenceTypeNode_Initialize(OpcUa_ReferenceTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
        Boolean_Initialize(&a_pValue->Symmetric);
        LocalizedText_Initialize(&a_pValue->InverseName);
    }
}

/*============================================================================
 * OpcUa_ReferenceTypeNode_Clear
 *===========================================================================*/
void OpcUa_ReferenceTypeNode_Clear(OpcUa_ReferenceTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
        Boolean_Clear(&a_pValue->Symmetric);
        LocalizedText_Clear(&a_pValue->InverseName);
    }
}

/*============================================================================
 * OpcUa_ReferenceTypeNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReferenceTypeNode_Encode(OpcUa_ReferenceTypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);
    status &= Boolean_Write(&a_pValue->Symmetric, msgBuf);
    status &= LocalizedText_Write(&a_pValue->InverseName, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ReferenceTypeNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReferenceTypeNode_Decode(OpcUa_ReferenceTypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReferenceTypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);
    status &= Boolean_Read(&a_pValue->Symmetric, msgBuf);
    status &= LocalizedText_Read(&a_pValue->InverseName, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ReferenceTypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReferenceTypeNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReferenceTypeNode_EncodeableType =
{
    "ReferenceTypeNode",
    OpcUaId_ReferenceTypeNode,
    OpcUaId_ReferenceTypeNode_Encoding_DefaultBinary,
    OpcUaId_ReferenceTypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReferenceTypeNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReferenceTypeNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReferenceTypeNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReferenceTypeNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReferenceTypeNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReferenceTypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MethodNode
/*============================================================================
 * OpcUa_MethodNode_Initialize
 *===========================================================================*/
void OpcUa_MethodNode_Initialize(OpcUa_MethodNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->Executable);
        Boolean_Initialize(&a_pValue->UserExecutable);
    }
}

/*============================================================================
 * OpcUa_MethodNode_Clear
 *===========================================================================*/
void OpcUa_MethodNode_Clear(OpcUa_MethodNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->Executable);
        Boolean_Clear(&a_pValue->UserExecutable);
    }
}

/*============================================================================
 * OpcUa_MethodNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MethodNode_Encode(OpcUa_MethodNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->Executable, msgBuf);
    status &= Boolean_Write(&a_pValue->UserExecutable, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_MethodNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MethodNode_Decode(OpcUa_MethodNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_MethodNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->Executable, msgBuf);
    status &= Boolean_Read(&a_pValue->UserExecutable, msgBuf);

    if(status != STATUS_OK){
        OpcUa_MethodNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_MethodNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_MethodNode_EncodeableType =
{
    "MethodNode",
    OpcUaId_MethodNode,
    OpcUaId_MethodNode_Encoding_DefaultBinary,
    OpcUaId_MethodNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_MethodNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_MethodNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_MethodNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_MethodNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_MethodNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_MethodNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ViewNode
/*============================================================================
 * OpcUa_ViewNode_Initialize
 *===========================================================================*/
void OpcUa_ViewNode_Initialize(OpcUa_ViewNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->ContainsNoLoops);
        Byte_Initialize(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * OpcUa_ViewNode_Clear
 *===========================================================================*/
void OpcUa_ViewNode_Clear(OpcUa_ViewNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->ContainsNoLoops);
        Byte_Clear(&a_pValue->EventNotifier);
    }
}

/*============================================================================
 * OpcUa_ViewNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ViewNode_Encode(OpcUa_ViewNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->ContainsNoLoops, msgBuf);
    status &= Byte_Write(&a_pValue->EventNotifier, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ViewNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ViewNode_Decode(OpcUa_ViewNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ViewNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->ContainsNoLoops, msgBuf);
    status &= Byte_Read(&a_pValue->EventNotifier, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ViewNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ViewNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ViewNode_EncodeableType =
{
    "ViewNode",
    OpcUaId_ViewNode,
    OpcUaId_ViewNode_Encoding_DefaultBinary,
    OpcUaId_ViewNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ViewNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ViewNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ViewNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ViewNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ViewNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ViewNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DataTypeNode
/*============================================================================
 * OpcUa_DataTypeNode_Initialize
 *===========================================================================*/
void OpcUa_DataTypeNode_Initialize(OpcUa_DataTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
        UInt32_Initialize(&a_pValue->WriteMask);
        UInt32_Initialize(&a_pValue->UserWriteMask);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceNode_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * OpcUa_DataTypeNode_Clear
 *===========================================================================*/
void OpcUa_DataTypeNode_Clear(OpcUa_DataTypeNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
        UInt32_Clear(&a_pValue->WriteMask);
        UInt32_Clear(&a_pValue->UserWriteMask);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceNode_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * OpcUa_DataTypeNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DataTypeNode_Encode(OpcUa_DataTypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);
    status &= UInt32_Write(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Write(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceNode_Encode);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DataTypeNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DataTypeNode_Decode(OpcUa_DataTypeNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DataTypeNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceNode), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceNode_Decode);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        OpcUa_DataTypeNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DataTypeNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DataTypeNode_EncodeableType =
{
    "DataTypeNode",
    OpcUaId_DataTypeNode,
    OpcUaId_DataTypeNode_Encoding_DefaultBinary,
    OpcUaId_DataTypeNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DataTypeNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DataTypeNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DataTypeNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DataTypeNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DataTypeNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DataTypeNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReferenceNode
/*============================================================================
 * OpcUa_ReferenceNode_Initialize
 *===========================================================================*/
void OpcUa_ReferenceNode_Initialize(OpcUa_ReferenceNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsInverse);
        ExpandedNodeId_Initialize(&a_pValue->TargetId);
    }
}

/*============================================================================
 * OpcUa_ReferenceNode_Clear
 *===========================================================================*/
void OpcUa_ReferenceNode_Clear(OpcUa_ReferenceNode* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsInverse);
        ExpandedNodeId_Clear(&a_pValue->TargetId);
    }
}

/*============================================================================
 * OpcUa_ReferenceNode_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReferenceNode_Encode(OpcUa_ReferenceNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ReferenceNode_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReferenceNode_Decode(OpcUa_ReferenceNode* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReferenceNode_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsInverse, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TargetId, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ReferenceNode_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReferenceNode_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReferenceNode_EncodeableType =
{
    "ReferenceNode",
    OpcUaId_ReferenceNode,
    OpcUaId_ReferenceNode_Encoding_DefaultBinary,
    OpcUaId_ReferenceNode_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReferenceNode),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReferenceNode_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReferenceNode_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReferenceNode_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReferenceNode_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReferenceNode_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Argument
/*============================================================================
 * OpcUa_Argument_Initialize
 *===========================================================================*/
void OpcUa_Argument_Initialize(OpcUa_Argument* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->Name);
        NodeId_Initialize(&a_pValue->DataType);
        Int32_Initialize(&a_pValue->ValueRank);
        SOPC_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        LocalizedText_Initialize(&a_pValue->Description);
    }
}

/*============================================================================
 * OpcUa_Argument_Clear
 *===========================================================================*/
void OpcUa_Argument_Clear(OpcUa_Argument* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->Name);
        NodeId_Clear(&a_pValue->DataType);
        Int32_Clear(&a_pValue->ValueRank);
        SOPC_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
        LocalizedText_Clear(&a_pValue->Description);
    }
}

/*============================================================================
 * OpcUa_Argument_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_Argument_Encode(OpcUa_Argument* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->Name, msgBuf);
    status &= NodeId_Write(&a_pValue->DataType, msgBuf);
    status &= Int32_Write(&a_pValue->ValueRank, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= LocalizedText_Write(&a_pValue->Description, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_Argument_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_Argument_Decode(OpcUa_Argument* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_Argument_Initialize(a_pValue);

    status &= String_Read(&a_pValue->Name, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);

    if(status != STATUS_OK){
        OpcUa_Argument_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_Argument_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_Argument_EncodeableType =
{
    "Argument",
    OpcUaId_Argument,
    OpcUaId_Argument_Encoding_DefaultBinary,
    OpcUaId_Argument_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_Argument),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_Argument_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_Argument_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_Argument_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_Argument_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_Argument_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EnumValueType
/*============================================================================
 * OpcUa_EnumValueType_Initialize
 *===========================================================================*/
void OpcUa_EnumValueType_Initialize(OpcUa_EnumValueType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int64_Initialize(&a_pValue->Value);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        LocalizedText_Initialize(&a_pValue->Description);
    }
}

/*============================================================================
 * OpcUa_EnumValueType_Clear
 *===========================================================================*/
void OpcUa_EnumValueType_Clear(OpcUa_EnumValueType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int64_Clear(&a_pValue->Value);
        LocalizedText_Clear(&a_pValue->DisplayName);
        LocalizedText_Clear(&a_pValue->Description);
    }
}

/*============================================================================
 * OpcUa_EnumValueType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EnumValueType_Encode(OpcUa_EnumValueType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_EnumValueType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EnumValueType_Decode(OpcUa_EnumValueType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EnumValueType_Initialize(a_pValue);

    status &= Int64_Read(&a_pValue->Value, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);

    if(status != STATUS_OK){
        OpcUa_EnumValueType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EnumValueType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EnumValueType_EncodeableType =
{
    "EnumValueType",
    OpcUaId_EnumValueType,
    OpcUaId_EnumValueType_Encoding_DefaultBinary,
    OpcUaId_EnumValueType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EnumValueType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EnumValueType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EnumValueType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EnumValueType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EnumValueType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EnumValueType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EnumField
/*============================================================================
 * OpcUa_EnumField_Initialize
 *===========================================================================*/
void OpcUa_EnumField_Initialize(OpcUa_EnumField* a_pValue)
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
 * OpcUa_EnumField_Clear
 *===========================================================================*/
void OpcUa_EnumField_Clear(OpcUa_EnumField* a_pValue)
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
 * OpcUa_EnumField_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EnumField_Encode(OpcUa_EnumField* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_EnumField_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EnumField_Decode(OpcUa_EnumField* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EnumField_Initialize(a_pValue);

    status &= Int64_Read(&a_pValue->Value, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= String_Read(&a_pValue->Name, msgBuf);

    if(status != STATUS_OK){
        OpcUa_EnumField_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EnumField_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EnumField_EncodeableType =
{
    "EnumField",
    OpcUaId_EnumField,
    OpcUaId_EnumField_Encoding_DefaultBinary,
    OpcUaId_EnumField_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EnumField),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EnumField_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EnumField_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EnumField_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EnumField_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EnumField_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_OptionSet
/*============================================================================
 * OpcUa_OptionSet_Initialize
 *===========================================================================*/
void OpcUa_OptionSet_Initialize(OpcUa_OptionSet* a_pValue)
{
    if (a_pValue != NULL)
    {
        ByteString_Initialize(&a_pValue->Value);
        ByteString_Initialize(&a_pValue->ValidBits);
    }
}

/*============================================================================
 * OpcUa_OptionSet_Clear
 *===========================================================================*/
void OpcUa_OptionSet_Clear(OpcUa_OptionSet* a_pValue)
{
    if (a_pValue != NULL)
    {
        ByteString_Clear(&a_pValue->Value);
        ByteString_Clear(&a_pValue->ValidBits);
    }
}

/*============================================================================
 * OpcUa_OptionSet_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_OptionSet_Encode(OpcUa_OptionSet* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_OptionSet_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_OptionSet_Decode(OpcUa_OptionSet* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_OptionSet_Initialize(a_pValue);

    status &= ByteString_Read(&a_pValue->Value, msgBuf);
    status &= ByteString_Read(&a_pValue->ValidBits, msgBuf);

    if(status != STATUS_OK){
        OpcUa_OptionSet_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_OptionSet_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_OptionSet_EncodeableType =
{
    "OptionSet",
    OpcUaId_OptionSet,
    OpcUaId_OptionSet_Encoding_DefaultBinary,
    OpcUaId_OptionSet_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_OptionSet),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_OptionSet_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_OptionSet_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_OptionSet_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_OptionSet_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_OptionSet_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TimeZoneDataType
/*============================================================================
 * OpcUa_TimeZoneDataType_Initialize
 *===========================================================================*/
void OpcUa_TimeZoneDataType_Initialize(OpcUa_TimeZoneDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int16_Initialize(&a_pValue->Offset);
        Boolean_Initialize(&a_pValue->DaylightSavingInOffset);
    }
}

/*============================================================================
 * OpcUa_TimeZoneDataType_Clear
 *===========================================================================*/
void OpcUa_TimeZoneDataType_Clear(OpcUa_TimeZoneDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Int16_Clear(&a_pValue->Offset);
        Boolean_Clear(&a_pValue->DaylightSavingInOffset);
    }
}

/*============================================================================
 * OpcUa_TimeZoneDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TimeZoneDataType_Encode(OpcUa_TimeZoneDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_TimeZoneDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TimeZoneDataType_Decode(OpcUa_TimeZoneDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_TimeZoneDataType_Initialize(a_pValue);

    status &= Int16_Read(&a_pValue->Offset, msgBuf);
    status &= Boolean_Read(&a_pValue->DaylightSavingInOffset, msgBuf);

    if(status != STATUS_OK){
        OpcUa_TimeZoneDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_TimeZoneDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_TimeZoneDataType_EncodeableType =
{
    "TimeZoneDataType",
    OpcUaId_TimeZoneDataType,
    OpcUaId_TimeZoneDataType_Encoding_DefaultBinary,
    OpcUaId_TimeZoneDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_TimeZoneDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_TimeZoneDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_TimeZoneDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_TimeZoneDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_TimeZoneDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_TimeZoneDataType_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_ApplicationDescription
/*============================================================================
 * OpcUa_ApplicationDescription_Initialize
 *===========================================================================*/
void OpcUa_ApplicationDescription_Initialize(OpcUa_ApplicationDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ApplicationUri);
        String_Initialize(&a_pValue->ProductUri);
        LocalizedText_Initialize(&a_pValue->ApplicationName);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->ApplicationType);
        String_Initialize(&a_pValue->GatewayServerUri);
        String_Initialize(&a_pValue->DiscoveryProfileUri);
        SOPC_Initialize_Array(&a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * OpcUa_ApplicationDescription_Clear
 *===========================================================================*/
void OpcUa_ApplicationDescription_Clear(OpcUa_ApplicationDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ApplicationUri);
        String_Clear(&a_pValue->ProductUri);
        LocalizedText_Clear(&a_pValue->ApplicationName);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->ApplicationType);
        String_Clear(&a_pValue->GatewayServerUri);
        String_Clear(&a_pValue->DiscoveryProfileUri);
        SOPC_Clear_Array(&a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * OpcUa_ApplicationDescription_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ApplicationDescription_Encode(OpcUa_ApplicationDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ApplicationUri, msgBuf);
    status &= String_Write(&a_pValue->ProductUri, msgBuf);
    status &= LocalizedText_Write(&a_pValue->ApplicationName, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ApplicationType);
    status &= String_Write(&a_pValue->GatewayServerUri, msgBuf);
    status &= String_Write(&a_pValue->DiscoveryProfileUri, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ApplicationDescription_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ApplicationDescription_Decode(OpcUa_ApplicationDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ApplicationDescription_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ApplicationUri, msgBuf);
    status &= String_Read(&a_pValue->ProductUri, msgBuf);
    status &= LocalizedText_Read(&a_pValue->ApplicationName, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ApplicationType);
    status &= String_Read(&a_pValue->GatewayServerUri, msgBuf);
    status &= String_Read(&a_pValue->DiscoveryProfileUri, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        OpcUa_ApplicationDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ApplicationDescription_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ApplicationDescription_EncodeableType =
{
    "ApplicationDescription",
    OpcUaId_ApplicationDescription,
    OpcUaId_ApplicationDescription_Encoding_DefaultBinary,
    OpcUaId_ApplicationDescription_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ApplicationDescription),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ApplicationDescription_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ApplicationDescription_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ApplicationDescription_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ApplicationDescription_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ApplicationDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RequestHeader
/*============================================================================
 * OpcUa_RequestHeader_Initialize
 *===========================================================================*/
void OpcUa_RequestHeader_Initialize(OpcUa_RequestHeader* a_pValue)
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
 * OpcUa_RequestHeader_Clear
 *===========================================================================*/
void OpcUa_RequestHeader_Clear(OpcUa_RequestHeader* a_pValue)
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
 * OpcUa_RequestHeader_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RequestHeader_Encode(OpcUa_RequestHeader* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_RequestHeader_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RequestHeader_Decode(OpcUa_RequestHeader* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RequestHeader_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->AuthenticationToken, msgBuf);
    status &= DateTime_Read(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestHandle, msgBuf);
    status &= UInt32_Read(&a_pValue->ReturnDiagnostics, msgBuf);
    status &= String_Read(&a_pValue->AuditEntryId, msgBuf);
    status &= UInt32_Read(&a_pValue->TimeoutHint, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->AdditionalHeader, msgBuf);

    if(status != STATUS_OK){
        OpcUa_RequestHeader_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RequestHeader_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RequestHeader_EncodeableType =
{
    "RequestHeader",
    OpcUaId_RequestHeader,
    OpcUaId_RequestHeader_Encoding_DefaultBinary,
    OpcUaId_RequestHeader_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RequestHeader),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RequestHeader_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RequestHeader_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RequestHeader_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RequestHeader_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RequestHeader_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ResponseHeader
/*============================================================================
 * OpcUa_ResponseHeader_Initialize
 *===========================================================================*/
void OpcUa_ResponseHeader_Initialize(OpcUa_ResponseHeader* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->Timestamp);
        UInt32_Initialize(&a_pValue->RequestHandle);
        StatusCode_Initialize(&a_pValue->ServiceResult);
        DiagnosticInfo_Initialize(&a_pValue->ServiceDiagnostics);
        SOPC_Initialize_Array(&a_pValue->NoOfStringTable, (void**) &a_pValue->StringTable, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
        ExtensionObject_Initialize(&a_pValue->AdditionalHeader);
    }
}

/*============================================================================
 * OpcUa_ResponseHeader_Clear
 *===========================================================================*/
void OpcUa_ResponseHeader_Clear(OpcUa_ResponseHeader* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->Timestamp);
        UInt32_Clear(&a_pValue->RequestHandle);
        StatusCode_Clear(&a_pValue->ServiceResult);
        DiagnosticInfo_Clear(&a_pValue->ServiceDiagnostics);
        SOPC_Clear_Array(&a_pValue->NoOfStringTable, (void**) &a_pValue->StringTable, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
        ExtensionObject_Clear(&a_pValue->AdditionalHeader);
    }
}

/*============================================================================
 * OpcUa_ResponseHeader_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ResponseHeader_Encode(OpcUa_ResponseHeader* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestHandle, msgBuf);
    status &= StatusCode_Write(&a_pValue->ServiceResult, msgBuf);
    status &= DiagnosticInfo_Write(&a_pValue->ServiceDiagnostics, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfStringTable, (void**) &a_pValue->StringTable, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);
    status &= ExtensionObject_Write(&a_pValue->AdditionalHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ResponseHeader_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ResponseHeader_Decode(OpcUa_ResponseHeader* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ResponseHeader_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestHandle, msgBuf);
    status &= StatusCode_Read(&a_pValue->ServiceResult, msgBuf);
    status &= DiagnosticInfo_Read(&a_pValue->ServiceDiagnostics, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfStringTable, (void**) &a_pValue->StringTable, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);
    status &= ExtensionObject_Read(&a_pValue->AdditionalHeader, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ResponseHeader_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ResponseHeader_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ResponseHeader_EncodeableType =
{
    "ResponseHeader",
    OpcUaId_ResponseHeader,
    OpcUaId_ResponseHeader_Encoding_DefaultBinary,
    OpcUaId_ResponseHeader_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ResponseHeader),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ResponseHeader_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ResponseHeader_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ResponseHeader_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ResponseHeader_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ResponseHeader_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ServiceFault
/*============================================================================
 * OpcUa_ServiceFault_Initialize
 *===========================================================================*/
void OpcUa_ServiceFault_Initialize(OpcUa_ServiceFault* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_ServiceFault_Clear
 *===========================================================================*/
void OpcUa_ServiceFault_Clear(OpcUa_ServiceFault* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_ServiceFault_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServiceFault_Encode(OpcUa_ServiceFault* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ServiceFault_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServiceFault_Decode(OpcUa_ServiceFault* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ServiceFault_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ServiceFault_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ServiceFault_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ServiceFault_EncodeableType =
{
    "ServiceFault",
    OpcUaId_ServiceFault,
    OpcUaId_ServiceFault_Encoding_DefaultBinary,
    OpcUaId_ServiceFault_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ServiceFault),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ServiceFault_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ServiceFault_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ServiceFault_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ServiceFault_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ServiceFault_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_FindServers
#ifndef OPCUA_EXCLUDE_FindServersRequest
/*============================================================================
 * OpcUa_FindServersRequest_Initialize
 *===========================================================================*/
void OpcUa_FindServersRequest_Initialize(OpcUa_FindServersRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        String_Initialize(&a_pValue->EndpointUrl);
        SOPC_Initialize_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfServerUris, (void**) &a_pValue->ServerUris, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * OpcUa_FindServersRequest_Clear
 *===========================================================================*/
void OpcUa_FindServersRequest_Clear(OpcUa_FindServersRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        String_Clear(&a_pValue->EndpointUrl);
        SOPC_Clear_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfServerUris, (void**) &a_pValue->ServerUris, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * OpcUa_FindServersRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_FindServersRequest_Encode(OpcUa_FindServersRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= String_Write(&a_pValue->EndpointUrl, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfServerUris, (void**) &a_pValue->ServerUris, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_FindServersRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_FindServersRequest_Decode(OpcUa_FindServersRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_FindServersRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfServerUris, (void**) &a_pValue->ServerUris, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        OpcUa_FindServersRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_FindServersRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_FindServersRequest_EncodeableType =
{
    "FindServersRequest",
    OpcUaId_FindServersRequest,
    OpcUaId_FindServersRequest_Encoding_DefaultBinary,
    OpcUaId_FindServersRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_FindServersRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_FindServersRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_FindServersRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_FindServersRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_FindServersRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_FindServersRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_FindServersResponse
/*============================================================================
 * OpcUa_FindServersResponse_Initialize
 *===========================================================================*/
void OpcUa_FindServersResponse_Initialize(OpcUa_FindServersResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                            sizeof(OpcUa_ApplicationDescription), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ApplicationDescription_Initialize);
    }
}

/*============================================================================
 * OpcUa_FindServersResponse_Clear
 *===========================================================================*/
void OpcUa_FindServersResponse_Clear(OpcUa_FindServersResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                       sizeof(OpcUa_ApplicationDescription), (SOPC_EncodeableObject_PfnClear*) OpcUa_ApplicationDescription_Clear);
    }
}

/*============================================================================
 * OpcUa_FindServersResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_FindServersResponse_Encode(OpcUa_FindServersResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                   sizeof(OpcUa_ApplicationDescription), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ApplicationDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_FindServersResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_FindServersResponse_Decode(OpcUa_FindServersResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_FindServersResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                  sizeof(OpcUa_ApplicationDescription), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ApplicationDescription_Decode);

    if(status != STATUS_OK){
        OpcUa_FindServersResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_FindServersResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_FindServersResponse_EncodeableType =
{
    "FindServersResponse",
    OpcUaId_FindServersResponse,
    OpcUaId_FindServersResponse_Encoding_DefaultBinary,
    OpcUaId_FindServersResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_FindServersResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_FindServersResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_FindServersResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_FindServersResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_FindServersResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_FindServersResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_ServerOnNetwork
/*============================================================================
 * OpcUa_ServerOnNetwork_Initialize
 *===========================================================================*/
void OpcUa_ServerOnNetwork_Initialize(OpcUa_ServerOnNetwork* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->RecordId);
        String_Initialize(&a_pValue->ServerName);
        String_Initialize(&a_pValue->DiscoveryUrl);
        SOPC_Initialize_Array(&a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * OpcUa_ServerOnNetwork_Clear
 *===========================================================================*/
void OpcUa_ServerOnNetwork_Clear(OpcUa_ServerOnNetwork* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->RecordId);
        String_Clear(&a_pValue->ServerName);
        String_Clear(&a_pValue->DiscoveryUrl);
        SOPC_Clear_Array(&a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * OpcUa_ServerOnNetwork_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerOnNetwork_Encode(OpcUa_ServerOnNetwork* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->RecordId, msgBuf);
    status &= String_Write(&a_pValue->ServerName, msgBuf);
    status &= String_Write(&a_pValue->DiscoveryUrl, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ServerOnNetwork_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerOnNetwork_Decode(OpcUa_ServerOnNetwork* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ServerOnNetwork_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->RecordId, msgBuf);
    status &= String_Read(&a_pValue->ServerName, msgBuf);
    status &= String_Read(&a_pValue->DiscoveryUrl, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        OpcUa_ServerOnNetwork_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ServerOnNetwork_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ServerOnNetwork_EncodeableType =
{
    "ServerOnNetwork",
    OpcUaId_ServerOnNetwork,
    OpcUaId_ServerOnNetwork_Encoding_DefaultBinary,
    OpcUaId_ServerOnNetwork_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ServerOnNetwork),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ServerOnNetwork_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ServerOnNetwork_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ServerOnNetwork_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ServerOnNetwork_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ServerOnNetwork_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetwork
#ifndef OPCUA_EXCLUDE_FindServersOnNetworkRequest
/*============================================================================
 * OpcUa_FindServersOnNetworkRequest_Initialize
 *===========================================================================*/
void OpcUa_FindServersOnNetworkRequest_Initialize(OpcUa_FindServersOnNetworkRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->StartingRecordId);
        UInt32_Initialize(&a_pValue->MaxRecordsToReturn);
        SOPC_Initialize_Array(&a_pValue->NoOfServerCapabilityFilter, (void**) &a_pValue->ServerCapabilityFilter, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * OpcUa_FindServersOnNetworkRequest_Clear
 *===========================================================================*/
void OpcUa_FindServersOnNetworkRequest_Clear(OpcUa_FindServersOnNetworkRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->StartingRecordId);
        UInt32_Clear(&a_pValue->MaxRecordsToReturn);
        SOPC_Clear_Array(&a_pValue->NoOfServerCapabilityFilter, (void**) &a_pValue->ServerCapabilityFilter, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * OpcUa_FindServersOnNetworkRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_FindServersOnNetworkRequest_Encode(OpcUa_FindServersOnNetworkRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->StartingRecordId, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxRecordsToReturn, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfServerCapabilityFilter, (void**) &a_pValue->ServerCapabilityFilter, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_FindServersOnNetworkRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_FindServersOnNetworkRequest_Decode(OpcUa_FindServersOnNetworkRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_FindServersOnNetworkRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->StartingRecordId, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxRecordsToReturn, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfServerCapabilityFilter, (void**) &a_pValue->ServerCapabilityFilter, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        OpcUa_FindServersOnNetworkRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_FindServersOnNetworkRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_FindServersOnNetworkRequest_EncodeableType =
{
    "FindServersOnNetworkRequest",
    OpcUaId_FindServersOnNetworkRequest,
    OpcUaId_FindServersOnNetworkRequest_Encoding_DefaultBinary,
    OpcUaId_FindServersOnNetworkRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_FindServersOnNetworkRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_FindServersOnNetworkRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_FindServersOnNetworkRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_FindServersOnNetworkRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_FindServersOnNetworkRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_FindServersOnNetworkRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_FindServersOnNetworkResponse
/*============================================================================
 * OpcUa_FindServersOnNetworkResponse_Initialize
 *===========================================================================*/
void OpcUa_FindServersOnNetworkResponse_Initialize(OpcUa_FindServersOnNetworkResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        DateTime_Initialize(&a_pValue->LastCounterResetTime);
        SOPC_Initialize_Array(&a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                            sizeof(OpcUa_ServerOnNetwork), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ServerOnNetwork_Initialize);
    }
}

/*============================================================================
 * OpcUa_FindServersOnNetworkResponse_Clear
 *===========================================================================*/
void OpcUa_FindServersOnNetworkResponse_Clear(OpcUa_FindServersOnNetworkResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        DateTime_Clear(&a_pValue->LastCounterResetTime);
        SOPC_Clear_Array(&a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                       sizeof(OpcUa_ServerOnNetwork), (SOPC_EncodeableObject_PfnClear*) OpcUa_ServerOnNetwork_Clear);
    }
}

/*============================================================================
 * OpcUa_FindServersOnNetworkResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_FindServersOnNetworkResponse_Encode(OpcUa_FindServersOnNetworkResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= DateTime_Write(&a_pValue->LastCounterResetTime, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                   sizeof(OpcUa_ServerOnNetwork), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ServerOnNetwork_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_FindServersOnNetworkResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_FindServersOnNetworkResponse_Decode(OpcUa_FindServersOnNetworkResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_FindServersOnNetworkResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= DateTime_Read(&a_pValue->LastCounterResetTime, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfServers, (void**) &a_pValue->Servers, 
                  sizeof(OpcUa_ServerOnNetwork), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ServerOnNetwork_Decode);

    if(status != STATUS_OK){
        OpcUa_FindServersOnNetworkResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_FindServersOnNetworkResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_FindServersOnNetworkResponse_EncodeableType =
{
    "FindServersOnNetworkResponse",
    OpcUaId_FindServersOnNetworkResponse,
    OpcUaId_FindServersOnNetworkResponse_Encoding_DefaultBinary,
    OpcUaId_FindServersOnNetworkResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_FindServersOnNetworkResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_FindServersOnNetworkResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_FindServersOnNetworkResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_FindServersOnNetworkResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_FindServersOnNetworkResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_FindServersOnNetworkResponse_Decode
};
#endif
#endif



#ifndef OPCUA_EXCLUDE_UserTokenPolicy
/*============================================================================
 * OpcUa_UserTokenPolicy_Initialize
 *===========================================================================*/
void OpcUa_UserTokenPolicy_Initialize(OpcUa_UserTokenPolicy* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->TokenType);
        String_Initialize(&a_pValue->IssuedTokenType);
        String_Initialize(&a_pValue->IssuerEndpointUrl);
        String_Initialize(&a_pValue->SecurityPolicyUri);
    }
}

/*============================================================================
 * OpcUa_UserTokenPolicy_Clear
 *===========================================================================*/
void OpcUa_UserTokenPolicy_Clear(OpcUa_UserTokenPolicy* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->TokenType);
        String_Clear(&a_pValue->IssuedTokenType);
        String_Clear(&a_pValue->IssuerEndpointUrl);
        String_Clear(&a_pValue->SecurityPolicyUri);
    }
}

/*============================================================================
 * OpcUa_UserTokenPolicy_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UserTokenPolicy_Encode(OpcUa_UserTokenPolicy* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->PolicyId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TokenType);
    status &= String_Write(&a_pValue->IssuedTokenType, msgBuf);
    status &= String_Write(&a_pValue->IssuerEndpointUrl, msgBuf);
    status &= String_Write(&a_pValue->SecurityPolicyUri, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_UserTokenPolicy_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UserTokenPolicy_Decode(OpcUa_UserTokenPolicy* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_UserTokenPolicy_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TokenType);
    status &= String_Read(&a_pValue->IssuedTokenType, msgBuf);
    status &= String_Read(&a_pValue->IssuerEndpointUrl, msgBuf);
    status &= String_Read(&a_pValue->SecurityPolicyUri, msgBuf);

    if(status != STATUS_OK){
        OpcUa_UserTokenPolicy_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_UserTokenPolicy_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_UserTokenPolicy_EncodeableType =
{
    "UserTokenPolicy",
    OpcUaId_UserTokenPolicy,
    OpcUaId_UserTokenPolicy_Encoding_DefaultBinary,
    OpcUaId_UserTokenPolicy_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_UserTokenPolicy),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_UserTokenPolicy_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_UserTokenPolicy_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_UserTokenPolicy_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_UserTokenPolicy_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_UserTokenPolicy_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EndpointDescription
/*============================================================================
 * OpcUa_EndpointDescription_Initialize
 *===========================================================================*/
void OpcUa_EndpointDescription_Initialize(OpcUa_EndpointDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->EndpointUrl);
        OpcUa_ApplicationDescription_Initialize(&a_pValue->Server);
        ByteString_Initialize(&a_pValue->ServerCertificate);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        String_Initialize(&a_pValue->SecurityPolicyUri);
        SOPC_Initialize_Array(&a_pValue->NoOfUserIdentityTokens, (void**) &a_pValue->UserIdentityTokens, 
                            sizeof(OpcUa_UserTokenPolicy), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_UserTokenPolicy_Initialize);
        String_Initialize(&a_pValue->TransportProfileUri);
        Byte_Initialize(&a_pValue->SecurityLevel);
    }
}

/*============================================================================
 * OpcUa_EndpointDescription_Clear
 *===========================================================================*/
void OpcUa_EndpointDescription_Clear(OpcUa_EndpointDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->EndpointUrl);
        OpcUa_ApplicationDescription_Clear(&a_pValue->Server);
        ByteString_Clear(&a_pValue->ServerCertificate);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        String_Clear(&a_pValue->SecurityPolicyUri);
        SOPC_Clear_Array(&a_pValue->NoOfUserIdentityTokens, (void**) &a_pValue->UserIdentityTokens, 
                       sizeof(OpcUa_UserTokenPolicy), (SOPC_EncodeableObject_PfnClear*) OpcUa_UserTokenPolicy_Clear);
        String_Clear(&a_pValue->TransportProfileUri);
        Byte_Clear(&a_pValue->SecurityLevel);
    }
}

/*============================================================================
 * OpcUa_EndpointDescription_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EndpointDescription_Encode(OpcUa_EndpointDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->EndpointUrl, msgBuf);
    status &= OpcUa_ApplicationDescription_Encode(&a_pValue->Server, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerCertificate, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= String_Write(&a_pValue->SecurityPolicyUri, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfUserIdentityTokens, (void**) &a_pValue->UserIdentityTokens, 
                   sizeof(OpcUa_UserTokenPolicy), (SOPC_EncodeableObject_PfnEncode*) OpcUa_UserTokenPolicy_Encode);
    status &= String_Write(&a_pValue->TransportProfileUri, msgBuf);
    status &= Byte_Write(&a_pValue->SecurityLevel, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_EndpointDescription_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EndpointDescription_Decode(OpcUa_EndpointDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EndpointDescription_Initialize(a_pValue);

    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= OpcUa_ApplicationDescription_Decode(&a_pValue->Server, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerCertificate, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= String_Read(&a_pValue->SecurityPolicyUri, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfUserIdentityTokens, (void**) &a_pValue->UserIdentityTokens, 
                  sizeof(OpcUa_UserTokenPolicy), (SOPC_EncodeableObject_PfnDecode*) OpcUa_UserTokenPolicy_Decode);
    status &= String_Read(&a_pValue->TransportProfileUri, msgBuf);
    status &= Byte_Read(&a_pValue->SecurityLevel, msgBuf);

    if(status != STATUS_OK){
        OpcUa_EndpointDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EndpointDescription_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EndpointDescription_EncodeableType =
{
    "EndpointDescription",
    OpcUaId_EndpointDescription,
    OpcUaId_EndpointDescription_Encoding_DefaultBinary,
    OpcUaId_EndpointDescription_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EndpointDescription),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EndpointDescription_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EndpointDescription_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EndpointDescription_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EndpointDescription_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EndpointDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_GetEndpoints
#ifndef OPCUA_EXCLUDE_GetEndpointsRequest
/*============================================================================
 * OpcUa_GetEndpointsRequest_Initialize
 *===========================================================================*/
void OpcUa_GetEndpointsRequest_Initialize(OpcUa_GetEndpointsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        String_Initialize(&a_pValue->EndpointUrl);
        SOPC_Initialize_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfProfileUris, (void**) &a_pValue->ProfileUris, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * OpcUa_GetEndpointsRequest_Clear
 *===========================================================================*/
void OpcUa_GetEndpointsRequest_Clear(OpcUa_GetEndpointsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        String_Clear(&a_pValue->EndpointUrl);
        SOPC_Clear_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfProfileUris, (void**) &a_pValue->ProfileUris, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * OpcUa_GetEndpointsRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_GetEndpointsRequest_Encode(OpcUa_GetEndpointsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= String_Write(&a_pValue->EndpointUrl, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfProfileUris, (void**) &a_pValue->ProfileUris, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_GetEndpointsRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_GetEndpointsRequest_Decode(OpcUa_GetEndpointsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_GetEndpointsRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfProfileUris, (void**) &a_pValue->ProfileUris, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        OpcUa_GetEndpointsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_GetEndpointsRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_GetEndpointsRequest_EncodeableType =
{
    "GetEndpointsRequest",
    OpcUaId_GetEndpointsRequest,
    OpcUaId_GetEndpointsRequest_Encoding_DefaultBinary,
    OpcUaId_GetEndpointsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_GetEndpointsRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_GetEndpointsRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_GetEndpointsRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_GetEndpointsRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_GetEndpointsRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_GetEndpointsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_GetEndpointsResponse
/*============================================================================
 * OpcUa_GetEndpointsResponse_Initialize
 *===========================================================================*/
void OpcUa_GetEndpointsResponse_Initialize(OpcUa_GetEndpointsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfEndpoints, (void**) &a_pValue->Endpoints, 
                            sizeof(OpcUa_EndpointDescription), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_EndpointDescription_Initialize);
    }
}

/*============================================================================
 * OpcUa_GetEndpointsResponse_Clear
 *===========================================================================*/
void OpcUa_GetEndpointsResponse_Clear(OpcUa_GetEndpointsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfEndpoints, (void**) &a_pValue->Endpoints, 
                       sizeof(OpcUa_EndpointDescription), (SOPC_EncodeableObject_PfnClear*) OpcUa_EndpointDescription_Clear);
    }
}

/*============================================================================
 * OpcUa_GetEndpointsResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_GetEndpointsResponse_Encode(OpcUa_GetEndpointsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfEndpoints, (void**) &a_pValue->Endpoints, 
                   sizeof(OpcUa_EndpointDescription), (SOPC_EncodeableObject_PfnEncode*) OpcUa_EndpointDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_GetEndpointsResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_GetEndpointsResponse_Decode(OpcUa_GetEndpointsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_GetEndpointsResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfEndpoints, (void**) &a_pValue->Endpoints, 
                  sizeof(OpcUa_EndpointDescription), (SOPC_EncodeableObject_PfnDecode*) OpcUa_EndpointDescription_Decode);

    if(status != STATUS_OK){
        OpcUa_GetEndpointsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_GetEndpointsResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_GetEndpointsResponse_EncodeableType =
{
    "GetEndpointsResponse",
    OpcUaId_GetEndpointsResponse,
    OpcUaId_GetEndpointsResponse_Encoding_DefaultBinary,
    OpcUaId_GetEndpointsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_GetEndpointsResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_GetEndpointsResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_GetEndpointsResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_GetEndpointsResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_GetEndpointsResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_GetEndpointsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_RegisteredServer
/*============================================================================
 * OpcUa_RegisteredServer_Initialize
 *===========================================================================*/
void OpcUa_RegisteredServer_Initialize(OpcUa_RegisteredServer* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ServerUri);
        String_Initialize(&a_pValue->ProductUri);
        SOPC_Initialize_Array(&a_pValue->NoOfServerNames, (void**) &a_pValue->ServerNames, 
                            sizeof(SOPC_LocalizedText), (SOPC_EncodeableObject_PfnInitialize*) LocalizedText_Initialize);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->ServerType);
        String_Initialize(&a_pValue->GatewayServerUri);
        SOPC_Initialize_Array(&a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
        String_Initialize(&a_pValue->SemaphoreFilePath);
        Boolean_Initialize(&a_pValue->IsOnline);
    }
}

/*============================================================================
 * OpcUa_RegisteredServer_Clear
 *===========================================================================*/
void OpcUa_RegisteredServer_Clear(OpcUa_RegisteredServer* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ServerUri);
        String_Clear(&a_pValue->ProductUri);
        SOPC_Clear_Array(&a_pValue->NoOfServerNames, (void**) &a_pValue->ServerNames, 
                       sizeof(SOPC_LocalizedText), (SOPC_EncodeableObject_PfnClear*) LocalizedText_Clear);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->ServerType);
        String_Clear(&a_pValue->GatewayServerUri);
        SOPC_Clear_Array(&a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
        String_Clear(&a_pValue->SemaphoreFilePath);
        Boolean_Clear(&a_pValue->IsOnline);
    }
}

/*============================================================================
 * OpcUa_RegisteredServer_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisteredServer_Encode(OpcUa_RegisteredServer* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ServerUri, msgBuf);
    status &= String_Write(&a_pValue->ProductUri, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfServerNames, (void**) &a_pValue->ServerNames, 
                   sizeof(SOPC_LocalizedText), (SOPC_EncodeableObject_PfnEncode*) LocalizedText_Write);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ServerType);
    status &= String_Write(&a_pValue->GatewayServerUri, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);
    status &= String_Write(&a_pValue->SemaphoreFilePath, msgBuf);
    status &= Boolean_Write(&a_pValue->IsOnline, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisteredServer_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisteredServer_Decode(OpcUa_RegisteredServer* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RegisteredServer_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ServerUri, msgBuf);
    status &= String_Read(&a_pValue->ProductUri, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfServerNames, (void**) &a_pValue->ServerNames, 
                  sizeof(SOPC_LocalizedText), (SOPC_EncodeableObject_PfnDecode*) LocalizedText_Read);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ServerType);
    status &= String_Read(&a_pValue->GatewayServerUri, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiscoveryUrls, (void**) &a_pValue->DiscoveryUrls, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);
    status &= String_Read(&a_pValue->SemaphoreFilePath, msgBuf);
    status &= Boolean_Read(&a_pValue->IsOnline, msgBuf);

    if(status != STATUS_OK){
        OpcUa_RegisteredServer_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisteredServer_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RegisteredServer_EncodeableType =
{
    "RegisteredServer",
    OpcUaId_RegisteredServer,
    OpcUaId_RegisteredServer_Encoding_DefaultBinary,
    OpcUaId_RegisteredServer_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RegisteredServer),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RegisteredServer_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RegisteredServer_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RegisteredServer_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RegisteredServer_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RegisteredServer_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer
#ifndef OPCUA_EXCLUDE_RegisterServerRequest
/*============================================================================
 * OpcUa_RegisterServerRequest_Initialize
 *===========================================================================*/
void OpcUa_RegisterServerRequest_Initialize(OpcUa_RegisterServerRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        OpcUa_RegisteredServer_Initialize(&a_pValue->Server);
    }
}

/*============================================================================
 * OpcUa_RegisterServerRequest_Clear
 *===========================================================================*/
void OpcUa_RegisterServerRequest_Clear(OpcUa_RegisterServerRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        OpcUa_RegisteredServer_Clear(&a_pValue->Server);
    }
}

/*============================================================================
 * OpcUa_RegisterServerRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterServerRequest_Encode(OpcUa_RegisterServerRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_RegisteredServer_Encode(&a_pValue->Server, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterServerRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterServerRequest_Decode(OpcUa_RegisterServerRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RegisterServerRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_RegisteredServer_Decode(&a_pValue->Server, msgBuf);

    if(status != STATUS_OK){
        OpcUa_RegisterServerRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterServerRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RegisterServerRequest_EncodeableType =
{
    "RegisterServerRequest",
    OpcUaId_RegisterServerRequest,
    OpcUaId_RegisterServerRequest_Encoding_DefaultBinary,
    OpcUaId_RegisterServerRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RegisterServerRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RegisterServerRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RegisterServerRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RegisterServerRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RegisterServerRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RegisterServerRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServerResponse
/*============================================================================
 * OpcUa_RegisterServerResponse_Initialize
 *===========================================================================*/
void OpcUa_RegisterServerResponse_Initialize(OpcUa_RegisterServerResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_RegisterServerResponse_Clear
 *===========================================================================*/
void OpcUa_RegisterServerResponse_Clear(OpcUa_RegisterServerResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_RegisterServerResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterServerResponse_Encode(OpcUa_RegisterServerResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterServerResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterServerResponse_Decode(OpcUa_RegisterServerResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RegisterServerResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        OpcUa_RegisterServerResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterServerResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RegisterServerResponse_EncodeableType =
{
    "RegisterServerResponse",
    OpcUaId_RegisterServerResponse,
    OpcUaId_RegisterServerResponse_Encoding_DefaultBinary,
    OpcUaId_RegisterServerResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RegisterServerResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RegisterServerResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RegisterServerResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RegisterServerResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RegisterServerResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RegisterServerResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_MdnsDiscoveryConfiguration
/*============================================================================
 * OpcUa_MdnsDiscoveryConfiguration_Initialize
 *===========================================================================*/
void OpcUa_MdnsDiscoveryConfiguration_Initialize(OpcUa_MdnsDiscoveryConfiguration* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->MdnsServerName);
        SOPC_Initialize_Array(&a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * OpcUa_MdnsDiscoveryConfiguration_Clear
 *===========================================================================*/
void OpcUa_MdnsDiscoveryConfiguration_Clear(OpcUa_MdnsDiscoveryConfiguration* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->MdnsServerName);
        SOPC_Clear_Array(&a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * OpcUa_MdnsDiscoveryConfiguration_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MdnsDiscoveryConfiguration_Encode(OpcUa_MdnsDiscoveryConfiguration* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->MdnsServerName, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_MdnsDiscoveryConfiguration_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MdnsDiscoveryConfiguration_Decode(OpcUa_MdnsDiscoveryConfiguration* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_MdnsDiscoveryConfiguration_Initialize(a_pValue);

    status &= String_Read(&a_pValue->MdnsServerName, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfServerCapabilities, (void**) &a_pValue->ServerCapabilities, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        OpcUa_MdnsDiscoveryConfiguration_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_MdnsDiscoveryConfiguration_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_MdnsDiscoveryConfiguration_EncodeableType =
{
    "MdnsDiscoveryConfiguration",
    OpcUaId_MdnsDiscoveryConfiguration,
    OpcUaId_MdnsDiscoveryConfiguration_Encoding_DefaultBinary,
    OpcUaId_MdnsDiscoveryConfiguration_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_MdnsDiscoveryConfiguration),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_MdnsDiscoveryConfiguration_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_MdnsDiscoveryConfiguration_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_MdnsDiscoveryConfiguration_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_MdnsDiscoveryConfiguration_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_MdnsDiscoveryConfiguration_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2
#ifndef OPCUA_EXCLUDE_RegisterServer2Request
/*============================================================================
 * OpcUa_RegisterServer2Request_Initialize
 *===========================================================================*/
void OpcUa_RegisterServer2Request_Initialize(OpcUa_RegisterServer2Request* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        OpcUa_RegisteredServer_Initialize(&a_pValue->Server);
        SOPC_Initialize_Array(&a_pValue->NoOfDiscoveryConfiguration, (void**) &a_pValue->DiscoveryConfiguration, 
                            sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnInitialize*) ExtensionObject_Initialize);
    }
}

/*============================================================================
 * OpcUa_RegisterServer2Request_Clear
 *===========================================================================*/
void OpcUa_RegisterServer2Request_Clear(OpcUa_RegisterServer2Request* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        OpcUa_RegisteredServer_Clear(&a_pValue->Server);
        SOPC_Clear_Array(&a_pValue->NoOfDiscoveryConfiguration, (void**) &a_pValue->DiscoveryConfiguration, 
                       sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnClear*) ExtensionObject_Clear);
    }
}

/*============================================================================
 * OpcUa_RegisterServer2Request_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterServer2Request_Encode(OpcUa_RegisterServer2Request* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_RegisteredServer_Encode(&a_pValue->Server, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiscoveryConfiguration, (void**) &a_pValue->DiscoveryConfiguration, 
                   sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnEncode*) ExtensionObject_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterServer2Request_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterServer2Request_Decode(OpcUa_RegisterServer2Request* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RegisterServer2Request_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_RegisteredServer_Decode(&a_pValue->Server, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiscoveryConfiguration, (void**) &a_pValue->DiscoveryConfiguration, 
                  sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnDecode*) ExtensionObject_Read);

    if(status != STATUS_OK){
        OpcUa_RegisterServer2Request_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterServer2Request_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RegisterServer2Request_EncodeableType =
{
    "RegisterServer2Request",
    OpcUaId_RegisterServer2Request,
    OpcUaId_RegisterServer2Request_Encoding_DefaultBinary,
    OpcUaId_RegisterServer2Request_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RegisterServer2Request),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RegisterServer2Request_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RegisterServer2Request_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RegisterServer2Request_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RegisterServer2Request_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RegisterServer2Request_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterServer2Response
/*============================================================================
 * OpcUa_RegisterServer2Response_Initialize
 *===========================================================================*/
void OpcUa_RegisterServer2Response_Initialize(OpcUa_RegisterServer2Response* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfConfigurationResults, (void**) &a_pValue->ConfigurationResults, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_RegisterServer2Response_Clear
 *===========================================================================*/
void OpcUa_RegisterServer2Response_Clear(OpcUa_RegisterServer2Response* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfConfigurationResults, (void**) &a_pValue->ConfigurationResults, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_RegisterServer2Response_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterServer2Response_Encode(OpcUa_RegisterServer2Response* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfConfigurationResults, (void**) &a_pValue->ConfigurationResults, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterServer2Response_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterServer2Response_Decode(OpcUa_RegisterServer2Response* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RegisterServer2Response_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfConfigurationResults, (void**) &a_pValue->ConfigurationResults, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_RegisterServer2Response_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterServer2Response_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RegisterServer2Response_EncodeableType =
{
    "RegisterServer2Response",
    OpcUaId_RegisterServer2Response,
    OpcUaId_RegisterServer2Response_Encoding_DefaultBinary,
    OpcUaId_RegisterServer2Response_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RegisterServer2Response),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RegisterServer2Response_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RegisterServer2Response_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RegisterServer2Response_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RegisterServer2Response_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RegisterServer2Response_Decode
};
#endif
#endif


#ifndef OPCUA_EXCLUDE_ChannelSecurityToken
/*============================================================================
 * OpcUa_ChannelSecurityToken_Initialize
 *===========================================================================*/
void OpcUa_ChannelSecurityToken_Initialize(OpcUa_ChannelSecurityToken* a_pValue)
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
 * OpcUa_ChannelSecurityToken_Clear
 *===========================================================================*/
void OpcUa_ChannelSecurityToken_Clear(OpcUa_ChannelSecurityToken* a_pValue)
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
 * OpcUa_ChannelSecurityToken_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ChannelSecurityToken_Encode(OpcUa_ChannelSecurityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ChannelSecurityToken_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ChannelSecurityToken_Decode(OpcUa_ChannelSecurityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ChannelSecurityToken_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->ChannelId, msgBuf);
    status &= UInt32_Read(&a_pValue->TokenId, msgBuf);
    status &= DateTime_Read(&a_pValue->CreatedAt, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedLifetime, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ChannelSecurityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ChannelSecurityToken_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ChannelSecurityToken_EncodeableType =
{
    "ChannelSecurityToken",
    OpcUaId_ChannelSecurityToken,
    OpcUaId_ChannelSecurityToken_Encoding_DefaultBinary,
    OpcUaId_ChannelSecurityToken_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ChannelSecurityToken),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ChannelSecurityToken_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ChannelSecurityToken_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ChannelSecurityToken_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ChannelSecurityToken_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ChannelSecurityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_OpenSecureChannel
#ifndef OPCUA_EXCLUDE_OpenSecureChannelRequest
/*============================================================================
 * OpcUa_OpenSecureChannelRequest_Initialize
 *===========================================================================*/
void OpcUa_OpenSecureChannelRequest_Initialize(OpcUa_OpenSecureChannelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->ClientProtocolVersion);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->RequestType);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        ByteString_Initialize(&a_pValue->ClientNonce);
        UInt32_Initialize(&a_pValue->RequestedLifetime);
    }
}

/*============================================================================
 * OpcUa_OpenSecureChannelRequest_Clear
 *===========================================================================*/
void OpcUa_OpenSecureChannelRequest_Clear(OpcUa_OpenSecureChannelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->ClientProtocolVersion);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->RequestType);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        ByteString_Clear(&a_pValue->ClientNonce);
        UInt32_Clear(&a_pValue->RequestedLifetime);
    }
}

/*============================================================================
 * OpcUa_OpenSecureChannelRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_OpenSecureChannelRequest_Encode(OpcUa_OpenSecureChannelRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->ClientProtocolVersion, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->RequestType);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= ByteString_Write(&a_pValue->ClientNonce, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestedLifetime, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_OpenSecureChannelRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_OpenSecureChannelRequest_Decode(OpcUa_OpenSecureChannelRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_OpenSecureChannelRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->ClientProtocolVersion, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->RequestType);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= ByteString_Read(&a_pValue->ClientNonce, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedLifetime, msgBuf);

    if(status != STATUS_OK){
        OpcUa_OpenSecureChannelRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_OpenSecureChannelRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_OpenSecureChannelRequest_EncodeableType =
{
    "OpenSecureChannelRequest",
    OpcUaId_OpenSecureChannelRequest,
    OpcUaId_OpenSecureChannelRequest_Encoding_DefaultBinary,
    OpcUaId_OpenSecureChannelRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_OpenSecureChannelRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_OpenSecureChannelRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_OpenSecureChannelRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_OpenSecureChannelRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_OpenSecureChannelRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_OpenSecureChannelRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_OpenSecureChannelResponse
/*============================================================================
 * OpcUa_OpenSecureChannelResponse_Initialize
 *===========================================================================*/
void OpcUa_OpenSecureChannelResponse_Initialize(OpcUa_OpenSecureChannelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UInt32_Initialize(&a_pValue->ServerProtocolVersion);
        OpcUa_ChannelSecurityToken_Initialize(&a_pValue->SecurityToken);
        ByteString_Initialize(&a_pValue->ServerNonce);
    }
}

/*============================================================================
 * OpcUa_OpenSecureChannelResponse_Clear
 *===========================================================================*/
void OpcUa_OpenSecureChannelResponse_Clear(OpcUa_OpenSecureChannelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UInt32_Clear(&a_pValue->ServerProtocolVersion);
        OpcUa_ChannelSecurityToken_Clear(&a_pValue->SecurityToken);
        ByteString_Clear(&a_pValue->ServerNonce);
    }
}

/*============================================================================
 * OpcUa_OpenSecureChannelResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_OpenSecureChannelResponse_Encode(OpcUa_OpenSecureChannelResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->ServerProtocolVersion, msgBuf);
    status &= OpcUa_ChannelSecurityToken_Encode(&a_pValue->SecurityToken, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerNonce, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_OpenSecureChannelResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_OpenSecureChannelResponse_Decode(OpcUa_OpenSecureChannelResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_OpenSecureChannelResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->ServerProtocolVersion, msgBuf);
    status &= OpcUa_ChannelSecurityToken_Decode(&a_pValue->SecurityToken, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerNonce, msgBuf);

    if(status != STATUS_OK){
        OpcUa_OpenSecureChannelResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_OpenSecureChannelResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_OpenSecureChannelResponse_EncodeableType =
{
    "OpenSecureChannelResponse",
    OpcUaId_OpenSecureChannelResponse,
    OpcUaId_OpenSecureChannelResponse_Encoding_DefaultBinary,
    OpcUaId_OpenSecureChannelResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_OpenSecureChannelResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_OpenSecureChannelResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_OpenSecureChannelResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_OpenSecureChannelResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_OpenSecureChannelResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_OpenSecureChannelResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_CloseSecureChannel
#ifndef OPCUA_EXCLUDE_CloseSecureChannelRequest
/*============================================================================
 * OpcUa_CloseSecureChannelRequest_Initialize
 *===========================================================================*/
void OpcUa_CloseSecureChannelRequest_Initialize(OpcUa_CloseSecureChannelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
    }
}

/*============================================================================
 * OpcUa_CloseSecureChannelRequest_Clear
 *===========================================================================*/
void OpcUa_CloseSecureChannelRequest_Clear(OpcUa_CloseSecureChannelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
    }
}

/*============================================================================
 * OpcUa_CloseSecureChannelRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CloseSecureChannelRequest_Encode(OpcUa_CloseSecureChannelRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CloseSecureChannelRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CloseSecureChannelRequest_Decode(OpcUa_CloseSecureChannelRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CloseSecureChannelRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CloseSecureChannelRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CloseSecureChannelRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CloseSecureChannelRequest_EncodeableType =
{
    "CloseSecureChannelRequest",
    OpcUaId_CloseSecureChannelRequest,
    OpcUaId_CloseSecureChannelRequest_Encoding_DefaultBinary,
    OpcUaId_CloseSecureChannelRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CloseSecureChannelRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CloseSecureChannelRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CloseSecureChannelRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CloseSecureChannelRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CloseSecureChannelRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CloseSecureChannelRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CloseSecureChannelResponse
/*============================================================================
 * OpcUa_CloseSecureChannelResponse_Initialize
 *===========================================================================*/
void OpcUa_CloseSecureChannelResponse_Initialize(OpcUa_CloseSecureChannelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_CloseSecureChannelResponse_Clear
 *===========================================================================*/
void OpcUa_CloseSecureChannelResponse_Clear(OpcUa_CloseSecureChannelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_CloseSecureChannelResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CloseSecureChannelResponse_Encode(OpcUa_CloseSecureChannelResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CloseSecureChannelResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CloseSecureChannelResponse_Decode(OpcUa_CloseSecureChannelResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CloseSecureChannelResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CloseSecureChannelResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CloseSecureChannelResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CloseSecureChannelResponse_EncodeableType =
{
    "CloseSecureChannelResponse",
    OpcUaId_CloseSecureChannelResponse,
    OpcUaId_CloseSecureChannelResponse_Encoding_DefaultBinary,
    OpcUaId_CloseSecureChannelResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CloseSecureChannelResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CloseSecureChannelResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CloseSecureChannelResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CloseSecureChannelResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CloseSecureChannelResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CloseSecureChannelResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_SignedSoftwareCertificate
/*============================================================================
 * OpcUa_SignedSoftwareCertificate_Initialize
 *===========================================================================*/
void OpcUa_SignedSoftwareCertificate_Initialize(OpcUa_SignedSoftwareCertificate* a_pValue)
{
    if (a_pValue != NULL)
    {
        ByteString_Initialize(&a_pValue->CertificateData);
        ByteString_Initialize(&a_pValue->Signature);
    }
}

/*============================================================================
 * OpcUa_SignedSoftwareCertificate_Clear
 *===========================================================================*/
void OpcUa_SignedSoftwareCertificate_Clear(OpcUa_SignedSoftwareCertificate* a_pValue)
{
    if (a_pValue != NULL)
    {
        ByteString_Clear(&a_pValue->CertificateData);
        ByteString_Clear(&a_pValue->Signature);
    }
}

/*============================================================================
 * OpcUa_SignedSoftwareCertificate_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SignedSoftwareCertificate_Encode(OpcUa_SignedSoftwareCertificate* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_SignedSoftwareCertificate_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SignedSoftwareCertificate_Decode(OpcUa_SignedSoftwareCertificate* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SignedSoftwareCertificate_Initialize(a_pValue);

    status &= ByteString_Read(&a_pValue->CertificateData, msgBuf);
    status &= ByteString_Read(&a_pValue->Signature, msgBuf);

    if(status != STATUS_OK){
        OpcUa_SignedSoftwareCertificate_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SignedSoftwareCertificate_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SignedSoftwareCertificate_EncodeableType =
{
    "SignedSoftwareCertificate",
    OpcUaId_SignedSoftwareCertificate,
    OpcUaId_SignedSoftwareCertificate_Encoding_DefaultBinary,
    OpcUaId_SignedSoftwareCertificate_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SignedSoftwareCertificate),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SignedSoftwareCertificate_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SignedSoftwareCertificate_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SignedSoftwareCertificate_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SignedSoftwareCertificate_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SignedSoftwareCertificate_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SignatureData
/*============================================================================
 * OpcUa_SignatureData_Initialize
 *===========================================================================*/
void OpcUa_SignatureData_Initialize(OpcUa_SignatureData* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->Algorithm);
        ByteString_Initialize(&a_pValue->Signature);
    }
}

/*============================================================================
 * OpcUa_SignatureData_Clear
 *===========================================================================*/
void OpcUa_SignatureData_Clear(OpcUa_SignatureData* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->Algorithm);
        ByteString_Clear(&a_pValue->Signature);
    }
}

/*============================================================================
 * OpcUa_SignatureData_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SignatureData_Encode(OpcUa_SignatureData* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_SignatureData_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SignatureData_Decode(OpcUa_SignatureData* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SignatureData_Initialize(a_pValue);

    status &= String_Read(&a_pValue->Algorithm, msgBuf);
    status &= ByteString_Read(&a_pValue->Signature, msgBuf);

    if(status != STATUS_OK){
        OpcUa_SignatureData_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SignatureData_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SignatureData_EncodeableType =
{
    "SignatureData",
    OpcUaId_SignatureData,
    OpcUaId_SignatureData_Encoding_DefaultBinary,
    OpcUaId_SignatureData_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SignatureData),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SignatureData_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SignatureData_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SignatureData_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SignatureData_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SignatureData_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateSession
#ifndef OPCUA_EXCLUDE_CreateSessionRequest
/*============================================================================
 * OpcUa_CreateSessionRequest_Initialize
 *===========================================================================*/
void OpcUa_CreateSessionRequest_Initialize(OpcUa_CreateSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        OpcUa_ApplicationDescription_Initialize(&a_pValue->ClientDescription);
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
 * OpcUa_CreateSessionRequest_Clear
 *===========================================================================*/
void OpcUa_CreateSessionRequest_Clear(OpcUa_CreateSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        OpcUa_ApplicationDescription_Clear(&a_pValue->ClientDescription);
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
 * OpcUa_CreateSessionRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateSessionRequest_Encode(OpcUa_CreateSessionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_ApplicationDescription_Encode(&a_pValue->ClientDescription, msgBuf);
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
 * OpcUa_CreateSessionRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateSessionRequest_Decode(OpcUa_CreateSessionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CreateSessionRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_ApplicationDescription_Decode(&a_pValue->ClientDescription, msgBuf);
    status &= String_Read(&a_pValue->ServerUri, msgBuf);
    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= String_Read(&a_pValue->SessionName, msgBuf);
    status &= ByteString_Read(&a_pValue->ClientNonce, msgBuf);
    status &= ByteString_Read(&a_pValue->ClientCertificate, msgBuf);
    status &= Double_Read(&a_pValue->RequestedSessionTimeout, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxResponseMessageSize, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CreateSessionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CreateSessionRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CreateSessionRequest_EncodeableType =
{
    "CreateSessionRequest",
    OpcUaId_CreateSessionRequest,
    OpcUaId_CreateSessionRequest_Encoding_DefaultBinary,
    OpcUaId_CreateSessionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CreateSessionRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CreateSessionRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CreateSessionRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CreateSessionRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CreateSessionRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CreateSessionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateSessionResponse
/*============================================================================
 * OpcUa_CreateSessionResponse_Initialize
 *===========================================================================*/
void OpcUa_CreateSessionResponse_Initialize(OpcUa_CreateSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        NodeId_Initialize(&a_pValue->SessionId);
        NodeId_Initialize(&a_pValue->AuthenticationToken);
        Double_Initialize(&a_pValue->RevisedSessionTimeout);
        ByteString_Initialize(&a_pValue->ServerNonce);
        ByteString_Initialize(&a_pValue->ServerCertificate);
        SOPC_Initialize_Array(&a_pValue->NoOfServerEndpoints, (void**) &a_pValue->ServerEndpoints, 
                            sizeof(OpcUa_EndpointDescription), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_EndpointDescription_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfServerSoftwareCertificates, (void**) &a_pValue->ServerSoftwareCertificates, 
                            sizeof(OpcUa_SignedSoftwareCertificate), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_SignedSoftwareCertificate_Initialize);
        OpcUa_SignatureData_Initialize(&a_pValue->ServerSignature);
        UInt32_Initialize(&a_pValue->MaxRequestMessageSize);
    }
}

/*============================================================================
 * OpcUa_CreateSessionResponse_Clear
 *===========================================================================*/
void OpcUa_CreateSessionResponse_Clear(OpcUa_CreateSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        NodeId_Clear(&a_pValue->SessionId);
        NodeId_Clear(&a_pValue->AuthenticationToken);
        Double_Clear(&a_pValue->RevisedSessionTimeout);
        ByteString_Clear(&a_pValue->ServerNonce);
        ByteString_Clear(&a_pValue->ServerCertificate);
        SOPC_Clear_Array(&a_pValue->NoOfServerEndpoints, (void**) &a_pValue->ServerEndpoints, 
                       sizeof(OpcUa_EndpointDescription), (SOPC_EncodeableObject_PfnClear*) OpcUa_EndpointDescription_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfServerSoftwareCertificates, (void**) &a_pValue->ServerSoftwareCertificates, 
                       sizeof(OpcUa_SignedSoftwareCertificate), (SOPC_EncodeableObject_PfnClear*) OpcUa_SignedSoftwareCertificate_Clear);
        OpcUa_SignatureData_Clear(&a_pValue->ServerSignature);
        UInt32_Clear(&a_pValue->MaxRequestMessageSize);
    }
}

/*============================================================================
 * OpcUa_CreateSessionResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateSessionResponse_Encode(OpcUa_CreateSessionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= NodeId_Write(&a_pValue->SessionId, msgBuf);
    status &= NodeId_Write(&a_pValue->AuthenticationToken, msgBuf);
    status &= Double_Write(&a_pValue->RevisedSessionTimeout, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerNonce, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerCertificate, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfServerEndpoints, (void**) &a_pValue->ServerEndpoints, 
                   sizeof(OpcUa_EndpointDescription), (SOPC_EncodeableObject_PfnEncode*) OpcUa_EndpointDescription_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfServerSoftwareCertificates, (void**) &a_pValue->ServerSoftwareCertificates, 
                   sizeof(OpcUa_SignedSoftwareCertificate), (SOPC_EncodeableObject_PfnEncode*) OpcUa_SignedSoftwareCertificate_Encode);
    status &= OpcUa_SignatureData_Encode(&a_pValue->ServerSignature, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxRequestMessageSize, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CreateSessionResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateSessionResponse_Decode(OpcUa_CreateSessionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CreateSessionResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= NodeId_Read(&a_pValue->SessionId, msgBuf);
    status &= NodeId_Read(&a_pValue->AuthenticationToken, msgBuf);
    status &= Double_Read(&a_pValue->RevisedSessionTimeout, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerNonce, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerCertificate, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfServerEndpoints, (void**) &a_pValue->ServerEndpoints, 
                  sizeof(OpcUa_EndpointDescription), (SOPC_EncodeableObject_PfnDecode*) OpcUa_EndpointDescription_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfServerSoftwareCertificates, (void**) &a_pValue->ServerSoftwareCertificates, 
                  sizeof(OpcUa_SignedSoftwareCertificate), (SOPC_EncodeableObject_PfnDecode*) OpcUa_SignedSoftwareCertificate_Decode);
    status &= OpcUa_SignatureData_Decode(&a_pValue->ServerSignature, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxRequestMessageSize, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CreateSessionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CreateSessionResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CreateSessionResponse_EncodeableType =
{
    "CreateSessionResponse",
    OpcUaId_CreateSessionResponse,
    OpcUaId_CreateSessionResponse_Encoding_DefaultBinary,
    OpcUaId_CreateSessionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CreateSessionResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CreateSessionResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CreateSessionResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CreateSessionResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CreateSessionResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CreateSessionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_UserIdentityToken
/*============================================================================
 * OpcUa_UserIdentityToken_Initialize
 *===========================================================================*/
void OpcUa_UserIdentityToken_Initialize(OpcUa_UserIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
    }
}

/*============================================================================
 * OpcUa_UserIdentityToken_Clear
 *===========================================================================*/
void OpcUa_UserIdentityToken_Clear(OpcUa_UserIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
    }
}

/*============================================================================
 * OpcUa_UserIdentityToken_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UserIdentityToken_Encode(OpcUa_UserIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_UserIdentityToken_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UserIdentityToken_Decode(OpcUa_UserIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_UserIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);

    if(status != STATUS_OK){
        OpcUa_UserIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_UserIdentityToken_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_UserIdentityToken_EncodeableType =
{
    "UserIdentityToken",
    OpcUaId_UserIdentityToken,
    OpcUaId_UserIdentityToken_Encoding_DefaultBinary,
    OpcUaId_UserIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_UserIdentityToken),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_UserIdentityToken_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_UserIdentityToken_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_UserIdentityToken_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_UserIdentityToken_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_UserIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AnonymousIdentityToken
/*============================================================================
 * OpcUa_AnonymousIdentityToken_Initialize
 *===========================================================================*/
void OpcUa_AnonymousIdentityToken_Initialize(OpcUa_AnonymousIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
    }
}

/*============================================================================
 * OpcUa_AnonymousIdentityToken_Clear
 *===========================================================================*/
void OpcUa_AnonymousIdentityToken_Clear(OpcUa_AnonymousIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
    }
}

/*============================================================================
 * OpcUa_AnonymousIdentityToken_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AnonymousIdentityToken_Encode(OpcUa_AnonymousIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_AnonymousIdentityToken_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AnonymousIdentityToken_Decode(OpcUa_AnonymousIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AnonymousIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);

    if(status != STATUS_OK){
        OpcUa_AnonymousIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AnonymousIdentityToken_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AnonymousIdentityToken_EncodeableType =
{
    "AnonymousIdentityToken",
    OpcUaId_AnonymousIdentityToken,
    OpcUaId_AnonymousIdentityToken_Encoding_DefaultBinary,
    OpcUaId_AnonymousIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AnonymousIdentityToken),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AnonymousIdentityToken_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AnonymousIdentityToken_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AnonymousIdentityToken_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AnonymousIdentityToken_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AnonymousIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_UserNameIdentityToken
/*============================================================================
 * OpcUa_UserNameIdentityToken_Initialize
 *===========================================================================*/
void OpcUa_UserNameIdentityToken_Initialize(OpcUa_UserNameIdentityToken* a_pValue)
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
 * OpcUa_UserNameIdentityToken_Clear
 *===========================================================================*/
void OpcUa_UserNameIdentityToken_Clear(OpcUa_UserNameIdentityToken* a_pValue)
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
 * OpcUa_UserNameIdentityToken_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UserNameIdentityToken_Encode(OpcUa_UserNameIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_UserNameIdentityToken_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UserNameIdentityToken_Decode(OpcUa_UserNameIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_UserNameIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= String_Read(&a_pValue->UserName, msgBuf);
    status &= ByteString_Read(&a_pValue->Password, msgBuf);
    status &= String_Read(&a_pValue->EncryptionAlgorithm, msgBuf);

    if(status != STATUS_OK){
        OpcUa_UserNameIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_UserNameIdentityToken_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_UserNameIdentityToken_EncodeableType =
{
    "UserNameIdentityToken",
    OpcUaId_UserNameIdentityToken,
    OpcUaId_UserNameIdentityToken_Encoding_DefaultBinary,
    OpcUaId_UserNameIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_UserNameIdentityToken),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_UserNameIdentityToken_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_UserNameIdentityToken_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_UserNameIdentityToken_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_UserNameIdentityToken_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_UserNameIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_X509IdentityToken
/*============================================================================
 * OpcUa_X509IdentityToken_Initialize
 *===========================================================================*/
void OpcUa_X509IdentityToken_Initialize(OpcUa_X509IdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
        ByteString_Initialize(&a_pValue->CertificateData);
    }
}

/*============================================================================
 * OpcUa_X509IdentityToken_Clear
 *===========================================================================*/
void OpcUa_X509IdentityToken_Clear(OpcUa_X509IdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
        ByteString_Clear(&a_pValue->CertificateData);
    }
}

/*============================================================================
 * OpcUa_X509IdentityToken_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_X509IdentityToken_Encode(OpcUa_X509IdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_X509IdentityToken_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_X509IdentityToken_Decode(OpcUa_X509IdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_X509IdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= ByteString_Read(&a_pValue->CertificateData, msgBuf);

    if(status != STATUS_OK){
        OpcUa_X509IdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_X509IdentityToken_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_X509IdentityToken_EncodeableType =
{
    "X509IdentityToken",
    OpcUaId_X509IdentityToken,
    OpcUaId_X509IdentityToken_Encoding_DefaultBinary,
    OpcUaId_X509IdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_X509IdentityToken),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_X509IdentityToken_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_X509IdentityToken_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_X509IdentityToken_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_X509IdentityToken_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_X509IdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_KerberosIdentityToken
/*============================================================================
 * OpcUa_KerberosIdentityToken_Initialize
 *===========================================================================*/
void OpcUa_KerberosIdentityToken_Initialize(OpcUa_KerberosIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
        ByteString_Initialize(&a_pValue->TicketData);
    }
}

/*============================================================================
 * OpcUa_KerberosIdentityToken_Clear
 *===========================================================================*/
void OpcUa_KerberosIdentityToken_Clear(OpcUa_KerberosIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
        ByteString_Clear(&a_pValue->TicketData);
    }
}

/*============================================================================
 * OpcUa_KerberosIdentityToken_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_KerberosIdentityToken_Encode(OpcUa_KerberosIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_KerberosIdentityToken_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_KerberosIdentityToken_Decode(OpcUa_KerberosIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_KerberosIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= ByteString_Read(&a_pValue->TicketData, msgBuf);

    if(status != STATUS_OK){
        OpcUa_KerberosIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_KerberosIdentityToken_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_KerberosIdentityToken_EncodeableType =
{
    "KerberosIdentityToken",
    OpcUaId_KerberosIdentityToken,
    OpcUaId_KerberosIdentityToken_Encoding_DefaultBinary,
    OpcUaId_KerberosIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_KerberosIdentityToken),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_KerberosIdentityToken_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_KerberosIdentityToken_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_KerberosIdentityToken_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_KerberosIdentityToken_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_KerberosIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_IssuedIdentityToken
/*============================================================================
 * OpcUa_IssuedIdentityToken_Initialize
 *===========================================================================*/
void OpcUa_IssuedIdentityToken_Initialize(OpcUa_IssuedIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->PolicyId);
        ByteString_Initialize(&a_pValue->TokenData);
        String_Initialize(&a_pValue->EncryptionAlgorithm);
    }
}

/*============================================================================
 * OpcUa_IssuedIdentityToken_Clear
 *===========================================================================*/
void OpcUa_IssuedIdentityToken_Clear(OpcUa_IssuedIdentityToken* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->PolicyId);
        ByteString_Clear(&a_pValue->TokenData);
        String_Clear(&a_pValue->EncryptionAlgorithm);
    }
}

/*============================================================================
 * OpcUa_IssuedIdentityToken_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_IssuedIdentityToken_Encode(OpcUa_IssuedIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_IssuedIdentityToken_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_IssuedIdentityToken_Decode(OpcUa_IssuedIdentityToken* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_IssuedIdentityToken_Initialize(a_pValue);

    status &= String_Read(&a_pValue->PolicyId, msgBuf);
    status &= ByteString_Read(&a_pValue->TokenData, msgBuf);
    status &= String_Read(&a_pValue->EncryptionAlgorithm, msgBuf);

    if(status != STATUS_OK){
        OpcUa_IssuedIdentityToken_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_IssuedIdentityToken_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_IssuedIdentityToken_EncodeableType =
{
    "IssuedIdentityToken",
    OpcUaId_IssuedIdentityToken,
    OpcUaId_IssuedIdentityToken_Encoding_DefaultBinary,
    OpcUaId_IssuedIdentityToken_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_IssuedIdentityToken),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_IssuedIdentityToken_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_IssuedIdentityToken_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_IssuedIdentityToken_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_IssuedIdentityToken_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_IssuedIdentityToken_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ActivateSession
#ifndef OPCUA_EXCLUDE_ActivateSessionRequest
/*============================================================================
 * OpcUa_ActivateSessionRequest_Initialize
 *===========================================================================*/
void OpcUa_ActivateSessionRequest_Initialize(OpcUa_ActivateSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        OpcUa_SignatureData_Initialize(&a_pValue->ClientSignature);
        SOPC_Initialize_Array(&a_pValue->NoOfClientSoftwareCertificates, (void**) &a_pValue->ClientSoftwareCertificates, 
                            sizeof(OpcUa_SignedSoftwareCertificate), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_SignedSoftwareCertificate_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
        ExtensionObject_Initialize(&a_pValue->UserIdentityToken);
        OpcUa_SignatureData_Initialize(&a_pValue->UserTokenSignature);
    }
}

/*============================================================================
 * OpcUa_ActivateSessionRequest_Clear
 *===========================================================================*/
void OpcUa_ActivateSessionRequest_Clear(OpcUa_ActivateSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        OpcUa_SignatureData_Clear(&a_pValue->ClientSignature);
        SOPC_Clear_Array(&a_pValue->NoOfClientSoftwareCertificates, (void**) &a_pValue->ClientSoftwareCertificates, 
                       sizeof(OpcUa_SignedSoftwareCertificate), (SOPC_EncodeableObject_PfnClear*) OpcUa_SignedSoftwareCertificate_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
        ExtensionObject_Clear(&a_pValue->UserIdentityToken);
        OpcUa_SignatureData_Clear(&a_pValue->UserTokenSignature);
    }
}

/*============================================================================
 * OpcUa_ActivateSessionRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ActivateSessionRequest_Encode(OpcUa_ActivateSessionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_SignatureData_Encode(&a_pValue->ClientSignature, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfClientSoftwareCertificates, (void**) &a_pValue->ClientSoftwareCertificates, 
                   sizeof(OpcUa_SignedSoftwareCertificate), (SOPC_EncodeableObject_PfnEncode*) OpcUa_SignedSoftwareCertificate_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);
    status &= ExtensionObject_Write(&a_pValue->UserIdentityToken, msgBuf);
    status &= OpcUa_SignatureData_Encode(&a_pValue->UserTokenSignature, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ActivateSessionRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ActivateSessionRequest_Decode(OpcUa_ActivateSessionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ActivateSessionRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_SignatureData_Decode(&a_pValue->ClientSignature, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfClientSoftwareCertificates, (void**) &a_pValue->ClientSoftwareCertificates, 
                  sizeof(OpcUa_SignedSoftwareCertificate), (SOPC_EncodeableObject_PfnDecode*) OpcUa_SignedSoftwareCertificate_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);
    status &= ExtensionObject_Read(&a_pValue->UserIdentityToken, msgBuf);
    status &= OpcUa_SignatureData_Decode(&a_pValue->UserTokenSignature, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ActivateSessionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ActivateSessionRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ActivateSessionRequest_EncodeableType =
{
    "ActivateSessionRequest",
    OpcUaId_ActivateSessionRequest,
    OpcUaId_ActivateSessionRequest_Encoding_DefaultBinary,
    OpcUaId_ActivateSessionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ActivateSessionRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ActivateSessionRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ActivateSessionRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ActivateSessionRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ActivateSessionRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ActivateSessionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ActivateSessionResponse
/*============================================================================
 * OpcUa_ActivateSessionResponse_Initialize
 *===========================================================================*/
void OpcUa_ActivateSessionResponse_Initialize(OpcUa_ActivateSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        ByteString_Initialize(&a_pValue->ServerNonce);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_ActivateSessionResponse_Clear
 *===========================================================================*/
void OpcUa_ActivateSessionResponse_Clear(OpcUa_ActivateSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        ByteString_Clear(&a_pValue->ServerNonce);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_ActivateSessionResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ActivateSessionResponse_Encode(OpcUa_ActivateSessionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= ByteString_Write(&a_pValue->ServerNonce, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ActivateSessionResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ActivateSessionResponse_Decode(OpcUa_ActivateSessionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ActivateSessionResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= ByteString_Read(&a_pValue->ServerNonce, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_ActivateSessionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ActivateSessionResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ActivateSessionResponse_EncodeableType =
{
    "ActivateSessionResponse",
    OpcUaId_ActivateSessionResponse,
    OpcUaId_ActivateSessionResponse_Encoding_DefaultBinary,
    OpcUaId_ActivateSessionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ActivateSessionResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ActivateSessionResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ActivateSessionResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ActivateSessionResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ActivateSessionResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ActivateSessionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_CloseSession
#ifndef OPCUA_EXCLUDE_CloseSessionRequest
/*============================================================================
 * OpcUa_CloseSessionRequest_Initialize
 *===========================================================================*/
void OpcUa_CloseSessionRequest_Initialize(OpcUa_CloseSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Boolean_Initialize(&a_pValue->DeleteSubscriptions);
    }
}

/*============================================================================
 * OpcUa_CloseSessionRequest_Clear
 *===========================================================================*/
void OpcUa_CloseSessionRequest_Clear(OpcUa_CloseSessionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        Boolean_Clear(&a_pValue->DeleteSubscriptions);
    }
}

/*============================================================================
 * OpcUa_CloseSessionRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CloseSessionRequest_Encode(OpcUa_CloseSessionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Write(&a_pValue->DeleteSubscriptions, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CloseSessionRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CloseSessionRequest_Decode(OpcUa_CloseSessionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CloseSessionRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Read(&a_pValue->DeleteSubscriptions, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CloseSessionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CloseSessionRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CloseSessionRequest_EncodeableType =
{
    "CloseSessionRequest",
    OpcUaId_CloseSessionRequest,
    OpcUaId_CloseSessionRequest_Encoding_DefaultBinary,
    OpcUaId_CloseSessionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CloseSessionRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CloseSessionRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CloseSessionRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CloseSessionRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CloseSessionRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CloseSessionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CloseSessionResponse
/*============================================================================
 * OpcUa_CloseSessionResponse_Initialize
 *===========================================================================*/
void OpcUa_CloseSessionResponse_Initialize(OpcUa_CloseSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_CloseSessionResponse_Clear
 *===========================================================================*/
void OpcUa_CloseSessionResponse_Clear(OpcUa_CloseSessionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_CloseSessionResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CloseSessionResponse_Encode(OpcUa_CloseSessionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CloseSessionResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CloseSessionResponse_Decode(OpcUa_CloseSessionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CloseSessionResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CloseSessionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CloseSessionResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CloseSessionResponse_EncodeableType =
{
    "CloseSessionResponse",
    OpcUaId_CloseSessionResponse,
    OpcUaId_CloseSessionResponse_Encoding_DefaultBinary,
    OpcUaId_CloseSessionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CloseSessionResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CloseSessionResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CloseSessionResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CloseSessionResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CloseSessionResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CloseSessionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_Cancel
#ifndef OPCUA_EXCLUDE_CancelRequest
/*============================================================================
 * OpcUa_CancelRequest_Initialize
 *===========================================================================*/
void OpcUa_CancelRequest_Initialize(OpcUa_CancelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->RequestHandle);
    }
}

/*============================================================================
 * OpcUa_CancelRequest_Clear
 *===========================================================================*/
void OpcUa_CancelRequest_Clear(OpcUa_CancelRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->RequestHandle);
    }
}

/*============================================================================
 * OpcUa_CancelRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CancelRequest_Encode(OpcUa_CancelRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestHandle, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CancelRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CancelRequest_Decode(OpcUa_CancelRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CancelRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestHandle, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CancelRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CancelRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CancelRequest_EncodeableType =
{
    "CancelRequest",
    OpcUaId_CancelRequest,
    OpcUaId_CancelRequest_Encoding_DefaultBinary,
    OpcUaId_CancelRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CancelRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CancelRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CancelRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CancelRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CancelRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CancelRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CancelResponse
/*============================================================================
 * OpcUa_CancelResponse_Initialize
 *===========================================================================*/
void OpcUa_CancelResponse_Initialize(OpcUa_CancelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UInt32_Initialize(&a_pValue->CancelCount);
    }
}

/*============================================================================
 * OpcUa_CancelResponse_Clear
 *===========================================================================*/
void OpcUa_CancelResponse_Clear(OpcUa_CancelResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UInt32_Clear(&a_pValue->CancelCount);
    }
}

/*============================================================================
 * OpcUa_CancelResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CancelResponse_Encode(OpcUa_CancelResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->CancelCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CancelResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CancelResponse_Decode(OpcUa_CancelResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CancelResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->CancelCount, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CancelResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CancelResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CancelResponse_EncodeableType =
{
    "CancelResponse",
    OpcUaId_CancelResponse,
    OpcUaId_CancelResponse_Encoding_DefaultBinary,
    OpcUaId_CancelResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CancelResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CancelResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CancelResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CancelResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CancelResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CancelResponse_Decode
};
#endif
#endif


#ifndef OPCUA_EXCLUDE_NodeAttributes
/*============================================================================
 * OpcUa_NodeAttributes_Initialize
 *===========================================================================*/
void OpcUa_NodeAttributes_Initialize(OpcUa_NodeAttributes* a_pValue)
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
 * OpcUa_NodeAttributes_Clear
 *===========================================================================*/
void OpcUa_NodeAttributes_Clear(OpcUa_NodeAttributes* a_pValue)
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
 * OpcUa_NodeAttributes_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NodeAttributes_Encode(OpcUa_NodeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_NodeAttributes_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NodeAttributes_Decode(OpcUa_NodeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_NodeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);

    if(status != STATUS_OK){
        OpcUa_NodeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_NodeAttributes_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_NodeAttributes_EncodeableType =
{
    "NodeAttributes",
    OpcUaId_NodeAttributes,
    OpcUaId_NodeAttributes_Encoding_DefaultBinary,
    OpcUaId_NodeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_NodeAttributes),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_NodeAttributes_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_NodeAttributes_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_NodeAttributes_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_NodeAttributes_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_NodeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ObjectAttributes
/*============================================================================
 * OpcUa_ObjectAttributes_Initialize
 *===========================================================================*/
void OpcUa_ObjectAttributes_Initialize(OpcUa_ObjectAttributes* a_pValue)
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
 * OpcUa_ObjectAttributes_Clear
 *===========================================================================*/
void OpcUa_ObjectAttributes_Clear(OpcUa_ObjectAttributes* a_pValue)
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
 * OpcUa_ObjectAttributes_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ObjectAttributes_Encode(OpcUa_ObjectAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ObjectAttributes_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ObjectAttributes_Decode(OpcUa_ObjectAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ObjectAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Byte_Read(&a_pValue->EventNotifier, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ObjectAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ObjectAttributes_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ObjectAttributes_EncodeableType =
{
    "ObjectAttributes",
    OpcUaId_ObjectAttributes,
    OpcUaId_ObjectAttributes_Encoding_DefaultBinary,
    OpcUaId_ObjectAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ObjectAttributes),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ObjectAttributes_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ObjectAttributes_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ObjectAttributes_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ObjectAttributes_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ObjectAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_VariableAttributes
/*============================================================================
 * OpcUa_VariableAttributes_Initialize
 *===========================================================================*/
void OpcUa_VariableAttributes_Initialize(OpcUa_VariableAttributes* a_pValue)
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
        SOPC_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Byte_Initialize(&a_pValue->AccessLevel);
        Byte_Initialize(&a_pValue->UserAccessLevel);
        Double_Initialize(&a_pValue->MinimumSamplingInterval);
        Boolean_Initialize(&a_pValue->Historizing);
    }
}

/*============================================================================
 * OpcUa_VariableAttributes_Clear
 *===========================================================================*/
void OpcUa_VariableAttributes_Clear(OpcUa_VariableAttributes* a_pValue)
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
        SOPC_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
        Byte_Clear(&a_pValue->AccessLevel);
        Byte_Clear(&a_pValue->UserAccessLevel);
        Double_Clear(&a_pValue->MinimumSamplingInterval);
        Boolean_Clear(&a_pValue->Historizing);
    }
}

/*============================================================================
 * OpcUa_VariableAttributes_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_VariableAttributes_Encode(OpcUa_VariableAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);
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
 * OpcUa_VariableAttributes_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_VariableAttributes_Decode(OpcUa_VariableAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_VariableAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Variant_Read(&a_pValue->Value, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Byte_Read(&a_pValue->AccessLevel, msgBuf);
    status &= Byte_Read(&a_pValue->UserAccessLevel, msgBuf);
    status &= Double_Read(&a_pValue->MinimumSamplingInterval, msgBuf);
    status &= Boolean_Read(&a_pValue->Historizing, msgBuf);

    if(status != STATUS_OK){
        OpcUa_VariableAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_VariableAttributes_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_VariableAttributes_EncodeableType =
{
    "VariableAttributes",
    OpcUaId_VariableAttributes,
    OpcUaId_VariableAttributes_Encoding_DefaultBinary,
    OpcUaId_VariableAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_VariableAttributes),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_VariableAttributes_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_VariableAttributes_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_VariableAttributes_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_VariableAttributes_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_VariableAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MethodAttributes
/*============================================================================
 * OpcUa_MethodAttributes_Initialize
 *===========================================================================*/
void OpcUa_MethodAttributes_Initialize(OpcUa_MethodAttributes* a_pValue)
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
 * OpcUa_MethodAttributes_Clear
 *===========================================================================*/
void OpcUa_MethodAttributes_Clear(OpcUa_MethodAttributes* a_pValue)
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
 * OpcUa_MethodAttributes_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MethodAttributes_Encode(OpcUa_MethodAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_MethodAttributes_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MethodAttributes_Decode(OpcUa_MethodAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_MethodAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->Executable, msgBuf);
    status &= Boolean_Read(&a_pValue->UserExecutable, msgBuf);

    if(status != STATUS_OK){
        OpcUa_MethodAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_MethodAttributes_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_MethodAttributes_EncodeableType =
{
    "MethodAttributes",
    OpcUaId_MethodAttributes,
    OpcUaId_MethodAttributes_Encoding_DefaultBinary,
    OpcUaId_MethodAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_MethodAttributes),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_MethodAttributes_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_MethodAttributes_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_MethodAttributes_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_MethodAttributes_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_MethodAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ObjectTypeAttributes
/*============================================================================
 * OpcUa_ObjectTypeAttributes_Initialize
 *===========================================================================*/
void OpcUa_ObjectTypeAttributes_Initialize(OpcUa_ObjectTypeAttributes* a_pValue)
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
 * OpcUa_ObjectTypeAttributes_Clear
 *===========================================================================*/
void OpcUa_ObjectTypeAttributes_Clear(OpcUa_ObjectTypeAttributes* a_pValue)
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
 * OpcUa_ObjectTypeAttributes_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ObjectTypeAttributes_Encode(OpcUa_ObjectTypeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ObjectTypeAttributes_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ObjectTypeAttributes_Decode(OpcUa_ObjectTypeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ObjectTypeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ObjectTypeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ObjectTypeAttributes_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ObjectTypeAttributes_EncodeableType =
{
    "ObjectTypeAttributes",
    OpcUaId_ObjectTypeAttributes,
    OpcUaId_ObjectTypeAttributes_Encoding_DefaultBinary,
    OpcUaId_ObjectTypeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ObjectTypeAttributes),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ObjectTypeAttributes_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ObjectTypeAttributes_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ObjectTypeAttributes_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ObjectTypeAttributes_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ObjectTypeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_VariableTypeAttributes
/*============================================================================
 * OpcUa_VariableTypeAttributes_Initialize
 *===========================================================================*/
void OpcUa_VariableTypeAttributes_Initialize(OpcUa_VariableTypeAttributes* a_pValue)
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
        SOPC_Initialize_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Boolean_Initialize(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * OpcUa_VariableTypeAttributes_Clear
 *===========================================================================*/
void OpcUa_VariableTypeAttributes_Clear(OpcUa_VariableTypeAttributes* a_pValue)
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
        SOPC_Clear_Array(&a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
        Boolean_Clear(&a_pValue->IsAbstract);
    }
}

/*============================================================================
 * OpcUa_VariableTypeAttributes_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_VariableTypeAttributes_Encode(OpcUa_VariableTypeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Boolean_Write(&a_pValue->IsAbstract, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_VariableTypeAttributes_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_VariableTypeAttributes_Decode(OpcUa_VariableTypeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_VariableTypeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Variant_Read(&a_pValue->Value, msgBuf);
    status &= NodeId_Read(&a_pValue->DataType, msgBuf);
    status &= Int32_Read(&a_pValue->ValueRank, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfArrayDimensions, (void**) &a_pValue->ArrayDimensions, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        OpcUa_VariableTypeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_VariableTypeAttributes_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_VariableTypeAttributes_EncodeableType =
{
    "VariableTypeAttributes",
    OpcUaId_VariableTypeAttributes,
    OpcUaId_VariableTypeAttributes_Encoding_DefaultBinary,
    OpcUaId_VariableTypeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_VariableTypeAttributes),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_VariableTypeAttributes_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_VariableTypeAttributes_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_VariableTypeAttributes_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_VariableTypeAttributes_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_VariableTypeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReferenceTypeAttributes
/*============================================================================
 * OpcUa_ReferenceTypeAttributes_Initialize
 *===========================================================================*/
void OpcUa_ReferenceTypeAttributes_Initialize(OpcUa_ReferenceTypeAttributes* a_pValue)
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
 * OpcUa_ReferenceTypeAttributes_Clear
 *===========================================================================*/
void OpcUa_ReferenceTypeAttributes_Clear(OpcUa_ReferenceTypeAttributes* a_pValue)
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
 * OpcUa_ReferenceTypeAttributes_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReferenceTypeAttributes_Encode(OpcUa_ReferenceTypeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ReferenceTypeAttributes_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReferenceTypeAttributes_Decode(OpcUa_ReferenceTypeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReferenceTypeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);
    status &= Boolean_Read(&a_pValue->Symmetric, msgBuf);
    status &= LocalizedText_Read(&a_pValue->InverseName, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ReferenceTypeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReferenceTypeAttributes_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReferenceTypeAttributes_EncodeableType =
{
    "ReferenceTypeAttributes",
    OpcUaId_ReferenceTypeAttributes,
    OpcUaId_ReferenceTypeAttributes_Encoding_DefaultBinary,
    OpcUaId_ReferenceTypeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReferenceTypeAttributes),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReferenceTypeAttributes_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReferenceTypeAttributes_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReferenceTypeAttributes_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReferenceTypeAttributes_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReferenceTypeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DataTypeAttributes
/*============================================================================
 * OpcUa_DataTypeAttributes_Initialize
 *===========================================================================*/
void OpcUa_DataTypeAttributes_Initialize(OpcUa_DataTypeAttributes* a_pValue)
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
 * OpcUa_DataTypeAttributes_Clear
 *===========================================================================*/
void OpcUa_DataTypeAttributes_Clear(OpcUa_DataTypeAttributes* a_pValue)
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
 * OpcUa_DataTypeAttributes_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DataTypeAttributes_Encode(OpcUa_DataTypeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_DataTypeAttributes_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DataTypeAttributes_Decode(OpcUa_DataTypeAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DataTypeAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->IsAbstract, msgBuf);

    if(status != STATUS_OK){
        OpcUa_DataTypeAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DataTypeAttributes_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DataTypeAttributes_EncodeableType =
{
    "DataTypeAttributes",
    OpcUaId_DataTypeAttributes,
    OpcUaId_DataTypeAttributes_Encoding_DefaultBinary,
    OpcUaId_DataTypeAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DataTypeAttributes),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DataTypeAttributes_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DataTypeAttributes_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DataTypeAttributes_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DataTypeAttributes_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DataTypeAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ViewAttributes
/*============================================================================
 * OpcUa_ViewAttributes_Initialize
 *===========================================================================*/
void OpcUa_ViewAttributes_Initialize(OpcUa_ViewAttributes* a_pValue)
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
 * OpcUa_ViewAttributes_Clear
 *===========================================================================*/
void OpcUa_ViewAttributes_Clear(OpcUa_ViewAttributes* a_pValue)
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
 * OpcUa_ViewAttributes_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ViewAttributes_Encode(OpcUa_ViewAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ViewAttributes_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ViewAttributes_Decode(OpcUa_ViewAttributes* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ViewAttributes_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SpecifiedAttributes, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);
    status &= UInt32_Read(&a_pValue->WriteMask, msgBuf);
    status &= UInt32_Read(&a_pValue->UserWriteMask, msgBuf);
    status &= Boolean_Read(&a_pValue->ContainsNoLoops, msgBuf);
    status &= Byte_Read(&a_pValue->EventNotifier, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ViewAttributes_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ViewAttributes_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ViewAttributes_EncodeableType =
{
    "ViewAttributes",
    OpcUaId_ViewAttributes,
    OpcUaId_ViewAttributes_Encoding_DefaultBinary,
    OpcUaId_ViewAttributes_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ViewAttributes),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ViewAttributes_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ViewAttributes_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ViewAttributes_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ViewAttributes_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ViewAttributes_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddNodesItem
/*============================================================================
 * OpcUa_AddNodesItem_Initialize
 *===========================================================================*/
void OpcUa_AddNodesItem_Initialize(OpcUa_AddNodesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Initialize(&a_pValue->ParentNodeId);
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        ExpandedNodeId_Initialize(&a_pValue->RequestedNewNodeId);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        ExtensionObject_Initialize(&a_pValue->NodeAttributes);
        ExpandedNodeId_Initialize(&a_pValue->TypeDefinition);
    }
}

/*============================================================================
 * OpcUa_AddNodesItem_Clear
 *===========================================================================*/
void OpcUa_AddNodesItem_Clear(OpcUa_AddNodesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Clear(&a_pValue->ParentNodeId);
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        ExpandedNodeId_Clear(&a_pValue->RequestedNewNodeId);
        QualifiedName_Clear(&a_pValue->BrowseName);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        ExtensionObject_Clear(&a_pValue->NodeAttributes);
        ExpandedNodeId_Clear(&a_pValue->TypeDefinition);
    }
}

/*============================================================================
 * OpcUa_AddNodesItem_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddNodesItem_Encode(OpcUa_AddNodesItem* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= ExpandedNodeId_Write(&a_pValue->ParentNodeId, msgBuf);
    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->RequestedNewNodeId, msgBuf);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= ExtensionObject_Write(&a_pValue->NodeAttributes, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->TypeDefinition, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AddNodesItem_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddNodesItem_Decode(OpcUa_AddNodesItem* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AddNodesItem_Initialize(a_pValue);

    status &= ExpandedNodeId_Read(&a_pValue->ParentNodeId, msgBuf);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->RequestedNewNodeId, msgBuf);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= ExtensionObject_Read(&a_pValue->NodeAttributes, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TypeDefinition, msgBuf);

    if(status != STATUS_OK){
        OpcUa_AddNodesItem_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AddNodesItem_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AddNodesItem_EncodeableType =
{
    "AddNodesItem",
    OpcUaId_AddNodesItem,
    OpcUaId_AddNodesItem_Encoding_DefaultBinary,
    OpcUaId_AddNodesItem_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AddNodesItem),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AddNodesItem_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AddNodesItem_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AddNodesItem_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AddNodesItem_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AddNodesItem_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddNodesResult
/*============================================================================
 * OpcUa_AddNodesResult_Initialize
 *===========================================================================*/
void OpcUa_AddNodesResult_Initialize(OpcUa_AddNodesResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        NodeId_Initialize(&a_pValue->AddedNodeId);
    }
}

/*============================================================================
 * OpcUa_AddNodesResult_Clear
 *===========================================================================*/
void OpcUa_AddNodesResult_Clear(OpcUa_AddNodesResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        NodeId_Clear(&a_pValue->AddedNodeId);
    }
}

/*============================================================================
 * OpcUa_AddNodesResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddNodesResult_Encode(OpcUa_AddNodesResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_AddNodesResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddNodesResult_Decode(OpcUa_AddNodesResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AddNodesResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= NodeId_Read(&a_pValue->AddedNodeId, msgBuf);

    if(status != STATUS_OK){
        OpcUa_AddNodesResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AddNodesResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AddNodesResult_EncodeableType =
{
    "AddNodesResult",
    OpcUaId_AddNodesResult,
    OpcUaId_AddNodesResult_Encoding_DefaultBinary,
    OpcUaId_AddNodesResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AddNodesResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AddNodesResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AddNodesResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AddNodesResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AddNodesResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AddNodesResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddNodes
#ifndef OPCUA_EXCLUDE_AddNodesRequest
/*============================================================================
 * OpcUa_AddNodesRequest_Initialize
 *===========================================================================*/
void OpcUa_AddNodesRequest_Initialize(OpcUa_AddNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfNodesToAdd, (void**) &a_pValue->NodesToAdd, 
                            sizeof(OpcUa_AddNodesItem), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_AddNodesItem_Initialize);
    }
}

/*============================================================================
 * OpcUa_AddNodesRequest_Clear
 *===========================================================================*/
void OpcUa_AddNodesRequest_Clear(OpcUa_AddNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfNodesToAdd, (void**) &a_pValue->NodesToAdd, 
                       sizeof(OpcUa_AddNodesItem), (SOPC_EncodeableObject_PfnClear*) OpcUa_AddNodesItem_Clear);
    }
}

/*============================================================================
 * OpcUa_AddNodesRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddNodesRequest_Encode(OpcUa_AddNodesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNodesToAdd, (void**) &a_pValue->NodesToAdd, 
                   sizeof(OpcUa_AddNodesItem), (SOPC_EncodeableObject_PfnEncode*) OpcUa_AddNodesItem_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AddNodesRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddNodesRequest_Decode(OpcUa_AddNodesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AddNodesRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNodesToAdd, (void**) &a_pValue->NodesToAdd, 
                  sizeof(OpcUa_AddNodesItem), (SOPC_EncodeableObject_PfnDecode*) OpcUa_AddNodesItem_Decode);

    if(status != STATUS_OK){
        OpcUa_AddNodesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AddNodesRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AddNodesRequest_EncodeableType =
{
    "AddNodesRequest",
    OpcUaId_AddNodesRequest,
    OpcUaId_AddNodesRequest_Encoding_DefaultBinary,
    OpcUaId_AddNodesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AddNodesRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AddNodesRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AddNodesRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AddNodesRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AddNodesRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AddNodesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddNodesResponse
/*============================================================================
 * OpcUa_AddNodesResponse_Initialize
 *===========================================================================*/
void OpcUa_AddNodesResponse_Initialize(OpcUa_AddNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_AddNodesResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_AddNodesResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_AddNodesResponse_Clear
 *===========================================================================*/
void OpcUa_AddNodesResponse_Clear(OpcUa_AddNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_AddNodesResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_AddNodesResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_AddNodesResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddNodesResponse_Encode(OpcUa_AddNodesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_AddNodesResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_AddNodesResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AddNodesResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddNodesResponse_Decode(OpcUa_AddNodesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AddNodesResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_AddNodesResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_AddNodesResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_AddNodesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AddNodesResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AddNodesResponse_EncodeableType =
{
    "AddNodesResponse",
    OpcUaId_AddNodesResponse,
    OpcUaId_AddNodesResponse_Encoding_DefaultBinary,
    OpcUaId_AddNodesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AddNodesResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AddNodesResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AddNodesResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AddNodesResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AddNodesResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AddNodesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesItem
/*============================================================================
 * OpcUa_AddReferencesItem_Initialize
 *===========================================================================*/
void OpcUa_AddReferencesItem_Initialize(OpcUa_AddReferencesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->SourceNodeId);
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsForward);
        String_Initialize(&a_pValue->TargetServerUri);
        ExpandedNodeId_Initialize(&a_pValue->TargetNodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->TargetNodeClass);
    }
}

/*============================================================================
 * OpcUa_AddReferencesItem_Clear
 *===========================================================================*/
void OpcUa_AddReferencesItem_Clear(OpcUa_AddReferencesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->SourceNodeId);
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsForward);
        String_Clear(&a_pValue->TargetServerUri);
        ExpandedNodeId_Clear(&a_pValue->TargetNodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->TargetNodeClass);
    }
}

/*============================================================================
 * OpcUa_AddReferencesItem_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddReferencesItem_Encode(OpcUa_AddReferencesItem* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->SourceNodeId, msgBuf);
    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsForward, msgBuf);
    status &= String_Write(&a_pValue->TargetServerUri, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->TargetNodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TargetNodeClass);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AddReferencesItem_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddReferencesItem_Decode(OpcUa_AddReferencesItem* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AddReferencesItem_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->SourceNodeId, msgBuf);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsForward, msgBuf);
    status &= String_Read(&a_pValue->TargetServerUri, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TargetNodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TargetNodeClass);

    if(status != STATUS_OK){
        OpcUa_AddReferencesItem_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AddReferencesItem_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AddReferencesItem_EncodeableType =
{
    "AddReferencesItem",
    OpcUaId_AddReferencesItem,
    OpcUaId_AddReferencesItem_Encoding_DefaultBinary,
    OpcUaId_AddReferencesItem_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AddReferencesItem),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AddReferencesItem_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AddReferencesItem_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AddReferencesItem_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AddReferencesItem_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AddReferencesItem_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddReferences
#ifndef OPCUA_EXCLUDE_AddReferencesRequest
/*============================================================================
 * OpcUa_AddReferencesRequest_Initialize
 *===========================================================================*/
void OpcUa_AddReferencesRequest_Initialize(OpcUa_AddReferencesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfReferencesToAdd, (void**) &a_pValue->ReferencesToAdd, 
                            sizeof(OpcUa_AddReferencesItem), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_AddReferencesItem_Initialize);
    }
}

/*============================================================================
 * OpcUa_AddReferencesRequest_Clear
 *===========================================================================*/
void OpcUa_AddReferencesRequest_Clear(OpcUa_AddReferencesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfReferencesToAdd, (void**) &a_pValue->ReferencesToAdd, 
                       sizeof(OpcUa_AddReferencesItem), (SOPC_EncodeableObject_PfnClear*) OpcUa_AddReferencesItem_Clear);
    }
}

/*============================================================================
 * OpcUa_AddReferencesRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddReferencesRequest_Encode(OpcUa_AddReferencesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferencesToAdd, (void**) &a_pValue->ReferencesToAdd, 
                   sizeof(OpcUa_AddReferencesItem), (SOPC_EncodeableObject_PfnEncode*) OpcUa_AddReferencesItem_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AddReferencesRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddReferencesRequest_Decode(OpcUa_AddReferencesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AddReferencesRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferencesToAdd, (void**) &a_pValue->ReferencesToAdd, 
                  sizeof(OpcUa_AddReferencesItem), (SOPC_EncodeableObject_PfnDecode*) OpcUa_AddReferencesItem_Decode);

    if(status != STATUS_OK){
        OpcUa_AddReferencesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AddReferencesRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AddReferencesRequest_EncodeableType =
{
    "AddReferencesRequest",
    OpcUaId_AddReferencesRequest,
    OpcUaId_AddReferencesRequest_Encoding_DefaultBinary,
    OpcUaId_AddReferencesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AddReferencesRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AddReferencesRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AddReferencesRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AddReferencesRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AddReferencesRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AddReferencesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AddReferencesResponse
/*============================================================================
 * OpcUa_AddReferencesResponse_Initialize
 *===========================================================================*/
void OpcUa_AddReferencesResponse_Initialize(OpcUa_AddReferencesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_AddReferencesResponse_Clear
 *===========================================================================*/
void OpcUa_AddReferencesResponse_Clear(OpcUa_AddReferencesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_AddReferencesResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddReferencesResponse_Encode(OpcUa_AddReferencesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AddReferencesResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AddReferencesResponse_Decode(OpcUa_AddReferencesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AddReferencesResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_AddReferencesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AddReferencesResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AddReferencesResponse_EncodeableType =
{
    "AddReferencesResponse",
    OpcUaId_AddReferencesResponse,
    OpcUaId_AddReferencesResponse_Encoding_DefaultBinary,
    OpcUaId_AddReferencesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AddReferencesResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AddReferencesResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AddReferencesResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AddReferencesResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AddReferencesResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AddReferencesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesItem
/*============================================================================
 * OpcUa_DeleteNodesItem_Initialize
 *===========================================================================*/
void OpcUa_DeleteNodesItem_Initialize(OpcUa_DeleteNodesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        Boolean_Initialize(&a_pValue->DeleteTargetReferences);
    }
}

/*============================================================================
 * OpcUa_DeleteNodesItem_Clear
 *===========================================================================*/
void OpcUa_DeleteNodesItem_Clear(OpcUa_DeleteNodesItem* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        Boolean_Clear(&a_pValue->DeleteTargetReferences);
    }
}

/*============================================================================
 * OpcUa_DeleteNodesItem_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteNodesItem_Encode(OpcUa_DeleteNodesItem* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_DeleteNodesItem_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteNodesItem_Decode(OpcUa_DeleteNodesItem* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteNodesItem_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= Boolean_Read(&a_pValue->DeleteTargetReferences, msgBuf);

    if(status != STATUS_OK){
        OpcUa_DeleteNodesItem_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteNodesItem_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteNodesItem_EncodeableType =
{
    "DeleteNodesItem",
    OpcUaId_DeleteNodesItem,
    OpcUaId_DeleteNodesItem_Encoding_DefaultBinary,
    OpcUaId_DeleteNodesItem_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteNodesItem),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteNodesItem_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteNodesItem_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteNodesItem_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteNodesItem_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteNodesItem_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodes
#ifndef OPCUA_EXCLUDE_DeleteNodesRequest
/*============================================================================
 * OpcUa_DeleteNodesRequest_Initialize
 *===========================================================================*/
void OpcUa_DeleteNodesRequest_Initialize(OpcUa_DeleteNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfNodesToDelete, (void**) &a_pValue->NodesToDelete, 
                            sizeof(OpcUa_DeleteNodesItem), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_DeleteNodesItem_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteNodesRequest_Clear
 *===========================================================================*/
void OpcUa_DeleteNodesRequest_Clear(OpcUa_DeleteNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfNodesToDelete, (void**) &a_pValue->NodesToDelete, 
                       sizeof(OpcUa_DeleteNodesItem), (SOPC_EncodeableObject_PfnClear*) OpcUa_DeleteNodesItem_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteNodesRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteNodesRequest_Encode(OpcUa_DeleteNodesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNodesToDelete, (void**) &a_pValue->NodesToDelete, 
                   sizeof(OpcUa_DeleteNodesItem), (SOPC_EncodeableObject_PfnEncode*) OpcUa_DeleteNodesItem_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteNodesRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteNodesRequest_Decode(OpcUa_DeleteNodesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteNodesRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNodesToDelete, (void**) &a_pValue->NodesToDelete, 
                  sizeof(OpcUa_DeleteNodesItem), (SOPC_EncodeableObject_PfnDecode*) OpcUa_DeleteNodesItem_Decode);

    if(status != STATUS_OK){
        OpcUa_DeleteNodesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteNodesRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteNodesRequest_EncodeableType =
{
    "DeleteNodesRequest",
    OpcUaId_DeleteNodesRequest,
    OpcUaId_DeleteNodesRequest_Encoding_DefaultBinary,
    OpcUaId_DeleteNodesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteNodesRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteNodesRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteNodesRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteNodesRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteNodesRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteNodesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteNodesResponse
/*============================================================================
 * OpcUa_DeleteNodesResponse_Initialize
 *===========================================================================*/
void OpcUa_DeleteNodesResponse_Initialize(OpcUa_DeleteNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteNodesResponse_Clear
 *===========================================================================*/
void OpcUa_DeleteNodesResponse_Clear(OpcUa_DeleteNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteNodesResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteNodesResponse_Encode(OpcUa_DeleteNodesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteNodesResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteNodesResponse_Decode(OpcUa_DeleteNodesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteNodesResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_DeleteNodesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteNodesResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteNodesResponse_EncodeableType =
{
    "DeleteNodesResponse",
    OpcUaId_DeleteNodesResponse,
    OpcUaId_DeleteNodesResponse_Encoding_DefaultBinary,
    OpcUaId_DeleteNodesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteNodesResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteNodesResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteNodesResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteNodesResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteNodesResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteNodesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesItem
/*============================================================================
 * OpcUa_DeleteReferencesItem_Initialize
 *===========================================================================*/
void OpcUa_DeleteReferencesItem_Initialize(OpcUa_DeleteReferencesItem* a_pValue)
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
 * OpcUa_DeleteReferencesItem_Clear
 *===========================================================================*/
void OpcUa_DeleteReferencesItem_Clear(OpcUa_DeleteReferencesItem* a_pValue)
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
 * OpcUa_DeleteReferencesItem_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteReferencesItem_Encode(OpcUa_DeleteReferencesItem* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_DeleteReferencesItem_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteReferencesItem_Decode(OpcUa_DeleteReferencesItem* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteReferencesItem_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->SourceNodeId, msgBuf);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsForward, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TargetNodeId, msgBuf);
    status &= Boolean_Read(&a_pValue->DeleteBidirectional, msgBuf);

    if(status != STATUS_OK){
        OpcUa_DeleteReferencesItem_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteReferencesItem_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteReferencesItem_EncodeableType =
{
    "DeleteReferencesItem",
    OpcUaId_DeleteReferencesItem,
    OpcUaId_DeleteReferencesItem_Encoding_DefaultBinary,
    OpcUaId_DeleteReferencesItem_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteReferencesItem),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteReferencesItem_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteReferencesItem_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteReferencesItem_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteReferencesItem_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteReferencesItem_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferences
#ifndef OPCUA_EXCLUDE_DeleteReferencesRequest
/*============================================================================
 * OpcUa_DeleteReferencesRequest_Initialize
 *===========================================================================*/
void OpcUa_DeleteReferencesRequest_Initialize(OpcUa_DeleteReferencesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfReferencesToDelete, (void**) &a_pValue->ReferencesToDelete, 
                            sizeof(OpcUa_DeleteReferencesItem), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_DeleteReferencesItem_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteReferencesRequest_Clear
 *===========================================================================*/
void OpcUa_DeleteReferencesRequest_Clear(OpcUa_DeleteReferencesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfReferencesToDelete, (void**) &a_pValue->ReferencesToDelete, 
                       sizeof(OpcUa_DeleteReferencesItem), (SOPC_EncodeableObject_PfnClear*) OpcUa_DeleteReferencesItem_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteReferencesRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteReferencesRequest_Encode(OpcUa_DeleteReferencesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferencesToDelete, (void**) &a_pValue->ReferencesToDelete, 
                   sizeof(OpcUa_DeleteReferencesItem), (SOPC_EncodeableObject_PfnEncode*) OpcUa_DeleteReferencesItem_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteReferencesRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteReferencesRequest_Decode(OpcUa_DeleteReferencesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteReferencesRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferencesToDelete, (void**) &a_pValue->ReferencesToDelete, 
                  sizeof(OpcUa_DeleteReferencesItem), (SOPC_EncodeableObject_PfnDecode*) OpcUa_DeleteReferencesItem_Decode);

    if(status != STATUS_OK){
        OpcUa_DeleteReferencesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteReferencesRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteReferencesRequest_EncodeableType =
{
    "DeleteReferencesRequest",
    OpcUaId_DeleteReferencesRequest,
    OpcUaId_DeleteReferencesRequest_Encoding_DefaultBinary,
    OpcUaId_DeleteReferencesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteReferencesRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteReferencesRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteReferencesRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteReferencesRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteReferencesRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteReferencesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteReferencesResponse
/*============================================================================
 * OpcUa_DeleteReferencesResponse_Initialize
 *===========================================================================*/
void OpcUa_DeleteReferencesResponse_Initialize(OpcUa_DeleteReferencesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteReferencesResponse_Clear
 *===========================================================================*/
void OpcUa_DeleteReferencesResponse_Clear(OpcUa_DeleteReferencesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteReferencesResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteReferencesResponse_Encode(OpcUa_DeleteReferencesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteReferencesResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteReferencesResponse_Decode(OpcUa_DeleteReferencesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteReferencesResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_DeleteReferencesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteReferencesResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteReferencesResponse_EncodeableType =
{
    "DeleteReferencesResponse",
    OpcUaId_DeleteReferencesResponse,
    OpcUaId_DeleteReferencesResponse_Encoding_DefaultBinary,
    OpcUaId_DeleteReferencesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteReferencesResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteReferencesResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteReferencesResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteReferencesResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteReferencesResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteReferencesResponse_Decode
};
#endif
#endif



#ifndef OPCUA_EXCLUDE_ViewDescription
/*============================================================================
 * OpcUa_ViewDescription_Initialize
 *===========================================================================*/
void OpcUa_ViewDescription_Initialize(OpcUa_ViewDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->ViewId);
        DateTime_Initialize(&a_pValue->Timestamp);
        UInt32_Initialize(&a_pValue->ViewVersion);
    }
}

/*============================================================================
 * OpcUa_ViewDescription_Clear
 *===========================================================================*/
void OpcUa_ViewDescription_Clear(OpcUa_ViewDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->ViewId);
        DateTime_Clear(&a_pValue->Timestamp);
        UInt32_Clear(&a_pValue->ViewVersion);
    }
}

/*============================================================================
 * OpcUa_ViewDescription_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ViewDescription_Encode(OpcUa_ViewDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ViewDescription_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ViewDescription_Decode(OpcUa_ViewDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ViewDescription_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ViewId, msgBuf);
    status &= DateTime_Read(&a_pValue->Timestamp, msgBuf);
    status &= UInt32_Read(&a_pValue->ViewVersion, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ViewDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ViewDescription_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ViewDescription_EncodeableType =
{
    "ViewDescription",
    OpcUaId_ViewDescription,
    OpcUaId_ViewDescription_Encoding_DefaultBinary,
    OpcUaId_ViewDescription_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ViewDescription),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ViewDescription_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ViewDescription_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ViewDescription_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ViewDescription_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ViewDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowseDescription
/*============================================================================
 * OpcUa_BrowseDescription_Initialize
 *===========================================================================*/
void OpcUa_BrowseDescription_Initialize(OpcUa_BrowseDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->BrowseDirection);
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IncludeSubtypes);
        UInt32_Initialize(&a_pValue->NodeClassMask);
        UInt32_Initialize(&a_pValue->ResultMask);
    }
}

/*============================================================================
 * OpcUa_BrowseDescription_Clear
 *===========================================================================*/
void OpcUa_BrowseDescription_Clear(OpcUa_BrowseDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->BrowseDirection);
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IncludeSubtypes);
        UInt32_Clear(&a_pValue->NodeClassMask);
        UInt32_Clear(&a_pValue->ResultMask);
    }
}

/*============================================================================
 * OpcUa_BrowseDescription_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseDescription_Encode(OpcUa_BrowseDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->BrowseDirection);
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
 * OpcUa_BrowseDescription_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseDescription_Decode(OpcUa_BrowseDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BrowseDescription_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->BrowseDirection);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IncludeSubtypes, msgBuf);
    status &= UInt32_Read(&a_pValue->NodeClassMask, msgBuf);
    status &= UInt32_Read(&a_pValue->ResultMask, msgBuf);

    if(status != STATUS_OK){
        OpcUa_BrowseDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseDescription_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BrowseDescription_EncodeableType =
{
    "BrowseDescription",
    OpcUaId_BrowseDescription,
    OpcUaId_BrowseDescription_Encoding_DefaultBinary,
    OpcUaId_BrowseDescription_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BrowseDescription),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BrowseDescription_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BrowseDescription_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BrowseDescription_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BrowseDescription_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BrowseDescription_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_ReferenceDescription
/*============================================================================
 * OpcUa_ReferenceDescription_Initialize
 *===========================================================================*/
void OpcUa_ReferenceDescription_Initialize(OpcUa_ReferenceDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsForward);
        ExpandedNodeId_Initialize(&a_pValue->NodeId);
        QualifiedName_Initialize(&a_pValue->BrowseName);
        LocalizedText_Initialize(&a_pValue->DisplayName);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        ExpandedNodeId_Initialize(&a_pValue->TypeDefinition);
    }
}

/*============================================================================
 * OpcUa_ReferenceDescription_Clear
 *===========================================================================*/
void OpcUa_ReferenceDescription_Clear(OpcUa_ReferenceDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsForward);
        ExpandedNodeId_Clear(&a_pValue->NodeId);
        QualifiedName_Clear(&a_pValue->BrowseName);
        LocalizedText_Clear(&a_pValue->DisplayName);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->NodeClass);
        ExpandedNodeId_Clear(&a_pValue->TypeDefinition);
    }
}

/*============================================================================
 * OpcUa_ReferenceDescription_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReferenceDescription_Encode(OpcUa_ReferenceDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsForward, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= QualifiedName_Write(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Write(&a_pValue->DisplayName, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= ExpandedNodeId_Write(&a_pValue->TypeDefinition, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ReferenceDescription_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReferenceDescription_Decode(OpcUa_ReferenceDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReferenceDescription_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsForward, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= QualifiedName_Read(&a_pValue->BrowseName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->NodeClass);
    status &= ExpandedNodeId_Read(&a_pValue->TypeDefinition, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ReferenceDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReferenceDescription_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReferenceDescription_EncodeableType =
{
    "ReferenceDescription",
    OpcUaId_ReferenceDescription,
    OpcUaId_ReferenceDescription_Encoding_DefaultBinary,
    OpcUaId_ReferenceDescription_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReferenceDescription),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReferenceDescription_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReferenceDescription_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReferenceDescription_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReferenceDescription_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReferenceDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowseResult
/*============================================================================
 * OpcUa_BrowseResult_Initialize
 *===========================================================================*/
void OpcUa_BrowseResult_Initialize(OpcUa_BrowseResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        ByteString_Initialize(&a_pValue->ContinuationPoint);
        SOPC_Initialize_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                            sizeof(OpcUa_ReferenceDescription), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReferenceDescription_Initialize);
    }
}

/*============================================================================
 * OpcUa_BrowseResult_Clear
 *===========================================================================*/
void OpcUa_BrowseResult_Clear(OpcUa_BrowseResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        ByteString_Clear(&a_pValue->ContinuationPoint);
        SOPC_Clear_Array(&a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                       sizeof(OpcUa_ReferenceDescription), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReferenceDescription_Clear);
    }
}

/*============================================================================
 * OpcUa_BrowseResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseResult_Encode(OpcUa_BrowseResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= ByteString_Write(&a_pValue->ContinuationPoint, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                   sizeof(OpcUa_ReferenceDescription), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReferenceDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseResult_Decode(OpcUa_BrowseResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BrowseResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferences, (void**) &a_pValue->References, 
                  sizeof(OpcUa_ReferenceDescription), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReferenceDescription_Decode);

    if(status != STATUS_OK){
        OpcUa_BrowseResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BrowseResult_EncodeableType =
{
    "BrowseResult",
    OpcUaId_BrowseResult,
    OpcUaId_BrowseResult_Encoding_DefaultBinary,
    OpcUaId_BrowseResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BrowseResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BrowseResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BrowseResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BrowseResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BrowseResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BrowseResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Browse
#ifndef OPCUA_EXCLUDE_BrowseRequest
/*============================================================================
 * OpcUa_BrowseRequest_Initialize
 *===========================================================================*/
void OpcUa_BrowseRequest_Initialize(OpcUa_BrowseRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        OpcUa_ViewDescription_Initialize(&a_pValue->View);
        UInt32_Initialize(&a_pValue->RequestedMaxReferencesPerNode);
        SOPC_Initialize_Array(&a_pValue->NoOfNodesToBrowse, (void**) &a_pValue->NodesToBrowse, 
                            sizeof(OpcUa_BrowseDescription), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_BrowseDescription_Initialize);
    }
}

/*============================================================================
 * OpcUa_BrowseRequest_Clear
 *===========================================================================*/
void OpcUa_BrowseRequest_Clear(OpcUa_BrowseRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        OpcUa_ViewDescription_Clear(&a_pValue->View);
        UInt32_Clear(&a_pValue->RequestedMaxReferencesPerNode);
        SOPC_Clear_Array(&a_pValue->NoOfNodesToBrowse, (void**) &a_pValue->NodesToBrowse, 
                       sizeof(OpcUa_BrowseDescription), (SOPC_EncodeableObject_PfnClear*) OpcUa_BrowseDescription_Clear);
    }
}

/*============================================================================
 * OpcUa_BrowseRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseRequest_Encode(OpcUa_BrowseRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_ViewDescription_Encode(&a_pValue->View, msgBuf);
    status &= UInt32_Write(&a_pValue->RequestedMaxReferencesPerNode, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNodesToBrowse, (void**) &a_pValue->NodesToBrowse, 
                   sizeof(OpcUa_BrowseDescription), (SOPC_EncodeableObject_PfnEncode*) OpcUa_BrowseDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseRequest_Decode(OpcUa_BrowseRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BrowseRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_ViewDescription_Decode(&a_pValue->View, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedMaxReferencesPerNode, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNodesToBrowse, (void**) &a_pValue->NodesToBrowse, 
                  sizeof(OpcUa_BrowseDescription), (SOPC_EncodeableObject_PfnDecode*) OpcUa_BrowseDescription_Decode);

    if(status != STATUS_OK){
        OpcUa_BrowseRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BrowseRequest_EncodeableType =
{
    "BrowseRequest",
    OpcUaId_BrowseRequest,
    OpcUaId_BrowseRequest_Encoding_DefaultBinary,
    OpcUaId_BrowseRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BrowseRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BrowseRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BrowseRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BrowseRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BrowseRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BrowseRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowseResponse
/*============================================================================
 * OpcUa_BrowseResponse_Initialize
 *===========================================================================*/
void OpcUa_BrowseResponse_Initialize(OpcUa_BrowseResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_BrowseResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_BrowseResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_BrowseResponse_Clear
 *===========================================================================*/
void OpcUa_BrowseResponse_Clear(OpcUa_BrowseResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_BrowseResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_BrowseResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_BrowseResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseResponse_Encode(OpcUa_BrowseResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_BrowseResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_BrowseResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseResponse_Decode(OpcUa_BrowseResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BrowseResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_BrowseResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_BrowseResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_BrowseResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BrowseResponse_EncodeableType =
{
    "BrowseResponse",
    OpcUaId_BrowseResponse,
    OpcUaId_BrowseResponse_Encoding_DefaultBinary,
    OpcUaId_BrowseResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BrowseResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BrowseResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BrowseResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BrowseResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BrowseResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BrowseResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_BrowseNext
#ifndef OPCUA_EXCLUDE_BrowseNextRequest
/*============================================================================
 * OpcUa_BrowseNextRequest_Initialize
 *===========================================================================*/
void OpcUa_BrowseNextRequest_Initialize(OpcUa_BrowseNextRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Boolean_Initialize(&a_pValue->ReleaseContinuationPoints);
        SOPC_Initialize_Array(&a_pValue->NoOfContinuationPoints, (void**) &a_pValue->ContinuationPoints, 
                            sizeof(SOPC_ByteString), (SOPC_EncodeableObject_PfnInitialize*) ByteString_Initialize);
    }
}

/*============================================================================
 * OpcUa_BrowseNextRequest_Clear
 *===========================================================================*/
void OpcUa_BrowseNextRequest_Clear(OpcUa_BrowseNextRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        Boolean_Clear(&a_pValue->ReleaseContinuationPoints);
        SOPC_Clear_Array(&a_pValue->NoOfContinuationPoints, (void**) &a_pValue->ContinuationPoints, 
                       sizeof(SOPC_ByteString), (SOPC_EncodeableObject_PfnClear*) ByteString_Clear);
    }
}

/*============================================================================
 * OpcUa_BrowseNextRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseNextRequest_Encode(OpcUa_BrowseNextRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Write(&a_pValue->ReleaseContinuationPoints, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfContinuationPoints, (void**) &a_pValue->ContinuationPoints, 
                   sizeof(SOPC_ByteString), (SOPC_EncodeableObject_PfnEncode*) ByteString_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseNextRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseNextRequest_Decode(OpcUa_BrowseNextRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BrowseNextRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Read(&a_pValue->ReleaseContinuationPoints, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfContinuationPoints, (void**) &a_pValue->ContinuationPoints, 
                  sizeof(SOPC_ByteString), (SOPC_EncodeableObject_PfnDecode*) ByteString_Read);

    if(status != STATUS_OK){
        OpcUa_BrowseNextRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseNextRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BrowseNextRequest_EncodeableType =
{
    "BrowseNextRequest",
    OpcUaId_BrowseNextRequest,
    OpcUaId_BrowseNextRequest_Encoding_DefaultBinary,
    OpcUaId_BrowseNextRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BrowseNextRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BrowseNextRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BrowseNextRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BrowseNextRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BrowseNextRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BrowseNextRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowseNextResponse
/*============================================================================
 * OpcUa_BrowseNextResponse_Initialize
 *===========================================================================*/
void OpcUa_BrowseNextResponse_Initialize(OpcUa_BrowseNextResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_BrowseResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_BrowseResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_BrowseNextResponse_Clear
 *===========================================================================*/
void OpcUa_BrowseNextResponse_Clear(OpcUa_BrowseNextResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_BrowseResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_BrowseResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_BrowseNextResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseNextResponse_Encode(OpcUa_BrowseNextResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_BrowseResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_BrowseResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseNextResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowseNextResponse_Decode(OpcUa_BrowseNextResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BrowseNextResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_BrowseResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_BrowseResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_BrowseNextResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowseNextResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BrowseNextResponse_EncodeableType =
{
    "BrowseNextResponse",
    OpcUaId_BrowseNextResponse,
    OpcUaId_BrowseNextResponse_Encoding_DefaultBinary,
    OpcUaId_BrowseNextResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BrowseNextResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BrowseNextResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BrowseNextResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BrowseNextResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BrowseNextResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BrowseNextResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_RelativePathElement
/*============================================================================
 * OpcUa_RelativePathElement_Initialize
 *===========================================================================*/
void OpcUa_RelativePathElement_Initialize(OpcUa_RelativePathElement* a_pValue)
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
 * OpcUa_RelativePathElement_Clear
 *===========================================================================*/
void OpcUa_RelativePathElement_Clear(OpcUa_RelativePathElement* a_pValue)
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
 * OpcUa_RelativePathElement_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RelativePathElement_Encode(OpcUa_RelativePathElement* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_RelativePathElement_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RelativePathElement_Decode(OpcUa_RelativePathElement* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RelativePathElement_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsInverse, msgBuf);
    status &= Boolean_Read(&a_pValue->IncludeSubtypes, msgBuf);
    status &= QualifiedName_Read(&a_pValue->TargetName, msgBuf);

    if(status != STATUS_OK){
        OpcUa_RelativePathElement_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RelativePathElement_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RelativePathElement_EncodeableType =
{
    "RelativePathElement",
    OpcUaId_RelativePathElement,
    OpcUaId_RelativePathElement_Encoding_DefaultBinary,
    OpcUaId_RelativePathElement_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RelativePathElement),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RelativePathElement_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RelativePathElement_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RelativePathElement_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RelativePathElement_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RelativePathElement_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RelativePath
/*============================================================================
 * OpcUa_RelativePath_Initialize
 *===========================================================================*/
void OpcUa_RelativePath_Initialize(OpcUa_RelativePath* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                            sizeof(OpcUa_RelativePathElement), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_RelativePathElement_Initialize);
    }
}

/*============================================================================
 * OpcUa_RelativePath_Clear
 *===========================================================================*/
void OpcUa_RelativePath_Clear(OpcUa_RelativePath* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                       sizeof(OpcUa_RelativePathElement), (SOPC_EncodeableObject_PfnClear*) OpcUa_RelativePathElement_Clear);
    }
}

/*============================================================================
 * OpcUa_RelativePath_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RelativePath_Encode(OpcUa_RelativePath* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                   sizeof(OpcUa_RelativePathElement), (SOPC_EncodeableObject_PfnEncode*) OpcUa_RelativePathElement_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RelativePath_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RelativePath_Decode(OpcUa_RelativePath* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RelativePath_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                  sizeof(OpcUa_RelativePathElement), (SOPC_EncodeableObject_PfnDecode*) OpcUa_RelativePathElement_Decode);

    if(status != STATUS_OK){
        OpcUa_RelativePath_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RelativePath_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RelativePath_EncodeableType =
{
    "RelativePath",
    OpcUaId_RelativePath,
    OpcUaId_RelativePath_Encoding_DefaultBinary,
    OpcUaId_RelativePath_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RelativePath),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RelativePath_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RelativePath_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RelativePath_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RelativePath_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RelativePath_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowsePath
/*============================================================================
 * OpcUa_BrowsePath_Initialize
 *===========================================================================*/
void OpcUa_BrowsePath_Initialize(OpcUa_BrowsePath* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->StartingNode);
        OpcUa_RelativePath_Initialize(&a_pValue->RelativePath);
    }
}

/*============================================================================
 * OpcUa_BrowsePath_Clear
 *===========================================================================*/
void OpcUa_BrowsePath_Clear(OpcUa_BrowsePath* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->StartingNode);
        OpcUa_RelativePath_Clear(&a_pValue->RelativePath);
    }
}

/*============================================================================
 * OpcUa_BrowsePath_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowsePath_Encode(OpcUa_BrowsePath* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->StartingNode, msgBuf);
    status &= OpcUa_RelativePath_Encode(&a_pValue->RelativePath, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowsePath_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowsePath_Decode(OpcUa_BrowsePath* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BrowsePath_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->StartingNode, msgBuf);
    status &= OpcUa_RelativePath_Decode(&a_pValue->RelativePath, msgBuf);

    if(status != STATUS_OK){
        OpcUa_BrowsePath_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowsePath_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BrowsePath_EncodeableType =
{
    "BrowsePath",
    OpcUaId_BrowsePath,
    OpcUaId_BrowsePath_Encoding_DefaultBinary,
    OpcUaId_BrowsePath_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BrowsePath),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BrowsePath_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BrowsePath_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BrowsePath_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BrowsePath_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BrowsePath_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathTarget
/*============================================================================
 * OpcUa_BrowsePathTarget_Initialize
 *===========================================================================*/
void OpcUa_BrowsePathTarget_Initialize(OpcUa_BrowsePathTarget* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Initialize(&a_pValue->TargetId);
        UInt32_Initialize(&a_pValue->RemainingPathIndex);
    }
}

/*============================================================================
 * OpcUa_BrowsePathTarget_Clear
 *===========================================================================*/
void OpcUa_BrowsePathTarget_Clear(OpcUa_BrowsePathTarget* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Clear(&a_pValue->TargetId);
        UInt32_Clear(&a_pValue->RemainingPathIndex);
    }
}

/*============================================================================
 * OpcUa_BrowsePathTarget_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowsePathTarget_Encode(OpcUa_BrowsePathTarget* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_BrowsePathTarget_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowsePathTarget_Decode(OpcUa_BrowsePathTarget* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BrowsePathTarget_Initialize(a_pValue);

    status &= ExpandedNodeId_Read(&a_pValue->TargetId, msgBuf);
    status &= UInt32_Read(&a_pValue->RemainingPathIndex, msgBuf);

    if(status != STATUS_OK){
        OpcUa_BrowsePathTarget_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowsePathTarget_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BrowsePathTarget_EncodeableType =
{
    "BrowsePathTarget",
    OpcUaId_BrowsePathTarget,
    OpcUaId_BrowsePathTarget_Encoding_DefaultBinary,
    OpcUaId_BrowsePathTarget_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BrowsePathTarget),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BrowsePathTarget_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BrowsePathTarget_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BrowsePathTarget_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BrowsePathTarget_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BrowsePathTarget_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_BrowsePathResult
/*============================================================================
 * OpcUa_BrowsePathResult_Initialize
 *===========================================================================*/
void OpcUa_BrowsePathResult_Initialize(OpcUa_BrowsePathResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        SOPC_Initialize_Array(&a_pValue->NoOfTargets, (void**) &a_pValue->Targets, 
                            sizeof(OpcUa_BrowsePathTarget), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_BrowsePathTarget_Initialize);
    }
}

/*============================================================================
 * OpcUa_BrowsePathResult_Clear
 *===========================================================================*/
void OpcUa_BrowsePathResult_Clear(OpcUa_BrowsePathResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        SOPC_Clear_Array(&a_pValue->NoOfTargets, (void**) &a_pValue->Targets, 
                       sizeof(OpcUa_BrowsePathTarget), (SOPC_EncodeableObject_PfnClear*) OpcUa_BrowsePathTarget_Clear);
    }
}

/*============================================================================
 * OpcUa_BrowsePathResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowsePathResult_Encode(OpcUa_BrowsePathResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfTargets, (void**) &a_pValue->Targets, 
                   sizeof(OpcUa_BrowsePathTarget), (SOPC_EncodeableObject_PfnEncode*) OpcUa_BrowsePathTarget_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowsePathResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BrowsePathResult_Decode(OpcUa_BrowsePathResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BrowsePathResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfTargets, (void**) &a_pValue->Targets, 
                  sizeof(OpcUa_BrowsePathTarget), (SOPC_EncodeableObject_PfnDecode*) OpcUa_BrowsePathTarget_Decode);

    if(status != STATUS_OK){
        OpcUa_BrowsePathResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BrowsePathResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BrowsePathResult_EncodeableType =
{
    "BrowsePathResult",
    OpcUaId_BrowsePathResult,
    OpcUaId_BrowsePathResult_Encoding_DefaultBinary,
    OpcUaId_BrowsePathResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BrowsePathResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BrowsePathResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BrowsePathResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BrowsePathResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BrowsePathResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BrowsePathResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIdsRequest
/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsRequest_Initialize
 *===========================================================================*/
void OpcUa_TranslateBrowsePathsToNodeIdsRequest_Initialize(OpcUa_TranslateBrowsePathsToNodeIdsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfBrowsePaths, (void**) &a_pValue->BrowsePaths, 
                            sizeof(OpcUa_BrowsePath), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_BrowsePath_Initialize);
    }
}

/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsRequest_Clear
 *===========================================================================*/
void OpcUa_TranslateBrowsePathsToNodeIdsRequest_Clear(OpcUa_TranslateBrowsePathsToNodeIdsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfBrowsePaths, (void**) &a_pValue->BrowsePaths, 
                       sizeof(OpcUa_BrowsePath), (SOPC_EncodeableObject_PfnClear*) OpcUa_BrowsePath_Clear);
    }
}

/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsRequest_Encode(OpcUa_TranslateBrowsePathsToNodeIdsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfBrowsePaths, (void**) &a_pValue->BrowsePaths, 
                   sizeof(OpcUa_BrowsePath), (SOPC_EncodeableObject_PfnEncode*) OpcUa_BrowsePath_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsRequest_Decode(OpcUa_TranslateBrowsePathsToNodeIdsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_TranslateBrowsePathsToNodeIdsRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfBrowsePaths, (void**) &a_pValue->BrowsePaths, 
                  sizeof(OpcUa_BrowsePath), (SOPC_EncodeableObject_PfnDecode*) OpcUa_BrowsePath_Decode);

    if(status != STATUS_OK){
        OpcUa_TranslateBrowsePathsToNodeIdsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType =
{
    "TranslateBrowsePathsToNodeIdsRequest",
    OpcUaId_TranslateBrowsePathsToNodeIdsRequest,
    OpcUaId_TranslateBrowsePathsToNodeIdsRequest_Encoding_DefaultBinary,
    OpcUaId_TranslateBrowsePathsToNodeIdsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_TranslateBrowsePathsToNodeIdsRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_TranslateBrowsePathsToNodeIdsRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_TranslateBrowsePathsToNodeIdsRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_TranslateBrowsePathsToNodeIdsRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_TranslateBrowsePathsToNodeIdsRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_TranslateBrowsePathsToNodeIdsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIdsResponse
/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsResponse_Initialize
 *===========================================================================*/
void OpcUa_TranslateBrowsePathsToNodeIdsResponse_Initialize(OpcUa_TranslateBrowsePathsToNodeIdsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_BrowsePathResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_BrowsePathResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsResponse_Clear
 *===========================================================================*/
void OpcUa_TranslateBrowsePathsToNodeIdsResponse_Clear(OpcUa_TranslateBrowsePathsToNodeIdsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_BrowsePathResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_BrowsePathResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsResponse_Encode(OpcUa_TranslateBrowsePathsToNodeIdsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_BrowsePathResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_BrowsePathResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TranslateBrowsePathsToNodeIdsResponse_Decode(OpcUa_TranslateBrowsePathsToNodeIdsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_TranslateBrowsePathsToNodeIdsResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_BrowsePathResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_BrowsePathResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_TranslateBrowsePathsToNodeIdsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType =
{
    "TranslateBrowsePathsToNodeIdsResponse",
    OpcUaId_TranslateBrowsePathsToNodeIdsResponse,
    OpcUaId_TranslateBrowsePathsToNodeIdsResponse_Encoding_DefaultBinary,
    OpcUaId_TranslateBrowsePathsToNodeIdsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_TranslateBrowsePathsToNodeIdsResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_TranslateBrowsePathsToNodeIdsResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_TranslateBrowsePathsToNodeIdsResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_TranslateBrowsePathsToNodeIdsResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_TranslateBrowsePathsToNodeIdsResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_TranslateBrowsePathsToNodeIdsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodes
#ifndef OPCUA_EXCLUDE_RegisterNodesRequest
/*============================================================================
 * OpcUa_RegisterNodesRequest_Initialize
 *===========================================================================*/
void OpcUa_RegisterNodesRequest_Initialize(OpcUa_RegisterNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfNodesToRegister, (void**) &a_pValue->NodesToRegister, 
                            sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnInitialize*) NodeId_Initialize);
    }
}

/*============================================================================
 * OpcUa_RegisterNodesRequest_Clear
 *===========================================================================*/
void OpcUa_RegisterNodesRequest_Clear(OpcUa_RegisterNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfNodesToRegister, (void**) &a_pValue->NodesToRegister, 
                       sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnClear*) NodeId_Clear);
    }
}

/*============================================================================
 * OpcUa_RegisterNodesRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterNodesRequest_Encode(OpcUa_RegisterNodesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNodesToRegister, (void**) &a_pValue->NodesToRegister, 
                   sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnEncode*) NodeId_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterNodesRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterNodesRequest_Decode(OpcUa_RegisterNodesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RegisterNodesRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNodesToRegister, (void**) &a_pValue->NodesToRegister, 
                  sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnDecode*) NodeId_Read);

    if(status != STATUS_OK){
        OpcUa_RegisterNodesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterNodesRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RegisterNodesRequest_EncodeableType =
{
    "RegisterNodesRequest",
    OpcUaId_RegisterNodesRequest,
    OpcUaId_RegisterNodesRequest_Encoding_DefaultBinary,
    OpcUaId_RegisterNodesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RegisterNodesRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RegisterNodesRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RegisterNodesRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RegisterNodesRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RegisterNodesRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RegisterNodesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RegisterNodesResponse
/*============================================================================
 * OpcUa_RegisterNodesResponse_Initialize
 *===========================================================================*/
void OpcUa_RegisterNodesResponse_Initialize(OpcUa_RegisterNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfRegisteredNodeIds, (void**) &a_pValue->RegisteredNodeIds, 
                            sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnInitialize*) NodeId_Initialize);
    }
}

/*============================================================================
 * OpcUa_RegisterNodesResponse_Clear
 *===========================================================================*/
void OpcUa_RegisterNodesResponse_Clear(OpcUa_RegisterNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfRegisteredNodeIds, (void**) &a_pValue->RegisteredNodeIds, 
                       sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnClear*) NodeId_Clear);
    }
}

/*============================================================================
 * OpcUa_RegisterNodesResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterNodesResponse_Encode(OpcUa_RegisterNodesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfRegisteredNodeIds, (void**) &a_pValue->RegisteredNodeIds, 
                   sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnEncode*) NodeId_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterNodesResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RegisterNodesResponse_Decode(OpcUa_RegisterNodesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RegisterNodesResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfRegisteredNodeIds, (void**) &a_pValue->RegisteredNodeIds, 
                  sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnDecode*) NodeId_Read);

    if(status != STATUS_OK){
        OpcUa_RegisterNodesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RegisterNodesResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RegisterNodesResponse_EncodeableType =
{
    "RegisterNodesResponse",
    OpcUaId_RegisterNodesResponse,
    OpcUaId_RegisterNodesResponse_Encoding_DefaultBinary,
    OpcUaId_RegisterNodesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RegisterNodesResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RegisterNodesResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RegisterNodesResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RegisterNodesResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RegisterNodesResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RegisterNodesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodes
#ifndef OPCUA_EXCLUDE_UnregisterNodesRequest
/*============================================================================
 * OpcUa_UnregisterNodesRequest_Initialize
 *===========================================================================*/
void OpcUa_UnregisterNodesRequest_Initialize(OpcUa_UnregisterNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfNodesToUnregister, (void**) &a_pValue->NodesToUnregister, 
                            sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnInitialize*) NodeId_Initialize);
    }
}

/*============================================================================
 * OpcUa_UnregisterNodesRequest_Clear
 *===========================================================================*/
void OpcUa_UnregisterNodesRequest_Clear(OpcUa_UnregisterNodesRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfNodesToUnregister, (void**) &a_pValue->NodesToUnregister, 
                       sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnClear*) NodeId_Clear);
    }
}

/*============================================================================
 * OpcUa_UnregisterNodesRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UnregisterNodesRequest_Encode(OpcUa_UnregisterNodesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNodesToUnregister, (void**) &a_pValue->NodesToUnregister, 
                   sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnEncode*) NodeId_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_UnregisterNodesRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UnregisterNodesRequest_Decode(OpcUa_UnregisterNodesRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_UnregisterNodesRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNodesToUnregister, (void**) &a_pValue->NodesToUnregister, 
                  sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnDecode*) NodeId_Read);

    if(status != STATUS_OK){
        OpcUa_UnregisterNodesRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_UnregisterNodesRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_UnregisterNodesRequest_EncodeableType =
{
    "UnregisterNodesRequest",
    OpcUaId_UnregisterNodesRequest,
    OpcUaId_UnregisterNodesRequest_Encoding_DefaultBinary,
    OpcUaId_UnregisterNodesRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_UnregisterNodesRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_UnregisterNodesRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_UnregisterNodesRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_UnregisterNodesRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_UnregisterNodesRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_UnregisterNodesRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_UnregisterNodesResponse
/*============================================================================
 * OpcUa_UnregisterNodesResponse_Initialize
 *===========================================================================*/
void OpcUa_UnregisterNodesResponse_Initialize(OpcUa_UnregisterNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_UnregisterNodesResponse_Clear
 *===========================================================================*/
void OpcUa_UnregisterNodesResponse_Clear(OpcUa_UnregisterNodesResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
    }
}

/*============================================================================
 * OpcUa_UnregisterNodesResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UnregisterNodesResponse_Encode(OpcUa_UnregisterNodesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_UnregisterNodesResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UnregisterNodesResponse_Decode(OpcUa_UnregisterNodesResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_UnregisterNodesResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);

    if(status != STATUS_OK){
        OpcUa_UnregisterNodesResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_UnregisterNodesResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_UnregisterNodesResponse_EncodeableType =
{
    "UnregisterNodesResponse",
    OpcUaId_UnregisterNodesResponse,
    OpcUaId_UnregisterNodesResponse_Encoding_DefaultBinary,
    OpcUaId_UnregisterNodesResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_UnregisterNodesResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_UnregisterNodesResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_UnregisterNodesResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_UnregisterNodesResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_UnregisterNodesResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_UnregisterNodesResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_EndpointConfiguration
/*============================================================================
 * OpcUa_EndpointConfiguration_Initialize
 *===========================================================================*/
void OpcUa_EndpointConfiguration_Initialize(OpcUa_EndpointConfiguration* a_pValue)
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
 * OpcUa_EndpointConfiguration_Clear
 *===========================================================================*/
void OpcUa_EndpointConfiguration_Clear(OpcUa_EndpointConfiguration* a_pValue)
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
 * OpcUa_EndpointConfiguration_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EndpointConfiguration_Encode(OpcUa_EndpointConfiguration* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_EndpointConfiguration_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EndpointConfiguration_Decode(OpcUa_EndpointConfiguration* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EndpointConfiguration_Initialize(a_pValue);

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
        OpcUa_EndpointConfiguration_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EndpointConfiguration_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EndpointConfiguration_EncodeableType =
{
    "EndpointConfiguration",
    OpcUaId_EndpointConfiguration,
    OpcUaId_EndpointConfiguration_Encoding_DefaultBinary,
    OpcUaId_EndpointConfiguration_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EndpointConfiguration),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EndpointConfiguration_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EndpointConfiguration_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EndpointConfiguration_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EndpointConfiguration_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EndpointConfiguration_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_SupportedProfile
/*============================================================================
 * OpcUa_SupportedProfile_Initialize
 *===========================================================================*/
void OpcUa_SupportedProfile_Initialize(OpcUa_SupportedProfile* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->OrganizationUri);
        String_Initialize(&a_pValue->ProfileId);
        String_Initialize(&a_pValue->ComplianceTool);
        DateTime_Initialize(&a_pValue->ComplianceDate);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->ComplianceLevel);
        SOPC_Initialize_Array(&a_pValue->NoOfUnsupportedUnitIds, (void**) &a_pValue->UnsupportedUnitIds, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * OpcUa_SupportedProfile_Clear
 *===========================================================================*/
void OpcUa_SupportedProfile_Clear(OpcUa_SupportedProfile* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->OrganizationUri);
        String_Clear(&a_pValue->ProfileId);
        String_Clear(&a_pValue->ComplianceTool);
        DateTime_Clear(&a_pValue->ComplianceDate);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->ComplianceLevel);
        SOPC_Clear_Array(&a_pValue->NoOfUnsupportedUnitIds, (void**) &a_pValue->UnsupportedUnitIds, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * OpcUa_SupportedProfile_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SupportedProfile_Encode(OpcUa_SupportedProfile* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->OrganizationUri, msgBuf);
    status &= String_Write(&a_pValue->ProfileId, msgBuf);
    status &= String_Write(&a_pValue->ComplianceTool, msgBuf);
    status &= DateTime_Write(&a_pValue->ComplianceDate, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ComplianceLevel);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfUnsupportedUnitIds, (void**) &a_pValue->UnsupportedUnitIds, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SupportedProfile_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SupportedProfile_Decode(OpcUa_SupportedProfile* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SupportedProfile_Initialize(a_pValue);

    status &= String_Read(&a_pValue->OrganizationUri, msgBuf);
    status &= String_Read(&a_pValue->ProfileId, msgBuf);
    status &= String_Read(&a_pValue->ComplianceTool, msgBuf);
    status &= DateTime_Read(&a_pValue->ComplianceDate, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ComplianceLevel);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfUnsupportedUnitIds, (void**) &a_pValue->UnsupportedUnitIds, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        OpcUa_SupportedProfile_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SupportedProfile_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SupportedProfile_EncodeableType =
{
    "SupportedProfile",
    OpcUaId_SupportedProfile,
    OpcUaId_SupportedProfile_Encoding_DefaultBinary,
    OpcUaId_SupportedProfile_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SupportedProfile),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SupportedProfile_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SupportedProfile_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SupportedProfile_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SupportedProfile_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SupportedProfile_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SoftwareCertificate
/*============================================================================
 * OpcUa_SoftwareCertificate_Initialize
 *===========================================================================*/
void OpcUa_SoftwareCertificate_Initialize(OpcUa_SoftwareCertificate* a_pValue)
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
        SOPC_Initialize_Array(&a_pValue->NoOfSupportedProfiles, (void**) &a_pValue->SupportedProfiles, 
                            sizeof(OpcUa_SupportedProfile), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_SupportedProfile_Initialize);
    }
}

/*============================================================================
 * OpcUa_SoftwareCertificate_Clear
 *===========================================================================*/
void OpcUa_SoftwareCertificate_Clear(OpcUa_SoftwareCertificate* a_pValue)
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
        SOPC_Clear_Array(&a_pValue->NoOfSupportedProfiles, (void**) &a_pValue->SupportedProfiles, 
                       sizeof(OpcUa_SupportedProfile), (SOPC_EncodeableObject_PfnClear*) OpcUa_SupportedProfile_Clear);
    }
}

/*============================================================================
 * OpcUa_SoftwareCertificate_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SoftwareCertificate_Encode(OpcUa_SoftwareCertificate* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfSupportedProfiles, (void**) &a_pValue->SupportedProfiles, 
                   sizeof(OpcUa_SupportedProfile), (SOPC_EncodeableObject_PfnEncode*) OpcUa_SupportedProfile_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SoftwareCertificate_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SoftwareCertificate_Decode(OpcUa_SoftwareCertificate* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SoftwareCertificate_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ProductName, msgBuf);
    status &= String_Read(&a_pValue->ProductUri, msgBuf);
    status &= String_Read(&a_pValue->VendorName, msgBuf);
    status &= ByteString_Read(&a_pValue->VendorProductCertificate, msgBuf);
    status &= String_Read(&a_pValue->SoftwareVersion, msgBuf);
    status &= String_Read(&a_pValue->BuildNumber, msgBuf);
    status &= DateTime_Read(&a_pValue->BuildDate, msgBuf);
    status &= String_Read(&a_pValue->IssuedBy, msgBuf);
    status &= DateTime_Read(&a_pValue->IssueDate, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfSupportedProfiles, (void**) &a_pValue->SupportedProfiles, 
                  sizeof(OpcUa_SupportedProfile), (SOPC_EncodeableObject_PfnDecode*) OpcUa_SupportedProfile_Decode);

    if(status != STATUS_OK){
        OpcUa_SoftwareCertificate_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SoftwareCertificate_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SoftwareCertificate_EncodeableType =
{
    "SoftwareCertificate",
    OpcUaId_SoftwareCertificate,
    OpcUaId_SoftwareCertificate_Encoding_DefaultBinary,
    OpcUaId_SoftwareCertificate_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SoftwareCertificate),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SoftwareCertificate_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SoftwareCertificate_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SoftwareCertificate_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SoftwareCertificate_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SoftwareCertificate_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_QueryDataDescription
/*============================================================================
 * OpcUa_QueryDataDescription_Initialize
 *===========================================================================*/
void OpcUa_QueryDataDescription_Initialize(OpcUa_QueryDataDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RelativePath_Initialize(&a_pValue->RelativePath);
        UInt32_Initialize(&a_pValue->AttributeId);
        String_Initialize(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * OpcUa_QueryDataDescription_Clear
 *===========================================================================*/
void OpcUa_QueryDataDescription_Clear(OpcUa_QueryDataDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RelativePath_Clear(&a_pValue->RelativePath);
        UInt32_Clear(&a_pValue->AttributeId);
        String_Clear(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * OpcUa_QueryDataDescription_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryDataDescription_Encode(OpcUa_QueryDataDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RelativePath_Encode(&a_pValue->RelativePath, msgBuf);
    status &= UInt32_Write(&a_pValue->AttributeId, msgBuf);
    status &= String_Write(&a_pValue->IndexRange, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryDataDescription_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryDataDescription_Decode(OpcUa_QueryDataDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_QueryDataDescription_Initialize(a_pValue);

    status &= OpcUa_RelativePath_Decode(&a_pValue->RelativePath, msgBuf);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);

    if(status != STATUS_OK){
        OpcUa_QueryDataDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryDataDescription_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_QueryDataDescription_EncodeableType =
{
    "QueryDataDescription",
    OpcUaId_QueryDataDescription,
    OpcUaId_QueryDataDescription_Encoding_DefaultBinary,
    OpcUaId_QueryDataDescription_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_QueryDataDescription),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_QueryDataDescription_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_QueryDataDescription_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_QueryDataDescription_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_QueryDataDescription_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_QueryDataDescription_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_NodeTypeDescription
/*============================================================================
 * OpcUa_NodeTypeDescription_Initialize
 *===========================================================================*/
void OpcUa_NodeTypeDescription_Initialize(OpcUa_NodeTypeDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Initialize(&a_pValue->TypeDefinitionNode);
        Boolean_Initialize(&a_pValue->IncludeSubTypes);
        SOPC_Initialize_Array(&a_pValue->NoOfDataToReturn, (void**) &a_pValue->DataToReturn, 
                            sizeof(OpcUa_QueryDataDescription), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_QueryDataDescription_Initialize);
    }
}

/*============================================================================
 * OpcUa_NodeTypeDescription_Clear
 *===========================================================================*/
void OpcUa_NodeTypeDescription_Clear(OpcUa_NodeTypeDescription* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Clear(&a_pValue->TypeDefinitionNode);
        Boolean_Clear(&a_pValue->IncludeSubTypes);
        SOPC_Clear_Array(&a_pValue->NoOfDataToReturn, (void**) &a_pValue->DataToReturn, 
                       sizeof(OpcUa_QueryDataDescription), (SOPC_EncodeableObject_PfnClear*) OpcUa_QueryDataDescription_Clear);
    }
}

/*============================================================================
 * OpcUa_NodeTypeDescription_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NodeTypeDescription_Encode(OpcUa_NodeTypeDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= ExpandedNodeId_Write(&a_pValue->TypeDefinitionNode, msgBuf);
    status &= Boolean_Write(&a_pValue->IncludeSubTypes, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDataToReturn, (void**) &a_pValue->DataToReturn, 
                   sizeof(OpcUa_QueryDataDescription), (SOPC_EncodeableObject_PfnEncode*) OpcUa_QueryDataDescription_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_NodeTypeDescription_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NodeTypeDescription_Decode(OpcUa_NodeTypeDescription* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_NodeTypeDescription_Initialize(a_pValue);

    status &= ExpandedNodeId_Read(&a_pValue->TypeDefinitionNode, msgBuf);
    status &= Boolean_Read(&a_pValue->IncludeSubTypes, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDataToReturn, (void**) &a_pValue->DataToReturn, 
                  sizeof(OpcUa_QueryDataDescription), (SOPC_EncodeableObject_PfnDecode*) OpcUa_QueryDataDescription_Decode);

    if(status != STATUS_OK){
        OpcUa_NodeTypeDescription_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_NodeTypeDescription_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_NodeTypeDescription_EncodeableType =
{
    "NodeTypeDescription",
    OpcUaId_NodeTypeDescription,
    OpcUaId_NodeTypeDescription_Encoding_DefaultBinary,
    OpcUaId_NodeTypeDescription_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_NodeTypeDescription),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_NodeTypeDescription_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_NodeTypeDescription_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_NodeTypeDescription_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_NodeTypeDescription_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_NodeTypeDescription_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_QueryDataSet
/*============================================================================
 * OpcUa_QueryDataSet_Initialize
 *===========================================================================*/
void OpcUa_QueryDataSet_Initialize(OpcUa_QueryDataSet* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Initialize(&a_pValue->NodeId);
        ExpandedNodeId_Initialize(&a_pValue->TypeDefinitionNode);
        SOPC_Initialize_Array(&a_pValue->NoOfValues, (void**) &a_pValue->Values, 
                            sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * OpcUa_QueryDataSet_Clear
 *===========================================================================*/
void OpcUa_QueryDataSet_Clear(OpcUa_QueryDataSet* a_pValue)
{
    if (a_pValue != NULL)
    {
        ExpandedNodeId_Clear(&a_pValue->NodeId);
        ExpandedNodeId_Clear(&a_pValue->TypeDefinitionNode);
        SOPC_Clear_Array(&a_pValue->NoOfValues, (void**) &a_pValue->Values, 
                       sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * OpcUa_QueryDataSet_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryDataSet_Encode(OpcUa_QueryDataSet* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= ExpandedNodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= ExpandedNodeId_Write(&a_pValue->TypeDefinitionNode, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfValues, (void**) &a_pValue->Values, 
                   sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryDataSet_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryDataSet_Decode(OpcUa_QueryDataSet* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_QueryDataSet_Initialize(a_pValue);

    status &= ExpandedNodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= ExpandedNodeId_Read(&a_pValue->TypeDefinitionNode, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfValues, (void**) &a_pValue->Values, 
                  sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        OpcUa_QueryDataSet_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryDataSet_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_QueryDataSet_EncodeableType =
{
    "QueryDataSet",
    OpcUaId_QueryDataSet,
    OpcUaId_QueryDataSet_Encoding_DefaultBinary,
    OpcUaId_QueryDataSet_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_QueryDataSet),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_QueryDataSet_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_QueryDataSet_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_QueryDataSet_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_QueryDataSet_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_QueryDataSet_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_NodeReference
/*============================================================================
 * OpcUa_NodeReference_Initialize
 *===========================================================================*/
void OpcUa_NodeReference_Initialize(OpcUa_NodeReference* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        NodeId_Initialize(&a_pValue->ReferenceTypeId);
        Boolean_Initialize(&a_pValue->IsForward);
        SOPC_Initialize_Array(&a_pValue->NoOfReferencedNodeIds, (void**) &a_pValue->ReferencedNodeIds, 
                            sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnInitialize*) NodeId_Initialize);
    }
}

/*============================================================================
 * OpcUa_NodeReference_Clear
 *===========================================================================*/
void OpcUa_NodeReference_Clear(OpcUa_NodeReference* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        NodeId_Clear(&a_pValue->ReferenceTypeId);
        Boolean_Clear(&a_pValue->IsForward);
        SOPC_Clear_Array(&a_pValue->NoOfReferencedNodeIds, (void**) &a_pValue->ReferencedNodeIds, 
                       sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnClear*) NodeId_Clear);
    }
}

/*============================================================================
 * OpcUa_NodeReference_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NodeReference_Encode(OpcUa_NodeReference* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= NodeId_Write(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Write(&a_pValue->IsForward, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReferencedNodeIds, (void**) &a_pValue->ReferencedNodeIds, 
                   sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnEncode*) NodeId_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_NodeReference_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NodeReference_Decode(OpcUa_NodeReference* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_NodeReference_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= NodeId_Read(&a_pValue->ReferenceTypeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsForward, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReferencedNodeIds, (void**) &a_pValue->ReferencedNodeIds, 
                  sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnDecode*) NodeId_Read);

    if(status != STATUS_OK){
        OpcUa_NodeReference_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_NodeReference_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_NodeReference_EncodeableType =
{
    "NodeReference",
    OpcUaId_NodeReference,
    OpcUaId_NodeReference_Encoding_DefaultBinary,
    OpcUaId_NodeReference_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_NodeReference),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_NodeReference_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_NodeReference_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_NodeReference_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_NodeReference_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_NodeReference_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElement
/*============================================================================
 * OpcUa_ContentFilterElement_Initialize
 *===========================================================================*/
void OpcUa_ContentFilterElement_Initialize(OpcUa_ContentFilterElement* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->FilterOperator);
        SOPC_Initialize_Array(&a_pValue->NoOfFilterOperands, (void**) &a_pValue->FilterOperands, 
                            sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnInitialize*) ExtensionObject_Initialize);
    }
}

/*============================================================================
 * OpcUa_ContentFilterElement_Clear
 *===========================================================================*/
void OpcUa_ContentFilterElement_Clear(OpcUa_ContentFilterElement* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->FilterOperator);
        SOPC_Clear_Array(&a_pValue->NoOfFilterOperands, (void**) &a_pValue->FilterOperands, 
                       sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnClear*) ExtensionObject_Clear);
    }
}

/*============================================================================
 * OpcUa_ContentFilterElement_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ContentFilterElement_Encode(OpcUa_ContentFilterElement* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->FilterOperator);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfFilterOperands, (void**) &a_pValue->FilterOperands, 
                   sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnEncode*) ExtensionObject_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ContentFilterElement_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ContentFilterElement_Decode(OpcUa_ContentFilterElement* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ContentFilterElement_Initialize(a_pValue);

    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->FilterOperator);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfFilterOperands, (void**) &a_pValue->FilterOperands, 
                  sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnDecode*) ExtensionObject_Read);

    if(status != STATUS_OK){
        OpcUa_ContentFilterElement_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ContentFilterElement_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ContentFilterElement_EncodeableType =
{
    "ContentFilterElement",
    OpcUaId_ContentFilterElement,
    OpcUaId_ContentFilterElement_Encoding_DefaultBinary,
    OpcUaId_ContentFilterElement_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ContentFilterElement),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ContentFilterElement_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ContentFilterElement_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ContentFilterElement_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ContentFilterElement_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ContentFilterElement_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ContentFilter
/*============================================================================
 * OpcUa_ContentFilter_Initialize
 *===========================================================================*/
void OpcUa_ContentFilter_Initialize(OpcUa_ContentFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                            sizeof(OpcUa_ContentFilterElement), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ContentFilterElement_Initialize);
    }
}

/*============================================================================
 * OpcUa_ContentFilter_Clear
 *===========================================================================*/
void OpcUa_ContentFilter_Clear(OpcUa_ContentFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                       sizeof(OpcUa_ContentFilterElement), (SOPC_EncodeableObject_PfnClear*) OpcUa_ContentFilterElement_Clear);
    }
}

/*============================================================================
 * OpcUa_ContentFilter_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ContentFilter_Encode(OpcUa_ContentFilter* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                   sizeof(OpcUa_ContentFilterElement), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ContentFilterElement_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ContentFilter_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ContentFilter_Decode(OpcUa_ContentFilter* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ContentFilter_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfElements, (void**) &a_pValue->Elements, 
                  sizeof(OpcUa_ContentFilterElement), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ContentFilterElement_Decode);

    if(status != STATUS_OK){
        OpcUa_ContentFilter_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ContentFilter_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ContentFilter_EncodeableType =
{
    "ContentFilter",
    OpcUaId_ContentFilter,
    OpcUaId_ContentFilter_Encoding_DefaultBinary,
    OpcUaId_ContentFilter_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ContentFilter),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ContentFilter_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ContentFilter_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ContentFilter_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ContentFilter_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ContentFilter_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ElementOperand
/*============================================================================
 * OpcUa_ElementOperand_Initialize
 *===========================================================================*/
void OpcUa_ElementOperand_Initialize(OpcUa_ElementOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->Index);
    }
}

/*============================================================================
 * OpcUa_ElementOperand_Clear
 *===========================================================================*/
void OpcUa_ElementOperand_Clear(OpcUa_ElementOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->Index);
    }
}

/*============================================================================
 * OpcUa_ElementOperand_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ElementOperand_Encode(OpcUa_ElementOperand* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ElementOperand_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ElementOperand_Decode(OpcUa_ElementOperand* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ElementOperand_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->Index, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ElementOperand_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ElementOperand_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ElementOperand_EncodeableType =
{
    "ElementOperand",
    OpcUaId_ElementOperand,
    OpcUaId_ElementOperand_Encoding_DefaultBinary,
    OpcUaId_ElementOperand_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ElementOperand),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ElementOperand_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ElementOperand_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ElementOperand_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ElementOperand_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ElementOperand_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_LiteralOperand
/*============================================================================
 * OpcUa_LiteralOperand_Initialize
 *===========================================================================*/
void OpcUa_LiteralOperand_Initialize(OpcUa_LiteralOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        Variant_Initialize(&a_pValue->Value);
    }
}

/*============================================================================
 * OpcUa_LiteralOperand_Clear
 *===========================================================================*/
void OpcUa_LiteralOperand_Clear(OpcUa_LiteralOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        Variant_Clear(&a_pValue->Value);
    }
}

/*============================================================================
 * OpcUa_LiteralOperand_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_LiteralOperand_Encode(OpcUa_LiteralOperand* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_LiteralOperand_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_LiteralOperand_Decode(OpcUa_LiteralOperand* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_LiteralOperand_Initialize(a_pValue);

    status &= Variant_Read(&a_pValue->Value, msgBuf);

    if(status != STATUS_OK){
        OpcUa_LiteralOperand_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_LiteralOperand_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_LiteralOperand_EncodeableType =
{
    "LiteralOperand",
    OpcUaId_LiteralOperand,
    OpcUaId_LiteralOperand_Encoding_DefaultBinary,
    OpcUaId_LiteralOperand_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_LiteralOperand),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_LiteralOperand_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_LiteralOperand_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_LiteralOperand_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_LiteralOperand_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_LiteralOperand_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AttributeOperand
/*============================================================================
 * OpcUa_AttributeOperand_Initialize
 *===========================================================================*/
void OpcUa_AttributeOperand_Initialize(OpcUa_AttributeOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        String_Initialize(&a_pValue->Alias);
        OpcUa_RelativePath_Initialize(&a_pValue->BrowsePath);
        UInt32_Initialize(&a_pValue->AttributeId);
        String_Initialize(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * OpcUa_AttributeOperand_Clear
 *===========================================================================*/
void OpcUa_AttributeOperand_Clear(OpcUa_AttributeOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        String_Clear(&a_pValue->Alias);
        OpcUa_RelativePath_Clear(&a_pValue->BrowsePath);
        UInt32_Clear(&a_pValue->AttributeId);
        String_Clear(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * OpcUa_AttributeOperand_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AttributeOperand_Encode(OpcUa_AttributeOperand* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= String_Write(&a_pValue->Alias, msgBuf);
    status &= OpcUa_RelativePath_Encode(&a_pValue->BrowsePath, msgBuf);
    status &= UInt32_Write(&a_pValue->AttributeId, msgBuf);
    status &= String_Write(&a_pValue->IndexRange, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AttributeOperand_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AttributeOperand_Decode(OpcUa_AttributeOperand* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AttributeOperand_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= String_Read(&a_pValue->Alias, msgBuf);
    status &= OpcUa_RelativePath_Decode(&a_pValue->BrowsePath, msgBuf);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);

    if(status != STATUS_OK){
        OpcUa_AttributeOperand_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AttributeOperand_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AttributeOperand_EncodeableType =
{
    "AttributeOperand",
    OpcUaId_AttributeOperand,
    OpcUaId_AttributeOperand_Encoding_DefaultBinary,
    OpcUaId_AttributeOperand_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AttributeOperand),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AttributeOperand_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AttributeOperand_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AttributeOperand_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AttributeOperand_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AttributeOperand_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SimpleAttributeOperand
/*============================================================================
 * OpcUa_SimpleAttributeOperand_Initialize
 *===========================================================================*/
void OpcUa_SimpleAttributeOperand_Initialize(OpcUa_SimpleAttributeOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->TypeDefinitionId);
        SOPC_Initialize_Array(&a_pValue->NoOfBrowsePath, (void**) &a_pValue->BrowsePath, 
                            sizeof(SOPC_QualifiedName), (SOPC_EncodeableObject_PfnInitialize*) QualifiedName_Initialize);
        UInt32_Initialize(&a_pValue->AttributeId);
        String_Initialize(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * OpcUa_SimpleAttributeOperand_Clear
 *===========================================================================*/
void OpcUa_SimpleAttributeOperand_Clear(OpcUa_SimpleAttributeOperand* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->TypeDefinitionId);
        SOPC_Clear_Array(&a_pValue->NoOfBrowsePath, (void**) &a_pValue->BrowsePath, 
                       sizeof(SOPC_QualifiedName), (SOPC_EncodeableObject_PfnClear*) QualifiedName_Clear);
        UInt32_Clear(&a_pValue->AttributeId);
        String_Clear(&a_pValue->IndexRange);
    }
}

/*============================================================================
 * OpcUa_SimpleAttributeOperand_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SimpleAttributeOperand_Encode(OpcUa_SimpleAttributeOperand* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->TypeDefinitionId, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfBrowsePath, (void**) &a_pValue->BrowsePath, 
                   sizeof(SOPC_QualifiedName), (SOPC_EncodeableObject_PfnEncode*) QualifiedName_Write);
    status &= UInt32_Write(&a_pValue->AttributeId, msgBuf);
    status &= String_Write(&a_pValue->IndexRange, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SimpleAttributeOperand_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SimpleAttributeOperand_Decode(OpcUa_SimpleAttributeOperand* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SimpleAttributeOperand_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->TypeDefinitionId, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfBrowsePath, (void**) &a_pValue->BrowsePath, 
                  sizeof(SOPC_QualifiedName), (SOPC_EncodeableObject_PfnDecode*) QualifiedName_Read);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);

    if(status != STATUS_OK){
        OpcUa_SimpleAttributeOperand_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SimpleAttributeOperand_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SimpleAttributeOperand_EncodeableType =
{
    "SimpleAttributeOperand",
    OpcUaId_SimpleAttributeOperand,
    OpcUaId_SimpleAttributeOperand_Encoding_DefaultBinary,
    OpcUaId_SimpleAttributeOperand_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SimpleAttributeOperand),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SimpleAttributeOperand_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SimpleAttributeOperand_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SimpleAttributeOperand_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SimpleAttributeOperand_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SimpleAttributeOperand_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterElementResult
/*============================================================================
 * OpcUa_ContentFilterElementResult_Initialize
 *===========================================================================*/
void OpcUa_ContentFilterElementResult_Initialize(OpcUa_ContentFilterElementResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        SOPC_Initialize_Array(&a_pValue->NoOfOperandStatusCodes, (void**) &a_pValue->OperandStatusCodes, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfOperandDiagnosticInfos, (void**) &a_pValue->OperandDiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_ContentFilterElementResult_Clear
 *===========================================================================*/
void OpcUa_ContentFilterElementResult_Clear(OpcUa_ContentFilterElementResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        SOPC_Clear_Array(&a_pValue->NoOfOperandStatusCodes, (void**) &a_pValue->OperandStatusCodes, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfOperandDiagnosticInfos, (void**) &a_pValue->OperandDiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_ContentFilterElementResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ContentFilterElementResult_Encode(OpcUa_ContentFilterElementResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfOperandStatusCodes, (void**) &a_pValue->OperandStatusCodes, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfOperandDiagnosticInfos, (void**) &a_pValue->OperandDiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ContentFilterElementResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ContentFilterElementResult_Decode(OpcUa_ContentFilterElementResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ContentFilterElementResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfOperandStatusCodes, (void**) &a_pValue->OperandStatusCodes, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfOperandDiagnosticInfos, (void**) &a_pValue->OperandDiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_ContentFilterElementResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ContentFilterElementResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ContentFilterElementResult_EncodeableType =
{
    "ContentFilterElementResult",
    OpcUaId_ContentFilterElementResult,
    OpcUaId_ContentFilterElementResult_Encoding_DefaultBinary,
    OpcUaId_ContentFilterElementResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ContentFilterElementResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ContentFilterElementResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ContentFilterElementResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ContentFilterElementResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ContentFilterElementResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ContentFilterElementResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ContentFilterResult
/*============================================================================
 * OpcUa_ContentFilterResult_Initialize
 *===========================================================================*/
void OpcUa_ContentFilterResult_Initialize(OpcUa_ContentFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfElementResults, (void**) &a_pValue->ElementResults, 
                            sizeof(OpcUa_ContentFilterElementResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ContentFilterElementResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfElementDiagnosticInfos, (void**) &a_pValue->ElementDiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_ContentFilterResult_Clear
 *===========================================================================*/
void OpcUa_ContentFilterResult_Clear(OpcUa_ContentFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfElementResults, (void**) &a_pValue->ElementResults, 
                       sizeof(OpcUa_ContentFilterElementResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_ContentFilterElementResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfElementDiagnosticInfos, (void**) &a_pValue->ElementDiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_ContentFilterResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ContentFilterResult_Encode(OpcUa_ContentFilterResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfElementResults, (void**) &a_pValue->ElementResults, 
                   sizeof(OpcUa_ContentFilterElementResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ContentFilterElementResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfElementDiagnosticInfos, (void**) &a_pValue->ElementDiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ContentFilterResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ContentFilterResult_Decode(OpcUa_ContentFilterResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ContentFilterResult_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfElementResults, (void**) &a_pValue->ElementResults, 
                  sizeof(OpcUa_ContentFilterElementResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ContentFilterElementResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfElementDiagnosticInfos, (void**) &a_pValue->ElementDiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_ContentFilterResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ContentFilterResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ContentFilterResult_EncodeableType =
{
    "ContentFilterResult",
    OpcUaId_ContentFilterResult,
    OpcUaId_ContentFilterResult_Encoding_DefaultBinary,
    OpcUaId_ContentFilterResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ContentFilterResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ContentFilterResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ContentFilterResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ContentFilterResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ContentFilterResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ContentFilterResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ParsingResult
/*============================================================================
 * OpcUa_ParsingResult_Initialize
 *===========================================================================*/
void OpcUa_ParsingResult_Initialize(OpcUa_ParsingResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        SOPC_Initialize_Array(&a_pValue->NoOfDataStatusCodes, (void**) &a_pValue->DataStatusCodes, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDataDiagnosticInfos, (void**) &a_pValue->DataDiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_ParsingResult_Clear
 *===========================================================================*/
void OpcUa_ParsingResult_Clear(OpcUa_ParsingResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        SOPC_Clear_Array(&a_pValue->NoOfDataStatusCodes, (void**) &a_pValue->DataStatusCodes, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDataDiagnosticInfos, (void**) &a_pValue->DataDiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_ParsingResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ParsingResult_Encode(OpcUa_ParsingResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDataStatusCodes, (void**) &a_pValue->DataStatusCodes, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDataDiagnosticInfos, (void**) &a_pValue->DataDiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ParsingResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ParsingResult_Decode(OpcUa_ParsingResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ParsingResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDataStatusCodes, (void**) &a_pValue->DataStatusCodes, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDataDiagnosticInfos, (void**) &a_pValue->DataDiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_ParsingResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ParsingResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ParsingResult_EncodeableType =
{
    "ParsingResult",
    OpcUaId_ParsingResult,
    OpcUaId_ParsingResult_Encoding_DefaultBinary,
    OpcUaId_ParsingResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ParsingResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ParsingResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ParsingResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ParsingResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ParsingResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ParsingResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_QueryFirst
#ifndef OPCUA_EXCLUDE_QueryFirstRequest
/*============================================================================
 * OpcUa_QueryFirstRequest_Initialize
 *===========================================================================*/
void OpcUa_QueryFirstRequest_Initialize(OpcUa_QueryFirstRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        OpcUa_ViewDescription_Initialize(&a_pValue->View);
        SOPC_Initialize_Array(&a_pValue->NoOfNodeTypes, (void**) &a_pValue->NodeTypes, 
                            sizeof(OpcUa_NodeTypeDescription), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_NodeTypeDescription_Initialize);
        OpcUa_ContentFilter_Initialize(&a_pValue->Filter);
        UInt32_Initialize(&a_pValue->MaxDataSetsToReturn);
        UInt32_Initialize(&a_pValue->MaxReferencesToReturn);
    }
}

/*============================================================================
 * OpcUa_QueryFirstRequest_Clear
 *===========================================================================*/
void OpcUa_QueryFirstRequest_Clear(OpcUa_QueryFirstRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        OpcUa_ViewDescription_Clear(&a_pValue->View);
        SOPC_Clear_Array(&a_pValue->NoOfNodeTypes, (void**) &a_pValue->NodeTypes, 
                       sizeof(OpcUa_NodeTypeDescription), (SOPC_EncodeableObject_PfnClear*) OpcUa_NodeTypeDescription_Clear);
        OpcUa_ContentFilter_Clear(&a_pValue->Filter);
        UInt32_Clear(&a_pValue->MaxDataSetsToReturn);
        UInt32_Clear(&a_pValue->MaxReferencesToReturn);
    }
}

/*============================================================================
 * OpcUa_QueryFirstRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryFirstRequest_Encode(OpcUa_QueryFirstRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_ViewDescription_Encode(&a_pValue->View, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNodeTypes, (void**) &a_pValue->NodeTypes, 
                   sizeof(OpcUa_NodeTypeDescription), (SOPC_EncodeableObject_PfnEncode*) OpcUa_NodeTypeDescription_Encode);
    status &= OpcUa_ContentFilter_Encode(&a_pValue->Filter, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxDataSetsToReturn, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxReferencesToReturn, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryFirstRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryFirstRequest_Decode(OpcUa_QueryFirstRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_QueryFirstRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= OpcUa_ViewDescription_Decode(&a_pValue->View, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNodeTypes, (void**) &a_pValue->NodeTypes, 
                  sizeof(OpcUa_NodeTypeDescription), (SOPC_EncodeableObject_PfnDecode*) OpcUa_NodeTypeDescription_Decode);
    status &= OpcUa_ContentFilter_Decode(&a_pValue->Filter, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxDataSetsToReturn, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxReferencesToReturn, msgBuf);

    if(status != STATUS_OK){
        OpcUa_QueryFirstRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryFirstRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_QueryFirstRequest_EncodeableType =
{
    "QueryFirstRequest",
    OpcUaId_QueryFirstRequest,
    OpcUaId_QueryFirstRequest_Encoding_DefaultBinary,
    OpcUaId_QueryFirstRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_QueryFirstRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_QueryFirstRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_QueryFirstRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_QueryFirstRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_QueryFirstRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_QueryFirstRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_QueryFirstResponse
/*============================================================================
 * OpcUa_QueryFirstResponse_Initialize
 *===========================================================================*/
void OpcUa_QueryFirstResponse_Initialize(OpcUa_QueryFirstResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                            sizeof(OpcUa_QueryDataSet), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_QueryDataSet_Initialize);
        ByteString_Initialize(&a_pValue->ContinuationPoint);
        SOPC_Initialize_Array(&a_pValue->NoOfParsingResults, (void**) &a_pValue->ParsingResults, 
                            sizeof(OpcUa_ParsingResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ParsingResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
        OpcUa_ContentFilterResult_Initialize(&a_pValue->FilterResult);
    }
}

/*============================================================================
 * OpcUa_QueryFirstResponse_Clear
 *===========================================================================*/
void OpcUa_QueryFirstResponse_Clear(OpcUa_QueryFirstResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                       sizeof(OpcUa_QueryDataSet), (SOPC_EncodeableObject_PfnClear*) OpcUa_QueryDataSet_Clear);
        ByteString_Clear(&a_pValue->ContinuationPoint);
        SOPC_Clear_Array(&a_pValue->NoOfParsingResults, (void**) &a_pValue->ParsingResults, 
                       sizeof(OpcUa_ParsingResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_ParsingResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
        OpcUa_ContentFilterResult_Clear(&a_pValue->FilterResult);
    }
}

/*============================================================================
 * OpcUa_QueryFirstResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryFirstResponse_Encode(OpcUa_QueryFirstResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                   sizeof(OpcUa_QueryDataSet), (SOPC_EncodeableObject_PfnEncode*) OpcUa_QueryDataSet_Encode);
    status &= ByteString_Write(&a_pValue->ContinuationPoint, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfParsingResults, (void**) &a_pValue->ParsingResults, 
                   sizeof(OpcUa_ParsingResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ParsingResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);
    status &= OpcUa_ContentFilterResult_Encode(&a_pValue->FilterResult, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryFirstResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryFirstResponse_Decode(OpcUa_QueryFirstResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_QueryFirstResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                  sizeof(OpcUa_QueryDataSet), (SOPC_EncodeableObject_PfnDecode*) OpcUa_QueryDataSet_Decode);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfParsingResults, (void**) &a_pValue->ParsingResults, 
                  sizeof(OpcUa_ParsingResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ParsingResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);
    status &= OpcUa_ContentFilterResult_Decode(&a_pValue->FilterResult, msgBuf);

    if(status != STATUS_OK){
        OpcUa_QueryFirstResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryFirstResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_QueryFirstResponse_EncodeableType =
{
    "QueryFirstResponse",
    OpcUaId_QueryFirstResponse,
    OpcUaId_QueryFirstResponse_Encoding_DefaultBinary,
    OpcUaId_QueryFirstResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_QueryFirstResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_QueryFirstResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_QueryFirstResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_QueryFirstResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_QueryFirstResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_QueryFirstResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_QueryNext
#ifndef OPCUA_EXCLUDE_QueryNextRequest
/*============================================================================
 * OpcUa_QueryNextRequest_Initialize
 *===========================================================================*/
void OpcUa_QueryNextRequest_Initialize(OpcUa_QueryNextRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Boolean_Initialize(&a_pValue->ReleaseContinuationPoint);
        ByteString_Initialize(&a_pValue->ContinuationPoint);
    }
}

/*============================================================================
 * OpcUa_QueryNextRequest_Clear
 *===========================================================================*/
void OpcUa_QueryNextRequest_Clear(OpcUa_QueryNextRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        Boolean_Clear(&a_pValue->ReleaseContinuationPoint);
        ByteString_Clear(&a_pValue->ContinuationPoint);
    }
}

/*============================================================================
 * OpcUa_QueryNextRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryNextRequest_Encode(OpcUa_QueryNextRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Write(&a_pValue->ReleaseContinuationPoint, msgBuf);
    status &= ByteString_Write(&a_pValue->ContinuationPoint, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryNextRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryNextRequest_Decode(OpcUa_QueryNextRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_QueryNextRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Read(&a_pValue->ReleaseContinuationPoint, msgBuf);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);

    if(status != STATUS_OK){
        OpcUa_QueryNextRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryNextRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_QueryNextRequest_EncodeableType =
{
    "QueryNextRequest",
    OpcUaId_QueryNextRequest,
    OpcUaId_QueryNextRequest_Encoding_DefaultBinary,
    OpcUaId_QueryNextRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_QueryNextRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_QueryNextRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_QueryNextRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_QueryNextRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_QueryNextRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_QueryNextRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_QueryNextResponse
/*============================================================================
 * OpcUa_QueryNextResponse_Initialize
 *===========================================================================*/
void OpcUa_QueryNextResponse_Initialize(OpcUa_QueryNextResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                            sizeof(OpcUa_QueryDataSet), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_QueryDataSet_Initialize);
        ByteString_Initialize(&a_pValue->RevisedContinuationPoint);
    }
}

/*============================================================================
 * OpcUa_QueryNextResponse_Clear
 *===========================================================================*/
void OpcUa_QueryNextResponse_Clear(OpcUa_QueryNextResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                       sizeof(OpcUa_QueryDataSet), (SOPC_EncodeableObject_PfnClear*) OpcUa_QueryDataSet_Clear);
        ByteString_Clear(&a_pValue->RevisedContinuationPoint);
    }
}

/*============================================================================
 * OpcUa_QueryNextResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryNextResponse_Encode(OpcUa_QueryNextResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                   sizeof(OpcUa_QueryDataSet), (SOPC_EncodeableObject_PfnEncode*) OpcUa_QueryDataSet_Encode);
    status &= ByteString_Write(&a_pValue->RevisedContinuationPoint, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryNextResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_QueryNextResponse_Decode(OpcUa_QueryNextResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_QueryNextResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfQueryDataSets, (void**) &a_pValue->QueryDataSets, 
                  sizeof(OpcUa_QueryDataSet), (SOPC_EncodeableObject_PfnDecode*) OpcUa_QueryDataSet_Decode);
    status &= ByteString_Read(&a_pValue->RevisedContinuationPoint, msgBuf);

    if(status != STATUS_OK){
        OpcUa_QueryNextResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_QueryNextResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_QueryNextResponse_EncodeableType =
{
    "QueryNextResponse",
    OpcUaId_QueryNextResponse,
    OpcUaId_QueryNextResponse_Encoding_DefaultBinary,
    OpcUaId_QueryNextResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_QueryNextResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_QueryNextResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_QueryNextResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_QueryNextResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_QueryNextResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_QueryNextResponse_Decode
};
#endif
#endif


#ifndef OPCUA_EXCLUDE_ReadValueId
/*============================================================================
 * OpcUa_ReadValueId_Initialize
 *===========================================================================*/
void OpcUa_ReadValueId_Initialize(OpcUa_ReadValueId* a_pValue)
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
 * OpcUa_ReadValueId_Clear
 *===========================================================================*/
void OpcUa_ReadValueId_Clear(OpcUa_ReadValueId* a_pValue)
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
 * OpcUa_ReadValueId_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadValueId_Encode(OpcUa_ReadValueId* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ReadValueId_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadValueId_Decode(OpcUa_ReadValueId* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReadValueId_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);
    status &= QualifiedName_Read(&a_pValue->DataEncoding, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ReadValueId_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadValueId_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReadValueId_EncodeableType =
{
    "ReadValueId",
    OpcUaId_ReadValueId,
    OpcUaId_ReadValueId_Encoding_DefaultBinary,
    OpcUaId_ReadValueId_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReadValueId),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReadValueId_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReadValueId_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReadValueId_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReadValueId_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReadValueId_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Read
#ifndef OPCUA_EXCLUDE_ReadRequest
/*============================================================================
 * OpcUa_ReadRequest_Initialize
 *===========================================================================*/
void OpcUa_ReadRequest_Initialize(OpcUa_ReadRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Double_Initialize(&a_pValue->MaxAge);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        SOPC_Initialize_Array(&a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                            sizeof(OpcUa_ReadValueId), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ReadValueId_Initialize);
    }
}

/*============================================================================
 * OpcUa_ReadRequest_Clear
 *===========================================================================*/
void OpcUa_ReadRequest_Clear(OpcUa_ReadRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        Double_Clear(&a_pValue->MaxAge);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        SOPC_Clear_Array(&a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                       sizeof(OpcUa_ReadValueId), (SOPC_EncodeableObject_PfnClear*) OpcUa_ReadValueId_Clear);
    }
}

/*============================================================================
 * OpcUa_ReadRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadRequest_Encode(OpcUa_ReadRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Double_Write(&a_pValue->MaxAge, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                   sizeof(OpcUa_ReadValueId), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ReadValueId_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadRequest_Decode(OpcUa_ReadRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReadRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Double_Read(&a_pValue->MaxAge, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                  sizeof(OpcUa_ReadValueId), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ReadValueId_Decode);

    if(status != STATUS_OK){
        OpcUa_ReadRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReadRequest_EncodeableType =
{
    "ReadRequest",
    OpcUaId_ReadRequest,
    OpcUaId_ReadRequest_Encoding_DefaultBinary,
    OpcUaId_ReadRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReadRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReadRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReadRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReadRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReadRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReadRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadResponse
/*============================================================================
 * OpcUa_ReadResponse_Initialize
 *===========================================================================*/
void OpcUa_ReadResponse_Initialize(OpcUa_ReadResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnInitialize*) DataValue_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_ReadResponse_Clear
 *===========================================================================*/
void OpcUa_ReadResponse_Clear(OpcUa_ReadResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnClear*) DataValue_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_ReadResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadResponse_Encode(OpcUa_ReadResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnEncode*) DataValue_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadResponse_Decode(OpcUa_ReadResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReadResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnDecode*) DataValue_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_ReadResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReadResponse_EncodeableType =
{
    "ReadResponse",
    OpcUaId_ReadResponse,
    OpcUaId_ReadResponse_Encoding_DefaultBinary,
    OpcUaId_ReadResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReadResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReadResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReadResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReadResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReadResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReadResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadValueId
/*============================================================================
 * OpcUa_HistoryReadValueId_Initialize
 *===========================================================================*/
void OpcUa_HistoryReadValueId_Initialize(OpcUa_HistoryReadValueId* a_pValue)
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
 * OpcUa_HistoryReadValueId_Clear
 *===========================================================================*/
void OpcUa_HistoryReadValueId_Clear(OpcUa_HistoryReadValueId* a_pValue)
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
 * OpcUa_HistoryReadValueId_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryReadValueId_Encode(OpcUa_HistoryReadValueId* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_HistoryReadValueId_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryReadValueId_Decode(OpcUa_HistoryReadValueId* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryReadValueId_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);
    status &= QualifiedName_Read(&a_pValue->DataEncoding, msgBuf);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);

    if(status != STATUS_OK){
        OpcUa_HistoryReadValueId_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryReadValueId_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryReadValueId_EncodeableType =
{
    "HistoryReadValueId",
    OpcUaId_HistoryReadValueId,
    OpcUaId_HistoryReadValueId_Encoding_DefaultBinary,
    OpcUaId_HistoryReadValueId_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryReadValueId),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryReadValueId_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryReadValueId_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryReadValueId_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryReadValueId_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryReadValueId_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadResult
/*============================================================================
 * OpcUa_HistoryReadResult_Initialize
 *===========================================================================*/
void OpcUa_HistoryReadResult_Initialize(OpcUa_HistoryReadResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        ByteString_Initialize(&a_pValue->ContinuationPoint);
        ExtensionObject_Initialize(&a_pValue->HistoryData);
    }
}

/*============================================================================
 * OpcUa_HistoryReadResult_Clear
 *===========================================================================*/
void OpcUa_HistoryReadResult_Clear(OpcUa_HistoryReadResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        ByteString_Clear(&a_pValue->ContinuationPoint);
        ExtensionObject_Clear(&a_pValue->HistoryData);
    }
}

/*============================================================================
 * OpcUa_HistoryReadResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryReadResult_Encode(OpcUa_HistoryReadResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_HistoryReadResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryReadResult_Decode(OpcUa_HistoryReadResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryReadResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= ByteString_Read(&a_pValue->ContinuationPoint, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->HistoryData, msgBuf);

    if(status != STATUS_OK){
        OpcUa_HistoryReadResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryReadResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryReadResult_EncodeableType =
{
    "HistoryReadResult",
    OpcUaId_HistoryReadResult,
    OpcUaId_HistoryReadResult_Encoding_DefaultBinary,
    OpcUaId_HistoryReadResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryReadResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryReadResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryReadResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryReadResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryReadResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryReadResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadEventDetails
/*============================================================================
 * OpcUa_ReadEventDetails_Initialize
 *===========================================================================*/
void OpcUa_ReadEventDetails_Initialize(OpcUa_ReadEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->NumValuesPerNode);
        DateTime_Initialize(&a_pValue->StartTime);
        DateTime_Initialize(&a_pValue->EndTime);
        OpcUa_EventFilter_Initialize(&a_pValue->Filter);
    }
}

/*============================================================================
 * OpcUa_ReadEventDetails_Clear
 *===========================================================================*/
void OpcUa_ReadEventDetails_Clear(OpcUa_ReadEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->NumValuesPerNode);
        DateTime_Clear(&a_pValue->StartTime);
        DateTime_Clear(&a_pValue->EndTime);
        OpcUa_EventFilter_Clear(&a_pValue->Filter);
    }
}

/*============================================================================
 * OpcUa_ReadEventDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadEventDetails_Encode(OpcUa_ReadEventDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->NumValuesPerNode, msgBuf);
    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Write(&a_pValue->EndTime, msgBuf);
    status &= OpcUa_EventFilter_Encode(&a_pValue->Filter, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadEventDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadEventDetails_Decode(OpcUa_ReadEventDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReadEventDetails_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->NumValuesPerNode, msgBuf);
    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->EndTime, msgBuf);
    status &= OpcUa_EventFilter_Decode(&a_pValue->Filter, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ReadEventDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadEventDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReadEventDetails_EncodeableType =
{
    "ReadEventDetails",
    OpcUaId_ReadEventDetails,
    OpcUaId_ReadEventDetails_Encoding_DefaultBinary,
    OpcUaId_ReadEventDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReadEventDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReadEventDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReadEventDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReadEventDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReadEventDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReadEventDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadRawModifiedDetails
/*============================================================================
 * OpcUa_ReadRawModifiedDetails_Initialize
 *===========================================================================*/
void OpcUa_ReadRawModifiedDetails_Initialize(OpcUa_ReadRawModifiedDetails* a_pValue)
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
 * OpcUa_ReadRawModifiedDetails_Clear
 *===========================================================================*/
void OpcUa_ReadRawModifiedDetails_Clear(OpcUa_ReadRawModifiedDetails* a_pValue)
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
 * OpcUa_ReadRawModifiedDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadRawModifiedDetails_Encode(OpcUa_ReadRawModifiedDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ReadRawModifiedDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadRawModifiedDetails_Decode(OpcUa_ReadRawModifiedDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReadRawModifiedDetails_Initialize(a_pValue);

    status &= Boolean_Read(&a_pValue->IsReadModified, msgBuf);
    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->EndTime, msgBuf);
    status &= UInt32_Read(&a_pValue->NumValuesPerNode, msgBuf);
    status &= Boolean_Read(&a_pValue->ReturnBounds, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ReadRawModifiedDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadRawModifiedDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReadRawModifiedDetails_EncodeableType =
{
    "ReadRawModifiedDetails",
    OpcUaId_ReadRawModifiedDetails,
    OpcUaId_ReadRawModifiedDetails_Encoding_DefaultBinary,
    OpcUaId_ReadRawModifiedDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReadRawModifiedDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReadRawModifiedDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReadRawModifiedDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReadRawModifiedDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReadRawModifiedDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReadRawModifiedDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadProcessedDetails
/*============================================================================
 * OpcUa_ReadProcessedDetails_Initialize
 *===========================================================================*/
void OpcUa_ReadProcessedDetails_Initialize(OpcUa_ReadProcessedDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->StartTime);
        DateTime_Initialize(&a_pValue->EndTime);
        Double_Initialize(&a_pValue->ProcessingInterval);
        SOPC_Initialize_Array(&a_pValue->NoOfAggregateType, (void**) &a_pValue->AggregateType, 
                            sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnInitialize*) NodeId_Initialize);
        OpcUa_AggregateConfiguration_Initialize(&a_pValue->AggregateConfiguration);
    }
}

/*============================================================================
 * OpcUa_ReadProcessedDetails_Clear
 *===========================================================================*/
void OpcUa_ReadProcessedDetails_Clear(OpcUa_ReadProcessedDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->StartTime);
        DateTime_Clear(&a_pValue->EndTime);
        Double_Clear(&a_pValue->ProcessingInterval);
        SOPC_Clear_Array(&a_pValue->NoOfAggregateType, (void**) &a_pValue->AggregateType, 
                       sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnClear*) NodeId_Clear);
        OpcUa_AggregateConfiguration_Clear(&a_pValue->AggregateConfiguration);
    }
}

/*============================================================================
 * OpcUa_ReadProcessedDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadProcessedDetails_Encode(OpcUa_ReadProcessedDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Write(&a_pValue->EndTime, msgBuf);
    status &= Double_Write(&a_pValue->ProcessingInterval, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfAggregateType, (void**) &a_pValue->AggregateType, 
                   sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnEncode*) NodeId_Write);
    status &= OpcUa_AggregateConfiguration_Encode(&a_pValue->AggregateConfiguration, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadProcessedDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadProcessedDetails_Decode(OpcUa_ReadProcessedDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReadProcessedDetails_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->EndTime, msgBuf);
    status &= Double_Read(&a_pValue->ProcessingInterval, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfAggregateType, (void**) &a_pValue->AggregateType, 
                  sizeof(SOPC_NodeId), (SOPC_EncodeableObject_PfnDecode*) NodeId_Read);
    status &= OpcUa_AggregateConfiguration_Decode(&a_pValue->AggregateConfiguration, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ReadProcessedDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadProcessedDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReadProcessedDetails_EncodeableType =
{
    "ReadProcessedDetails",
    OpcUaId_ReadProcessedDetails,
    OpcUaId_ReadProcessedDetails_Encoding_DefaultBinary,
    OpcUaId_ReadProcessedDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReadProcessedDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReadProcessedDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReadProcessedDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReadProcessedDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReadProcessedDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReadProcessedDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ReadAtTimeDetails
/*============================================================================
 * OpcUa_ReadAtTimeDetails_Initialize
 *===========================================================================*/
void OpcUa_ReadAtTimeDetails_Initialize(OpcUa_ReadAtTimeDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                            sizeof(SOPC_DateTime), (SOPC_EncodeableObject_PfnInitialize*) DateTime_Initialize);
        Boolean_Initialize(&a_pValue->UseSimpleBounds);
    }
}

/*============================================================================
 * OpcUa_ReadAtTimeDetails_Clear
 *===========================================================================*/
void OpcUa_ReadAtTimeDetails_Clear(OpcUa_ReadAtTimeDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                       sizeof(SOPC_DateTime), (SOPC_EncodeableObject_PfnClear*) DateTime_Clear);
        Boolean_Clear(&a_pValue->UseSimpleBounds);
    }
}

/*============================================================================
 * OpcUa_ReadAtTimeDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadAtTimeDetails_Encode(OpcUa_ReadAtTimeDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                   sizeof(SOPC_DateTime), (SOPC_EncodeableObject_PfnEncode*) DateTime_Write);
    status &= Boolean_Write(&a_pValue->UseSimpleBounds, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadAtTimeDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ReadAtTimeDetails_Decode(OpcUa_ReadAtTimeDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ReadAtTimeDetails_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                  sizeof(SOPC_DateTime), (SOPC_EncodeableObject_PfnDecode*) DateTime_Read);
    status &= Boolean_Read(&a_pValue->UseSimpleBounds, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ReadAtTimeDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ReadAtTimeDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ReadAtTimeDetails_EncodeableType =
{
    "ReadAtTimeDetails",
    OpcUaId_ReadAtTimeDetails,
    OpcUaId_ReadAtTimeDetails_Encoding_DefaultBinary,
    OpcUaId_ReadAtTimeDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ReadAtTimeDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ReadAtTimeDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ReadAtTimeDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ReadAtTimeDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ReadAtTimeDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ReadAtTimeDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryData
/*============================================================================
 * OpcUa_HistoryData_Initialize
 *===========================================================================*/
void OpcUa_HistoryData_Initialize(OpcUa_HistoryData* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                            sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnInitialize*) DataValue_Initialize);
    }
}

/*============================================================================
 * OpcUa_HistoryData_Clear
 *===========================================================================*/
void OpcUa_HistoryData_Clear(OpcUa_HistoryData* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                       sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnClear*) DataValue_Clear);
    }
}

/*============================================================================
 * OpcUa_HistoryData_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryData_Encode(OpcUa_HistoryData* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                   sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnEncode*) DataValue_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryData_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryData_Decode(OpcUa_HistoryData* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryData_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                  sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnDecode*) DataValue_Read);

    if(status != STATUS_OK){
        OpcUa_HistoryData_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryData_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryData_EncodeableType =
{
    "HistoryData",
    OpcUaId_HistoryData,
    OpcUaId_HistoryData_Encoding_DefaultBinary,
    OpcUaId_HistoryData_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryData),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryData_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryData_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryData_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryData_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryData_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ModificationInfo
/*============================================================================
 * OpcUa_ModificationInfo_Initialize
 *===========================================================================*/
void OpcUa_ModificationInfo_Initialize(OpcUa_ModificationInfo* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->ModificationTime);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->UpdateType);
        String_Initialize(&a_pValue->UserName);
    }
}

/*============================================================================
 * OpcUa_ModificationInfo_Clear
 *===========================================================================*/
void OpcUa_ModificationInfo_Clear(OpcUa_ModificationInfo* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->ModificationTime);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->UpdateType);
        String_Clear(&a_pValue->UserName);
    }
}

/*============================================================================
 * OpcUa_ModificationInfo_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModificationInfo_Encode(OpcUa_ModificationInfo* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->ModificationTime, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->UpdateType);
    status &= String_Write(&a_pValue->UserName, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ModificationInfo_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModificationInfo_Decode(OpcUa_ModificationInfo* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ModificationInfo_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->ModificationTime, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->UpdateType);
    status &= String_Read(&a_pValue->UserName, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ModificationInfo_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ModificationInfo_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ModificationInfo_EncodeableType =
{
    "ModificationInfo",
    OpcUaId_ModificationInfo,
    OpcUaId_ModificationInfo_Encoding_DefaultBinary,
    OpcUaId_ModificationInfo_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ModificationInfo),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ModificationInfo_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ModificationInfo_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ModificationInfo_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ModificationInfo_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ModificationInfo_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryModifiedData
/*============================================================================
 * OpcUa_HistoryModifiedData_Initialize
 *===========================================================================*/
void OpcUa_HistoryModifiedData_Initialize(OpcUa_HistoryModifiedData* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                            sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnInitialize*) DataValue_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfModificationInfos, (void**) &a_pValue->ModificationInfos, 
                            sizeof(OpcUa_ModificationInfo), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_ModificationInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_HistoryModifiedData_Clear
 *===========================================================================*/
void OpcUa_HistoryModifiedData_Clear(OpcUa_HistoryModifiedData* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                       sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnClear*) DataValue_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfModificationInfos, (void**) &a_pValue->ModificationInfos, 
                       sizeof(OpcUa_ModificationInfo), (SOPC_EncodeableObject_PfnClear*) OpcUa_ModificationInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_HistoryModifiedData_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryModifiedData_Encode(OpcUa_HistoryModifiedData* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                   sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnEncode*) DataValue_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfModificationInfos, (void**) &a_pValue->ModificationInfos, 
                   sizeof(OpcUa_ModificationInfo), (SOPC_EncodeableObject_PfnEncode*) OpcUa_ModificationInfo_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryModifiedData_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryModifiedData_Decode(OpcUa_HistoryModifiedData* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryModifiedData_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDataValues, (void**) &a_pValue->DataValues, 
                  sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnDecode*) DataValue_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfModificationInfos, (void**) &a_pValue->ModificationInfos, 
                  sizeof(OpcUa_ModificationInfo), (SOPC_EncodeableObject_PfnDecode*) OpcUa_ModificationInfo_Decode);

    if(status != STATUS_OK){
        OpcUa_HistoryModifiedData_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryModifiedData_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryModifiedData_EncodeableType =
{
    "HistoryModifiedData",
    OpcUaId_HistoryModifiedData,
    OpcUaId_HistoryModifiedData_Encoding_DefaultBinary,
    OpcUaId_HistoryModifiedData_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryModifiedData),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryModifiedData_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryModifiedData_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryModifiedData_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryModifiedData_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryModifiedData_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryEvent
/*============================================================================
 * OpcUa_HistoryEvent_Initialize
 *===========================================================================*/
void OpcUa_HistoryEvent_Initialize(OpcUa_HistoryEvent* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                            sizeof(OpcUa_HistoryEventFieldList), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_HistoryEventFieldList_Initialize);
    }
}

/*============================================================================
 * OpcUa_HistoryEvent_Clear
 *===========================================================================*/
void OpcUa_HistoryEvent_Clear(OpcUa_HistoryEvent* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                       sizeof(OpcUa_HistoryEventFieldList), (SOPC_EncodeableObject_PfnClear*) OpcUa_HistoryEventFieldList_Clear);
    }
}

/*============================================================================
 * OpcUa_HistoryEvent_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryEvent_Encode(OpcUa_HistoryEvent* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                   sizeof(OpcUa_HistoryEventFieldList), (SOPC_EncodeableObject_PfnEncode*) OpcUa_HistoryEventFieldList_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryEvent_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryEvent_Decode(OpcUa_HistoryEvent* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryEvent_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                  sizeof(OpcUa_HistoryEventFieldList), (SOPC_EncodeableObject_PfnDecode*) OpcUa_HistoryEventFieldList_Decode);

    if(status != STATUS_OK){
        OpcUa_HistoryEvent_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryEvent_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryEvent_EncodeableType =
{
    "HistoryEvent",
    OpcUaId_HistoryEvent,
    OpcUaId_HistoryEvent_Encoding_DefaultBinary,
    OpcUaId_HistoryEvent_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryEvent),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryEvent_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryEvent_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryEvent_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryEvent_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryEvent_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryRead
#ifndef OPCUA_EXCLUDE_HistoryReadRequest
/*============================================================================
 * OpcUa_HistoryReadRequest_Initialize
 *===========================================================================*/
void OpcUa_HistoryReadRequest_Initialize(OpcUa_HistoryReadRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        ExtensionObject_Initialize(&a_pValue->HistoryReadDetails);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        Boolean_Initialize(&a_pValue->ReleaseContinuationPoints);
        SOPC_Initialize_Array(&a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                            sizeof(OpcUa_HistoryReadValueId), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_HistoryReadValueId_Initialize);
    }
}

/*============================================================================
 * OpcUa_HistoryReadRequest_Clear
 *===========================================================================*/
void OpcUa_HistoryReadRequest_Clear(OpcUa_HistoryReadRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        ExtensionObject_Clear(&a_pValue->HistoryReadDetails);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        Boolean_Clear(&a_pValue->ReleaseContinuationPoints);
        SOPC_Clear_Array(&a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                       sizeof(OpcUa_HistoryReadValueId), (SOPC_EncodeableObject_PfnClear*) OpcUa_HistoryReadValueId_Clear);
    }
}

/*============================================================================
 * OpcUa_HistoryReadRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryReadRequest_Encode(OpcUa_HistoryReadRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= ExtensionObject_Write(&a_pValue->HistoryReadDetails, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= Boolean_Write(&a_pValue->ReleaseContinuationPoints, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                   sizeof(OpcUa_HistoryReadValueId), (SOPC_EncodeableObject_PfnEncode*) OpcUa_HistoryReadValueId_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryReadRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryReadRequest_Decode(OpcUa_HistoryReadRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryReadRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->HistoryReadDetails, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= Boolean_Read(&a_pValue->ReleaseContinuationPoints, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNodesToRead, (void**) &a_pValue->NodesToRead, 
                  sizeof(OpcUa_HistoryReadValueId), (SOPC_EncodeableObject_PfnDecode*) OpcUa_HistoryReadValueId_Decode);

    if(status != STATUS_OK){
        OpcUa_HistoryReadRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryReadRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryReadRequest_EncodeableType =
{
    "HistoryReadRequest",
    OpcUaId_HistoryReadRequest,
    OpcUaId_HistoryReadRequest_Encoding_DefaultBinary,
    OpcUaId_HistoryReadRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryReadRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryReadRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryReadRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryReadRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryReadRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryReadRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryReadResponse
/*============================================================================
 * OpcUa_HistoryReadResponse_Initialize
 *===========================================================================*/
void OpcUa_HistoryReadResponse_Initialize(OpcUa_HistoryReadResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_HistoryReadResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_HistoryReadResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_HistoryReadResponse_Clear
 *===========================================================================*/
void OpcUa_HistoryReadResponse_Clear(OpcUa_HistoryReadResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_HistoryReadResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_HistoryReadResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_HistoryReadResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryReadResponse_Encode(OpcUa_HistoryReadResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_HistoryReadResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_HistoryReadResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryReadResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryReadResponse_Decode(OpcUa_HistoryReadResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryReadResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_HistoryReadResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_HistoryReadResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_HistoryReadResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryReadResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryReadResponse_EncodeableType =
{
    "HistoryReadResponse",
    OpcUaId_HistoryReadResponse,
    OpcUaId_HistoryReadResponse_Encoding_DefaultBinary,
    OpcUaId_HistoryReadResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryReadResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryReadResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryReadResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryReadResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryReadResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryReadResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_WriteValue
/*============================================================================
 * OpcUa_WriteValue_Initialize
 *===========================================================================*/
void OpcUa_WriteValue_Initialize(OpcUa_WriteValue* a_pValue)
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
 * OpcUa_WriteValue_Clear
 *===========================================================================*/
void OpcUa_WriteValue_Clear(OpcUa_WriteValue* a_pValue)
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
 * OpcUa_WriteValue_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_WriteValue_Encode(OpcUa_WriteValue* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_WriteValue_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_WriteValue_Decode(OpcUa_WriteValue* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_WriteValue_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= UInt32_Read(&a_pValue->AttributeId, msgBuf);
    status &= String_Read(&a_pValue->IndexRange, msgBuf);
    status &= DataValue_Read(&a_pValue->Value, msgBuf);

    if(status != STATUS_OK){
        OpcUa_WriteValue_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_WriteValue_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_WriteValue_EncodeableType =
{
    "WriteValue",
    OpcUaId_WriteValue,
    OpcUaId_WriteValue_Encoding_DefaultBinary,
    OpcUaId_WriteValue_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_WriteValue),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_WriteValue_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_WriteValue_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_WriteValue_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_WriteValue_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_WriteValue_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Write
#ifndef OPCUA_EXCLUDE_WriteRequest
/*============================================================================
 * OpcUa_WriteRequest_Initialize
 *===========================================================================*/
void OpcUa_WriteRequest_Initialize(OpcUa_WriteRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfNodesToWrite, (void**) &a_pValue->NodesToWrite, 
                            sizeof(OpcUa_WriteValue), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_WriteValue_Initialize);
    }
}

/*============================================================================
 * OpcUa_WriteRequest_Clear
 *===========================================================================*/
void OpcUa_WriteRequest_Clear(OpcUa_WriteRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfNodesToWrite, (void**) &a_pValue->NodesToWrite, 
                       sizeof(OpcUa_WriteValue), (SOPC_EncodeableObject_PfnClear*) OpcUa_WriteValue_Clear);
    }
}

/*============================================================================
 * OpcUa_WriteRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_WriteRequest_Encode(OpcUa_WriteRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNodesToWrite, (void**) &a_pValue->NodesToWrite, 
                   sizeof(OpcUa_WriteValue), (SOPC_EncodeableObject_PfnEncode*) OpcUa_WriteValue_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_WriteRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_WriteRequest_Decode(OpcUa_WriteRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_WriteRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNodesToWrite, (void**) &a_pValue->NodesToWrite, 
                  sizeof(OpcUa_WriteValue), (SOPC_EncodeableObject_PfnDecode*) OpcUa_WriteValue_Decode);

    if(status != STATUS_OK){
        OpcUa_WriteRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_WriteRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_WriteRequest_EncodeableType =
{
    "WriteRequest",
    OpcUaId_WriteRequest,
    OpcUaId_WriteRequest_Encoding_DefaultBinary,
    OpcUaId_WriteRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_WriteRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_WriteRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_WriteRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_WriteRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_WriteRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_WriteRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_WriteResponse
/*============================================================================
 * OpcUa_WriteResponse_Initialize
 *===========================================================================*/
void OpcUa_WriteResponse_Initialize(OpcUa_WriteResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_WriteResponse_Clear
 *===========================================================================*/
void OpcUa_WriteResponse_Clear(OpcUa_WriteResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_WriteResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_WriteResponse_Encode(OpcUa_WriteResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_WriteResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_WriteResponse_Decode(OpcUa_WriteResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_WriteResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_WriteResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_WriteResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_WriteResponse_EncodeableType =
{
    "WriteResponse",
    OpcUaId_WriteResponse,
    OpcUaId_WriteResponse_Encoding_DefaultBinary,
    OpcUaId_WriteResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_WriteResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_WriteResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_WriteResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_WriteResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_WriteResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_WriteResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateDetails
/*============================================================================
 * OpcUa_HistoryUpdateDetails_Initialize
 *===========================================================================*/
void OpcUa_HistoryUpdateDetails_Initialize(OpcUa_HistoryUpdateDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
    }
}

/*============================================================================
 * OpcUa_HistoryUpdateDetails_Clear
 *===========================================================================*/
void OpcUa_HistoryUpdateDetails_Clear(OpcUa_HistoryUpdateDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
    }
}

/*============================================================================
 * OpcUa_HistoryUpdateDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryUpdateDetails_Encode(OpcUa_HistoryUpdateDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_HistoryUpdateDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryUpdateDetails_Decode(OpcUa_HistoryUpdateDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryUpdateDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);

    if(status != STATUS_OK){
        OpcUa_HistoryUpdateDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryUpdateDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryUpdateDetails_EncodeableType =
{
    "HistoryUpdateDetails",
    OpcUaId_HistoryUpdateDetails,
    OpcUaId_HistoryUpdateDetails_Encoding_DefaultBinary,
    OpcUaId_HistoryUpdateDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryUpdateDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryUpdateDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryUpdateDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryUpdateDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryUpdateDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryUpdateDetails_Decode
};
#endif



#ifndef OPCUA_EXCLUDE_UpdateDataDetails
/*============================================================================
 * OpcUa_UpdateDataDetails_Initialize
 *===========================================================================*/
void OpcUa_UpdateDataDetails_Initialize(OpcUa_UpdateDataDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        SOPC_Initialize_Array(&a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                            sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnInitialize*) DataValue_Initialize);
    }
}

/*============================================================================
 * OpcUa_UpdateDataDetails_Clear
 *===========================================================================*/
void OpcUa_UpdateDataDetails_Clear(OpcUa_UpdateDataDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        SOPC_Clear_Array(&a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                       sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnClear*) DataValue_Clear);
    }
}

/*============================================================================
 * OpcUa_UpdateDataDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UpdateDataDetails_Encode(OpcUa_UpdateDataDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                   sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnEncode*) DataValue_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_UpdateDataDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UpdateDataDetails_Decode(OpcUa_UpdateDataDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_UpdateDataDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                  sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnDecode*) DataValue_Read);

    if(status != STATUS_OK){
        OpcUa_UpdateDataDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_UpdateDataDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_UpdateDataDetails_EncodeableType =
{
    "UpdateDataDetails",
    OpcUaId_UpdateDataDetails,
    OpcUaId_UpdateDataDetails_Encoding_DefaultBinary,
    OpcUaId_UpdateDataDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_UpdateDataDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_UpdateDataDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_UpdateDataDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_UpdateDataDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_UpdateDataDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_UpdateDataDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_UpdateStructureDataDetails
/*============================================================================
 * OpcUa_UpdateStructureDataDetails_Initialize
 *===========================================================================*/
void OpcUa_UpdateStructureDataDetails_Initialize(OpcUa_UpdateStructureDataDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        SOPC_Initialize_Array(&a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                            sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnInitialize*) DataValue_Initialize);
    }
}

/*============================================================================
 * OpcUa_UpdateStructureDataDetails_Clear
 *===========================================================================*/
void OpcUa_UpdateStructureDataDetails_Clear(OpcUa_UpdateStructureDataDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        SOPC_Clear_Array(&a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                       sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnClear*) DataValue_Clear);
    }
}

/*============================================================================
 * OpcUa_UpdateStructureDataDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UpdateStructureDataDetails_Encode(OpcUa_UpdateStructureDataDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                   sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnEncode*) DataValue_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_UpdateStructureDataDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UpdateStructureDataDetails_Decode(OpcUa_UpdateStructureDataDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_UpdateStructureDataDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfUpdateValues, (void**) &a_pValue->UpdateValues, 
                  sizeof(SOPC_DataValue), (SOPC_EncodeableObject_PfnDecode*) DataValue_Read);

    if(status != STATUS_OK){
        OpcUa_UpdateStructureDataDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_UpdateStructureDataDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_UpdateStructureDataDetails_EncodeableType =
{
    "UpdateStructureDataDetails",
    OpcUaId_UpdateStructureDataDetails,
    OpcUaId_UpdateStructureDataDetails_Encoding_DefaultBinary,
    OpcUaId_UpdateStructureDataDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_UpdateStructureDataDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_UpdateStructureDataDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_UpdateStructureDataDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_UpdateStructureDataDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_UpdateStructureDataDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_UpdateStructureDataDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_UpdateEventDetails
/*============================================================================
 * OpcUa_UpdateEventDetails_Initialize
 *===========================================================================*/
void OpcUa_UpdateEventDetails_Initialize(OpcUa_UpdateEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        OpcUa_EventFilter_Initialize(&a_pValue->Filter);
        SOPC_Initialize_Array(&a_pValue->NoOfEventData, (void**) &a_pValue->EventData, 
                            sizeof(OpcUa_HistoryEventFieldList), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_HistoryEventFieldList_Initialize);
    }
}

/*============================================================================
 * OpcUa_UpdateEventDetails_Clear
 *===========================================================================*/
void OpcUa_UpdateEventDetails_Clear(OpcUa_UpdateEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->PerformInsertReplace);
        OpcUa_EventFilter_Clear(&a_pValue->Filter);
        SOPC_Clear_Array(&a_pValue->NoOfEventData, (void**) &a_pValue->EventData, 
                       sizeof(OpcUa_HistoryEventFieldList), (SOPC_EncodeableObject_PfnClear*) OpcUa_HistoryEventFieldList_Clear);
    }
}

/*============================================================================
 * OpcUa_UpdateEventDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UpdateEventDetails_Encode(OpcUa_UpdateEventDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= OpcUa_EventFilter_Encode(&a_pValue->Filter, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfEventData, (void**) &a_pValue->EventData, 
                   sizeof(OpcUa_HistoryEventFieldList), (SOPC_EncodeableObject_PfnEncode*) OpcUa_HistoryEventFieldList_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_UpdateEventDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_UpdateEventDetails_Decode(OpcUa_UpdateEventDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_UpdateEventDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->PerformInsertReplace);
    status &= OpcUa_EventFilter_Decode(&a_pValue->Filter, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfEventData, (void**) &a_pValue->EventData, 
                  sizeof(OpcUa_HistoryEventFieldList), (SOPC_EncodeableObject_PfnDecode*) OpcUa_HistoryEventFieldList_Decode);

    if(status != STATUS_OK){
        OpcUa_UpdateEventDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_UpdateEventDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_UpdateEventDetails_EncodeableType =
{
    "UpdateEventDetails",
    OpcUaId_UpdateEventDetails,
    OpcUaId_UpdateEventDetails_Encoding_DefaultBinary,
    OpcUaId_UpdateEventDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_UpdateEventDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_UpdateEventDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_UpdateEventDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_UpdateEventDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_UpdateEventDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_UpdateEventDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteRawModifiedDetails
/*============================================================================
 * OpcUa_DeleteRawModifiedDetails_Initialize
 *===========================================================================*/
void OpcUa_DeleteRawModifiedDetails_Initialize(OpcUa_DeleteRawModifiedDetails* a_pValue)
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
 * OpcUa_DeleteRawModifiedDetails_Clear
 *===========================================================================*/
void OpcUa_DeleteRawModifiedDetails_Clear(OpcUa_DeleteRawModifiedDetails* a_pValue)
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
 * OpcUa_DeleteRawModifiedDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteRawModifiedDetails_Encode(OpcUa_DeleteRawModifiedDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_DeleteRawModifiedDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteRawModifiedDetails_Decode(OpcUa_DeleteRawModifiedDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteRawModifiedDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= Boolean_Read(&a_pValue->IsDeleteModified, msgBuf);
    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->EndTime, msgBuf);

    if(status != STATUS_OK){
        OpcUa_DeleteRawModifiedDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteRawModifiedDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteRawModifiedDetails_EncodeableType =
{
    "DeleteRawModifiedDetails",
    OpcUaId_DeleteRawModifiedDetails,
    OpcUaId_DeleteRawModifiedDetails_Encoding_DefaultBinary,
    OpcUaId_DeleteRawModifiedDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteRawModifiedDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteRawModifiedDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteRawModifiedDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteRawModifiedDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteRawModifiedDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteRawModifiedDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteAtTimeDetails
/*============================================================================
 * OpcUa_DeleteAtTimeDetails_Initialize
 *===========================================================================*/
void OpcUa_DeleteAtTimeDetails_Initialize(OpcUa_DeleteAtTimeDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_Array(&a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                            sizeof(SOPC_DateTime), (SOPC_EncodeableObject_PfnInitialize*) DateTime_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteAtTimeDetails_Clear
 *===========================================================================*/
void OpcUa_DeleteAtTimeDetails_Clear(OpcUa_DeleteAtTimeDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_Array(&a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                       sizeof(SOPC_DateTime), (SOPC_EncodeableObject_PfnClear*) DateTime_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteAtTimeDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteAtTimeDetails_Encode(OpcUa_DeleteAtTimeDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                   sizeof(SOPC_DateTime), (SOPC_EncodeableObject_PfnEncode*) DateTime_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteAtTimeDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteAtTimeDetails_Decode(OpcUa_DeleteAtTimeDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteAtTimeDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfReqTimes, (void**) &a_pValue->ReqTimes, 
                  sizeof(SOPC_DateTime), (SOPC_EncodeableObject_PfnDecode*) DateTime_Read);

    if(status != STATUS_OK){
        OpcUa_DeleteAtTimeDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteAtTimeDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteAtTimeDetails_EncodeableType =
{
    "DeleteAtTimeDetails",
    OpcUaId_DeleteAtTimeDetails,
    OpcUaId_DeleteAtTimeDetails_Encoding_DefaultBinary,
    OpcUaId_DeleteAtTimeDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteAtTimeDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteAtTimeDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteAtTimeDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteAtTimeDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteAtTimeDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteAtTimeDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteEventDetails
/*============================================================================
 * OpcUa_DeleteEventDetails_Initialize
 *===========================================================================*/
void OpcUa_DeleteEventDetails_Initialize(OpcUa_DeleteEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->NodeId);
        SOPC_Initialize_Array(&a_pValue->NoOfEventIds, (void**) &a_pValue->EventIds, 
                            sizeof(SOPC_ByteString), (SOPC_EncodeableObject_PfnInitialize*) ByteString_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteEventDetails_Clear
 *===========================================================================*/
void OpcUa_DeleteEventDetails_Clear(OpcUa_DeleteEventDetails* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->NodeId);
        SOPC_Clear_Array(&a_pValue->NoOfEventIds, (void**) &a_pValue->EventIds, 
                       sizeof(SOPC_ByteString), (SOPC_EncodeableObject_PfnClear*) ByteString_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteEventDetails_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteEventDetails_Encode(OpcUa_DeleteEventDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfEventIds, (void**) &a_pValue->EventIds, 
                   sizeof(SOPC_ByteString), (SOPC_EncodeableObject_PfnEncode*) ByteString_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteEventDetails_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteEventDetails_Decode(OpcUa_DeleteEventDetails* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteEventDetails_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->NodeId, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfEventIds, (void**) &a_pValue->EventIds, 
                  sizeof(SOPC_ByteString), (SOPC_EncodeableObject_PfnDecode*) ByteString_Read);

    if(status != STATUS_OK){
        OpcUa_DeleteEventDetails_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteEventDetails_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteEventDetails_EncodeableType =
{
    "DeleteEventDetails",
    OpcUaId_DeleteEventDetails,
    OpcUaId_DeleteEventDetails_Encoding_DefaultBinary,
    OpcUaId_DeleteEventDetails_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteEventDetails),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteEventDetails_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteEventDetails_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteEventDetails_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteEventDetails_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteEventDetails_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateResult
/*============================================================================
 * OpcUa_HistoryUpdateResult_Initialize
 *===========================================================================*/
void OpcUa_HistoryUpdateResult_Initialize(OpcUa_HistoryUpdateResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        SOPC_Initialize_Array(&a_pValue->NoOfOperationResults, (void**) &a_pValue->OperationResults, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_HistoryUpdateResult_Clear
 *===========================================================================*/
void OpcUa_HistoryUpdateResult_Clear(OpcUa_HistoryUpdateResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        SOPC_Clear_Array(&a_pValue->NoOfOperationResults, (void**) &a_pValue->OperationResults, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_HistoryUpdateResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryUpdateResult_Encode(OpcUa_HistoryUpdateResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfOperationResults, (void**) &a_pValue->OperationResults, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryUpdateResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryUpdateResult_Decode(OpcUa_HistoryUpdateResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryUpdateResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfOperationResults, (void**) &a_pValue->OperationResults, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_HistoryUpdateResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryUpdateResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryUpdateResult_EncodeableType =
{
    "HistoryUpdateResult",
    OpcUaId_HistoryUpdateResult,
    OpcUaId_HistoryUpdateResult_Encoding_DefaultBinary,
    OpcUaId_HistoryUpdateResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryUpdateResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryUpdateResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryUpdateResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryUpdateResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryUpdateResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryUpdateResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdate
#ifndef OPCUA_EXCLUDE_HistoryUpdateRequest
/*============================================================================
 * OpcUa_HistoryUpdateRequest_Initialize
 *===========================================================================*/
void OpcUa_HistoryUpdateRequest_Initialize(OpcUa_HistoryUpdateRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfHistoryUpdateDetails, (void**) &a_pValue->HistoryUpdateDetails, 
                            sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnInitialize*) ExtensionObject_Initialize);
    }
}

/*============================================================================
 * OpcUa_HistoryUpdateRequest_Clear
 *===========================================================================*/
void OpcUa_HistoryUpdateRequest_Clear(OpcUa_HistoryUpdateRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfHistoryUpdateDetails, (void**) &a_pValue->HistoryUpdateDetails, 
                       sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnClear*) ExtensionObject_Clear);
    }
}

/*============================================================================
 * OpcUa_HistoryUpdateRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryUpdateRequest_Encode(OpcUa_HistoryUpdateRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfHistoryUpdateDetails, (void**) &a_pValue->HistoryUpdateDetails, 
                   sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnEncode*) ExtensionObject_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryUpdateRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryUpdateRequest_Decode(OpcUa_HistoryUpdateRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryUpdateRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfHistoryUpdateDetails, (void**) &a_pValue->HistoryUpdateDetails, 
                  sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnDecode*) ExtensionObject_Read);

    if(status != STATUS_OK){
        OpcUa_HistoryUpdateRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryUpdateRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryUpdateRequest_EncodeableType =
{
    "HistoryUpdateRequest",
    OpcUaId_HistoryUpdateRequest,
    OpcUaId_HistoryUpdateRequest_Encoding_DefaultBinary,
    OpcUaId_HistoryUpdateRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryUpdateRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryUpdateRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryUpdateRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryUpdateRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryUpdateRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryUpdateRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryUpdateResponse
/*============================================================================
 * OpcUa_HistoryUpdateResponse_Initialize
 *===========================================================================*/
void OpcUa_HistoryUpdateResponse_Initialize(OpcUa_HistoryUpdateResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_HistoryUpdateResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_HistoryUpdateResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_HistoryUpdateResponse_Clear
 *===========================================================================*/
void OpcUa_HistoryUpdateResponse_Clear(OpcUa_HistoryUpdateResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_HistoryUpdateResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_HistoryUpdateResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_HistoryUpdateResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryUpdateResponse_Encode(OpcUa_HistoryUpdateResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_HistoryUpdateResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_HistoryUpdateResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryUpdateResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryUpdateResponse_Decode(OpcUa_HistoryUpdateResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryUpdateResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_HistoryUpdateResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_HistoryUpdateResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_HistoryUpdateResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryUpdateResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryUpdateResponse_EncodeableType =
{
    "HistoryUpdateResponse",
    OpcUaId_HistoryUpdateResponse,
    OpcUaId_HistoryUpdateResponse_Encoding_DefaultBinary,
    OpcUaId_HistoryUpdateResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryUpdateResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryUpdateResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryUpdateResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryUpdateResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryUpdateResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryUpdateResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_CallMethodRequest
/*============================================================================
 * OpcUa_CallMethodRequest_Initialize
 *===========================================================================*/
void OpcUa_CallMethodRequest_Initialize(OpcUa_CallMethodRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->ObjectId);
        NodeId_Initialize(&a_pValue->MethodId);
        SOPC_Initialize_Array(&a_pValue->NoOfInputArguments, (void**) &a_pValue->InputArguments, 
                            sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * OpcUa_CallMethodRequest_Clear
 *===========================================================================*/
void OpcUa_CallMethodRequest_Clear(OpcUa_CallMethodRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->ObjectId);
        NodeId_Clear(&a_pValue->MethodId);
        SOPC_Clear_Array(&a_pValue->NoOfInputArguments, (void**) &a_pValue->InputArguments, 
                       sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * OpcUa_CallMethodRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CallMethodRequest_Encode(OpcUa_CallMethodRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->ObjectId, msgBuf);
    status &= NodeId_Write(&a_pValue->MethodId, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfInputArguments, (void**) &a_pValue->InputArguments, 
                   sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CallMethodRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CallMethodRequest_Decode(OpcUa_CallMethodRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CallMethodRequest_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->ObjectId, msgBuf);
    status &= NodeId_Read(&a_pValue->MethodId, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfInputArguments, (void**) &a_pValue->InputArguments, 
                  sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        OpcUa_CallMethodRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CallMethodRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CallMethodRequest_EncodeableType =
{
    "CallMethodRequest",
    OpcUaId_CallMethodRequest,
    OpcUaId_CallMethodRequest_Encoding_DefaultBinary,
    OpcUaId_CallMethodRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CallMethodRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CallMethodRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CallMethodRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CallMethodRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CallMethodRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CallMethodRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CallMethodResult
/*============================================================================
 * OpcUa_CallMethodResult_Initialize
 *===========================================================================*/
void OpcUa_CallMethodResult_Initialize(OpcUa_CallMethodResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        SOPC_Initialize_Array(&a_pValue->NoOfInputArgumentResults, (void**) &a_pValue->InputArgumentResults, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfInputArgumentDiagnosticInfos, (void**) &a_pValue->InputArgumentDiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfOutputArguments, (void**) &a_pValue->OutputArguments, 
                            sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * OpcUa_CallMethodResult_Clear
 *===========================================================================*/
void OpcUa_CallMethodResult_Clear(OpcUa_CallMethodResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        SOPC_Clear_Array(&a_pValue->NoOfInputArgumentResults, (void**) &a_pValue->InputArgumentResults, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfInputArgumentDiagnosticInfos, (void**) &a_pValue->InputArgumentDiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfOutputArguments, (void**) &a_pValue->OutputArguments, 
                       sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * OpcUa_CallMethodResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CallMethodResult_Encode(OpcUa_CallMethodResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfInputArgumentResults, (void**) &a_pValue->InputArgumentResults, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfInputArgumentDiagnosticInfos, (void**) &a_pValue->InputArgumentDiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfOutputArguments, (void**) &a_pValue->OutputArguments, 
                   sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CallMethodResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CallMethodResult_Decode(OpcUa_CallMethodResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CallMethodResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfInputArgumentResults, (void**) &a_pValue->InputArgumentResults, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfInputArgumentDiagnosticInfos, (void**) &a_pValue->InputArgumentDiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfOutputArguments, (void**) &a_pValue->OutputArguments, 
                  sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        OpcUa_CallMethodResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CallMethodResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CallMethodResult_EncodeableType =
{
    "CallMethodResult",
    OpcUaId_CallMethodResult,
    OpcUaId_CallMethodResult_Encoding_DefaultBinary,
    OpcUaId_CallMethodResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CallMethodResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CallMethodResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CallMethodResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CallMethodResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CallMethodResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CallMethodResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Call
#ifndef OPCUA_EXCLUDE_CallRequest
/*============================================================================
 * OpcUa_CallRequest_Initialize
 *===========================================================================*/
void OpcUa_CallRequest_Initialize(OpcUa_CallRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfMethodsToCall, (void**) &a_pValue->MethodsToCall, 
                            sizeof(OpcUa_CallMethodRequest), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_CallMethodRequest_Initialize);
    }
}

/*============================================================================
 * OpcUa_CallRequest_Clear
 *===========================================================================*/
void OpcUa_CallRequest_Clear(OpcUa_CallRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfMethodsToCall, (void**) &a_pValue->MethodsToCall, 
                       sizeof(OpcUa_CallMethodRequest), (SOPC_EncodeableObject_PfnClear*) OpcUa_CallMethodRequest_Clear);
    }
}

/*============================================================================
 * OpcUa_CallRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CallRequest_Encode(OpcUa_CallRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfMethodsToCall, (void**) &a_pValue->MethodsToCall, 
                   sizeof(OpcUa_CallMethodRequest), (SOPC_EncodeableObject_PfnEncode*) OpcUa_CallMethodRequest_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CallRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CallRequest_Decode(OpcUa_CallRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CallRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfMethodsToCall, (void**) &a_pValue->MethodsToCall, 
                  sizeof(OpcUa_CallMethodRequest), (SOPC_EncodeableObject_PfnDecode*) OpcUa_CallMethodRequest_Decode);

    if(status != STATUS_OK){
        OpcUa_CallRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CallRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CallRequest_EncodeableType =
{
    "CallRequest",
    OpcUaId_CallRequest,
    OpcUaId_CallRequest_Encoding_DefaultBinary,
    OpcUaId_CallRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CallRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CallRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CallRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CallRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CallRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CallRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CallResponse
/*============================================================================
 * OpcUa_CallResponse_Initialize
 *===========================================================================*/
void OpcUa_CallResponse_Initialize(OpcUa_CallResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_CallMethodResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_CallMethodResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_CallResponse_Clear
 *===========================================================================*/
void OpcUa_CallResponse_Clear(OpcUa_CallResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_CallMethodResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_CallMethodResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_CallResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CallResponse_Encode(OpcUa_CallResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_CallMethodResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_CallMethodResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CallResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CallResponse_Decode(OpcUa_CallResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CallResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_CallMethodResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_CallMethodResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_CallResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CallResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CallResponse_EncodeableType =
{
    "CallResponse",
    OpcUaId_CallResponse,
    OpcUaId_CallResponse_Encoding_DefaultBinary,
    OpcUaId_CallResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CallResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CallResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CallResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CallResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CallResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CallResponse_Decode
};
#endif
#endif




#ifndef OPCUA_EXCLUDE_DataChangeFilter
/*============================================================================
 * OpcUa_DataChangeFilter_Initialize
 *===========================================================================*/
void OpcUa_DataChangeFilter_Initialize(OpcUa_DataChangeFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->Trigger);
        UInt32_Initialize(&a_pValue->DeadbandType);
        Double_Initialize(&a_pValue->DeadbandValue);
    }
}

/*============================================================================
 * OpcUa_DataChangeFilter_Clear
 *===========================================================================*/
void OpcUa_DataChangeFilter_Clear(OpcUa_DataChangeFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->Trigger);
        UInt32_Clear(&a_pValue->DeadbandType);
        Double_Clear(&a_pValue->DeadbandValue);
    }
}

/*============================================================================
 * OpcUa_DataChangeFilter_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DataChangeFilter_Encode(OpcUa_DataChangeFilter* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->Trigger);
    status &= UInt32_Write(&a_pValue->DeadbandType, msgBuf);
    status &= Double_Write(&a_pValue->DeadbandValue, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DataChangeFilter_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DataChangeFilter_Decode(OpcUa_DataChangeFilter* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DataChangeFilter_Initialize(a_pValue);

    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->Trigger);
    status &= UInt32_Read(&a_pValue->DeadbandType, msgBuf);
    status &= Double_Read(&a_pValue->DeadbandValue, msgBuf);

    if(status != STATUS_OK){
        OpcUa_DataChangeFilter_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DataChangeFilter_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DataChangeFilter_EncodeableType =
{
    "DataChangeFilter",
    OpcUaId_DataChangeFilter,
    OpcUaId_DataChangeFilter_Encoding_DefaultBinary,
    OpcUaId_DataChangeFilter_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DataChangeFilter),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DataChangeFilter_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DataChangeFilter_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DataChangeFilter_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DataChangeFilter_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DataChangeFilter_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EventFilter
/*============================================================================
 * OpcUa_EventFilter_Initialize
 *===========================================================================*/
void OpcUa_EventFilter_Initialize(OpcUa_EventFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfSelectClauses, (void**) &a_pValue->SelectClauses, 
                            sizeof(OpcUa_SimpleAttributeOperand), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_SimpleAttributeOperand_Initialize);
        OpcUa_ContentFilter_Initialize(&a_pValue->WhereClause);
    }
}

/*============================================================================
 * OpcUa_EventFilter_Clear
 *===========================================================================*/
void OpcUa_EventFilter_Clear(OpcUa_EventFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfSelectClauses, (void**) &a_pValue->SelectClauses, 
                       sizeof(OpcUa_SimpleAttributeOperand), (SOPC_EncodeableObject_PfnClear*) OpcUa_SimpleAttributeOperand_Clear);
        OpcUa_ContentFilter_Clear(&a_pValue->WhereClause);
    }
}

/*============================================================================
 * OpcUa_EventFilter_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EventFilter_Encode(OpcUa_EventFilter* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfSelectClauses, (void**) &a_pValue->SelectClauses, 
                   sizeof(OpcUa_SimpleAttributeOperand), (SOPC_EncodeableObject_PfnEncode*) OpcUa_SimpleAttributeOperand_Encode);
    status &= OpcUa_ContentFilter_Encode(&a_pValue->WhereClause, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_EventFilter_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EventFilter_Decode(OpcUa_EventFilter* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EventFilter_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfSelectClauses, (void**) &a_pValue->SelectClauses, 
                  sizeof(OpcUa_SimpleAttributeOperand), (SOPC_EncodeableObject_PfnDecode*) OpcUa_SimpleAttributeOperand_Decode);
    status &= OpcUa_ContentFilter_Decode(&a_pValue->WhereClause, msgBuf);

    if(status != STATUS_OK){
        OpcUa_EventFilter_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EventFilter_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EventFilter_EncodeableType =
{
    "EventFilter",
    OpcUaId_EventFilter,
    OpcUaId_EventFilter_Encoding_DefaultBinary,
    OpcUaId_EventFilter_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EventFilter),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EventFilter_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EventFilter_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EventFilter_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EventFilter_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EventFilter_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AggregateConfiguration
/*============================================================================
 * OpcUa_AggregateConfiguration_Initialize
 *===========================================================================*/
void OpcUa_AggregateConfiguration_Initialize(OpcUa_AggregateConfiguration* a_pValue)
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
 * OpcUa_AggregateConfiguration_Clear
 *===========================================================================*/
void OpcUa_AggregateConfiguration_Clear(OpcUa_AggregateConfiguration* a_pValue)
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
 * OpcUa_AggregateConfiguration_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AggregateConfiguration_Encode(OpcUa_AggregateConfiguration* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_AggregateConfiguration_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AggregateConfiguration_Decode(OpcUa_AggregateConfiguration* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AggregateConfiguration_Initialize(a_pValue);

    status &= Boolean_Read(&a_pValue->UseServerCapabilitiesDefaults, msgBuf);
    status &= Boolean_Read(&a_pValue->TreatUncertainAsBad, msgBuf);
    status &= Byte_Read(&a_pValue->PercentDataBad, msgBuf);
    status &= Byte_Read(&a_pValue->PercentDataGood, msgBuf);
    status &= Boolean_Read(&a_pValue->UseSlopedExtrapolation, msgBuf);

    if(status != STATUS_OK){
        OpcUa_AggregateConfiguration_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AggregateConfiguration_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AggregateConfiguration_EncodeableType =
{
    "AggregateConfiguration",
    OpcUaId_AggregateConfiguration,
    OpcUaId_AggregateConfiguration_Encoding_DefaultBinary,
    OpcUaId_AggregateConfiguration_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AggregateConfiguration),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AggregateConfiguration_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AggregateConfiguration_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AggregateConfiguration_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AggregateConfiguration_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AggregateConfiguration_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilter
/*============================================================================
 * OpcUa_AggregateFilter_Initialize
 *===========================================================================*/
void OpcUa_AggregateFilter_Initialize(OpcUa_AggregateFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->StartTime);
        NodeId_Initialize(&a_pValue->AggregateType);
        Double_Initialize(&a_pValue->ProcessingInterval);
        OpcUa_AggregateConfiguration_Initialize(&a_pValue->AggregateConfiguration);
    }
}

/*============================================================================
 * OpcUa_AggregateFilter_Clear
 *===========================================================================*/
void OpcUa_AggregateFilter_Clear(OpcUa_AggregateFilter* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->StartTime);
        NodeId_Clear(&a_pValue->AggregateType);
        Double_Clear(&a_pValue->ProcessingInterval);
        OpcUa_AggregateConfiguration_Clear(&a_pValue->AggregateConfiguration);
    }
}

/*============================================================================
 * OpcUa_AggregateFilter_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AggregateFilter_Encode(OpcUa_AggregateFilter* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= NodeId_Write(&a_pValue->AggregateType, msgBuf);
    status &= Double_Write(&a_pValue->ProcessingInterval, msgBuf);
    status &= OpcUa_AggregateConfiguration_Encode(&a_pValue->AggregateConfiguration, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AggregateFilter_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AggregateFilter_Decode(OpcUa_AggregateFilter* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AggregateFilter_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= NodeId_Read(&a_pValue->AggregateType, msgBuf);
    status &= Double_Read(&a_pValue->ProcessingInterval, msgBuf);
    status &= OpcUa_AggregateConfiguration_Decode(&a_pValue->AggregateConfiguration, msgBuf);

    if(status != STATUS_OK){
        OpcUa_AggregateFilter_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AggregateFilter_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AggregateFilter_EncodeableType =
{
    "AggregateFilter",
    OpcUaId_AggregateFilter,
    OpcUaId_AggregateFilter_Encoding_DefaultBinary,
    OpcUaId_AggregateFilter_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AggregateFilter),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AggregateFilter_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AggregateFilter_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AggregateFilter_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AggregateFilter_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AggregateFilter_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EventFilterResult
/*============================================================================
 * OpcUa_EventFilterResult_Initialize
 *===========================================================================*/
void OpcUa_EventFilterResult_Initialize(OpcUa_EventFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfSelectClauseResults, (void**) &a_pValue->SelectClauseResults, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfSelectClauseDiagnosticInfos, (void**) &a_pValue->SelectClauseDiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
        OpcUa_ContentFilterResult_Initialize(&a_pValue->WhereClauseResult);
    }
}

/*============================================================================
 * OpcUa_EventFilterResult_Clear
 *===========================================================================*/
void OpcUa_EventFilterResult_Clear(OpcUa_EventFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfSelectClauseResults, (void**) &a_pValue->SelectClauseResults, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfSelectClauseDiagnosticInfos, (void**) &a_pValue->SelectClauseDiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
        OpcUa_ContentFilterResult_Clear(&a_pValue->WhereClauseResult);
    }
}

/*============================================================================
 * OpcUa_EventFilterResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EventFilterResult_Encode(OpcUa_EventFilterResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfSelectClauseResults, (void**) &a_pValue->SelectClauseResults, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfSelectClauseDiagnosticInfos, (void**) &a_pValue->SelectClauseDiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);
    status &= OpcUa_ContentFilterResult_Encode(&a_pValue->WhereClauseResult, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_EventFilterResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EventFilterResult_Decode(OpcUa_EventFilterResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EventFilterResult_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfSelectClauseResults, (void**) &a_pValue->SelectClauseResults, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfSelectClauseDiagnosticInfos, (void**) &a_pValue->SelectClauseDiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);
    status &= OpcUa_ContentFilterResult_Decode(&a_pValue->WhereClauseResult, msgBuf);

    if(status != STATUS_OK){
        OpcUa_EventFilterResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EventFilterResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EventFilterResult_EncodeableType =
{
    "EventFilterResult",
    OpcUaId_EventFilterResult,
    OpcUaId_EventFilterResult_Encoding_DefaultBinary,
    OpcUaId_EventFilterResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EventFilterResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EventFilterResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EventFilterResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EventFilterResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EventFilterResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EventFilterResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AggregateFilterResult
/*============================================================================
 * OpcUa_AggregateFilterResult_Initialize
 *===========================================================================*/
void OpcUa_AggregateFilterResult_Initialize(OpcUa_AggregateFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->RevisedStartTime);
        Double_Initialize(&a_pValue->RevisedProcessingInterval);
        OpcUa_AggregateConfiguration_Initialize(&a_pValue->RevisedAggregateConfiguration);
    }
}

/*============================================================================
 * OpcUa_AggregateFilterResult_Clear
 *===========================================================================*/
void OpcUa_AggregateFilterResult_Clear(OpcUa_AggregateFilterResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->RevisedStartTime);
        Double_Clear(&a_pValue->RevisedProcessingInterval);
        OpcUa_AggregateConfiguration_Clear(&a_pValue->RevisedAggregateConfiguration);
    }
}

/*============================================================================
 * OpcUa_AggregateFilterResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AggregateFilterResult_Encode(OpcUa_AggregateFilterResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->RevisedStartTime, msgBuf);
    status &= Double_Write(&a_pValue->RevisedProcessingInterval, msgBuf);
    status &= OpcUa_AggregateConfiguration_Encode(&a_pValue->RevisedAggregateConfiguration, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AggregateFilterResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AggregateFilterResult_Decode(OpcUa_AggregateFilterResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AggregateFilterResult_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->RevisedStartTime, msgBuf);
    status &= Double_Read(&a_pValue->RevisedProcessingInterval, msgBuf);
    status &= OpcUa_AggregateConfiguration_Decode(&a_pValue->RevisedAggregateConfiguration, msgBuf);

    if(status != STATUS_OK){
        OpcUa_AggregateFilterResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AggregateFilterResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AggregateFilterResult_EncodeableType =
{
    "AggregateFilterResult",
    OpcUaId_AggregateFilterResult,
    OpcUaId_AggregateFilterResult_Encoding_DefaultBinary,
    OpcUaId_AggregateFilterResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AggregateFilterResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AggregateFilterResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AggregateFilterResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AggregateFilterResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AggregateFilterResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AggregateFilterResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoringParameters
/*============================================================================
 * OpcUa_MonitoringParameters_Initialize
 *===========================================================================*/
void OpcUa_MonitoringParameters_Initialize(OpcUa_MonitoringParameters* a_pValue)
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
 * OpcUa_MonitoringParameters_Clear
 *===========================================================================*/
void OpcUa_MonitoringParameters_Clear(OpcUa_MonitoringParameters* a_pValue)
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
 * OpcUa_MonitoringParameters_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoringParameters_Encode(OpcUa_MonitoringParameters* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_MonitoringParameters_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoringParameters_Decode(OpcUa_MonitoringParameters* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_MonitoringParameters_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->ClientHandle, msgBuf);
    status &= Double_Read(&a_pValue->SamplingInterval, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->Filter, msgBuf);
    status &= UInt32_Read(&a_pValue->QueueSize, msgBuf);
    status &= Boolean_Read(&a_pValue->DiscardOldest, msgBuf);

    if(status != STATUS_OK){
        OpcUa_MonitoringParameters_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_MonitoringParameters_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_MonitoringParameters_EncodeableType =
{
    "MonitoringParameters",
    OpcUaId_MonitoringParameters,
    OpcUaId_MonitoringParameters_Encoding_DefaultBinary,
    OpcUaId_MonitoringParameters_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_MonitoringParameters),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_MonitoringParameters_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_MonitoringParameters_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_MonitoringParameters_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_MonitoringParameters_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_MonitoringParameters_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemCreateRequest
/*============================================================================
 * OpcUa_MonitoredItemCreateRequest_Initialize
 *===========================================================================*/
void OpcUa_MonitoredItemCreateRequest_Initialize(OpcUa_MonitoredItemCreateRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ReadValueId_Initialize(&a_pValue->ItemToMonitor);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->MonitoringMode);
        OpcUa_MonitoringParameters_Initialize(&a_pValue->RequestedParameters);
    }
}

/*============================================================================
 * OpcUa_MonitoredItemCreateRequest_Clear
 *===========================================================================*/
void OpcUa_MonitoredItemCreateRequest_Clear(OpcUa_MonitoredItemCreateRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ReadValueId_Clear(&a_pValue->ItemToMonitor);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->MonitoringMode);
        OpcUa_MonitoringParameters_Clear(&a_pValue->RequestedParameters);
    }
}

/*============================================================================
 * OpcUa_MonitoredItemCreateRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemCreateRequest_Encode(OpcUa_MonitoredItemCreateRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ReadValueId_Encode(&a_pValue->ItemToMonitor, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->MonitoringMode);
    status &= OpcUa_MonitoringParameters_Encode(&a_pValue->RequestedParameters, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_MonitoredItemCreateRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemCreateRequest_Decode(OpcUa_MonitoredItemCreateRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_MonitoredItemCreateRequest_Initialize(a_pValue);

    status &= OpcUa_ReadValueId_Decode(&a_pValue->ItemToMonitor, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->MonitoringMode);
    status &= OpcUa_MonitoringParameters_Decode(&a_pValue->RequestedParameters, msgBuf);

    if(status != STATUS_OK){
        OpcUa_MonitoredItemCreateRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_MonitoredItemCreateRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_MonitoredItemCreateRequest_EncodeableType =
{
    "MonitoredItemCreateRequest",
    OpcUaId_MonitoredItemCreateRequest,
    OpcUaId_MonitoredItemCreateRequest_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemCreateRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_MonitoredItemCreateRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_MonitoredItemCreateRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_MonitoredItemCreateRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_MonitoredItemCreateRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_MonitoredItemCreateRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_MonitoredItemCreateRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemCreateResult
/*============================================================================
 * OpcUa_MonitoredItemCreateResult_Initialize
 *===========================================================================*/
void OpcUa_MonitoredItemCreateResult_Initialize(OpcUa_MonitoredItemCreateResult* a_pValue)
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
 * OpcUa_MonitoredItemCreateResult_Clear
 *===========================================================================*/
void OpcUa_MonitoredItemCreateResult_Clear(OpcUa_MonitoredItemCreateResult* a_pValue)
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
 * OpcUa_MonitoredItemCreateResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemCreateResult_Encode(OpcUa_MonitoredItemCreateResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_MonitoredItemCreateResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemCreateResult_Decode(OpcUa_MonitoredItemCreateResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_MonitoredItemCreateResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= UInt32_Read(&a_pValue->MonitoredItemId, msgBuf);
    status &= Double_Read(&a_pValue->RevisedSamplingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedQueueSize, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->FilterResult, msgBuf);

    if(status != STATUS_OK){
        OpcUa_MonitoredItemCreateResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_MonitoredItemCreateResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_MonitoredItemCreateResult_EncodeableType =
{
    "MonitoredItemCreateResult",
    OpcUaId_MonitoredItemCreateResult,
    OpcUaId_MonitoredItemCreateResult_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemCreateResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_MonitoredItemCreateResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_MonitoredItemCreateResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_MonitoredItemCreateResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_MonitoredItemCreateResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_MonitoredItemCreateResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_MonitoredItemCreateResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItems
#ifndef OPCUA_EXCLUDE_CreateMonitoredItemsRequest
/*============================================================================
 * OpcUa_CreateMonitoredItemsRequest_Initialize
 *===========================================================================*/
void OpcUa_CreateMonitoredItemsRequest_Initialize(OpcUa_CreateMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        SOPC_Initialize_Array(&a_pValue->NoOfItemsToCreate, (void**) &a_pValue->ItemsToCreate, 
                            sizeof(OpcUa_MonitoredItemCreateRequest), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_MonitoredItemCreateRequest_Initialize);
    }
}

/*============================================================================
 * OpcUa_CreateMonitoredItemsRequest_Clear
 *===========================================================================*/
void OpcUa_CreateMonitoredItemsRequest_Clear(OpcUa_CreateMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        SOPC_Clear_Array(&a_pValue->NoOfItemsToCreate, (void**) &a_pValue->ItemsToCreate, 
                       sizeof(OpcUa_MonitoredItemCreateRequest), (SOPC_EncodeableObject_PfnClear*) OpcUa_MonitoredItemCreateRequest_Clear);
    }
}

/*============================================================================
 * OpcUa_CreateMonitoredItemsRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateMonitoredItemsRequest_Encode(OpcUa_CreateMonitoredItemsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfItemsToCreate, (void**) &a_pValue->ItemsToCreate, 
                   sizeof(OpcUa_MonitoredItemCreateRequest), (SOPC_EncodeableObject_PfnEncode*) OpcUa_MonitoredItemCreateRequest_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CreateMonitoredItemsRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateMonitoredItemsRequest_Decode(OpcUa_CreateMonitoredItemsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CreateMonitoredItemsRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfItemsToCreate, (void**) &a_pValue->ItemsToCreate, 
                  sizeof(OpcUa_MonitoredItemCreateRequest), (SOPC_EncodeableObject_PfnDecode*) OpcUa_MonitoredItemCreateRequest_Decode);

    if(status != STATUS_OK){
        OpcUa_CreateMonitoredItemsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CreateMonitoredItemsRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CreateMonitoredItemsRequest_EncodeableType =
{
    "CreateMonitoredItemsRequest",
    OpcUaId_CreateMonitoredItemsRequest,
    OpcUaId_CreateMonitoredItemsRequest_Encoding_DefaultBinary,
    OpcUaId_CreateMonitoredItemsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CreateMonitoredItemsRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CreateMonitoredItemsRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CreateMonitoredItemsRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CreateMonitoredItemsRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CreateMonitoredItemsRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CreateMonitoredItemsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateMonitoredItemsResponse
/*============================================================================
 * OpcUa_CreateMonitoredItemsResponse_Initialize
 *===========================================================================*/
void OpcUa_CreateMonitoredItemsResponse_Initialize(OpcUa_CreateMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_MonitoredItemCreateResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_MonitoredItemCreateResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_CreateMonitoredItemsResponse_Clear
 *===========================================================================*/
void OpcUa_CreateMonitoredItemsResponse_Clear(OpcUa_CreateMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_MonitoredItemCreateResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_MonitoredItemCreateResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_CreateMonitoredItemsResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateMonitoredItemsResponse_Encode(OpcUa_CreateMonitoredItemsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_MonitoredItemCreateResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_MonitoredItemCreateResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_CreateMonitoredItemsResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateMonitoredItemsResponse_Decode(OpcUa_CreateMonitoredItemsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CreateMonitoredItemsResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_MonitoredItemCreateResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_MonitoredItemCreateResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_CreateMonitoredItemsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CreateMonitoredItemsResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CreateMonitoredItemsResponse_EncodeableType =
{
    "CreateMonitoredItemsResponse",
    OpcUaId_CreateMonitoredItemsResponse,
    OpcUaId_CreateMonitoredItemsResponse_Encoding_DefaultBinary,
    OpcUaId_CreateMonitoredItemsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CreateMonitoredItemsResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CreateMonitoredItemsResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CreateMonitoredItemsResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CreateMonitoredItemsResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CreateMonitoredItemsResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CreateMonitoredItemsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemModifyRequest
/*============================================================================
 * OpcUa_MonitoredItemModifyRequest_Initialize
 *===========================================================================*/
void OpcUa_MonitoredItemModifyRequest_Initialize(OpcUa_MonitoredItemModifyRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->MonitoredItemId);
        OpcUa_MonitoringParameters_Initialize(&a_pValue->RequestedParameters);
    }
}

/*============================================================================
 * OpcUa_MonitoredItemModifyRequest_Clear
 *===========================================================================*/
void OpcUa_MonitoredItemModifyRequest_Clear(OpcUa_MonitoredItemModifyRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->MonitoredItemId);
        OpcUa_MonitoringParameters_Clear(&a_pValue->RequestedParameters);
    }
}

/*============================================================================
 * OpcUa_MonitoredItemModifyRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemModifyRequest_Encode(OpcUa_MonitoredItemModifyRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->MonitoredItemId, msgBuf);
    status &= OpcUa_MonitoringParameters_Encode(&a_pValue->RequestedParameters, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_MonitoredItemModifyRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemModifyRequest_Decode(OpcUa_MonitoredItemModifyRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_MonitoredItemModifyRequest_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->MonitoredItemId, msgBuf);
    status &= OpcUa_MonitoringParameters_Decode(&a_pValue->RequestedParameters, msgBuf);

    if(status != STATUS_OK){
        OpcUa_MonitoredItemModifyRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_MonitoredItemModifyRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_MonitoredItemModifyRequest_EncodeableType =
{
    "MonitoredItemModifyRequest",
    OpcUaId_MonitoredItemModifyRequest,
    OpcUaId_MonitoredItemModifyRequest_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemModifyRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_MonitoredItemModifyRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_MonitoredItemModifyRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_MonitoredItemModifyRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_MonitoredItemModifyRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_MonitoredItemModifyRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_MonitoredItemModifyRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemModifyResult
/*============================================================================
 * OpcUa_MonitoredItemModifyResult_Initialize
 *===========================================================================*/
void OpcUa_MonitoredItemModifyResult_Initialize(OpcUa_MonitoredItemModifyResult* a_pValue)
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
 * OpcUa_MonitoredItemModifyResult_Clear
 *===========================================================================*/
void OpcUa_MonitoredItemModifyResult_Clear(OpcUa_MonitoredItemModifyResult* a_pValue)
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
 * OpcUa_MonitoredItemModifyResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemModifyResult_Encode(OpcUa_MonitoredItemModifyResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_MonitoredItemModifyResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemModifyResult_Decode(OpcUa_MonitoredItemModifyResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_MonitoredItemModifyResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= Double_Read(&a_pValue->RevisedSamplingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedQueueSize, msgBuf);
    status &= ExtensionObject_Read(&a_pValue->FilterResult, msgBuf);

    if(status != STATUS_OK){
        OpcUa_MonitoredItemModifyResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_MonitoredItemModifyResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_MonitoredItemModifyResult_EncodeableType =
{
    "MonitoredItemModifyResult",
    OpcUaId_MonitoredItemModifyResult,
    OpcUaId_MonitoredItemModifyResult_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemModifyResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_MonitoredItemModifyResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_MonitoredItemModifyResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_MonitoredItemModifyResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_MonitoredItemModifyResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_MonitoredItemModifyResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_MonitoredItemModifyResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
#ifndef OPCUA_EXCLUDE_ModifyMonitoredItemsRequest
/*============================================================================
 * OpcUa_ModifyMonitoredItemsRequest_Initialize
 *===========================================================================*/
void OpcUa_ModifyMonitoredItemsRequest_Initialize(OpcUa_ModifyMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        SOPC_Initialize_Array(&a_pValue->NoOfItemsToModify, (void**) &a_pValue->ItemsToModify, 
                            sizeof(OpcUa_MonitoredItemModifyRequest), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_MonitoredItemModifyRequest_Initialize);
    }
}

/*============================================================================
 * OpcUa_ModifyMonitoredItemsRequest_Clear
 *===========================================================================*/
void OpcUa_ModifyMonitoredItemsRequest_Clear(OpcUa_ModifyMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->TimestampsToReturn);
        SOPC_Clear_Array(&a_pValue->NoOfItemsToModify, (void**) &a_pValue->ItemsToModify, 
                       sizeof(OpcUa_MonitoredItemModifyRequest), (SOPC_EncodeableObject_PfnClear*) OpcUa_MonitoredItemModifyRequest_Clear);
    }
}

/*============================================================================
 * OpcUa_ModifyMonitoredItemsRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModifyMonitoredItemsRequest_Encode(OpcUa_ModifyMonitoredItemsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfItemsToModify, (void**) &a_pValue->ItemsToModify, 
                   sizeof(OpcUa_MonitoredItemModifyRequest), (SOPC_EncodeableObject_PfnEncode*) OpcUa_MonitoredItemModifyRequest_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ModifyMonitoredItemsRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModifyMonitoredItemsRequest_Decode(OpcUa_ModifyMonitoredItemsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ModifyMonitoredItemsRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->TimestampsToReturn);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfItemsToModify, (void**) &a_pValue->ItemsToModify, 
                  sizeof(OpcUa_MonitoredItemModifyRequest), (SOPC_EncodeableObject_PfnDecode*) OpcUa_MonitoredItemModifyRequest_Decode);

    if(status != STATUS_OK){
        OpcUa_ModifyMonitoredItemsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ModifyMonitoredItemsRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ModifyMonitoredItemsRequest_EncodeableType =
{
    "ModifyMonitoredItemsRequest",
    OpcUaId_ModifyMonitoredItemsRequest,
    OpcUaId_ModifyMonitoredItemsRequest_Encoding_DefaultBinary,
    OpcUaId_ModifyMonitoredItemsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ModifyMonitoredItemsRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ModifyMonitoredItemsRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ModifyMonitoredItemsRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ModifyMonitoredItemsRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ModifyMonitoredItemsRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ModifyMonitoredItemsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ModifyMonitoredItemsResponse
/*============================================================================
 * OpcUa_ModifyMonitoredItemsResponse_Initialize
 *===========================================================================*/
void OpcUa_ModifyMonitoredItemsResponse_Initialize(OpcUa_ModifyMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_MonitoredItemModifyResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_MonitoredItemModifyResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_ModifyMonitoredItemsResponse_Clear
 *===========================================================================*/
void OpcUa_ModifyMonitoredItemsResponse_Clear(OpcUa_ModifyMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_MonitoredItemModifyResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_MonitoredItemModifyResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_ModifyMonitoredItemsResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModifyMonitoredItemsResponse_Encode(OpcUa_ModifyMonitoredItemsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_MonitoredItemModifyResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_MonitoredItemModifyResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ModifyMonitoredItemsResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModifyMonitoredItemsResponse_Decode(OpcUa_ModifyMonitoredItemsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ModifyMonitoredItemsResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_MonitoredItemModifyResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_MonitoredItemModifyResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_ModifyMonitoredItemsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ModifyMonitoredItemsResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ModifyMonitoredItemsResponse_EncodeableType =
{
    "ModifyMonitoredItemsResponse",
    OpcUaId_ModifyMonitoredItemsResponse,
    OpcUaId_ModifyMonitoredItemsResponse_Encoding_DefaultBinary,
    OpcUaId_ModifyMonitoredItemsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ModifyMonitoredItemsResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ModifyMonitoredItemsResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ModifyMonitoredItemsResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ModifyMonitoredItemsResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ModifyMonitoredItemsResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ModifyMonitoredItemsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringMode
#ifndef OPCUA_EXCLUDE_SetMonitoringModeRequest
/*============================================================================
 * OpcUa_SetMonitoringModeRequest_Initialize
 *===========================================================================*/
void OpcUa_SetMonitoringModeRequest_Initialize(OpcUa_SetMonitoringModeRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->MonitoringMode);
        SOPC_Initialize_Array(&a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * OpcUa_SetMonitoringModeRequest_Clear
 *===========================================================================*/
void OpcUa_SetMonitoringModeRequest_Clear(OpcUa_SetMonitoringModeRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->MonitoringMode);
        SOPC_Clear_Array(&a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * OpcUa_SetMonitoringModeRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetMonitoringModeRequest_Encode(OpcUa_SetMonitoringModeRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->MonitoringMode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SetMonitoringModeRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetMonitoringModeRequest_Decode(OpcUa_SetMonitoringModeRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SetMonitoringModeRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->MonitoringMode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        OpcUa_SetMonitoringModeRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SetMonitoringModeRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SetMonitoringModeRequest_EncodeableType =
{
    "SetMonitoringModeRequest",
    OpcUaId_SetMonitoringModeRequest,
    OpcUaId_SetMonitoringModeRequest_Encoding_DefaultBinary,
    OpcUaId_SetMonitoringModeRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SetMonitoringModeRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SetMonitoringModeRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SetMonitoringModeRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SetMonitoringModeRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SetMonitoringModeRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SetMonitoringModeRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SetMonitoringModeResponse
/*============================================================================
 * OpcUa_SetMonitoringModeResponse_Initialize
 *===========================================================================*/
void OpcUa_SetMonitoringModeResponse_Initialize(OpcUa_SetMonitoringModeResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_SetMonitoringModeResponse_Clear
 *===========================================================================*/
void OpcUa_SetMonitoringModeResponse_Clear(OpcUa_SetMonitoringModeResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_SetMonitoringModeResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetMonitoringModeResponse_Encode(OpcUa_SetMonitoringModeResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SetMonitoringModeResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetMonitoringModeResponse_Decode(OpcUa_SetMonitoringModeResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SetMonitoringModeResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_SetMonitoringModeResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SetMonitoringModeResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SetMonitoringModeResponse_EncodeableType =
{
    "SetMonitoringModeResponse",
    OpcUaId_SetMonitoringModeResponse,
    OpcUaId_SetMonitoringModeResponse_Encoding_DefaultBinary,
    OpcUaId_SetMonitoringModeResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SetMonitoringModeResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SetMonitoringModeResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SetMonitoringModeResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SetMonitoringModeResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SetMonitoringModeResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SetMonitoringModeResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetTriggering
#ifndef OPCUA_EXCLUDE_SetTriggeringRequest
/*============================================================================
 * OpcUa_SetTriggeringRequest_Initialize
 *===========================================================================*/
void OpcUa_SetTriggeringRequest_Initialize(OpcUa_SetTriggeringRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UInt32_Initialize(&a_pValue->TriggeringItemId);
        SOPC_Initialize_Array(&a_pValue->NoOfLinksToAdd, (void**) &a_pValue->LinksToAdd, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfLinksToRemove, (void**) &a_pValue->LinksToRemove, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * OpcUa_SetTriggeringRequest_Clear
 *===========================================================================*/
void OpcUa_SetTriggeringRequest_Clear(OpcUa_SetTriggeringRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        UInt32_Clear(&a_pValue->TriggeringItemId);
        SOPC_Clear_Array(&a_pValue->NoOfLinksToAdd, (void**) &a_pValue->LinksToAdd, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfLinksToRemove, (void**) &a_pValue->LinksToRemove, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * OpcUa_SetTriggeringRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetTriggeringRequest_Encode(OpcUa_SetTriggeringRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Write(&a_pValue->TriggeringItemId, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfLinksToAdd, (void**) &a_pValue->LinksToAdd, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfLinksToRemove, (void**) &a_pValue->LinksToRemove, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SetTriggeringRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetTriggeringRequest_Decode(OpcUa_SetTriggeringRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SetTriggeringRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Read(&a_pValue->TriggeringItemId, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfLinksToAdd, (void**) &a_pValue->LinksToAdd, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfLinksToRemove, (void**) &a_pValue->LinksToRemove, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        OpcUa_SetTriggeringRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SetTriggeringRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SetTriggeringRequest_EncodeableType =
{
    "SetTriggeringRequest",
    OpcUaId_SetTriggeringRequest,
    OpcUaId_SetTriggeringRequest_Encoding_DefaultBinary,
    OpcUaId_SetTriggeringRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SetTriggeringRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SetTriggeringRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SetTriggeringRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SetTriggeringRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SetTriggeringRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SetTriggeringRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SetTriggeringResponse
/*============================================================================
 * OpcUa_SetTriggeringResponse_Initialize
 *===========================================================================*/
void OpcUa_SetTriggeringResponse_Initialize(OpcUa_SetTriggeringResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfAddResults, (void**) &a_pValue->AddResults, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfAddDiagnosticInfos, (void**) &a_pValue->AddDiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfRemoveResults, (void**) &a_pValue->RemoveResults, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfRemoveDiagnosticInfos, (void**) &a_pValue->RemoveDiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_SetTriggeringResponse_Clear
 *===========================================================================*/
void OpcUa_SetTriggeringResponse_Clear(OpcUa_SetTriggeringResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfAddResults, (void**) &a_pValue->AddResults, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfAddDiagnosticInfos, (void**) &a_pValue->AddDiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfRemoveResults, (void**) &a_pValue->RemoveResults, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfRemoveDiagnosticInfos, (void**) &a_pValue->RemoveDiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_SetTriggeringResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetTriggeringResponse_Encode(OpcUa_SetTriggeringResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfAddResults, (void**) &a_pValue->AddResults, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfAddDiagnosticInfos, (void**) &a_pValue->AddDiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfRemoveResults, (void**) &a_pValue->RemoveResults, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfRemoveDiagnosticInfos, (void**) &a_pValue->RemoveDiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SetTriggeringResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetTriggeringResponse_Decode(OpcUa_SetTriggeringResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SetTriggeringResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfAddResults, (void**) &a_pValue->AddResults, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfAddDiagnosticInfos, (void**) &a_pValue->AddDiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfRemoveResults, (void**) &a_pValue->RemoveResults, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfRemoveDiagnosticInfos, (void**) &a_pValue->RemoveDiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_SetTriggeringResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SetTriggeringResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SetTriggeringResponse_EncodeableType =
{
    "SetTriggeringResponse",
    OpcUaId_SetTriggeringResponse,
    OpcUaId_SetTriggeringResponse_Encoding_DefaultBinary,
    OpcUaId_SetTriggeringResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SetTriggeringResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SetTriggeringResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SetTriggeringResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SetTriggeringResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SetTriggeringResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SetTriggeringResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
#ifndef OPCUA_EXCLUDE_DeleteMonitoredItemsRequest
/*============================================================================
 * OpcUa_DeleteMonitoredItemsRequest_Initialize
 *===========================================================================*/
void OpcUa_DeleteMonitoredItemsRequest_Initialize(OpcUa_DeleteMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        SOPC_Initialize_Array(&a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteMonitoredItemsRequest_Clear
 *===========================================================================*/
void OpcUa_DeleteMonitoredItemsRequest_Clear(OpcUa_DeleteMonitoredItemsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        SOPC_Clear_Array(&a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteMonitoredItemsRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteMonitoredItemsRequest_Encode(OpcUa_DeleteMonitoredItemsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteMonitoredItemsRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteMonitoredItemsRequest_Decode(OpcUa_DeleteMonitoredItemsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteMonitoredItemsRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfMonitoredItemIds, (void**) &a_pValue->MonitoredItemIds, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        OpcUa_DeleteMonitoredItemsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteMonitoredItemsRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteMonitoredItemsRequest_EncodeableType =
{
    "DeleteMonitoredItemsRequest",
    OpcUaId_DeleteMonitoredItemsRequest,
    OpcUaId_DeleteMonitoredItemsRequest_Encoding_DefaultBinary,
    OpcUaId_DeleteMonitoredItemsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteMonitoredItemsRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteMonitoredItemsRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteMonitoredItemsRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteMonitoredItemsRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteMonitoredItemsRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteMonitoredItemsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteMonitoredItemsResponse
/*============================================================================
 * OpcUa_DeleteMonitoredItemsResponse_Initialize
 *===========================================================================*/
void OpcUa_DeleteMonitoredItemsResponse_Initialize(OpcUa_DeleteMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteMonitoredItemsResponse_Clear
 *===========================================================================*/
void OpcUa_DeleteMonitoredItemsResponse_Clear(OpcUa_DeleteMonitoredItemsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteMonitoredItemsResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteMonitoredItemsResponse_Encode(OpcUa_DeleteMonitoredItemsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteMonitoredItemsResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteMonitoredItemsResponse_Decode(OpcUa_DeleteMonitoredItemsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteMonitoredItemsResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_DeleteMonitoredItemsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteMonitoredItemsResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteMonitoredItemsResponse_EncodeableType =
{
    "DeleteMonitoredItemsResponse",
    OpcUaId_DeleteMonitoredItemsResponse,
    OpcUaId_DeleteMonitoredItemsResponse_Encoding_DefaultBinary,
    OpcUaId_DeleteMonitoredItemsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteMonitoredItemsResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteMonitoredItemsResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteMonitoredItemsResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteMonitoredItemsResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteMonitoredItemsResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteMonitoredItemsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscription
#ifndef OPCUA_EXCLUDE_CreateSubscriptionRequest
/*============================================================================
 * OpcUa_CreateSubscriptionRequest_Initialize
 *===========================================================================*/
void OpcUa_CreateSubscriptionRequest_Initialize(OpcUa_CreateSubscriptionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Double_Initialize(&a_pValue->RequestedPublishingInterval);
        UInt32_Initialize(&a_pValue->RequestedLifetimeCount);
        UInt32_Initialize(&a_pValue->RequestedMaxKeepAliveCount);
        UInt32_Initialize(&a_pValue->MaxNotificationsPerPublish);
        Boolean_Initialize(&a_pValue->PublishingEnabled);
        Byte_Initialize(&a_pValue->Priority);
    }
}

/*============================================================================
 * OpcUa_CreateSubscriptionRequest_Clear
 *===========================================================================*/
void OpcUa_CreateSubscriptionRequest_Clear(OpcUa_CreateSubscriptionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        Double_Clear(&a_pValue->RequestedPublishingInterval);
        UInt32_Clear(&a_pValue->RequestedLifetimeCount);
        UInt32_Clear(&a_pValue->RequestedMaxKeepAliveCount);
        UInt32_Clear(&a_pValue->MaxNotificationsPerPublish);
        Boolean_Clear(&a_pValue->PublishingEnabled);
        Byte_Clear(&a_pValue->Priority);
    }
}

/*============================================================================
 * OpcUa_CreateSubscriptionRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateSubscriptionRequest_Encode(OpcUa_CreateSubscriptionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
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
 * OpcUa_CreateSubscriptionRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateSubscriptionRequest_Decode(OpcUa_CreateSubscriptionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CreateSubscriptionRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Double_Read(&a_pValue->RequestedPublishingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedMaxKeepAliveCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxNotificationsPerPublish, msgBuf);
    status &= Boolean_Read(&a_pValue->PublishingEnabled, msgBuf);
    status &= Byte_Read(&a_pValue->Priority, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CreateSubscriptionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CreateSubscriptionRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CreateSubscriptionRequest_EncodeableType =
{
    "CreateSubscriptionRequest",
    OpcUaId_CreateSubscriptionRequest,
    OpcUaId_CreateSubscriptionRequest_Encoding_DefaultBinary,
    OpcUaId_CreateSubscriptionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CreateSubscriptionRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CreateSubscriptionRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CreateSubscriptionRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CreateSubscriptionRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CreateSubscriptionRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CreateSubscriptionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_CreateSubscriptionResponse
/*============================================================================
 * OpcUa_CreateSubscriptionResponse_Initialize
 *===========================================================================*/
void OpcUa_CreateSubscriptionResponse_Initialize(OpcUa_CreateSubscriptionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        Double_Initialize(&a_pValue->RevisedPublishingInterval);
        UInt32_Initialize(&a_pValue->RevisedLifetimeCount);
        UInt32_Initialize(&a_pValue->RevisedMaxKeepAliveCount);
    }
}

/*============================================================================
 * OpcUa_CreateSubscriptionResponse_Clear
 *===========================================================================*/
void OpcUa_CreateSubscriptionResponse_Clear(OpcUa_CreateSubscriptionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        Double_Clear(&a_pValue->RevisedPublishingInterval);
        UInt32_Clear(&a_pValue->RevisedLifetimeCount);
        UInt32_Clear(&a_pValue->RevisedMaxKeepAliveCount);
    }
}

/*============================================================================
 * OpcUa_CreateSubscriptionResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateSubscriptionResponse_Encode(OpcUa_CreateSubscriptionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
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
 * OpcUa_CreateSubscriptionResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_CreateSubscriptionResponse_Decode(OpcUa_CreateSubscriptionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_CreateSubscriptionResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= Double_Read(&a_pValue->RevisedPublishingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedMaxKeepAliveCount, msgBuf);

    if(status != STATUS_OK){
        OpcUa_CreateSubscriptionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_CreateSubscriptionResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_CreateSubscriptionResponse_EncodeableType =
{
    "CreateSubscriptionResponse",
    OpcUaId_CreateSubscriptionResponse,
    OpcUaId_CreateSubscriptionResponse_Encoding_DefaultBinary,
    OpcUaId_CreateSubscriptionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_CreateSubscriptionResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_CreateSubscriptionResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_CreateSubscriptionResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_CreateSubscriptionResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_CreateSubscriptionResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_CreateSubscriptionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscription
#ifndef OPCUA_EXCLUDE_ModifySubscriptionRequest
/*============================================================================
 * OpcUa_ModifySubscriptionRequest_Initialize
 *===========================================================================*/
void OpcUa_ModifySubscriptionRequest_Initialize(OpcUa_ModifySubscriptionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        Double_Initialize(&a_pValue->RequestedPublishingInterval);
        UInt32_Initialize(&a_pValue->RequestedLifetimeCount);
        UInt32_Initialize(&a_pValue->RequestedMaxKeepAliveCount);
        UInt32_Initialize(&a_pValue->MaxNotificationsPerPublish);
        Byte_Initialize(&a_pValue->Priority);
    }
}

/*============================================================================
 * OpcUa_ModifySubscriptionRequest_Clear
 *===========================================================================*/
void OpcUa_ModifySubscriptionRequest_Clear(OpcUa_ModifySubscriptionRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        Double_Clear(&a_pValue->RequestedPublishingInterval);
        UInt32_Clear(&a_pValue->RequestedLifetimeCount);
        UInt32_Clear(&a_pValue->RequestedMaxKeepAliveCount);
        UInt32_Clear(&a_pValue->MaxNotificationsPerPublish);
        Byte_Clear(&a_pValue->Priority);
    }
}

/*============================================================================
 * OpcUa_ModifySubscriptionRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModifySubscriptionRequest_Encode(OpcUa_ModifySubscriptionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
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
 * OpcUa_ModifySubscriptionRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModifySubscriptionRequest_Decode(OpcUa_ModifySubscriptionRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ModifySubscriptionRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= Double_Read(&a_pValue->RequestedPublishingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RequestedMaxKeepAliveCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxNotificationsPerPublish, msgBuf);
    status &= Byte_Read(&a_pValue->Priority, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ModifySubscriptionRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ModifySubscriptionRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ModifySubscriptionRequest_EncodeableType =
{
    "ModifySubscriptionRequest",
    OpcUaId_ModifySubscriptionRequest,
    OpcUaId_ModifySubscriptionRequest_Encoding_DefaultBinary,
    OpcUaId_ModifySubscriptionRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ModifySubscriptionRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ModifySubscriptionRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ModifySubscriptionRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ModifySubscriptionRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ModifySubscriptionRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ModifySubscriptionRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ModifySubscriptionResponse
/*============================================================================
 * OpcUa_ModifySubscriptionResponse_Initialize
 *===========================================================================*/
void OpcUa_ModifySubscriptionResponse_Initialize(OpcUa_ModifySubscriptionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        Double_Initialize(&a_pValue->RevisedPublishingInterval);
        UInt32_Initialize(&a_pValue->RevisedLifetimeCount);
        UInt32_Initialize(&a_pValue->RevisedMaxKeepAliveCount);
    }
}

/*============================================================================
 * OpcUa_ModifySubscriptionResponse_Clear
 *===========================================================================*/
void OpcUa_ModifySubscriptionResponse_Clear(OpcUa_ModifySubscriptionResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        Double_Clear(&a_pValue->RevisedPublishingInterval);
        UInt32_Clear(&a_pValue->RevisedLifetimeCount);
        UInt32_Clear(&a_pValue->RevisedMaxKeepAliveCount);
    }
}

/*============================================================================
 * OpcUa_ModifySubscriptionResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModifySubscriptionResponse_Encode(OpcUa_ModifySubscriptionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= Double_Write(&a_pValue->RevisedPublishingInterval, msgBuf);
    status &= UInt32_Write(&a_pValue->RevisedLifetimeCount, msgBuf);
    status &= UInt32_Write(&a_pValue->RevisedMaxKeepAliveCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ModifySubscriptionResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModifySubscriptionResponse_Decode(OpcUa_ModifySubscriptionResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ModifySubscriptionResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= Double_Read(&a_pValue->RevisedPublishingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedLifetimeCount, msgBuf);
    status &= UInt32_Read(&a_pValue->RevisedMaxKeepAliveCount, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ModifySubscriptionResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ModifySubscriptionResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ModifySubscriptionResponse_EncodeableType =
{
    "ModifySubscriptionResponse",
    OpcUaId_ModifySubscriptionResponse,
    OpcUaId_ModifySubscriptionResponse_Encoding_DefaultBinary,
    OpcUaId_ModifySubscriptionResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ModifySubscriptionResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ModifySubscriptionResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ModifySubscriptionResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ModifySubscriptionResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ModifySubscriptionResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ModifySubscriptionResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingMode
#ifndef OPCUA_EXCLUDE_SetPublishingModeRequest
/*============================================================================
 * OpcUa_SetPublishingModeRequest_Initialize
 *===========================================================================*/
void OpcUa_SetPublishingModeRequest_Initialize(OpcUa_SetPublishingModeRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        Boolean_Initialize(&a_pValue->PublishingEnabled);
        SOPC_Initialize_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * OpcUa_SetPublishingModeRequest_Clear
 *===========================================================================*/
void OpcUa_SetPublishingModeRequest_Clear(OpcUa_SetPublishingModeRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        Boolean_Clear(&a_pValue->PublishingEnabled);
        SOPC_Clear_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * OpcUa_SetPublishingModeRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetPublishingModeRequest_Encode(OpcUa_SetPublishingModeRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Write(&a_pValue->PublishingEnabled, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SetPublishingModeRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetPublishingModeRequest_Decode(OpcUa_SetPublishingModeRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SetPublishingModeRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= Boolean_Read(&a_pValue->PublishingEnabled, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        OpcUa_SetPublishingModeRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SetPublishingModeRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SetPublishingModeRequest_EncodeableType =
{
    "SetPublishingModeRequest",
    OpcUaId_SetPublishingModeRequest,
    OpcUaId_SetPublishingModeRequest_Encoding_DefaultBinary,
    OpcUaId_SetPublishingModeRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SetPublishingModeRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SetPublishingModeRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SetPublishingModeRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SetPublishingModeRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SetPublishingModeRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SetPublishingModeRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SetPublishingModeResponse
/*============================================================================
 * OpcUa_SetPublishingModeResponse_Initialize
 *===========================================================================*/
void OpcUa_SetPublishingModeResponse_Initialize(OpcUa_SetPublishingModeResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_SetPublishingModeResponse_Clear
 *===========================================================================*/
void OpcUa_SetPublishingModeResponse_Clear(OpcUa_SetPublishingModeResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_SetPublishingModeResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetPublishingModeResponse_Encode(OpcUa_SetPublishingModeResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SetPublishingModeResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SetPublishingModeResponse_Decode(OpcUa_SetPublishingModeResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SetPublishingModeResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_SetPublishingModeResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SetPublishingModeResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SetPublishingModeResponse_EncodeableType =
{
    "SetPublishingModeResponse",
    OpcUaId_SetPublishingModeResponse,
    OpcUaId_SetPublishingModeResponse_Encoding_DefaultBinary,
    OpcUaId_SetPublishingModeResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SetPublishingModeResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SetPublishingModeResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SetPublishingModeResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SetPublishingModeResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SetPublishingModeResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SetPublishingModeResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_NotificationMessage
/*============================================================================
 * OpcUa_NotificationMessage_Initialize
 *===========================================================================*/
void OpcUa_NotificationMessage_Initialize(OpcUa_NotificationMessage* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SequenceNumber);
        DateTime_Initialize(&a_pValue->PublishTime);
        SOPC_Initialize_Array(&a_pValue->NoOfNotificationData, (void**) &a_pValue->NotificationData, 
                            sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnInitialize*) ExtensionObject_Initialize);
    }
}

/*============================================================================
 * OpcUa_NotificationMessage_Clear
 *===========================================================================*/
void OpcUa_NotificationMessage_Clear(OpcUa_NotificationMessage* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SequenceNumber);
        DateTime_Clear(&a_pValue->PublishTime);
        SOPC_Clear_Array(&a_pValue->NoOfNotificationData, (void**) &a_pValue->NotificationData, 
                       sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnClear*) ExtensionObject_Clear);
    }
}

/*============================================================================
 * OpcUa_NotificationMessage_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NotificationMessage_Encode(OpcUa_NotificationMessage* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->SequenceNumber, msgBuf);
    status &= DateTime_Write(&a_pValue->PublishTime, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNotificationData, (void**) &a_pValue->NotificationData, 
                   sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnEncode*) ExtensionObject_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_NotificationMessage_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NotificationMessage_Decode(OpcUa_NotificationMessage* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_NotificationMessage_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SequenceNumber, msgBuf);
    status &= DateTime_Read(&a_pValue->PublishTime, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNotificationData, (void**) &a_pValue->NotificationData, 
                  sizeof(SOPC_ExtensionObject), (SOPC_EncodeableObject_PfnDecode*) ExtensionObject_Read);

    if(status != STATUS_OK){
        OpcUa_NotificationMessage_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_NotificationMessage_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_NotificationMessage_EncodeableType =
{
    "NotificationMessage",
    OpcUaId_NotificationMessage,
    OpcUaId_NotificationMessage_Encoding_DefaultBinary,
    OpcUaId_NotificationMessage_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_NotificationMessage),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_NotificationMessage_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_NotificationMessage_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_NotificationMessage_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_NotificationMessage_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_NotificationMessage_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DataChangeNotification
/*============================================================================
 * OpcUa_DataChangeNotification_Initialize
 *===========================================================================*/
void OpcUa_DataChangeNotification_Initialize(OpcUa_DataChangeNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfMonitoredItems, (void**) &a_pValue->MonitoredItems, 
                            sizeof(OpcUa_MonitoredItemNotification), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_MonitoredItemNotification_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_DataChangeNotification_Clear
 *===========================================================================*/
void OpcUa_DataChangeNotification_Clear(OpcUa_DataChangeNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfMonitoredItems, (void**) &a_pValue->MonitoredItems, 
                       sizeof(OpcUa_MonitoredItemNotification), (SOPC_EncodeableObject_PfnClear*) OpcUa_MonitoredItemNotification_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_DataChangeNotification_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DataChangeNotification_Encode(OpcUa_DataChangeNotification* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfMonitoredItems, (void**) &a_pValue->MonitoredItems, 
                   sizeof(OpcUa_MonitoredItemNotification), (SOPC_EncodeableObject_PfnEncode*) OpcUa_MonitoredItemNotification_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DataChangeNotification_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DataChangeNotification_Decode(OpcUa_DataChangeNotification* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DataChangeNotification_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfMonitoredItems, (void**) &a_pValue->MonitoredItems, 
                  sizeof(OpcUa_MonitoredItemNotification), (SOPC_EncodeableObject_PfnDecode*) OpcUa_MonitoredItemNotification_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_DataChangeNotification_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DataChangeNotification_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DataChangeNotification_EncodeableType =
{
    "DataChangeNotification",
    OpcUaId_DataChangeNotification,
    OpcUaId_DataChangeNotification_Encoding_DefaultBinary,
    OpcUaId_DataChangeNotification_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DataChangeNotification),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DataChangeNotification_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DataChangeNotification_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DataChangeNotification_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DataChangeNotification_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DataChangeNotification_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_MonitoredItemNotification
/*============================================================================
 * OpcUa_MonitoredItemNotification_Initialize
 *===========================================================================*/
void OpcUa_MonitoredItemNotification_Initialize(OpcUa_MonitoredItemNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->ClientHandle);
        DataValue_Initialize(&a_pValue->Value);
    }
}

/*============================================================================
 * OpcUa_MonitoredItemNotification_Clear
 *===========================================================================*/
void OpcUa_MonitoredItemNotification_Clear(OpcUa_MonitoredItemNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->ClientHandle);
        DataValue_Clear(&a_pValue->Value);
    }
}

/*============================================================================
 * OpcUa_MonitoredItemNotification_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemNotification_Encode(OpcUa_MonitoredItemNotification* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_MonitoredItemNotification_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_MonitoredItemNotification_Decode(OpcUa_MonitoredItemNotification* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_MonitoredItemNotification_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->ClientHandle, msgBuf);
    status &= DataValue_Read(&a_pValue->Value, msgBuf);

    if(status != STATUS_OK){
        OpcUa_MonitoredItemNotification_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_MonitoredItemNotification_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_MonitoredItemNotification_EncodeableType =
{
    "MonitoredItemNotification",
    OpcUaId_MonitoredItemNotification,
    OpcUaId_MonitoredItemNotification_Encoding_DefaultBinary,
    OpcUaId_MonitoredItemNotification_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_MonitoredItemNotification),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_MonitoredItemNotification_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_MonitoredItemNotification_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_MonitoredItemNotification_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_MonitoredItemNotification_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_MonitoredItemNotification_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EventNotificationList
/*============================================================================
 * OpcUa_EventNotificationList_Initialize
 *===========================================================================*/
void OpcUa_EventNotificationList_Initialize(OpcUa_EventNotificationList* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                            sizeof(OpcUa_EventFieldList), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_EventFieldList_Initialize);
    }
}

/*============================================================================
 * OpcUa_EventNotificationList_Clear
 *===========================================================================*/
void OpcUa_EventNotificationList_Clear(OpcUa_EventNotificationList* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                       sizeof(OpcUa_EventFieldList), (SOPC_EncodeableObject_PfnClear*) OpcUa_EventFieldList_Clear);
    }
}

/*============================================================================
 * OpcUa_EventNotificationList_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EventNotificationList_Encode(OpcUa_EventNotificationList* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                   sizeof(OpcUa_EventFieldList), (SOPC_EncodeableObject_PfnEncode*) OpcUa_EventFieldList_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_EventNotificationList_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EventNotificationList_Decode(OpcUa_EventNotificationList* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EventNotificationList_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfEvents, (void**) &a_pValue->Events, 
                  sizeof(OpcUa_EventFieldList), (SOPC_EncodeableObject_PfnDecode*) OpcUa_EventFieldList_Decode);

    if(status != STATUS_OK){
        OpcUa_EventNotificationList_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EventNotificationList_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EventNotificationList_EncodeableType =
{
    "EventNotificationList",
    OpcUaId_EventNotificationList,
    OpcUaId_EventNotificationList_Encoding_DefaultBinary,
    OpcUaId_EventNotificationList_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EventNotificationList),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EventNotificationList_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EventNotificationList_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EventNotificationList_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EventNotificationList_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EventNotificationList_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EventFieldList
/*============================================================================
 * OpcUa_EventFieldList_Initialize
 *===========================================================================*/
void OpcUa_EventFieldList_Initialize(OpcUa_EventFieldList* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->ClientHandle);
        SOPC_Initialize_Array(&a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                            sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * OpcUa_EventFieldList_Clear
 *===========================================================================*/
void OpcUa_EventFieldList_Clear(OpcUa_EventFieldList* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->ClientHandle);
        SOPC_Clear_Array(&a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                       sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * OpcUa_EventFieldList_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EventFieldList_Encode(OpcUa_EventFieldList* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= UInt32_Write(&a_pValue->ClientHandle, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                   sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_EventFieldList_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EventFieldList_Decode(OpcUa_EventFieldList* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EventFieldList_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->ClientHandle, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                  sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        OpcUa_EventFieldList_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EventFieldList_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EventFieldList_EncodeableType =
{
    "EventFieldList",
    OpcUaId_EventFieldList,
    OpcUaId_EventFieldList_Encoding_DefaultBinary,
    OpcUaId_EventFieldList_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EventFieldList),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EventFieldList_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EventFieldList_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EventFieldList_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EventFieldList_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EventFieldList_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_HistoryEventFieldList
/*============================================================================
 * OpcUa_HistoryEventFieldList_Initialize
 *===========================================================================*/
void OpcUa_HistoryEventFieldList_Initialize(OpcUa_HistoryEventFieldList* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                            sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnInitialize*) Variant_Initialize);
    }
}

/*============================================================================
 * OpcUa_HistoryEventFieldList_Clear
 *===========================================================================*/
void OpcUa_HistoryEventFieldList_Clear(OpcUa_HistoryEventFieldList* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                       sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnClear*) Variant_Clear);
    }
}

/*============================================================================
 * OpcUa_HistoryEventFieldList_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryEventFieldList_Encode(OpcUa_HistoryEventFieldList* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                   sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnEncode*) Variant_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryEventFieldList_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_HistoryEventFieldList_Decode(OpcUa_HistoryEventFieldList* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_HistoryEventFieldList_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfEventFields, (void**) &a_pValue->EventFields, 
                  sizeof(SOPC_Variant), (SOPC_EncodeableObject_PfnDecode*) Variant_Read);

    if(status != STATUS_OK){
        OpcUa_HistoryEventFieldList_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_HistoryEventFieldList_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_HistoryEventFieldList_EncodeableType =
{
    "HistoryEventFieldList",
    OpcUaId_HistoryEventFieldList,
    OpcUaId_HistoryEventFieldList_Encoding_DefaultBinary,
    OpcUaId_HistoryEventFieldList_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_HistoryEventFieldList),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_HistoryEventFieldList_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_HistoryEventFieldList_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_HistoryEventFieldList_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_HistoryEventFieldList_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_HistoryEventFieldList_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_StatusChangeNotification
/*============================================================================
 * OpcUa_StatusChangeNotification_Initialize
 *===========================================================================*/
void OpcUa_StatusChangeNotification_Initialize(OpcUa_StatusChangeNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->Status);
        DiagnosticInfo_Initialize(&a_pValue->DiagnosticInfo);
    }
}

/*============================================================================
 * OpcUa_StatusChangeNotification_Clear
 *===========================================================================*/
void OpcUa_StatusChangeNotification_Clear(OpcUa_StatusChangeNotification* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->Status);
        DiagnosticInfo_Clear(&a_pValue->DiagnosticInfo);
    }
}

/*============================================================================
 * OpcUa_StatusChangeNotification_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_StatusChangeNotification_Encode(OpcUa_StatusChangeNotification* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_StatusChangeNotification_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_StatusChangeNotification_Decode(OpcUa_StatusChangeNotification* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_StatusChangeNotification_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->Status, msgBuf);
    status &= DiagnosticInfo_Read(&a_pValue->DiagnosticInfo, msgBuf);

    if(status != STATUS_OK){
        OpcUa_StatusChangeNotification_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_StatusChangeNotification_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_StatusChangeNotification_EncodeableType =
{
    "StatusChangeNotification",
    OpcUaId_StatusChangeNotification,
    OpcUaId_StatusChangeNotification_Encoding_DefaultBinary,
    OpcUaId_StatusChangeNotification_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_StatusChangeNotification),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_StatusChangeNotification_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_StatusChangeNotification_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_StatusChangeNotification_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_StatusChangeNotification_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_StatusChangeNotification_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SubscriptionAcknowledgement
/*============================================================================
 * OpcUa_SubscriptionAcknowledgement_Initialize
 *===========================================================================*/
void OpcUa_SubscriptionAcknowledgement_Initialize(OpcUa_SubscriptionAcknowledgement* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UInt32_Initialize(&a_pValue->SequenceNumber);
    }
}

/*============================================================================
 * OpcUa_SubscriptionAcknowledgement_Clear
 *===========================================================================*/
void OpcUa_SubscriptionAcknowledgement_Clear(OpcUa_SubscriptionAcknowledgement* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->SubscriptionId);
        UInt32_Clear(&a_pValue->SequenceNumber);
    }
}

/*============================================================================
 * OpcUa_SubscriptionAcknowledgement_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SubscriptionAcknowledgement_Encode(OpcUa_SubscriptionAcknowledgement* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_SubscriptionAcknowledgement_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SubscriptionAcknowledgement_Decode(OpcUa_SubscriptionAcknowledgement* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SubscriptionAcknowledgement_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Read(&a_pValue->SequenceNumber, msgBuf);

    if(status != STATUS_OK){
        OpcUa_SubscriptionAcknowledgement_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SubscriptionAcknowledgement_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SubscriptionAcknowledgement_EncodeableType =
{
    "SubscriptionAcknowledgement",
    OpcUaId_SubscriptionAcknowledgement,
    OpcUaId_SubscriptionAcknowledgement_Encoding_DefaultBinary,
    OpcUaId_SubscriptionAcknowledgement_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SubscriptionAcknowledgement),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SubscriptionAcknowledgement_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SubscriptionAcknowledgement_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SubscriptionAcknowledgement_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SubscriptionAcknowledgement_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SubscriptionAcknowledgement_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Publish
#ifndef OPCUA_EXCLUDE_PublishRequest
/*============================================================================
 * OpcUa_PublishRequest_Initialize
 *===========================================================================*/
void OpcUa_PublishRequest_Initialize(OpcUa_PublishRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfSubscriptionAcknowledgements, (void**) &a_pValue->SubscriptionAcknowledgements, 
                            sizeof(OpcUa_SubscriptionAcknowledgement), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_SubscriptionAcknowledgement_Initialize);
    }
}

/*============================================================================
 * OpcUa_PublishRequest_Clear
 *===========================================================================*/
void OpcUa_PublishRequest_Clear(OpcUa_PublishRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfSubscriptionAcknowledgements, (void**) &a_pValue->SubscriptionAcknowledgements, 
                       sizeof(OpcUa_SubscriptionAcknowledgement), (SOPC_EncodeableObject_PfnClear*) OpcUa_SubscriptionAcknowledgement_Clear);
    }
}

/*============================================================================
 * OpcUa_PublishRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_PublishRequest_Encode(OpcUa_PublishRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfSubscriptionAcknowledgements, (void**) &a_pValue->SubscriptionAcknowledgements, 
                   sizeof(OpcUa_SubscriptionAcknowledgement), (SOPC_EncodeableObject_PfnEncode*) OpcUa_SubscriptionAcknowledgement_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_PublishRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_PublishRequest_Decode(OpcUa_PublishRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_PublishRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfSubscriptionAcknowledgements, (void**) &a_pValue->SubscriptionAcknowledgements, 
                  sizeof(OpcUa_SubscriptionAcknowledgement), (SOPC_EncodeableObject_PfnDecode*) OpcUa_SubscriptionAcknowledgement_Decode);

    if(status != STATUS_OK){
        OpcUa_PublishRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_PublishRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_PublishRequest_EncodeableType =
{
    "PublishRequest",
    OpcUaId_PublishRequest,
    OpcUaId_PublishRequest_Encoding_DefaultBinary,
    OpcUaId_PublishRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_PublishRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_PublishRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_PublishRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_PublishRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_PublishRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_PublishRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_PublishResponse
/*============================================================================
 * OpcUa_PublishResponse_Initialize
 *===========================================================================*/
void OpcUa_PublishResponse_Initialize(OpcUa_PublishResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        SOPC_Initialize_Array(&a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Boolean_Initialize(&a_pValue->MoreNotifications);
        OpcUa_NotificationMessage_Initialize(&a_pValue->NotificationMessage);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_PublishResponse_Clear
 *===========================================================================*/
void OpcUa_PublishResponse_Clear(OpcUa_PublishResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        SOPC_Clear_Array(&a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
        Boolean_Clear(&a_pValue->MoreNotifications);
        OpcUa_NotificationMessage_Clear(&a_pValue->NotificationMessage);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_PublishResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_PublishResponse_Encode(OpcUa_PublishResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Boolean_Write(&a_pValue->MoreNotifications, msgBuf);
    status &= OpcUa_NotificationMessage_Encode(&a_pValue->NotificationMessage, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_PublishResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_PublishResponse_Decode(OpcUa_PublishResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_PublishResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Boolean_Read(&a_pValue->MoreNotifications, msgBuf);
    status &= OpcUa_NotificationMessage_Decode(&a_pValue->NotificationMessage, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_PublishResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_PublishResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_PublishResponse_EncodeableType =
{
    "PublishResponse",
    OpcUaId_PublishResponse,
    OpcUaId_PublishResponse_Encoding_DefaultBinary,
    OpcUaId_PublishResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_PublishResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_PublishResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_PublishResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_PublishResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_PublishResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_PublishResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_Republish
#ifndef OPCUA_EXCLUDE_RepublishRequest
/*============================================================================
 * OpcUa_RepublishRequest_Initialize
 *===========================================================================*/
void OpcUa_RepublishRequest_Initialize(OpcUa_RepublishRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        UInt32_Initialize(&a_pValue->SubscriptionId);
        UInt32_Initialize(&a_pValue->RetransmitSequenceNumber);
    }
}

/*============================================================================
 * OpcUa_RepublishRequest_Clear
 *===========================================================================*/
void OpcUa_RepublishRequest_Clear(OpcUa_RepublishRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        UInt32_Clear(&a_pValue->SubscriptionId);
        UInt32_Clear(&a_pValue->RetransmitSequenceNumber);
    }
}

/*============================================================================
 * OpcUa_RepublishRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RepublishRequest_Encode(OpcUa_RepublishRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Write(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Write(&a_pValue->RetransmitSequenceNumber, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RepublishRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RepublishRequest_Decode(OpcUa_RepublishRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RepublishRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= UInt32_Read(&a_pValue->SubscriptionId, msgBuf);
    status &= UInt32_Read(&a_pValue->RetransmitSequenceNumber, msgBuf);

    if(status != STATUS_OK){
        OpcUa_RepublishRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RepublishRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RepublishRequest_EncodeableType =
{
    "RepublishRequest",
    OpcUaId_RepublishRequest,
    OpcUaId_RepublishRequest_Encoding_DefaultBinary,
    OpcUaId_RepublishRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RepublishRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RepublishRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RepublishRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RepublishRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RepublishRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RepublishRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_RepublishResponse
/*============================================================================
 * OpcUa_RepublishResponse_Initialize
 *===========================================================================*/
void OpcUa_RepublishResponse_Initialize(OpcUa_RepublishResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        OpcUa_NotificationMessage_Initialize(&a_pValue->NotificationMessage);
    }
}

/*============================================================================
 * OpcUa_RepublishResponse_Clear
 *===========================================================================*/
void OpcUa_RepublishResponse_Clear(OpcUa_RepublishResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        OpcUa_NotificationMessage_Clear(&a_pValue->NotificationMessage);
    }
}

/*============================================================================
 * OpcUa_RepublishResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RepublishResponse_Encode(OpcUa_RepublishResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= OpcUa_NotificationMessage_Encode(&a_pValue->NotificationMessage, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RepublishResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RepublishResponse_Decode(OpcUa_RepublishResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RepublishResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= OpcUa_NotificationMessage_Decode(&a_pValue->NotificationMessage, msgBuf);

    if(status != STATUS_OK){
        OpcUa_RepublishResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RepublishResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RepublishResponse_EncodeableType =
{
    "RepublishResponse",
    OpcUaId_RepublishResponse,
    OpcUaId_RepublishResponse_Encoding_DefaultBinary,
    OpcUaId_RepublishResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RepublishResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RepublishResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RepublishResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RepublishResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RepublishResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RepublishResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_TransferResult
/*============================================================================
 * OpcUa_TransferResult_Initialize
 *===========================================================================*/
void OpcUa_TransferResult_Initialize(OpcUa_TransferResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        SOPC_Initialize_Array(&a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * OpcUa_TransferResult_Clear
 *===========================================================================*/
void OpcUa_TransferResult_Clear(OpcUa_TransferResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        SOPC_Clear_Array(&a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * OpcUa_TransferResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TransferResult_Encode(OpcUa_TransferResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= StatusCode_Write(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_TransferResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TransferResult_Decode(OpcUa_TransferResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_TransferResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfAvailableSequenceNumbers, (void**) &a_pValue->AvailableSequenceNumbers, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        OpcUa_TransferResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_TransferResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_TransferResult_EncodeableType =
{
    "TransferResult",
    OpcUaId_TransferResult,
    OpcUaId_TransferResult_Encoding_DefaultBinary,
    OpcUaId_TransferResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_TransferResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_TransferResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_TransferResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_TransferResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_TransferResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_TransferResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptions
#ifndef OPCUA_EXCLUDE_TransferSubscriptionsRequest
/*============================================================================
 * OpcUa_TransferSubscriptionsRequest_Initialize
 *===========================================================================*/
void OpcUa_TransferSubscriptionsRequest_Initialize(OpcUa_TransferSubscriptionsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
        Boolean_Initialize(&a_pValue->SendInitialValues);
    }
}

/*============================================================================
 * OpcUa_TransferSubscriptionsRequest_Clear
 *===========================================================================*/
void OpcUa_TransferSubscriptionsRequest_Clear(OpcUa_TransferSubscriptionsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
        Boolean_Clear(&a_pValue->SendInitialValues);
    }
}

/*============================================================================
 * OpcUa_TransferSubscriptionsRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TransferSubscriptionsRequest_Encode(OpcUa_TransferSubscriptionsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);
    status &= Boolean_Write(&a_pValue->SendInitialValues, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_TransferSubscriptionsRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TransferSubscriptionsRequest_Decode(OpcUa_TransferSubscriptionsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_TransferSubscriptionsRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);
    status &= Boolean_Read(&a_pValue->SendInitialValues, msgBuf);

    if(status != STATUS_OK){
        OpcUa_TransferSubscriptionsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_TransferSubscriptionsRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_TransferSubscriptionsRequest_EncodeableType =
{
    "TransferSubscriptionsRequest",
    OpcUaId_TransferSubscriptionsRequest,
    OpcUaId_TransferSubscriptionsRequest_Encoding_DefaultBinary,
    OpcUaId_TransferSubscriptionsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_TransferSubscriptionsRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_TransferSubscriptionsRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_TransferSubscriptionsRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_TransferSubscriptionsRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_TransferSubscriptionsRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_TransferSubscriptionsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_TransferSubscriptionsResponse
/*============================================================================
 * OpcUa_TransferSubscriptionsResponse_Initialize
 *===========================================================================*/
void OpcUa_TransferSubscriptionsResponse_Initialize(OpcUa_TransferSubscriptionsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(OpcUa_TransferResult), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_TransferResult_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_TransferSubscriptionsResponse_Clear
 *===========================================================================*/
void OpcUa_TransferSubscriptionsResponse_Clear(OpcUa_TransferSubscriptionsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(OpcUa_TransferResult), (SOPC_EncodeableObject_PfnClear*) OpcUa_TransferResult_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_TransferSubscriptionsResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TransferSubscriptionsResponse_Encode(OpcUa_TransferSubscriptionsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(OpcUa_TransferResult), (SOPC_EncodeableObject_PfnEncode*) OpcUa_TransferResult_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_TransferSubscriptionsResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_TransferSubscriptionsResponse_Decode(OpcUa_TransferSubscriptionsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_TransferSubscriptionsResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(OpcUa_TransferResult), (SOPC_EncodeableObject_PfnDecode*) OpcUa_TransferResult_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_TransferSubscriptionsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_TransferSubscriptionsResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_TransferSubscriptionsResponse_EncodeableType =
{
    "TransferSubscriptionsResponse",
    OpcUaId_TransferSubscriptionsResponse,
    OpcUaId_TransferSubscriptionsResponse_Encoding_DefaultBinary,
    OpcUaId_TransferSubscriptionsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_TransferSubscriptionsResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_TransferSubscriptionsResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_TransferSubscriptionsResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_TransferSubscriptionsResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_TransferSubscriptionsResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_TransferSubscriptionsResponse_Decode
};
#endif
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptions
#ifndef OPCUA_EXCLUDE_DeleteSubscriptionsRequest
/*============================================================================
 * OpcUa_DeleteSubscriptionsRequest_Initialize
 *===========================================================================*/
void OpcUa_DeleteSubscriptionsRequest_Initialize(OpcUa_DeleteSubscriptionsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Initialize(&a_pValue->RequestHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                            sizeof(uint32_t), (SOPC_EncodeableObject_PfnInitialize*) UInt32_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteSubscriptionsRequest_Clear
 *===========================================================================*/
void OpcUa_DeleteSubscriptionsRequest_Clear(OpcUa_DeleteSubscriptionsRequest* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_RequestHeader_Clear(&a_pValue->RequestHeader);
        SOPC_Clear_Array(&a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                       sizeof(uint32_t), (SOPC_EncodeableObject_PfnClear*) UInt32_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteSubscriptionsRequest_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteSubscriptionsRequest_Encode(OpcUa_DeleteSubscriptionsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_RequestHeader_Encode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                   sizeof(uint32_t), (SOPC_EncodeableObject_PfnEncode*) UInt32_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteSubscriptionsRequest_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteSubscriptionsRequest_Decode(OpcUa_DeleteSubscriptionsRequest* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteSubscriptionsRequest_Initialize(a_pValue);

    status &= OpcUa_RequestHeader_Decode(&a_pValue->RequestHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfSubscriptionIds, (void**) &a_pValue->SubscriptionIds, 
                  sizeof(uint32_t), (SOPC_EncodeableObject_PfnDecode*) UInt32_Read);

    if(status != STATUS_OK){
        OpcUa_DeleteSubscriptionsRequest_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteSubscriptionsRequest_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteSubscriptionsRequest_EncodeableType =
{
    "DeleteSubscriptionsRequest",
    OpcUaId_DeleteSubscriptionsRequest,
    OpcUaId_DeleteSubscriptionsRequest_Encoding_DefaultBinary,
    OpcUaId_DeleteSubscriptionsRequest_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteSubscriptionsRequest),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteSubscriptionsRequest_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteSubscriptionsRequest_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteSubscriptionsRequest_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteSubscriptionsRequest_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteSubscriptionsRequest_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DeleteSubscriptionsResponse
/*============================================================================
 * OpcUa_DeleteSubscriptionsResponse_Initialize
 *===========================================================================*/
void OpcUa_DeleteSubscriptionsResponse_Initialize(OpcUa_DeleteSubscriptionsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Initialize(&a_pValue->ResponseHeader);
        SOPC_Initialize_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                            sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnInitialize*) StatusCode_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                            sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnInitialize*) DiagnosticInfo_Initialize);
    }
}

/*============================================================================
 * OpcUa_DeleteSubscriptionsResponse_Clear
 *===========================================================================*/
void OpcUa_DeleteSubscriptionsResponse_Clear(OpcUa_DeleteSubscriptionsResponse* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_ResponseHeader_Clear(&a_pValue->ResponseHeader);
        SOPC_Clear_Array(&a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                       sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnClear*) StatusCode_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                       sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnClear*) DiagnosticInfo_Clear);
    }
}

/*============================================================================
 * OpcUa_DeleteSubscriptionsResponse_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteSubscriptionsResponse_Encode(OpcUa_DeleteSubscriptionsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_ResponseHeader_Encode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                   sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnEncode*) StatusCode_Write);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                   sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnEncode*) DiagnosticInfo_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteSubscriptionsResponse_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DeleteSubscriptionsResponse_Decode(OpcUa_DeleteSubscriptionsResponse* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DeleteSubscriptionsResponse_Initialize(a_pValue);

    status &= OpcUa_ResponseHeader_Decode(&a_pValue->ResponseHeader, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfResults, (void**) &a_pValue->Results, 
                  sizeof(SOPC_StatusCode), (SOPC_EncodeableObject_PfnDecode*) StatusCode_Read);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfDiagnosticInfos, (void**) &a_pValue->DiagnosticInfos, 
                  sizeof(SOPC_DiagnosticInfo), (SOPC_EncodeableObject_PfnDecode*) DiagnosticInfo_Read);

    if(status != STATUS_OK){
        OpcUa_DeleteSubscriptionsResponse_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DeleteSubscriptionsResponse_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DeleteSubscriptionsResponse_EncodeableType =
{
    "DeleteSubscriptionsResponse",
    OpcUaId_DeleteSubscriptionsResponse,
    OpcUaId_DeleteSubscriptionsResponse_Encoding_DefaultBinary,
    OpcUaId_DeleteSubscriptionsResponse_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DeleteSubscriptionsResponse),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DeleteSubscriptionsResponse_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DeleteSubscriptionsResponse_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DeleteSubscriptionsResponse_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DeleteSubscriptionsResponse_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DeleteSubscriptionsResponse_Decode
};
#endif
#endif


#ifndef OPCUA_EXCLUDE_BuildInfo
/*============================================================================
 * OpcUa_BuildInfo_Initialize
 *===========================================================================*/
void OpcUa_BuildInfo_Initialize(OpcUa_BuildInfo* a_pValue)
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
 * OpcUa_BuildInfo_Clear
 *===========================================================================*/
void OpcUa_BuildInfo_Clear(OpcUa_BuildInfo* a_pValue)
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
 * OpcUa_BuildInfo_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BuildInfo_Encode(OpcUa_BuildInfo* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_BuildInfo_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_BuildInfo_Decode(OpcUa_BuildInfo* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_BuildInfo_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ProductUri, msgBuf);
    status &= String_Read(&a_pValue->ManufacturerName, msgBuf);
    status &= String_Read(&a_pValue->ProductName, msgBuf);
    status &= String_Read(&a_pValue->SoftwareVersion, msgBuf);
    status &= String_Read(&a_pValue->BuildNumber, msgBuf);
    status &= DateTime_Read(&a_pValue->BuildDate, msgBuf);

    if(status != STATUS_OK){
        OpcUa_BuildInfo_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_BuildInfo_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_BuildInfo_EncodeableType =
{
    "BuildInfo",
    OpcUaId_BuildInfo,
    OpcUaId_BuildInfo_Encoding_DefaultBinary,
    OpcUaId_BuildInfo_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_BuildInfo),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_BuildInfo_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_BuildInfo_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_BuildInfo_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_BuildInfo_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_BuildInfo_Decode
};
#endif



#ifndef OPCUA_EXCLUDE_RedundantServerDataType
/*============================================================================
 * OpcUa_RedundantServerDataType_Initialize
 *===========================================================================*/
void OpcUa_RedundantServerDataType_Initialize(OpcUa_RedundantServerDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ServerId);
        Byte_Initialize(&a_pValue->ServiceLevel);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->ServerState);
    }
}

/*============================================================================
 * OpcUa_RedundantServerDataType_Clear
 *===========================================================================*/
void OpcUa_RedundantServerDataType_Clear(OpcUa_RedundantServerDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ServerId);
        Byte_Clear(&a_pValue->ServiceLevel);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->ServerState);
    }
}

/*============================================================================
 * OpcUa_RedundantServerDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RedundantServerDataType_Encode(OpcUa_RedundantServerDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ServerId, msgBuf);
    status &= Byte_Write(&a_pValue->ServiceLevel, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ServerState);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_RedundantServerDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_RedundantServerDataType_Decode(OpcUa_RedundantServerDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_RedundantServerDataType_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ServerId, msgBuf);
    status &= Byte_Read(&a_pValue->ServiceLevel, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->ServerState);

    if(status != STATUS_OK){
        OpcUa_RedundantServerDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_RedundantServerDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_RedundantServerDataType_EncodeableType =
{
    "RedundantServerDataType",
    OpcUaId_RedundantServerDataType,
    OpcUaId_RedundantServerDataType_Encoding_DefaultBinary,
    OpcUaId_RedundantServerDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_RedundantServerDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_RedundantServerDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_RedundantServerDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_RedundantServerDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_RedundantServerDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_RedundantServerDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EndpointUrlListDataType
/*============================================================================
 * OpcUa_EndpointUrlListDataType_Initialize
 *===========================================================================*/
void OpcUa_EndpointUrlListDataType_Initialize(OpcUa_EndpointUrlListDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Initialize_Array(&a_pValue->NoOfEndpointUrlList, (void**) &a_pValue->EndpointUrlList, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
    }
}

/*============================================================================
 * OpcUa_EndpointUrlListDataType_Clear
 *===========================================================================*/
void OpcUa_EndpointUrlListDataType_Clear(OpcUa_EndpointUrlListDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        SOPC_Clear_Array(&a_pValue->NoOfEndpointUrlList, (void**) &a_pValue->EndpointUrlList, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
    }
}

/*============================================================================
 * OpcUa_EndpointUrlListDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EndpointUrlListDataType_Encode(OpcUa_EndpointUrlListDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfEndpointUrlList, (void**) &a_pValue->EndpointUrlList, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_EndpointUrlListDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EndpointUrlListDataType_Decode(OpcUa_EndpointUrlListDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EndpointUrlListDataType_Initialize(a_pValue);

    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfEndpointUrlList, (void**) &a_pValue->EndpointUrlList, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);

    if(status != STATUS_OK){
        OpcUa_EndpointUrlListDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EndpointUrlListDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EndpointUrlListDataType_EncodeableType =
{
    "EndpointUrlListDataType",
    OpcUaId_EndpointUrlListDataType,
    OpcUaId_EndpointUrlListDataType_Encoding_DefaultBinary,
    OpcUaId_EndpointUrlListDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EndpointUrlListDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EndpointUrlListDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EndpointUrlListDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EndpointUrlListDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EndpointUrlListDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EndpointUrlListDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_NetworkGroupDataType
/*============================================================================
 * OpcUa_NetworkGroupDataType_Initialize
 *===========================================================================*/
void OpcUa_NetworkGroupDataType_Initialize(OpcUa_NetworkGroupDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->ServerUri);
        SOPC_Initialize_Array(&a_pValue->NoOfNetworkPaths, (void**) &a_pValue->NetworkPaths, 
                            sizeof(OpcUa_EndpointUrlListDataType), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_EndpointUrlListDataType_Initialize);
    }
}

/*============================================================================
 * OpcUa_NetworkGroupDataType_Clear
 *===========================================================================*/
void OpcUa_NetworkGroupDataType_Clear(OpcUa_NetworkGroupDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->ServerUri);
        SOPC_Clear_Array(&a_pValue->NoOfNetworkPaths, (void**) &a_pValue->NetworkPaths, 
                       sizeof(OpcUa_EndpointUrlListDataType), (SOPC_EncodeableObject_PfnClear*) OpcUa_EndpointUrlListDataType_Clear);
    }
}

/*============================================================================
 * OpcUa_NetworkGroupDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NetworkGroupDataType_Encode(OpcUa_NetworkGroupDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= String_Write(&a_pValue->ServerUri, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfNetworkPaths, (void**) &a_pValue->NetworkPaths, 
                   sizeof(OpcUa_EndpointUrlListDataType), (SOPC_EncodeableObject_PfnEncode*) OpcUa_EndpointUrlListDataType_Encode);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_NetworkGroupDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_NetworkGroupDataType_Decode(OpcUa_NetworkGroupDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_NetworkGroupDataType_Initialize(a_pValue);

    status &= String_Read(&a_pValue->ServerUri, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfNetworkPaths, (void**) &a_pValue->NetworkPaths, 
                  sizeof(OpcUa_EndpointUrlListDataType), (SOPC_EncodeableObject_PfnDecode*) OpcUa_EndpointUrlListDataType_Decode);

    if(status != STATUS_OK){
        OpcUa_NetworkGroupDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_NetworkGroupDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_NetworkGroupDataType_EncodeableType =
{
    "NetworkGroupDataType",
    OpcUaId_NetworkGroupDataType,
    OpcUaId_NetworkGroupDataType_Encoding_DefaultBinary,
    OpcUaId_NetworkGroupDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_NetworkGroupDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_NetworkGroupDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_NetworkGroupDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_NetworkGroupDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_NetworkGroupDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_NetworkGroupDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SamplingIntervalDiagnosticsDataType
/*============================================================================
 * OpcUa_SamplingIntervalDiagnosticsDataType_Initialize
 *===========================================================================*/
void OpcUa_SamplingIntervalDiagnosticsDataType_Initialize(OpcUa_SamplingIntervalDiagnosticsDataType* a_pValue)
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
 * OpcUa_SamplingIntervalDiagnosticsDataType_Clear
 *===========================================================================*/
void OpcUa_SamplingIntervalDiagnosticsDataType_Clear(OpcUa_SamplingIntervalDiagnosticsDataType* a_pValue)
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
 * OpcUa_SamplingIntervalDiagnosticsDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SamplingIntervalDiagnosticsDataType_Encode(OpcUa_SamplingIntervalDiagnosticsDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_SamplingIntervalDiagnosticsDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SamplingIntervalDiagnosticsDataType_Decode(OpcUa_SamplingIntervalDiagnosticsDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SamplingIntervalDiagnosticsDataType_Initialize(a_pValue);

    status &= Double_Read(&a_pValue->SamplingInterval, msgBuf);
    status &= UInt32_Read(&a_pValue->MonitoredItemCount, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxMonitoredItemCount, msgBuf);
    status &= UInt32_Read(&a_pValue->DisabledMonitoredItemCount, msgBuf);

    if(status != STATUS_OK){
        OpcUa_SamplingIntervalDiagnosticsDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SamplingIntervalDiagnosticsDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SamplingIntervalDiagnosticsDataType_EncodeableType =
{
    "SamplingIntervalDiagnosticsDataType",
    OpcUaId_SamplingIntervalDiagnosticsDataType,
    OpcUaId_SamplingIntervalDiagnosticsDataType_Encoding_DefaultBinary,
    OpcUaId_SamplingIntervalDiagnosticsDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SamplingIntervalDiagnosticsDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SamplingIntervalDiagnosticsDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SamplingIntervalDiagnosticsDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SamplingIntervalDiagnosticsDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SamplingIntervalDiagnosticsDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SamplingIntervalDiagnosticsDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ServerDiagnosticsSummaryDataType
/*============================================================================
 * OpcUa_ServerDiagnosticsSummaryDataType_Initialize
 *===========================================================================*/
void OpcUa_ServerDiagnosticsSummaryDataType_Initialize(OpcUa_ServerDiagnosticsSummaryDataType* a_pValue)
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
 * OpcUa_ServerDiagnosticsSummaryDataType_Clear
 *===========================================================================*/
void OpcUa_ServerDiagnosticsSummaryDataType_Clear(OpcUa_ServerDiagnosticsSummaryDataType* a_pValue)
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
 * OpcUa_ServerDiagnosticsSummaryDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerDiagnosticsSummaryDataType_Encode(OpcUa_ServerDiagnosticsSummaryDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ServerDiagnosticsSummaryDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerDiagnosticsSummaryDataType_Decode(OpcUa_ServerDiagnosticsSummaryDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ServerDiagnosticsSummaryDataType_Initialize(a_pValue);

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
        OpcUa_ServerDiagnosticsSummaryDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ServerDiagnosticsSummaryDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ServerDiagnosticsSummaryDataType_EncodeableType =
{
    "ServerDiagnosticsSummaryDataType",
    OpcUaId_ServerDiagnosticsSummaryDataType,
    OpcUaId_ServerDiagnosticsSummaryDataType_Encoding_DefaultBinary,
    OpcUaId_ServerDiagnosticsSummaryDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ServerDiagnosticsSummaryDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ServerDiagnosticsSummaryDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ServerDiagnosticsSummaryDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ServerDiagnosticsSummaryDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ServerDiagnosticsSummaryDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ServerDiagnosticsSummaryDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ServerStatusDataType
/*============================================================================
 * OpcUa_ServerStatusDataType_Initialize
 *===========================================================================*/
void OpcUa_ServerStatusDataType_Initialize(OpcUa_ServerStatusDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Initialize(&a_pValue->StartTime);
        DateTime_Initialize(&a_pValue->CurrentTime);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->State);
        OpcUa_BuildInfo_Initialize(&a_pValue->BuildInfo);
        UInt32_Initialize(&a_pValue->SecondsTillShutdown);
        LocalizedText_Initialize(&a_pValue->ShutdownReason);
    }
}

/*============================================================================
 * OpcUa_ServerStatusDataType_Clear
 *===========================================================================*/
void OpcUa_ServerStatusDataType_Clear(OpcUa_ServerStatusDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        DateTime_Clear(&a_pValue->StartTime);
        DateTime_Clear(&a_pValue->CurrentTime);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->State);
        OpcUa_BuildInfo_Clear(&a_pValue->BuildInfo);
        UInt32_Clear(&a_pValue->SecondsTillShutdown);
        LocalizedText_Clear(&a_pValue->ShutdownReason);
    }
}

/*============================================================================
 * OpcUa_ServerStatusDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerStatusDataType_Encode(OpcUa_ServerStatusDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= DateTime_Write(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Write(&a_pValue->CurrentTime, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->State);
    status &= OpcUa_BuildInfo_Encode(&a_pValue->BuildInfo, msgBuf);
    status &= UInt32_Write(&a_pValue->SecondsTillShutdown, msgBuf);
    status &= LocalizedText_Write(&a_pValue->ShutdownReason, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ServerStatusDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServerStatusDataType_Decode(OpcUa_ServerStatusDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ServerStatusDataType_Initialize(a_pValue);

    status &= DateTime_Read(&a_pValue->StartTime, msgBuf);
    status &= DateTime_Read(&a_pValue->CurrentTime, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->State);
    status &= OpcUa_BuildInfo_Decode(&a_pValue->BuildInfo, msgBuf);
    status &= UInt32_Read(&a_pValue->SecondsTillShutdown, msgBuf);
    status &= LocalizedText_Read(&a_pValue->ShutdownReason, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ServerStatusDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ServerStatusDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ServerStatusDataType_EncodeableType =
{
    "ServerStatusDataType",
    OpcUaId_ServerStatusDataType,
    OpcUaId_ServerStatusDataType_Encoding_DefaultBinary,
    OpcUaId_ServerStatusDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ServerStatusDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ServerStatusDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ServerStatusDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ServerStatusDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ServerStatusDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ServerStatusDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SessionDiagnosticsDataType
/*============================================================================
 * OpcUa_SessionDiagnosticsDataType_Initialize
 *===========================================================================*/
void OpcUa_SessionDiagnosticsDataType_Initialize(OpcUa_SessionDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->SessionId);
        String_Initialize(&a_pValue->SessionName);
        OpcUa_ApplicationDescription_Initialize(&a_pValue->ClientDescription);
        String_Initialize(&a_pValue->ServerUri);
        String_Initialize(&a_pValue->EndpointUrl);
        SOPC_Initialize_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
        Double_Initialize(&a_pValue->ActualSessionTimeout);
        UInt32_Initialize(&a_pValue->MaxResponseMessageSize);
        DateTime_Initialize(&a_pValue->ClientConnectionTime);
        DateTime_Initialize(&a_pValue->ClientLastContactTime);
        UInt32_Initialize(&a_pValue->CurrentSubscriptionsCount);
        UInt32_Initialize(&a_pValue->CurrentMonitoredItemsCount);
        UInt32_Initialize(&a_pValue->CurrentPublishRequestsInQueue);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->TotalRequestCount);
        UInt32_Initialize(&a_pValue->UnauthorizedRequestCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->ReadCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->HistoryReadCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->WriteCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->HistoryUpdateCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->CallCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->CreateMonitoredItemsCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->ModifyMonitoredItemsCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->SetMonitoringModeCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->SetTriggeringCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->DeleteMonitoredItemsCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->CreateSubscriptionCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->ModifySubscriptionCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->SetPublishingModeCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->PublishCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->RepublishCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->TransferSubscriptionsCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->DeleteSubscriptionsCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->AddNodesCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->AddReferencesCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->DeleteNodesCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->DeleteReferencesCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->BrowseCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->BrowseNextCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->TranslateBrowsePathsToNodeIdsCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->QueryFirstCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->QueryNextCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->RegisterNodesCount);
        OpcUa_ServiceCounterDataType_Initialize(&a_pValue->UnregisterNodesCount);
    }
}

/*============================================================================
 * OpcUa_SessionDiagnosticsDataType_Clear
 *===========================================================================*/
void OpcUa_SessionDiagnosticsDataType_Clear(OpcUa_SessionDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->SessionId);
        String_Clear(&a_pValue->SessionName);
        OpcUa_ApplicationDescription_Clear(&a_pValue->ClientDescription);
        String_Clear(&a_pValue->ServerUri);
        String_Clear(&a_pValue->EndpointUrl);
        SOPC_Clear_Array(&a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
        Double_Clear(&a_pValue->ActualSessionTimeout);
        UInt32_Clear(&a_pValue->MaxResponseMessageSize);
        DateTime_Clear(&a_pValue->ClientConnectionTime);
        DateTime_Clear(&a_pValue->ClientLastContactTime);
        UInt32_Clear(&a_pValue->CurrentSubscriptionsCount);
        UInt32_Clear(&a_pValue->CurrentMonitoredItemsCount);
        UInt32_Clear(&a_pValue->CurrentPublishRequestsInQueue);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->TotalRequestCount);
        UInt32_Clear(&a_pValue->UnauthorizedRequestCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->ReadCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->HistoryReadCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->WriteCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->HistoryUpdateCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->CallCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->CreateMonitoredItemsCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->ModifyMonitoredItemsCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->SetMonitoringModeCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->SetTriggeringCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->DeleteMonitoredItemsCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->CreateSubscriptionCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->ModifySubscriptionCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->SetPublishingModeCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->PublishCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->RepublishCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->TransferSubscriptionsCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->DeleteSubscriptionsCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->AddNodesCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->AddReferencesCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->DeleteNodesCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->DeleteReferencesCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->BrowseCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->BrowseNextCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->TranslateBrowsePathsToNodeIdsCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->QueryFirstCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->QueryNextCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->RegisterNodesCount);
        OpcUa_ServiceCounterDataType_Clear(&a_pValue->UnregisterNodesCount);
    }
}

/*============================================================================
 * OpcUa_SessionDiagnosticsDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SessionDiagnosticsDataType_Encode(OpcUa_SessionDiagnosticsDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->SessionId, msgBuf);
    status &= String_Write(&a_pValue->SessionName, msgBuf);
    status &= OpcUa_ApplicationDescription_Encode(&a_pValue->ClientDescription, msgBuf);
    status &= String_Write(&a_pValue->ServerUri, msgBuf);
    status &= String_Write(&a_pValue->EndpointUrl, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);
    status &= Double_Write(&a_pValue->ActualSessionTimeout, msgBuf);
    status &= UInt32_Write(&a_pValue->MaxResponseMessageSize, msgBuf);
    status &= DateTime_Write(&a_pValue->ClientConnectionTime, msgBuf);
    status &= DateTime_Write(&a_pValue->ClientLastContactTime, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentSubscriptionsCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentMonitoredItemsCount, msgBuf);
    status &= UInt32_Write(&a_pValue->CurrentPublishRequestsInQueue, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->TotalRequestCount, msgBuf);
    status &= UInt32_Write(&a_pValue->UnauthorizedRequestCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->ReadCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->HistoryReadCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->WriteCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->HistoryUpdateCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->CallCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->CreateMonitoredItemsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->ModifyMonitoredItemsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->SetMonitoringModeCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->SetTriggeringCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->DeleteMonitoredItemsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->CreateSubscriptionCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->ModifySubscriptionCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->SetPublishingModeCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->PublishCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->RepublishCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->TransferSubscriptionsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->DeleteSubscriptionsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->AddNodesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->AddReferencesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->DeleteNodesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->DeleteReferencesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->BrowseCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->BrowseNextCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->TranslateBrowsePathsToNodeIdsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->QueryFirstCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->QueryNextCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->RegisterNodesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Encode(&a_pValue->UnregisterNodesCount, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SessionDiagnosticsDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SessionDiagnosticsDataType_Decode(OpcUa_SessionDiagnosticsDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SessionDiagnosticsDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->SessionId, msgBuf);
    status &= String_Read(&a_pValue->SessionName, msgBuf);
    status &= OpcUa_ApplicationDescription_Decode(&a_pValue->ClientDescription, msgBuf);
    status &= String_Read(&a_pValue->ServerUri, msgBuf);
    status &= String_Read(&a_pValue->EndpointUrl, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfLocaleIds, (void**) &a_pValue->LocaleIds, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);
    status &= Double_Read(&a_pValue->ActualSessionTimeout, msgBuf);
    status &= UInt32_Read(&a_pValue->MaxResponseMessageSize, msgBuf);
    status &= DateTime_Read(&a_pValue->ClientConnectionTime, msgBuf);
    status &= DateTime_Read(&a_pValue->ClientLastContactTime, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentSubscriptionsCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentMonitoredItemsCount, msgBuf);
    status &= UInt32_Read(&a_pValue->CurrentPublishRequestsInQueue, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->TotalRequestCount, msgBuf);
    status &= UInt32_Read(&a_pValue->UnauthorizedRequestCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->ReadCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->HistoryReadCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->WriteCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->HistoryUpdateCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->CallCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->CreateMonitoredItemsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->ModifyMonitoredItemsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->SetMonitoringModeCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->SetTriggeringCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->DeleteMonitoredItemsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->CreateSubscriptionCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->ModifySubscriptionCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->SetPublishingModeCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->PublishCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->RepublishCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->TransferSubscriptionsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->DeleteSubscriptionsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->AddNodesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->AddReferencesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->DeleteNodesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->DeleteReferencesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->BrowseCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->BrowseNextCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->TranslateBrowsePathsToNodeIdsCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->QueryFirstCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->QueryNextCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->RegisterNodesCount, msgBuf);
    status &= OpcUa_ServiceCounterDataType_Decode(&a_pValue->UnregisterNodesCount, msgBuf);

    if(status != STATUS_OK){
        OpcUa_SessionDiagnosticsDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SessionDiagnosticsDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SessionDiagnosticsDataType_EncodeableType =
{
    "SessionDiagnosticsDataType",
    OpcUaId_SessionDiagnosticsDataType,
    OpcUaId_SessionDiagnosticsDataType_Encoding_DefaultBinary,
    OpcUaId_SessionDiagnosticsDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SessionDiagnosticsDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SessionDiagnosticsDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SessionDiagnosticsDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SessionDiagnosticsDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SessionDiagnosticsDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SessionDiagnosticsDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SessionSecurityDiagnosticsDataType
/*============================================================================
 * OpcUa_SessionSecurityDiagnosticsDataType_Initialize
 *===========================================================================*/
void OpcUa_SessionSecurityDiagnosticsDataType_Initialize(OpcUa_SessionSecurityDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->SessionId);
        String_Initialize(&a_pValue->ClientUserIdOfSession);
        SOPC_Initialize_Array(&a_pValue->NoOfClientUserIdHistory, (void**) &a_pValue->ClientUserIdHistory, 
                            sizeof(SOPC_String), (SOPC_EncodeableObject_PfnInitialize*) String_Initialize);
        String_Initialize(&a_pValue->AuthenticationMechanism);
        String_Initialize(&a_pValue->Encoding);
        String_Initialize(&a_pValue->TransportProtocol);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        String_Initialize(&a_pValue->SecurityPolicyUri);
        ByteString_Initialize(&a_pValue->ClientCertificate);
    }
}

/*============================================================================
 * OpcUa_SessionSecurityDiagnosticsDataType_Clear
 *===========================================================================*/
void OpcUa_SessionSecurityDiagnosticsDataType_Clear(OpcUa_SessionSecurityDiagnosticsDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->SessionId);
        String_Clear(&a_pValue->ClientUserIdOfSession);
        SOPC_Clear_Array(&a_pValue->NoOfClientUserIdHistory, (void**) &a_pValue->ClientUserIdHistory, 
                       sizeof(SOPC_String), (SOPC_EncodeableObject_PfnClear*) String_Clear);
        String_Clear(&a_pValue->AuthenticationMechanism);
        String_Clear(&a_pValue->Encoding);
        String_Clear(&a_pValue->TransportProtocol);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->SecurityMode);
        String_Clear(&a_pValue->SecurityPolicyUri);
        ByteString_Clear(&a_pValue->ClientCertificate);
    }
}

/*============================================================================
 * OpcUa_SessionSecurityDiagnosticsDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SessionSecurityDiagnosticsDataType_Encode(OpcUa_SessionSecurityDiagnosticsDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->SessionId, msgBuf);
    status &= String_Write(&a_pValue->ClientUserIdOfSession, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfClientUserIdHistory, (void**) &a_pValue->ClientUserIdHistory, 
                   sizeof(SOPC_String), (SOPC_EncodeableObject_PfnEncode*) String_Write);
    status &= String_Write(&a_pValue->AuthenticationMechanism, msgBuf);
    status &= String_Write(&a_pValue->Encoding, msgBuf);
    status &= String_Write(&a_pValue->TransportProtocol, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= String_Write(&a_pValue->SecurityPolicyUri, msgBuf);
    status &= ByteString_Write(&a_pValue->ClientCertificate, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_SessionSecurityDiagnosticsDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SessionSecurityDiagnosticsDataType_Decode(OpcUa_SessionSecurityDiagnosticsDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SessionSecurityDiagnosticsDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->SessionId, msgBuf);
    status &= String_Read(&a_pValue->ClientUserIdOfSession, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfClientUserIdHistory, (void**) &a_pValue->ClientUserIdHistory, 
                  sizeof(SOPC_String), (SOPC_EncodeableObject_PfnDecode*) String_Read);
    status &= String_Read(&a_pValue->AuthenticationMechanism, msgBuf);
    status &= String_Read(&a_pValue->Encoding, msgBuf);
    status &= String_Read(&a_pValue->TransportProtocol, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->SecurityMode);
    status &= String_Read(&a_pValue->SecurityPolicyUri, msgBuf);
    status &= ByteString_Read(&a_pValue->ClientCertificate, msgBuf);

    if(status != STATUS_OK){
        OpcUa_SessionSecurityDiagnosticsDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SessionSecurityDiagnosticsDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SessionSecurityDiagnosticsDataType_EncodeableType =
{
    "SessionSecurityDiagnosticsDataType",
    OpcUaId_SessionSecurityDiagnosticsDataType,
    OpcUaId_SessionSecurityDiagnosticsDataType_Encoding_DefaultBinary,
    OpcUaId_SessionSecurityDiagnosticsDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SessionSecurityDiagnosticsDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SessionSecurityDiagnosticsDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SessionSecurityDiagnosticsDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SessionSecurityDiagnosticsDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SessionSecurityDiagnosticsDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SessionSecurityDiagnosticsDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ServiceCounterDataType
/*============================================================================
 * OpcUa_ServiceCounterDataType_Initialize
 *===========================================================================*/
void OpcUa_ServiceCounterDataType_Initialize(OpcUa_ServiceCounterDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Initialize(&a_pValue->TotalCount);
        UInt32_Initialize(&a_pValue->ErrorCount);
    }
}

/*============================================================================
 * OpcUa_ServiceCounterDataType_Clear
 *===========================================================================*/
void OpcUa_ServiceCounterDataType_Clear(OpcUa_ServiceCounterDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        UInt32_Clear(&a_pValue->TotalCount);
        UInt32_Clear(&a_pValue->ErrorCount);
    }
}

/*============================================================================
 * OpcUa_ServiceCounterDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServiceCounterDataType_Encode(OpcUa_ServiceCounterDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ServiceCounterDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ServiceCounterDataType_Decode(OpcUa_ServiceCounterDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ServiceCounterDataType_Initialize(a_pValue);

    status &= UInt32_Read(&a_pValue->TotalCount, msgBuf);
    status &= UInt32_Read(&a_pValue->ErrorCount, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ServiceCounterDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ServiceCounterDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ServiceCounterDataType_EncodeableType =
{
    "ServiceCounterDataType",
    OpcUaId_ServiceCounterDataType,
    OpcUaId_ServiceCounterDataType_Encoding_DefaultBinary,
    OpcUaId_ServiceCounterDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ServiceCounterDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ServiceCounterDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ServiceCounterDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ServiceCounterDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ServiceCounterDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ServiceCounterDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_StatusResult
/*============================================================================
 * OpcUa_StatusResult_Initialize
 *===========================================================================*/
void OpcUa_StatusResult_Initialize(OpcUa_StatusResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Initialize(&a_pValue->StatusCode);
        DiagnosticInfo_Initialize(&a_pValue->DiagnosticInfo);
    }
}

/*============================================================================
 * OpcUa_StatusResult_Clear
 *===========================================================================*/
void OpcUa_StatusResult_Clear(OpcUa_StatusResult* a_pValue)
{
    if (a_pValue != NULL)
    {
        StatusCode_Clear(&a_pValue->StatusCode);
        DiagnosticInfo_Clear(&a_pValue->DiagnosticInfo);
    }
}

/*============================================================================
 * OpcUa_StatusResult_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_StatusResult_Encode(OpcUa_StatusResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_StatusResult_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_StatusResult_Decode(OpcUa_StatusResult* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_StatusResult_Initialize(a_pValue);

    status &= StatusCode_Read(&a_pValue->StatusCode, msgBuf);
    status &= DiagnosticInfo_Read(&a_pValue->DiagnosticInfo, msgBuf);

    if(status != STATUS_OK){
        OpcUa_StatusResult_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_StatusResult_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_StatusResult_EncodeableType =
{
    "StatusResult",
    OpcUaId_StatusResult,
    OpcUaId_StatusResult_Encoding_DefaultBinary,
    OpcUaId_StatusResult_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_StatusResult),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_StatusResult_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_StatusResult_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_StatusResult_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_StatusResult_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_StatusResult_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SubscriptionDiagnosticsDataType
/*============================================================================
 * OpcUa_SubscriptionDiagnosticsDataType_Initialize
 *===========================================================================*/
void OpcUa_SubscriptionDiagnosticsDataType_Initialize(OpcUa_SubscriptionDiagnosticsDataType* a_pValue)
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
 * OpcUa_SubscriptionDiagnosticsDataType_Clear
 *===========================================================================*/
void OpcUa_SubscriptionDiagnosticsDataType_Clear(OpcUa_SubscriptionDiagnosticsDataType* a_pValue)
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
 * OpcUa_SubscriptionDiagnosticsDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SubscriptionDiagnosticsDataType_Encode(OpcUa_SubscriptionDiagnosticsDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_SubscriptionDiagnosticsDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SubscriptionDiagnosticsDataType_Decode(OpcUa_SubscriptionDiagnosticsDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SubscriptionDiagnosticsDataType_Initialize(a_pValue);

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
        OpcUa_SubscriptionDiagnosticsDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SubscriptionDiagnosticsDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SubscriptionDiagnosticsDataType_EncodeableType =
{
    "SubscriptionDiagnosticsDataType",
    OpcUaId_SubscriptionDiagnosticsDataType,
    OpcUaId_SubscriptionDiagnosticsDataType_Encoding_DefaultBinary,
    OpcUaId_SubscriptionDiagnosticsDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SubscriptionDiagnosticsDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SubscriptionDiagnosticsDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SubscriptionDiagnosticsDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SubscriptionDiagnosticsDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SubscriptionDiagnosticsDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SubscriptionDiagnosticsDataType_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_ModelChangeStructureDataType
/*============================================================================
 * OpcUa_ModelChangeStructureDataType_Initialize
 *===========================================================================*/
void OpcUa_ModelChangeStructureDataType_Initialize(OpcUa_ModelChangeStructureDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->Affected);
        NodeId_Initialize(&a_pValue->AffectedType);
        Byte_Initialize(&a_pValue->Verb);
    }
}

/*============================================================================
 * OpcUa_ModelChangeStructureDataType_Clear
 *===========================================================================*/
void OpcUa_ModelChangeStructureDataType_Clear(OpcUa_ModelChangeStructureDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->Affected);
        NodeId_Clear(&a_pValue->AffectedType);
        Byte_Clear(&a_pValue->Verb);
    }
}

/*============================================================================
 * OpcUa_ModelChangeStructureDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModelChangeStructureDataType_Encode(OpcUa_ModelChangeStructureDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ModelChangeStructureDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ModelChangeStructureDataType_Decode(OpcUa_ModelChangeStructureDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ModelChangeStructureDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->Affected, msgBuf);
    status &= NodeId_Read(&a_pValue->AffectedType, msgBuf);
    status &= Byte_Read(&a_pValue->Verb, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ModelChangeStructureDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ModelChangeStructureDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ModelChangeStructureDataType_EncodeableType =
{
    "ModelChangeStructureDataType",
    OpcUaId_ModelChangeStructureDataType,
    OpcUaId_ModelChangeStructureDataType_Encoding_DefaultBinary,
    OpcUaId_ModelChangeStructureDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ModelChangeStructureDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ModelChangeStructureDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ModelChangeStructureDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ModelChangeStructureDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ModelChangeStructureDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ModelChangeStructureDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_SemanticChangeStructureDataType
/*============================================================================
 * OpcUa_SemanticChangeStructureDataType_Initialize
 *===========================================================================*/
void OpcUa_SemanticChangeStructureDataType_Initialize(OpcUa_SemanticChangeStructureDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->Affected);
        NodeId_Initialize(&a_pValue->AffectedType);
    }
}

/*============================================================================
 * OpcUa_SemanticChangeStructureDataType_Clear
 *===========================================================================*/
void OpcUa_SemanticChangeStructureDataType_Clear(OpcUa_SemanticChangeStructureDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->Affected);
        NodeId_Clear(&a_pValue->AffectedType);
    }
}

/*============================================================================
 * OpcUa_SemanticChangeStructureDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SemanticChangeStructureDataType_Encode(OpcUa_SemanticChangeStructureDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_SemanticChangeStructureDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_SemanticChangeStructureDataType_Decode(OpcUa_SemanticChangeStructureDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_SemanticChangeStructureDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->Affected, msgBuf);
    status &= NodeId_Read(&a_pValue->AffectedType, msgBuf);

    if(status != STATUS_OK){
        OpcUa_SemanticChangeStructureDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_SemanticChangeStructureDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_SemanticChangeStructureDataType_EncodeableType =
{
    "SemanticChangeStructureDataType",
    OpcUaId_SemanticChangeStructureDataType,
    OpcUaId_SemanticChangeStructureDataType_Encoding_DefaultBinary,
    OpcUaId_SemanticChangeStructureDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_SemanticChangeStructureDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_SemanticChangeStructureDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_SemanticChangeStructureDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_SemanticChangeStructureDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_SemanticChangeStructureDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_SemanticChangeStructureDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Range
/*============================================================================
 * OpcUa_Range_Initialize
 *===========================================================================*/
void OpcUa_Range_Initialize(OpcUa_Range* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Initialize(&a_pValue->Low);
        Double_Initialize(&a_pValue->High);
    }
}

/*============================================================================
 * OpcUa_Range_Clear
 *===========================================================================*/
void OpcUa_Range_Clear(OpcUa_Range* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Clear(&a_pValue->Low);
        Double_Clear(&a_pValue->High);
    }
}

/*============================================================================
 * OpcUa_Range_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_Range_Encode(OpcUa_Range* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_Range_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_Range_Decode(OpcUa_Range* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_Range_Initialize(a_pValue);

    status &= Double_Read(&a_pValue->Low, msgBuf);
    status &= Double_Read(&a_pValue->High, msgBuf);

    if(status != STATUS_OK){
        OpcUa_Range_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_Range_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_Range_EncodeableType =
{
    "Range",
    OpcUaId_Range,
    OpcUaId_Range_Encoding_DefaultBinary,
    OpcUaId_Range_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_Range),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_Range_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_Range_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_Range_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_Range_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_Range_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_EUInformation
/*============================================================================
 * OpcUa_EUInformation_Initialize
 *===========================================================================*/
void OpcUa_EUInformation_Initialize(OpcUa_EUInformation* a_pValue)
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
 * OpcUa_EUInformation_Clear
 *===========================================================================*/
void OpcUa_EUInformation_Clear(OpcUa_EUInformation* a_pValue)
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
 * OpcUa_EUInformation_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EUInformation_Encode(OpcUa_EUInformation* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_EUInformation_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_EUInformation_Decode(OpcUa_EUInformation* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_EUInformation_Initialize(a_pValue);

    status &= String_Read(&a_pValue->NamespaceUri, msgBuf);
    status &= Int32_Read(&a_pValue->UnitId, msgBuf);
    status &= LocalizedText_Read(&a_pValue->DisplayName, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Description, msgBuf);

    if(status != STATUS_OK){
        OpcUa_EUInformation_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_EUInformation_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_EUInformation_EncodeableType =
{
    "EUInformation",
    OpcUaId_EUInformation,
    OpcUaId_EUInformation_Encoding_DefaultBinary,
    OpcUaId_EUInformation_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_EUInformation),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_EUInformation_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_EUInformation_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_EUInformation_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_EUInformation_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_EUInformation_Decode
};
#endif


#ifndef OPCUA_EXCLUDE_ComplexNumberType
/*============================================================================
 * OpcUa_ComplexNumberType_Initialize
 *===========================================================================*/
void OpcUa_ComplexNumberType_Initialize(OpcUa_ComplexNumberType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Float_Initialize(&a_pValue->Real);
        Float_Initialize(&a_pValue->Imaginary);
    }
}

/*============================================================================
 * OpcUa_ComplexNumberType_Clear
 *===========================================================================*/
void OpcUa_ComplexNumberType_Clear(OpcUa_ComplexNumberType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Float_Clear(&a_pValue->Real);
        Float_Clear(&a_pValue->Imaginary);
    }
}

/*============================================================================
 * OpcUa_ComplexNumberType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ComplexNumberType_Encode(OpcUa_ComplexNumberType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_ComplexNumberType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ComplexNumberType_Decode(OpcUa_ComplexNumberType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ComplexNumberType_Initialize(a_pValue);

    status &= Float_Read(&a_pValue->Real, msgBuf);
    status &= Float_Read(&a_pValue->Imaginary, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ComplexNumberType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ComplexNumberType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ComplexNumberType_EncodeableType =
{
    "ComplexNumberType",
    OpcUaId_ComplexNumberType,
    OpcUaId_ComplexNumberType_Encoding_DefaultBinary,
    OpcUaId_ComplexNumberType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ComplexNumberType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ComplexNumberType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ComplexNumberType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ComplexNumberType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ComplexNumberType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ComplexNumberType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_DoubleComplexNumberType
/*============================================================================
 * OpcUa_DoubleComplexNumberType_Initialize
 *===========================================================================*/
void OpcUa_DoubleComplexNumberType_Initialize(OpcUa_DoubleComplexNumberType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Initialize(&a_pValue->Real);
        Double_Initialize(&a_pValue->Imaginary);
    }
}

/*============================================================================
 * OpcUa_DoubleComplexNumberType_Clear
 *===========================================================================*/
void OpcUa_DoubleComplexNumberType_Clear(OpcUa_DoubleComplexNumberType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Clear(&a_pValue->Real);
        Double_Clear(&a_pValue->Imaginary);
    }
}

/*============================================================================
 * OpcUa_DoubleComplexNumberType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DoubleComplexNumberType_Encode(OpcUa_DoubleComplexNumberType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_DoubleComplexNumberType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_DoubleComplexNumberType_Decode(OpcUa_DoubleComplexNumberType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_DoubleComplexNumberType_Initialize(a_pValue);

    status &= Double_Read(&a_pValue->Real, msgBuf);
    status &= Double_Read(&a_pValue->Imaginary, msgBuf);

    if(status != STATUS_OK){
        OpcUa_DoubleComplexNumberType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_DoubleComplexNumberType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_DoubleComplexNumberType_EncodeableType =
{
    "DoubleComplexNumberType",
    OpcUaId_DoubleComplexNumberType,
    OpcUaId_DoubleComplexNumberType_Encoding_DefaultBinary,
    OpcUaId_DoubleComplexNumberType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_DoubleComplexNumberType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_DoubleComplexNumberType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_DoubleComplexNumberType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_DoubleComplexNumberType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_DoubleComplexNumberType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_DoubleComplexNumberType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_AxisInformation
/*============================================================================
 * OpcUa_AxisInformation_Initialize
 *===========================================================================*/
void OpcUa_AxisInformation_Initialize(OpcUa_AxisInformation* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_EUInformation_Initialize(&a_pValue->EngineeringUnits);
        OpcUa_Range_Initialize(&a_pValue->EURange);
        LocalizedText_Initialize(&a_pValue->Title);
        SOPC_Initialize_EnumeratedType((int32_t*) &a_pValue->AxisScaleType);
        SOPC_Initialize_Array(&a_pValue->NoOfAxisSteps, (void**) &a_pValue->AxisSteps, 
                            sizeof(double), (SOPC_EncodeableObject_PfnInitialize*) Double_Initialize);
    }
}

/*============================================================================
 * OpcUa_AxisInformation_Clear
 *===========================================================================*/
void OpcUa_AxisInformation_Clear(OpcUa_AxisInformation* a_pValue)
{
    if (a_pValue != NULL)
    {
        OpcUa_EUInformation_Clear(&a_pValue->EngineeringUnits);
        OpcUa_Range_Clear(&a_pValue->EURange);
        LocalizedText_Clear(&a_pValue->Title);
        SOPC_Clear_EnumeratedType((int32_t*) &a_pValue->AxisScaleType);
        SOPC_Clear_Array(&a_pValue->NoOfAxisSteps, (void**) &a_pValue->AxisSteps, 
                       sizeof(double), (SOPC_EncodeableObject_PfnClear*) Double_Clear);
    }
}

/*============================================================================
 * OpcUa_AxisInformation_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AxisInformation_Encode(OpcUa_AxisInformation* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= OpcUa_EUInformation_Encode(&a_pValue->EngineeringUnits, msgBuf);
    status &= OpcUa_Range_Encode(&a_pValue->EURange, msgBuf);
    status &= LocalizedText_Write(&a_pValue->Title, msgBuf);
    status &= SOPC_Write_EnumeratedType(msgBuf, (int32_t*) &a_pValue->AxisScaleType);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfAxisSteps, (void**) &a_pValue->AxisSteps, 
                   sizeof(double), (SOPC_EncodeableObject_PfnEncode*) Double_Write);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_AxisInformation_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_AxisInformation_Decode(OpcUa_AxisInformation* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_AxisInformation_Initialize(a_pValue);

    status &= OpcUa_EUInformation_Decode(&a_pValue->EngineeringUnits, msgBuf);
    status &= OpcUa_Range_Decode(&a_pValue->EURange, msgBuf);
    status &= LocalizedText_Read(&a_pValue->Title, msgBuf);
    status &= SOPC_Read_EnumeratedType(msgBuf, (int32_t*) &a_pValue->AxisScaleType);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfAxisSteps, (void**) &a_pValue->AxisSteps, 
                  sizeof(double), (SOPC_EncodeableObject_PfnDecode*) Double_Read);

    if(status != STATUS_OK){
        OpcUa_AxisInformation_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_AxisInformation_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_AxisInformation_EncodeableType =
{
    "AxisInformation",
    OpcUaId_AxisInformation,
    OpcUaId_AxisInformation_Encoding_DefaultBinary,
    OpcUaId_AxisInformation_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_AxisInformation),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_AxisInformation_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_AxisInformation_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_AxisInformation_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_AxisInformation_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_AxisInformation_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_XVType
/*============================================================================
 * OpcUa_XVType_Initialize
 *===========================================================================*/
void OpcUa_XVType_Initialize(OpcUa_XVType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Initialize(&a_pValue->X);
        Float_Initialize(&a_pValue->Value);
    }
}

/*============================================================================
 * OpcUa_XVType_Clear
 *===========================================================================*/
void OpcUa_XVType_Clear(OpcUa_XVType* a_pValue)
{
    if (a_pValue != NULL)
    {
        Double_Clear(&a_pValue->X);
        Float_Clear(&a_pValue->Value);
    }
}

/*============================================================================
 * OpcUa_XVType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_XVType_Encode(OpcUa_XVType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_XVType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_XVType_Decode(OpcUa_XVType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_XVType_Initialize(a_pValue);

    status &= Double_Read(&a_pValue->X, msgBuf);
    status &= Float_Read(&a_pValue->Value, msgBuf);

    if(status != STATUS_OK){
        OpcUa_XVType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_XVType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_XVType_EncodeableType =
{
    "XVType",
    OpcUaId_XVType,
    OpcUaId_XVType_Encoding_DefaultBinary,
    OpcUaId_XVType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_XVType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_XVType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_XVType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_XVType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_XVType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_XVType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_ProgramDiagnosticDataType
/*============================================================================
 * OpcUa_ProgramDiagnosticDataType_Initialize
 *===========================================================================*/
void OpcUa_ProgramDiagnosticDataType_Initialize(OpcUa_ProgramDiagnosticDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Initialize(&a_pValue->CreateSessionId);
        String_Initialize(&a_pValue->CreateClientName);
        DateTime_Initialize(&a_pValue->InvocationCreationTime);
        DateTime_Initialize(&a_pValue->LastTransitionTime);
        String_Initialize(&a_pValue->LastMethodCall);
        NodeId_Initialize(&a_pValue->LastMethodSessionId);
        SOPC_Initialize_Array(&a_pValue->NoOfLastMethodInputArguments, (void**) &a_pValue->LastMethodInputArguments, 
                            sizeof(OpcUa_Argument), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_Argument_Initialize);
        SOPC_Initialize_Array(&a_pValue->NoOfLastMethodOutputArguments, (void**) &a_pValue->LastMethodOutputArguments, 
                            sizeof(OpcUa_Argument), (SOPC_EncodeableObject_PfnInitialize*) OpcUa_Argument_Initialize);
        DateTime_Initialize(&a_pValue->LastMethodCallTime);
        OpcUa_StatusResult_Initialize(&a_pValue->LastMethodReturnStatus);
    }
}

/*============================================================================
 * OpcUa_ProgramDiagnosticDataType_Clear
 *===========================================================================*/
void OpcUa_ProgramDiagnosticDataType_Clear(OpcUa_ProgramDiagnosticDataType* a_pValue)
{
    if (a_pValue != NULL)
    {
        NodeId_Clear(&a_pValue->CreateSessionId);
        String_Clear(&a_pValue->CreateClientName);
        DateTime_Clear(&a_pValue->InvocationCreationTime);
        DateTime_Clear(&a_pValue->LastTransitionTime);
        String_Clear(&a_pValue->LastMethodCall);
        NodeId_Clear(&a_pValue->LastMethodSessionId);
        SOPC_Clear_Array(&a_pValue->NoOfLastMethodInputArguments, (void**) &a_pValue->LastMethodInputArguments, 
                       sizeof(OpcUa_Argument), (SOPC_EncodeableObject_PfnClear*) OpcUa_Argument_Clear);
        SOPC_Clear_Array(&a_pValue->NoOfLastMethodOutputArguments, (void**) &a_pValue->LastMethodOutputArguments, 
                       sizeof(OpcUa_Argument), (SOPC_EncodeableObject_PfnClear*) OpcUa_Argument_Clear);
        DateTime_Clear(&a_pValue->LastMethodCallTime);
        OpcUa_StatusResult_Clear(&a_pValue->LastMethodReturnStatus);
    }
}

/*============================================================================
 * OpcUa_ProgramDiagnosticDataType_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ProgramDiagnosticDataType_Encode(OpcUa_ProgramDiagnosticDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    status &= NodeId_Write(&a_pValue->CreateSessionId, msgBuf);
    status &= String_Write(&a_pValue->CreateClientName, msgBuf);
    status &= DateTime_Write(&a_pValue->InvocationCreationTime, msgBuf);
    status &= DateTime_Write(&a_pValue->LastTransitionTime, msgBuf);
    status &= String_Write(&a_pValue->LastMethodCall, msgBuf);
    status &= NodeId_Write(&a_pValue->LastMethodSessionId, msgBuf);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfLastMethodInputArguments, (void**) &a_pValue->LastMethodInputArguments, 
                   sizeof(OpcUa_Argument), (SOPC_EncodeableObject_PfnEncode*) OpcUa_Argument_Encode);
    status &= SOPC_Write_Array(msgBuf, &a_pValue->NoOfLastMethodOutputArguments, (void**) &a_pValue->LastMethodOutputArguments, 
                   sizeof(OpcUa_Argument), (SOPC_EncodeableObject_PfnEncode*) OpcUa_Argument_Encode);
    status &= DateTime_Write(&a_pValue->LastMethodCallTime, msgBuf);
    status &= OpcUa_StatusResult_Encode(&a_pValue->LastMethodReturnStatus, msgBuf);

    if((status & STATUS_NOK) != 0){
        status = STATUS_NOK;
    }

    return status;
}

/*============================================================================
 * OpcUa_ProgramDiagnosticDataType_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_ProgramDiagnosticDataType_Decode(OpcUa_ProgramDiagnosticDataType* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_ProgramDiagnosticDataType_Initialize(a_pValue);

    status &= NodeId_Read(&a_pValue->CreateSessionId, msgBuf);
    status &= String_Read(&a_pValue->CreateClientName, msgBuf);
    status &= DateTime_Read(&a_pValue->InvocationCreationTime, msgBuf);
    status &= DateTime_Read(&a_pValue->LastTransitionTime, msgBuf);
    status &= String_Read(&a_pValue->LastMethodCall, msgBuf);
    status &= NodeId_Read(&a_pValue->LastMethodSessionId, msgBuf);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfLastMethodInputArguments, (void**) &a_pValue->LastMethodInputArguments, 
                  sizeof(OpcUa_Argument), (SOPC_EncodeableObject_PfnDecode*) OpcUa_Argument_Decode);
    status &= SOPC_Read_Array(msgBuf, &a_pValue->NoOfLastMethodOutputArguments, (void**) &a_pValue->LastMethodOutputArguments, 
                  sizeof(OpcUa_Argument), (SOPC_EncodeableObject_PfnDecode*) OpcUa_Argument_Decode);
    status &= DateTime_Read(&a_pValue->LastMethodCallTime, msgBuf);
    status &= OpcUa_StatusResult_Decode(&a_pValue->LastMethodReturnStatus, msgBuf);

    if(status != STATUS_OK){
        OpcUa_ProgramDiagnosticDataType_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_ProgramDiagnosticDataType_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_ProgramDiagnosticDataType_EncodeableType =
{
    "ProgramDiagnosticDataType",
    OpcUaId_ProgramDiagnosticDataType,
    OpcUaId_ProgramDiagnosticDataType_Encoding_DefaultBinary,
    OpcUaId_ProgramDiagnosticDataType_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_ProgramDiagnosticDataType),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_ProgramDiagnosticDataType_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_ProgramDiagnosticDataType_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_ProgramDiagnosticDataType_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_ProgramDiagnosticDataType_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_ProgramDiagnosticDataType_Decode
};
#endif

#ifndef OPCUA_EXCLUDE_Annotation
/*============================================================================
 * OpcUa_Annotation_Initialize
 *===========================================================================*/
void OpcUa_Annotation_Initialize(OpcUa_Annotation* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Initialize(&a_pValue->Message);
        String_Initialize(&a_pValue->UserName);
        DateTime_Initialize(&a_pValue->AnnotationTime);
    }
}

/*============================================================================
 * OpcUa_Annotation_Clear
 *===========================================================================*/
void OpcUa_Annotation_Clear(OpcUa_Annotation* a_pValue)
{
    if (a_pValue != NULL)
    {
        String_Clear(&a_pValue->Message);
        String_Clear(&a_pValue->UserName);
        DateTime_Clear(&a_pValue->AnnotationTime);
    }
}

/*============================================================================
 * OpcUa_Annotation_Encode
 *===========================================================================*/
SOPC_StatusCode OpcUa_Annotation_Encode(OpcUa_Annotation* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
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
 * OpcUa_Annotation_Decode
 *===========================================================================*/
SOPC_StatusCode OpcUa_Annotation_Decode(OpcUa_Annotation* a_pValue, SOPC_MsgBuffer* msgBuf)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    
    if(a_pValue != NULL){
        status = STATUS_OK;
    }

    OpcUa_Annotation_Initialize(a_pValue);

    status &= String_Read(&a_pValue->Message, msgBuf);
    status &= String_Read(&a_pValue->UserName, msgBuf);
    status &= DateTime_Read(&a_pValue->AnnotationTime, msgBuf);

    if(status != STATUS_OK){
        OpcUa_Annotation_Clear(a_pValue);
    }

    return status;
}

/*============================================================================
 * OpcUa_Annotation_EncodeableType
 *===========================================================================*/
struct SOPC_EncodeableType OpcUa_Annotation_EncodeableType =
{
    "Annotation",
    OpcUaId_Annotation,
    OpcUaId_Annotation_Encoding_DefaultBinary,
    OpcUaId_Annotation_Encoding_DefaultXml,
    NULL,
    sizeof(OpcUa_Annotation),
    (SOPC_EncodeableObject_PfnInitialize*)OpcUa_Annotation_Initialize,
    (SOPC_EncodeableObject_PfnClear*)OpcUa_Annotation_Clear,
    (SOPC_EncodeableObject_PfnGetSize*)NULL,
//    (SOPC_EncodeableObject_PfnGetSize*)OpcUa_Annotation_GetSize,
    (SOPC_EncodeableObject_PfnEncode*)OpcUa_Annotation_Encode,
    (SOPC_EncodeableObject_PfnDecode*)OpcUa_Annotation_Decode
};
#endif


void SOPC_Initialize_EnumeratedType(int32_t* enumerationValue)
{
    *enumerationValue = 0;
}

void SOPC_Initialize_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                           SOPC_EncodeableObject_PfnInitialize* initFct)
{
    (void) initFct;
    (void) sizeOfElt;
    *noOfElts = 0;
    *eltsArray = NULL;
}

void SOPC_Clear_EnumeratedType(int32_t* enumerationValue){
    *enumerationValue = 0;
}

void SOPC_Clear_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                      SOPC_EncodeableObject_PfnClear* clearFct)
{
    int32_t idx = 0;
    uint32_t pos = 0;
    SOPC_Byte* byteArray = *eltsArray;
    for (idx = 0; idx < *noOfElts; idx ++){
        pos = idx * sizeOfElt;
        clearFct(&(byteArray[pos]));
    }
    
    free(*eltsArray);
    
    *noOfElts = 0;
    *eltsArray = NULL;
}

SOPC_StatusCode SOPC_Read_EnumeratedType(SOPC_MsgBuffer* msgBuffer, int32_t* enumerationValue){
    return Int32_Read(enumerationValue, msgBuffer);
}

SOPC_StatusCode SOPC_Read_Array(SOPC_MsgBuffer* msgBuffer, int32_t* noOfElts, void** eltsArray,
                                size_t sizeOfElt, SOPC_EncodeableObject_PfnDecode* decodeFct)
{
    assert(msgBuffer != NULL && *eltsArray == NULL && noOfElts != NULL);
    SOPC_StatusCode status = STATUS_OK;
    SOPC_Byte* byteArray = *eltsArray;
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
                    
SOPC_StatusCode SOPC_Write_EnumeratedType(SOPC_MsgBuffer* msgBuffer, int32_t* enumerationValue){
    return Int32_Write(enumerationValue, msgBuffer);
}

SOPC_StatusCode SOPC_Write_Array(SOPC_MsgBuffer* msgBuffer, int32_t* noOfElts, void** eltsArray,
                                 size_t sizeOfElt, SOPC_EncodeableObject_PfnEncode* encodeFct){
    assert(msgBuffer != NULL && *eltsArray != NULL && noOfElts != NULL);
    SOPC_StatusCode status = STATUS_OK;
    SOPC_Byte* byteArray = *eltsArray;
    
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
static SOPC_EncodeableType* g_KnownEncodeableTypes[] =
{
    #ifndef OPCUA_EXCLUDE_Node
    &OpcUa_Node_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_InstanceNode
    &OpcUa_InstanceNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TypeNode
    &OpcUa_TypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ObjectNode
    &OpcUa_ObjectNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ObjectTypeNode
    &OpcUa_ObjectTypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_VariableNode
    &OpcUa_VariableNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_VariableTypeNode
    &OpcUa_VariableTypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReferenceTypeNode
    &OpcUa_ReferenceTypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MethodNode
    &OpcUa_MethodNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ViewNode
    &OpcUa_ViewNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DataTypeNode
    &OpcUa_DataTypeNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReferenceNode
    &OpcUa_ReferenceNode_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Argument
    &OpcUa_Argument_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EnumValueType
    &OpcUa_EnumValueType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EnumField
    &OpcUa_EnumField_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_OptionSet
    &OpcUa_OptionSet_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TimeZoneDataType
    &OpcUa_TimeZoneDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ApplicationDescription
    &OpcUa_ApplicationDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RequestHeader
    &OpcUa_RequestHeader_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ResponseHeader
    &OpcUa_ResponseHeader_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServiceFault
    &OpcUa_ServiceFault_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_FindServers
    &OpcUa_FindServersRequest_EncodeableType,
    &OpcUa_FindServersResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServerOnNetwork
    &OpcUa_ServerOnNetwork_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_FindServersOnNetwork
    &OpcUa_FindServersOnNetworkRequest_EncodeableType,
    &OpcUa_FindServersOnNetworkResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UserTokenPolicy
    &OpcUa_UserTokenPolicy_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EndpointDescription
    &OpcUa_EndpointDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_GetEndpoints
    &OpcUa_GetEndpointsRequest_EncodeableType,
    &OpcUa_GetEndpointsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisteredServer
    &OpcUa_RegisteredServer_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisterServer
    &OpcUa_RegisterServerRequest_EncodeableType,
    &OpcUa_RegisterServerResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MdnsDiscoveryConfiguration
    &OpcUa_MdnsDiscoveryConfiguration_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisterServer2
    &OpcUa_RegisterServer2Request_EncodeableType,
    &OpcUa_RegisterServer2Response_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ChannelSecurityToken
    &OpcUa_ChannelSecurityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_OpenSecureChannel
    &OpcUa_OpenSecureChannelRequest_EncodeableType,
    &OpcUa_OpenSecureChannelResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CloseSecureChannel
    &OpcUa_CloseSecureChannelRequest_EncodeableType,
    &OpcUa_CloseSecureChannelResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SignedSoftwareCertificate
    &OpcUa_SignedSoftwareCertificate_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SignatureData
    &OpcUa_SignatureData_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CreateSession
    &OpcUa_CreateSessionRequest_EncodeableType,
    &OpcUa_CreateSessionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UserIdentityToken
    &OpcUa_UserIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AnonymousIdentityToken
    &OpcUa_AnonymousIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UserNameIdentityToken
    &OpcUa_UserNameIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_X509IdentityToken
    &OpcUa_X509IdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_KerberosIdentityToken
    &OpcUa_KerberosIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_IssuedIdentityToken
    &OpcUa_IssuedIdentityToken_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ActivateSession
    &OpcUa_ActivateSessionRequest_EncodeableType,
    &OpcUa_ActivateSessionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CloseSession
    &OpcUa_CloseSessionRequest_EncodeableType,
    &OpcUa_CloseSessionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Cancel
    &OpcUa_CancelRequest_EncodeableType,
    &OpcUa_CancelResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NodeAttributes
    &OpcUa_NodeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ObjectAttributes
    &OpcUa_ObjectAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_VariableAttributes
    &OpcUa_VariableAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MethodAttributes
    &OpcUa_MethodAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ObjectTypeAttributes
    &OpcUa_ObjectTypeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_VariableTypeAttributes
    &OpcUa_VariableTypeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReferenceTypeAttributes
    &OpcUa_ReferenceTypeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DataTypeAttributes
    &OpcUa_DataTypeAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ViewAttributes
    &OpcUa_ViewAttributes_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddNodesItem
    &OpcUa_AddNodesItem_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddNodesResult
    &OpcUa_AddNodesResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddNodes
    &OpcUa_AddNodesRequest_EncodeableType,
    &OpcUa_AddNodesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddReferencesItem
    &OpcUa_AddReferencesItem_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AddReferences
    &OpcUa_AddReferencesRequest_EncodeableType,
    &OpcUa_AddReferencesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteNodesItem
    &OpcUa_DeleteNodesItem_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteNodes
    &OpcUa_DeleteNodesRequest_EncodeableType,
    &OpcUa_DeleteNodesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteReferencesItem
    &OpcUa_DeleteReferencesItem_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteReferences
    &OpcUa_DeleteReferencesRequest_EncodeableType,
    &OpcUa_DeleteReferencesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ViewDescription
    &OpcUa_ViewDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowseDescription
    &OpcUa_BrowseDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReferenceDescription
    &OpcUa_ReferenceDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowseResult
    &OpcUa_BrowseResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Browse
    &OpcUa_BrowseRequest_EncodeableType,
    &OpcUa_BrowseResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowseNext
    &OpcUa_BrowseNextRequest_EncodeableType,
    &OpcUa_BrowseNextResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RelativePathElement
    &OpcUa_RelativePathElement_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RelativePath
    &OpcUa_RelativePath_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowsePath
    &OpcUa_BrowsePath_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowsePathTarget
    &OpcUa_BrowsePathTarget_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BrowsePathResult
    &OpcUa_BrowsePathResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TranslateBrowsePathsToNodeIds
    &OpcUa_TranslateBrowsePathsToNodeIdsRequest_EncodeableType,
    &OpcUa_TranslateBrowsePathsToNodeIdsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RegisterNodes
    &OpcUa_RegisterNodesRequest_EncodeableType,
    &OpcUa_RegisterNodesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UnregisterNodes
    &OpcUa_UnregisterNodesRequest_EncodeableType,
    &OpcUa_UnregisterNodesResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EndpointConfiguration
    &OpcUa_EndpointConfiguration_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SupportedProfile
    &OpcUa_SupportedProfile_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SoftwareCertificate
    &OpcUa_SoftwareCertificate_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryDataDescription
    &OpcUa_QueryDataDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NodeTypeDescription
    &OpcUa_NodeTypeDescription_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryDataSet
    &OpcUa_QueryDataSet_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NodeReference
    &OpcUa_NodeReference_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ContentFilterElement
    &OpcUa_ContentFilterElement_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ContentFilter
    &OpcUa_ContentFilter_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ElementOperand
    &OpcUa_ElementOperand_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_LiteralOperand
    &OpcUa_LiteralOperand_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AttributeOperand
    &OpcUa_AttributeOperand_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SimpleAttributeOperand
    &OpcUa_SimpleAttributeOperand_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ContentFilterElementResult
    &OpcUa_ContentFilterElementResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ContentFilterResult
    &OpcUa_ContentFilterResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ParsingResult
    &OpcUa_ParsingResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryFirst
    &OpcUa_QueryFirstRequest_EncodeableType,
    &OpcUa_QueryFirstResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_QueryNext
    &OpcUa_QueryNextRequest_EncodeableType,
    &OpcUa_QueryNextResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadValueId
    &OpcUa_ReadValueId_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Read
    &OpcUa_ReadRequest_EncodeableType,
    &OpcUa_ReadResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryReadValueId
    &OpcUa_HistoryReadValueId_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryReadResult
    &OpcUa_HistoryReadResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadEventDetails
    &OpcUa_ReadEventDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadRawModifiedDetails
    &OpcUa_ReadRawModifiedDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadProcessedDetails
    &OpcUa_ReadProcessedDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ReadAtTimeDetails
    &OpcUa_ReadAtTimeDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryData
    &OpcUa_HistoryData_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModificationInfo
    &OpcUa_ModificationInfo_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryModifiedData
    &OpcUa_HistoryModifiedData_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryEvent
    &OpcUa_HistoryEvent_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryRead
    &OpcUa_HistoryReadRequest_EncodeableType,
    &OpcUa_HistoryReadResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_WriteValue
    &OpcUa_WriteValue_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Write
    &OpcUa_WriteRequest_EncodeableType,
    &OpcUa_WriteResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryUpdateDetails
    &OpcUa_HistoryUpdateDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UpdateDataDetails
    &OpcUa_UpdateDataDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UpdateStructureDataDetails
    &OpcUa_UpdateStructureDataDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_UpdateEventDetails
    &OpcUa_UpdateEventDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteRawModifiedDetails
    &OpcUa_DeleteRawModifiedDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteAtTimeDetails
    &OpcUa_DeleteAtTimeDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteEventDetails
    &OpcUa_DeleteEventDetails_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryUpdateResult
    &OpcUa_HistoryUpdateResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryUpdate
    &OpcUa_HistoryUpdateRequest_EncodeableType,
    &OpcUa_HistoryUpdateResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CallMethodRequest
    &OpcUa_CallMethodRequest_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CallMethodResult
    &OpcUa_CallMethodResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Call
    &OpcUa_CallRequest_EncodeableType,
    &OpcUa_CallResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DataChangeFilter
    &OpcUa_DataChangeFilter_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EventFilter
    &OpcUa_EventFilter_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AggregateConfiguration
    &OpcUa_AggregateConfiguration_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AggregateFilter
    &OpcUa_AggregateFilter_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EventFilterResult
    &OpcUa_EventFilterResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AggregateFilterResult
    &OpcUa_AggregateFilterResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoringParameters
    &OpcUa_MonitoringParameters_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemCreateRequest
    &OpcUa_MonitoredItemCreateRequest_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemCreateResult
    &OpcUa_MonitoredItemCreateResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CreateMonitoredItems
    &OpcUa_CreateMonitoredItemsRequest_EncodeableType,
    &OpcUa_CreateMonitoredItemsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemModifyRequest
    &OpcUa_MonitoredItemModifyRequest_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemModifyResult
    &OpcUa_MonitoredItemModifyResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModifyMonitoredItems
    &OpcUa_ModifyMonitoredItemsRequest_EncodeableType,
    &OpcUa_ModifyMonitoredItemsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SetMonitoringMode
    &OpcUa_SetMonitoringModeRequest_EncodeableType,
    &OpcUa_SetMonitoringModeResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SetTriggering
    &OpcUa_SetTriggeringRequest_EncodeableType,
    &OpcUa_SetTriggeringResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteMonitoredItems
    &OpcUa_DeleteMonitoredItemsRequest_EncodeableType,
    &OpcUa_DeleteMonitoredItemsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_CreateSubscription
    &OpcUa_CreateSubscriptionRequest_EncodeableType,
    &OpcUa_CreateSubscriptionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModifySubscription
    &OpcUa_ModifySubscriptionRequest_EncodeableType,
    &OpcUa_ModifySubscriptionResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SetPublishingMode
    &OpcUa_SetPublishingModeRequest_EncodeableType,
    &OpcUa_SetPublishingModeResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NotificationMessage
    &OpcUa_NotificationMessage_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DataChangeNotification
    &OpcUa_DataChangeNotification_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_MonitoredItemNotification
    &OpcUa_MonitoredItemNotification_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EventNotificationList
    &OpcUa_EventNotificationList_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EventFieldList
    &OpcUa_EventFieldList_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_HistoryEventFieldList
    &OpcUa_HistoryEventFieldList_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_StatusChangeNotification
    &OpcUa_StatusChangeNotification_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SubscriptionAcknowledgement
    &OpcUa_SubscriptionAcknowledgement_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Publish
    &OpcUa_PublishRequest_EncodeableType,
    &OpcUa_PublishResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Republish
    &OpcUa_RepublishRequest_EncodeableType,
    &OpcUa_RepublishResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TransferResult
    &OpcUa_TransferResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_TransferSubscriptions
    &OpcUa_TransferSubscriptionsRequest_EncodeableType,
    &OpcUa_TransferSubscriptionsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DeleteSubscriptions
    &OpcUa_DeleteSubscriptionsRequest_EncodeableType,
    &OpcUa_DeleteSubscriptionsResponse_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_BuildInfo
    &OpcUa_BuildInfo_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_RedundantServerDataType
    &OpcUa_RedundantServerDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EndpointUrlListDataType
    &OpcUa_EndpointUrlListDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_NetworkGroupDataType
    &OpcUa_NetworkGroupDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SamplingIntervalDiagnosticsDataType
    &OpcUa_SamplingIntervalDiagnosticsDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServerDiagnosticsSummaryDataType
    &OpcUa_ServerDiagnosticsSummaryDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServerStatusDataType
    &OpcUa_ServerStatusDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SessionDiagnosticsDataType
    &OpcUa_SessionDiagnosticsDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SessionSecurityDiagnosticsDataType
    &OpcUa_SessionSecurityDiagnosticsDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ServiceCounterDataType
    &OpcUa_ServiceCounterDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_StatusResult
    &OpcUa_StatusResult_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SubscriptionDiagnosticsDataType
    &OpcUa_SubscriptionDiagnosticsDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ModelChangeStructureDataType
    &OpcUa_ModelChangeStructureDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_SemanticChangeStructureDataType
    &OpcUa_SemanticChangeStructureDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Range
    &OpcUa_Range_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_EUInformation
    &OpcUa_EUInformation_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ComplexNumberType
    &OpcUa_ComplexNumberType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_DoubleComplexNumberType
    &OpcUa_DoubleComplexNumberType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_AxisInformation
    &OpcUa_AxisInformation_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_XVType
    &OpcUa_XVType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_ProgramDiagnosticDataType
    &OpcUa_ProgramDiagnosticDataType_EncodeableType,
    #endif
    #ifndef OPCUA_EXCLUDE_Annotation
    &OpcUa_Annotation_EncodeableType,
    #endif
    NULL
};

SOPC_EncodeableType** SOPC_KnownEncodeableTypes = g_KnownEncodeableTypes;
/* This is the last line of an autogenerated file. */
