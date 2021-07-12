/**
 * OPC Foundation OPC UA Safety Mapper
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Tianzhe Yu
 *    Copyright 2021 (c) Matthias Riedl
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief Interface of the SafetyProvider Mapper
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the SafetyProvider Mapper.
 *
 * Safety-Related: no
 */

#ifndef __UAM_SAFETYPROVIDER_H
#define __UAM_SAFETYPROVIDER_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uam_types.h"

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

#ifdef __cplusplus
extern "C"
{
#endif

    /*-------------*/
    /*  T Y P E S  */
    /*-------------*/

    /**
     * \brief Function type for setting the parameters of the SafetyProvider Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSafetyProviderParam_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        const UAM_SafetyProviderParam_type* const
            pzParam /**< [in] Pointer to the parameter set of the SafetyProvider Mapper */
    );

    /**
     * \brief Function type for getting the RequestSPDU of a SafetyProvider instance (SafetyProvider Mapper),
     *        the RequestSPDU will be cached in the mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_GetRequestSPDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        UAS_RequestSpdu_type* const pzSpdu      /**< [in] Pointer to the RequestSPDU */
    );

    /**
     * \brief Function type for setting the ResponseSPDU of a SafetyProvider instance (SafetyProvider Mapper)
     * Safetyprovider write responseSPU into mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetResponseSPDU_ftype(
        const UAM_InstanceId_type dwInstanceId,   /**< [in] Instance ID */
        uint16 wLengthOfSafetyData,               /**< [in] Length of the serialized safety-related application data */
        uint16 wLengthOfNonSafetyData,            /**< [in] Length of the serialized non-safety application data */
        const UAS_ResponseSpdu_type* const pzSpdu /**< [in] Pointer to the ResponseSPDU */
    );

    /*---------------------*/
    /*  F U N C T I O N S  */
    /*---------------------*/

#ifdef _WIN32
    /**
     * Set the parameters of the SafetyProvider Mapper
     */
    UAM_SetSafetyProviderParam_ftype UAM_SetSafetyProviderParam;

    /**
     * Get the RequestSPDU of a SafetyProvider instance (SafetyProvider Mapper)
     *
     */
    UAM_GetRequestSPDU_ftype UAM_GetRequestSPDU;

    /**
     * Set the ResponseSPDU of a SafetyProvider instance (SafetyProvider Mapper)
     */
    UAM_SetResponseSPDU_ftype UAM_SetResponseSPDU;
#endif
#ifdef __GNUC__
    /**
     * Set the parameters of the SafetyProvider Mapper
     */
    UAM_API UAM_ErrorCode UAM_SetSafetyProviderParam(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        const UAM_SafetyProviderParam_type* const
            pzParam /**< Pointer to the parameter set of the SafetyProvider Mapper */
    );
    /**
     * Get the RequestSPDU of a SafetyProvider instance (SafetyProvider Mapper)
     *
     */
    UAM_API UAM_ErrorCode UAM_GetRequestSPDU(const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
                                             UAS_RequestSpdu_type* const pzSpdu      /**< Pointer to the RequestSPDU */
    );
    /**
     * Set the ResponseSPDU of a SafetyProvider instance (SafetyProvider Mapper)
     */
    UAM_API UAM_ErrorCode UAM_SetResponseSPDU(
        const UAM_InstanceId_type dwInstanceId,   /**< Instance ID */
        uint16 wLengthOfSafetyData,               /**< Length of the serialized safety-related application data */
        uint16 wLengthOfNonSafetyData,            /**< Length of the serialized non-safety application data */
        const UAS_ResponseSpdu_type* const pzSpdu /**< Pointer to the ResponseSPDU */
    );

#endif

    /*---------------------*/
    /*  V A R I A B L E S  */
    /*---------------------*/

#ifdef __cplusplus
}
#endif /* extern C */

#endif /* __UAM_SAFETYPROVIDER_H */
