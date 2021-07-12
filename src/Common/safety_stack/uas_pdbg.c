/**
 * OPC Foundation OPC UA Safety Stack
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief OPC UA SafetyProvider debug functions
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the functions of the debug output module for the SafetyProvider.
 *
 * Safety-Related: no
 */


/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_def.h"

#ifdef UASDEF_DBG

#include "uas_type.h"
#include "uas.h"


/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_pdbg.h"

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/


/*--------------------------------------------------------------------------*/
/******************************** L O C A L *********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

#ifdef UASDEF_DBG_DETAILS

/** @name Length of process data.
 *  These macros define the limits for the state values.
 */
/**@{*/
#define UASPDBG_FIRST_STATE  ( UASPROV_S0_INITIALIZE )    /**< First state value of the SafetyProvider */
#define UASPDBG_LAST_STATE   ( UASPROV_S2_PREPARE_SPDU )  /**< Last state value of the SafetyProvider */
/**@}*/

/* This Makro is used to read the value of a single bit from a byte. */
#define /*lint -e(961)*/ UASPDBG_GET_BIT(byte,bit)   /*lint -e(835)*/(UAS_Bool)( ( ( byte ) BITAND (UAS_UInt8)( 0x01u << ( bit ) ) ) >> ( bit ) )

#endif /* DBG_FDL_DETAILS */

/*-------------*/
/*  T Y P E S  */
/*-------------*/


/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/


/*****************************************************************************
****  LOCAL DATA DECLARATIONS                                             ****
*****************************************************************************/

/*--------------------------------------------------------------------------*/
/************** I M P L E M E N T A T I O N   ( G L O B A L S ) *************/
/*--------------------------------------------------------------------------*/

