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

#include "wrapper_string.h"

#include <assert.h>
#include <string.h>
#include <limits.h>

#include "sopc_base_types.h"
#include "sopc_helper_string.h"
#include "sopc_builtintypes.h"

#define NOLENGTH UINT32_MAX

SOPC_String* OpcUa_String_FromCString(char* cString)
{
    SOPC_String* string = SOPC_String_Create();
    if(SOPC_String_AttachFromCstring(string, cString) == STATUS_OK){
        return (SOPC_String*) string;
    }else{
        SOPC_String_Delete(string);
        return NULL;
    }
}

SOPC_StatusCode OpcUa_String_Initialize(SOPC_String* str)
{
    SOPC_String_Initialize(str);
    return STATUS_OK;
}

SOPC_StatusCode OpcUa_String_CreateNewString(char*         srcStr,
                                             uint32_t      length,
                                             uint32_t      bufferSize,
                                             uint8_t       doCopy,
                                             uint8_t       freeOnClear,
                                             SOPC_String** newString)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(doCopy == FALSE){
        // Buffer size is not configurable: it should be ignored
        bufferSize = 0;
    }
    assert(bufferSize == 0 || bufferSize == length); // Do not accept buffer size != length
    if(srcStr != NULL &&
       (NOLENGTH == length || strlen(srcStr) == length) && // Do not accept strlen != length
       newString != NULL)
    {
        *newString = SOPC_String_Create();
        if(*newString == NULL){
            return STATUS_NOK;
        }
        OpcUa_String_AttachToString(srcStr,
                                    length,
                                    bufferSize,
                                    doCopy,
                                    freeOnClear,
                                    *newString);
    }
    return status;
}

SOPC_StatusCode OpcUa_String_AttachToString(char*        srcStr,
                                            uint32_t     length,
                                            uint32_t     bufferSize,
                                            uint8_t      doCopy,
                                            uint8_t      freeOnClear,
                                            SOPC_String* newString)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(doCopy == FALSE){
        // Buffer size is not configurable: it should be ignored
        bufferSize = 0;
    }
    assert(bufferSize == 0 || bufferSize == length); // Do not accept buffer size != length
    if(srcStr != NULL &&
       (NOLENGTH == length || strlen(srcStr) == length) && // Do not accept strlen != length
       newString != NULL && newString->Data == NULL) // Do not accept non empty string
    {
        if(doCopy == FALSE){
            status = SOPC_String_AttachFromCstring(newString, srcStr);
        }else{
            status = SOPC_String_CopyFromCString(newString, srcStr);
        }
        newString->DoNotClear = (freeOnClear == FALSE);
    }
    return status;
}

void OpcUa_String_Delete(SOPC_String** str)
{
    SOPC_String_Delete(*str);
    *str = NULL;
}

void OpcUa_String_Clear(SOPC_String* str)
{
    SOPC_String_Clear(str);
}

char* OpcUa_String_GetRawString(const SOPC_String* str){
    return (char*) SOPC_String_GetRawCString(str);
}

uint8_t OpcUa_String_IsEmpty(const SOPC_String* str)
{
    return str != NULL && str->Length == 0 && str->Data != NULL;
}

uint8_t OpcUa_String_IsNull(const SOPC_String* str)
{
    return str == NULL || str->Data == NULL;
}

uint32_t OpcUa_String_StrLen(const SOPC_String*  str)
{
    assert(CHAR_BIT == 8);
    if(str == NULL || str->Data == NULL){
        return 0;
    }
    // uint32_t conversion ensured by SOPC_String construction (max length = INT32_MAX)
    return (uint32_t) strlen((char*)str->Data);
}

uint32_t OpcUa_String_StrSize(const SOPC_String*  str)
{
    return OpcUa_String_StrLen(str);
}

SOPC_StatusCode OpcUa_String_StrnCpy(SOPC_String*       dest,
                                     const SOPC_String* src,
                                     uint32_t           length)
{
    if(NOLENGTH != length && src->Length != (int32_t) length){
        return STATUS_NOK;
    }else{
        return SOPC_String_Copy(dest, src);
    }
}

SOPC_StatusCode OpcUa_String_StrnCat(SOPC_String*       dest,
                                     const SOPC_String* src,
                                     uint32_t           length)
{
    SOPC_StatusCode status = STATUS_OK;
    uint32_t len = 0;
    char* concat = NULL;
    if(dest->Length > 0 && src->Length > 0){
        len = dest->Length + src->Length;
    }else if(src->Length > 0){
        len = src->Length;
    }else if(dest->Length > 0){
        len = dest->Length;
    }else{
        status = STATUS_INVALID_PARAMETERS;
    }
    if(STATUS_OK == status){
        assert(CHAR_BIT == 8);
        assert(length == NOLENGTH || length == len); // Do not accept length != concat length
        concat = malloc(sizeof(char)*(len+1));
        if(concat == NULL){
            status = STATUS_NOK;
        }
        if(STATUS_OK == status &&
           dest->Length > 0){
            if(concat != memcpy(concat, dest->Data, dest->Length)){
                status = STATUS_NOK;
            }
        }

        if(STATUS_OK == status &&
           src->Length > 0){
            if((concat+len-src->Length) != memcpy((concat+len-src->Length), src->Data, src->Length)){
                return STATUS_NOK;
            }
        }

        if(STATUS_OK == status){
            concat[len] = '\0';
        }

        if(STATUS_OK == status){
            SOPC_String_Clear(dest); // Clear destination
            status = SOPC_String_InitializeFromCString(dest, concat);
        }

        if(concat != NULL){
            free(concat);
        }
    }
    return status;
}

int32_t OpcUa_String_StrnCmp(const SOPC_String* str1,
                             const SOPC_String* str2,
                             uint32_t           length,
                             uint8_t            ignoreCase)
{
    int32_t res = -1;
    assert(CHAR_BIT == 8);
    if(str1->Length == str2->Length ||
       (str1->Length >= (int64_t) length && str2->Length >= (int64_t) length))
    {
        if((str1->Length <= 0 && str2->Length <= 0) || length == 0){
            res = 0;
        }else if(ignoreCase == FALSE){
            res = strncmp((char*) str1->Data, (char*) str2->Data, length);
        }else{
            res = SOPC_strncmp_ignore_case((char*) str1->Data, (char*) str2->Data, length);
        }
    }else if(str1->Length > str2->Length){
         res = +1;
    }else{
        res = -1;
    }
    return res;
}

// TODO: read only ? not enforced
SOPC_StatusCode OpcUa_String_AttachReadOnly(SOPC_String* dest,
                                            char*        src){
    return SOPC_String_AttachFromCstring(dest, src);
}

SOPC_StatusCode OpcUa_String_AttachCopy(SOPC_String* dest,
                                        char*        src){
    SOPC_StatusCode status = STATUS_NOK;
    status = SOPC_String_CopyFromCString(dest, src);
    return status;
}

SOPC_StatusCode OpcUa_String_AttachWithOwnership(SOPC_String* dest,
                                                 char*        src)
{
    SOPC_StatusCode status;
    status = SOPC_String_AttachFromCstring(dest, src);
    dest->DoNotClear = FALSE;
    return status;
}
