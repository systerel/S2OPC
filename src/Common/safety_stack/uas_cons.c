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
 * \brief OPC UA SafetyConsumer definition
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the functions of the OPC UA SafetyConsumer.
 *
 * Safety-Related: yes
 */


/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_def.h"
#include "uas_type.h"
#include "uas.h"
#include "uas_crc.h"
#include "uas_rvar.h"


/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_cons.h"


#ifdef UASDEF_DBG

  #include "uas_cdbg.h"

#endif /* ifdef UASDEF_DBG */


/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/


/*--------------------------------------------------------------------------*/
/******************************** L O C A L *********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/* supress the following lint warnings because uniform setting/reading of bit variables in F-PDU and F-Parameters:
"cast from pointer to unsigned long [Encompasses MISRA 2004 Rule 11.1, required], [MISRA 2004 Rule 11.3, advisory]"
"Violates MISRA 2004 Required Rule 19.4, Disallowed definition for macro 'FDDRV_GET_BIT'"
"A zero has been given as right argument to operator '<<'" */

/* This Makro is used to read the value of a single bit from a byte. */
#define /*lint -e(961)*/ UASCONS_GET_BIT(byte,bit)   /*lint -e(835)*/ (UAS_Bool)( ( ( byte ) BITAND (UAS_UInt8)( 0x01u << ( bit ) ) ) >> ( bit ) )

/* This Makro is used to set the value of a single bit within a byte to 1. */
#define /*lint -e(960,961)*/ UASCONS_SET_BIT(byte,bit)   /*lint -e(835)*/ ( ( byte ) = (UAS_UInt8)( ( byte ) BITOR (UAS_UInt8)( 0x01u << ( bit ) ) ) )

/* This Makro is used to set the value of a single bit within a byte to 0. */
#define /*lint -e(960,961)*/ UASCONS_DELETE_BIT(byte,bit)   /*lint -e(835)*/ ( ( byte ) = (UAS_UInt8)( ( byte ) BITAND (UAS_UInt8)( BITNOT (UAS_UInt8)( 0x01u << ( bit ) ) ) ) )


/*-------------*/
/*  T Y P E S  */
/*-------------*/


/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/* Execute activity states */
static UAS_UInt8 byUASCONS_StateWaitForEvent
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/* Execute state 'PrepareRequest' */
static UAS_UInt8 byUASCONS_StatePrepareRequest
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/* Execute state 'CRCCheckSPDU' */
static UAS_UInt8 byUASCONS_StateCRCCheckSPDU
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/* Execute state 'CheckResponseSPDU' */
static UAS_UInt8 byUASCONS_StateCheckResponseSPDU
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/* Execute state 'Error' */
static UAS_UInt8 byUASCONS_StateError
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/* Execute state 'ProvideSafetyData' */
static UAS_UInt8 byUASCONS_StateProvideSafetyData
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/* Calculate the SPDU_ID_i */
static void vUASCONS_CalcSpduId_i
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/* Set diagnostic information */
static void vUASCONS_SetDiag
(
  UAS_SafetyConsumerDI_type * const pzDI,       /**< Pointer to the diagnostic interface */
  UAS_RequestSpdu_type * const pzRequestSpdu,   /**< Pointer to the RequestSPDU */
  UAS_SafetyDiagIdentifier_type const nDiagId,  /**< Identifier for the type of diagnostic output */
  UAS_Bool const bIsPermanent                   /**< Indicate a permanent error */
);

/* Use FSV */
static void vUASCONS_UseFsv
(
  UAS_SafetyConsumer_type * const pzInstance,    /**< Pointer to the instance data   */
  UAS_Bool bConsumerTimerExpired                 /**< Indicates if the ConsumerTimer has been expired */
);

/* Use PV */
static void vUASCONS_UsePv
(
  UAS_SafetyConsumer_type * const pzInstance    /**< Pointer to the instance data   */
  );

/**
 * Check the redundant (inverse) instance data
 */
static UAS_UInt8 byUASCONS_CheckRedundantData
(
  const UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  const UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/**
 * Calculation of an FCS over the static instance data
 */
static UAS_UInt32  dwUASCONS_CalculateFcs
(
  const UAS_SafetyConsumer_type * const pzInstance     /**< Pointer to the instance data   */
);

/**
 * Reset of instance data
 */
static void vUASCONS_ResetInstanceData
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine, /**< Pointer to the redundant state machine data */
  UAS_Bool bAll
);

/**
 * Check the instance data
 */
static UAS_ParameterError_type nUASCONS_ValidInstanceData
(
  const UAS_SafetyConsumer_type * const pzInstance          /**< Pointer to the instance data   */
);

/**
 * Check the SPI parameter
 */
static UAS_ParameterError_type nUASCONS_ValidSPIParam
(
  const UAS_SafetyConsumerSPI_type * const pzSPI   /**< Pointer to the parameters of the SPI */
);

/**
* Check for valid Parameters
*/
static UAS_Bool bUASCONS_ParametersOK
(
  const UAS_SafetyConsumerSPI_type * const pzSPI,     /**< Pointer to the parameters of the SPI */
  const UAS_SafetyConsumerSAPII_type * const pzSAPI   /**< Pointer to the input parameters of the SAPI */
);

/**
* Check for empty SPDU
*/
static UAS_Bool bUASCONS_NoEmptySPDU
(
  const UAS_ResponseSpdu_type * const pzResponseSpdu  /** Pointer to the ResponseSPDU */
);



/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/


/*--------------------------------------------------------------------------*/
/************** I M P L E M E N T A T I O N   ( G L O B A L S ) *************/
/*--------------------------------------------------------------------------*/

/**
  * Initialization of SafetyConsumer state machine
  * This function initializes an SafetyConsumer state machine instance.
  * After successful check of the isntance data set it initializes the SafetyConsumer
  * state machine. It has to be called before using any other function of this module.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \param[in/out]  pzInstance       - Pointer to the redundant instance data
  * \param[out]     pnResult         - Pointer to an output variable for the result of the parameter check
  * \return UASCONS_OK if ok, error code if error occured
  */
UAS_UInt8 byUASCONS_Init
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine,
  UAS_SafetyConsumer_type   * const pzInstance,
  UAS_ParameterError_type   * const pnResult
)
{
  UAS_UInt8  byRetVal = UASCONS_DEFAULT_ERR;
  UAS_UInt16 wIndex = 0u;
  UAS_UInt32 dwFcs = 0uL;

  /* Function parameter invalid? */
  if ( ( NULL EQ pzStateMachine) OR
       ( NULL EQ r_pzStateMachine) OR
       ( NULL EQ pzInstance ) OR
       ( NULL EQ pnResult ) )
  {
    byRetVal = UASCONS_POINTER_ERR;
  } /* if */

  /* Redundant parameters invalid? */
  else if ( ( UASRVAR_INVALID_USIGN32( pzStateMachine->dwParamFcs ) ) OR
            ( UASRVAR_INVALID_POINTER( pzStateMachine->pzInstanceData ) ) )
  {
    byRetVal = UASCONS_SOFT_ERR;
  } /* else if */

  /* State machine instance instance already used? */
  else if ( ( 0uL  NOT_EQ pzStateMachine->dwParamFcs ) OR
            ( NULL NOT_EQ pzStateMachine->pzInstanceData ) )
  {
    byRetVal = UASCONS_STATE_ERR;
  } /* if */

  /* Instance paramater invalid? */
  else
  {
    *pnResult = nUASCONS_ValidInstanceData( pzInstance );

    if ( UAS_PARAMETER_OK EQ *pnResult )
    {
      *pnResult = nUASCONS_ValidSPIParam( &pzInstance->zSPI );
      if ( UAS_PARAMETER_OK EQ *pnResult )
      {
        /* Reset state machine parameters */
        vUASCONS_ResetInstanceData( pzStateMachine, r_pzStateMachine, 1u );

        /* Accept instance parameter */
        dwFcs = dwUASCONS_CalculateFcs( pzInstance );

        /* Initialize state machine parameters */
        UASRVAR_SET_POINTER( pzStateMachine->pzInstanceData, pzInstance );

        /* Initialize the RequestSPDU */
        pzInstance->zRequestSPDU.dwSafetyConsumerId = 0uL;
        pzInstance->zRequestSPDU.dwMonitoringNumber = 0uL;
        pzInstance->zRequestSPDU.byFlags            = 0x00u;

        /* Initialize the SAPI outputs */
        for ( wIndex = 0u; wIndex < pzInstance->wSafetyDataLength; wIndex++ )
        {
          /*lint -e(960) */  /* supress warning "pointer arithmetic other than array indexing used"
                             because of initialization of output data buffer */
          pzInstance->zOutputSAPI.pbySerializedSafetyData[wIndex] = UAS_FSV;
        } /* for */
        for ( wIndex = 0u; wIndex < pzInstance->wNonSafetyDataLength; wIndex++ )
        {
          /*lint -e(960) */  /* supress warning "pointer arithmetic other than array indexing used"
                             because of initialization of output data buffer */
          pzInstance->zOutputSAPI.pbySerializedNonSafetyData[wIndex] = UAS_FSV;
        } /* for */
        pzInstance->zOutputSAPI.bFsvActivated = 1u;
        pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;
        pzInstance->zOutputSAPI.bOperatorAckProvider = 0u;
        pzInstance->zOutputSAPI.bTestModeActivated = 0u;

        /* Store the FCS over the static driver parameter */
        UASRVAR_SET_USIGN32( pzStateMachine->dwParamFcs, dwFcs );

      } /* if UAS_PARAMETER_OK */
    } /* if UAS_PARAMETER_OK */

    byRetVal = UASCONS_OK;
  } /* else */

  return byRetVal;
} /* end of function */


