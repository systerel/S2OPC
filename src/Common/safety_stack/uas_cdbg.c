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
 * \brief OPC UA SafetyConsumer debug functions
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the functions of the debug output module for the SafetyConsumer.
 *
 * Safety-Related: no
 */

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_def.h"

#ifdef UASDEF_DBG

#include "uas.h"
#include "uas_type.h"

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_cdbg.h"

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
#define UASCDBG_FIRST_STATE (UASCONS_S10_INITIALIZE)         /**< First state value of the SafetyConsumer */
#define UASCDBG_LAST_STATE (UASCONS_S18_PROVIDE_SAFETY_DATA) /**< Last state value of the SafetyConsumer */
/**@}*/

/* This Makro is used to read the value of a single bit from a byte. */
#define /*lint -e(961)*/ UASCDBG_GET_BIT(byte, bit) /*lint -e(835)*/ \
    (UAS_Bool)(((byte) BITAND(UAS_UInt8)(0x01u << (bit))) >> (bit))

#endif /* UASDEF_DBG_DETAILS */

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/**
 * UAS debug output of BaseID.
 */
void vUASCDBG_PrintBaseId(const UAS_Char* pszName,     /**< name of the BaseID parameter */
                          const UAS_GUID_type* wBaseId /**< pointer to the BaseID parameter */
);

/**
 * querying the error type string
 */
const UAS_Char* vUASCDBG_SafetyDiagnosticIdToString(
    const UAS_SafetyDiagIdentifier_type nSafetyDiagnosticId /**< SafetyDiagnosticID */
);

/*****************************************************************************
****  LOCAL DATA DECLARATIONS                                             ****
*****************************************************************************/

/*--------------------------------------------------------------------------*/
/************** I M P L E M E N T A T I O N   ( G L O B A L S ) *************/
/*--------------------------------------------------------------------------*/

/**
 * UAS debug output of SafetyConsumer state info
 * This function prints state information of an UAS SafetyConsumer instance
 * for debugging purpose.
 * \param[in/out]  pzInstance - Pointer to the state machine data
 */
