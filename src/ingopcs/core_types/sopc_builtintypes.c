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

#include "sopc_builtintypes.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef void (BuiltInFunction) (void*);

void SOPC_Boolean_Initialize(SOPC_Boolean* b){
    if(b != NULL)
        *b = FALSE;
}

void SOPC_Boolean_Clear(SOPC_Boolean* b){
    if(b != NULL)
        *b = FALSE;
}

void SOPC_SByte_Initialize(SOPC_SByte* sbyte){
    if(sbyte != NULL)
        *sbyte = 0;
}

void SOPC_SByte_Clear(SOPC_SByte* sbyte){
    if(sbyte != NULL)
        *sbyte = 0;
}

void SOPC_Byte_Initialize(SOPC_Byte* byte){
    if(byte != NULL)
        *byte = 0;
}

void SOPC_Byte_Clear(SOPC_Byte* byte){
    if(byte != NULL)
        *byte = 0;
}

void SOPC_Int16_Initialize(int16_t* intv){
    if(intv != NULL)
        *intv = 0;
}

void SOPC_Int16_Clear(int16_t* intv){
    if(intv != NULL)
        *intv = 0;
}

void SOPC_UInt16_Initialize(uint16_t* uint){
    if(uint != NULL)
        *uint = 0;
}
void SOPC_UInt16_Clear(uint16_t* uint){
    if(uint != NULL)
        *uint = 0;
}

void SOPC_Int32_Initialize(int32_t* intv){
    if(intv != NULL)
        *intv = 0;
}
void SOPC_Int32_Clear(int32_t* intv){
    if(intv != NULL)
        *intv = 0;
}

void SOPC_UInt32_Initialize(uint32_t* uint){
    if(uint != NULL)
        *uint = 0;
}
void SOPC_UInt32_Clear(uint32_t* uint){
    if(uint != NULL)
        *uint = 0;
}

void SOPC_Int64_Initialize(int64_t* intv){
    if(intv != NULL)
        *intv = 0;
}
void SOPC_Int64_Clear(int64_t* intv){
    if(intv != NULL)
        *intv = 0;
}

void SOPC_UInt64_Initialize(uint64_t* uint){
    if(uint != NULL)
        *uint = 0;
}
void SOPC_UInt64_Clear(uint64_t* uint){
    if(uint != NULL)
        *uint = 0;
}

void SOPC_Float_Initialize(float* f){
    if(f != NULL)
        *f = 0.0;
}

void SOPC_Float_Clear(float* f){
    if(f != NULL)
        *f = 0.0;
}

void SOPC_Double_Initialize(double* d){
    if(d != NULL)
        *d = 0.0;
}

void SOPC_Double_Clear(double* d){
    if(d != NULL)
        *d = 0.0;
}

void SOPC_ByteString_Initialize(SOPC_ByteString* bstring){
    if(bstring != NULL){
        bstring->Length = -1;
        bstring->Data = NULL;
    }
}

SOPC_ByteString* SOPC_ByteString_Create(){
    SOPC_ByteString* bstring = NULL;
    bstring = (SOPC_ByteString*) malloc(sizeof(SOPC_ByteString));
    SOPC_ByteString_Initialize(bstring);
    return bstring;
}

