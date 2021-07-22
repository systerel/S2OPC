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
 * \brief Constant definitions for return codes of the OPC UA Safety Stack (UAS)
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Safety-Related: yes
 */

#ifndef INC_UASRVAL_H

#define INC_UASRVAL_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/** @name Module identifier in error codes
 *  These macros define the return values of the functions.
 */
/**@{*/
#define UASRVAL_UASMNGR_ID 0x10u /**< Safety instance manager           */
#define UASRVAL_UASPROV_ID 0x20u /**< SafetyProvider                    */
#define UASRVAL_UASCONS_ID 0x30u /**< SafetyConsumer                    */
#define UASRVAL_UASAPP_ID 0x40u  /**< Safety application                */
#define UASRVAL_UASUT_ID 0x50u   /**< Upper tester application          */
#define UASRVAL_UASPAR_ID 0x60u  /**< Device parameterization           */
#define UASRVAL_UASSYNC_ID 0x70u /**< Channel synchronization           */
#define UASRVAL_UAM_ID 0x80u     /**< OPC UA Mapper                     */
/**@}*/

/** @name Error type in error codes
 *  These macros define the return values of the functions.
 */
/**@{*/
#define UASRVAL_NO_ERROR 0x00u      /**< Function result OK                 */
#define UASRVAL_STATE_ERR 0x01u     /**< Function not allowed in that state */
#define UASRVAL_MEMORY_ERR 0x02u    /**< Not enough memory available        */
#define UASRVAL_POINTER_ERR 0x03u   /**< Pointer is NULL                    */
#define UASRVAL_FCT_PARAM_ERR 0x04u /**< Parameter is faulty                */
#define UASRVAL_DEVICE_ERR 0x05u    /**< Device error                       */
#define UASRVAL_NOT_IMPL 0x06u      /**< Function not implemented yet       */
#define UASRVAL_NO_DATA 0x07u       /**< No data available                  */
#define UASRVAL_COMM_ERR 0x08u      /**< Communication error                */
#define UASRVAL_TIMER_ERR 0x09u     /**< Timer error                        */
#define UASRVAL_SOFT_ERR 0x0au      /**< Soft error                         */
#define UASRVAL_SYNC_ERR 0x0bu      /**< Synchronization error              */
#define UASRVAL_DEFAULT_ERR 0x0fu   /**< Function didn't set return value   */
/**@}*/

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

#endif /* INC_UASRVAL_H */
