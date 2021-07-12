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
 * \brief OPC UA SafetyConsumer interface definition
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the interface of the OPC UA SafetyConsumer.
 *
 * Safety-Related: yes
 */


#ifndef INC_UASCONS_H

#define INC_UASCONS_H


/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_stdtypes.h"
#include "uas_type.h"
#include "uas.h"
#include "uas_time.h"


/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/** @name Error codes of the UAS SafetyConsumer
 *  These macros define the return values of the functions.
 */
/**@{*/
#define UASCONS_OK            ( UASRVAL_NO_ERROR ) /**< Function result OK                 */
#define UASCONS_STATE_ERR     ( UASRVAL_UASCONS_ID + UASRVAL_STATE_ERR     ) /**< Function not allowed in that state */
#define UASCONS_MEMORY_ERR    ( UASRVAL_UASCONS_ID + UASRVAL_MEMORY_ERR    ) /**< Not enough memory available        */
#define UASCONS_POINTER_ERR   ( UASRVAL_UASCONS_ID + UASRVAL_POINTER_ERR   ) /**< Pointer is NULL                    */
#define UASCONS_FCT_PARAM_ERR ( UASRVAL_UASCONS_ID + UASRVAL_FCT_PARAM_ERR ) /**< Parameter is faulty                */
#define UASCONS_DEVICE_ERR    ( UASRVAL_UASCONS_ID + UASRVAL_DEVICE_ERR    ) /**< Device error                       */
#define UASCONS_NOT_IMPL      ( UASRVAL_UASCONS_ID + UASRVAL_NOT_IMPL      ) /**< Function not implemented yet       */
#define UASCONS_NO_DATA       ( UASRVAL_UASCONS_ID + UASRVAL_NO_DATA       ) /**< No data available                  */
#define UASCONS_COMM_ERR      ( UASRVAL_UASCONS_ID + UASRVAL_COMM_ERR      ) /**< Communication error                */
#define UASCONS_TIMER_ERR     ( UASRVAL_UASCONS_ID + UASRVAL_TIMER_ERR     ) /**< Timer error                        */
#define UASCONS_SOFT_ERR      ( UASRVAL_UASCONS_ID + UASRVAL_SOFT_ERR      ) /**< Soft error                         */
#define UASCONS_SYNC_ERR      ( UASRVAL_UASCONS_ID + UASRVAL_SYNC_ERR      ) /**< Synchronization error              */
#define UASCONS_DEFAULT_ERR   ( UASRVAL_UASCONS_ID + UASRVAL_DEFAULT_ERR   ) /**< Function didn't set return value   */
/**@}*/


/*-------------*/
/*  T Y P E S  */
/*-------------*/

/**
  * States enumeration of the SafetyProvider state machine
  */
typedef enum UASCONS_State_enum
{
  UASCONS_S10_INITIALIZE            = 10,
  UASCONS_S11_WAIT_FOR_RESTART      = 11,
  UASCONS_S12_INITIALIZE_MNR        = 12,
  UASCONS_S13_PREPARE_REQUEST       = 13,
  UASCONS_S14_WAIT_FOR_CHANGED_SPDU = 14,
  UASCONS_S15_CRC_CHECK_SPDU        = 15,
  UASCONS_S16_CHECK_RESPONSE_SPDU   = 16,
  UASCONS_S17_ERROR                 = 17,
  UASCONS_S18_PROVIDE_SAFETY_DATA   = 18,

} UASCONS_State_type;

/**
  * This structure contains the internal data of an SafetyConsumer instance
  */
typedef struct UASCONS_StateMachine_struct
{
  /* Contains the FCS over the static instance parameter */
  UAS_UInt32  dwParamFcs;

  /** State of the SafetyConsumer state machine according to the OPC UA Safety specification */
  UASCONS_State_type nState;

  /** Local memory for errors which request operator acknowledgment. */
  UAS_Bool bFaultReqOA_i;

  /** Auxiliary flag indicating that operator acknowledgment is allowed.
    * It is true, if the input SAPI.OperatorAckConsumer has been 'false' since FaultReqOA_i was set. */
  UAS_Bool bOperatorAckConsumerAllowed_i;

  /** Local Monitoring Number (MNR). */
  UAS_UInt32 dwMNR_i;

  /** Local memory for previous MNR. */
  UAS_UInt32 dwPrevMNR_i;

  /** Local memory for SafetyConsumerID in use. */
  UAS_UInt32 dwConsumerID_i;

  /** Local variable used to store the result of the CRC-check. */
  UAS_Bool bCRCCheck_i;

  /** Local variable used to store the result of the CRC-check. */
  UAS_Bool bSPDUCheck_i;

  /** Local variable to store the expected SPDU_ID. */
  UAS_SPDUID_type zSPDUID_i;

  /** Local memory for previous ActivateFSV */
  UAS_Bool bPrevActivateFSV_i;

  /** Timer instance data for ConsumerTimer */
  UASTIME_Instance_type zConsumerTimer;

  /** Indicates if the ConsumerTimer has been expired */
  UAS_Bool bConsumerTimerExpired;

  /** Timer instance data  for ErrorIntervalTimer */
  UASTIME_Instance_type zErrorIntervalTimer;

  /** Indicates if the ErrorIntervalTimer has been expired */
  UAS_Bool bErrorIntervalTimerExpired;

  /** Pointer to the external instance data */
  UAS_SafetyConsumer_type *pzInstanceData;

} UASCONS_StateMachine_type;


/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/**
  * Initialization of a SafetyConsumer state machine instance
  */
UAS_UInt8 byUASCONS_Init
(
  UASCONS_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine,  /**< Pointer to the redundant state machine data */
  UAS_SafetyConsumer_type   * const pzInstance,        /**< Pointer to the instance data                */
  UAS_ParameterError_type   * const pnResult           /**< Pointer to an output variable for the result of the parameter check */
);

/**
  * De-initialization of a SafetyConsumer state machine instance
  */
UAS_UInt8 byUASCONS_Reset
(
  UASCONS_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine   /**< Pointer to the redundant state machine data */
);

/**
  * Re-Parameterization of a SafetyConsumer state machine instance
  */
UAS_UInt8 byUASCONS_ChangeSPI
(
  UASCONS_StateMachine_type  * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type  * const r_pzStateMachine,  /**< Pointer to the redundant state machine data */
  UAS_SafetyConsumerSPI_type * const pzNewSPI,          /**< Pointer to the new parameters               */
  UAS_ParameterError_type    * const pnResult           /**< Pointer to an output variable for the result of the parameter check */
);

/**
  * Re-Parameterization of the SafetyConsumerTimeout
  */
UAS_UInt8 byUASCONS_ChangeTimeout
(
  UASCONS_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine,  /**< Pointer to the redundant state machine data */
  UAS_UInt32                   const dwTimeout,        /**< New SafetyConsumerTimeout value             */
  UAS_ParameterError_type   * const pnResult           /**< Pointer to an output variable for the result of the parameter check */
);

/**
  * Start of a SafetyConsumer state machine instance
  */
UAS_UInt8 byUASCONS_Start
(
  UASCONS_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine   /**< Pointer to the redundant state machine data */
);

/**
  * Stop of a SafetyConsumer state machine instance
  */
UAS_UInt8 byUASCONS_Stop
(
  UASCONS_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine   /**< Pointer to the redundant state machine data */
);

/**
  * Execution of a SafetyConsumer state machine instance
  */
UAS_UInt8 byUASCONS_Execute
(
  UASCONS_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine   /**< Pointer to the redundant state machine data */
);


/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/


#endif /* INC_UASCONS_H */
