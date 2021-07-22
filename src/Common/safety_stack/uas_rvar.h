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
 * \brief OPC UA Safety redundant variable interface definition
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the interface of the OPC UA Safety redundant variables module.
 *
 * Safety-Related: yes
 */

#ifndef INC_UASRVAR_H

#define INC_UASRVAR_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_stdtypes.h"

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/**
 * Maximum value for the error count
 */
#define UASRVAR_MAX_ERROR_COUNT 0xFFFFu

/** @name redundant UAS_Bool
 * Processing of redundant variables of type UAS_Bool.
 */
/**@{*/

/** Set redundant variable of type UAS_Bool. */
#define UASRVAR_SET_BOOL(var, value) \
    (var) = (value);                 \
    (r_##var) = (UAS_Bool)(COMPL(var))

/** Valid redundant variables of type UAS_Bool. */
#define UASRVAR_VALID_BOOL(var) ((var) EQ(UAS_Bool)(COMPL(r_##var)))

/** Invalid redundant variables of type UAS_Bool. */
#define UASRVAR_INVALID_BOOL(var) ((var) NOT_EQ(UAS_Bool)(COMPL(r_##var)))

/**@}*/

/** @name redundant UAS_UInt8
 * Processing of redundant variables of type UAS_UInt8.
 */
/**@{*/

/** Set redundant variable of type UAS_UInt8. */
#define UASRVAR_SET_USIGN8(var, value) \
    (var) = (value);                   \
    (r_##var) = (UAS_UInt8)(COMPL(var))

/** Valid redundant variables of type UAS_UInt8. */
#define UASRVAR_VALID_USIGN8(var) ((var) EQ(UAS_UInt8)(COMPL(r_##var)))

/** Invalid redundant variables of type UAS_UInt8. */
#define UASRVAR_INVALID_USIGN8(var) ((var) NOT_EQ(UAS_UInt8)(COMPL(r_##var)))

/**@}*/

/** @name redundant UAS_UInt16
 * Processing of redundant variables of type UAS_UInt16.
 */
/**@{*/

/** Set redundant variable of type UAS_UInt16. */
#define UASRVAR_SET_USIGN16(var, value) \
    (var) = (value);                    \
    (r_##var) = (UAS_UInt16)(COMPL(var))

/** Valid redundant variables of type UAS_UInt16. */
#define UASRVAR_VALID_USIGN16(var) ((var) EQ(UAS_UInt16)(COMPL(r_##var)))

/** Invalid redundant variables of type UAS_UInt16. */
#define UASRVAR_INVALID_USIGN16(var) ((var) NOT_EQ(UAS_UInt16)(COMPL(r_##var)))

/**@}*/

/** @name redundant UAS_UInt32
 * Processing of redundant variables of type UAS_UInt32.
 */
/**@{*/

/** Set redundant variable of type UAS_UInt32. */
#define UASRVAR_SET_USIGN32(var, value) \
    (var) = (value);                    \
    (r_##var) = (UAS_UInt32)(COMPL(var))

/** Valid redundant variables of type UAS_UInt32. */
#define UASRVAR_VALID_USIGN32(var) ((var) EQ(UAS_UInt32)(COMPL(r_##var)))

/** Invalid redundant variables of type UAS_UInt32. */
#define UASRVAR_INVALID_USIGN32(var) ((var) NOT_EQ(UAS_UInt32)(COMPL(r_##var)))

/**@}*/

/** @name redundant UAS_UInt64
 * Processing of redundant variables of type UAS_UInt32.
 */
/**@{*/

/** Set redundant variable of type UAS_UInt64. */
#define UASRVAR_SET_USIGN64(var, value) \
    (var) = (value);                    \
    (r_##var) = (UAS_UInt64)(COMPL(var))

/** Valid redundant variables of type UAS_UInt32. */
#define UASRVAR_VALID_USIGN64(var) ((var) EQ(UAS_UInt64)(COMPL(r_##var)))

/** Invalid redundant variables of type UAS_UInt64. */
#define UASRVAR_INVALID_USIGN64(var) ((var) NOT_EQ(UAS_UInt64)(COMPL(r_##var)))

/**@}*/

/** @name redundant Pointer
 * Processing of redundant variables of type Pointer.
 * Supress warning "cast from pointer to unsigned long [Encompasses MISRA 2004 Rule 11.1,
 * required], [MISRA 2004 Rule 11.3, advisory]" because this cast is necessary to
 * calculate the inverse value of the pointer to be stored in redundant variable.
 */
/**@{*/

/** Set redundant variable of type Pointer. */
#define UASRVAR_SET_POINTER(var, value) \
    (var) = (value);                    \
    (r_##var) = /*lint -e(923)*/ /*lint -e(960)*/ (void*) (COMPL((UAS_INVERSE_PTR)(var)))

/** Valid redundant variables of type Pointer. */
#define UASRVAR_VALID_POINTER(var) \
    ((var) EQ /*lint -e(923)*/ /*lint -e(960)*/ (void*)(COMPL((UAS_INVERSE_PTR)(r_##var))))

/** Invalid redundant variables of type Pointer. */
#define UASRVAR_INVALID_POINTER(var) \
    ((var) NOT_EQ /*lint -e(923)*/ /*lint -e(960)*/ (void*)(COMPL((UAS_INVERSE_PTR)(r_##var))))

/**@}*/

/** @name redundant SafetyProvider state
 * Processing of redundant variables of type UASPROV_State_type.
 * Supress warning "Violates MISRA 2004 Required Rule 12.7, Bitwise operator applied
 * to signed underlying type: ~" because this cast is necessary to calculate the
 * inverse value of the enumeration to be stored in redundant variable
 */
/**@{*/

/** Set redundant variable of type UASPROV_State_type. */
#define UASRVAR_SET_PROVSTATE(var, value) \
    (var) = (value);                      \
    (r_##var) = /*lint -e(960)*/ (UASPROV_State_type)(COMPL(var))

/** Valid redundant variables of type UASPROV_State_type. */
#define UASRVAR_VALID_PROVSTATE(var) ((var) EQ /*lint -e(960)*/ (UASPROV_State_type)(COMPL(r_##var)))

/** Invalid redundant variables of type UASPROV_State_type. */
#define UASRVAR_INVALID_PROVSTATE(var) ((var) NOT_EQ /*lint -e(960)*/ (UASPROV_State_type)(COMPL(r_##var)))

/**@}*/

/** @name redundant SafetyConsumer state
 * Processing of redundant variables of type UASCONS_State_type.
 * Supress warning "Violates MISRA 2004 Required Rule 12.7, Bitwise operator applied
 * to signed underlying type: ~" because this cast is necessary to calculate the
 * inverse value of the enumeration to be stored in redundant variable
 */
/**@{*/

/** Set redundant variable of type UASCONS_State_type. */
#define UASRVAR_SET_CONSSTATE(var, value) \
    (var) = (value);                      \
    (r_##var) = /*lint -e(960)*/ (UASCONS_State_type)(COMPL(var))

/** Valid redundant variables of type UASCONS_State_type. */
#define UASRVAR_VALID_CONSSTATE(var) ((var) EQ /*lint -e(960)*/ (UASCONS_State_type)(COMPL(r_##var)))

/** Invalid redundant variables of type UASCONS_State_type. */
#define UASRVAR_INVALID_CONSSTATE(var) ((var) NOT_EQ /*lint -e(960)*/ (UASCONS_State_type)(COMPL(r_##var)))

/**@}*/

/** @name redundant error count
 * Processing of redundant error count.
 */
/**@{*/

/** Set redundant error count. */
#define UASRVAR_INCR_ERROR_COUNT(err) \
    (err)++;                          \
    (r_##err)--

/** Valid redundant error count. */
#define UASRVAR_VALID_ERROR_COUNT(err) ((0u EQ(err)) AND((err) EQ(UASRVAR_MAX_ERROR_COUNT - (r_##err))))

/** Invalid redundant error count. */
#define UASRVAR_INVALID_ERROR_COUNT(err) ((0u < (err)) OR((err) NOT_EQ(UASRVAR_MAX_ERROR_COUNT - (r_##err))))

/**@}*/

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/**
 * Initialization of the redundant data
 */
void vUASRVAR_Init(void);

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

#endif /* INC_UASRVAR_H */
