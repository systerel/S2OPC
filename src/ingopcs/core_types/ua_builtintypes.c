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

void Boolean_Initialize(UA_Boolean* b){
    *b = UA_FALSE;
}

void Boolean_Clear(UA_Boolean* b){
    *b = UA_FALSE;
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
        bstring->length = -1;
        bstring->characters = NULL;
        bstring->clearBytes = 1; // True unless characters attached
    }
}

UA_ByteString* ByteString_Create(){
    UA_ByteString* bstring = NULL;
    bstring = (UA_ByteString*) malloc(sizeof(UA_ByteString));
    ByteString_Initialize(bstring);
    return bstring;
}

StatusCode ByteString_InitializeFixedSize(UA_ByteString* bstring, uint32_t size){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(bstring != NULL){
        status = STATUS_OK;
        ByteString_Initialize(bstring);
        bstring->length = size;
        bstring->characters = (UA_Byte*) malloc (sizeof(UA_Byte)*size);
        if(bstring->characters != NULL){
            memset(bstring->characters, 0, size);
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

StatusCode ByteString_AttachFromBytes(UA_ByteString* dest, UA_Byte* bytes, int32_t length)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && bytes != NULL
       && length > 0){
        status = STATUS_OK;
        dest->length = length;
        dest->characters = bytes;
        dest->clearBytes = UA_FALSE; // dest->characters will not be freed on clear
    }
    return status;
}

StatusCode ByteString_AttachFrom(UA_ByteString* dest, UA_ByteString* src)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && src != NULL
       && src->length > 0 && src->characters != NULL){
        status = STATUS_OK;
        dest->length = src->length;
        dest->characters = src->characters;
        dest->clearBytes = UA_FALSE; // dest->characters will not be freed on clear
    }
    return status;
}

StatusCode ByteString_Copy(UA_ByteString* dest, const UA_ByteString* src){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && dest->characters == NULL &&
       src != NULL && src->length > 0){
        status = STATUS_OK;
        dest->length = src->length;
        dest->characters = (UA_Byte*) malloc (sizeof(UA_Byte)*dest->length);
        if(dest->characters != NULL){
            // No need of secure copy, both have same size here
            memcpy(dest->characters, src->characters, dest->length);
        }else{
            status = STATUS_NOK;
        }
    }
    return status;
}

