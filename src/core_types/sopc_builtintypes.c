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

#include "base_tools.h"

void SOPC_Boolean_InitializeAux(void* value){
    SOPC_Boolean_Initialize((SOPC_Boolean*) value);
}

void SOPC_Boolean_Initialize(SOPC_Boolean* b){
    if(b != NULL)
        *b = FALSE;
}


void SOPC_Boolean_ClearAux(void* value){
    SOPC_Boolean_Clear((SOPC_Boolean*) value);
}

void SOPC_Boolean_Clear(SOPC_Boolean* b){
    if(b != NULL)
        *b = FALSE;
}


void SOPC_SByte_InitializeAux(void* value){
    SOPC_SByte_Initialize((SOPC_SByte*) value);
}

void SOPC_SByte_Initialize(SOPC_SByte* sbyte){
    if(sbyte != NULL)
        *sbyte = 0;
}


void SOPC_SByte_ClearAux(void* value){
    SOPC_SByte_Clear((SOPC_SByte*) value);
}

void SOPC_SByte_Clear(SOPC_SByte* sbyte){
    if(sbyte != NULL)
        *sbyte = 0;
}


void SOPC_Byte_InitializeAux(void* value){
    SOPC_Byte_Initialize((SOPC_Byte*) value);
}

void SOPC_Byte_Initialize(SOPC_Byte* byte){
    if(byte != NULL)
        *byte = 0;
}


void SOPC_Byte_ClearAux(void* value){
    SOPC_Byte_Clear((SOPC_Byte*) value);
}

void SOPC_Byte_Clear(SOPC_Byte* byte){
    if(byte != NULL)
        *byte = 0;
}


void SOPC_Int16_InitializeAux(void* value){
    SOPC_Int16_Initialize((int16_t*) value);
}

void SOPC_Int16_Initialize(int16_t* intv){
    if(intv != NULL)
        *intv = 0;
}


void SOPC_Int16_ClearAux(void* value){
    SOPC_Int16_Clear((int16_t*) value);
}

void SOPC_Int16_Clear(int16_t* intv){
    if(intv != NULL)
        *intv = 0;
}


void SOPC_UInt16_InitializeAux(void* value){
    SOPC_UInt16_Initialize((uint16_t*) value);
}

void SOPC_UInt16_Initialize(uint16_t* uint){
    if(uint != NULL)
        *uint = 0;
}

void SOPC_UInt16_ClearAux(void* value){
    SOPC_UInt16_Clear((uint16_t*) value);
}
void SOPC_UInt16_Clear(uint16_t* uint){
    if(uint != NULL)
        *uint = 0;
}


void SOPC_Int32_InitializeAux(void* value){
    SOPC_Int32_Initialize((int32_t*) value);
}

void SOPC_Int32_Initialize(int32_t* intv){
    if(intv != NULL)
        *intv = 0;
}

void SOPC_Int32_ClearAux(void* value){
    SOPC_Int32_Clear((int32_t*) value);
}
void SOPC_Int32_Clear(int32_t* intv){
    if(intv != NULL)
        *intv = 0;
}


void SOPC_UInt32_InitializeAux(void* value){
    SOPC_UInt32_Initialize((uint32_t*) value);
}

void SOPC_UInt32_Initialize(uint32_t* uint){
    if(uint != NULL)
        *uint = 0;
}

void SOPC_UInt32_ClearAux(void* value){
    SOPC_UInt32_Clear((uint32_t*) value);
}
void SOPC_UInt32_Clear(uint32_t* uint){
    if(uint != NULL)
        *uint = 0;
}


void SOPC_Int64_InitializeAux(void* value){
    SOPC_Int64_Initialize((int64_t*) value);
}

void SOPC_Int64_Initialize(int64_t* intv){
    if(intv != NULL)
        *intv = 0;
}

void SOPC_Int64_ClearAux(void* value){
    SOPC_Int64_Clear((int64_t*) value);
}
void SOPC_Int64_Clear(int64_t* intv){
    if(intv != NULL)
        *intv = 0;
}


void SOPC_UInt64_InitializeAux(void* value){
    SOPC_UInt64_Initialize((uint64_t*) value);
}

void SOPC_UInt64_Initialize(uint64_t* uint){
    if(uint != NULL)
        *uint = 0;
}