/**
* UAS debug output of SafetyProvider state info
* This function prints state information of an UAS SafetyProvider instance
* for debugging purpose.
* \param[in/out]  pzStateMachine - Pointer to the state machine data
*/
void vUASPDBG_PrintSafetyProviderState
(
  const UASPROV_StateMachine_type * const pzStateMachine
)
{
  if  ( NULL NOT_EQ pzStateMachine )
  {

#ifdef UASDEF_DBG_DETAILS

    const UAS_Char *pcSProvStateName[] =
    {
      "S0_Initialization",
      "S1_WaitForRequest",
      "S2_PrepareSPDU",
    };

    UASDEF_LOG_DEBUG( "  ### Configuration parameters" );
    UASDEF_LOG_DEBUG( "        Handle                     = %u (0x%08X)", pzStateMachine->pzInstanceData->dwHandle, pzStateMachine->pzInstanceData->dwHandle );
    UASDEF_LOG_DEBUG( "        SafetyDataLength           = %u", pzStateMachine->pzInstanceData->wSafetyDataLength );
    UASDEF_LOG_DEBUG( "        NonSafetyDataLength        = %u", pzStateMachine->pzInstanceData->wNonSafetyDataLength );
    UASDEF_LOG_DEBUG( "  ### SPI parameters");
    UASDEF_LOG_DEBUG( "        SafetyProviderID           = %u (0x%08X)", pzStateMachine->pzInstanceData->zSPI.dwSafetyProviderId, pzStateMachine->pzInstanceData->zSPI.dwSafetyProviderId );
    UASDEF_LOG_DEBUG( "        SafetyBaseID               = 0x%08X-%08X-%08X-%08X",
      pzStateMachine->pzInstanceData->zSPI.zSafetyBaseId.dwPart1, pzStateMachine->pzInstanceData->zSPI.zSafetyBaseId.dwPart2, pzStateMachine->pzInstanceData->zSPI.zSafetyBaseId.dwPart3, pzStateMachine->pzInstanceData->zSPI.zSafetyBaseId.dwPart4 );
    UASDEF_LOG_DEBUG( "        SafetyProviderLevel        = %u", UASDEF_SAFETY_LEVEL );
    UASDEF_LOG_DEBUG( "        SafetyStructureSignature   = %u (0x%08X)", pzStateMachine->pzInstanceData->zSPI.dwSafetyStructureSignature, pzStateMachine->pzInstanceData->zSPI.dwSafetyStructureSignature );
    UASDEF_LOG_DEBUG( "        SafetyStructureSignatureVersion is not used in the UAS" );
    UASDEF_LOG_DEBUG( "        SafetyStructureIdentifier is not used in the UAS" );
    UASDEF_LOG_DEBUG( "        SafetyProviderDelay is not used in the UAS" );
    UASDEF_LOG_DEBUG( "  ### SAPI input parameters" );
    UASDEF_LOG_DATA ( "        SerializedSafetyData      ", pzStateMachine->pzInstanceData->wSafetyDataLength, pzStateMachine->pzInstanceData->zInputSAPI.pbySerializedSafetyData );
    UASDEF_LOG_DATA ( "        SerializedNonSafetyData   ", pzStateMachine->pzInstanceData->wNonSafetyDataLength, pzStateMachine->pzInstanceData->zInputSAPI.pbySerializedNonSafetyData );
    UASDEF_LOG_DEBUG( "        EnableTestMode             = %u", pzStateMachine->pzInstanceData->zInputSAPI.bEnableTestMode );
    UASDEF_LOG_DEBUG( "        OperatorAckProvider        = %u", pzStateMachine->pzInstanceData->zInputSAPI.bOperatorAckProvider );
    UASDEF_LOG_DEBUG( "        ActivateFSV                = %u", pzStateMachine->pzInstanceData->zInputSAPI.bActivateFsv );
    UASDEF_LOG_DEBUG( "        SafetyProviderID           = %u (0x%08X)", pzStateMachine->pzInstanceData->zInputSAPI.dwSafetyProviderId, pzStateMachine->pzInstanceData->zInputSAPI.dwSafetyProviderId );
    UASDEF_LOG_DEBUG( "        SafetyBaseID               = 0x%08X-%08X-%08X-%08X",
      pzStateMachine->pzInstanceData->zInputSAPI.zSafetyBaseId.dwPart1, pzStateMachine->pzInstanceData->zInputSAPI.zSafetyBaseId.dwPart2, pzStateMachine->pzInstanceData->zInputSAPI.zSafetyBaseId.dwPart3, pzStateMachine->pzInstanceData->zInputSAPI.zSafetyBaseId.dwPart4 );
    UASDEF_LOG_DEBUG( "  ### SAPI output parameters" );
    UASDEF_LOG_DEBUG( "        OperatorAckRequested       = %u", pzStateMachine->pzInstanceData->zOutputSAPI.bOperatorAckRequested );
    UASDEF_LOG_DEBUG( "        SafetyConsumerID           = %u (0x%08X)", pzStateMachine->pzInstanceData->zOutputSAPI.dwSafetyConsumerId, pzStateMachine->pzInstanceData->zOutputSAPI.dwSafetyConsumerId );
    UASDEF_LOG_DEBUG( "        MonitoringNumber           = %u (0x%08X)", pzStateMachine->pzInstanceData->zOutputSAPI.dwMonitoringNumber, pzStateMachine->pzInstanceData->zOutputSAPI.dwMonitoringNumber );
    UASDEF_LOG_DEBUG( "  ### RequestSPDU parameters" );
    UASDEF_LOG_DEBUG( "        SafetyConsumerID           = %u (0x%08X)", pzStateMachine->pzInstanceData->zRequestSPDU.dwSafetyConsumerId, pzStateMachine->pzInstanceData->zRequestSPDU.dwSafetyConsumerId );
    UASDEF_LOG_DEBUG( "        MonitoringNumber           = %u (0x%08X)", pzStateMachine->pzInstanceData->zRequestSPDU.dwMonitoringNumber, pzStateMachine->pzInstanceData->zRequestSPDU.dwMonitoringNumber );
    UASDEF_LOG_DEBUG( "        Flags                      = 0x%02X", pzStateMachine->pzInstanceData->zRequestSPDU.byFlags );
    UASDEF_LOG_DEBUG( "        Flags.CommunicationError   = %u", UASPDBG_GET_BIT( pzStateMachine->pzInstanceData->zRequestSPDU.byFlags, UAS_BITPOS_COMMUNICATION_ERROR ) );
    UASDEF_LOG_DEBUG( "        Flags.OperatorAckRequested = %u", UASPDBG_GET_BIT( pzStateMachine->pzInstanceData->zRequestSPDU.byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED ) );
    UASDEF_LOG_DEBUG( "        Flags.FSV_Activated        = %u", UASPDBG_GET_BIT( pzStateMachine->pzInstanceData->zRequestSPDU.byFlags, UAS_BITPOS_FSV_ACTIVATED ) );
    UASDEF_LOG_DEBUG( "  ### ResponseSPDU parameters" );
    UASDEF_LOG_DATA ( "        SerializedSafetyData      ", pzStateMachine->pzInstanceData->wSafetyDataLength, pzStateMachine->pzInstanceData->zResponseSPDU.pbySerializedSafetyData );
    UASDEF_LOG_DEBUG( "        Flags                      = 0x%02X", pzStateMachine->pzInstanceData->zResponseSPDU.byFlags );
    UASDEF_LOG_DEBUG( "        Flags.OperatorAckProvider  = %u", UASPDBG_GET_BIT( pzStateMachine->pzInstanceData->zResponseSPDU.byFlags, UAS_BITPOS_OPERATOR_ACK_PROVIDER ) );
    UASDEF_LOG_DEBUG( "        Flags.ActivateFSV          = %u", UASPDBG_GET_BIT( pzStateMachine->pzInstanceData->zResponseSPDU.byFlags, UAS_BITPOS_ACTIVATE_FSV ) );
    UASDEF_LOG_DEBUG( "        Flags.TestModeActivated    = %u", UASPDBG_GET_BIT( pzStateMachine->pzInstanceData->zResponseSPDU.byFlags, UAS_BITPOS_TEST_MODE_ACTIVATED ) );
    UASDEF_LOG_DEBUG( "        SPDU_ID                    = 0x%08X-%08X-%08X",
      pzStateMachine->pzInstanceData->zResponseSPDU.zSpduId.dwPart1, pzStateMachine->pzInstanceData->zResponseSPDU.zSpduId.dwPart2, pzStateMachine->pzInstanceData->zResponseSPDU.zSpduId.dwPart3 );
    UASDEF_LOG_DEBUG( "        SafetyConsumerId           = %u (0x%08X)", pzStateMachine->pzInstanceData->zResponseSPDU.dwSafetyConsumerId, pzStateMachine->pzInstanceData->zResponseSPDU.dwSafetyConsumerId );
    UASDEF_LOG_DEBUG( "        MonitoringNumber           = %u (0x%08X)", pzStateMachine->pzInstanceData->zResponseSPDU.dwMonitoringNumber, pzStateMachine->pzInstanceData->zResponseSPDU.dwMonitoringNumber );
    UASDEF_LOG_DEBUG( "        CRC                        = 0x%08X", pzStateMachine->pzInstanceData->zResponseSPDU.dwCrc );
    UASDEF_LOG_DATA ( "        SerializedNonSafetyData   ", pzStateMachine->pzInstanceData->wNonSafetyDataLength, pzStateMachine->pzInstanceData->zResponseSPDU.pbySerializedNonSafetyData );
    UASDEF_LOG_DEBUG( "  ### Internal items" );
    UASDEF_LOG_DEBUG( "        FCS over Parameter         = 0x%08X", pzStateMachine->dwParamFcs );
    if ( ( pzStateMachine->nState < UASPDBG_FIRST_STATE ) OR ( pzStateMachine->nState > UASPDBG_LAST_STATE ) )
    {
      UASDEF_LOG_DEBUG( "        State                      = %u (unknown)", pzStateMachine->nState );
    } /* else if */
    else
    {
      UASDEF_LOG_DEBUG( "        State                      = %u (%s)", pzStateMachine->nState, pcSProvStateName[( (UAS_Int16)pzStateMachine->nState - UASPDBG_FIRST_STATE ) + 1] );
    } /* else */
    UASDEF_LOG_DEBUG( "        CommDone                   = %u", pzStateMachine->pzInstanceData->bCommDone );
    UASDEF_LOG_DEBUG( "        AppDone                    = %u", pzStateMachine->pzInstanceData->bAppDone );
    UASDEF_LOG_DEBUG( "" );

#else /* UASDEF_DBG_DETAILS */

    UASDEF_LOG_DEBUG ( "        State: 0x%02X", pzStateMachine->nState );

#endif /* if UASDEF_DBG_DETAILS */

  } /* if */

} /* end of function */


/**
 * DBG output of transition name
 * This function prints parameter name of a fired transition for debugging purpose.
 * \param[in]  pszTransitionName - Transition name
*/
void vUASPDBG_PrintTransitionName
(
  const UAS_Char * const pszTransitionName
)
{
  static UAS_UInt16  uTransitionCount = 0u;

  uTransitionCount++;

#ifdef UASDEF_DBG_DETAILS

  UASDEF_LOG_DEBUG("  ###%06d: transition %s executed", uTransitionCount, pszTransitionName);

#else /* UASDEF_DBG_DETAILS */

    UASDEF_LOG_DEBUG("%06d: %s", uTransitionCount, pszTransitionName);

#endif /* if UASDEF_DBG_DETAILS */

} /* end of function */


/*--------------------------------------------------------------------------*/
/*************** I M P L E M E N T A T I O N   ( L O C A L S ) **************/
/*--------------------------------------------------------------------------*/



#endif /* ifdef UASDEF_DBG */

/* end of file */
