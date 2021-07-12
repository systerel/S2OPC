/**
 * OPC Foundation OPC UA Safety Stack
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief OPC UA Safety crc calculation interface definition.
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the OPC UA Safety crc calculation module.
 *
 * Safety-Related: yes
 */

#ifndef INC_UASCRC_H

#define INC_UASCRC_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_type.h"

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
 * 32 bit CRC calculation
 */
extern UAS_UInt32 dwUASCRC_Calculate(const UAS_ResponseSpdu_type* pzResponseSpdu, /**< Pointer to the ResponseSPDU */
                                     const UAS_UInt16 wSafetyDataLen /**< Length of the serialized SafetyData */
);

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

#endif /* INC_UASTIME_H */
