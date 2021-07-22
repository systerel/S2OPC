/**
 * OPC Foundation OPC UA Safety Stack
 *
 * Copyright (c) 2021 OPC Foundation. All rights reserved.
 * This Software is licensed under OPC Foundation's proprietary Enhanced
 * Commercial Software License Agreement [LINK], and may only be used by
 * authorized Licensees in accordance with the terms of such license.
 * THE SOFTWARE IS LICENSED "AS-IS" WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.
 * This notice must be included in all copies or substantial portions of the Software.
 *
 * \file
 *
 * \brief Basic data type definitions
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines basic data types for the OPC UA Safety protocol.
 *
 * Safety-Related: no
 */

#ifndef INC_UASSTDTYPES_H

#define INC_UASSTDTYPES_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include <stdint.h>

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/** @name Boolean values
 *  These macros define the values of the boolean data type.
 */
/**@{*/
#define UAS_FALSE 0x00u /**< Value 'false' of a boolean data type. */
#define UAS_TRUE 0x01u  /**< Value 'true' of a boolean data type.  */
/**@}*/

/**
 * Value of an uninitialized pointer (NULL).
 */
#ifdef NULL
#define UAS_NULL NULL
#else
#define UAS_NULL ((void*) 0)
#endif

/**
 * Value of an unspecific pointer (VOID).
 */
#ifdef VOID
#define UAS_VOID VOID
#else
#define UAS_VOID void
#endif

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/** @name Basic data types
 *  These type definitions define the basic data types used in the UAS.
 */
/**@{*/
typedef unsigned char UAS_Bool; /**< A two-state logical value (Bool). */
typedef char UAS_Char;          /**< An character value (Char). */
typedef int8_t UAS_Int8;        /**< An integer value between -128 and 127 (Int8). */
typedef int16_t UAS_Int16;      /**< An integer value between -32768 and 32767 (Int16). */
typedef int32_t UAS_Int32;      /**< An integer value between -2147483648 and 2147483647 (Int32). */
typedef int64_t UAS_Int64;      /**< An integer value between -9223372036854775808 and 9223372036854775807 (Int64). */
typedef uint8_t UAS_UInt8;      /**< An integer value between 0 and 255 (UInt8).  */
typedef uint16_t UAS_UInt16;    /**< An integer value between 0 and 65535 (UInt16).  */
typedef uint32_t UAS_UInt32;    /**< An integer value between 0 and 4294967295 (UInt32). */
typedef uint64_t UAS_UInt64;    /**< An integer value between 0 and 18446744073709551615 (UInt64). */

/** size of a pointer, to be adapted ( UAS_Int16, UAS_Int32, UAS_Int64 )
 * only used for the redundant (inverse) storage of pointer variables
 */
// SYSTEREL REVIEW : uintptr_t or intptr_t type ensures the compatibility with pointers.
#define UAS_INVERSE_PTR uintptr_t

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

// SYSTEREL REVIEW :Add undefined symbols.. TBC
//#define UInt8 UAS_UInt8
//#define UInt16 UAS_UInt16
//#define UInt32 UAS_UInt32
//#define Int8 UAS_Int8
//#define Int16 UAS_Int16
//#define Int32 UAS_Int32
#define LARGE_INTEGER uint64_t
#define STDFUNC_LARGE_INTEGER LARGE_INTEGER
#define STDFUNC_strcpy strcpy
#define STDFUNC_strcat strcat
#define STDFUNC_strncat strncat
#define STDFUNC_snprintf snprintf
#define STDFUNC_vsnprintf vsnprintf
#define STDFUNC_strtoul strtoul
#define STDFUNC_strtol strtol
#define STDFUNC_strcmp strcmp
#define STDFUNC_memset memset
#define STDFUNC_strlen strlen

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

#endif /* ifndef INC_UASSTDTYPES_H */
