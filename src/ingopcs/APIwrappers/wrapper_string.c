#include "wrapper_string.h"

#include <assert.h>
#include <string.h>
#include <ctype.h>

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
        newString->ClearBytes = freeOnClear;
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
    if(str == NULL || str->Data == NULL){
        return 0;
    }
    return strlen((char*)str->Data);
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
    uint32_t len = 0;
    if(dest->Length > 0 && src->Length > 0){
        len = dest->Length + src->Length;
    }else if(src->Length > 0){
        len = src->Length;
    }else if(dest->Length > 0){
        len = dest->Length;
    }else{
        return STATUS_INVALID_PARAMETERS;
    }
    assert(length == NOLENGTH || length == len); // Do not accept length != concat length
    char concat[len+1];
    if(dest->Length > 0){
        if(concat != memcpy(concat, dest->Data, dest->Length))
            return STATUS_NOK;
    }
    if(src->Length > 0){
        if((concat+len-src->Length) != memcpy((concat+len-src->Length), src->Data, src->Length))
            return STATUS_NOK;
    }
    concat[len] = '\0';

    SOPC_String_Clear(dest); // Clear destination
    return SOPC_String_InitializeFromCString(dest, concat);
}

int32_t OpcUa_String_StrnCmp(const SOPC_String* str1,
                             const SOPC_String* str2,
                             uint32_t           length,
                             uint8_t            ignoreCase)
{
    int32_t res = -1;
    int32_t idx = 0;
    int lc1, lc2;
    if(ignoreCase == FALSE){
        if(STATUS_OK != SOPC_String_Compare(str1, str2, &res)){
            assert(FALSE);
        }
    }else{
        if(str1->Length == str2->Length ||
           (str1->Length >= (int32_t) length && str2->Length >= (int32_t) length))
        {
            if((str1->Length <= 0 && str2->Length <= 0) || length == 0){
                res = 0;
            }else if(ignoreCase == FALSE){
                res = strncmp((char*) str1->Data, (char*) str2->Data, length);
            }else{
                res = 0;
                for(idx = 0; idx < str1->Length && idx < str2->Length && res == 0; idx ++){
                    lc1 = tolower(str1->Data[idx]);
                    lc2 = tolower(str2->Data[idx]);
                    if(lc1 < lc2){
                        res = -1;
                    }else if(lc1 > lc2){
                        res = +1;
                    }
                }
            }
        }else if(str1->Length > str2->Length){
             res = +1;
        }else{
            res = -1;
        }
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
    dest->ClearBytes = 1;
    return status;
}
