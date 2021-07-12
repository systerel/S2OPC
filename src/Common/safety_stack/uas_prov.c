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
 * \brief OPC UA SafetyProvider definition
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the functions of the OPC UA SafetyProvider.
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

#include "uas_prov.h"


#ifdef UASDEF_DBG

  #include "uas_pdbg.h"

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

/* This Makro is used to set the value of a single bit within a byte to 1. */
#define /*lint -e(960,961)*/ UASPROV_SET_BIT(byte,bit)   /*lint -e(835)*/ ( ( byte ) = (UAS_UInt8)( ( byte ) BITOR (UAS_UInt8)( 0x01u << ( bit ) ) ) )

/* This Makro is used to read the value of a single bit from a byte. */
#define /*lint -e(961)*/ UASPROV_GET_BIT(byte,bit)   /*lint -e(835)*/ (UAS_Bool)( ( ( byte ) BITAND (UAS_UInt8)( 0x01u << ( bit ) ) ) >> ( bit ) )


/*-------------*/
/*  T Y P E S  */
/*-------------*/


/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/* Execute state 'WaitForRequest' */
static UAS_UInt8 byUASPROV_StateWaitForRequest
(
  UASPROV_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/* Execute state 'PrepareSPDU' */
static UAS_UInt8 byUASPROV_StatePrepareSpdu
(
  UASPROV_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/* Builds the ResponseSPDU */
static void vUASPROV_BuildResponseSPDU
(
  const UASPROV_StateMachine_type * const pzStateMachine    /**< Pointer to the primary state machine data   */
);

/**
 * Check the redundant (inverse) instance data
 */
static UAS_UInt8 byUASPROV_CheckRedundantData
(
  const UASPROV_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  const UASPROV_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
);

/**
 * Calculation of an FCS over the static instance data
 */
static UAS_UInt32  byUASPROV_CalculateFcs
(
  const UAS_SafetyProvider_type * const pzInstance    /**< Pointer to the instance data   */
);

/**
 * Reset of instance data
 */
static void vUASPROV_ResetInstanceData
(
  UASPROV_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type * const r_pzStateMachine, /**< Pointer to the redundant state machine data */
  UAS_Bool bAll
);

/**
 * Check the instance data
 */
static UAS_ParameterError_type nUASPROV_ValidInstanceData
(
  const UAS_SafetyProvider_type * const  pzInstance  /**< Pointer to the instance data   */
);

/**
 * Check the SPI parameter
 */
static UAS_ParameterError_type nUASPROV_ValidSPIParam
(
  const UAS_SafetyProviderSPI_type * const pzSPI   /**< Pointer to the SPI parameters   */
);


/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/


/*--------------------------------------------------------------------------*/
/************** I M P L E M E N T A T I O N   ( G L O B A L S ) *************/
/*--------------------------------------------------------------------------*/

/**
  * Initialization of SafetyProvider state machine
  * This function initializes an SafetyProvider state machine instance.
  * After successful check of the isntance data set it initializes the SafetyProvider
  * state machine. It has to be called before using any other function of this module.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \param[in/out]  pzInstance       - Pointer to the redundant instance data
  * \param[out]     pnResult         - Pointer to an output variable for the result of the parameter check
  * \return UASPROV_OK if ok, error code if error occured
  */
UAS_UInt8 byUASPROV_Init
(
  UASPROV_StateMachine_type * const pzStateMachine,
  UASPROV_StateMachine_type * const r_pzStateMachine,
  UAS_SafetyProvider_type   * const pzInstance,
  UAS_ParameterError_type   * const pnResult
)
{
  UAS_UInt8  byRetVal = UASPROV_DEFAULT_ERR;
  UAS_UInt16 wIndex = 0u;
  UAS_UInt32 dwFcs = 0uL;

  /* Function parameter invalid? */
  if ( ( NULL EQ pzStateMachine) OR
       ( NULL EQ r_pzStateMachine) OR
       ( NULL EQ pzInstance) OR
       ( NULL EQ pnResult ) )
  {
    byRetVal = UASPROV_POINTER_ERR;
  } /* if */

  /* Redundant parameters invalid? */
  else if ( ( UASRVAR_INVALID_USIGN32( pzStateMachine->dwParamFcs ) ) OR
            ( UASRVAR_INVALID_POINTER( pzStateMachine->pzInstanceData ) ) )
  {
    byRetVal = UASPROV_SOFT_ERR;
  } /* else if */

  /* State machine instance instance already used? */
  else if ( ( 0uL  NOT_EQ pzStateMachine->dwParamFcs ) OR
            ( NULL NOT_EQ pzStateMachine->pzInstanceData ) )
  {
    byRetVal = UASPROV_STATE_ERR;
  } /* if */

  /* Instance paramater invalid? */
  else
  {
    *pnResult = nUASPROV_ValidInstanceData( pzInstance );

    if ( UAS_PARAMETER_OK EQ *pnResult )
    {
      *pnResult = nUASPROV_ValidSPIParam( &pzInstance->zSPI );
      if ( UAS_PARAMETER_OK EQ *pnResult )
      {
        /* Reset state machine parameters */
        vUASPROV_ResetInstanceData( pzStateMachine, r_pzStateMachine, 1u );

        /* Accept instance parameter */
        dwFcs = byUASPROV_CalculateFcs( pzInstance );

        /* Initialize state machine parameters */
        UASRVAR_SET_POINTER( pzStateMachine->pzInstanceData, pzInstance );

        /* Initialize the ResponseSPDU */
        for ( wIndex = 0u; wIndex < pzInstance->wSafetyDataLength; wIndex++ )
        {
          /*lint -e(960) */  /* supress warning "pointer arithmetic other than array indexing used"
                                because of initialization of output data buffer */
          pzInstance->zResponseSPDU.pbySerializedSafetyData[wIndex] = UAS_FSV;
        } /* for */
        pzInstance->zResponseSPDU.byFlags = 0x00u;
        pzInstance->zResponseSPDU.zSpduId.dwPart1 = 0uL;
        pzInstance->zResponseSPDU.zSpduId.dwPart2 = 0uL;
        pzInstance->zResponseSPDU.zSpduId.dwPart3 = 0uL;
        pzInstance->zResponseSPDU.dwSafetyConsumerId = 0uL;
        pzInstance->zResponseSPDU.dwMonitoringNumber = 0uL;
        pzInstance->zResponseSPDU.dwCrc = 0uL;
        for (wIndex = 0u; wIndex < pzInstance->wNonSafetyDataLength; wIndex++)
        {
          /*lint -e(960) */  /* supress warning "pointer arithmetic other than array indexing used"
                                because of initialization of output data buffer */
          pzInstance->zResponseSPDU.pbySerializedNonSafetyData[wIndex] = UAS_FSV;
        } /* for */

        /* Initialize the SAPI outputs */
        pzInstance->zOutputSAPI.dwMonitoringNumber = 0uL;
        pzInstance->zOutputSAPI.dwSafetyConsumerId = 0uL;
        pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;

        /* Store the FCS over the static driver parameter */
        UASRVAR_SET_USIGN32( pzStateMachine->dwParamFcs, dwFcs );

      } /* if UAS_PARAMETER_OK */
    } /* if UAS_PARAMETER_OK */

    byRetVal = UASPROV_OK;
  } /* else */

  return byRetVal;
} /* end of function */


  /**
  * De-initialization of a SafetyProvider state machine instance
  * This function de-initializes an SafetyProvider state machine instance.
  * All instance data (attributes of the SafetyProvider state machine) will be written
  * with initial values.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASPROV_OK if ok, error code if error occured
  */
UAS_UInt8 byUASPROV_Reset
(
  UASPROV_StateMachine_type * const pzStateMachine,
  UASPROV_StateMachine_type * const r_pzStateMachine
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASPROV_CheckRedundantData(pzStateMachine, r_pzStateMachine);

  if (UASPROV_OK EQ byRetVal )
  {
    if ( UASPROV_S0_INITIALIZE NOT_EQ pzStateMachine->nState )
    {
      byRetVal = byUASPROV_Stop( pzStateMachine, r_pzStateMachine );
    } /* if */
  } /* if */

  /* Reset the instance attributes and parameters */
  if ( UASPROV_POINTER_ERR NOT_EQ byRetVal )
  {
    vUASPROV_ResetInstanceData( pzStateMachine, r_pzStateMachine, 1u );
  } /* if */

  return byRetVal;
} /* end of function */


/**
  * Re-Parameterization of a SafetyProvider state machine instance
  * If the instance was initialized and not yet started the given SPI parameters will be checked.
  * Otherwise an error code will be returned.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * Note 1: An instance must first be parameterized successfully by byUASPROV_ChangeSPI
  *         before it can be started by byUASPROV_Start.
  * Note 2: A started instance must first be stopped by byUASPROV_Stop
  *         before it can be re-parameterized by byUASPROV_ChangeSPI.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \param[in]      pzNewSPI         - Pointer to the new parameters
  * \param[out]     pnResult         - Pointer to an output variable for the result of the parameter check
  * \return UASPROV_OK if ok, error code if error occured
  */
UAS_UInt8 byUASPROV_ChangeSPI
(
  UASPROV_StateMachine_type  * const pzStateMachine,
  UASPROV_StateMachine_type  * const r_pzStateMachine,
  UAS_SafetyProviderSPI_type * const pzNewSPI,
  UAS_ParameterError_type    * const pnResult
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASPROV_CheckRedundantData(pzStateMachine, r_pzStateMachine);
  UAS_UInt32 dwFcs = 0uL;

  if (UASPROV_OK EQ byRetVal )
  {
    if ( UASPROV_S0_INITIALIZE NOT_EQ pzStateMachine->nState )
    {
      /*** Check the new parameters */
      *pnResult = nUASPROV_ValidSPIParam( pzNewSPI );
      if ( UAS_PARAMETER_OK EQ *pnResult )
      {
        /* Accept the new parameters */

        /* Copy the parameters */
        pzStateMachine->pzInstanceData->zSPI = *pzNewSPI;

        /* Calculate and store the new FCS over the static driver parameter */
        dwFcs = byUASPROV_CalculateFcs( pzStateMachine->pzInstanceData );
        UASRVAR_SET_USIGN32( pzStateMachine->dwParamFcs, dwFcs );
      } /* if */
      byRetVal = UASPROV_OK;
    } /* if */
    else
    {
      byRetVal = UASPROV_STATE_ERR;
    } /* else */
  } /* if */

  return byRetVal;
} /* end of function */


/**
  * Start of a SafetyProvider state machine instance
  * This function starts an SafetyProvider state machine instance.
  * If the instance was parameterized and not yet started the start transitions
  * of the SafetyProvider state machine will be executed.
  * Otherwise an error code will be returned.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASPROV_OK if ok, error code if error occured
  */
UAS_UInt8 byUASPROV_Start
(
  UASPROV_StateMachine_type * const pzStateMachine,
  UASPROV_StateMachine_type * const r_pzStateMachine
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASPROV_CheckRedundantData(pzStateMachine, r_pzStateMachine);

  UAS_UInt16 wIndex = 0u;

  if ( UASPROV_OK EQ byRetVal )
  {
    if ( ( UASPROV_S0_INITIALIZE EQ pzStateMachine->nState ) &&
         ( NULL NOT_EQ pzStateMachine->pzInstanceData ) &&
         ( 0uL NOT_EQ pzStateMachine->dwParamFcs ) )
    {
      UAS_SafetyProvider_type * const pzInstance = pzStateMachine->pzInstanceData;

      /*** Start transition (Initialization) */
#ifdef UASDEF_DBG
      vUASPDBG_PrintTransitionName( "Initialization" );
#endif /* ifdef UASDEF_DBG */
      for ( wIndex = 0u; wIndex < pzStateMachine->pzInstanceData->wSafetyDataLength; wIndex++ )
      {
        /*lint -e(960) */  /* supress warning "pointer arithmetic other than array indexing used"
                           because of initialization of output data buffer */
        pzInstance->zInputSAPI.pbySerializedSafetyData[wIndex] = UAS_FSV;
      } /* for */
      pzInstance->zOutputSAPI.dwMonitoringNumber = 0uL;
      pzInstance->zOutputSAPI.dwSafetyConsumerId = 0uL;
      pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;
      UASRVAR_SET_USIGN32( pzStateMachine->zRequestSpdu_i.dwSafetyConsumerId, 0uL );
      UASRVAR_SET_USIGN32( pzStateMachine->zRequestSpdu_i.dwMonitoringNumber, 0uL );
      UASRVAR_SET_USIGN8( pzStateMachine->zRequestSpdu_i.byFlags, 0u );

      /*** T1 ****/
#ifdef UASDEF_DBG
      vUASPDBG_PrintTransitionName( "T1" );
#endif /* ifdef UASDEF_DBG */
      UASRVAR_SET_PROVSTATE( pzStateMachine->nState, UASPROV_S1_WAIT_FOR_REQUEST );

      /* log the resulting module state */
#ifdef UASDEF_DBG
      vUASPDBG_PrintSafetyProviderState( pzStateMachine );
#endif /* ifdef UASDEF_DBG */

      byRetVal = UASPROV_OK;
    } /* if */
    else
    {
      byRetVal = UASPROV_STATE_ERR;
    } /* else */
  } /* if */

  return byRetVal;
} /* end of function */


/**
  * Stop of a SafetyProvider state machine instance
  * This function stops an SafetyProvider state machine instance.
  * If the instance was started the SafetyProvider state machine will be stopped.
  * Otherwise an error code will be returned.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASPROV_OK if ok, error code if error occured
  */
UAS_UInt8 byUASPROV_Stop
(
  UASPROV_StateMachine_type * const pzStateMachine,
  UASPROV_StateMachine_type * const r_pzStateMachine
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASPROV_CheckRedundantData(pzStateMachine, r_pzStateMachine);

  if ( UASPROV_OK EQ byRetVal )
  {
    if ( UASPROV_S0_INITIALIZE NOT_EQ pzStateMachine->nState )
    {
      UAS_SafetyProvider_type * const pzInstance = pzStateMachine->pzInstanceData;
      UAS_UInt16 wIndex;

      /* Reset SAPI output values */
      pzInstance->zOutputSAPI.dwSafetyConsumerId = 0uL;
      pzInstance->zOutputSAPI.dwMonitoringNumber = 0uL;
      pzInstance->zOutputSAPI.bOperatorAckRequested = 0u;

      /* Reset ResponseSPDU values */
      for ( wIndex = 0u; wIndex < pzInstance->wSafetyDataLength; wIndex++ )
      {
        pzInstance->zResponseSPDU.pbySerializedSafetyData[wIndex] = UAS_FSV;
      } /* for */
      pzInstance->zResponseSPDU.byFlags = 0u;
      pzInstance->zResponseSPDU.zSpduId.dwPart1= 0uL;
      pzInstance->zResponseSPDU.zSpduId.dwPart2= 0uL;
      pzInstance->zResponseSPDU.zSpduId.dwPart3= 0uL;
      pzInstance->zResponseSPDU.dwSafetyConsumerId = 0uL;
      pzInstance->zResponseSPDU.dwMonitoringNumber = 0uL;
      pzInstance->zResponseSPDU.dwCrc = 0uL;
      for ( wIndex = 0u; wIndex < pzInstance->wNonSafetyDataLength; wIndex++ )
      {
        pzInstance->zResponseSPDU.pbySerializedNonSafetyData[wIndex] = UAS_FSV;
      } /* for */

      /* Reset only the instance parameters but not the instance configuration */
      vUASPROV_ResetInstanceData( pzStateMachine, r_pzStateMachine, 0u );

      byRetVal = UASPROV_OK;
    } /* if */
  } /* if */

  return byRetVal;
} /* end of function */


/**
  * Execution of a SafetyProvider state machine instance
  * This function executes an SafetyProvider state machine instance by calling
  * the functions for the
  *    'WaitForRequest' state,
  *    'PrepareSPDU' state.
  * For each received RequestSPDU one state of each of this groups is passed
  * until the corresponding ResponseSPDU is generated.
  * The partitioning and successive execution of this state groups achieves
  * the execution of the SafetyProvider state machine until the next interruptable
  * "activity" state where the state machine waits for new input such as timeout
  * or new SPDU.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASPROV_OK if ok, error code if error occured
  */
UAS_UInt8 byUASPROV_Execute
(
  UASPROV_StateMachine_type * const pzStateMachine,
  UASPROV_StateMachine_type * const r_pzStateMachine
)
{
  /* check the redundant instance data */
  UAS_UInt8  byRetVal = byUASPROV_CheckRedundantData(pzStateMachine, r_pzStateMachine);

  /* execute 'WaitForRequest' state */
  if ( UASPROV_OK EQ byRetVal )
  {
    byRetVal = byUASPROV_StateWaitForRequest( pzStateMachine, r_pzStateMachine );
  } /* if */

  /* execute 'PrepareSPDU' state */
  if ( UASPROV_OK EQ byRetVal )
  {
    byRetVal = byUASPROV_StatePrepareSpdu( pzStateMachine, r_pzStateMachine );
  } /* if */

#ifdef UASDEF_DBG

  vUASPDBG_PrintSafetyProviderState( pzStateMachine );

#endif /* ifdef UASDEF_DBG */

  return byRetVal;
} /* end of function */

/*--------------------------------------------------------------------------*/
/*************** I M P L E M E N T A T I O N   ( L O C A L S ) **************/
/*--------------------------------------------------------------------------*/

/**
  * Execute state 'WaitForRequest'
  * This function executes the transitions from the 'WaitForRequest' states of the
  * SafetyProvider state machine.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASPROV_OK if ok, error code if error occured
  */
static UAS_UInt8 byUASPROV_StateWaitForRequest
(
  UASPROV_StateMachine_type * const pzStateMachine,
  UASPROV_StateMachine_type * const r_pzStateMachine
)
{
  UAS_UInt8  byRetVal = UASPROV_DEFAULT_ERR;

  switch ( pzStateMachine->nState )
  {
    case UASPROV_S1_WAIT_FOR_REQUEST:
    {
      UAS_RequestSpdu_type * const pzRequestSPDU = &pzStateMachine->pzInstanceData->zRequestSPDU;
      UAS_SafetyProviderSAPIO_type * const pzOutputSAPI = &pzStateMachine->pzInstanceData->zOutputSAPI;

      /*** T2 ***/
      /* When: [RequestSPDU_i <> RequestSPDU] */
      if ( ( pzStateMachine->zRequestSpdu_i.dwSafetyConsumerId NOT_EQ pzRequestSPDU->dwSafetyConsumerId ) OR
           ( pzStateMachine->zRequestSpdu_i.dwMonitoringNumber NOT_EQ pzRequestSPDU->dwMonitoringNumber ) OR
           ( pzStateMachine->zRequestSpdu_i.byFlags NOT_EQ pzRequestSPDU->byFlags ) )
      {
#ifdef UASDEF_DBG
        vUASPDBG_PrintTransitionName( "T2" );
#endif /* ifdef UASDEF_DBG */
        UASRVAR_SET_USIGN32( pzStateMachine->zRequestSpdu_i.dwSafetyConsumerId, pzRequestSPDU->dwSafetyConsumerId );
        UASRVAR_SET_USIGN32( pzStateMachine->zRequestSpdu_i.dwMonitoringNumber, pzRequestSPDU->dwMonitoringNumber );
        UASRVAR_SET_USIGN8 ( pzStateMachine->zRequestSpdu_i.byFlags, pzRequestSPDU->byFlags );
        pzOutputSAPI->dwMonitoringNumber = pzRequestSPDU->dwMonitoringNumber;
        pzOutputSAPI->dwSafetyConsumerId = pzRequestSPDU->dwSafetyConsumerId;
        pzOutputSAPI->bOperatorAckRequested = UASPROV_GET_BIT( pzRequestSPDU->byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED );

        UASRVAR_SET_PROVSTATE( pzStateMachine->nState, UASPROV_S2_PREPARE_SPDU );
      } /* if */
      byRetVal = UASPROV_OK;
      break;
    } /* case UASPROV_S1_WAIT_FOR_REQUEST */

    case UASPROV_S0_INITIALIZE: /* state machine not yet started */
    {
      /* do nothing */
      byRetVal = UASPROV_OK;
      break;
    } /* 'no operation' cases */

    case UASPROV_S2_PREPARE_SPDU:
    default:
    {
#ifdef UASDEF_DBG
      vUASPDBG_PrintTransitionName( "byUASPROV_StateWaitForRequest: Invalid state!" );
#endif /* ifdef UASDEF_DBG */
      byRetVal = UASPROV_STATE_ERR;
      break;
    } /* faulty cases */
  }  /* switch */

  return byRetVal;
} /* end of function */


/**
  * Execute state 'PrepareSPDU'
  * This function executes the transitions from the 'PrepareSPDU' states of the
  * SafetyProvider state machine.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASPROV_OK if ok, error code if error occured
  */
static UAS_UInt8 byUASPROV_StatePrepareSpdu
(
  UASPROV_StateMachine_type * const pzStateMachine,
  UASPROV_StateMachine_type * const r_pzStateMachine
)
{
  UAS_UInt8  byRetVal = UASPROV_DEFAULT_ERR;

  switch ( pzStateMachine->nState )
  {
    case UASPROV_S2_PREPARE_SPDU:
    {
      /*** T3 ***/
#ifdef UASDEF_DBG
      vUASPDBG_PrintTransitionName( "T3" );
#endif /* ifdef UASDEF_DBG */

      UASRVAR_SET_PROVSTATE( pzStateMachine->nState, UASPROV_S1_WAIT_FOR_REQUEST );
      vUASPROV_BuildResponseSPDU ( pzStateMachine );
      byRetVal = UASPROV_OK;
      break;
    } /* case UASPROV_S2_PREPARE_SPDU */

    case UASPROV_S0_INITIALIZE: /* state machine not yet started */
    case UASPROV_S1_WAIT_FOR_REQUEST: /* unchanged RequestSPDU */
    {
      /* do nothing */
      byRetVal = UASPROV_OK;
      break;
    } /* 'no operation' cases */

    default:
    {
#ifdef UASDEF_DBG
      vUASPDBG_PrintTransitionName( "byUASPROV_StateWaitForRequest: Invalid state!" );
#endif /* ifdef UASDEF_DBG */
      byRetVal = UASPROV_STATE_ERR;
      break;
    } /* faulty cases */
  }  /* switch */

  return byRetVal;
} /* end of function */


/**
  * Build the ResponseSPDU
  * This function builds the ResponseSPDU.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  */
static void vUASPROV_BuildResponseSPDU
(
  const UASPROV_StateMachine_type * const pzStateMachine
)
{
  UAS_ResponseSpdu_type * const pzResponseSpdu = &pzStateMachine->pzInstanceData->zResponseSPDU;
  UAS_SafetyProviderSAPII_type * const pzInputSAPI = &pzStateMachine->pzInstanceData->zInputSAPI;
  UAS_SafetyProviderSPI_type * const pzSPI = &pzStateMachine->pzInstanceData->zSPI;
  UAS_UInt16 wIndex;
  UAS_UInt16 wDataLength;

  /* Copy the SafetyData */
  wDataLength = pzStateMachine->pzInstanceData->wSafetyDataLength;
  for ( wIndex = 0u; wIndex < wDataLength; wIndex++ )
  {
    pzResponseSpdu->pbySerializedSafetyData[wIndex] = pzInputSAPI->pbySerializedSafetyData[wIndex];
  } /* for */

  /* Copy the Flags */
  pzResponseSpdu->byFlags = 0u;
  if ( pzInputSAPI->bActivateFsv )
  {
    UASPROV_SET_BIT( pzResponseSpdu->byFlags, UAS_BITPOS_ACTIVATE_FSV );
  } /* if */
  if ( pzInputSAPI->bOperatorAckProvider )
  {
    UASPROV_SET_BIT( pzResponseSpdu->byFlags, UAS_BITPOS_OPERATOR_ACK_PROVIDER );
  } /* if */
  if ( pzInputSAPI->bEnableTestMode )
  {
    UASPROV_SET_BIT( pzResponseSpdu->byFlags, UAS_BITPOS_TEST_MODE_ACTIVATED );
  } /* if */

  /* Determine the SafetyBaseID to be used in SPDU_ID */
  if ( ( 0uL EQ pzInputSAPI->zSafetyBaseId.dwData1     ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.wData2      ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.wData3      ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.abyData4[0] ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.abyData4[1] ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.abyData4[2] ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.abyData4[3] ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.abyData4[4] ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.abyData4[5] ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.abyData4[6] ) AND
       ( 0u  EQ pzInputSAPI->zSafetyBaseId.abyData4[7] ) )
  {
    /* SPDU_ID_1_i := BaseID (bytes 0…3) XOR SafetyProviderLevel_ID */
    pzResponseSpdu->zSpduId.dwPart1 = pzSPI->zSafetyBaseId.dwData1 XOR UASDEF_SAFETY_PROVIDER_LEVEL_ID;
    /* SPDU_ID_2_i := BaseID (bytes 4…7) XOR SafetyStructureSignature_i */
    pzResponseSpdu->zSpduId.dwPart2 = ( ( (UAS_UInt32)pzSPI->zSafetyBaseId.wData2 ) + (UAS_UInt32)pzSPI->zSafetyBaseId.wData3 * 256uL * 256uL )  XOR pzSPI->dwSafetyStructureSignature;
    /* SPDU_ID_3_i := BaseID (bytes 8…11) XOR BaseID (bytes 12…15) XOR ProviderID */
    pzResponseSpdu->zSpduId.dwPart3 =
    ( ( (UAS_UInt32)pzSPI->zSafetyBaseId.abyData4[0] ) + ( (UAS_UInt32)pzSPI->zSafetyBaseId.abyData4[1] * 256uL ) + ( (UAS_UInt32)pzSPI->zSafetyBaseId.abyData4[2] * 256u * 256uL ) + (UAS_UInt32)( pzSPI->zSafetyBaseId.abyData4[3] * 256uL * 256uL * 256uL ) ) XOR
    ( ( (UAS_UInt32)pzSPI->zSafetyBaseId.abyData4[4] ) + ( (UAS_UInt32)pzSPI->zSafetyBaseId.abyData4[5] * 256uL ) + ( (UAS_UInt32)pzSPI->zSafetyBaseId.abyData4[6] * 256u * 256uL ) + (UAS_UInt32)( pzSPI->zSafetyBaseId.abyData4[7] * 256uL * 256uL * 256uL ) );
  } /* if */
  else
  {
    pzResponseSpdu->zSpduId.dwPart1 = pzInputSAPI->zSafetyBaseId.dwData1 XOR UASDEF_SAFETY_PROVIDER_LEVEL_ID;
    pzResponseSpdu->zSpduId.dwPart2 = ( ( (UAS_UInt32)pzInputSAPI->zSafetyBaseId.wData2 ) + (UAS_UInt32)pzInputSAPI->zSafetyBaseId.wData3 * 256uL * 256uL )  XOR pzSPI->dwSafetyStructureSignature;
    pzResponseSpdu->zSpduId.dwPart3 =
    ( ( (UAS_UInt32)pzInputSAPI->zSafetyBaseId.abyData4[0] ) + ( (UAS_UInt32)pzInputSAPI->zSafetyBaseId.abyData4[1] * 256uL ) + ( (UAS_UInt32)pzInputSAPI->zSafetyBaseId.abyData4[2] * 256u * 256uL ) + (UAS_UInt32)( pzInputSAPI->zSafetyBaseId.abyData4[3] * 256uL * 256uL * 256uL ) ) XOR
    ( ( (UAS_UInt32)pzInputSAPI->zSafetyBaseId.abyData4[4] ) + ( (UAS_UInt32)pzInputSAPI->zSafetyBaseId.abyData4[5] * 256uL ) + ( (UAS_UInt32)pzInputSAPI->zSafetyBaseId.abyData4[6] * 256u * 256uL ) + (UAS_UInt32)( pzInputSAPI->zSafetyBaseId.abyData4[7] * 256uL * 256uL * 256uL ) );
  } /* else */

  /* Determine the ProviderID to be used in SPDU_ID */
  if ( 0uL EQ pzInputSAPI->dwSafetyProviderId )
  {
    pzResponseSpdu->zSpduId.dwPart3 = pzResponseSpdu->zSpduId.dwPart3 XOR pzSPI->dwSafetyProviderId;
  } /* if */
  else
  {
    pzResponseSpdu->zSpduId.dwPart3 = pzResponseSpdu->zSpduId.dwPart3 XOR pzInputSAPI->dwSafetyProviderId;
  } /* else */
  pzResponseSpdu->dwSafetyConsumerId = pzStateMachine->zRequestSpdu_i.dwSafetyConsumerId;
  pzResponseSpdu->dwMonitoringNumber = pzStateMachine->zRequestSpdu_i.dwMonitoringNumber;
  pzResponseSpdu->dwCrc = dwUASCRC_Calculate( pzResponseSpdu, pzStateMachine->pzInstanceData->wSafetyDataLength );

  /* Copy the NonSafetyData */
  wDataLength = pzStateMachine->pzInstanceData->wNonSafetyDataLength;
  for ( wIndex = 0u; wIndex < wDataLength; wIndex++ )
  {
    pzResponseSpdu->pbySerializedNonSafetyData[wIndex] = pzInputSAPI->pbySerializedNonSafetyData[wIndex];
  } /* for */
} /* end of function */


/**
  * Check the redundant (inverse) instance data
  * This function checks the redundant instance data to detect unexpected manipulations
  * in the "static values". Such manipulations can be caused e.g. by soft errors or
  * systematic faults..
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \return UASPROV_OK if ok, error code if error occured
  */
static UAS_UInt8 byUASPROV_CheckRedundantData
(
  const UASPROV_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  const UASPROV_StateMachine_type * const r_pzStateMachine  /**< Pointer to the redundant state machine data */
)
{
  UAS_UInt8  byRetVal = UASPROV_DEFAULT_ERR;

  if ( ( NULL EQ pzStateMachine   ) OR
       ( NULL EQ r_pzStateMachine ) )
  {
    byRetVal = UASPROV_POINTER_ERR;
  } /* if */
  else if ( UASRVAR_INVALID_USIGN32( pzStateMachine->dwParamFcs ) )
  {
    byRetVal = UASPROV_SOFT_ERR;
  } /* else if */
  else if ( UASRVAR_INVALID_PROVSTATE  ( pzStateMachine->nState ) )
  {
    byRetVal = UASPROV_SOFT_ERR;
  } /* else if */
  else if ( UASRVAR_INVALID_POINTER( pzStateMachine->pzInstanceData ) )
  {
    byRetVal = UASPROV_SOFT_ERR;
  } /* else if */
  else
  {
    UAS_UInt32  dwParamFcs = byUASPROV_CalculateFcs( pzStateMachine->pzInstanceData );
    if ( dwParamFcs NOT_EQ pzStateMachine->dwParamFcs )
    {
      byRetVal = UASPROV_SOFT_ERR;
    } /* if */
    else
    {
      byRetVal = UASPROV_OK;
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
static UAS_UInt32  byUASPROV_CalculateFcs
(
  const UAS_SafetyProvider_type * const pzInstance
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

    /* build FCS across pointer to SafetyProvider instance data */
    pbyParam = (UAS_UInt8 *)( &pzInstance );
    for ( byIndex = 0u; byIndex < sizeof (UAS_UInt8 *); byIndex++ )
    {
      dwParamFcs += pbyParam[byIndex];
    } /* for */

    /* build FCS across pointer to SafetyData in the SAPI */
    pbyParam = (UAS_UInt8 *)( &pzInstance->zInputSAPI.pbySerializedSafetyData);
    for ( byIndex = 0u; byIndex < sizeof (UAS_UInt8 *); byIndex++ )
    {
      dwParamFcs += pbyParam[byIndex];
    } /* for */

    /* build FCS across pointer to NonSafetyData in the SAPI */
    pbyParam = (UAS_UInt8 *)( &pzInstance->zInputSAPI.pbySerializedNonSafetyData );
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
    for ( byIndex = 0u; byIndex < sizeof (UAS_SafetyProviderSPI_type); byIndex++ )
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
  * This function resets the attributes of an SafetyProvider state machine instance.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in/out]  pzStateMachine   - Pointer to the primary state machine data
  * \param[in/out]  r_pzStateMachine - Pointer to the redundant state machine data
  * \param[in]      bAll - Determins if all attributes shall be resetted
  */
static void vUASPROV_ResetInstanceData
(
  UASPROV_StateMachine_type * const pzStateMachine,   /**< Pointer to the primary state machine data   */
  UASPROV_StateMachine_type * const r_pzStateMachine, /**< Pointer to the redundant state machine data */
  UAS_Bool bAll
)
{
  UASRVAR_SET_PROVSTATE ( pzStateMachine->nState, UASPROV_S0_INITIALIZE );
  UASRVAR_SET_USIGN32   ( pzStateMachine->zRequestSpdu_i.dwSafetyConsumerId, 0uL );
  UASRVAR_SET_USIGN32   ( pzStateMachine->zRequestSpdu_i.dwMonitoringNumber, 0uL );
  UASRVAR_SET_USIGN8    ( pzStateMachine->zRequestSpdu_i.byFlags, 0u );

  if ( 1u EQ bAll )
  {
    UASRVAR_SET_POINTER( pzStateMachine->pzInstanceData, NULL );
  } /* if */
} /* end of function */


/**
  * Check the instance data
  * This function checks the given instance data structure for the SafetyProvider instance.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in]  pzInstance   - Pointer to the instance data
  * \return UAS_PARAMETER_OK if parameters are valid, or disgnosis information if parameters are invalid
  */
static UAS_ParameterError_type nUASPROV_ValidInstanceData
(
  const UAS_SafetyProvider_type * const  pzInstance
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
  } /* if */

  /***** Check pointer to SafetyData *****/
  else if ( ( NULL EQ pzInstance->zResponseSPDU.pbySerializedSafetyData ) OR
            ( NULL EQ pzInstance->zInputSAPI.pbySerializedSafetyData ) )
  {
    nResult = UAS_INVALID_SAFETY_DATA_POINTER;
  } /* if */

  /***** Check pointer to NonSafetyData *****/
  else if ( ( NULL EQ pzInstance->zResponseSPDU.pbySerializedNonSafetyData ) OR
            ( NULL EQ pzInstance->zInputSAPI.pbySerializedNonSafetyData ) )
  {
    nResult = UAS_INVALID_NON_SAFETY_DATA_POINTER;
  } /* if */

  /***** No parameter error found *****/
  else
  {
    nResult = UAS_PARAMETER_OK;
  } /* else */

  return ( nResult );
} /* end of function */


/**
  * Check the SPI parameter
  * This function checks the SPI parameter of the SafetyProvider instance.
  * No plausibility checks for instance data will be done here, these checks were done
  * by the corresponding UAS function before calling this function.
  * \param[in]  pzSPI - Pointer to the SPI parameters
  * \return UAS_PARAMETER_OK if parameters are valid, or disgnosis information if parameters are invalid
  */
static UAS_ParameterError_type nUASPROV_ValidSPIParam
(
  const UAS_SafetyProviderSPI_type * const pzSPI
)
{
  UAS_ParameterError_type nResult = UAS_PARAMETER_OK;

  /*** check the BaseID ***/
  if ( ( 0uL EQ pzSPI->zSafetyBaseId.dwData1 ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.wData2 ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.wData3 ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.abyData4[0] ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.abyData4[1] ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.abyData4[2] ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.abyData4[3] ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.abyData4[4] ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.abyData4[5] ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.abyData4[6] ) AND
       ( 0uL EQ pzSPI->zSafetyBaseId.abyData4[7] ) )
  {
    nResult = UAS_INVALID_SAFETY_BASE_ID;
  } /* if */

  /*** check the SafetyProviderID ***/
  else if ( 0uL EQ pzSPI->dwSafetyProviderId )
  {
    nResult = UAS_INVALID_SAFETY_PROVIDER_ID;
  } /* if */

  /*** check the SafetyStructureSignature ***/
  else if ( 0uL EQ pzSPI->dwSafetyStructureSignature )
  {
    nResult = UAS_INVALID_SAFETY_STRUCTURE_SIGNATURE;
  } /* if */

  /*** No parameter error found ***/
  else
  {
    nResult = UAS_PARAMETER_OK;
  } /* else */

  return ( nResult );
} /* end of function */