/**
* De-initialization of a SafetyConsumer state machine instance
* This function de-initializes an SafetyConsumer state machine instance.
* All instance data (attributes of the SafetyConsumer state machine) will be written
* with initial values.
* No plausibility checks for instance data will be done here, these checks were done
* by the corresponding UAS function before calling this function.
* \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
* \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
* \return UASCONS_OK if ok, error code if error occured
*/
UAS_UInt8 byUASCONS_Reset
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASCONS_CheckRedundantData(pzStateMachine, r_pzStateMachine);

  if (UASCONS_OK EQ byRetVal )
  {
    if ( UASCONS_S10_INITIALIZE < pzStateMachine->nState )
    {
      byRetVal = byUASCONS_Stop( pzStateMachine, r_pzStateMachine );
    } /* if */
  } /* if */

  /* Reset the instance attributes and parameters */
  if ( UASCONS_POINTER_ERR NOT_EQ byRetVal )
  {
    vUASCONS_ResetInstanceData( pzStateMachine, r_pzStateMachine, 1u );
  } /* if */

  return byRetVal;
} /* end of function */


  /**
  * Re-Parameterization of a SafetyConsumer state machine instance
  * If the instance was initialized and not yet started the given SPI parameters will be checked.
  * Otherwise an error code will be returned.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * Note 1: An instance must first be parameterized successfully by byUASCONS_ChangeSPI
  *         before it can be started by byUASCONS_Start.
  * Note 2: A started instance must first be stopped by byUASCONS_Stop
  *         before it can be re-parameterized by byUASCONS_ChangeSPI.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \param[in]      pzNewSPI         - Pointer to the new parameters
  * \param[out]     pnResult         - Pointer to an output variable for the result of the parameter check
  * \return UASCONS_OK if ok, error code if error occured
  */
UAS_UInt8 byUASCONS_ChangeSPI
(
  UASCONS_StateMachine_type  * const pzStateMachine,
  UASCONS_StateMachine_type  * const r_pzStateMachine,
  UAS_SafetyConsumerSPI_type * const pzNewSPI,
  UAS_ParameterError_type    * const pnResult
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASCONS_CheckRedundantData(pzStateMachine, r_pzStateMachine);
  UAS_UInt32 dwFcs = 0uL;

  if (UASCONS_OK EQ byRetVal )
  {
    if ( UASCONS_S10_INITIALIZE NOT_EQ pzStateMachine->nState )
    {
      /*** Check the new parameters */
      *pnResult = nUASCONS_ValidSPIParam( pzNewSPI );
      if ( UAS_PARAMETER_OK EQ *pnResult )
      {
        /* Accept the new parameters */

        /* Copy the parameters */
        pzStateMachine->pzInstanceData->zSPI = *pzNewSPI;

        /* Calculate and store the new FCS over the static driver parameter */
        dwFcs = dwUASCONS_CalculateFcs( pzStateMachine->pzInstanceData );
        UASRVAR_SET_USIGN32( pzStateMachine->dwParamFcs, dwFcs );
      } /* if */
      byRetVal = UASCONS_OK;
    } /* if */
    else
    {
      byRetVal = UASCONS_STATE_ERR;
    } /* else */
  } /* if */

  return byRetVal;
} /* end of function */


  /**
  * Re-Parameterization of the SafetyConsumerTimeout
  * This function changes the SafetyConsumerTimeout value of the SafetyConsumer state machine instance.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \param[in]      dwTimeout        - New SafetyConsumerTimeout value
  * \param[out]     pnResult         - Pointer to an output variable for the result of the parameter check
  * \return UASCONS_OK if ok, error code if error occured
  */
  /**
  * Parameterization of the SafetyConsumerTimeOut of a SafetyConsumer state machine instance
  */
UAS_UInt8 byUASCONS_ChangeTimeout
(
  UASCONS_StateMachine_type  * const pzStateMachine,
  UASCONS_StateMachine_type  * const r_pzStateMachine,
  UAS_UInt32                   const dwTimeout,
  UAS_ParameterError_type    * const pnResult
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASCONS_CheckRedundantData( pzStateMachine, r_pzStateMachine );
  UAS_UInt32 dwFcs = 0uL;

  if ( UASCONS_OK EQ byRetVal )
  {
    if ( UASCONS_S10_INITIALIZE NOT_EQ pzStateMachine->nState )
    {
      /*** Check the new parameters */
      if ( 0uL < dwTimeout )
      {
        /* Accept the new parameters */

        /* Copy the parameters */
        pzStateMachine->pzInstanceData->zSPI.dwSafetyConsumerTimeout = dwTimeout;

        /* Calculate and store the new FCS over the static driver parameter */
        dwFcs = dwUASCONS_CalculateFcs( pzStateMachine->pzInstanceData );
        UASRVAR_SET_USIGN32( pzStateMachine->dwParamFcs, dwFcs );

        *pnResult = UAS_PARAMETER_OK;
      } /* if */
      else
      {
        *pnResult = UAS_INVALID_SAFETY_CONSUMER_TIMEOUT;
      } /* if */

      byRetVal = UASCONS_OK;
    } /* if */
    else
    {
      byRetVal = UASCONS_STATE_ERR;
    } /* else */
  } /* if */

  return byRetVal;
} /* end of function */


  /**
  * Start of a SafetyConsumer state machine instance
  * This function starts an SafetyConsumer state machine instance.
  * If the instance was parameterized and not yet started the start transitions
  * of the SafetyConsumer state machine will be executed.
  * Otherwise an error code will be returned.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASCONS_OK if ok, error code if error occured
  */
UAS_UInt8 byUASCONS_Start
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASCONS_CheckRedundantData(pzStateMachine, r_pzStateMachine);

  if ( UASCONS_OK EQ byRetVal )
  {
    if ( ( UASCONS_S10_INITIALIZE EQ pzStateMachine->nState ) &&
         ( NULL NOT_EQ pzStateMachine->pzInstanceData ) &&
         ( 0uL NOT_EQ pzStateMachine->dwParamFcs ) )
    {
      UAS_SafetyConsumer_type * const pzInstance = pzStateMachine->pzInstanceData;

      /*** Start transition (Initialization) */
#ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "Initialization" );
#endif /* ifdef UASDEF_DBG */
      vUASCONS_UseFsv( pzInstance, pzStateMachine->bConsumerTimerExpired );
      pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;
      pzInstance->zOutputSAPI.bOperatorAckProvider = 0u;
      pzStateMachine->bFaultReqOA_i = 0u;
      pzStateMachine->bOperatorAckConsumerAllowed_i = 0u;
      pzInstance->zOutputSAPI.bTestModeActivated = 0u;
      pzInstance->zRequestSPDU.dwSafetyConsumerId = 0uL;
      pzInstance->zRequestSPDU.dwMonitoringNumber = 0uL;
      pzInstance->zRequestSPDU.byFlags = 0u;

      /*** T12 ****/
#ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "T12" );
#endif /* ifdef UASDEF_DBG */
      UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S11_WAIT_FOR_RESTART );

      /* log the resulting module state */
#ifdef UASDEF_DBG
      vUASCDBG_PrintSafetyConsumerState( pzStateMachine );
#endif /* ifdef UASDEF_DBG */

      byRetVal = UASCONS_OK;
    } /* if */
    else
    {
      byRetVal = UASCONS_STATE_ERR;
    } /* else */
  } /* if */

  return byRetVal;
} /* end of function */


/**
  * Stop of a SafetyConsumer state machine instance
  * This function stops an SafetyConsumer state machine instance.
  * If the instance was started the SafetyConsumer state machine will be stopped.
  * Otherwise an error code will be returned.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASCONS_OK if ok, error code if error occured
  */
UAS_UInt8 byUASCONS_Stop
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASCONS_CheckRedundantData(pzStateMachine, r_pzStateMachine);

  if ( UASCONS_OK EQ byRetVal )
  {
    if ( UASCONS_S10_INITIALIZE NOT_EQ pzStateMachine->nState )
    {
      UAS_SafetyConsumer_type * const pzInstance = pzStateMachine->pzInstanceData;

      /* Reset SAPI output values */
      vUASCONS_UseFsv( pzInstance, pzStateMachine->bConsumerTimerExpired );
      pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;
      pzInstance->zOutputSAPI.bOperatorAckProvider = 0u;
      pzInstance->zOutputSAPI.bTestModeActivated = 0u;

      /* Reset RequestSPDU values */
      pzInstance->zRequestSPDU.dwSafetyConsumerId = 0uL;
      pzInstance->zRequestSPDU.dwMonitoringNumber = 0uL;
      pzInstance->zRequestSPDU.byFlags = 0u;

      /* Reset only the instance parameters but not the instance configuration */
      vUASCONS_ResetInstanceData( pzStateMachine, r_pzStateMachine, 0u );

      byRetVal = UASCONS_OK;
    } /* if */
  } /* if */

  return byRetVal;
} /* end of function */


