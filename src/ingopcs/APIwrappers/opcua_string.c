/* Copyright (c) 1996-2016, OPC Foundation. All rights reserved.

   The source code in this file is covered under a dual-license scenario:
     - RCL: for OPC Foundation members in good-standing
     - GPL V2: everybody else

   RCL license terms accompanied with this source code. See http://opcfoundation.org/License/RCL/1.00/

   GNU General Public License as published by the Free Software Foundation;
   version 2 of the License are accompanied with this source code. See http://opcfoundation.org/License/GPLv2

   This source code is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "opcua_string.h"

#include <assert.h>
#include <string.h>

#include "sopc_builtintypes.h"


OpcUa_String* OpcUa_String_FromCString( OpcUa_StringA   a_strCString )
{
    SOPC_String* string = SOPC_String_Create();
    if(SOPC_String_AttachFromCstring(string, a_strCString) == STATUS_OK){
        return (OpcUa_String*) string;
    }else{
        SOPC_String_Delete(string);
        return NULL;
    }
}

/*============================================================================
 * Initialize a string.
 *===========================================================================*/
OpcUa_StatusCode OpcUa_String_Initialize( /*  bi */ OpcUa_String* a_pString )
{
    SOPC_String_Initialize(a_pString);

    return STATUS_OK;
}

/*============================================================================
* Create a new OpcUa_String
*===========================================================================*/
OpcUa_StatusCode OpcUa_String_CreateNewString(   /*  in */ OpcUa_StringA            a_strSource,
                                                 /*  in */ OpcUa_UInt32             a_uLength,
                                                 /*  in */ OpcUa_UInt32             a_uBufferSize,
                                                 /*  in */ OpcUa_Boolean            a_bDoCopy,
                                                 /*  in */ OpcUa_Boolean            a_bFreeOnClear,
                                                 /* out */ OpcUa_String**           a_ppNewString)
{
    (void) a_strSource;
    (void) a_uLength;
    (void) a_uBufferSize;
    (void) a_bDoCopy;
    (void) a_bFreeOnClear;
    (void) a_ppNewString;
    return STATUS_NOK;
}


/*============================================================================
* 
*===========================================================================*/
OpcUa_StatusCode OpcUa_String_AttachToString(  /* in */ OpcUa_StringA              a_strSource,
                                               /* in */ OpcUa_UInt32               a_uLength,
                                               /* in */ OpcUa_UInt32               a_uBufferSize,
                                               /* in */ OpcUa_Boolean              a_bDoCopy,
                                               /* in */ OpcUa_Boolean              a_bFreeOnClear,
                                               /* bi */ OpcUa_String*              a_pNewString)
{
    (void) a_strSource;
    (void) a_uLength;
    (void) a_uBufferSize;
    (void) a_bDoCopy;
    (void) a_bFreeOnClear;
    (void) a_pNewString;
    return STATUS_NOK;
}


/*============================================================================
 * Free memory for string
 *===========================================================================*/
OpcUa_Void OpcUa_String_Delete(OpcUa_String** a_ppString)
{
    SOPC_String_Delete(*a_ppString);
    *a_ppString = NULL;
}


/*============================================================================
* 
*===========================================================================*/
OpcUa_Void OpcUa_String_Clear(OpcUa_String* a_pString)
{
    SOPC_String_Clear(a_pString);
}


/*============================================================================
* Get pointer to internal raw string.
*===========================================================================*/
#define OpcUa_String_GetRawString SOPC_String_GetCString

/*============================================================================
* Check if string is empty.
*===========================================================================*/
OpcUa_Boolean OpcUa_String_IsEmpty(const OpcUa_String* a_pString)
{
    return a_pString != NULL && a_pString->Length == 0 && a_pString->Data != NULL;
}


/*============================================================================
* Check if string is null.
*===========================================================================*/
OpcUa_Boolean OpcUa_String_IsNull(const OpcUa_String* a_pString)
{
    return a_pString == NULL || a_pString->Data == NULL;
}


/*============================================================================
* Get number of bytes.
*===========================================================================*/
//OpcUa_UInt32 OpcUa_String_StrSize(const OpcUa_String* a_pString)
//{
//    ???
//}


/*============================================================================
* Get number of characters.
*===========================================================================*/
OpcUa_UInt32 OpcUa_String_StrLen(const OpcUa_String*  a_pString)
{
    if(a_pString == NULL || a_pString->Data){
        return 0;
    }
    return strlen((char*)a_pString->Data);
}

/*============================================================================
* Copy string.
*===========================================================================*/
OpcUa_StatusCode OpcUa_String_StrnCpy( OpcUa_String*       a_pDestString,
                                       const OpcUa_String* a_pSrcString,
                                       OpcUa_UInt32        a_uLength) 
{
    if(a_pSrcString->Length != (int32_t) a_uLength){
        return STATUS_NOK;
    }else{
        return SOPC_String_Copy(a_pDestString, a_pSrcString);
    }
}

/*============================================================================
* Append string.
*===========================================================================*/
OpcUa_StatusCode OpcUa_String_StrnCat(  OpcUa_String*       a_pDestString,
                                        const OpcUa_String* a_pSrcString,
                                        OpcUa_UInt32        a_uLength)
{
    (void) a_pDestString;
    (void) a_pSrcString;
    (void) a_uLength;
    return STATUS_NOK;
}


/*============================================================================
 * Compare two OpcUa_Strings
 *===========================================================================*/
OpcUa_Int32 OpcUa_String_StrnCmp(   const OpcUa_String* a_pLeftString, 
                                    const OpcUa_String* a_pRightString,
                                    OpcUa_UInt32        a_uLength,
                                    OpcUa_Boolean       a_bIgnoreCase )
{
    int32_t res = -1;
    assert(a_bIgnoreCase == FALSE);
    if(STATUS_OK == SOPC_String_Compare(a_pLeftString, a_pRightString, &res)){
        assert((int32_t) a_uLength == a_pLeftString->Length || (int32_t) a_uLength == a_pRightString->Length);
    }else{
        assert(FALSE);
    }
    return res;
}



/*============================================================================
 * OpcUa_String_AttachReadOnly
 *===========================================================================*/
// TODO: read only ? not enforced
#define OpcUa_String_AttachReadOnly SOPC_String_AttachFromCstring

/*============================================================================
 * OpcUa_String_AttachCopy
 *===========================================================================*/
#define OpcUa_String_AttachCopy SOPC_String_CopyFromCString

/*============================================================================
 * OpcUa_String_AttachWithOwnership
 *===========================================================================*/
OpcUa_StatusCode OpcUa_String_AttachWithOwnership(OpcUa_String* a_pDst, OpcUa_StringA a_pSrc)
{
    OpcUa_StatusCode status;
    status = SOPC_String_AttachFromCstring(a_pDst, a_pSrc);
    a_pDst->ClearBytes = 1;
    return status;
}

