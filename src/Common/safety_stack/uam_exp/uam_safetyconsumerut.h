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
 * \brief Interface of the SafetyConsumerUT Mapper
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the SafetyConsumerUT Mapper.
 *
 * Safety-Related: no
 */

#ifndef __UAM_SAFETYCONSUMERUT_H
#define __UAM_SAFETYCONSUMERUT_H

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
     * \brief Function type for setting the parameters of the SafetyConsumerUT Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSafetyConsumerUTParam_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        const UAM_SafetyConsumerUTParam_type* const
            pzParam /**< [in] Pointer to the parameter set of the SafetyConsumerUT Mapper */
    );

    /**
     * \brief Function type for browsing the parameters of the corresponding SafetyConsumerUTA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_BrowseSafetyConsumerUTAParam_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        UAM_SafetyConsumerUTAParam_type* const
            pzParam /**< [out] Pointer to the parameter set of the SafetyConsumerUTA Mapper */
    );

    /**
     * \brief Function type for getting the Test Control Data Unit with SafetyConsumerSAPI Outputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_GetSConsOutputTCDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        uint16* const pwLengthOfSafetyData,     /**< [in,out] input (Expected) length of the serialized safety-related
                                                   application data, return the actual length cached/configured in mapper*/
        uint16* const
            pwLengthOfNonSafetyData, /**< [in,out] input (Expected) length of the serialized non-safety application
                                        data, return the actual length cached/configured in mapper*/
        UAM_SConsOutputTCDU_type* const
            pzTcdu /**< [out] Pointer to the Test Control Data Unit with SafetyConsumerSAPI Outputs */
    );

    /**
     * \brief Function type for setting the Test Control Data Unit with SafetyConsumerSAPI Inputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSConsInputTCDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        const UAM_SConsInputTCDU_type* const
            pzTcdu /**< [in] Pointer to the Test Control Data Unit with SafetyConsumerSAPI Inputs */
    );

    /*---------------------*/
    /*  F U N C T I O N S  */
    /*---------------------*/

#ifdef _WIN32
    /**
     * Set the parameters of the SafetyConsumerUT Mapper
     */
    UAM_SetSafetyConsumerUTParam_ftype UAM_SetSafetyConsumerUTParam;

    /**
     * Browse the parameters of the corresponding SafetyConsumerUTA Mapper
     */
    UAM_BrowseSafetyConsumerUTAParam_ftype UAM_BrowseSafetyConsumerUTAParam;

    /**
     * Get the Test Control Data Unit with SafetyConsumerSAPI Outputs
     */
    UAM_GetSConsOutputTCDU_ftype UAM_GetSConsOutputTCDU;

    /**
     * Set the Test Control Data Unit with SafetyConsumerSAPI Inputs
     */
    UAM_SetSConsInputTCDU_ftype UAM_SetSConsInputTCDU;
#endif
#ifdef __GNUC__
    /**
     * Set the parameters of the SafetyConsumerUT Mapper
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetSafetyConsumerUTParam(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        const UAM_SafetyConsumerParam_type* const
            pzParam /**< Pointer to the parameter set of the SafetyConsumer Mapper */
    );
    /**
     * Browse the parameters of the corresponding SafetyConsumerUTA Mapper
     */
    UAM_API UAM_ErrorCode WINAPI UAM_BrowseSafetyConsumerUTAParam(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        UAM_SafetyConsumerUTAParam_type* const
            pzParam /**< Pointer to the parameter set of the SafetyConsumerUTA Mapper */
    );
    /**
     * Get the Test Control Data Unit with SafetyConsumerSAPI Outputs
     */
    UAM_API UAM_ErrorCode WINAPI UAM_GetSConsOutputTCDU(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        uint16* const pwLengthOfSafetyData, /**< (Expected) length of the serialized safety-related application data */
        uint16* const pwLengthOfNonSafetyData, /**< (Expected) length of the serialized non-safety application data */
        UAM_SConsOutputTCDU_type* const
            pzTcdu /**< Pointer to the Test Control Data Unit with SafetyConsumerSAPI Outputs */
    );
    /**
     * Set the Test Control Data Unit with SafetyConsumerSAPI Inputs
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetSConsInputTCDU(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        const UAM_SConsInputTCDU_type* const
            pzTcdu /**< Pointer to the Test Control Data Unit with SafetyConsumerSAPI Inputs */
    );
#endif

    /*---------------------*/
    /*  V A R I A B L E S  */
    /*---------------------*/

#ifdef __cplusplus
}
#endif /* extern C */

#endif /* __UAM_SAFETYCONSUMERUT_H */
