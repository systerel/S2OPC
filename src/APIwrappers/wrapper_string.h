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

#ifndef SOPC_WRAPPER_STRING_H_
#define SOPC_WRAPPER_STRING_H_

#include "sopc_builtintypes.h"

BEGIN_EXTERN_C

SOPC_String* OpcUa_String_FromCString(char* cString);
SOPC_StatusCode OpcUa_String_Initialize(SOPC_String* str);
SOPC_StatusCode OpcUa_String_CreateNewString(char*         srcStr,
                                             uint32_t      length,
                                             uint32_t      bufferSize,
                                             uint8_t       doCopy,
                                             uint8_t       freeOnClear,
                                             SOPC_String** newString);

SOPC_StatusCode OpcUa_String_AttachToString(char*        srcStr,
                                            uint32_t     length,
                                            uint32_t     bufferSize,
                                            uint8_t      doCopy,
                                            uint8_t      freeOnClear,
                                            SOPC_String* newString);

void OpcUa_String_Delete(SOPC_String** str);
void OpcUa_String_Clear(SOPC_String* str);
char* OpcUa_String_GetRawString(const SOPC_String* str);
uint8_t OpcUa_String_IsEmpty(const SOPC_String* str);
uint8_t OpcUa_String_IsNull(const SOPC_String* str);
SOPC_StatusCode OpcUa_String_StrnCpy(SOPC_String*       dest,
                                     const SOPC_String* src,
                                     uint32_t           length);
SOPC_StatusCode OpcUa_String_StrnCat(SOPC_String*       dest,
                                     const SOPC_String* src,
                                     uint32_t           length);
uint32_t OpcUa_String_StrLen(const SOPC_String* str);
uint32_t OpcUa_String_StrSize(const SOPC_String* str);
int32_t OpcUa_String_StrnCmp(const SOPC_String* str1,
                             const SOPC_String* str2,
                             uint32_t           length,
                             uint8_t            ignoreCase);

// TODO: read only ? not enforced
SOPC_StatusCode OpcUa_String_AttachReadOnly(SOPC_String* dest,
                                            char*        src); // Must be const
SOPC_StatusCode OpcUa_String_AttachCopy(SOPC_String* dest,
                                        char*        src);
SOPC_StatusCode OpcUa_String_AttachWithOwnership(SOPC_String* dest,
                                                 char*        src);

END_EXTERN_C

#endif /* SOPC_WRAPPER_STRING_H_ */
