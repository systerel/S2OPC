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
 * \brief OPC UA Safety crc calculation definition.
 *
 * \date      2021-07-08
 * \revision  0.4
 * \status    in work
 *
 * Defines the functions of the OPC UA Safety crc calculation module.
 *
 * Safety-Related: yes
 */


/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_def.h"
#include "uas_type.h"


/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_crc.h"


/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/


/*--------------------------------------------------------------------------*/
/******************************** L O C A L *********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/* lint -e961 */  /* supress warning "Function-like macro defined: 'UASCRC_CALC'" because of
   determining the CRC calculation method (runtime or memory optimized) via different macro definitions */
/* lint -e960 */  /* supress warning "pointer arithmetic other than array indexing used" because of
   determining the CRC calculation method (runtime or memory optimized) via different macro definitions */

/** This macro calculates an 32 bit CRC value for an octet by means of the lookup table.
  * IN: r - Start value for the calculation
  * IN: v - Value for which the CRC shall be calculated
  */
#define UASCRC_CALC_CRC32(r,v) /*lint -e(960)*/ (dwUASCRC_CRC32Table[(((r)>>24) XOR (v)) BITAND 0xffu] XOR (UAS_UInt32)((r)<<8))


/*-------------*/
/*  T Y P E S  */
/*-------------*/


/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/*lint +e961 */  /* end of supress warning */
/*lint +e960 */  /* end of supress warning */


/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

/**
  * Lookup table for the 32 bit CRC calculation according to the OPC UA Safety specification
  */