/**
  * Execution of a SafetyConsumer state machine instance
  * This function executes an SafetyConsumer state machine instance by calling
  * the functions for the
  *    'WaitForRequest' state,
  *    'PrepareSPDU' state.
  * For each received RequestSPDU one state of each of this groups is passed
  * until the corresponding ResponseSPDU is generated.
  * The partitioning and successive execution of this state groups achieves
  * the execution of the SafetyConsumer state machine until the next interruptable
  * "activity" state where the state machine waits for new input such as timeout
  * or new SPDU.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASCONS_OK if ok, error code if error occured
  */
UAS_UInt8 byUASCONS_Execute
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASCONS_CheckRedundantData(pzStateMachine, r_pzStateMachine);

  /* execute activity states 'Wait for (Re)Start' and 'WaitForSPDU' */
  if ( UASCONS_OK EQ byRetVal )
  {
    byRetVal = byUASCONS_StateWaitForEvent( pzStateMachine, r_pzStateMachine );
  } /* if */

#ifdef UASDEF_DBG

  vUASCDBG_PrintSafetyConsumerState( pzStateMachine );

#endif /* ifdef UASDEF_DBG */

  return byRetVal;
} /* end of function */


/*--------------------------------------------------------------------------*/
/*************** I M P L E M E N T A T I O N   ( L O C A L S ) **************/
/*--------------------------------------------------------------------------*/

/**
  * Execute activity states
  * This function executes the transitions from the activity states 'Wait for (Re)Start' and
  * 'WaitForSPDU' of the SafetyConsumer state machine.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASCONS_OK if ok, error code if error occured
  */
static UAS_UInt8 byUASCONS_StateWaitForEvent
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  UAS_UInt8  byRetVal = UASCONS_DEFAULT_ERR;
  UASTIME_Timestamp_type dwCurrentTime = 0uLL; /*TODO*/
  UAS_SafetyConsumer_type * const pzInstance = pzStateMachine->pzInstanceData;

  switch ( pzStateMachine->nState )
  {
    case UASCONS_S11_WAIT_FOR_RESTART:
    {
      if ( 0u NOT_EQ pzInstance->zInputSAPI.bEnable )
      {
        /*** T13 ***/
        if ( bUASCONS_ParametersOK( &pzInstance->zSPI, &pzInstance->zInputSAPI ) )
        {
          if ( UAS_MNR_MIN > pzStateMachine->dwMNR_i )
          {
            UASRVAR_SET_USIGN32( pzStateMachine->dwMNR_i, UAS_MNR_MIN );
          } /* if */
#ifdef UASDEF_DBG
          vUASCDBG_PrintTransitionName( "T13" );
#endif /* ifdef UASDEF_DBG */
          vUASTIME_StartErrorIntervalLimit( &pzStateMachine->zErrorIntervalTimer, &r_pzStateMachine->zErrorIntervalTimer, dwCurrentTime, pzInstance->zSPI.wSafetyErrorIntervalLimit );
          vUASCONS_CalcSpduId_i( pzStateMachine, r_pzStateMachine );
          UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S12_INITIALIZE_MNR );

          /*** T14 ***/
#ifdef UASDEF_DBG
          vUASCDBG_PrintTransitionName( "T14" );
#endif /* ifdef UASDEF_DBG */
          vUASTIME_StartWatchdog( &pzStateMachine->zConsumerTimer, &r_pzStateMachine->zConsumerTimer, dwCurrentTime, pzInstance->zSPI.dwSafetyConsumerTimeout);
          UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S13_PREPARE_REQUEST );
          byRetVal = byUASCONS_StatePrepareRequest( pzStateMachine, r_pzStateMachine );
        } /* if */

        /*** T27 ***/
        else
        {
#ifdef UASDEF_DBG
          vUASCDBG_PrintTransitionName( "T27" );
#endif /* ifdef UASDEF_DBG */
          vUASCONS_SetDiag( &pzInstance->zDI, &pzInstance->zRequestSPDU, UAS_DIAG_PARAMETERS_INVALID, 1u );
          byRetVal = UASCONS_OK;
        }
      } /* if */
      else
      {
        byRetVal = UASCONS_OK;
      }
      break;
    } /* case UASCONS_S11_WAIT_FOR_RESTART */

    case UASCONS_S14_WAIT_FOR_CHANGED_SPDU:
    {
      UAS_ResponseSpdu_type * const pzResponseSpdu = &pzInstance->zResponseSPDU;
      UAS_RequestSpdu_type * const pzRequestSpdu = &pzInstance->zRequestSPDU;

      pzStateMachine->bConsumerTimerExpired = bUASTIME_Expired( &pzStateMachine->zConsumerTimer, &r_pzStateMachine->zConsumerTimer, dwCurrentTime );
      pzStateMachine->bErrorIntervalTimerExpired = bUASTIME_Expired( &pzStateMachine->zErrorIntervalTimer, &r_pzStateMachine->zErrorIntervalTimer, dwCurrentTime );

      /*** T18 ***/
      if ( pzStateMachine->bConsumerTimerExpired )
      {
#ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T18" );
#endif /* ifdef UASDEF_DBG */
        vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_COMM_ERR_TO, 1u );
        vUASCONS_UseFsv( pzInstance, pzStateMachine->bConsumerTimerExpired );
        if (1u EQ pzInstance->zSPI.bSafetyOperatorAckNecessary)
        {
          pzStateMachine->bFaultReqOA_i = 1u;
          pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;
          UASCONS_SET_BIT( pzRequestSpdu->byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED );
        } /* if */
        else
        {
          /* do nothing */
        } /* else */
        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S17_ERROR );
        byRetVal = byUASCONS_StateError( pzStateMachine, r_pzStateMachine );
      } /* if */

      /*** T17 ***/
      else if ( ( bUASCONS_NoEmptySPDU( pzResponseSpdu ) ) && ( pzResponseSpdu->dwMonitoringNumber NOT_EQ pzStateMachine->dwPrevMNR_i ))
      {
#ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T17" );
#endif /* ifdef UASDEF_DBG */
        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S15_CRC_CHECK_SPDU );
        /* execute 'CheckSPDU' states */
        byRetVal = byUASCONS_StateCRCCheckSPDU( pzStateMachine, r_pzStateMachine );
      } /* else if */

      else
      {
        /* old MNR, ignore SPDU */
        byRetVal = UASCONS_OK;
      } /* else if */
      break;
    } /* case UASCONS_S14_WAIT_FOR_CHANGED_SPDU */

    case UASCONS_S10_INITIALIZE: /* state machine not yet started */
    {
      byRetVal = UASCONS_OK;
      break;
    } /* 'no operation' cases */

    case UASCONS_S12_INITIALIZE_MNR:
    case UASCONS_S13_PREPARE_REQUEST:
    case UASCONS_S15_CRC_CHECK_SPDU:
    case UASCONS_S16_CHECK_RESPONSE_SPDU:
    case UASCONS_S17_ERROR:
    case UASCONS_S18_PROVIDE_SAFETY_DATA:
    default:
    {

#ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "byUASCONS_StateWaitForRequest: Invalid state!" );
#endif /* ifdef UASDEF_DBG */
      byRetVal = UASCONS_STATE_ERR;
      break;
    } /* faulty cases */
  }  /* switch */

  return byRetVal;
} /* end of function */


/**
  * Execute state 'PrepareRequest'
  * This function executes the transitions from the 'PrepareRequest' state of the
  * SafetyConsumer state machine.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASCONS_OK if ok, error code if error occured
  */