SOPC_StatusCode SOPC_ByteString_InitializeFixedSize(SOPC_ByteString* bstring, uint32_t size){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(bstring != NULL){
        status = STATUS_OK;
        SOPC_ByteString_Initialize(bstring);
        bstring->Length = size;
        bstring->Data = (SOPC_Byte*) malloc (sizeof(SOPC_Byte)*size);
        if(bstring->Data != NULL){
            memset(bstring->Data, 0, size);
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_ByteString_CopyFromBytes(SOPC_ByteString* dest, SOPC_Byte* bytes, int32_t length)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && bytes != NULL
       && length > 0){
        dest->Length = length;
        dest->Data = malloc(sizeof(SOPC_Byte)*length);
        if(dest->Data != NULL){
            memcpy(dest->Data, bytes, length);
            status = STATUS_OK;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_ByteString_Copy(SOPC_ByteString* dest, const SOPC_ByteString* src){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && dest->Data == NULL &&
       src != NULL && src->Length > 0){
        status = STATUS_OK;
        dest->Length = src->Length;
        dest->Data = (SOPC_Byte*) malloc (sizeof(SOPC_Byte)*dest->Length);
        if(dest->Data != NULL){
            // No need of secure copy, both have same size here
            memcpy(dest->Data, src->Data, dest->Length);
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

void SOPC_ByteString_Clear(SOPC_ByteString* bstring){
    if(bstring != NULL){
        if(bstring->Data != NULL){
            free(bstring->Data);
            bstring->Data = NULL;
        }
        SOPC_ByteString_Initialize(bstring);
    }
}

void SOPC_ByteString_Delete(SOPC_ByteString* bstring){
    if(bstring != NULL){
        SOPC_ByteString_Clear(bstring);
        free(bstring);
    }
}

void SOPC_String_Initialize(SOPC_String* string){
    SOPC_ByteString_Initialize((SOPC_ByteString*) string);
    string->DoNotClear = FALSE; // False unless characters attached
}

SOPC_String* SOPC_String_Create(){
    return (SOPC_String*) SOPC_ByteString_Create();
}

SOPC_StatusCode SOPC_String_AttachFrom(SOPC_String* dest, SOPC_String* src){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && src != NULL
       && src->Length > 0 && src->Data != NULL){
        status = STATUS_OK;
        dest->Length = src->Length;
        dest->Data = src->Data;
        dest->DoNotClear = 1; // dest->characters will not be freed on clear
    }
    return status;
}

SOPC_StatusCode SOPC_String_AttachFromCstring(SOPC_String* dest, char* src){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && dest->Data == NULL && src != NULL){
        status = STATUS_OK;
        if(CHAR_BIT == 8){
            dest->Length = strlen(src);
            dest->Data = (uint8_t*) src;
            dest->DoNotClear = 1; // dest->characters will not be freed on clear
        }else{
            assert(FALSE);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_String_Copy(SOPC_String* dest, const SOPC_String* src){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && dest->Data == NULL && src != NULL){
        // Keep null terminator for C string compatibility
        status = STATUS_OK;
        dest->Length = src->Length;
        if(dest->Length > 0){
            dest->Data = (SOPC_Byte*) malloc (sizeof(SOPC_Byte)*dest->Length+1);
            if(dest->Data != NULL){
                // No need of secure copy, both have same size here
                memcpy(dest->Data, src->Data, dest->Length);
                dest->Data[dest->Length] = '\0';
                // Since it is a copy, be sure to clear bytes on clear
                dest->DoNotClear = FALSE;
            }else{
                status = STATUS_NOK;
            }
        }
    }
    return status;
}

void SOPC_String_Clear(SOPC_String* string){
    if(string != NULL){
        if(string->Data != NULL &&
           string->DoNotClear == FALSE){
            free(string->Data);
            string->Data = NULL;
        }
        SOPC_String_Initialize(string);
    }
}

void SOPC_String_Delete(SOPC_String* string){
    if(NULL != string){
        SOPC_String_Clear(string);
        free(string);
    }
}

SOPC_StatusCode SOPC_String_CopyFromCString(SOPC_String* string, const char* cString){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    size_t stringLength = 0;
    size_t idx = 0;
    if(string != NULL && string->Data == NULL
       && cString != NULL){
        status = STATUS_OK;

        stringLength = strlen(cString);
        if(stringLength > 0 &&
           stringLength <= INT32_MAX)
        {
            // length without null terminator
            string->Length = stringLength;
            // keep terminator for compatibility with char* strings
            string->Data = (SOPC_Byte*) malloc(sizeof(SOPC_Byte)*(stringLength+1));
            if(string->Data != NULL){
                // Keep null terminator for compatibility with c strings !
                if(CHAR_BIT == 8){
                    memcpy(string->Data, cString, stringLength + 1);
                }else{
                    // On systems for which char is not encoded on 1 byte
                    for(idx = 0; idx < stringLength + 1; idx++){
                        string->Data[idx] = (uint8_t) cString[idx];
                    }
                }
            }else{
                status = STATUS_NOK;
            }
        }else{
            status = STATUS_NOK;
        }
    }

    return status;
}

SOPC_StatusCode SOPC_String_InitializeFromCString(SOPC_String* string, const char* cString){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    if(string != NULL){
        SOPC_String_Initialize(string);
        status = SOPC_String_CopyFromCString(string, cString);
    }

    return status;
}


char* SOPC_String_GetCString(const SOPC_String* string){
    char* cString = NULL;
    int32_t idx = 0;
    if(string != NULL &&
       string->Length > 0)
    {
        cString = (char*) malloc(sizeof(char)* (string->Length + 1));
        if(cString != NULL){
            if(CHAR_BIT == 8){
                memcpy(cString, string->Data, string->Length + 1);
            }else{
                // On systems for which char is not encoded on 1 byte
                for(idx = 0; idx < string->Length + 1; idx++){
                    cString[idx] = (char) string->Data[idx];
                }
            }
        }
    }
    return cString;
}

const char* SOPC_String_GetRawCString(const SOPC_String* string){
    char* cString = NULL;
    if(string != NULL &&
       string->Length > 0)
    {
        if(CHAR_BIT == 8){
            cString = (char*) string->Data;
            assert(string->Data[string->Length] == '\0');
        }else{
            assert(FALSE);
        }
    }
    return cString;
}

SOPC_StatusCode SOPC_ByteString_Compare(const SOPC_ByteString* left,
                                   const SOPC_ByteString* right,
                                   int32_t*               comparison)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    if(left != NULL && right != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        if(left->Length == right->Length ||
           (left->Length <= 0 && right->Length <= 0)){
            if(left->Length <= 0 && right->Length <= 0){
                *comparison = 0;
            }else{
                *comparison = memcmp(left->Data, right->Data, left->Length);
            }
        }else if(left->Length > right->Length){

            *comparison = +1;
        }else{
            *comparison = -1;
        }
    }

    return status;
}

uint8_t SOPC_ByteString_Equal(const SOPC_ByteString* left,
                              const SOPC_ByteString* right)
{
    int32_t compare = 0;
    uint8_t result = FALSE;

    if(SOPC_ByteString_Compare(left, right, &compare) == STATUS_OK){
        result = compare == 0;
    }

    return result;
}

SOPC_StatusCode SOPC_String_Compare(const SOPC_String* left,
                               const SOPC_String* right,
                               int32_t*           comparison)
{

    return SOPC_ByteString_Compare((SOPC_ByteString*) left,
                              (SOPC_ByteString*) right, comparison);
}

uint32_t SOPC_String_Equal(const SOPC_String* left,
                           const SOPC_String* right)
{
    return SOPC_ByteString_Equal((SOPC_ByteString*) left,
                              (SOPC_ByteString*) right);
}

void SOPC_XmlElement_Initialize(SOPC_XmlElement* xmlElt){
    if(xmlElt != NULL){
        SOPC_ByteString_Initialize((SOPC_ByteString*) xmlElt);
    }
}

void SOPC_XmlElement_Clear(SOPC_XmlElement* xmlElt){
    SOPC_ByteString_Clear((SOPC_ByteString*) xmlElt);
}


void SOPC_DateTime_Initialize(SOPC_DateTime* dateTime){
    if(dateTime != NULL){
        dateTime->Low32 = 0;
        dateTime->High32 = 0;
    }
}

void SOPC_DateTime_Clear(SOPC_DateTime* dateTime){
    SOPC_DateTime_Initialize(dateTime);
}

int64_t SOPC_DateTime_ToInt64(const SOPC_DateTime* dateTime){
    int64_t result = 0;
    uint64_t shiftHigh = 0;
    if(dateTime != NULL){
        shiftHigh = dateTime->High32;
        result = dateTime->Low32 + (shiftHigh << 32);
    }
    return result;
}

void SOPC_DateTime_FromInt64(SOPC_DateTime* dateTime, int64_t date){
    if(dateTime != NULL){
        dateTime->Low32 = date & 0x00000000FFFFFFFF;
        dateTime->High32 = date >> 32;
    }
}

void SOPC_Guid_Initialize(SOPC_Guid* guid){
    if(guid != NULL){
        memset(guid, 0, sizeof(SOPC_Guid));
    }
}

void SOPC_Guid_Clear(SOPC_Guid* guid){
    if(guid != NULL){
        memset(guid, 0, sizeof(SOPC_Guid));
    }
}

void SOPC_NodeId_Initialize(SOPC_NodeId* nodeId){
    if(nodeId != NULL){
        memset(nodeId, 0, sizeof(SOPC_NodeId));
    }
}

void SOPC_NodeId_InitType(SOPC_NodeId* nodeId, SOPC_IdentifierType knownIdType){
    if(nodeId != NULL){
        nodeId->Namespace = 0; // OPCUA namespace
        nodeId->IdentifierType = knownIdType;
        switch(knownIdType){
            case IdentifierType_Numeric:
                SOPC_UInt32_Initialize(&nodeId->Data.Numeric);
                break;
            case IdentifierType_String:
                SOPC_String_Initialize(&nodeId->Data.String);
                break;
            case IdentifierType_Guid:
                nodeId->Data.Guid = malloc(sizeof(SOPC_Guid));
                SOPC_Guid_Initialize(nodeId->Data.Guid);
                break;
            case IdentifierType_ByteString:
                SOPC_ByteString_Initialize(&nodeId->Data.Bstring);
                break;
        }
    }
}

void SOPC_NodeId_Clear(SOPC_NodeId* nodeId){
    if(nodeId != NULL){
        nodeId->Namespace = 0; // OPCUA namespace
        switch(nodeId->IdentifierType){
            case IdentifierType_Numeric:
                SOPC_UInt32_Clear(&nodeId->Data.Numeric);
                break;
            case IdentifierType_String:
                SOPC_String_Clear(&nodeId->Data.String);
                break;
            case IdentifierType_Guid:
                SOPC_Guid_Clear(nodeId->Data.Guid);
                if(nodeId->Data.Guid != NULL){
                    free(nodeId->Data.Guid);
                }
                nodeId->Data.Guid = NULL;
                break;
            case IdentifierType_ByteString:
                SOPC_ByteString_Clear(&nodeId->Data.Bstring);
                break;
        }
        nodeId->IdentifierType = IdentifierType_Numeric;
    }
}

void SOPC_ExpandedNodeId_Initialize(SOPC_ExpandedNodeId* expNodeId){
    if(expNodeId != NULL){
        SOPC_String_Initialize(&expNodeId->NamespaceUri);
        SOPC_NodeId_Initialize(&expNodeId->NodeId);
        SOPC_UInt32_Initialize(&expNodeId->ServerIndex);
    }
}

void SOPC_ExpandedNodeId_Clear(SOPC_ExpandedNodeId* expNodeId){
    if(expNodeId != NULL){
        SOPC_String_Clear(&expNodeId->NamespaceUri);
        SOPC_NodeId_Clear(&expNodeId->NodeId);
        SOPC_UInt32_Clear(&expNodeId->ServerIndex);
    }
}

void SOPC_StatusCode_Initialize(SOPC_StatusCode* status){
    if(status != NULL){
        *status = STATUS_OK;
    }
}

void SOPC_StatusCode_Clear(SOPC_StatusCode* status){
    if(status != NULL){
        *status = STATUS_OK;
    }
}

void SOPC_DiagnosticInfo_Initialize(SOPC_DiagnosticInfo* diagInfo){
    if(diagInfo != NULL){
        diagInfo->SymbolicId = -1;
        diagInfo->NamespaceUri = -1;
        diagInfo->Locale = -1;
        diagInfo->LocalizedText = -1;
        SOPC_String_Initialize(&diagInfo->AdditionalInfo);
        diagInfo->InnerStatusCode = STATUS_OK;
        diagInfo->InnerDiagnosticInfo = NULL;
    }
}

void SOPC_DiagnosticInfo_Clear(SOPC_DiagnosticInfo* diagInfo){
    if(diagInfo != NULL){
        SOPC_String_Clear(&diagInfo->AdditionalInfo);
        if(diagInfo->InnerDiagnosticInfo != NULL){
            SOPC_DiagnosticInfo_Clear(diagInfo->InnerDiagnosticInfo);
            free(diagInfo->InnerDiagnosticInfo);
        }
        diagInfo->SymbolicId = -1;
        diagInfo->NamespaceUri = -1;
        diagInfo->Locale = -1;
        diagInfo->LocalizedText = -1;
        diagInfo->InnerStatusCode = STATUS_OK;
        diagInfo->InnerDiagnosticInfo = NULL;
    }
}


void SOPC_QualifiedName_Initialize(SOPC_QualifiedName* qname){
    if(qname != NULL){
        qname->NamespaceIndex = 0;
        SOPC_String_Initialize(&qname->Name);
    }
}

void SOPC_QualifiedName_Clear(SOPC_QualifiedName* qname){
    if(qname != NULL){
        qname->NamespaceIndex = 0;
        SOPC_String_Clear(&qname->Name);
    }
}

void SOPC_LocalizedText_Initialize(SOPC_LocalizedText* localizedText){
    if(localizedText != NULL){
        SOPC_String_Initialize(&localizedText->Locale);
        SOPC_String_Initialize(&localizedText->Text);
    }
}

void SOPC_LocalizedText_Clear(SOPC_LocalizedText* localizedText){
    if(localizedText != NULL){
        SOPC_String_Clear(&localizedText->Locale);
        SOPC_String_Clear(&localizedText->Text);
    }
}

void SOPC_ExtensionObject_Initialize(SOPC_ExtensionObject* extObj){
    if(extObj != NULL){
        memset(extObj, 0, sizeof(SOPC_ExtensionObject));
        SOPC_ExpandedNodeId_Initialize(&extObj->TypeId);
        extObj->Length = -1;
    }
}

void SOPC_ExtensionObject_Clear(SOPC_ExtensionObject* extObj){
    if(extObj != NULL){
        SOPC_ExpandedNodeId_Clear(&extObj->TypeId);
        switch(extObj->Encoding){
            case SOPC_ExtObjBodyEncoding_None:
                break;
            case SOPC_ExtObjBodyEncoding_ByteString:
                SOPC_ByteString_Clear(&extObj->Body.Bstring);
                break;
            case SOPC_ExtObjBodyEncoding_XMLElement:
                SOPC_XmlElement_Clear(&extObj->Body.Xml);
                break;
            case SOPC_ExtObjBodyEncoding_Object:
                extObj->Body.Object.ObjType->Clear(extObj->Body.Object.Value);
                free(extObj->Body.Object.Value);
                extObj->Body.Object.Value = NULL;
                break;
        }
        extObj->Length = -1;
    }
}

void ApplyToVariantNonArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                       SOPC_VariantValue val,
                                       BuiltInFunction* builtInFunction){
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            builtInFunction(&val.Boolean);
            break;
        case SOPC_SByte_Id:
            builtInFunction(&val.Sbyte);
            break;
        case SOPC_Byte_Id:
            builtInFunction(&val.Byte);
            break;
        case SOPC_Int16_Id:
            builtInFunction(&val.Int16);
            break;
        case SOPC_UInt16_Id:
            builtInFunction(&val.Uint16);
            break;
        case SOPC_Int32_Id:
            builtInFunction(&val.Int32);
            break;
        case SOPC_UInt32_Id:
            builtInFunction(&val.Uint32);
            break;
        case SOPC_Int64_Id:
            builtInFunction(&val.Int64);
            break;
        case SOPC_UInt64_Id:
            builtInFunction(&val.Uint64);
            break;
        case SOPC_Float_Id:
            builtInFunction(&val.Floatv);
            break;
        case SOPC_Double_Id:
            builtInFunction(&val.Doublev);
            break;
        case SOPC_String_Id:
            builtInFunction(&val.String);
            break;
        case SOPC_DateTime_Id:
            builtInFunction(&val.Date);
            break;
        case SOPC_Guid_Id:
            builtInFunction(val.Guid);
            break;
        case SOPC_ByteString_Id:
            builtInFunction(&val.Bstring);
            break;
        case SOPC_XmlElement_Id:
            builtInFunction(&val.XmlElt);
            break;
        case SOPC_NodeId_Id:
            builtInFunction(val.NodeId);
            break;
        case SOPC_ExpandedNodeId_Id:
            builtInFunction(val.ExpNodeId);
            break;
        case SOPC_StatusCode_Id:
            builtInFunction(&val.Status);
            break;
        case SOPC_QualifiedName_Id:
            builtInFunction(val.Qname);
            break;
        case SOPC_LocalizedText_Id:
            builtInFunction(val.LocalizedText);
            break;
        case SOPC_ExtensionObject_Id:
            builtInFunction(val.ExtObject);
            break;
        case SOPC_DataValue_Id:
            builtInFunction(val.DataValue);
            break;
        case SOPC_Variant_Id:
            assert(FALSE);
            break;
        case SOPC_DiagnosticInfo_Id:
            builtInFunction(val.DiagInfo);
            break;
        default:
            break;
    }
}

void ApplyToVariantArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                    SOPC_VariantArrayValue array,
                                    int32_t length,
                                    BuiltInFunction* builtInFunction){
    int32_t idx = 0;
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.BooleanArr[idx]);
            }
            break;
        case SOPC_SByte_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.SbyteArr[idx]);
            }
            break;
        case SOPC_Byte_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.ByteArr[idx]);
            }
            break;
        case SOPC_Int16_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Int16Arr[idx]);
            }
            break;
        case SOPC_UInt16_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Uint16Arr[idx]);
            }
            break;
        case SOPC_Int32_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Int32Arr[idx]);
            }
            break;
        case SOPC_UInt32_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Uint32Arr[idx]);
            }
            break;
        case SOPC_Int64_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Int64Arr[idx]);
            }
            break;
        case SOPC_UInt64_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Uint64Arr[idx]);
            }
            break;
        case SOPC_Float_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.FloatvArr[idx]);
            }
            break;
        case SOPC_Double_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.DoublevArr[idx]);
            }
            break;
        case SOPC_String_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.StringArr[idx]);
            }
            break;
        case SOPC_DateTime_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.DateArr[idx]);
            }
            break;
        case SOPC_Guid_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.GuidArr[idx]);
            }
            break;
        case SOPC_ByteString_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.BstringArr[idx]);
            }
            break;
        case SOPC_XmlElement_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.XmlEltArr[idx]);
            }
            break;
        case SOPC_NodeId_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.NodeIdArr[idx]);
            }
            break;
        case SOPC_ExpandedNodeId_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.ExpNodeIdArr[idx]);
            }
            break;
        case SOPC_StatusCode_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.StatusArr[idx]);
            }
            break;
        case SOPC_QualifiedName_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.QnameArr[idx]);
            }
            break;
        case SOPC_LocalizedText_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.LocalizedTextArr[idx]);
            }
            break;
        case SOPC_ExtensionObject_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.ExtObjectArr[idx]);
            }
            break;
        case SOPC_DataValue_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.DataValueArr[idx]);
            }
            break;
        case SOPC_Variant_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.VariantArr[idx]);
            }
            break;
        case SOPC_DiagnosticInfo_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.DiagInfoArr[idx]);
            }
            break;
        default:
            break;
    }
}