void vUASCDBG_PrintSafetyConsumerState(const UASCONS_StateMachine_type* const pzStateMachine)
{
    if (NULL NOT_EQ pzStateMachine)
    {
#ifdef UASDEF_DBG_DETAILS

        const UAS_Char* pcSConsStateName[] = {
            "S10_Initialization",     "S11_Wait_for_(Re)Start", "S12_initialize_MNR",    "S13_PrepareRequest",
            "S14_WaitForChangedSPDU", "S15_CRCCheckSPDU",       "S16_CheckResponseSPDU", "S17_Error",
            "S18_ProvideSafetyData",
        };
        UAS_SafetyConsumer_type* pzInstanceData = pzStateMachine->pzInstanceData;

        UASDEF_LOG_DEBUG("  ### Configuration parameters");
        UASDEF_LOG_DEBUG("        Handle                       = %u (0x%08X)", pzInstanceData->dwHandle,
                         pzInstanceData->dwHandle);
        UASDEF_LOG_DEBUG("        SafetyDataLength             = %u", pzInstanceData->wSafetyDataLength);
        UASDEF_LOG_DEBUG("        NonSafetyDataLength          = %u", pzInstanceData->wNonSafetyDataLength);
        UASDEF_LOG_DEBUG("  ### SPI parameters");
        UASDEF_LOG_DEBUG("        SafetyProviderID             = %u (0x%08X)", pzInstanceData->zSPI.dwSafetyProviderId,
                         pzInstanceData->zSPI.dwSafetyProviderId);
        vUASCDBG_PrintBaseId("        SafetyBaseID                ", &pzInstanceData->zSPI.zSafetyBaseId);
        UASDEF_LOG_DEBUG("        SafetyConsumerID             = %u (0x%08X)", pzInstanceData->zSPI.dwSafetyConsumerId,
                         pzInstanceData->zSPI.dwSafetyConsumerId);
        UASDEF_LOG_DEBUG("        SafetyProviderLevel          = %u", pzInstanceData->zSPI.bySafetyProviderLevel);
        UASDEF_LOG_DEBUG("        SafetyStructureSignature     = %u (0x%08X)",
                         pzInstanceData->zSPI.dwSafetyStructureSignature,
                         pzInstanceData->zSPI.dwSafetyStructureSignature);
        UASDEF_LOG_DEBUG("        SafetyStructureSignatureVersion is not used in the UAS");
        UASDEF_LOG_DEBUG("        SafetyStructureIdentifier is not used in the UAS");
        UASDEF_LOG_DEBUG("        SafetyConsumerTimeout        = %u", pzInstanceData->zSPI.dwSafetyConsumerTimeout);
        UASDEF_LOG_DEBUG("        SafetyOperatorAckNecessary   = %u", pzInstanceData->zSPI.bSafetyOperatorAckNecessary);
        UASDEF_LOG_DEBUG("        SafetyErrorIntervalLimit     = %u", pzInstanceData->zSPI.wSafetyErrorIntervalLimit);
        UASDEF_LOG_DEBUG("  ### SAPI input parameters");
        UASDEF_LOG_DEBUG("        Enable                       = %u", pzInstanceData->zInputSAPI.bEnable);
        UASDEF_LOG_DEBUG("        OperatorAckConsumer          = %u", pzInstanceData->zInputSAPI.bOperatorAckConsumer);
        UASDEF_LOG_DEBUG("        SafetyProviderID             = %u (0x%08X)",
                         pzInstanceData->zInputSAPI.dwSafetyProviderId, pzInstanceData->zInputSAPI.dwSafetyProviderId);
        vUASCDBG_PrintBaseId("        SafetyBaseID                ", &pzInstanceData->zSPI.zSafetyBaseId);
        UASDEF_LOG_DEBUG("        SafetyBaseID                ", &pzInstanceData->zInputSAPI.zSafetyBaseId);
        UASDEF_LOG_DEBUG("        SafetyConsumerID             = %u (0x%08X)",
                         pzInstanceData->zInputSAPI.dwSafetyConsumerId, pzInstanceData->zInputSAPI.dwSafetyConsumerId);
        UASDEF_LOG_DEBUG("  ### SAPI output parameters");
        UASDEF_LOG_DATA("        SerializedSafetyData        ", pzInstanceData->wSafetyDataLength,
                        pzInstanceData->zOutputSAPI.pbySerializedSafetyData);
        UASDEF_LOG_DATA("        SerializedNonSafetyData     ", pzInstanceData->wNonSafetyDataLength,
                        pzInstanceData->zOutputSAPI.pbySerializedNonSafetyData);
        UASDEF_LOG_DEBUG("        FSV_Activated                = %u", pzInstanceData->zOutputSAPI.bFsvActivated);
        UASDEF_LOG_DEBUG("        OperatorAckRequested         = %u",
                         pzInstanceData->zOutputSAPI.bOperatorAckRequested);
        UASDEF_LOG_DEBUG("        OperatorAckProvider          = %u", pzInstanceData->zOutputSAPI.bOperatorAckProvider);
        UASDEF_LOG_DEBUG("        TestModeActivated            = %u", pzInstanceData->zOutputSAPI.bTestModeActivated);
        UASDEF_LOG_DEBUG("  ### DI parameters");
        UASDEF_LOG_DEBUG("        SafetyDiagnosticID           = 0x%04X (%s)", pzInstanceData->zDI.nSafetyDiagnosticId,
                         vUASCDBG_SafetyDiagnosticIdToString(pzInstanceData->zDI.nSafetyDiagnosticId));
        UASDEF_LOG_DEBUG("        Permanent                    = %u", pzInstanceData->zDI.bPermanent);
        UASDEF_LOG_DEBUG("  ### RequestSPDU parameters");
        UASDEF_LOG_DEBUG("        SafetyConsumerID             = %u (0x%08X)",
                         pzInstanceData->zRequestSPDU.dwSafetyConsumerId,
                         pzInstanceData->zRequestSPDU.dwSafetyConsumerId);
        UASDEF_LOG_DEBUG("        MonitoringNumber             = %u (0x%08X)",
                         pzInstanceData->zRequestSPDU.dwMonitoringNumber,
                         pzInstanceData->zRequestSPDU.dwMonitoringNumber);
        UASDEF_LOG_DEBUG("        Flags                        = 0x%02X", pzInstanceData->zRequestSPDU.byFlags);
        UASDEF_LOG_DEBUG("        Flags.CommunicationError     = %u",
                         UASCDBG_GET_BIT(pzInstanceData->zRequestSPDU.byFlags, UAS_BITPOS_COMMUNICATION_ERROR));
        UASDEF_LOG_DEBUG("        Flags.OperatorAckRequested   = %u",
                         UASCDBG_GET_BIT(pzInstanceData->zRequestSPDU.byFlags, UAS_BITPOS_OPERATOR_ACK_REQUESTED));
        UASDEF_LOG_DEBUG("        Flags.FSV_Activated          = %u",
                         UASCDBG_GET_BIT(pzInstanceData->zRequestSPDU.byFlags, UAS_BITPOS_FSV_ACTIVATED));
        UASDEF_LOG_DEBUG("  ### ResponseSPDU parameters");
        UASDEF_LOG_DATA("        SerializedSafetyData        ", pzInstanceData->wSafetyDataLength,
                        pzInstanceData->zResponseSPDU.pbySerializedSafetyData);
        UASDEF_LOG_DEBUG("        Flags                        = 0x%02X", pzInstanceData->zResponseSPDU.byFlags);
        UASDEF_LOG_DEBUG("        Flags.OperatorAckProvider    = %u",
                         UASCDBG_GET_BIT(pzInstanceData->zResponseSPDU.byFlags, UAS_BITPOS_OPERATOR_ACK_PROVIDER));
        UASDEF_LOG_DEBUG("        Flags.ActivateFSV            = %u",
                         UASCDBG_GET_BIT(pzInstanceData->zResponseSPDU.byFlags, UAS_BITPOS_ACTIVATE_FSV));
        UASDEF_LOG_DEBUG("        Flags.TestModeActivated      = %u",
                         UASCDBG_GET_BIT(pzInstanceData->zResponseSPDU.byFlags, UAS_BITPOS_TEST_MODE_ACTIVATED));
        UASDEF_LOG_DEBUG("        SPDU_ID                      = 0x%08X-%08X-%08X",
                         pzInstanceData->zResponseSPDU.zSpduId.dwPart1, pzInstanceData->zResponseSPDU.zSpduId.dwPart2,
                         pzInstanceData->zResponseSPDU.zSpduId.dwPart3);
        UASDEF_LOG_DEBUG("        SafetyConsumerId             = %u (0x%08X)",
                         pzInstanceData->zResponseSPDU.dwSafetyConsumerId,
                         pzInstanceData->zResponseSPDU.dwSafetyConsumerId);
        UASDEF_LOG_DEBUG("        MonitoringNumber             = %u (0x%08X)",
                         pzInstanceData->zResponseSPDU.dwMonitoringNumber,
                         pzInstanceData->zResponseSPDU.dwMonitoringNumber);
        UASDEF_LOG_DEBUG("        CRC                          = %u (0x%08X)", pzInstanceData->zResponseSPDU.dwCrc,
                         pzInstanceData->zResponseSPDU.dwCrc);
        UASDEF_LOG_DATA("        SerializedNonSafetyData", pzInstanceData->wNonSafetyDataLength,
                        pzInstanceData->zResponseSPDU.pbySerializedNonSafetyData);
        UASDEF_LOG_DEBUG("  ### Internal items");
        if ((pzStateMachine->nState < UASCDBG_FIRST_STATE) OR(pzStateMachine->nState > UASCDBG_LAST_STATE))
        {
            UASDEF_LOG_DEBUG("        State                        = %u (unknown)", pzStateMachine->nState);
        } /* else if */
        else
        {
            UASDEF_LOG_DEBUG("        State                        = %u (%s)", pzStateMachine->nState,
                             pcSConsStateName[(UAS_Int16) pzStateMachine->nState - UASCDBG_FIRST_STATE]);
        } /* else */
        UASDEF_LOG_DEBUG("        FaultReqOA_i                 = %u", pzStateMachine->bFaultReqOA_i);
        UASDEF_LOG_DEBUG("        OperatorAckConsumerAllowed_i = %u", pzStateMachine->bOperatorAckConsumerAllowed_i);
        UASDEF_LOG_DEBUG("        MNR_i                        = %u (0x%08X)", pzStateMachine->dwMNR_i,
                         pzStateMachine->dwMNR_i);
        UASDEF_LOG_DEBUG("        prevMNR_i                    = %u (0x%08X)", pzStateMachine->dwPrevMNR_i,
                         pzStateMachine->dwPrevMNR_i);
        UASDEF_LOG_DEBUG("        ConsumerID_i                 = %u (0x%08X)", pzStateMachine->dwConsumerID_i,
                         pzStateMachine->dwConsumerID_i);
        UASDEF_LOG_DEBUG("        CRCCheck_i                   = %u", pzStateMachine->bCRCCheck_i);
        UASDEF_LOG_DEBUG("        SPDUCheck_i                  = %u", pzStateMachine->bSPDUCheck_i);
        UASDEF_LOG_DEBUG("        SPDU_ID_i                    = 0x%08X-%08X-%08X", pzStateMachine->zSPDUID_i.dwPart1,
                         pzStateMachine->zSPDUID_i.dwPart2, pzStateMachine->zSPDUID_i.dwPart3);
        UASDEF_LOG_DEBUG("        PrevActivateFSV_i            = %u", pzStateMachine->bPrevActivateFSV_i);
        UASDEF_LOG_DEBUG("        ConsumerTimerExpired         = %u", pzStateMachine->bConsumerTimerExpired);
        UASDEF_LOG_DEBUG("        bErrorIntervalTimerExpired   = %u", pzStateMachine->bErrorIntervalTimerExpired);
        UASDEF_LOG_DEBUG("        CommDone                     = %u", pzInstanceData->bCommDone);
        UASDEF_LOG_DEBUG("        AppDone                      = %u", pzInstanceData->bAppDone);
        UASDEF_LOG_DEBUG("");

#else /* UASDEF_DBG_DETAILS */

        UASDEF_LOG_DEBUG("        State: 0x%02X", pzStateMachine->nState);

#endif /* if UASDEF_DBG_DETAILS */

    } /* if */

} /* end of function */