static const UAS_UInt32 dwUASCRC_CRC32Table[0x100u] =
{
  /* 00-07 */ 0x00000000u, 0xF4ACFB13u, 0x1DF50D35u, 0xE959F626u, 0x3BEA1A6Au, 0xCF46E179u, 0x261F175Fu, 0xD2B3EC4Cu,
  /* 08-0f */ 0x77D434D4u, 0x8378CFC7u, 0x6A2139E1u, 0x9E8DC2F2u, 0x4C3E2EBEu, 0xB892D5ADu, 0x51CB238Bu, 0xA567D898u,
  /* 10-17 */ 0xEFA869A8u, 0x1B0492BBu, 0xF25D649Du, 0x06F19F8Eu, 0xD44273C2u, 0x20EE88D1u, 0xC9B77EF7u, 0x3D1B85E4u,
  /* 18-1f */ 0x987C5D7Cu, 0x6CD0A66Fu, 0x85895049u, 0x7125AB5Au, 0xA3964716u, 0x573ABC05u, 0xBE634A23u, 0x4ACFB130u,
  /* 20-27 */ 0x2BFC2843u, 0xDF50D350u, 0x36092576u, 0xC2A5DE65u, 0x10163229u, 0xE4BAC93Au, 0x0DE33F1Cu, 0xF94FC40Fu,
  /* 28-2f */ 0x5C281C97u, 0xA884E784u, 0x41DD11A2u, 0xB571EAB1u, 0x67C206FDu, 0x936EFDEEu, 0x7A370BC8u, 0x8E9BF0DBu,
  /* 30-37 */ 0xC45441EBu, 0x30F8BAF8u, 0xD9A14CDEu, 0x2D0DB7CDu, 0xFFBE5B81u, 0x0B12A092u, 0xE24B56B4u, 0x16E7ADA7u,
  /* 38-3f */ 0xB380753Fu, 0x472C8E2Cu, 0xAE75780Au, 0x5AD98319u, 0x886A6F55u, 0x7CC69446u, 0x959F6260u, 0x61339973u,
  /* 40-47 */ 0x57F85086u, 0xA354AB95u, 0x4A0D5DB3u, 0xBEA1A6A0u, 0x6C124AECu, 0x98BEB1FFu, 0x71E747D9u, 0x854BBCCAu,
  /* 48-4f */ 0x202C6452u, 0xD4809F41u, 0x3DD96967u, 0xC9759274u, 0x1BC67E38u, 0xEF6A852Bu, 0x0633730Du, 0xF29F881Eu,
  /* 50-57 */ 0xB850392Eu, 0x4CFCC23Du, 0xA5A5341Bu, 0x5109CF08u, 0x83BA2344u, 0x7716D857u, 0x9E4F2E71u, 0x6AE3D562u,
  /* 58-5f */ 0xCF840DFAu, 0x3B28F6E9u, 0xD27100CFu, 0x26DDFBDCu, 0xF46E1790u, 0x00C2EC83u, 0xE99B1AA5u, 0x1D37E1B6u,
  /* 60-67 */ 0x7C0478C5u, 0x88A883D6u, 0x61F175F0u, 0x955D8EE3u, 0x47EE62AFu, 0xB34299BCu, 0x5A1B6F9Au, 0xAEB79489u,
  /* 68-6f */ 0x0BD04C11u, 0xFF7CB702u, 0x16254124u, 0xE289BA37u, 0x303A567Bu, 0xC496AD68u, 0x2DCF5B4Eu, 0xD963A05Du,
  /* 70-77 */ 0x93AC116Du, 0x6700EA7Eu, 0x8E591C58u, 0x7AF5E74Bu, 0xA8460B07u, 0x5CEAF014u, 0xB5B30632u, 0x411FFD21u,
  /* 78-7f */ 0xE47825B9u, 0x10D4DEAAu, 0xF98D288Cu, 0x0D21D39Fu, 0xDF923FD3u, 0x2B3EC4C0u, 0xC26732E6u, 0x36CBC9F5u,
  /* 80-87 */ 0xAFF0A10Cu, 0x5B5C5A1Fu, 0xB205AC39u, 0x46A9572Au, 0x941ABB66u, 0x60B64075u, 0x89EFB653u, 0x7D434D40u,
  /* 88-8f */ 0xD82495D8u, 0x2C886ECBu, 0xC5D198EDu, 0x317D63FEu, 0xE3CE8FB2u, 0x176274A1u, 0xFE3B8287u, 0x0A977994u,
  /* 90-97 */ 0x4058C8A4u, 0xB4F433B7u, 0x5DADC591u, 0xA9013E82u, 0x7BB2D2CEu, 0x8F1E29DDu, 0x6647DFFBu, 0x92EB24E8u,
  /* 98-9f */ 0x378CFC70u, 0xC3200763u, 0x2A79F145u, 0xDED50A56u, 0x0C66E61Au, 0xF8CA1D09u, 0x1193EB2Fu, 0xE53F103Cu,
  /* a0-a7 */ 0x840C894Fu, 0x70A0725Cu, 0x99F9847Au, 0x6D557F69u, 0xBFE69325u, 0x4B4A6836u, 0xA2139E10u, 0x56BF6503u,
  /* a8-af */ 0xF3D8BD9Bu, 0x07744688u, 0xEE2DB0AEu, 0x1A814BBDu, 0xC832A7F1u, 0x3C9E5CE2u, 0xD5C7AAC4u, 0x216B51D7u,
  /* b0-b7 */ 0x6BA4E0E7u, 0x9F081BF4u, 0x7651EDD2u, 0x82FD16C1u, 0x504EFA8Du, 0xA4E2019Eu, 0x4DBBF7B8u, 0xB9170CABu,
  /* b8-bf */ 0x1C70D433u, 0xE8DC2F20u, 0x0185D906u, 0xF5292215u, 0x279ACE59u, 0xD336354Au, 0x3A6FC36Cu, 0xCEC3387Fu,
  /* c0-c7 */ 0xF808F18Au, 0x0CA40A99u, 0xE5FDFCBFu, 0x115107ACu, 0xC3E2EBE0u, 0x374E10F3u, 0xDE17E6D5u, 0x2ABB1DC6u,
  /* c8-cf */ 0x8FDCC55Eu, 0x7B703E4Du, 0x9229C86Bu, 0x66853378u, 0xB436DF34u, 0x409A2427u, 0xA9C3D201u, 0x5D6F2912u,
  /* d0-d7 */ 0x17A09822u, 0xE30C6331u, 0x0A559517u, 0xFEF96E04u, 0x2C4A8248u, 0xD8E6795Bu, 0x31BF8F7Du, 0xC513746Eu,
  /* d8-df */ 0x6074ACF6u, 0x94D857E5u, 0x7D81A1C3u, 0x892D5AD0u, 0x5B9EB69Cu, 0xAF324D8Fu, 0x466BBBA9u, 0xB2C740BAu,
  /* e0-e7 */ 0xD3F4D9C9u, 0x275822DAu, 0xCE01D4FCu, 0x3AAD2FEFu, 0xE81EC3A3u, 0x1CB238B0u, 0xF5EBCE96u, 0x01473585u,
  /* e8-ef */ 0xA420ED1Du, 0x508C160Eu, 0xB9D5E028u, 0x4D791B3Bu, 0x9FCAF777u, 0x6B660C64u, 0x823FFA42u, 0x76930151u,
  /* f0-f7 */ 0x3C5CB061u, 0xC8F04B72u, 0x21A9BD54u, 0xD5054647u, 0x07B6AA0Bu, 0xF31A5118u, 0x1A43A73Eu, 0xEEEF5C2Du,
  /* f8-ff */ 0x4B8884B5u, 0xBF247FA6u, 0x567D8980u, 0xA2D17293u, 0x70629EDFu, 0x84CE65CCu, 0x6D9793EAu, 0x993B68F9u,
};