void SOPC_Variant_Initialize(SOPC_Variant* variant){
    if(variant != NULL){
        memset(variant, 0, sizeof(SOPC_Variant));
    }
}

BuiltInFunction* GetBuiltInTypeClearFunction(SOPC_BuiltinId builtInTypeId){
    BuiltInFunction* clearFunction = NULL;
    switch(builtInTypeId){
            case SOPC_Boolean_Id:
                clearFunction = (BuiltInFunction*) SOPC_Boolean_Clear;
                break;
            case SOPC_SByte_Id:
                clearFunction = (BuiltInFunction*) SOPC_SByte_Clear;
                break;
            case SOPC_Byte_Id:
                clearFunction = (BuiltInFunction*) SOPC_Byte_Clear;
                break;
            case SOPC_Int16_Id:
                clearFunction = (BuiltInFunction*) SOPC_Int16_Clear;
                break;
            case SOPC_UInt16_Id:
                clearFunction = (BuiltInFunction*) SOPC_UInt16_Clear;
                break;
            case SOPC_Int32_Id:
                clearFunction = (BuiltInFunction*) SOPC_Int32_Clear;
                break;
            case SOPC_UInt32_Id:
                clearFunction = (BuiltInFunction*) SOPC_UInt32_Clear;
                break;
            case SOPC_Int64_Id:
                clearFunction = (BuiltInFunction*) SOPC_Int64_Clear;
                break;
            case SOPC_UInt64_Id:
                clearFunction = (BuiltInFunction*) SOPC_UInt64_Clear;
                break;
            case SOPC_Float_Id:
                clearFunction = (BuiltInFunction*) SOPC_Float_Clear;
                break;
            case SOPC_Double_Id:
                clearFunction = (BuiltInFunction*) SOPC_Double_Clear;
                break;
            case SOPC_String_Id:
                clearFunction = (BuiltInFunction*) SOPC_String_Clear;
                break;
            case SOPC_DateTime_Id:
                clearFunction = (BuiltInFunction*) SOPC_DateTime_Clear;
                break;
            case SOPC_Guid_Id:
                clearFunction = (BuiltInFunction*) SOPC_Guid_Clear;
                break;
            case SOPC_ByteString_Id:
                clearFunction = (BuiltInFunction*) SOPC_ByteString_Clear;
                break;
            case SOPC_XmlElement_Id:
                clearFunction = (BuiltInFunction*) SOPC_XmlElement_Clear;
                break;
            case SOPC_NodeId_Id:
                clearFunction = (BuiltInFunction*) SOPC_NodeId_Clear;
                break;
            case SOPC_ExpandedNodeId_Id:
                clearFunction = (BuiltInFunction*) SOPC_ExpandedNodeId_Clear;
                break;
            case SOPC_StatusCode_Id:
                clearFunction = (BuiltInFunction*) SOPC_StatusCode_Clear;
                break;
            case SOPC_QualifiedName_Id:
                clearFunction = (BuiltInFunction*) SOPC_QualifiedName_Clear;
                break;
            case SOPC_LocalizedText_Id:
                clearFunction = (BuiltInFunction*) SOPC_LocalizedText_Clear;
                break;
            case SOPC_ExtensionObject_Id:
                clearFunction = (BuiltInFunction*) SOPC_ExtensionObject_Clear;
                break;
            case SOPC_DataValue_Id:
                clearFunction = (BuiltInFunction*) SOPC_DataValue_Clear;
                break;
            case SOPC_Variant_Id:
                clearFunction = (BuiltInFunction*) SOPC_Variant_Clear;
                break;
            case SOPC_DiagnosticInfo_Id:
                clearFunction = (BuiltInFunction*) SOPC_DiagnosticInfo_Clear;
                break;
            default:
                break;
        }
    return clearFunction;
}

void FreeVariantArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                 SOPC_VariantArrayValue* array)
{
    switch(builtInTypeId){
        case SOPC_Null_Id:
            break;
        case SOPC_Boolean_Id:
            if(array->BooleanArr != NULL){
                free(array->BooleanArr);
                array->BooleanArr = NULL;
            }
            break;
        case SOPC_SByte_Id:
            if(array->SbyteArr != NULL){
                free(array->SbyteArr);
                array->SbyteArr = NULL;
            }
            break;
        case SOPC_Byte_Id:
            if(array->ByteArr != NULL){
                free(array->ByteArr);
                array->ByteArr = NULL;
            }
            break;
        case SOPC_Int16_Id:
            if(array->Int16Arr != NULL){
                free(array->Int16Arr);
                array->Int16Arr = NULL;
            }
            break;
        case SOPC_UInt16_Id:
            if(array->Uint16Arr != NULL){
                free(array->Uint16Arr);
                array->Uint16Arr = NULL;
            }
            break;
        case SOPC_Int32_Id:
            if(array->Int32Arr != NULL){
                free(array->Int32Arr);
                array->Int32Arr = NULL;
            }
            break;
        case SOPC_UInt32_Id:
            if(array->Uint32Arr != NULL){
                free(array->Uint32Arr);
                array->Uint32Arr = NULL;
            }
            break;
        case SOPC_Int64_Id:
            if(array->Int64Arr != NULL){
                free(array->Int64Arr);
                array->Int64Arr = NULL;
            }
            break;
        case SOPC_UInt64_Id:
            if(array->Uint64Arr != NULL){
                free(array->Uint64Arr);
                array->Uint64Arr = NULL;
            }
            break;
        case SOPC_Float_Id:
            if(array->FloatvArr != NULL){
                free(array->FloatvArr);
                array->FloatvArr = NULL;
            }
            break;
        case SOPC_Double_Id:
            if(array->DoublevArr != NULL){
                free(array->DoublevArr);
                array->DoublevArr = NULL;
            }
            break;
        case SOPC_String_Id:
            if(array->StringArr != NULL){
                free(array->StringArr);
                array->StringArr = NULL;
            }
            break;
        case SOPC_DateTime_Id:
            if(array->DateArr != NULL){
                free(array->DateArr);
                array->DateArr = NULL;
            }
            break;
        case SOPC_Guid_Id:
            if(array->GuidArr != NULL){
                free(array->GuidArr);
                array->GuidArr = NULL;
            }
            break;
        case SOPC_ByteString_Id:
            if(array->BstringArr != NULL){
                free(array->BstringArr);
                array->BstringArr = NULL;
            }
            break;
        case SOPC_XmlElement_Id:
            if(array->XmlEltArr != NULL){
                free(array->XmlEltArr);
                array->XmlEltArr = NULL;
            }
            break;
        case SOPC_NodeId_Id:
            if(array->NodeIdArr != NULL){
                free(array->NodeIdArr);
                array->NodeIdArr = NULL;
            }
            break;
        case SOPC_ExpandedNodeId_Id:
            if(array->ExpNodeIdArr != NULL){
                free(array->ExpNodeIdArr);
                array->ExpNodeIdArr = NULL;
            }
            break;
        case SOPC_StatusCode_Id:
            if(array->StatusArr != NULL){
                free(array->StatusArr);
                array->StatusArr = NULL;
            }
            break;
        case SOPC_QualifiedName_Id:
            if(array->QnameArr != NULL){
                free(array->QnameArr);
                array->QnameArr = NULL;
            }
            break;
        case SOPC_LocalizedText_Id:
            if(array->LocalizedTextArr != NULL){
                free(array->LocalizedTextArr);
                array->LocalizedTextArr = NULL;
            }
            break;
        case SOPC_ExtensionObject_Id:
            if(array->ExtObjectArr != NULL){
                free(array->ExtObjectArr);
                array->ExtObjectArr = NULL;
            }
            break;
        case SOPC_DataValue_Id:
            if(array->DataValueArr != NULL){
                free(array->DataValueArr);
                array->DataValueArr = NULL;
            }
            break;
        case SOPC_Variant_Id:
            if(array->VariantArr != NULL){
                free(array->VariantArr);
                array->VariantArr = NULL;
            }
            break;
        case SOPC_DiagnosticInfo_Id:
            if(array->DiagInfoArr != NULL){
                free(array->DiagInfoArr);
                array->DiagInfoArr = NULL;
            }
            break;
    }
}