void ByteString_Clear(UA_ByteString* bstring){
    if(bstring != NULL){
        if(bstring->characters != NULL &&
           bstring->clearBytes != UA_FALSE){
            free(bstring->characters);
            bstring->characters = NULL;
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

StatusCode String_AttachFrom(UA_String* dest, UA_String* src){
    return ByteString_AttachFrom((UA_ByteString*) dest, (UA_ByteString*) src);
}

StatusCode String_AttachFromCstring(UA_String* dest, char* src){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && dest->characters == NULL && src != NULL){
        status = STATUS_OK;
        if(CHAR_BIT == 8){
            dest->length = strlen(src);
            dest->characters = (uint8_t*) src;
            dest->clearBytes = UA_FALSE; // dest->characters will not be freed on clear
        }else{
            assert(UA_FALSE);
        }
    }
    return status;
}

StatusCode String_Copy(UA_String* dest, const UA_String* src){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    if(dest != NULL && dest->characters == NULL && src != NULL){
        // Keep null terminator for C string compatibility
        status = STATUS_OK;
        dest->length = src->length;
        dest->characters = (UA_Byte*) malloc (sizeof(UA_Byte)*dest->length+1);
        if(dest->characters != NULL){
            // No need of secure copy, both have same size here
            memcpy(dest->characters, src->characters, dest->length);
            dest->characters[dest->length] = '\0';
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
    String_Delete((UA_ByteString*) string);
}

StatusCode String_CopyFromCString(UA_String* string, const char* cString){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    size_t stringLength = 0;
    size_t idx = 0;
    if(string != NULL && string->characters == NULL
       && cString != NULL){
        status = STATUS_OK;

        stringLength = strlen(cString);
        if(stringLength > 0 &&
           stringLength <= INT32_MAX)
        {
            // length without null terminator
            string->length = stringLength;
            // keep terminator for compatibility with char* strings
            string->characters = (UA_Byte*) malloc(sizeof(UA_Byte)*(stringLength+1));
            if(string->characters != NULL){
                // Keep null terminator for compatibility with c strings !
                if(CHAR_BIT == 8){
                    memcpy(string->characters, cString, stringLength + 1);
                }else{
                    // On systems for which char is not encoded on 1 byte
                    for(idx = 0; idx < stringLength + 1; idx++){
                        string->characters[idx] = (uint8_t) cString[idx];
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

StatusCode String_InitializeFromCString(UA_String* string, const char* cString){
    StatusCode status = STATUS_INVALID_PARAMETERS;

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
       string->length > 0)
    {
        cString = (char*) malloc(sizeof(char)* (string->length + 1));
        if(cString != NULL){
            if(CHAR_BIT == 8){
                memcpy(cString, string->characters, string->length + 1);
            }else{
                // On systems for which char is not encoded on 1 byte
                for(idx = 0; idx < string->length + 1; idx++){
                    cString[idx] = (char) string->characters[idx];
                }
            }
        }
    }
    return cString;
}

const char* String_GetRawCString(const UA_String* string){
    char* cString = NULL;
    if(string != NULL &&
       string->length > 0)
    {
        if(CHAR_BIT == 8){
            cString = (char*) string->characters;
            assert(string->characters[string->length] == '\0');
        }else{
            assert(UA_FALSE);
        }
    }
    return cString;
}

StatusCode ByteString_Compare(const UA_ByteString* left,
                              const UA_ByteString* right,
                              uint32_t*            comparison)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;

    if(left != NULL && right != NULL){
        status = STATUS_OK;
    }

    if(status == STATUS_OK){
        if(left->length == right->length ||
           (left->length <= 0 && right->length <= 0)){
            if(left->length <= 0 && right->length <= 0){
                *comparison = 0;
            }else{
                *comparison = memcmp(left->characters, right->characters, left->length);
            }
        }else if(left->length > right->length){

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
    uint32_t compare = 0;
    uint32_t result = UA_FALSE;

    if(ByteString_Compare(left, right, &compare) == STATUS_OK){
        result = compare == 0;
    }

    return result;
}

StatusCode String_Compare(const UA_String* left,
                          const UA_String* right,
                          uint32_t*        comparison)
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
    nodeId->namespace = 0; // OPCUA namespace
    nodeId->identifierType = knownIdType;
    switch(knownIdType){
        case IdentifierType_Numeric:
            UInt32_Initialize(&nodeId->numeric);
            break;
        case IdentifierType_String:
            String_Initialize(&nodeId->string);
            break;
        case IdentifierType_Guid:
            Guid_Initialize(&nodeId->guid);
            break;
        case IdentifierType_ByteString:
            ByteString_Initialize(&nodeId->bstring);
            break;
    }
}

void NodeId_Clear(UA_NodeId* nodeId){
    nodeId->namespace = 0; // OPCUA namespace
    switch(nodeId->identifierType){
        case IdentifierType_Numeric:
            UInt32_Clear(&nodeId->numeric);
            break;
        case IdentifierType_String:
            String_Clear(&nodeId->string);
            break;
        case IdentifierType_Guid:
            Guid_Clear(&nodeId->guid);
            break;
        case IdentifierType_ByteString:
            ByteString_Clear(&nodeId->bstring);
            break;
    }
    nodeId->identifierType = IdentifierType_Numeric;
}

void ExpandedNodeId_Initialize(UA_ExpandedNodeId* expNodeId){
    String_Initialize(&expNodeId->namespaceUri);
    NodeId_Initialize(&expNodeId->nodeId);
    UInt32_Initialize(&expNodeId->serverIndex);
}

void ExpandedNodeId_Clear(UA_ExpandedNodeId* expNodeId){
    String_Initialize(&expNodeId->namespaceUri);
    NodeId_Initialize(&expNodeId->nodeId);
    UInt32_Initialize(&expNodeId->serverIndex);
}

void StatusCode_Initialize(StatusCode* status){
    *status = STATUS_OK;
}

void StatusCode_Clear(StatusCode* status){
    *status = STATUS_OK;
}

void DiagnosticInfo_Initialize(UA_DiagnosticInfo* diagInfo){
    if(diagInfo != NULL){
        diagInfo->symbolicId = -1;
        diagInfo->namespaceUri = -1;
        diagInfo->locale = -1;
        diagInfo->localizedText = -1;
        String_Initialize(&diagInfo->additionalInfo);
        diagInfo->innerStatusCode = STATUS_OK;
        diagInfo->innerDiagnosticInfo = NULL;
    }
}

void DiagnosticInfo_Clear(UA_DiagnosticInfo* diagInfo){
    if(diagInfo != NULL){
        String_Clear(&diagInfo->additionalInfo);
        if(diagInfo->innerDiagnosticInfo != NULL){
            DiagnosticInfo_Clear(diagInfo->innerDiagnosticInfo);
            free(diagInfo->innerDiagnosticInfo);
        }
        diagInfo->symbolicId = -1;
        diagInfo->namespaceUri = -1;
        diagInfo->locale = -1;
        diagInfo->localizedText = -1;
        diagInfo->innerStatusCode = STATUS_OK;
        diagInfo->innerDiagnosticInfo = NULL;
    }
}


void QualifiedName_Initialize(UA_QualifiedName* qname){
    qname->namespaceIndex = 0;
    String_Initialize(&qname->name);
}

void QualifiedName_Clear(UA_QualifiedName* qname){
    qname->namespaceIndex = 0;
    String_Clear(&qname->name);
}

void LocalizedText_Initialize(UA_LocalizedText* localizedText){
    String_Initialize(&localizedText->locale);
    String_Initialize(&localizedText->text);
}

void LocalizedText_Clear(UA_LocalizedText* localizedText){
    String_Clear(&localizedText->locale);
    String_Clear(&localizedText->text);
}

void ExtensionObject_Initialize(UA_ExtensionObject* extObj){
    memset(extObj, 0, sizeof(UA_ExtensionObject));
    NodeId_Initialize(&extObj->typeId);
    extObj->length = -1;
}

void ExtensionObject_Clear(UA_ExtensionObject* extObj){
    NodeId_Clear(&extObj->typeId);
    switch(extObj->encoding){
        case UA_ExtObjBodyEncoding_None:
            break;
        case UA_ExtObjBodyEncoding_ByteString:
            ByteString_Clear(&extObj->body.bstring);
            break;
        case UA_ExtObjBodyEncoding_XMLElement:
            XmlElement_Clear(&extObj->body.xml);
            break;
        case UA_ExtObjBodyEncoding_Object:
            extObj->body.object.objType->clearFunction(extObj->body.object.value);
            break;
    }
    extObj->length = -1;
}

typedef void (*BuiltInFunction) (void*);

void ApplyToVariantNonArrayBuiltInType(UA_BuiltinId builtInTypeId,
                                       UA_VariantValue val,
                                       BuiltInFunction builtInFunction){
    switch(builtInTypeId){
        case UA_Boolean_Id:
            builtInFunction(&val.boolean);
            break;
        case UA_SByte_Id:
            builtInFunction(&val.sbyte);
            break;
        case UA_Byte_Id:
            builtInFunction(&val.byte);
            break;
        case UA_Int16_Id:
            builtInFunction(&val.int16);
            break;
        case UA_UInt16_Id:
            builtInFunction(&val.uint16);
            break;
        case UA_Int32_Id:
            builtInFunction(&val.int32);
            break;
        case UA_UInt32_Id:
            builtInFunction(&val.uint32);
            break;
        case UA_Int64_Id:
            builtInFunction(&val.int64);
            break;
        case UA_UInt64_Id:
            builtInFunction(&val.uint64);
            break;
        case UA_Float_Id:
            builtInFunction(&val.floatv);
            break;
        case UA_Double_Id:
            builtInFunction(&val.doublev);
            break;
        case UA_String_Id:
            builtInFunction(&val.string);
            break;
        case UA_DateTime_Id:
            builtInFunction(&val.date);
            break;
        case UA_Guid_Id:
            builtInFunction(val.guid);
            break;
        case UA_ByteString_Id:
            builtInFunction(&val.bstring);
            break;
        case UA_XmlElement_Id:
            builtInFunction(&val.xmlElt);
            break;
        case UA_NodeId_Id:
            builtInFunction(val.nodeId);
            break;
        case UA_ExpandedNodeId_Id:
            builtInFunction(val.expNodeId);
            break;
        case UA_StatusCode_Id:
            builtInFunction(&val.status);
            break;
        case UA_QualifiedName_Id:
            builtInFunction(val.qname);
            break;
        case UA_LocalizedText_Id:
            builtInFunction(val.localizedText);
            break;
        case UA_ExtensionObject_Id:
            builtInFunction(val.extObject);
            break;
        case UA_DataValue_Id:
            builtInFunction(val.dataValue);
            break;
        case UA_Variant_Id:
            assert(UA_FALSE);
            break;
        case UA_DiagnosticInfo_Id:
            builtInFunction(val.diagInfo);
            break;
        default:
            break;
    }
}

void ApplyToVariantArrayBuiltInType(UA_BuiltinId builtInTypeId,
                                    UA_VariantArrayValue array,
                                    int32_t length,
                                    BuiltInFunction builtInFunction){
    int32_t idx = 0;
    switch(builtInTypeId){
        case UA_Boolean_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.booleanArr[idx]);
            }
            break;
        case UA_SByte_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.sbyteArr[idx]);
            }
            break;
        case UA_Byte_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.byteArr[idx]);
            }
            break;
        case UA_Int16_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.int16Arr[idx]);
            }
            break;
        case UA_UInt16_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.uint16Arr[idx]);
            }
            break;
        case UA_Int32_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.int32Arr[idx]);
            }
            break;
        case UA_UInt32_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.uint32Arr[idx]);
            }
            break;
        case UA_Int64_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.int64Arr[idx]);
            }
            break;
        case UA_UInt64_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.uint64Arr[idx]);
            }
            break;
        case UA_Float_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.floatvArr[idx]);
            }
            break;
        case UA_Double_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.doublevArr[idx]);
            }
            break;
        case UA_String_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.stringArr[idx]);
            }
            break;
        case UA_DateTime_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.dateArr[idx]);
            }
            break;
        case UA_Guid_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.guidArr[idx]);
            }
            break;
        case UA_ByteString_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.bstringArr[idx]);
            }
            break;
        case UA_XmlElement_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.xmlEltArr[idx]);
            }
            break;
        case UA_NodeId_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.nodeIdArr[idx]);
            }
            break;
        case UA_ExpandedNodeId_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.expNodeIdArr[idx]);
            }
            break;
        case UA_StatusCode_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.statusArr[idx]);
            }
            break;
        case UA_QualifiedName_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.qnameArr[idx]);
            }
            break;
        case UA_LocalizedText_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.localizedTextArr[idx]);
            }
            break;
        case UA_ExtensionObject_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.extObjectArr[idx]);
            }
            break;
        case UA_DataValue_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.dataValueArr[idx]);
            }
            break;
        case UA_Variant_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.variantArr[idx]);
            }
            break;
        case UA_DiagnosticInfo_Id:
            for(idx = 0; idx < length; idx++){
                builtInFunction(&array.diagInfoArr[idx]);
            }
            break;
        default:
            break;
    }
}

