/**
 * OPC Foundation OPC UA Safety Stack
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief OPC UA SafetyProvider debug interface definition
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the debug output module for the SafetyProvider.
 *
 * Safety-Related: no
 */

#ifndef INC_UASPDBG_H

#define INC_UASPDBG_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_prov.h"
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
 * UAS debug output of SafetyProvider state info
 */
extern void vUASPDBG_PrintSafetyProviderState(
    /** Pointer to the state machine data */
    const UASPROV_StateMachine_type* const pzStateMachine);

/**
 * DBG output of transition name
 */
extern void vUASPDBG_PrintTransitionName(
    /** Name of the transition */
    const UAS_Char* const pszTransitionName);

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

#endif /* INC_UASPDBG_H */