void FreeVariantNonArrayBuiltInType(SOPC_BuiltinId builtInTypeId,
                                    SOPC_VariantValue *val)
{
    switch(builtInTypeId){
        case SOPC_Null_Id:
        case SOPC_Boolean_Id:
        case SOPC_SByte_Id:
        case SOPC_Byte_Id:
        case SOPC_Int16_Id:
        case SOPC_UInt16_Id:
        case SOPC_Int32_Id:
        case SOPC_UInt32_Id:
        case SOPC_Int64_Id:
        case SOPC_UInt64_Id:
        case SOPC_Float_Id:
        case SOPC_Double_Id:
        case SOPC_String_Id:
        case SOPC_DateTime_Id:
        case SOPC_ByteString_Id:
        case SOPC_XmlElement_Id:
        case SOPC_StatusCode_Id:
            break;
        case SOPC_Guid_Id:
            if(NULL != val->Guid){
                free(val->Guid);
            }
            break;
        case SOPC_NodeId_Id:
            if(NULL != val->NodeId){
                free(val->NodeId);
            }
            break;
        case SOPC_ExpandedNodeId_Id:
            if(NULL != val->ExpNodeId){
                free(val->ExpNodeId);
            }
            break;
        case SOPC_QualifiedName_Id:
            if(NULL != val->Qname){
                free(val->Qname);
            }
            break;
        case SOPC_LocalizedText_Id:
            if(NULL != val->LocalizedText){
                free(val->LocalizedText);
            }
            break;
        case SOPC_ExtensionObject_Id:
            if(NULL != val->ExtObject){
                free(val->ExtObject);
            }
            break;
        case SOPC_DataValue_Id:
            if(NULL != val->DataValue){
                free(val->DataValue);
            }
            break;
        case SOPC_Variant_Id:
            assert(FALSE);
            break;
        case SOPC_DiagnosticInfo_Id:
            if(NULL != val->DiagInfo){
                free(val->DiagInfo);
            }
            break;
    }
}