static UAS_UInt8 byUASCONS_StatePrepareRequest
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  UAS_UInt8  byRetVal = UASCONS_DEFAULT_ERR;

  switch ( pzStateMachine->nState )
  {
    case UASCONS_S13_PREPARE_REQUEST:
    {
      UAS_SafetyConsumer_type * const pzInstance = pzStateMachine->pzInstanceData;

      /*** T16 ***/
#ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "T16" );
#endif /* ifdef UASDEF_DBG */
      UASRVAR_SET_USIGN32( pzStateMachine->dwPrevMNR_i, pzStateMachine->dwMNR_i );
      if ( UAS_MNR_MAX EQ pzStateMachine->dwMNR_i )
      {
        UASRVAR_SET_USIGN32( pzStateMachine->dwMNR_i, UAS_MNR_MIN );
      } /* if */
      else
      {
        UASRVAR_SET_USIGN32( pzStateMachine->dwMNR_i, pzStateMachine->dwMNR_i + 1u );
      } /* else */
      /* <Build RequestSPDU> */
      if ( 0uL EQ pzInstance->zInputSAPI.dwSafetyConsumerId )
      {
        pzStateMachine->dwConsumerID_i = pzInstance->zSPI.dwSafetyConsumerId;
      } /* if */
      else
      {
        pzStateMachine->dwConsumerID_i = pzInstance->zInputSAPI.dwSafetyConsumerId;
      } /* else */
      pzInstance->zRequestSPDU.dwSafetyConsumerId = pzStateMachine->dwConsumerID_i;
      pzInstance->zRequestSPDU.dwMonitoringNumber = pzStateMachine->dwMNR_i;
      /* RequestSPDU.Flags already set in the transitions. */

      UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S14_WAIT_FOR_CHANGED_SPDU );
      byRetVal = UASCONS_OK;
      break;
    } /* case UASCONS_S13_PREPARE_REQUEST */

    case UASCONS_S11_WAIT_FOR_RESTART: /* state machine not yet started */
    {
      byRetVal = UASCONS_OK;
      break;
    } /* 'no operation' cases */

    case UASCONS_S10_INITIALIZE:
    case UASCONS_S12_INITIALIZE_MNR:
    case UASCONS_S14_WAIT_FOR_CHANGED_SPDU:
    case UASCONS_S15_CRC_CHECK_SPDU:
    case UASCONS_S16_CHECK_RESPONSE_SPDU:
    case UASCONS_S17_ERROR:
    case UASCONS_S18_PROVIDE_SAFETY_DATA:
    default:
    {

#ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "byUASCONS_PrepareRequest: Invalid state!" );
#endif /* ifdef UASDEF_DBG */

      byRetVal = UASCONS_STATE_ERR;
      break;
    } /* faulty cases */
  }  /* switch */

  return byRetVal;
} /* end of function */


/**
  * Execute state 'CRCCheckSPDU'
  * This function executes the transitions from the 'CRCCheckSPDU' state of the
  * SafetyConsumer state machine.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASCONS_OK if ok, error code if error occured
  */
static UAS_UInt8 byUASCONS_StateCRCCheckSPDU
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  UAS_UInt8  byRetVal = UASCONS_DEFAULT_ERR;
  UASTIME_Timestamp_type dwCurrentTime = 0uLL; /*TODO*/

  switch ( pzStateMachine->nState )
  {
    case UASCONS_S15_CRC_CHECK_SPDU:
    {
      UAS_SafetyConsumer_type * const pzInstance = pzStateMachine->pzInstanceData;
      UAS_RequestSpdu_type * const pzRequestSpdu = &pzInstance->zRequestSPDU;
      UAS_ResponseSpdu_type * const pzResponseSPDU = &pzInstance->zResponseSPDU;
      UAS_UInt32 dwCalculatedCrc = dwUASCRC_Calculate( pzResponseSPDU, pzInstance->wSafetyDataLength);

      /* CRC check ok */
      if ( pzResponseSPDU->dwCrc EQ dwCalculatedCrc )
      {
        UASRVAR_SET_BOOL( pzStateMachine->bCRCCheck_i, 1u );
        /*** T21 ***/
#ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T21" );
#endif /* ifdef UASDEF_DBG */
        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S16_CHECK_RESPONSE_SPDU );
        byRetVal = byUASCONS_StateCheckResponseSPDU( pzStateMachine, r_pzStateMachine );
      } /* if */

      /* CRC error and SafetyErrorIntervalTimer expired */
      else if ( pzStateMachine->bErrorIntervalTimerExpired )
      {
        UASRVAR_SET_BOOL( pzStateMachine->bCRCCheck_i, 0u );
        /*** T19 ***/
#ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T19" );
#endif /* ifdef UASDEF_DBG */
        vUASTIME_StartErrorIntervalLimit( &pzStateMachine->zErrorIntervalTimer, &r_pzStateMachine->zErrorIntervalTimer, dwCurrentTime, pzInstance->zSPI.wSafetyErrorIntervalLimit );
        vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_CRC_ERR_IGN, 0u );
        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S13_PREPARE_REQUEST );
        byRetVal = byUASCONS_StatePrepareRequest( pzStateMachine, r_pzStateMachine );
      } /* else if */

      /* CRC error and SafetyErrorIntervalTimer not expired */
      else
      {
        UASRVAR_SET_BOOL( pzStateMachine->bCRCCheck_i, 0u );
        /*** T20 ***/
#ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T20" );
#endif /* ifdef UASDEF_DBG */
        vUASTIME_StartErrorIntervalLimit( &pzStateMachine->zErrorIntervalTimer, &r_pzStateMachine->zErrorIntervalTimer, dwCurrentTime, pzInstance->zSPI.wSafetyErrorIntervalLimit );
        vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_CRC_ERR_OA, 1u );
        vUASCONS_UseFsv( pzInstance, pzStateMachine->bConsumerTimerExpired );
        pzStateMachine->bFaultReqOA_i=1u;
        pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;
        UASCONS_DELETE_BIT( pzRequestSpdu->byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED );
        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S17_ERROR );
        byRetVal = byUASCONS_StateError( pzStateMachine, r_pzStateMachine );
      } /* else */
      break;
    } /* case UASCONS_S15_CRC_CHECK_SPDU */

    case UASCONS_S11_WAIT_FOR_RESTART: /* state machine not yet started */
    {
      byRetVal = UASCONS_OK;
      break;
    } /* 'no operation' cases */

    case UASCONS_S10_INITIALIZE:
    case UASCONS_S12_INITIALIZE_MNR:
    case UASCONS_S13_PREPARE_REQUEST:
    case UASCONS_S14_WAIT_FOR_CHANGED_SPDU:
    case UASCONS_S16_CHECK_RESPONSE_SPDU:
    case UASCONS_S17_ERROR:
    case UASCONS_S18_PROVIDE_SAFETY_DATA:
    default:
    {

#ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "byUASCONS_PrepareRequest: Invalid state!" );
#endif /* ifdef UASDEF_DBG */

      byRetVal = UASCONS_STATE_ERR;
      break;
    } /* faulty cases */
  }  /* switch */

  return byRetVal;
} /* end of function */


/**
  * Execute state 'CheckResponseSPDU'
  * This function executes the transitions from the 'CheckResponseSPDU' state of the
  * SafetyConsumer state machine.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASCONS_OK if ok, error code if error occured
  */
