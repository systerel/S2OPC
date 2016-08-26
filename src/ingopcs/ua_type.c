/*
 * opcua_ingopcs_type.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "ua_types.h"

UA_ByteString* ByteString_Create(){
    UA_ByteString* bstring = UA_NULL;
    bstring = (UA_ByteString*) malloc(sizeof(UA_ByteString));
    if(bstring != UA_NULL){
        bstring->length = -1;
        bstring->characters = UA_NULL;
    }
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

void ByteString_Delete(UA_ByteString* bstring){
    if(bstring != UA_NULL){
        if(bstring->characters != UA_NULL){
            free(bstring->characters);
        }
        free(bstring);
    }
}

UA_String* String_Create(){
    return (UA_String*) ByteString_Create();
}
UA_String* String_Copy(UA_String* src){
    return (UA_String*) ByteString_Copy((UA_ByteString*) src);
}
void String_Delete(UA_String* bstring){
    ByteString_Delete((UA_ByteString*) bstring);
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
                String_Delete(string);
            }
        }else{
            String_Delete(string);
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