void SOPC_UInt64_ClearAux(void* value){
    SOPC_UInt64_Clear((uint64_t*) value);
}
void SOPC_UInt64_Clear(uint64_t* uint){
    if(uint != NULL)
        *uint = 0;
}


void SOPC_Float_InitializeAux(void* value){
    SOPC_Float_Initialize((float*) value);
}

void SOPC_Float_Initialize(float* f){
    if(f != NULL)
        *f = 0.0;
}


void SOPC_Float_ClearAux(void* value){
    SOPC_Float_Clear((float*) value);
}

void SOPC_Float_Clear(float* f){
    if(f != NULL)
        *f = 0.0;
}


void SOPC_Double_InitializeAux(void* value){
    SOPC_Double_Initialize((double*) value);
}

void SOPC_Double_Initialize(double* d){
    if(d != NULL)
        *d = 0.0;
}


void SOPC_Double_ClearAux(void* value){
    SOPC_Double_Clear((double*) value);
}

void SOPC_Double_Clear(double* d){
    if(d != NULL)
        *d = 0.0;
}


void SOPC_ByteString_InitializeAux(void* value){
    SOPC_ByteString_Initialize((SOPC_ByteString*) value);
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


void SOPC_ByteString_ClearAux(void* value){
    SOPC_ByteString_Clear((SOPC_ByteString*) value);
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


void SOPC_String_InitializeAux(void* value){
    SOPC_String_Initialize((SOPC_String*) value);
}

void SOPC_String_Initialize(SOPC_String* string){
    if(string != NULL){
        string->Length = -1;
        string->Data = NULL;
        string->DoNotClear = FALSE; // False unless characters attached
    }
}

SOPC_String* SOPC_String_Create(){
    SOPC_String* string = NULL;
    string = (SOPC_String*) malloc(sizeof(SOPC_String));
    SOPC_String_Initialize(string);
    return string;
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
    size_t stringLength = 0;
    if(dest != NULL && dest->Data == NULL && src != NULL){
        assert(CHAR_BIT == 8);
        stringLength = strlen(src);
        if(stringLength <= INT32_MAX)
        {
            status = STATUS_OK;
            dest->Length = (int32_t) stringLength;
            dest->Data = (uint8_t*) src;
            dest->DoNotClear = 1; // dest->characters will not be freed on clear
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


void SOPC_String_ClearAux(void* value){
    SOPC_String_Clear((SOPC_String*) value);
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
            string->Length = (int32_t) stringLength;
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
                                    uint8_t            ignoreCase,
                                    int32_t*           comparison)
{
    if(left == NULL || right == NULL || comparison == NULL)
        return STATUS_INVALID_PARAMETERS;

    if(left->Length == right->Length)
    {
        assert(CHAR_BIT == 8);
        if(left->Length <= 0 && right->Length <= 0){
            *comparison = 0;
        }else if(ignoreCase == FALSE){
            *comparison = strcmp((char*) left->Data, (char*) right->Data);
        }else{
            *comparison = strncmp_ignore_case((char*) left->Data, (char*) right->Data, left->Length);
        }
    }else if(left->Length > right->Length){
         *comparison = +1;
    }else{
        *comparison = -1;
    }

    return STATUS_OK;
}

uint32_t SOPC_String_Equal(const SOPC_String* left,
                           const SOPC_String* right)
{
    int32_t compare = 0;
    uint8_t result = FALSE;

    if(SOPC_String_Compare(left, right, FALSE, &compare) == STATUS_OK){
        result = compare == 0;
    }

    return result;
}


void SOPC_XmlElement_InitializeAux(void* value){
    SOPC_XmlElement_Initialize((SOPC_XmlElement*) value);
}

void SOPC_XmlElement_Initialize(SOPC_XmlElement* xmlElt){
    if(xmlElt != NULL){
        SOPC_ByteString_Initialize((SOPC_ByteString*) xmlElt);
    }
}


void SOPC_XmlElement_ClearAux(void* value){
    SOPC_XmlElement_Clear((SOPC_XmlElement*) value);
}

void SOPC_XmlElement_Clear(SOPC_XmlElement* xmlElt){
    SOPC_ByteString_Clear((SOPC_ByteString*) xmlElt);
}



void SOPC_DateTime_InitializeAux(void* value){
    SOPC_DateTime_Initialize((SOPC_DateTime*) value);
}


void SOPC_DateTime_Initialize(SOPC_DateTime* dateTime){
    if(dateTime != NULL){
        dateTime->Low32 = 0;
        dateTime->High32 = 0;
    }
}


void SOPC_DateTime_ClearAux(void* value){
    SOPC_DateTime_Clear((SOPC_DateTime*) value);
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


void SOPC_Guid_InitializeAux(void* value){
    SOPC_Guid_Initialize((SOPC_Guid*) value);
}

void SOPC_Guid_Initialize(SOPC_Guid* guid){
    if(guid != NULL){
        memset(guid, 0, sizeof(SOPC_Guid));
    }
}


void SOPC_Guid_ClearAux(void* value){
    SOPC_Guid_Clear((SOPC_Guid*) value);
}

void SOPC_Guid_Clear(SOPC_Guid* guid){
    if(guid != NULL){
        memset(guid, 0, sizeof(SOPC_Guid));
    }
}


void SOPC_NodeId_InitializeAux(void* value){
    SOPC_NodeId_Initialize((SOPC_NodeId*) value);
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


void SOPC_NodeId_ClearAux(void* value){
    SOPC_NodeId_Clear((SOPC_NodeId*) value);
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


void SOPC_ExpandedNodeId_InitializeAux(void* value){
    SOPC_ExpandedNodeId_Initialize((SOPC_ExpandedNodeId*) value);
}

void SOPC_ExpandedNodeId_Initialize(SOPC_ExpandedNodeId* expNodeId){
    if(expNodeId != NULL){
        SOPC_String_Initialize(&expNodeId->NamespaceUri);
        SOPC_NodeId_Initialize(&expNodeId->NodeId);
        SOPC_UInt32_Initialize(&expNodeId->ServerIndex);
    }
}


void SOPC_ExpandedNodeId_ClearAux(void* value){
    SOPC_ExpandedNodeId_Clear((SOPC_ExpandedNodeId*) value);
}

void SOPC_ExpandedNodeId_Clear(SOPC_ExpandedNodeId* expNodeId){
    if(expNodeId != NULL){
        SOPC_String_Clear(&expNodeId->NamespaceUri);
        SOPC_NodeId_Clear(&expNodeId->NodeId);
        SOPC_UInt32_Clear(&expNodeId->ServerIndex);
    }
}


void SOPC_StatusCode_InitializeAux(void* value){
    SOPC_StatusCode_Initialize((SOPC_StatusCode*) value);
}

void SOPC_StatusCode_Initialize(SOPC_StatusCode* status){
    if(status != NULL){
        *status = STATUS_OK;
    }
}


void SOPC_StatusCode_ClearAux(void* value){
    SOPC_StatusCode_Clear((SOPC_StatusCode*) value);
}

void SOPC_StatusCode_Clear(SOPC_StatusCode* status){
    if(status != NULL){
        *status = STATUS_OK;
    }
}


void SOPC_DiagnosticInfo_InitializeAux(void* value){
    SOPC_DiagnosticInfo_Initialize((SOPC_DiagnosticInfo*) value);
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


void SOPC_DiagnosticInfo_ClearAux(void* value){
    SOPC_DiagnosticInfo_Clear((SOPC_DiagnosticInfo*) value);
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



void SOPC_QualifiedName_InitializeAux(void* value){
    SOPC_QualifiedName_Initialize((SOPC_QualifiedName*) value);
}


void SOPC_QualifiedName_Initialize(SOPC_QualifiedName* qname){
    if(qname != NULL){
        qname->NamespaceIndex = 0;
        SOPC_String_Initialize(&qname->Name);
    }
}


void SOPC_QualifiedName_ClearAux(void* value){
    SOPC_QualifiedName_Clear((SOPC_QualifiedName*) value);
}

void SOPC_QualifiedName_Clear(SOPC_QualifiedName* qname){
    if(qname != NULL){
        qname->NamespaceIndex = 0;
        SOPC_String_Clear(&qname->Name);
    }
}


void SOPC_LocalizedText_InitializeAux(void* value){
    SOPC_LocalizedText_Initialize((SOPC_LocalizedText*) value);
}

void SOPC_LocalizedText_Initialize(SOPC_LocalizedText* localizedText){
    if(localizedText != NULL){
        SOPC_String_Initialize(&localizedText->Locale);
        SOPC_String_Initialize(&localizedText->Text);
    }
}


void SOPC_LocalizedText_ClearAux(void* value){
    SOPC_LocalizedText_Clear((SOPC_LocalizedText*) value);
}

void SOPC_LocalizedText_Clear(SOPC_LocalizedText* localizedText){
    if(localizedText != NULL){
        SOPC_String_Clear(&localizedText->Locale);
        SOPC_String_Clear(&localizedText->Text);
    }
}


void SOPC_ExtensionObject_InitializeAux(void* value){
    SOPC_ExtensionObject_Initialize((SOPC_ExtensionObject*) value);
}

void SOPC_ExtensionObject_Initialize(SOPC_ExtensionObject* extObj){
    if(extObj != NULL){
        memset(extObj, 0, sizeof(SOPC_ExtensionObject));
        SOPC_ExpandedNodeId_Initialize(&extObj->TypeId);
        extObj->Length = -1;
    }
}


void SOPC_ExtensionObject_ClearAux(void* value){
    SOPC_ExtensionObject_Clear((SOPC_ExtensionObject*) value);
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

void ApplyToVariantNonArrayBuiltInType(SOPC_BuiltinId                    builtInTypeId,
                                       SOPC_VariantValue*                val,
                                       SOPC_EncodeableObject_PfnClear*   clearAuxFunction){
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            clearAuxFunction((void*)&val->Boolean);
            break;
        case SOPC_SByte_Id:
            clearAuxFunction((void*)&val->Sbyte);
            break;
        case SOPC_Byte_Id:
            clearAuxFunction((void*)&val->Byte);
            break;
        case SOPC_Int16_Id:
            clearAuxFunction((void*)&val->Int16);
            break;
        case SOPC_UInt16_Id:
            clearAuxFunction((void*)&val->Uint16);
            break;
        case SOPC_Int32_Id:
            clearAuxFunction((void*)&val->Int32);
            break;
        case SOPC_UInt32_Id:
            clearAuxFunction((void*)&val->Uint32);
            break;
        case SOPC_Int64_Id:
            clearAuxFunction((void*)&val->Int64);
            break;
        case SOPC_UInt64_Id:
            clearAuxFunction((void*)&val->Uint64);
            break;
        case SOPC_Float_Id:
            clearAuxFunction((void*)&val->Floatv);
            break;
        case SOPC_Double_Id:
            clearAuxFunction((void*)&val->Doublev);
            break;
        case SOPC_String_Id:
            clearAuxFunction((void*)&val->String);
            break;
        case SOPC_DateTime_Id:
            clearAuxFunction((void*)&val->Date);
            break;
        case SOPC_Guid_Id:
            clearAuxFunction((void*)val->Guid);
            break;
        case SOPC_ByteString_Id:
            clearAuxFunction((void*)&val->Bstring);
            break;
        case SOPC_XmlElement_Id:
            clearAuxFunction((void*)&val->XmlElt);
            break;
        case SOPC_NodeId_Id:
            clearAuxFunction((void*)val->NodeId);
            break;
        case SOPC_ExpandedNodeId_Id:
            clearAuxFunction((void*)val->ExpNodeId);
            break;
        case SOPC_StatusCode_Id:
            clearAuxFunction((void*)&val->Status);
            break;
        case SOPC_QualifiedName_Id:
            clearAuxFunction((void*)val->Qname);
            break;
        case SOPC_LocalizedText_Id:
            clearAuxFunction((void*)val->LocalizedText);
            break;
        case SOPC_ExtensionObject_Id:
            clearAuxFunction((void*)val->ExtObject);
            break;
        case SOPC_DataValue_Id:
            clearAuxFunction((void*)val->DataValue);
            break;
        case SOPC_DiagnosticInfo_Id:
            clearAuxFunction((void*)val->DiagInfo);
            break;
        case SOPC_Variant_Id:
            // Part 6 Table 14 (v1.03): "The value shall not be a Variant
            //                           but it could be an array of Variants."
            //Note: Variant is not encoded in INGOPCS stack for this case
            break;
        default:
            break;
    }
}

void ApplyToVariantArrayBuiltInType(SOPC_BuiltinId                  builtInTypeId,
                                    SOPC_VariantArrayValue          array,
                                    int32_t*                        length,
                                    SOPC_EncodeableObject_PfnClear* clearAuxFunction){
    switch(builtInTypeId){
        case SOPC_Boolean_Id:
            SOPC_Clear_Array(length, (void**) &array.BooleanArr, sizeof(SOPC_Boolean), clearAuxFunction);
            break;
        case SOPC_SByte_Id:
            SOPC_Clear_Array(length, (void**) &array.SbyteArr, sizeof(SOPC_SByte), clearAuxFunction);
            break;
        case SOPC_Byte_Id:
            SOPC_Clear_Array(length, (void**) &array.ByteArr, sizeof(SOPC_Byte), clearAuxFunction);
            break;
        case SOPC_Int16_Id:
            SOPC_Clear_Array(length, (void**) &array.Int16Arr, sizeof(int16_t), clearAuxFunction);
            break;
        case SOPC_UInt16_Id:
            SOPC_Clear_Array(length, (void**) &array.Uint16Arr, sizeof(uint16_t), clearAuxFunction);
            break;
        case SOPC_Int32_Id:
            SOPC_Clear_Array(length, (void**) &array.Int32Arr, sizeof(int32_t), clearAuxFunction);
            break;
        case SOPC_UInt32_Id:
            SOPC_Clear_Array(length, (void**) &array.Uint32Arr, sizeof(uint32_t), clearAuxFunction);
            break;
        case SOPC_Int64_Id:
            SOPC_Clear_Array(length, (void**) &array.Int64Arr, sizeof(int64_t), clearAuxFunction);
            break;
        case SOPC_UInt64_Id:
            SOPC_Clear_Array(length, (void**) &array.Uint64Arr, sizeof(uint64_t), clearAuxFunction);
            break;
        case SOPC_Float_Id:
            SOPC_Clear_Array(length, (void**) &array.FloatvArr, sizeof(float), clearAuxFunction);
            break;
        case SOPC_Double_Id:
            SOPC_Clear_Array(length, (void**) &array.DoublevArr, sizeof(double), clearAuxFunction);
            break;
        case SOPC_String_Id:
            SOPC_Clear_Array(length, (void**) &array.StringArr, sizeof(SOPC_String), clearAuxFunction);
            break;
        case SOPC_DateTime_Id:
            SOPC_Clear_Array(length, (void**) &array.DateArr, sizeof(SOPC_DateTime), clearAuxFunction);
            break;
        case SOPC_Guid_Id:
            SOPC_Clear_Array(length, (void**) &array.GuidArr, sizeof(SOPC_Guid), clearAuxFunction);
            break;
        case SOPC_ByteString_Id:
            SOPC_Clear_Array(length, (void**) &array.BstringArr, sizeof(SOPC_ByteString), clearAuxFunction);
            break;
        case SOPC_XmlElement_Id:
            SOPC_Clear_Array(length, (void**) &array.XmlEltArr, sizeof(SOPC_XmlElement), clearAuxFunction);
            break;
        case SOPC_NodeId_Id:
            SOPC_Clear_Array(length, (void**) &array.NodeIdArr, sizeof(SOPC_NodeId), clearAuxFunction);
            break;
        case SOPC_ExpandedNodeId_Id:
            SOPC_Clear_Array(length, (void**) &array.ExpNodeIdArr, sizeof(SOPC_ExpandedNodeId), clearAuxFunction);
            break;
        case SOPC_StatusCode_Id:
            SOPC_Clear_Array(length, (void**) &array.StatusArr, sizeof(SOPC_StatusCode), clearAuxFunction);
            break;
        case SOPC_QualifiedName_Id:
            SOPC_Clear_Array(length, (void**) &array.QnameArr, sizeof(SOPC_QualifiedName), clearAuxFunction);
            break;
        case SOPC_LocalizedText_Id:
            SOPC_Clear_Array(length, (void**) &array.LocalizedTextArr, sizeof(SOPC_LocalizedText), clearAuxFunction);
            break;
        case SOPC_ExtensionObject_Id:
            SOPC_Clear_Array(length, (void**) &array.ExtObjectArr, sizeof(SOPC_ExtensionObject), clearAuxFunction);
            break;
        case SOPC_DataValue_Id:
            SOPC_Clear_Array(length, (void**) &array.DataValueArr, sizeof(SOPC_DataValue), clearAuxFunction);
            break;
        case SOPC_Variant_Id:
            SOPC_Clear_Array(length, (void**) &array.VariantArr, sizeof(SOPC_Variant), clearAuxFunction);
            break;
        case SOPC_DiagnosticInfo_Id:
            SOPC_Clear_Array(length, (void**) &array.DiagInfoArr, sizeof(SOPC_DiagnosticInfo), clearAuxFunction);
            break;
        default:
            break;
    }
}


void SOPC_Variant_InitializeAux(void* value){
    SOPC_Variant_Initialize((SOPC_Variant*) value);
}

void SOPC_Variant_Initialize(SOPC_Variant* variant){
    if(variant != NULL){
        memset(variant, 0, sizeof(SOPC_Variant));
    }
}
 
SOPC_EncodeableObject_PfnClear* GetBuiltInTypeClearFunction(SOPC_BuiltinId builtInTypeId){
    SOPC_EncodeableObject_PfnClear* clearFunction = NULL;
    switch(builtInTypeId){
            case SOPC_Boolean_Id:
                clearFunction = SOPC_Boolean_ClearAux;
                break;
            case SOPC_SByte_Id:
                clearFunction = SOPC_SByte_ClearAux;
                break;
            case SOPC_Byte_Id:
                clearFunction = SOPC_Byte_ClearAux;
                break;
            case SOPC_Int16_Id:
                clearFunction = SOPC_Int16_ClearAux;
                break;
            case SOPC_UInt16_Id:
                clearFunction = SOPC_UInt16_ClearAux;
                break;
            case SOPC_Int32_Id:
                clearFunction = SOPC_Int32_ClearAux;
                break;
            case SOPC_UInt32_Id:
                clearFunction = SOPC_UInt32_ClearAux;
                break;
            case SOPC_Int64_Id:
                clearFunction = SOPC_Int64_ClearAux;
                break;
            case SOPC_UInt64_Id:
                clearFunction = SOPC_UInt64_ClearAux;
                break;
            case SOPC_Float_Id:
                clearFunction = SOPC_Float_ClearAux;
                break;
            case SOPC_Double_Id:
                clearFunction = SOPC_Double_ClearAux;
                break;
            case SOPC_String_Id:
                clearFunction = SOPC_String_ClearAux;
                break;
            case SOPC_DateTime_Id:
                clearFunction = SOPC_DateTime_ClearAux;
                break;
            case SOPC_Guid_Id:
                clearFunction = SOPC_Guid_ClearAux;
                break;
            case SOPC_ByteString_Id:
                clearFunction = SOPC_ByteString_ClearAux;
                break;
            case SOPC_XmlElement_Id:
                clearFunction = SOPC_XmlElement_ClearAux;
                break;
            case SOPC_NodeId_Id:
                clearFunction = SOPC_NodeId_ClearAux;
                break;
            case SOPC_ExpandedNodeId_Id:
                clearFunction = SOPC_ExpandedNodeId_ClearAux;
                break;
            case SOPC_StatusCode_Id:
                clearFunction = SOPC_StatusCode_ClearAux;
                break;
            case SOPC_QualifiedName_Id:
                clearFunction = SOPC_QualifiedName_ClearAux;
                break;
            case SOPC_LocalizedText_Id:
                clearFunction = SOPC_LocalizedText_ClearAux;
                break;
            case SOPC_ExtensionObject_Id:
                clearFunction = SOPC_ExtensionObject_ClearAux;
                break;
            case SOPC_DataValue_Id:
                clearFunction = SOPC_DataValue_ClearAux;
                break;
            case SOPC_Variant_Id:
                clearFunction = SOPC_Variant_ClearAux;
                break;
            case SOPC_DiagnosticInfo_Id:
                clearFunction = SOPC_DiagnosticInfo_ClearAux;
                break;
            default:
                break;
        }
    return clearFunction;
}

void FreeVariantNonArrayBuiltInType(SOPC_BuiltinId     builtInTypeId,
                                    SOPC_VariantValue* val)
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
            val->Guid = NULL;
            break;
        case SOPC_NodeId_Id:
            if(NULL != val->NodeId){
                free(val->NodeId);
            }
            val->NodeId = NULL;
            break;
        case SOPC_ExpandedNodeId_Id:
            if(NULL != val->ExpNodeId){
                free(val->ExpNodeId);
            }
            val->ExpNodeId = NULL;
            break;
        case SOPC_QualifiedName_Id:
            if(NULL != val->Qname){
                free(val->Qname);
            }
            val->Qname = NULL;
            break;
        case SOPC_LocalizedText_Id:
            if(NULL != val->LocalizedText){
                free(val->LocalizedText);
            }
            val->LocalizedText = NULL;
            break;
        case SOPC_ExtensionObject_Id:
            if(NULL != val->ExtObject){
                free(val->ExtObject);
            }
            val->ExtObject = NULL;
            break;
        case SOPC_DataValue_Id:
            if(NULL != val->DataValue){
                free(val->DataValue);
            }
            val->DataValue = NULL;
            break;
        case SOPC_DiagnosticInfo_Id:
            if(NULL != val->DiagInfo){
                free(val->DiagInfo);
            }
            val->DiagInfo = NULL;
            break;
        case SOPC_Variant_Id:
            // Part 6 Table 14 (v1.03): "The value shall not be a Variant
            //                           but it could be an array of Variants."
            //Note: Variant is not encoded in INGOPCS stack for this case
            break;
    }
}


void SOPC_Variant_ClearAux(void* value){
    SOPC_Variant_Clear((SOPC_Variant*) value);
}

void SOPC_Variant_Clear(SOPC_Variant* variant){
    uint8_t error = FALSE;
    int64_t matrixLength = 1; // For multiplication to compute from dimensions values
    int32_t idx = 0;
    if(variant != NULL){
        SOPC_EncodeableObject_PfnClear* clearFunction = GetBuiltInTypeClearFunction(variant->BuiltInTypeId);
        if(clearFunction == NULL)
            return;

        switch(variant->ArrayType){
            case SOPC_VariantArrayType_SingleValue:
                ApplyToVariantNonArrayBuiltInType(variant->BuiltInTypeId,
                                                  &variant->Value,
                                                  clearFunction);
                FreeVariantNonArrayBuiltInType(variant->BuiltInTypeId,
                                               &variant->Value);
                break;
            case SOPC_VariantArrayType_Array:
                ApplyToVariantArrayBuiltInType(variant->BuiltInTypeId,
                                               variant->Value.Array.Content,
                                               &variant->Value.Array.Length,
                                               clearFunction);
                break;
            case SOPC_VariantArrayType_Matrix:
                if(variant->Value.Matrix.Dimensions == 0){
                    matrixLength = 0;
                }
                for(idx = 0; idx < variant->Value.Matrix.Dimensions && FALSE == error; idx ++){
                    if(variant->Value.Matrix.ArrayDimensions[idx] > 0 &&
                       matrixLength * variant->Value.Matrix.ArrayDimensions[idx] <= INT32_MAX)
                    {
                        matrixLength *= variant->Value.Matrix.ArrayDimensions[idx];
                    }else{
                        error = 1;
                    }
                }
                if(FALSE == error){
                    free(variant->Value.Matrix.ArrayDimensions);
                    variant->Value.Matrix.ArrayDimensions = NULL;
                    ApplyToVariantArrayBuiltInType(variant->BuiltInTypeId,
                                                   variant->Value.Matrix.Content,
                                                   (int32_t*) &matrixLength,
                                                   clearFunction);
                    variant->Value.Matrix.Dimensions = 0;
                }
                break;
        }

        // Reset internal properties
        SOPC_Variant_Initialize(variant);
    }
}


void SOPC_DataValue_InitializeAux(void* value){
    SOPC_DataValue_Initialize((SOPC_DataValue*) value);
}

void SOPC_DataValue_Initialize(SOPC_DataValue* dataValue){
    if(dataValue != NULL){
        memset(dataValue, 0, sizeof(SOPC_DataValue));
    }
}

void SOPC_DataValue_ClearAux(void* value){
    SOPC_DataValue_Clear((SOPC_DataValue*) value);
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


void SOPC_Initialize_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                           SOPC_EncodeableObject_PfnInitialize* initFct)
{
    (void) initFct;
    (void) sizeOfElt;
    *noOfElts = 0;
    *eltsArray = NULL;
}

void SOPC_Clear_Array(int32_t* noOfElts, void** eltsArray, size_t sizeOfElt,
                      SOPC_EncodeableObject_PfnClear* clearFct)
{
    size_t idx = 0;
    size_t pos = 0;
    SOPC_Byte* byteArray = NULL;
    if(noOfElts != NULL && *noOfElts >= 0 &&
       (size_t) *noOfElts <= SIZE_MAX &&
       eltsArray != NULL){
        byteArray = *eltsArray;
        if(byteArray != NULL){
            for (idx = 0; idx < (size_t) *noOfElts; idx ++){
                pos = idx * sizeOfElt;
                clearFct(&(byteArray[pos]));
            }

            free(*eltsArray);
        }
        *noOfElts = 0;
        *eltsArray = NULL;
    }
}
