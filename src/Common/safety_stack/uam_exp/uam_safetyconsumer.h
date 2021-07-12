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
 * \brief Interface of the SafetyConsumer Mapper
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the SafetyConsumer Mapper.
 *
 * Safety-Related: no
 */

#ifndef __UAM_SAFETYCONSUMER_H
#define __UAM_SAFETYCONSUMER_H

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
     * \brief Function type for setting the parameters of the SafetyConsumer Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSafetyConsumerParam_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        const UAM_SafetyConsumerParam_type* const
            pzParam /**< [in] Pointer to the parameter set of the SafetyConsumer Mapper */
    );

    /**
     * \brief Function type for getting the ResponseSPDU of a SafetyConsumer instance (SafetyConsumer Mapper)
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_GetResponseSPDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        uint16* const pwLengthOfSafetyData,     /**< [in,out] input (Expected) length of the serialized safety-related
                                                   application data, return the actual length cached/configured in mapper */
        uint16* const
            pwLengthOfNonSafetyData, /**< [in,out] input (Expected) length of the serialized non-safety application
                                        data, return the actual length cached/configured in mapper */
        UAS_ResponseSpdu_type* const pzSpdu /**< [out] Pointer to the ResponseSPDU */
    );

    /**
     * \brief Function type for setting the RequestSPDU of a SafetyConsumer instance (SafetyConsumer Mapper)
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetRequestSPDU_ftype(
        const UAM_InstanceId_type dwInstanceId,  /**< [in] Instance ID */
        const UAS_RequestSpdu_type* const pzSpdu /**< [in] Pointer to the RequestSPDU */
    );

    /*---------------------*/
    /*  F U N C T I O N S  */
    /*---------------------*/

#ifdef _WIN32
    /**
     * Set the parameters of the SafetyConsumer Mapper
     */
    UAM_SetSafetyConsumerParam_ftype UAM_SetSafetyConsumerParam;

    /**
     * Get the ResponseSPDU of a SafetyConsumer instance (SafetyConsumer Mapper)
     */
    UAM_GetResponseSPDU_ftype UAM_GetResponseSPDU;

    /**
     * Set the RequestSPDU of a SafetyConsumer instance (SafetyConsumer Mapper)
     */
    UAM_SetRequestSPDU_ftype UAM_SetRequestSPDU;
#endif
#ifdef __GNUC__
    /**
     * Function  for setting the parameters of the SafetyConsumer Mapper
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetSafetyConsumerParam(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        const UAM_SafetyConsumerParam_type* const
            pzParam /**< Pointer to the parameter set of the SafetyConsumer Mapper */
    );

    /**
     * Get the ResponseSPDU of a SafetyConsumer instance (SafetyConsumer Mapper)
     */
    UAM_API UAM_ErrorCode WINAPI UAM_GetResponseSPDU(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        uint16* const pwLengthOfSafetyData, /**< (Expected) length of the serialized safety-related application data */
        uint16* const pwLengthOfNonSafetyData, /**< (Expected) length of the serialized non-safety application data */
        UAS_ResponseSpdu_type* const pzSpdu    /**< Pointer to the ResponseSPDU */
    );

    /**
     * Set the RequestSPDU of a SafetyConsumer instance (SafetyConsumer Mapper)
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetRequestSPDU(
        const UAM_InstanceId_type dwInstanceId,  /**< Instance ID */
        const UAS_RequestSpdu_type* const pzSpdu /**< Pointer to the RequestSPDU */
    );
#endif

    /*---------------------*/
    /*  V A R I A B L E S  */
    /*---------------------*/

#ifdef __cplusplus
}
#endif /* extern C */

#endif /* __UAM_SAFETYCONSUMER_H */
