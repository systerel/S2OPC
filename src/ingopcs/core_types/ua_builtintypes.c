/*
 * ua_ingopcs_type.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "ua_builtintypes.h"

typedef void (BuiltInFunction) (void*);

void Boolean_Initialize(UA_Boolean* b){
    *b = FALSE;
}

void Boolean_Clear(UA_Boolean* b){
    *b = FALSE;
}

void SByte_Initialize(UA_SByte* sbyte){
    *sbyte = 0;
}

void SByte_Clear(UA_SByte* sbyte){
    *sbyte = 0;
}

void Byte_Initialize(UA_Byte* byte){
    *byte = 0;
}

void Byte_Clear(UA_Byte* byte){
    *byte = 0;
}

void Int16_Initialize(int16_t* intv){
    *intv = 0;
}

void Int16_Clear(int16_t* intv){
    *intv = 0;
}

void UInt16_Initialize(uint16_t* uint){
    *uint = 0;
}
void UInt16_Clear(uint16_t* uint){
    *uint = 0;
}

void Int32_Initialize(int32_t* intv){
    *intv = 0;
}
void Int32_Clear(int32_t* intv){
    *intv = 0;
}

void UInt32_Initialize(uint32_t* uint){
    *uint = 0;
}
void UInt32_Clear(uint32_t* uint){
    *uint = 0;
}

void Int64_Initialize(int64_t* intv){
    *intv = 0;
}
void Int64_Clear(int64_t* intv){
    *intv = 0;
}

void UInt64_Initialize(uint64_t* uint){
    *uint = 0;
}
void UInt64_Clear(uint64_t* uint){
    *uint = 0;
}

void Float_Initialize(float* f){
    *f = 0.0;
}

void Float_Clear(float* f){
    *f = 0.0;
}

void Double_Initialize(double* d){
    *d = 0.0;
}

void Double_Clear(double* d){
    *d = 0.0;
}

void ByteString_Initialize(UA_ByteString* bstring){
    if(bstring != NULL){
        bstring->Length = -1;
        bstring->Data = NULL;
        bstring->ClearBytes = 1; // True unless characters attached
    }
}

UA_ByteString* ByteString_Create(){
    UA_ByteString* bstring = NULL;
    bstring = (UA_ByteString*) malloc(sizeof(UA_ByteString));
    ByteString_Initialize(bstring);
    return bstring;
}

SOPC_StatusCode ByteString_InitializeFixedSize(UA_ByteString* bstring, uint32_t size){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(bstring != NULL){
        status = STATUS_OK;
        ByteString_Initialize(bstring);
        bstring->Length = size;
        bstring->Data = (UA_Byte*) malloc (sizeof(UA_Byte)*size);
        if(bstring->Data != NULL){
            memset(bstring->Data, 0, size);
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

SOPC_StatusCode ByteString_AttachFromBytes(UA_ByteString* dest, UA_Byte* bytes, int32_t length)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && bytes != NULL
       && length > 0){
        status = STATUS_OK;
        dest->Length = length;
        dest->Data = bytes;
        dest->ClearBytes = FALSE; // dest->characters will not be freed on clear
    }
    return status;
}

SOPC_StatusCode ByteString_AttachFrom(UA_ByteString* dest, UA_ByteString* src)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && src != NULL
       && src->Length > 0 && src->Data != NULL){
        status = STATUS_OK;
        dest->Length = src->Length;
        dest->Data = src->Data;
        dest->ClearBytes = FALSE; // dest->characters will not be freed on clear
    }
    return status;
}

SOPC_StatusCode ByteString_Copy(UA_ByteString* dest, const UA_ByteString* src){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && dest->Data == NULL &&
       src != NULL && src->Length > 0){
        status = STATUS_OK;
        dest->Length = src->Length;
        dest->Data = (UA_Byte*) malloc (sizeof(UA_Byte)*dest->Length);
        if(dest->Data != NULL){
            // No need of secure copy, both have same size here
            memcpy(dest->Data, src->Data, dest->Length);
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

void ByteString_Clear(UA_ByteString* bstring){
    if(bstring != NULL){
        if(bstring->Data != NULL &&
           bstring->ClearBytes != FALSE){
            free(bstring->Data);
            bstring->Data = NULL;
        }
    }
}

void ByteString_Delete(UA_ByteString* bstring){
    if(bstring != NULL){
        ByteString_Clear(bstring);
        free(bstring);
    }
}

void String_Initialize(UA_String* string){
    ByteString_Initialize((UA_ByteString*) string);
}

UA_String* String_Create(){
    return (UA_String*) ByteString_Create();
}

SOPC_StatusCode String_AttachFrom(UA_String* dest, UA_String* src){
    return ByteString_AttachFrom((UA_ByteString*) dest, (UA_ByteString*) src);
}

SOPC_StatusCode String_AttachFromCstring(UA_String* dest, char* src){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && dest->Data == NULL && src != NULL){
        status = STATUS_OK;
        if(CHAR_BIT == 8){
            dest->Length = strlen(src);
            dest->Data = (uint8_t*) src;
            dest->ClearBytes = FALSE; // dest->characters will not be freed on clear
        }else{
            assert(FALSE);
        }
    }
    return status;
}

SOPC_StatusCode String_Copy(UA_String* dest, const UA_String* src){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && dest->Data == NULL && src != NULL){
        // Keep null terminator for C string compatibility
        status = STATUS_OK;
        dest->Length = src->Length;
        dest->Data = (UA_Byte*) malloc (sizeof(UA_Byte)*dest->Length+1);
        if(dest->Data != NULL){
            // No need of secure copy, both have same size here
            memcpy(dest->Data, src->Data, dest->Length);
            dest->Data[dest->Length] = '\0';
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

void String_Clear(UA_String* string){
    ByteString_Clear((UA_ByteString*) string);
}

void String_Delete(UA_String* string){
    ByteString_Delete((UA_ByteString*) string);
}

SOPC_StatusCode String_CopyFromCString(UA_String* string, const char* cString){
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
            string->Data = (UA_Byte*) malloc(sizeof(UA_Byte)*(stringLength+1));
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

SOPC_StatusCode String_InitializeFromCString(UA_String* string, const char* cString){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;

    if(string != NULL){
        String_Initialize(string);
        status = String_CopyFromCString(string, cString);
    }

    return status;
}


char* String_GetCString(const UA_String* string){
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

const char* String_GetRawCString(const UA_String* string){
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

SOPC_StatusCode ByteString_Compare(const UA_ByteString* left,
                              const UA_ByteString* right,
                              int32_t*             comparison)
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

uint32_t ByteString_Equal(const UA_ByteString* left,
                          const UA_ByteString* right)
{
    int32_t compare = 0;
    uint32_t result = FALSE;

    if(ByteString_Compare(left, right, &compare) == STATUS_OK){
        result = compare == 0;
    }

    return result;
}

SOPC_StatusCode String_Compare(const UA_String* left,
                          const UA_String* right,
                          int32_t*         comparison)
{

    return ByteString_Compare((UA_ByteString*) left,
                              (UA_ByteString*) right, comparison);
}

uint32_t String_Equal(const UA_String* left,
                      const UA_String* right)
{
    return ByteString_Equal((UA_ByteString*) left,
                              (UA_ByteString*) right);
}

void XmlElement_Initialize(UA_XmlElement* xmlElt){
    ByteString_Initialize((UA_ByteString*) xmlElt);
}

void XmlElement_Clear(UA_XmlElement* xmlElt){
    ByteString_Clear((UA_ByteString*) xmlElt);
}


void DateTime_Initialize(UA_DateTime* dateTime){
    *dateTime = 0;
}

void DateTime_Clear(UA_DateTime* dateTime){
    *dateTime = 0;
}

void Guid_Initialize(UA_Guid* guid){
    memset(guid, 0, sizeof(UA_Guid));
}

void Guid_Clear(UA_Guid* guid){
    memset(guid, 0, sizeof(UA_Guid));
}

void NodeId_Initialize(UA_NodeId* nodeId){
    memset(nodeId, 0, sizeof(UA_NodeId));
}

void NodeId_InitType(UA_NodeId* nodeId, UA_IdentifierType knownIdType){
    nodeId->Namespace = 0; // OPCUA namespace
    nodeId->IdentifierType = knownIdType;
    switch(knownIdType){
        case IdentifierType_Numeric:
            UInt32_Initialize(&nodeId->Data.Numeric);
            break;
        case IdentifierType_String:
            String_Initialize(&nodeId->Data.String);
            break;
        case IdentifierType_Guid:
            Guid_Initialize(&nodeId->Data.Guid);
            break;
        case IdentifierType_ByteString:
            ByteString_Initialize(&nodeId->Data.Bstring);
            break;
    }
}

void NodeId_Clear(UA_NodeId* nodeId){
    nodeId->Namespace = 0; // OPCUA namespace
    switch(nodeId->IdentifierType){
        case IdentifierType_Numeric:
            UInt32_Clear(&nodeId->Data.Numeric);
            break;
        case IdentifierType_String:
            String_Clear(&nodeId->Data.String);
            break;
        case IdentifierType_Guid:
            Guid_Clear(&nodeId->Data.Guid);
            break;
        case IdentifierType_ByteString:
            ByteString_Clear(&nodeId->Data.Bstring);
            break;
    }
    nodeId->IdentifierType = IdentifierType_Numeric;
}

void ExpandedNodeId_Initialize(UA_ExpandedNodeId* expNodeId){
    String_Initialize(&expNodeId->NamespaceUri);
    NodeId_Initialize(&expNodeId->NodeId);
    UInt32_Initialize(&expNodeId->ServerIndex);
}

void ExpandedNodeId_Clear(UA_ExpandedNodeId* expNodeId){
    String_Initialize(&expNodeId->NamespaceUri);
    NodeId_Initialize(&expNodeId->NodeId);
    UInt32_Initialize(&expNodeId->ServerIndex);
}

void StatusCode_Initialize(SOPC_StatusCode* status){
    *status = STATUS_OK;
}

void StatusCode_Clear(SOPC_StatusCode* status){
    *status = STATUS_OK;
}

void DiagnosticInfo_Initialize(UA_DiagnosticInfo* diagInfo){
    if(diagInfo != NULL){
        diagInfo->SymbolicId = -1;
        diagInfo->NamespaceUri = -1;
        diagInfo->Locale = -1;
        diagInfo->LocalizedText = -1;
        String_Initialize(&diagInfo->AdditionalInfo);
        diagInfo->InnerStatusCode = STATUS_OK;
        diagInfo->InnerDiagnosticInfo = NULL;
    }
}

void DiagnosticInfo_Clear(UA_DiagnosticInfo* diagInfo){
    if(diagInfo != NULL){
        String_Clear(&diagInfo->AdditionalInfo);
        if(diagInfo->InnerDiagnosticInfo != NULL){
            DiagnosticInfo_Clear(diagInfo->InnerDiagnosticInfo);
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


void QualifiedName_Initialize(UA_QualifiedName* qname){
    qname->NamespaceIndex = 0;
    String_Initialize(&qname->Name);
}

void QualifiedName_Clear(UA_QualifiedName* qname){
    qname->NamespaceIndex = 0;
    String_Clear(&qname->Name);
}

void LocalizedText_Initialize(UA_LocalizedText* localizedText){
    String_Initialize(&localizedText->Locale);
    String_Initialize(&localizedText->Text);
}

void LocalizedText_Clear(UA_LocalizedText* localizedText){
    String_Clear(&localizedText->Locale);
    String_Clear(&localizedText->Text);
}

void ExtensionObject_Initialize(UA_ExtensionObject* extObj){
    memset(extObj, 0, sizeof(UA_ExtensionObject));
    NodeId_Initialize(&extObj->TypeId);
    extObj->Length = -1;
}

void ExtensionObject_Clear(UA_ExtensionObject* extObj){
    NodeId_Clear(&extObj->TypeId);
    switch(extObj->Encoding){
        case UA_ExtObjBodyEncoding_None:
            break;
        case UA_ExtObjBodyEncoding_ByteString:
            ByteString_Clear(&extObj->Body.Bstring);
            break;
        case UA_ExtObjBodyEncoding_XMLElement:
            XmlElement_Clear(&extObj->Body.Xml);
            break;
        case UA_ExtObjBodyEncoding_Object:
            extObj->Body.Object.ObjType->Clear(extObj->Body.Object.Value);
            break;
    }
    extObj->Length = -1;
}

void ApplyToVariantNonArrayBuiltInType(UA_BuiltinId builtInTypeId,
                                       UA_VariantValue val,
                                       BuiltInFunction* builtInFunction){
    switch(builtInTypeId){
        case UA_Boolean_Id:
            builtInFunction(&val.Boolean);
            break;
        case UA_SByte_Id:
            builtInFunction(&val.Sbyte);
            break;
        case UA_Byte_Id:
            builtInFunction(&val.Byte);
            break;
        case UA_Int16_Id:
            builtInFunction(&val.Int16);
            break;
        case UA_UInt16_Id:
            builtInFunction(&val.Uint16);
            break;
        case UA_Int32_Id:
            builtInFunction(&val.Int32);
            break;
        case UA_UInt32_Id:
            builtInFunction(&val.Uint32);
            break;
        case UA_Int64_Id:
            builtInFunction(&val.Int64);
            break;
        case UA_UInt64_Id:
            builtInFunction(&val.Uint64);
            break;
        case UA_Float_Id:
            builtInFunction(&val.Floatv);
            break;
        case UA_Double_Id:
            builtInFunction(&val.Doublev);
            break;
        case UA_String_Id:
            builtInFunction(&val.String);
            break;
        case UA_DateTime_Id:
            builtInFunction(&val.Date);
            break;
        case UA_Guid_Id:
            builtInFunction(val.Guid);
            break;
        case UA_ByteString_Id:
            builtInFunction(&val.Bstring);
            break;
        case UA_XmlElement_Id:
            builtInFunction(&val.XmlElt);
            break;
        case UA_NodeId_Id:
            builtInFunction(val.NodeId);
            break;
        case UA_ExpandedNodeId_Id:
            builtInFunction(val.ExpNodeId);
            break;
        case UA_StatusCode_Id:
            builtInFunction(&val.Status);
            break;
        case UA_QualifiedName_Id:
            builtInFunction(val.Qname);
            break;
        case UA_LocalizedText_Id:
            builtInFunction(val.LocalizedText);
            break;
        case UA_ExtensionObject_Id:
            builtInFunction(val.ExtObject);
            break;
        case UA_DataValue_Id:
            builtInFunction(val.DataValue);
            break;
        case UA_Variant_Id:
            assert(FALSE);
            break;
        case UA_DiagnosticInfo_Id:
            builtInFunction(val.DiagInfo);
            break;
        default:
            break;
    }
}

void ApplyToVariantArrayBuiltInType(UA_BuiltinId builtInTypeId,
                                    UA_VariantArrayValue array,
                                    int32_t length,
                                    BuiltInFunction* builtInFunction){
    int32_t idx = 0;
    switch(builtInTypeId){
        case UA_Boolean_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.BooleanArr[idx]);
            }
            break;
        case UA_SByte_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.SbyteArr[idx]);
            }
            break;
        case UA_Byte_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.ByteArr[idx]);
            }
            break;
        case UA_Int16_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Int16Arr[idx]);
            }
            break;
        case UA_UInt16_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Uint16Arr[idx]);
            }
            break;
        case UA_Int32_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Int32Arr[idx]);
            }
            break;
        case UA_UInt32_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Uint32Arr[idx]);
            }
            break;
        case UA_Int64_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Int64Arr[idx]);
            }
            break;
        case UA_UInt64_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.Uint64Arr[idx]);
            }
            break;
        case UA_Float_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.FloatvArr[idx]);
            }
            break;
        case UA_Double_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.DoublevArr[idx]);
            }
            break;
        case UA_String_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.StringArr[idx]);
            }
            break;
        case UA_DateTime_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.DateArr[idx]);
            }
            break;
        case UA_Guid_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.GuidArr[idx]);
            }
            break;
        case UA_ByteString_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.BstringArr[idx]);
            }
            break;
        case UA_XmlElement_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.XmlEltArr[idx]);
            }
            break;
        case UA_NodeId_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.NodeIdArr[idx]);
            }
            break;
        case UA_ExpandedNodeId_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.ExpNodeIdArr[idx]);
            }
            break;
        case UA_StatusCode_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.StatusArr[idx]);
            }
            break;
        case UA_QualifiedName_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.QnameArr[idx]);
            }
            break;
        case UA_LocalizedText_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.LocalizedTextArr[idx]);
            }
            break;
        case UA_ExtensionObject_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.ExtObjectArr[idx]);
            }
            break;
        case UA_DataValue_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.DataValueArr[idx]);
            }
            break;
        case UA_Variant_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.VariantArr[idx]);
            }
            break;
        case UA_DiagnosticInfo_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.DiagInfoArr[idx]);
            }
            break;
        default:
            break;
    }
}

void Variant_Initialize(UA_Variant* variant){
    memset(variant, 0, sizeof(UA_Variant));
}

BuiltInFunction* GetBuiltInTypeClearFunction(UA_BuiltinId builtInTypeId){
    BuiltInFunction* clearFunction = NULL;
    switch(builtInTypeId){
            case UA_Boolean_Id:
                clearFunction = (BuiltInFunction*) Boolean_Clear;
                break;
            case UA_SByte_Id:
                clearFunction = (BuiltInFunction*) SByte_Clear;
                break;
            case UA_Byte_Id:
                clearFunction = (BuiltInFunction*) Byte_Clear;
                break;
            case UA_Int16_Id:
                clearFunction = (BuiltInFunction*) Int16_Clear;
                break;
            case UA_UInt16_Id:
                clearFunction = (BuiltInFunction*) UInt16_Clear;
                break;
            case UA_Int32_Id:
                clearFunction = (BuiltInFunction*) Int32_Clear;
                break;
            case UA_UInt32_Id:
                clearFunction = (BuiltInFunction*) UInt32_Clear;
                break;
            case UA_Int64_Id:
                clearFunction = (BuiltInFunction*) Int64_Clear;
                break;
            case UA_UInt64_Id:
                clearFunction = (BuiltInFunction*) UInt64_Clear;
                break;
            case UA_Float_Id:
                clearFunction = (BuiltInFunction*) Float_Clear;
                break;
            case UA_Double_Id:
                clearFunction = (BuiltInFunction*) Double_Clear;
                break;
            case UA_String_Id:
                clearFunction = (BuiltInFunction*) String_Clear;
                break;
            case UA_DateTime_Id:
                clearFunction = (BuiltInFunction*) DateTime_Clear;
                break;
            case UA_Guid_Id:
                clearFunction = (BuiltInFunction*) Guid_Clear;
                break;
            case UA_ByteString_Id:
                clearFunction = (BuiltInFunction*) ByteString_Clear;
                break;
            case UA_XmlElement_Id:
                clearFunction = (BuiltInFunction*) XmlElement_Clear;
                break;
            case UA_NodeId_Id:
                clearFunction = (BuiltInFunction*) NodeId_Clear;
                break;
            case UA_ExpandedNodeId_Id:
                clearFunction = (BuiltInFunction*) ExpandedNodeId_Clear;
                break;
            case UA_StatusCode_Id:
                clearFunction = (BuiltInFunction*) StatusCode_Clear;
                break;
            case UA_QualifiedName_Id:
                clearFunction = (BuiltInFunction*) QualifiedName_Clear;
                break;
            case UA_LocalizedText_Id:
                clearFunction = (BuiltInFunction*) LocalizedText_Clear;
                break;
            case UA_ExtensionObject_Id:
                clearFunction = (BuiltInFunction*) ExtensionObject_Clear;
                break;
            case UA_DataValue_Id:
                clearFunction = (BuiltInFunction*) DataValue_Clear;
                break;
            case UA_Variant_Id:
                clearFunction = (BuiltInFunction*) Variant_Clear;
                break;
            case UA_DiagnosticInfo_Id:
                clearFunction = (BuiltInFunction*) DiagnosticInfo_Clear;
                break;
            default:
                break;
        }
    return clearFunction;
}

void Variant_Clear(UA_Variant* variant){
    BuiltInFunction* clearFunction = GetBuiltInTypeClearFunction(variant->BuiltInTypeMask);
    // Matrix flag => array flag
    assert(((variant->ArrayTypeMask & UA_VariantArrayMatrixFlag) != 0 &&
             (variant->ArrayTypeMask & UA_VariantArrayValueFlag) != 0)
           || ((variant->ArrayTypeMask & UA_VariantArrayMatrixFlag) == 0));

    if((variant->ArrayTypeMask & UA_VariantArrayValueFlag) != 0){
        int32_t length = 0;
        if((variant->ArrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
            int32_t idx = 0;
            for(idx = 0; idx < variant->Value.Matrix.Dimensions; idx ++){
                length *= variant->Value.Matrix.ArrayDimensions[idx];
            }
            ApplyToVariantArrayBuiltInType(variant->BuiltInTypeMask,
                                           variant->Value.Matrix.Content,
                                           length,
                                           clearFunction);
        }else{
            ApplyToVariantArrayBuiltInType(variant->BuiltInTypeMask,
                                           variant->Value.Array.Content,
                                           variant->Value.Array.Length,
                                           clearFunction);
        }
    }else{
        ApplyToVariantNonArrayBuiltInType(variant->BuiltInTypeMask,
                                          variant->Value,
                                          clearFunction);
    }
}

void DataValue_Initialize(UA_DataValue* dataValue){
    memset(dataValue, 0, sizeof(UA_DataValue));
}
void DataValue_Clear(UA_DataValue* dataValue){
    Variant_Clear(&dataValue->Value);
    StatusCode_Clear(&dataValue->Status);
    dataValue->SourceTimestamp = 0;
    dataValue->ServerTimestamp = 0;
    dataValue->SourcePicoSeconds = 0;
    dataValue->ServerPicoSeconds = 0;
}