static UAS_UInt8 byUASCONS_StateCheckResponseSPDU
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  UAS_UInt8  byRetVal = UASCONS_DEFAULT_ERR;
  UASTIME_Timestamp_type dwCurrentTime = 0uLL; /*TODO*/

  switch ( pzStateMachine->nState )
  {
    case UASCONS_S16_CHECK_RESPONSE_SPDU:
    {
      UAS_SafetyConsumer_type * const pzInstance = pzStateMachine->pzInstanceData;
      UAS_ResponseSpdu_type * const pzResponseSpdu = &pzInstance->zResponseSPDU;
      UAS_RequestSpdu_type * const pzRequestSpdu = &pzInstance->zRequestSPDU;

      /* SPDU ok */
      if ( ( pzResponseSpdu->zSpduId.dwPart1 EQ pzStateMachine->zSPDUID_i.dwPart1 ) AND
           ( pzResponseSpdu->zSpduId.dwPart2 EQ pzStateMachine->zSPDUID_i.dwPart2 ) AND
           ( pzResponseSpdu->zSpduId.dwPart3 EQ pzStateMachine->zSPDUID_i.dwPart3 ) AND
           ( pzResponseSpdu->dwSafetyConsumerId EQ pzStateMachine->dwConsumerID_i ) AND
           ( pzResponseSpdu->dwMonitoringNumber EQ pzStateMachine->dwMNR_i ) )
      {
        /* <risingEdge ResponseSPDU.Flags.ActivateFSV> */
        UAS_Bool bRisingEdgeOfActivateFsv = 0u;
        UAS_Bool bActivateFSV = UASCONS_GET_BIT( pzResponseSpdu->byFlags, UAS_BITPOS_ACTIVATE_FSV );

        UASRVAR_SET_BOOL( pzStateMachine->bSPDUCheck_i, 1u );
        /*** T22 ***/
#ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T22" );
#endif /* ifdef UASDEF_DBG */
        /* indicate OA from provider */
        pzInstance->zOutputSAPI.bOperatorAckProvider = UASCONS_DELETE_BIT( pzResponseSpdu->byFlags, UAS_BITPOS_OPERATOR_ACK_PROVIDER );

        /* detection of a rising edge of ResponseSPDU.Flags.ActivateFSV */
        if ( ( 0u NOT_EQ bActivateFSV ) AND ( 0u NOT_EQ pzStateMachine->bPrevActivateFSV_i ) )
        {
          bRisingEdgeOfActivateFsv = 1u;
        } /* if */
        UASRVAR_SET_BOOL( pzStateMachine->bPrevActivateFSV_i, bActivateFSV );

        /* OA requested due to edge at ActivateFSV? */
        if ( ( 0u NOT_EQ bRisingEdgeOfActivateFsv ) AND ( 0u NOT_EQ pzInstance->zSPI.bSafetyOperatorAckNecessary ) )
        {
          pzStateMachine->bFaultReqOA_i = 1u;
          vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_FSV_REQUESTED, 1u );
        } /* if */
        else
        {
          /* do nothing */
        } /* else */

        /* Set Flags if OA requested */
        if ( pzStateMachine->bFaultReqOA_i )
        {
          pzInstance->zOutputSAPI.bOperatorAckRequested = 1u;
          UASCONS_SET_BIT( pzRequestSpdu->byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED );
          pzStateMachine->bOperatorAckConsumerAllowed_i = 0u;
          pzStateMachine->bFaultReqOA_i = 0u;
        } /* if */
        else
        {
          /* do nothing */
        } /* else */

        /* Wait until OperatorAckConsumer is not active */
        if ( 0u NOT_EQ  pzInstance->zInputSAPI.bOperatorAckConsumer )
        {
          pzStateMachine->bOperatorAckConsumerAllowed_i = 1u;
        } /* if */
        else
        {
          /* do nothing */
        } /* else */

        /* Reset flags after OA */
        if ( ( 0u NOT_EQ pzInstance->zInputSAPI.bOperatorAckConsumer ) AND ( 0u NOT_EQ pzStateMachine->bOperatorAckConsumerAllowed_i ) )
        {
          pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;
          UASCONS_DELETE_BIT( pzRequestSpdu->byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED );
        } /* if */
        else
        {
          /* do nothing */
        } /* else */

        /* Set process values */
        if ( ( 0u NOT_EQ pzInstance->zOutputSAPI.bOperatorAckRequested ) OR ( 0u NOT_EQ UASCONS_GET_BIT( pzResponseSpdu->byFlags, UAS_BITPOS_ACTIVATE_FSV ) ) )
        {
          vUASCONS_UseFsv( pzInstance, pzStateMachine->bConsumerTimerExpired );
        } /* if */
        else
        {
          vUASCONS_UsePv( pzInstance );
        } /* else */

        /* Notify safety application that SafetyProvider is in test mode */
        pzInstance->zOutputSAPI.bTestModeActivated = UASCONS_GET_BIT( pzResponseSpdu->byFlags, UAS_BITPOS_TEST_MODE_ACTIVATED );

        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S18_PROVIDE_SAFETY_DATA );
        byRetVal = byUASCONS_StateProvideSafetyData( pzStateMachine, r_pzStateMachine );
      } /* if */

      /* SPDU error and SafetyErrorIntervalTimer expired */
      else if ( pzStateMachine->bErrorIntervalTimerExpired )
      {
        UASRVAR_SET_BOOL( pzStateMachine->bSPDUCheck_i, 0u );
        /*** T23 ***/
#ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T23" );
#endif /* ifdef UASDEF_DBG */
        vUASTIME_StartErrorIntervalLimit( &pzStateMachine->zErrorIntervalTimer, &r_pzStateMachine->zErrorIntervalTimer, dwCurrentTime, pzInstance->zSPI.wSafetyErrorIntervalLimit );
        /* Send diagnostic message according the detected error */
        if ( pzResponseSpdu->dwSafetyConsumerId NOT_EQ pzStateMachine->dwConsumerID_i )
        {
          vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_COID_ERR_IGN, 0u );
        } /* if */
        else
        {
          if ( pzResponseSpdu->dwMonitoringNumber NOT_EQ pzStateMachine->dwMNR_i )
          {
            vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_MNR_ERR_IGN, 0u );
          } /* if */
          else
          {
            /* do nothing */
          } /* else */
          if ( ( pzResponseSpdu->zSpduId.dwPart1 NOT_EQ pzStateMachine->zSPDUID_i.dwPart1 ) ||
               ( pzResponseSpdu->zSpduId.dwPart2 NOT_EQ pzStateMachine->zSPDUID_i.dwPart2 ) ||
               ( pzResponseSpdu->zSpduId.dwPart3 NOT_EQ pzStateMachine->zSPDUID_i.dwPart3 ) )
          {
            vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_SD_ID_ERR_IGN, 0u );
          } /* if */
          else
          {
            /* do nothing */
          } /* else */
        } /* else */
        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S13_PREPARE_REQUEST );
        byRetVal = byUASCONS_StatePrepareRequest( pzStateMachine, r_pzStateMachine );
      } /* else if */

      /* SPDU error and SafetyErrorIntervalTimer not expired */
      else
      {
        UASRVAR_SET_BOOL( pzStateMachine->bSPDUCheck_i, 0u );
        /*** T24 ***/
#ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T24" );
#endif /* ifdef UASDEF_DBG */
        vUASTIME_StartErrorIntervalLimit( &pzStateMachine->zErrorIntervalTimer, &r_pzStateMachine->zErrorIntervalTimer, dwCurrentTime, pzInstance->zSPI.wSafetyErrorIntervalLimit );
        /* Send diagnostic message according the detected error */
        if ( pzResponseSpdu->dwSafetyConsumerId NOT_EQ pzStateMachine->dwConsumerID_i )
        {
          vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_COID_ERR_OA, 1u );
        } /* if */
        else
        {
          if ( pzResponseSpdu->dwMonitoringNumber NOT_EQ pzStateMachine->dwMNR_i )
          {
            vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_MNR_ERR_OA, 1u );
          } /* if */
          else
          {
            /* do nothing */
          } /* else */
          if ( ( pzResponseSpdu->zSpduId.dwPart1 NOT_EQ pzStateMachine->zSPDUID_i.dwPart1 ) ||
               ( pzResponseSpdu->zSpduId.dwPart2 NOT_EQ pzStateMachine->zSPDUID_i.dwPart2 ) ||
               ( pzResponseSpdu->zSpduId.dwPart3 NOT_EQ pzStateMachine->zSPDUID_i.dwPart3 ) )
          {
            vUASCONS_SetDiag( &pzInstance->zDI, pzRequestSpdu, UAS_DIAG_SD_ID_ERR_OA_1, 1u );
          } /* if */
          else
          {
            /* do nothing */
          } /* else */
        } /* else */
        pzStateMachine->bFaultReqOA_i = 1u;
        pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;
        UASCONS_DELETE_BIT( pzRequestSpdu->byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED );
        vUASCONS_UseFsv( pzInstance, pzStateMachine->bConsumerTimerExpired );
        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S17_ERROR );
        byRetVal = byUASCONS_StateError( pzStateMachine, r_pzStateMachine );
      } /* else */
      break;
    } /* case UASCONS_S16_CHECK_RESPONSE_SPDU */

    case UASCONS_S11_WAIT_FOR_RESTART: /* state machine not yet started */
    {
      byRetVal = UASCONS_OK;
      break;
    } /* 'no operation' cases */

    case UASCONS_S10_INITIALIZE:
    case UASCONS_S12_INITIALIZE_MNR:
    case UASCONS_S13_PREPARE_REQUEST:
    case UASCONS_S14_WAIT_FOR_CHANGED_SPDU:
    case UASCONS_S15_CRC_CHECK_SPDU:
    case UASCONS_S17_ERROR:
    case UASCONS_S18_PROVIDE_SAFETY_DATA:
    default:
    {

#ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "byUASCONS_PrepareRequest: Invalid state!" );
#endif /* ifdef UASDEF_DBG */

      byRetVal = UASCONS_STATE_ERR;
      break;
    } /* faulty cases */
  }  /* switch */

  return byRetVal;
} /* end of function */

/**
* Execute state 'Error'
* This function executes the transitions from the 'Error' states of the
* SafetyConsumer state machine.
* No plausibility checks for instance data will be done here, these checks were done
* by the corresponding UAS function before calling this function.
* \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
* \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
* \return UASCONS_OK if ok, error code if error occured
*/
static UAS_UInt8 byUASCONS_StateError
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  UAS_UInt8  byRetVal = UASCONS_DEFAULT_ERR;

  switch ( pzStateMachine->nState )
  {
    case UASCONS_S17_ERROR:
    {
      /*** T25 ***/
    #ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "T25" );
    #endif /* ifdef UASDEF_DBG */
      UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S18_PROVIDE_SAFETY_DATA );
      byRetVal = byUASCONS_StateProvideSafetyData( pzStateMachine, r_pzStateMachine );
      break;
    } /* case UASCONS_S17_ERROR */

    case UASCONS_S11_WAIT_FOR_RESTART: /* state machine not yet started */
    {
      byRetVal = UASCONS_OK;
      break;
  } /* 'no operation' cases */

    case UASCONS_S10_INITIALIZE:
    case UASCONS_S12_INITIALIZE_MNR:
    case UASCONS_S13_PREPARE_REQUEST:
    case UASCONS_S14_WAIT_FOR_CHANGED_SPDU:
    case UASCONS_S15_CRC_CHECK_SPDU:
    case UASCONS_S16_CHECK_RESPONSE_SPDU:
    case UASCONS_S18_PROVIDE_SAFETY_DATA:
    default:
    {

    #ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "byUASCONS_PrepareRequest: Invalid state!" );
    #endif /* ifdef UASDEF_DBG */

      byRetVal = UASCONS_STATE_ERR;
      break;
    } /* faulty cases */
  }  /* switch */

  return byRetVal;
} /* end of function */


/**
  * Execute state 'ProvideSafetyData'
  * This function executes the transitions from the 'ProvideSafetyData' states of the
  * SafetyConsumer state machine.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASCONS_OK if ok, error code if error occured
  */
