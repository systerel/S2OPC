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
 * \brief Interface of the SafetyConsumerUTA Mapper
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the SafetyConsumerUTA Mapper.
 *
 * Safety-Related: no
 */

#ifndef __UAM_SAFETYCONSUMERUTA_H
#define __UAM_SAFETYCONSUMERUTA_H

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
     * \brief Set the parameters of the SafetyConsumerUTA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSafetyConsumerUTAParam_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        const UAM_SafetyConsumerUTAParam_type* const
            pzParam /**< [in] Pointer to the parameter set of the SafetyConsumerUTA Mapper */
    );

    /**
     * \brief Get the Test Control Data Unit with SafetyConsumerSAPI Inputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_GetSConsInputTCDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        UAM_SConsInputTCDU_type* const
            pzTcdu /**< [out] Pointer to the Test Control Data Unit with SafetyConsumerSAPI Inputs */
    );

    /**
     * \brief Set the Test Control Data Unit with SafetyConsumerSAPI Outputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSConsOutputTCDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        uint16 wLengthOfSafetyData,             /**< [in] Length of the serialized safety-related application data */
        uint16 wLengthOfNonSafetyData,          /**< [in] Length of the serialized non-safety application data */
        const UAM_SConsOutputTCDU_type* const
            pzTcdu /**< [in] Pointer to the Test Control Data Unit with SafetyConsumerSAPI Outputs */
    );

    /*---------------------*/
    /*  F U N C T I O N S  */
    /*---------------------*/

#ifdef _WIN32
    /**
     * Set the parameters of the SafetyConsumerUTA Mapper
     */
    UAM_SetSafetyConsumerUTAParam_ftype UAM_SetSafetyConsumerUTAParam;

    /**
     * Get the Test Control Data Unit with SafetyConsumerSAPI Inputs
     */
    UAM_GetSConsInputTCDU_ftype UAM_GetSConsInputTCDU;

    /**
     * Set the Test Control Data Unit with SafetyConsumerSAPI Outputs
     */
    UAM_SetSConsOutputTCDU_ftype UAM_SetSConsOutputTCDU;
#endif
#ifdef __GNUC__
    /**
     * \brief Set the parameters of the SafetyConsumerUTA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetSafetyConsumerUTAParam(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        const UAM_SafetyConsumerUTAParam_type* const
            pzParam /**< [in] Pointer to the parameter set of the SafetyConsumerUTA Mapper */
    );

    /**
     * \brief Get the Test Control Data Unit with SafetyConsumerSAPI Inputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API UAM_ErrorCode WINAPI UAM_GetSConsInputTCDU(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        UAM_SConsInputTCDU_type* const
            pzTcdu /**< [out] Pointer to the Test Control Data Unit with SafetyConsumerSAPI Inputs */
    );

    /**
     * \brief Set the Test Control Data Unit with SafetyConsumerSAPI Outputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetSConsOutputTCDU(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        uint16 wLengthOfSafetyData,             /**< [in] Length of the serialized safety-related application data */
        uint16 wLengthOfNonSafetyData,          /**< [in] Length of the serialized non-safety application data */
        const UAM_SConsOutputTCDU_type* const
            pzTcdu /**< [in] Pointer to the Test Control Data Unit with SafetyConsumerSAPI Outputs */
    );
#endif
    /*---------------------*/
    /*  V A R I A B L E S  */
    /*---------------------*/

#ifdef __cplusplus
}
#endif /* extern C */

#endif /* __UAM_SAFETYCONSUMERUTA_H */