void Variant_Initialize(UA_Variant* variant){
    memset(variant, 0, sizeof(UA_Variant));
}

void* GetBuiltInTypeClearFunction(UA_BuiltinId builtInTypeId){
    void* clearFunction = NULL;
    switch(builtInTypeId){
            case UA_Boolean_Id:
                clearFunction = Boolean_Clear;
                break;
            case UA_SByte_Id:
                clearFunction = SByte_Clear;
                break;
            case UA_Byte_Id:
                clearFunction = Byte_Clear;
                break;
            case UA_Int16_Id:
                clearFunction = Int16_Clear;
                break;
            case UA_UInt16_Id:
                clearFunction = UInt16_Clear;
                break;
            case UA_Int32_Id:
                clearFunction = Int32_Clear;
                break;
            case UA_UInt32_Id:
                clearFunction = UInt32_Clear;
                break;
            case UA_Int64_Id:
                clearFunction = Int64_Clear;
                break;
            case UA_UInt64_Id:
                clearFunction = UInt64_Clear;
                break;
            case UA_Float_Id:
                clearFunction = Float_Clear;
                break;
            case UA_Double_Id:
                clearFunction = Double_Clear;
                break;
            case UA_String_Id:
                clearFunction = String_Clear;
                break;
            case UA_DateTime_Id:
                clearFunction = DateTime_Clear;
                break;
            case UA_Guid_Id:
                clearFunction = Guid_Clear;
                break;
            case UA_ByteString_Id:
                clearFunction = ByteString_Clear;
                break;
            case UA_XmlElement_Id:
                clearFunction = XmlElement_Clear;
                break;
            case UA_NodeId_Id:
                clearFunction = NodeId_Clear;
                break;
            case UA_ExpandedNodeId_Id:
                clearFunction = ExpandedNodeId_Clear;
                break;
            case UA_StatusCode_Id:
                clearFunction = StatusCode_Clear;
                break;
            case UA_QualifiedName_Id:
                clearFunction = QualifiedName_Clear;
                break;
            case UA_LocalizedText_Id:
                clearFunction = LocalizedText_Clear;
                break;
            case UA_ExtensionObject_Id:
                clearFunction = ExtensionObject_Clear;
                break;
            case UA_DataValue_Id:
                clearFunction = DataValue_Clear;
                break;
            case UA_Variant_Id:
                clearFunction = Variant_Clear;
                break;
            case UA_DiagnosticInfo_Id:
                clearFunction = DiagnosticInfo_Clear;
                break;
            default:
                break;
        }
    return clearFunction;
}