static UAS_UInt8 byUASCONS_StateProvideSafetyData
(
  UASCONS_StateMachine_type * const pzStateMachine,
  UASCONS_StateMachine_type * const r_pzStateMachine
)
{
  UAS_UInt8  byRetVal = UASCONS_DEFAULT_ERR;
  UASTIME_Timestamp_type dwCurrentTime = 0uLL; /*TODO*/

  switch ( pzStateMachine->nState )
  {
    case UASCONS_S18_PROVIDE_SAFETY_DATA:
    {
      UAS_SafetyConsumer_type * const pzInstance = pzStateMachine->pzInstanceData;

      /*** T26 ***/
      if ( 0u NOT_EQ pzInstance->zInputSAPI.bEnable )
      {
#ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T26" );
#endif /* ifdef UASDEF_DBG */
        vUASTIME_StartWatchdog( &pzStateMachine->zConsumerTimer, &r_pzStateMachine->zConsumerTimer, dwCurrentTime, pzInstance->zSPI.dwSafetyConsumerTimeout );
        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S13_PREPARE_REQUEST );
        byRetVal = byUASCONS_StatePrepareRequest( pzStateMachine, r_pzStateMachine );
      } /* if */

      /*** T15 ***/
      else
      {
      #ifdef UASDEF_DBG
        vUASCDBG_PrintTransitionName( "T15" );
      #endif /* ifdef UASDEF_DBG */
        vUASCONS_UseFsv( pzInstance, pzStateMachine->bConsumerTimerExpired );
        UASRVAR_SET_CONSSTATE( pzStateMachine->nState, UASCONS_S11_WAIT_FOR_RESTART );
        byRetVal = UASCONS_OK;
      } /* else */

      break;
    } /* case UASCONS_S13_PREPARE_REQUEST */

    case UASCONS_S11_WAIT_FOR_RESTART: /* state machine not yet started */
    {
      byRetVal = UASCONS_OK;
      break;
    } /* 'no operation' cases */

    case UASCONS_S10_INITIALIZE:
    case UASCONS_S12_INITIALIZE_MNR:
    case UASCONS_S13_PREPARE_REQUEST:
    case UASCONS_S14_WAIT_FOR_CHANGED_SPDU:
    case UASCONS_S15_CRC_CHECK_SPDU:
    case UASCONS_S16_CHECK_RESPONSE_SPDU:
    case UASCONS_S17_ERROR:
    default:
    {

    #ifdef UASDEF_DBG
      vUASCDBG_PrintTransitionName( "byUASCONS_PrepareRequest: Invalid state!" );
    #endif /* ifdef UASDEF_DBG */

      byRetVal = UASCONS_STATE_ERR;
      break;
    } /* faulty cases */
  }  /* switch */

  return byRetVal;
} /* end of function */


/**
  * Calculate the SPDU_ID_i
  * This function calculates the SPDU_ID_i.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  */
static void vUASCONS_CalcSpduId_i
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
)
{
  UAS_SafetyConsumer_type * pzInstance = pzStateMachine->pzInstanceData;
  UAS_UInt32 dwSafetyLevelId = 0uL;
  UAS_UInt32 dwProviderId = 0uL;
  UAS_GUID_type zBaseId = { 0uL, 0uL, 0uL, 0uL };
  UAS_UInt32 dwSpduId_x = 0uL;

  switch ( pzInstance->zSPI.bySafetyProviderLevel )
  {
    case UAS_SAFETY_LEVEL_1:
    {
      dwSafetyLevelId = UAS_SAFETY_LEVEL_1_ID;
      break;
    } /* case */
    case UAS_SAFETY_LEVEL_2:
    {
      dwSafetyLevelId = UAS_SAFETY_LEVEL_2_ID;
      break;
    } /* case */
    case UAS_SAFETY_LEVEL_3:
    {
      dwSafetyLevelId = UAS_SAFETY_LEVEL_3_ID;
      break;
    } /* case */
    case UAS_SAFETY_LEVEL_4:
    {
      dwSafetyLevelId = UAS_SAFETY_LEVEL_4_ID;
      break;
    } /* case */
    default:
    {
      dwSafetyLevelId = 0uL;
      break;
    } /* default */
  } /* switch */

  /* Determine the SafetyBaseID to be used */
  if ( ( 0uL EQ pzInstance->zInputSAPI.zSafetyBaseId.dwData1 ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.wData2 ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.wData3 ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.abyData4[0] ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.abyData4[1] ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.abyData4[2] ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.abyData4[3] ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.abyData4[4] ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.abyData4[5] ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.abyData4[6] ) AND
       ( 0u EQ pzInstance->zInputSAPI.zSafetyBaseId.abyData4[7] ) )
  {
    zBaseId = pzInstance->zSPI.zSafetyBaseId;
  } /* if */
  else
  {
    zBaseId = pzInstance->zInputSAPI.zSafetyBaseId;
  } /* else */

  /* Determine the SafetyProviderID to be used */
  if ( 0uL EQ pzInstance->zInputSAPI.dwSafetyProviderId )
  {
    dwProviderId = pzInstance->zSPI.dwSafetyProviderId;
  } /* if */
  else
  {
    dwProviderId = pzInstance->zInputSAPI.dwSafetyProviderId;
  } /* else */

  /* SPDU_ID_1_i := BaseID (bytes 0…3) XOR SafetyProviderLevel_ID */
  dwSpduId_x = zBaseId.dwData1 XOR dwSafetyLevelId;
  UASRVAR_SET_USIGN32( pzStateMachine->zSPDUID_i.dwPart1, dwSpduId_x );
  /* SPDU_ID_2_i := BaseID (bytes 4…7) XOR SafetyStructureSignature_i */
  dwSpduId_x = ( ( (UAS_UInt32)zBaseId.wData2 ) + (UAS_UInt32)zBaseId.wData3 * 256uL * 256uL )  XOR pzInstance->zSPI.dwSafetyStructureSignature;
  UASRVAR_SET_USIGN32( pzStateMachine->zSPDUID_i.dwPart2, dwSpduId_x );
  /* SPDU_ID_3_i := BaseID (bytes 8…11) XOR BaseID (bytes 12…15) XOR ProviderID */
  dwSpduId_x =
    ( ( (UAS_UInt32)zBaseId.abyData4[0] ) + ( (UAS_UInt32)zBaseId.abyData4[1] * 256uL ) + ( (UAS_UInt32)zBaseId.abyData4[2] * 256u * 256uL ) + (UAS_UInt32)( zBaseId.abyData4[3] * 256uL * 256uL * 256uL ) ) XOR
    ( ( (UAS_UInt32)zBaseId.abyData4[4] ) + ( (UAS_UInt32)zBaseId.abyData4[5] * 256uL ) + ( (UAS_UInt32)zBaseId.abyData4[6] * 256u * 256uL ) + (UAS_UInt32)( zBaseId.abyData4[7] * 256uL * 256uL * 256uL ) ) XOR
      dwProviderId;
  UASRVAR_SET_USIGN32( pzStateMachine->zSPDUID_i.dwPart3, dwSpduId_x );
} /* end of function */


/**
  * Set diagnostic information
  * This function implements the internal item <Set Diag(ID, Boolean isPpermanent)>
  * of the SafetyConsumer state machine.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzDI   - Pointer to the diagnostic interface
  * \param[in/out]  pzRequestSpdu - Pointer to the RequestSPDU
  * \param[in]      nDiagID - Identifier for the type of diagnostic output
  * \param[in]      bIsPermanent - Indicate a permanent error
  */
static void vUASCONS_SetDiag
(
  UAS_SafetyConsumerDI_type * const pzDI,       /**< Pointer to the diagnostic interface */
  UAS_RequestSpdu_type * const pzRequestSpdu,   /**< Pointer to the RequestSPDU */
  UAS_SafetyDiagIdentifier_type const nDiagId,  /**< Identifier for the type of diagnostic output */
  UAS_Bool const bIsPermanent                   /**< Indicate a permanent error */
)
{
  UAS_Bool bCommunicationError = UASCONS_GET_BIT( pzRequestSpdu->byFlags, UAS_BITPOS_COMMUNICATION_ERROR );

  if ( bCommunicationError )
  {
    pzDI->nSafetyDiagnosticId = nDiagId;
    pzDI->bPermanent = bIsPermanent;
  } /* if */
  else
  {
    /* do nothing */
  } /* else */

  if ( bIsPermanent )
  {
    UASCONS_SET_BIT( pzRequestSpdu->byFlags, UAS_BITPOS_COMMUNICATION_ERROR );
  } /* if */
  else
  {
    UASCONS_DELETE_BIT( pzRequestSpdu->byFlags, UAS_BITPOS_COMMUNICATION_ERROR );
  } /* else */
} /* end of function */