void SOPC_Variant_Clear(SOPC_Variant* variant){
    int32_t length = 0;
    int32_t idx = 0;
    if(variant != NULL){
        BuiltInFunction* clearFunction = GetBuiltInTypeClearFunction(variant->BuiltInTypeId);
        if(clearFunction == NULL)
            return;

        switch(variant->ArrayType){
            case SOPC_VariantArrayType_SingleValue:
                ApplyToVariantNonArrayBuiltInType(variant->BuiltInTypeId,
                                                  variant->Value,
                                                  clearFunction);
                FreeVariantNonArrayBuiltInType(variant->BuiltInTypeId,
                                               &variant->Value);
                break;
            case SOPC_VariantArrayType_Array:
                ApplyToVariantArrayBuiltInType(variant->BuiltInTypeId,
                                               variant->Value.Array.Content,
                                               variant->Value.Array.Length,
                                               clearFunction);
                FreeVariantArrayBuiltInType(variant->BuiltInTypeId,
                                            &variant->Value.Array.Content);

                break;
            case SOPC_VariantArrayType_Matrix:
                for(idx = 0; idx < variant->Value.Matrix.Dimensions; idx ++){
                    length *= variant->Value.Matrix.ArrayDimensions[idx];
                }
                free(variant->Value.Matrix.ArrayDimensions);
                ApplyToVariantArrayBuiltInType(variant->BuiltInTypeId,
                                               variant->Value.Matrix.Content,
                                               length,
                                               clearFunction);
                FreeVariantArrayBuiltInType(variant->BuiltInTypeId,
                                            &variant->Value.Matrix.Content);
                break;
        }
    }
}

void SOPC_DataValue_Initialize(SOPC_DataValue* dataValue){
    if(dataValue != NULL){
        memset(dataValue, 0, sizeof(SOPC_DataValue));
    }
}
void SOPC_DataValue_Clear(SOPC_DataValue* dataValue){
    if(dataValue != NULL){
        SOPC_Variant_Clear(&dataValue->Value);
        SOPC_StatusCode_Clear(&dataValue->Status);
        SOPC_DateTime_Clear(&dataValue->SourceTimestamp);
        SOPC_DateTime_Clear(&dataValue->ServerTimestamp);
        dataValue->SourcePicoSeconds = 0;
        dataValue->ServerPicoSeconds = 0;
    }
}

