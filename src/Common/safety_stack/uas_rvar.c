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
 * \brief OPC UA Safety redundant variable definition
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the functions for handling of redundant variables in the
 * OPC UA Safety stack
 *
 * Safety-Related: yes
 */


/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_def.h"
#include "uas_type.h"
#include "uas.h"
#include "uas_prov.h"
#include "uas_cons.h"


/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_rvar.h"


/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

/** Redundant (inverse) error count of the UAS timer module   */
UAS_UInt16  r_wUASTIME_ErrorCount = UASRVAR_MAX_ERROR_COUNT;

/** Redundant (inverse) Array of instance data sets for the SafetyProvider state machines */
UASPROV_StateMachine_type r_azUAS_ProvStateMachines[UASDEF_MAX_SAFETYPROVIDERS];

/** Redundant (inverse) Array of instance data sets for the SafetyConsumer state machines */
UASCONS_StateMachine_type r_azUAS_ConsStateMachines[UASDEF_MAX_SAFETYCONSUMERS];

/** Redundant (inverse) error code of the UAS manager  */
UAS_UInt8  r_byUAS_Error;


/*--------------------------------------------------------------------------*/
/******************************** L O C A L *********************************/
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
* Fill block of memory
*/
static void vUASRVAR_Memset
(
  void *pbyAddress,   /**< Pointer to the block of memory */
  UAS_UInt8 byValue,  /**< Value to be set                */
  UAS_UInt32 dwLength /**< Length of the block of memory  */
);

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/


/*--------------------------------------------------------------------------*/
/************** I M P L E M E N T A T I O N   ( G L O B A L S ) *************/
/*--------------------------------------------------------------------------*/

/**
* UAS redundant variable initialization
* This function initializes the redundant variables of the OPC UA Safety stack.
*/
void vUASRVAR_Init( void )
{
  UAS_UInt32  dwLength;

  /* Initialize redundant (inverse) error count of the FDTimer module */
  r_wUASTIME_ErrorCount = UASRVAR_MAX_ERROR_COUNT;

  /* Initialize redundant (inverse) UAS instance data sets */
  dwLength = sizeof( r_azUAS_ProvStateMachines );
  vUASRVAR_Memset( (void *)r_azUAS_ProvStateMachines, (UAS_UInt8)0xFF, dwLength);
  dwLength = sizeof( r_azUAS_ConsStateMachines );
  vUASRVAR_Memset( (void *)r_azUAS_ConsStateMachines, (UAS_UInt8)0xFF, dwLength );

  /* Redundant (inverse) error code of the FDL module  */
  r_byUAS_Error = (UAS_UInt8)0xFFu;

} /* end of function */


/*--------------------------------------------------------------------------*/
/*************** I M P L E M E N T A T I O N   ( L O C A L S ) **************/
/*--------------------------------------------------------------------------*/

/**
* Fill block of memory
* This function sets all bytes in a block of memory to a specified value.
* \param[out]  pbyAddress  - pointer to the block of memory
* \param[in]   byValue     - value to be set
* \param[in]   ulLength    - length of the block of memory
*/
static void vUASRVAR_Memset
(
  void *pbyAddress,
  UAS_UInt8 byValue,
  UAS_UInt32  ulLength
)
{
  UAS_UInt32  dwIndex = 0u;
  UAS_UInt8  *pbyTmp = (UAS_UInt8 *)pbyAddress;

  if ( NULL NOT_EQ pbyAddress)
  {
    for ( dwIndex = 0u; dwIndex < ulLength; dwIndex++)
    {
      /*lint -e(960) */  /* supress warning "pointer arithmetic other than array indexing used" because of
      initialization of redundant variable buffer with inverse values */
      pbyTmp[dwIndex] = byValue;
    } /* for */
  } /* if */
}

/* end of file */

