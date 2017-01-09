/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "wrapper_builtintypes.h"

#include <assert.h>
#include <string.h>

void OpcUa_SByte_Initialize(SOPC_SByte* sbyte){
    SOPC_SByte_Initialize(sbyte);
}
void OpcUa_SByte_Clear(SOPC_SByte* sbyte){
    SOPC_SByte_Clear(sbyte);
}
void OpcUa_Byte_Initialize(SOPC_Byte* byte){
    SOPC_Byte_Initialize(byte);
}
void OpcUa_Byte_Clear(SOPC_Byte* byte){
    SOPC_Byte_Clear(byte);
}
void OpcUa_Int16_Initialize(int16_t* intv){
    SOPC_Int16_Initialize(intv);
}
void OpcUa_Int16_Clear(int16_t* intv){
    SOPC_Int16_Clear(intv);
}
void OpcUa_UInt16_Initialize(uint16_t* uint){
    SOPC_UInt16_Initialize(uint);
}
void OpcUa_UInt16_Clear(uint16_t* uint){
    SOPC_UInt16_Clear(uint);
}
void OpcUa_Int32_Initialize(int32_t* intv){
    SOPC_Int32_Initialize(intv);
}
void OpcUa_Int32_Clear(int32_t* intv){
    SOPC_Int32_Clear(intv);
}
void OpcUa_UInt32_Initialize(uint32_t* uintv){
    SOPC_UInt32_Initialize(uintv);
}
void OpcUa_UInt32_Clear(uint32_t* uintv){
    SOPC_UInt32_Clear(uintv);
}
void OpcUa_Int64_Initialize(int64_t* intv){
    SOPC_Int64_Initialize(intv);
}
void OpcUa_Int64_Clear(int64_t* intv){
    SOPC_Int64_Clear(intv);
}
void OpcUa_UInt64_Initialize(uint64_t* uintv){
    SOPC_UInt64_Initialize(uintv);
}
void OpcUa_UInt64_Clear(uint64_t* uintv){
    SOPC_UInt64_Clear(uintv);
}
void OpcUa_Float_Initialize(float* f){
    SOPC_Float_Initialize(f);
}
void OpcUa_Float_Clear(float* f){
    SOPC_Float_Clear(f);
}
void OpcUa_Double_Initialize(double* d){
    SOPC_Double_Initialize(d);
}
void OpcUa_Double_Clear(double* d){
    SOPC_Double_Clear(d);
}
void OpcUa_ByteString_Initialize(SOPC_ByteString* bstring){
    SOPC_ByteString_Initialize(bstring);
}
void OpcUa_ByteString_Clear(SOPC_ByteString* bstring){
    SOPC_ByteString_Clear(bstring);
}
void OpcUa_XmlElement_Initialize(SOPC_XmlElement* xmlElt){
    SOPC_XmlElement_Initialize(xmlElt);
}
void OpcUa_XmlElement_Clear(SOPC_XmlElement* xmlElt){
    SOPC_XmlElement_Clear(xmlElt);
}
void OpcUa_DateTime_Initialize(SOPC_DateTime* dateTime){
    SOPC_DateTime_Initialize(dateTime);
}
void OpcUa_DateTime_Clear(SOPC_DateTime* dateTime){
    SOPC_DateTime_Clear(dateTime);
}
void OpcUa_Guid_Initialize(SOPC_Guid* guid){
    SOPC_Guid_Initialize(guid);
}
void OpcUa_Guid_Clear(SOPC_Guid* guid){
    SOPC_Guid_Clear(guid);
}
void OpcUa_NodeId_Initialize(SOPC_NodeId* nodeId){
    SOPC_NodeId_Initialize(nodeId);
}
void OpcUa_NodeId_Clear(SOPC_NodeId* nodeId){
    SOPC_NodeId_Clear(nodeId);
}
int OpcUa_NodeId_IsNull(SOPC_NodeId* nodeId){
    SOPC_Byte nullBytes[sizeof(SOPC_NodeId)];
    if(nodeId != NULL){
        assert(nullBytes == memset(nullBytes, 0, sizeof(SOPC_NodeId)));
        return memcmp(nullBytes, nodeId, sizeof(SOPC_NodeId));
    }else{
        return 1;
    }
}
void OpcUa_ExpandedNodeId_Initialize(SOPC_ExpandedNodeId* expNodeId){
    SOPC_ExpandedNodeId_Initialize(expNodeId);
}
void OpcUa_ExpandedNodeId_Clear(SOPC_ExpandedNodeId* expNodeId){
    SOPC_ExpandedNodeId_Clear(expNodeId);
}
void OpcUa_StatusCode_Initialize(SOPC_StatusCode* status){
    SOPC_StatusCode_Initialize(status);
}
void OpcUa_StatusCode_Clear(SOPC_StatusCode* status){
    SOPC_StatusCode_Clear(status);
}
void OpcUa_DiagnosticInfo_Initialize(SOPC_DiagnosticInfo* diagInfo){
    SOPC_DiagnosticInfo_Initialize(diagInfo);
}
void OpcUa_DiagnosticInfo_Clear(SOPC_DiagnosticInfo* diagInfo){
    SOPC_DiagnosticInfo_Clear(diagInfo);
}
void OpcUa_QualifiedName_Initialize(SOPC_QualifiedName* qname){
    SOPC_QualifiedName_Initialize(qname);
}
void OpcUa_QualifiedName_Clear(SOPC_QualifiedName* qname){
    SOPC_QualifiedName_Clear(qname);
}
void OpcUa_LocalizedText_Initialize(SOPC_LocalizedText* localizedText){
    SOPC_LocalizedText_Initialize(localizedText);
}
void OpcUa_LocalizedText_Clear(SOPC_LocalizedText* localizedText){
    SOPC_LocalizedText_Clear(localizedText);
}
void OpcUa_ExtensionObject_Initialize(SOPC_ExtensionObject* extObj){
    SOPC_ExtensionObject_Initialize(extObj);
}
void OpcUa_ExtensionObject_Clear(SOPC_ExtensionObject* extObj){
    SOPC_ExtensionObject_Clear(extObj);
}
void OpcUa_Variant_Initialize(SOPC_Variant* variant){
    SOPC_Variant_Initialize(variant);
}
void OpcUa_Variant_Clear(SOPC_Variant* variant){
    SOPC_Variant_Clear(variant);
}
void OpcUa_DataValue_Initialize(SOPC_DataValue* dataValue){
    SOPC_DataValue_Initialize(dataValue);
}
void OpcUa_DataValue_Clear(SOPC_DataValue* dataValue){
    SOPC_DataValue_Clear(dataValue);
}


