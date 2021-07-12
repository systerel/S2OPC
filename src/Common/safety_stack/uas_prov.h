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
 * \brief OPC UA SafetyProvider interface definition
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the interface of the OPC UA SafetyProvider.
 *
 * Safety-Related: yes
 */


#ifndef INC_UASPROV_H

#define INC_UASPROV_H


/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_stdtypes.h"
#include "uas_type.h"
#include "uas.h"


/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/** @name Error codes of the UAS SafetyProvider
 *  These macros define the return values of the functions.
 */
/**@{*/
#define UASPROV_OK            ( UASRVAL_NO_ERROR ) /**< Function result OK                 */
#define UASPROV_STATE_ERR     ( UASRVAL_UASPROV_ID + UASRVAL_STATE_ERR     ) /**< Function not allowed in that state */
#define UASPROV_MEMORY_ERR    ( UASRVAL_UASPROV_ID + UASRVAL_MEMORY_ERR    ) /**< Not enough memory available        */
#define UASPROV_POINTER_ERR   ( UASRVAL_UASPROV_ID + UASRVAL_POINTER_ERR   ) /**< Pointer is NULL                    */
#define UASPROV_FCT_PARAM_ERR ( UASRVAL_UASPROV_ID + UASRVAL_FCT_PARAM_ERR ) /**< Parameter is faulty                */
#define UASPROV_DEVICE_ERR    ( UASRVAL_UASPROV_ID + UASRVAL_DEVICE_ERR    ) /**< Device error                       */
#define UASPROV_NOT_IMPL      ( UASRVAL_UASPROV_ID + UASRVAL_NOT_IMPL      ) /**< Function not implemented yet       */
#define UASPROV_NO_DATA       ( UASRVAL_UASPROV_ID + UASRVAL_NO_DATA       ) /**< No data available                  */
#define UASPROV_COMM_ERR      ( UASRVAL_UASPROV_ID + UASRVAL_COMM_ERR      ) /**< Communication error                */
#define UASPROV_TIMER_ERR     ( UASRVAL_UASPROV_ID + UASRVAL_TIMER_ERR     ) /**< Timer error                        */
#define UASPROV_SOFT_ERR      ( UASRVAL_UASPROV_ID + UASRVAL_SOFT_ERR      ) /**< Soft error                         */
#define UASPROV_SYNC_ERR      ( UASRVAL_UASPROV_ID + UASRVAL_SYNC_ERR      ) /**< Synchronization error              */
#define UASPROV_DEFAULT_ERR   ( UASRVAL_UASPROV_ID + UASRVAL_DEFAULT_ERR   ) /**< Function didn't set return value   */
/**@}*/


/*-------------*/
/*  T Y P E S  */
/*-------------*/

/**
  * States enumeration of the SafetyProvider state machine
  */
typedef enum UASPROV_State_enum
{
  UASPROV_S0_INITIALIZE       = 0,
  UASPROV_S1_WAIT_FOR_REQUEST = 1,
  UASPROV_S2_PREPARE_SPDU     = 2
} UASPROV_State_type;

/**
  * This structure contains the internal data of an SafetyProvider instance
  */
typedef struct UASPROV_StateMachine_struct
{
  /* Contains the FCS over the static instance parameter */
  UAS_UInt32  dwParamFcs;

  /** State of the SafetyProvider state machine according to the OPC UA Safety specification */
  UASPROV_State_type nState;

  /** Local Memory for RequestSPDU (required to react on changes). */
  UAS_RequestSpdu_type zRequestSpdu_i;

  /** Pointer to the external instance data */
  UAS_SafetyProvider_type *pzInstanceData;

} UASPROV_StateMachine_type;

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/**
  * Initialization of a SafetyProvider state machine instance
  */
UAS_UInt8 byUASPROV_Init
(
  UASPROV_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type * const r_pzStateMachine,  /**< Pointer to the redundant state machine data */
  UAS_SafetyProvider_type   * const pzInstance,        /**< Pointer to the instance data                */
  UAS_ParameterError_type   * const pnResult           /**< Pointer to an output variable for the result of the parameter check */
);

/**
  * De-initialization of a SafetyProvider state machine instance
  */
UAS_UInt8 byUASPROV_Reset
(
  UASPROV_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type * const r_pzStateMachine   /**< Pointer to the redundant state machine data */
);

/**
  * Parameterization of a SafetyProvider state machine instance
  */
UAS_UInt8 byUASPROV_ChangeSPI
(
  UASPROV_StateMachine_type  * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type  * const r_pzStateMachine,  /**< Pointer to the redundant state machine data */
  UAS_SafetyProviderSPI_type * const pzNewSPI,          /**< Pointer to the new parameters               */
  UAS_ParameterError_type    * const pnResult           /**< Pointer to an output variable for the result of the parameter check */
);

/**
  * Start of a SafetyProvider state machine instance
  */
UAS_UInt8 byUASPROV_Start
(
  UASPROV_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type * const r_pzStateMachine   /**< Pointer to the redundant state machine data */
);

/**
  * Stop of a SafetyProvider state machine instance
  */
UAS_UInt8 byUASPROV_Stop
(
  UASPROV_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type * const r_pzStateMachine   /**< Pointer to the redundant state machine data */
);

/**
  * Execution of the SafetyProvider state machine instance
  */
UAS_UInt8 byUASPROV_Execute
(
  UASPROV_StateMachine_type * const pzStateMachine,    /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type * const r_pzStateMachine   /**< Pointer to the redundant state machine data */
);


/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/


#endif /* INC_UASPROV_H */
