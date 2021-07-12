/**
 * OPC Foundation OPC UA Safety Stack
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief Static configuration of the OPC UA Safety Stack (UAS).
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Safety-Related: yes
 */

#ifndef INC_UASDEF_H

#define INC_UASDEF_H

/** UASDEF_DBG activates debug outputs */
#define UASDEF_DBG

#ifdef UASDEF_DBG

/** UASDEF_DBG_DETAILS activates detailed debug outputs */
#define UASDEF_DBG_DETAILS

/** UASDEF_DBG_TIMER activates debug outputs in timer functions */
#define UASDEF_DBG_TIMER

#endif /* ifdef UASDEF_DBG */

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

/** Include definitions for standard data types */
#include "uas_stdtypes.h"

/** Include definitions for return codes */
#include "uas_rval.h"

#ifdef UASDEF_DBG

/** Include definitions for debug outputs */
#include "uas_logitf.h"

#endif /* ifdef UASDEF_DBG */

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

#ifndef NULL

/** Value definition for null pointer */
#define NULL ((void*) 0)

#endif

/**
 * Safety level of the whole OPC UA Safety device implementation including this UAS
 * Allowed values: UAS_SAFETY_LEVEL_1
 *                 UAS_SAFETY_LEVEL_2
 *                 UAS_SAFETY_LEVEL_3
 *                 UAS_SAFETY_LEVEL_4
 */
#define UASDEF_SAFETY_LEVEL UAS_SAFETY_LEVEL_3

/**
 * SafetyProviderLevel_ID of the whole OPC UA Safety device implementation including this UAS.
 * The value has to be consistent with the value of UASDEF_SAFETY_LEVEL.
 * Allowed values: UAS_SAFETY_LEVEL_1_ID
 *                 UAS_SAFETY_LEVEL_2_ID
 *                 UAS_SAFETY_LEVEL_3_ID
 *                 UAS_SAFETY_LEVEL_4_ID
 */
#define UASDEF_SAFETY_PROVIDER_LEVEL_ID UAS_SAFETY_LEVEL_3_ID

/**
 * Maximum number of SafetyProvider instances
 * Allowed values: 1 to (2^16)-1 (depending on available mmemory)
 */
#define UASDEF_MAX_SAFETYPROVIDERS 50u

/**
 * Maximum number of SafetyConsumer instances
 * Allowed values: 1 to (2^16)-1 (depending on available mmemory)
 */
#define UASDEF_MAX_SAFETYCONSUMERS 50u

/**
 * Maximum process value length
 * Allowed values: 1 to 1500
 */
#define UASDEF_MAX_PV_LENGTH 1500u

/**
 * Configuration of CRC generation:
 * If UASDEF_CRC_CALC_WITH_LOOKUPTABLE is defined the CRC calculation will
 * be made by lookup table (runtime-optimized, requires more memory space).
 * Otherwise the CRC calculation will be made by formula.
 */
#define UASDEF_CRC_CALC_WITH_LOOKUPTABLE

/**
 * Implementation of dynamic address parameters in the SAPI
 * If UASDEF_DYNAMIC_ADDRESS_PARAM_IMPLEMENTED is defined the optional SAPI
 * parameters for dynamic addessing are enabled. It can be used for the
 * realization of dynamic systems with ... of temporary safety connections.
 * ATTENTION!!!
 * The UAS user is responsible for the dynamic connections.
 * The dynamic address parameters shall only be used if the re-adressing of
 * safety connections is protected by appropriate measures in the UAS application
 * The UAS contains no measures for that purpose.
 */
#define UASDEF_DYNAMIC_ADDRESS_PARAM_IMPLEMENTED

/**
 * Attach function to output debug messages
 */
#ifdef UASDEF_DBG

#define UASDEF_LOG_DEBUG LOG_DebugTrace
#define UASDEF_LOG_DATA LOG_DebugData

#endif /* ifdef DBG_FDL */

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

#endif /* INC_UASDEF_H */