/*--------------------------------------------------------------------------*/
/************** I M P L E M E N T A T I O N   ( G L O B A L S ) *************/
/*--------------------------------------------------------------------------*/

/**
 * CRC calculation over ResponseSPDU.
 * This function calculates a 32 bit CRC signature across the ResponseSPDU
 * according to the OPC UA Safety specification. If the calculation results
 * in a "0", the value is set to "1".
 * \param[in]  pzResponseSpdu - Pointer to the ResponseSPDU
 * \param[in]  wSafetyDataLen - Length of the serialized SafetyData
 * \return CRC signature value or 0 in case of invalid parameter.
 */

UAS_UInt32 dwUASCRC_Calculate
(
  const UAS_ResponseSpdu_type *pzResponseSpdu,
  const UAS_UInt16 wSafetyDataLen
)
{
  /* CRC value, has to be calcutated and returned. Value 0 indicates an error. */
  UAS_UInt32 dwCrc = 1uL;
  /* Index in the SPDU parameter */
  UAS_UInt16 wIndex = 0u;
  /* Octet in ulMnr */
  UAS_UInt8  byOctet = 0u;

  /* use only valid parameters */
  if ( NULL NOT_EQ pzResponseSpdu )
  {
    if ( NULL NOT_EQ pzResponseSpdu->pbySerializedSafetyData )
    {
      /* calculate crc over the STrailer */
      byOctet = (UAS_UInt8)( ( pzResponseSpdu->dwMonitoringNumber ) >> 24 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( ( pzResponseSpdu->dwMonitoringNumber ) >> 16 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( ( pzResponseSpdu->dwMonitoringNumber ) >> 8 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->dwMonitoringNumber );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->dwSafetyConsumerId >> 24 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->dwSafetyConsumerId >> 16 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->dwSafetyConsumerId >> 8 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->dwSafetyConsumerId );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart3 >> 24 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart3 >> 16 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart3 >> 8 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart3 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart2 >> 24 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart2 >> 16 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart2 >> 8 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart2 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart1 >> 24 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart1 >> 16 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart1 >> 8 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      byOctet = (UAS_UInt8)( pzResponseSpdu->zSpduId.dwPart1 );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, byOctet );
      dwCrc = UASCRC_CALC_CRC32( dwCrc, pzResponseSpdu->byFlags );
      /* calculate crc over SafetyData in reverse order */
      for ( wIndex = wSafetyDataLen; wIndex > 0u; wIndex-- )
      {
        dwCrc = UASCRC_CALC_CRC32( dwCrc, pzResponseSpdu->pbySerializedSafetyData[wIndex-1u] );
      } /* while */

      /* replace value 0 with 1 according to the OPS Ua Safety specification */
      if ( 0uL EQ dwCrc )
      {
        dwCrc = 1uL;
      } /* if */
    } /* if */
  } /* if */

  /* return the result */
  return ( dwCrc );

} /* end of function */



/*--------------------------------------------------------------------------*/
/*************** I M P L E M E N T A T I O N   ( L O C A L S ) **************/
/*--------------------------------------------------------------------------*/

/* end of file */
