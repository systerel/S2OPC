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
 * \brief OPC UA Safety timer interface definition.
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the interface of the OPC UA Safety timer module.
 *
 * Safety-Related: yes
 */


#ifndef INC_UASTIME_H

#define INC_UASTIME_H


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


/*-------------*/
/*  T Y P E S  */
/*-------------*/

/**
* Type definition for the UAM timestamp
*/
typedef UAS_UInt64  UASTIME_Timestamp_type;


/**
 * This structure contains the data of one timer instance
 */
typedef struct UASTIME_Instance_struct
{
  /** Watchdog time */
  UASTIME_Timestamp_type dwWatchdogTime;
  /** This variable contains the system time tick for base timer 1ms */
  UASTIME_Timestamp_type dwCurrentTime;
  /** This variable contains the time tick value of expiration */
  UASTIME_Timestamp_type dwExpiringTime;
  /** This flag contains the system alternating millennium (0/1) */
  UAS_UInt8 byCurrentMillennium;
  /** This flag contains the alternating millennium (0/1) of expiration */
  UAS_UInt8 byExpiringMillennium;
  /** This flag indicates if timer is running ( UAS_TRUE ) or not ( UAS_FALSE ) */
  UAS_Bool bRunning;
} UASTIME_Instance_type;

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/**
  * UAS timer module initialization
  */
extern void vUASTIME_Init
(
  UASTIME_Instance_type * const pzTimerInst,   /**< Pointer to the primary instance data   */
  UASTIME_Instance_type * const r_pzTimerInst, /**< Pointer to the redundant instance data */
  UASTIME_Timestamp_type const dwCurrentTime   /**< current time value */
);

/**
  * UAS timer instance start
  */
extern void vUASTIME_StartWatchdog
(
  UASTIME_Instance_type * const pzTimerInst,   /**< Pointer to the primary instance data   */
  UASTIME_Instance_type * const r_pzTimerInst, /**< Pointer to the redundant instance data */
  UASTIME_Timestamp_type const dwCurrentTime,  /**< current time value in microseconds*/
  UAS_UInt32 const dwWatchdogTime              /**< Watchdog time in microseconds */
);

/**
  * UAS timer instance restart
  */
extern void vUASTIME_StartErrorIntervalLimit
(
  UASTIME_Instance_type * const pzTimerInst,   /**< Pointer to the primary instance data   */
  UASTIME_Instance_type * const r_pzTimerInst, /**< Pointer to the redundant instance data */
  UASTIME_Timestamp_type const dwCurrentTime,  /**< current time value in microseconds*/
  UAS_UInt16 const wErrorIntervalLimit          /**< Watchdog time in microseconds */
);

/**
  * UAS timer instance stop
  */
extern void vUASTIME_Stop
(
  UASTIME_Instance_type * const pzTimerInst,   /**< Pointer to the primary instance data   */
  UASTIME_Instance_type * const r_pzTimerInst, /**< Pointer to the redundant instance data */
  UASTIME_Timestamp_type const dwCurrentTime   /**< current time value */
);

/**
  * UAS timer instance expired - checking for timeout
  */
extern UAS_Bool bUASTIME_Expired
(
  UASTIME_Instance_type * const pzTimerInst,   /**< Pointer to the primary instance data   */
  UASTIME_Instance_type * const r_pzTimerInst, /**< Pointer to the redundant instance data */
  UASTIME_Timestamp_type const dwCurrentTime   /**< current time value */
);


/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

/**
  * This variable is used by the FDTimer functions to store invocation errors
  * of the FDDrv functions. The FDL interface functions check this variable
  * and set the resulting error code. The value for no error is "0"
  */
extern UAS_UInt16  wUASTIME_ErrorCount;

/**
  * Redundant (inverse) error count of the FDTimer module
  * All redundant variables are defined im module FDRVars
  */
extern UAS_UInt16  r_wUASTIME_ErrorCount;


#endif /* INC_UASTIME_H */