void Variant_Clear(UA_Variant* variant){
    void* clearFunction = GetBuiltInTypeClearFunction(variant->builtInTypeMask);
    // Matrix flag => array flag
    assert(((variant->arrayTypeMask & UA_VariantArrayMatrixFlag) != 0 &&
             (variant->arrayTypeMask & UA_VariantArrayValueFlag) != 0)
           || ((variant->arrayTypeMask & UA_VariantArrayMatrixFlag) == 0));

    if((variant->arrayTypeMask & UA_VariantArrayValueFlag) != 0){
        int32_t length = 0;
        if((variant->arrayTypeMask & UA_VariantArrayMatrixFlag) != 0){
            int32_t idx = 0;
            for(idx = 0; idx < variant->value.matrix.dimensions; idx ++){
                length *= variant->value.matrix.arrayDimensions[idx];
            }
            ApplyToVariantArrayBuiltInType(variant->builtInTypeMask,
                                           variant->value.matrix.content,
                                           length,
                                           clearFunction);
        }else{
            ApplyToVariantArrayBuiltInType(variant->builtInTypeMask,
                                           variant->value.array.content,
                                           variant->value.array.length,
                                           clearFunction);
        }
    }else{
        ApplyToVariantNonArrayBuiltInType(variant->builtInTypeMask,
                                          variant->value,
                                          clearFunction);
    }
}

void DataValue_Initialize(UA_DataValue* dataValue){
    memset(dataValue, 0, sizeof(UA_DataValue));
}
void DataValue_Clear(UA_DataValue* dataValue){
    Variant_Clear(&dataValue->value);
    StatusCode_Clear(&dataValue->status);
    dataValue->sourceTimestamp = 0;
    dataValue->serverTimestamp = 0;
    dataValue->sourcePicoSeconds = 0;
    dataValue->serverPicoSeconds = 0;
}