/**
 * DBG output of transition name
 * This function prints parameter name of a fired transition for debugging purpose.
 * \param[in]  pszTransitionName - Transition name
 */
void vUASCDBG_PrintTransitionName(const UAS_Char* const pszTransitionName)
{
    static UAS_UInt16 uTransitionCount = 0u;

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

/**
 * UAS debug output of BaseID.
 * This function prints a BaseID parameter.
 * \param[in]  pszName - name of the BaseID parameter.
 * \param[in]  zBaseId - pointer to the BaseID parameter.
 */
void vUASCDBG_PrintBaseId(const UAS_Char* pszName, const UAS_GUID_type* pzBaseId)
{
    /* Guid values to be represented in form <Data1>-<Data2>-<Data3>-<Data4[0:1]>-<Data4[2:7]> */
    UASDEF_LOG_DEBUG("%s = %08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", pszName, pzBaseId->dwData1,
                     pzBaseId->wData2, pzBaseId->wData3, pzBaseId->abyData4[0], pzBaseId->abyData4[1],
                     pzBaseId->abyData4[2], pzBaseId->abyData4[3], pzBaseId->abyData4[4], pzBaseId->abyData4[5],
                     pzBaseId->abyData4[6], pzBaseId->abyData4[7]);
}

/**
 * querying the error type string
 * This function returns the error type string of the SafetyDiagnosticID.
 * \param[in]  nSafetyDiagnosticId - SafetyDiagnosticID.
 */
const UAS_Char* vUASCDBG_SafetyDiagnosticIdToString(const UAS_SafetyDiagIdentifier_type nSafetyDiagnosticId)
{
    UAS_Char* pszRetValue = NULL;

    switch (nSafetyDiagnosticId)
    {
    case UAS_DIAG_OK:
    {
        pszRetValue = "no error";
        break;
    } /* case */
    case UAS_DIAG_SD_ID_ERR_IGN:
    {
        pszRetValue = "The SafetyConsumer has discarded a message due to an incorrect ID.";
        break;
    } /* case */
    case UAS_DIAG_SD_ID_ERR_OA_1:
    {
        pszRetValue =
            "Mismatch of SafetyBaseID. The SafetyConsumer has switched to fail-safe substitute values due to an "
            "incorrect ID. Operator acknowledgment is required.";
        break;
    } /* case */
    case UAS_DIAG_SD_ID_ERR_OA_2:
    {
        pszRetValue =
            "Mismatch of SafetyProviderID. The SafetyConsumer has switched to fail-safe substitute values due to an "
            "incorrect ID. Operator acknowledgment is required.";
        break;
    } /* case */
    case UAS_DIAG_SD_ID_ERR_OA_3:
    {
        pszRetValue =
            "Mismatch of safety data structure or identifier. The SafetyConsumer has switched to fail-safe substitute "
            "values due to an incorrect ID. Operator acknowledgment is required.";
        break;
    } /* case */
    case UAS_DIAG_SD_ID_ERR_OA_4:
    {
        pszRetValue =
            "Mismatch of SafetyProviderLevel. The SafetyConsumer has switched to fail-safe substitute values due to an "
            "incorrect ID. Operator acknowledgment is required.";
        break;
    } /* case */
    case UAS_DIAG_CRC_ERR_IGN:
    {
        pszRetValue = "The SafetyConsumer has discarded a message due to a CRC error (data corruption).";
        break;
    } /* case */
    case UAS_DIAG_CRC_ERR_OA:
    {
        pszRetValue =
            "The SafetyConsumer has switched to fail-safe substitute values due to a CRC error (data corruption). "
            "Operator acknowledgment is required.";
        break;
    } /* case */
    case UAS_DIAG_COID_ERR_IGN:
    {
        pszRetValue = "The SafetyConsumer has discarded a message due to an incorrect ConsumerID.";
        break;
    } /* case */
    case UAS_DIAG_COID_ERR_OA:
    {
        pszRetValue =
            "The SafetyConsumer has switched to fail-safe substitute values due to an incorrect consumer ID. Operator "
            "acknowledgment is required.";
        break;
    } /* case */
    case UAS_DIAG_MNR_ERR_IGN:
    {
        pszRetValue = "The SafetyConsumer has discarded a message due to an incorrect monitoring number.";
        break;
    } /* case */
    case UAS_DIAG_MNR_ERR_OA:
    {
        pszRetValue =
            "The SafetyConsumer has switched to fail-safe substitute values due to an incorrect monitoring number. "
            "Operator acknowledgment is required.";
        break;
    } /* case */
    case UAS_DIAG_COMM_ERR_TO:
    {
        pszRetValue = "The SafetyConsumer has switched to fail-safe substitute values due to timeout.";
        break;
    } /* case */
    case UAS_DIAG_APPL_ERR_TO:
    {
        pszRetValue =
            "The SafetyConsumer has switched to fail-safe substitute values at the request of the safety application.";
        break;
    } /* case */
    case UAS_DIAG_FSV_REQUESTED:
    {
        pszRetValue =
            "The SafetyConsumer has switched to fail-safe substitute values at the request of the SafetyProvider. "
            "Operator acknowledgment is required.";
        break;
    } /* case */
    default:
    {
        pszRetValue = "unknown";
        break;
    } /* default */
    } /* switch */

    return (pszRetValue);
}

#endif /* ifdef UASDEF_DBG */

/* end of file */