/**
* Use FSV
* This function implements the internal item <Use FSV> of the SafetyConsumer state machine.
* No plausibility checks for instance data will be done here, these checks were done
* by the corresponding UAS function before calling this function.
* \param[in/out]  pzInstance            - Pointer to the instance data
* \param[in]      bConsumerTimerExpired - Indicates if the ConsumerTimer has been expired
*/
static void vUASCONS_UseFsv
(
  UAS_SafetyConsumer_type * const pzInstance,
  UAS_Bool bConsumerTimerExpired
)
{
  UAS_UInt16 wIndex;
  UAS_UInt16 wDataLength = pzInstance->wSafetyDataLength;
  UAS_UInt8 * pbyData = pzInstance->zOutputSAPI.pbySerializedSafetyData;

  /* SAPI.SafetyData is set to binary 0 */
  for ( wIndex = 0u; wIndex < wDataLength; wIndex++ )
  {
    pbyData[wIndex] = UAS_FSV;
  } /* for */

  wDataLength = pzInstance->wNonSafetyDataLength;
  pbyData = pzInstance->zOutputSAPI.pbySerializedNonSafetyData;
  if ( ( 0u NOT_EQ bConsumerTimerExpired ) OR ( 0u NOT_EQ pzInstance->zInputSAPI.bEnable ) )
  {
    /* SAPI.NonSafetyData is set to binary 0 */
    for ( wIndex = 0u; wIndex < wDataLength; wIndex++ )
    {
      pbyData[wIndex] = UAS_FSV;
    } /* for */
  } /* if */
  else
  {
    /* SAPI.NonSafetyData is set to ResponseSPDU.NonSafetyData */
    UAS_UInt8 * pbyReceivedData = pzInstance->zResponseSPDU.pbySerializedNonSafetyData;
    for ( wIndex = 0u; wIndex < wDataLength; wIndex++ )
    {
      pbyData[wIndex] = pbyReceivedData[wIndex];
    } /* for */
  } /* else */
  pzInstance->zOutputSAPI.bFsvActivated = 1u;
  UASCONS_SET_BIT( pzInstance->zRequestSPDU.byFlags, 1u );
} /* end of function */


/**
  * Use PV
  * This function implements the internal item <Use PV> of the SafetyConsumer state machine.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzInstance - Pointer to the instance data
  */
static void vUASCONS_UsePv
(
  UAS_SafetyConsumer_type * const pzInstance
)
{
  UAS_UInt16 wIndex;
  UAS_UInt16 wDataLength = pzInstance->wSafetyDataLength;
  UAS_UInt8 * pbyOutputData = pzInstance->zOutputSAPI.pbySerializedSafetyData;
  UAS_UInt8 * pbyReceivedData = pzInstance->zResponseSPDU.pbySerializedSafetyData;

  /* SAPI.SafetyData is set to ResponseSPDU.SafetyData */
  for ( wIndex = 0u; wIndex < wDataLength; wIndex++ )
  {
    pbyOutputData[wIndex] = pbyReceivedData[wIndex];
  } /* for */

  /* SAPI.NonSafetyData is set to ResponseSPDU.NonSafetyData */
  wDataLength = pzInstance->wNonSafetyDataLength;
  pbyOutputData = pzInstance->zOutputSAPI.pbySerializedNonSafetyData;
  pbyReceivedData = pzInstance->zResponseSPDU.pbySerializedNonSafetyData;
  for ( wIndex = 0u; wIndex < wDataLength; wIndex++ )
  {
    pbyOutputData[wIndex] = pbyReceivedData[wIndex];
  } /* for */

  pzInstance->zOutputSAPI.bFsvActivated = 0u;
  UASCONS_DELETE_BIT( pzInstance->zRequestSPDU.byFlags, UAS_BITPOS_FSV_ACTIVATED );
  UASCONS_DELETE_BIT( pzInstance->zRequestSPDU.byFlags, UAS_BITPOS_COMMUNICATION_ERROR );
} /* end of function */


/**
  * Check the redundant (inverse) instance data
  * This function checks the redundant instance data to detect unexpected manipulations
  * in the "static values". Such manipulations can be caused e.g. by soft errors or
  * systematic faults..
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASCONS_OK if ok, error code if error occured
  */
static UAS_UInt8 byUASCONS_CheckRedundantData
(
  const UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  const UASCONS_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
)
{
  UAS_UInt8  byRetVal = UASCONS_DEFAULT_ERR;

  if ( ( NULL EQ pzStateMachine   ) OR
       ( NULL EQ r_pzStateMachine ) )
  {
    byRetVal = UASCONS_POINTER_ERR;
  } /* if */
  else if ( UASRVAR_INVALID_USIGN32( pzStateMachine->dwParamFcs ) )
  {
    byRetVal = UASCONS_SOFT_ERR;
  } /* else if */
  else if ( UASRVAR_INVALID_CONSSTATE  ( pzStateMachine->nState ) )
  {
    byRetVal = UASCONS_SOFT_ERR;
  } /* else if */
  else if ( UASRVAR_INVALID_POINTER( pzStateMachine->pzInstanceData ) )
  {
    byRetVal = UASCONS_SOFT_ERR;
  } /* else if */
  else
  {
    UAS_UInt32 dwParamFcs = dwUASCONS_CalculateFcs( pzStateMachine->pzInstanceData );
    if ( dwParamFcs NOT_EQ pzStateMachine->dwParamFcs )
    {
      byRetVal = UASCONS_SOFT_ERR;
    } /* if */
    else
    {
      byRetVal = UASCONS_OK;
    } /* else */
  } /* else */

  return byRetVal;
} /* end of function */


/**
  * Calculation of an FCS over the static instance data
  * This function calculates a FCS over the static instance parameter.
  * \param[in]  pzStateMachine - Pointer to the instance data
  * \return FSC over the static instance parameter
  */
static UAS_UInt32  dwUASCONS_CalculateFcs
(
  const UAS_SafetyConsumer_type * const pzInstance
)
{
  UAS_UInt32 dwParamFcs = 0uL;

  if ( NULL NOT_EQ pzInstance )
  {
    UAS_UInt8 *pbyParam;
    UAS_UInt8 byIndex;

    /* build FCS across local parameter */
    dwParamFcs += pzInstance->dwHandle;
    dwParamFcs += pzInstance->wSafetyDataLength;
    dwParamFcs += pzInstance->wNonSafetyDataLength;

    /*lint -e928 */  /* supress warning "cast from pointer to pointer" because of
    block operations to calculate a frame check sequence across the static instance parameter */
    /*lint -e960 */  /* start of supress warning "Violates MISRA 2004 Required Rule 17.4, pointer arithmetic other than array indexing used"
    because of block operations to calculate a frame check sequence across the static instance parameter */

    /* build FCS across pointer to SafetyConsumer instance data */
    pbyParam = (UAS_UInt8 *)( &pzInstance );
    for ( byIndex = 0u; byIndex < sizeof (UAS_UInt8 *); byIndex++ )
    {
      dwParamFcs += pbyParam[byIndex];
    } /* for */

    /* build FCS across pointer to SafetyData in the SAPI */
    pbyParam = (UAS_UInt8 *)( &pzInstance->zOutputSAPI.pbySerializedSafetyData );
    for ( byIndex = 0u; byIndex < sizeof (UAS_UInt8 *); byIndex++ )
    {
      dwParamFcs += pbyParam[byIndex];
    } /* for */

      /* build FCS across pointer to NonSafetyData in the SAPI */
    pbyParam = (UAS_UInt8 *)( &pzInstance->zOutputSAPI.pbySerializedNonSafetyData );
    for ( byIndex = 0u; byIndex < sizeof (UAS_UInt8 *); byIndex++ )
    {
      dwParamFcs += pbyParam[byIndex];
    } /* for */

    /* build FCS across pointer to SafetyData in the ResponseSPDU */
    pbyParam = (UAS_UInt8 *)( &pzInstance->zResponseSPDU.pbySerializedSafetyData );
    for ( byIndex = 0u; byIndex < sizeof ( UAS_UInt8 * ); byIndex++ )
    {
      dwParamFcs += pbyParam[byIndex];
    } /* for */

    /* build FCS across pointer to NonSafetyData in the ResponseSPDU */
    pbyParam = (UAS_UInt8 *)( &pzInstance->zResponseSPDU.pbySerializedNonSafetyData );
    for ( byIndex = 0u; byIndex < sizeof ( UAS_UInt8 * ); byIndex++ )
    {
      dwParamFcs += pbyParam[byIndex];
    } /* for */

      /* build FCS across SPI parameter */
    pbyParam = (UAS_UInt8 *)( &pzInstance->zSPI );
    for ( byIndex = 0u; byIndex < sizeof (UAS_SafetyConsumerSPI_type); byIndex++ )
    {
      dwParamFcs += pbyParam[byIndex];
    } /* for */

    /*lint +e928 */  /* end of supress warning */
    /*lint +e960 */  /* end of supress warning */

    if ( 0uL EQ dwParamFcs )
    {
      dwParamFcs = 1uL;
    } /* if */
  } /* if */

  return dwParamFcs;
} /* end of function */


/**
  * Reset of instance data
  * This function resets the attributes of an SafetyConsumer state machine instance.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \param[in]      bAll - Determins if all attributes shall be resetted
  */
