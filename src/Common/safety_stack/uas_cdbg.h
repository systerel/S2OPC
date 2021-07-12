/**
 * OPC Foundation OPC UA Safety Stack
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief OPC UA SafetyConsumer debug interface definition
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the debug output module for the SafetyConsumer.
 *
 * Safety-Related: no
 */

#ifndef INC_UASCDBG_H

#define INC_UASCDBG_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_cons.h"
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

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/**
 * Debug output of SafetyConsumer state infos
 */
extern void vUASCDBG_PrintSafetyConsumerState(
    /** Pointer to the state machine data */
    const UASCONS_StateMachine_type* const pzStateMachine);

/**
 * DBG output of transition name
 */
extern void vUASCDBG_PrintTransitionName(
    /** Name of the transition */
    const UAS_Char* const pszTransitionName);

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

#endif /* INC_UASCDBG_H */
