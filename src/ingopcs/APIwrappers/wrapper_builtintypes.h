/*
 * wrapper_builtintypes.h
 *
 *  Created on: Nov 23, 2016
 *      Author: vincent
 */

#ifndef SOPC_WRAPPER_BUILTINTYPES_H_
#define SOPC_WRAPPER_BUILTINTYPES_H_

#include "sopc_builtintypes.h"

void OpcUa_Boolean_Initialize(SOPC_Boolean* b);
void OpcUa_Boolean_Clear(SOPC_Boolean* b);

void OpcUa_SByte_Initialize(SOPC_SByte* sbyte);
void OpcUa_SByte_Clear(SOPC_SByte* sbyte);

void OpcUa_Byte_Initialize(SOPC_Byte* byte);
void OpcUa_Byte_Clear(SOPC_Byte* byte);

void OpcUa_Int16_Initialize(int16_t* intv);
void OpcUa_Int16_Clear(int16_t* intv);

void OpcUa_UInt16_Initialize(uint16_t* uint);
void OpcUa_UInt16_Clear(uint16_t* uint);

void OpcUa_Int32_Initialize(int32_t* intv);
void OpcUa_Int32_Clear(int32_t* intv);

void OpcUa_UInt32_Initialize(uint32_t* uint);
void OpcUa_UInt32_Clear(uint32_t* uint);

void OpcUa_Int64_Initialize(int64_t* intv);
void OpcUa_Int64_Clear(int64_t* intv);

void OpcUa_UInt64_Initialize(uint64_t* uint);
void OpcUa_UInt64_Clear(uint64_t* uint);

void OpcUa_Float_Initialize(float* f);
void OpcUa_Float_Clear(float* f);

void OpcUa_Double_Initialize(double* d);
void OpcUa_Double_Clear(double* d);

void OpcUa_ByteString_Initialize(SOPC_ByteString* bstring);
void OpcUa_ByteString_Clear(SOPC_ByteString* bstring);

void OpcUa_XmlElement_Initialize(SOPC_XmlElement* xmlElt);
void OpcUa_XmlElement_Clear(SOPC_XmlElement* xmlElt);

void OpcUa_DateTime_Initialize(SOPC_DateTime* dateTime);
void OpcUa_DateTime_Clear(SOPC_DateTime* dateTime);

void OpcUa_Guid_Initialize(SOPC_Guid* guid);
void OpcUa_Guid_Clear(SOPC_Guid* guid);

void OpcUa_NodeId_Initialize(SOPC_NodeId* nodeId);
void OpcUa_NodeId_Clear(SOPC_NodeId* nodeId);

void OpcUa_ExpandedNodeId_Initialize(SOPC_ExpandedNodeId* expNodeId);
void OpcUa_ExpandedNodeId_Clear(SOPC_ExpandedNodeId* expNodeId);

void OpcUa_StatusCode_Initialize(SOPC_StatusCode* status);
void OpcUa_StatusCode_Clear(SOPC_StatusCode* status);

void OpcUa_DiagnosticInfo_Initialize(SOPC_DiagnosticInfo* diagInfo);
void OpcUa_DiagnosticInfo_Clear(SOPC_DiagnosticInfo* diagInfo);

void OpcUa_QualifiedName_Initialize(SOPC_QualifiedName* qname);
void OpcUa_QualifiedName_Clear(SOPC_QualifiedName* qname);

void OpcUa_LocalizedText_Initialize(SOPC_LocalizedText* localizedText);
void OpcUa_LocalizedText_Clear(SOPC_LocalizedText* localizedText);

void OpcUa_ExtensionObject_Initialize(SOPC_ExtensionObject* extObj);
void OpcUa_ExtensionObject_Clear(SOPC_ExtensionObject* extObj);

void OpcUa_Variant_Initialize(SOPC_Variant* variant);
void OpcUa_Variant_Clear(SOPC_Variant* variant);

void OpcUa_DataValue_Initialize(SOPC_DataValue* dataValue);
void OpcUa_DataValue_Clear(SOPC_DataValue* dataValue);

#endif /* SOPC_WRAPPER_BUILTINTYPES_H_ */
