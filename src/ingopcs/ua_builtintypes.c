/*
 * opcua_ingopcs_type.c
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
    if(bstring != UA_NULL){
        assert(bstring->characters == UA_NULL);
        bstring->length = -1;
        bstring->characters = UA_NULL;
    }
}

UA_ByteString* ByteString_Create(){
    UA_ByteString* bstring = UA_NULL;
    bstring = (UA_ByteString*) malloc(sizeof(UA_ByteString));
    ByteString_Initialize(bstring);
    return bstring;
}

UA_ByteString* ByteString_CreateFixedSize(uint32_t size){
    UA_ByteString* bstring = UA_NULL;
    bstring = (UA_ByteString*) malloc(sizeof(UA_ByteString));
    if(bstring != UA_NULL){
        bstring->length = size;
        bstring->characters = (UA_Byte*) malloc (sizeof(UA_Byte)*size);
        if(bstring->characters != UA_NULL){
            memset(bstring->characters, 0, size);
        }else{
            free(bstring);
            bstring = UA_NULL;
        }
    }
    return bstring;
}

UA_ByteString* ByteString_Copy(UA_ByteString* src){
    UA_ByteString* dest = UA_NULL;
    if(src != UA_NULL){
        dest = ByteString_Create();
        if(dest != UA_NULL){
            if(src->length > 0){
                dest->length = src->length;
                dest->characters = (UA_Byte*) malloc (sizeof(UA_Byte)*dest->length);
                // No need of secure copy, both have same size here
                memcpy(dest->characters, src->characters, dest->length);
            } // else only -1 value is a correct value as created
        }
    }
    return dest;
}

void ByteString_Clear(UA_ByteString* bstring){
    if(bstring != UA_NULL){
        if(bstring->characters != UA_NULL){
            free(bstring->characters);
        }
        free(bstring);
    }
}

void String_Initialize(UA_String* string){
    ByteString_Initialize((UA_ByteString*) string);
}

UA_String* String_Create(){
    return (UA_String*) ByteString_Create();
}
UA_String* String_Copy(UA_String* src){
    return (UA_String*) ByteString_Copy((UA_ByteString*) src);
}
void String_Clear(UA_String* string){
    ByteString_Clear((UA_ByteString*) string);
}

UA_String* String_CreateFromCString(char* cString){
    UA_String* string = UA_NULL;
    size_t stringLength = 0;
    int32_t idx = 0;
    if(cString != UA_NULL){
        string = String_Create();
        stringLength = strlen(cString);
        if(string != UA_NULL &&
           stringLength > 0 &&
           stringLength <= INT32_MAX)
        {
            string->length = stringLength;
            string->characters = (UA_Byte*) malloc(sizeof(UA_Byte)*stringLength);
            if(string->characters != UA_NULL){
                if(CHAR_BIT == 8){
                    memcpy(string->characters, cString, stringLength);
                }else{
                    // On systems for which char is not encoded on 1 byte
                    for(idx = 0; idx < stringLength; idx++){
                        string->characters[idx] = (uint8_t) cString[idx];
                    }
                }
            }else{
                String_Clear(string);
            }
        }else{
            String_Clear(string);
        }
    }
    return string;
}


char* String_GetCString(UA_String* string){
    char* cString = UA_NULL;
    int32_t idx = 0;
    if(string != UA_NULL &&
       string->length > 0)
    {
        cString = (char*) malloc(sizeof(char)* (string->length + 1));
        if(cString != UA_NULL){
            if(CHAR_BIT == 8){
                memcpy(cString, string->characters, string->length);
            }else{
                // On systems for which char is not encoded on 1 byte
                for(idx = 0; idx < string->length; idx++){
                    cString[idx] = (char) string->characters[idx];
                }
            }
            cString[string->length] = '\0';
        }
    }
    return cString;
}

StatusCode ByteString_Compare(UA_ByteString* left,
                              UA_ByteString* right,
                              uint32_t*      comparison)
{
    StatusCode status = STATUS_INVALID_PARAMETERS;

    if(left != UA_NULL && right != UA_NULL){
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

uint32_t ByteString_Equal(UA_ByteString* left,
                          UA_ByteString* right)
{
    uint32_t compare = 0;
    uint32_t result = UA_FALSE;

    if(ByteString_Compare(left, right, &compare) == STATUS_OK){
        result = compare == 0;
    }

    return result;
}

StatusCode String_Compare(UA_String* left,
                          UA_String* right,
                          uint32_t*  comparison)
{

    return ByteString_Compare((UA_ByteString*) left,
                              (UA_ByteString*) right, comparison);
}

uint32_t String_Equal(UA_String* left,
                      UA_String* right)
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
    nodeId->identifierType = IdentifierType_Undefined;
}

void NodeId_InitType(UA_NodeId* nodeId, UA_IdentifierType knownIdType){
    assert(nodeId->identifierType == IdentifierType_Undefined);
    nodeId->namespace = 0; // OPCUA namespace
    nodeId->identifierType = knownIdType;
    switch(knownIdType){
        case IdentifierType_Undefined:
            assert(UA_FALSE);
            break;
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
        case IdentifierType_Undefined:
            break;
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
    nodeId->identifierType = IdentifierType_Undefined;
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
    diagInfo->symbolicId = -1;
    diagInfo->namespaceUri = -1;
    diagInfo->locale = -1;
    diagInfo->localizedText = -1;
    String_Initialize(&diagInfo->additionalInfo);
    diagInfo->innerStatusCode = STATUS_OK;
    diagInfo->innerDiagnosticInfo = UA_NULL;
}

void DiagnosticInfo_Clear(UA_DiagnosticInfo* diagInfo){
    String_Clear(&diagInfo->additionalInfo);
    if(diagInfo->innerDiagnosticInfo != UA_NULL){
        DiagnosticInfo_Clear(diagInfo->innerDiagnosticInfo);
        free(diagInfo->innerDiagnosticInfo);
    }
    diagInfo->symbolicId = -1;
    diagInfo->namespaceUri = -1;
    diagInfo->locale = -1;
    diagInfo->localizedText = -1;
    diagInfo->innerStatusCode = STATUS_OK;
    diagInfo->innerDiagnosticInfo = UA_NULL;
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
            // TODO: EncodeableType !!!!
            //extObj->body.object.objType->Clear(extObj->body.object.value);
            break;
    }
    extObj->length = 0;
}

void Variant_Initialize(UA_Variant* variant){
    memset(variant, 0, sizeof(UA_Variant));
}

void Variant_Clear(UA_Variant* variant){
    //check (variant->arrayTypeMask){

    switch(variant->builtInTypeMask){
        case UA_Boolean_Id:

        case UA_SByte_Id:
        case UA_Byte_Id:
        case UA_Int16_Id:
        case UA_UInt16_Id:
        case UA_Int32_Id:
        case UA_UInt32_Id:
        case UA_Int64_Id:
        case UA_UInt64_Id:
        case UA_Float_Id:
        case UA_Double_Id:
        case UA_String_Id:
        case UA_DateTime_Id:
        case UA_Guid_Id:
        case UA_ByteString_Id:
        case UA_XmlElement_Id:
        case UA_NodeId_Id:
        case UA_ExpandedNodeId_Id:
        case UA_StatusCode_Id:
        case UA_QualifiedName_Id:
        case UA_LocalizedText_Id:
        case UA_ExtensionObject_Id:
        case UA_DataValue_Id:
        case UA_Variant_Id:
        case UA_DiagnosticInfo_Id:
            break;
    }
}