static void vUASCONS_ResetInstanceData
(
  UASCONS_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASCONS_StateMachine_type * const r_pzStateMachine, /**< Pointer to the redundant state machine data */
  UAS_Bool bAll
)
{
  UASRVAR_SET_CONSSTATE ( pzStateMachine->nState, UASCONS_S10_INITIALIZE );
  UASRVAR_SET_BOOL      ( pzStateMachine->bFaultReqOA_i, 0u );
  UASRVAR_SET_BOOL      ( pzStateMachine->bOperatorAckConsumerAllowed_i, 0u );
  UASRVAR_SET_USIGN32   ( pzStateMachine->dwMNR_i, 0uL );
  UASRVAR_SET_USIGN32   ( pzStateMachine->dwPrevMNR_i, 0uL );
  UASRVAR_SET_USIGN32   ( pzStateMachine->dwConsumerID_i, 0uL );
  UASRVAR_SET_BOOL      ( pzStateMachine->bCRCCheck_i, 0u );
  UASRVAR_SET_BOOL      ( pzStateMachine->bSPDUCheck_i, 0u );
  UASRVAR_SET_USIGN32   ( pzStateMachine->zSPDUID_i.dwPart1, 0uL );
  UASRVAR_SET_USIGN32   ( pzStateMachine->zSPDUID_i.dwPart2, 0uL );
  UASRVAR_SET_USIGN32   ( pzStateMachine->zSPDUID_i.dwPart3, 0uL );

  if ( 1u EQ bAll )
  {
    UASRVAR_SET_POINTER( pzStateMachine->pzInstanceData, NULL );
  } /* if */
} /* end of function */


/**
  * Check the instance data
  * This function checks the given instance data structure for the SafetyConsumer instance.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in]  pzInstance   - Pointer to the instance data
  * \return UAS_PARAMETER_OK if parameters are valid, or disgnosis information if parameters are invalid
  */
static UAS_ParameterError_type nUASCONS_ValidInstanceData
(
  const UAS_SafetyConsumer_type * const  pzInstance
)
{
  UAS_ParameterError_type nResult = UAS_PARAMETER_OK;

  /***** Check SafetyData length *****/
  if ( ( 0uL EQ pzInstance->wSafetyDataLength ) OR
       ( UASDEF_MAX_PV_LENGTH < pzInstance->wSafetyDataLength ) )
  {
    nResult = UAS_INVALID_SAFETY_DATA_LENGTH;
  } /* if */

  /***** Check NonSafetyData length *****/
  else if ( ( 0uL EQ pzInstance->wNonSafetyDataLength ) OR
            ( UASDEF_MAX_PV_LENGTH < pzInstance->wNonSafetyDataLength ) )
  {
    nResult = UAS_INVALID_NON_SAFETY_DATA_LENGTH;
  } /* else if */

  /***** Check pointer to SafetyData *****/
  else if ( ( NULL EQ pzInstance->zResponseSPDU.pbySerializedSafetyData ) OR
            ( NULL EQ pzInstance->zOutputSAPI.pbySerializedSafetyData ) )
  {
    nResult = UAS_INVALID_SAFETY_DATA_POINTER;
  } /* else if */

  /***** Check pointer to NonSafetyData *****/
  else if ( ( NULL EQ pzInstance->zResponseSPDU.pbySerializedNonSafetyData ) OR
         ( NULL EQ pzInstance->zOutputSAPI.pbySerializedNonSafetyData ) )
  {
    nResult = UAS_INVALID_NON_SAFETY_DATA_POINTER;
  } /* else if */

  /***** No parameter error found *****/
  else
  {
    nResult = UAS_PARAMETER_OK;
  } /* else */

  return ( nResult );
} /* end of function */


/**
  * Check the SPI parameter
  * This function checks the SPI parameter of the SafetyConsumer instance.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in]  pzSPI - Pointer to the SPI parameters
  * \return UAS_PARAMETER_OK if parameters are valid, or disgnosis information if parameters are invalid
  */
static UAS_ParameterError_type nUASCONS_ValidSPIParam
(
  const UAS_SafetyConsumerSPI_type * const pzSPI   /**< Pointer to the primary state machine data   */
)
{
  UAS_ParameterError_type nResult = UAS_PARAMETER_OK;

  /*** check the SafetyProviderID: all values are valid ***/

  /*** check the BaseID: all values are valid ***/

  /*** check the SafetyConsumerID: all values are valid ***/

  /*** check the SafetyProviderLevel ***/
  if ( ( UAS_SAFETY_LEVEL_1 > pzSPI->bySafetyProviderLevel ) OR
       ( UAS_SAFETY_LEVEL_4 < pzSPI->bySafetyProviderLevel ) )
  {
    nResult = UAS_INVALID_SAFETY_PROVIDER_LEVEL;
  } /* if */

  /*** check the SafetyStructureSignature ***/
  else if ( 0uL EQ pzSPI->dwSafetyStructureSignature )
  {
    nResult = UAS_INVALID_SAFETY_STRUCTURE_SIGNATURE;
  } /* if */

  /*** check the SafetyConsumerTimeout ***/
  else if ( 0uL EQ pzSPI->dwSafetyConsumerTimeout )
  {
    nResult = UAS_INVALID_SAFETY_CONSUMER_TIMEOUT;
  } /* if */

  /*** check the SafetyErrorIntervalLimit ***/
  else if ( (   6u NOT_EQ pzSPI->wSafetyErrorIntervalLimit ) AND
            (  60u NOT_EQ pzSPI->wSafetyErrorIntervalLimit ) AND
            ( 600u NOT_EQ pzSPI->wSafetyErrorIntervalLimit ) )
  {
    nResult = UAS_INVALID_SAFETY_ERROR_INTERVAL_LIMIT;
  } /* if */

  /*** No parameter error found ***/
  else
  {
    nResult = UAS_PARAMETER_OK;
  } /* else */

  return ( nResult );
} /* end of function */


/**
  * Check for valid Parameters
  * This function checks the values of the ID parameters. They shall not have the value 0
  * at both interfaces (SPI, SAPI).
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in]  pzResponseSpdu - Pointer to the ResponseSPDU
  * \return 0 if Parameters invalid otherwise 1
  */
static UAS_Bool bUASCONS_ParametersOK
(
  const UAS_SafetyConsumerSPI_type * const pzSPI,     /**< Pointer to the parameters of the SPI */
  const UAS_SafetyConsumerSAPII_type * const pzSAPI   /**< Pointer to the input parameters of the SAPI */
)
{
  UAS_Bool bResult = 1;

  if ( ( 0 EQ pzSAPI->zSafetyBaseId.dwData1     ) AND ( 0 EQ pzSPI->zSafetyBaseId.dwData1     ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.wData2      ) AND ( 0 EQ pzSPI->zSafetyBaseId.wData2      ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.wData3      ) AND ( 0 EQ pzSPI->zSafetyBaseId.wData3      ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.abyData4[0] ) AND ( 0 EQ pzSPI->zSafetyBaseId.abyData4[0] ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.abyData4[1] ) AND ( 0 EQ pzSPI->zSafetyBaseId.abyData4[1] ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.abyData4[2] ) AND ( 0 EQ pzSPI->zSafetyBaseId.abyData4[2] ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.abyData4[3] ) AND ( 0 EQ pzSPI->zSafetyBaseId.abyData4[3] ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.abyData4[4] ) AND ( 0 EQ pzSPI->zSafetyBaseId.abyData4[4] ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.abyData4[5] ) AND ( 0 EQ pzSPI->zSafetyBaseId.abyData4[5] ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.abyData4[6] ) AND ( 0 EQ pzSPI->zSafetyBaseId.abyData4[6] ) AND
       ( 0 EQ pzSAPI->zSafetyBaseId.abyData4[7] ) AND ( 0 EQ pzSPI->zSafetyBaseId.abyData4[7] ) )
  {
    bResult = 0;
  } /* if */

  if ( ( 0 EQ pzSAPI->dwSafetyProviderId ) AND ( 0 EQ pzSPI->dwSafetyProviderId ) )
  {
    bResult = 0;
  } /* if */

  if ( ( 0 EQ pzSAPI->dwSafetyConsumerId ) AND ( 0 EQ pzSPI->dwSafetyConsumerId ) )
  {
    bResult = 0;
  } /* if */

  if ( 0 EQ pzSPI->dwSafetyStructureSignature )
  {
    bResult = 0;
  } /* if */

  return ( bResult );

}

/**
  * Check for empty SPDU
  * This function checks if the ResponseSPDU is empty.
  * [RQ3.3] SPDUs with all values ( incl.CRC signature ) being zero shall be ignored by the receiver.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in]  pzResponseSpdu - Pointer to the ResponseSPDU
  * \return 0 if SPDU is empty otherwise 1
  */
static UAS_Bool bUASCONS_NoEmptySPDU
(
  const UAS_ResponseSpdu_type * const pzResponseSpdu
)
{
  UAS_Bool bRetVal = 0;

  if ( 0uL NOT_EQ pzResponseSpdu->dwMonitoringNumber)
  {
    bRetVal = 1;
  } /* if */
  else if ( 0uL NOT_EQ pzResponseSpdu->dwCrc )
  {
    bRetVal = 1;
  } /* else if */

  return bRetVal;
}






