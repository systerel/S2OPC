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
 * \brief Interface of the SafetyProviderUTA Mapper
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the SafetyProviderUTA Mapper.
 *
 * Safety-Related: no
 */

#ifndef __UAM_SAFETYPROVIDERUT_H
#define __UAM_SAFETYPROVIDERUT_H

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
     * \brief Function type for setting the parameters of the SafetyProviderUTA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSafetyProviderUTAParam_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        const UAM_SafetyProviderUTAParam_type* const
            pzParam /**< [in] Pointer to the parameter set of the SafetyProviderUTA Mapper */
    );

    /**
     * \brief Function type for getting the Test Control Data Unit with SafetyProviderSAPI Inputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_GetSProvInputTCDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        uint16* const pwLengthOfSafetyData,     /**< [in,out] input(Expected) length of the serialized safety-related
                                                   application data, return the actual length cached/configured in mapper*/
        uint16* const
            pwLengthOfNonSafetyData, /**< [in,out] input(Expected) length of the serialized non-safety application data,
                                        return the actual length cached/configured in mapper */
        UAM_SProvInputTCDU_type* const
            pzTcdu /**< [out] Pointer to the Test Control Data Unit with SafetyProviderSAPI Inputs */
    );

    /**
     * \brief Function type for setting the Test Control Data Unit with SafetyProviderSAPI Outputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSProvOutputTCDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        const UAM_SProvOutputTCDU_type* const
            pzTcdu /**< [in] Pointer to the Test Control Data Unit with SafetyProviderSAPI Outputs */
    );

    /*---------------------*/
    /*  F U N C T I O N S  */
    /*---------------------*/

#ifdef _WIN32
    /**
     * Set the parameters of the SafetyProviderUTA Mapper
     */
    UAM_SetSafetyProviderUTAParam_ftype UAM_SetSafetyProviderUTAParam;

    /**
     * Get the Test Control Data Unit with SafetyProviderSAPI Inputs
     */
    UAM_GetSProvInputTCDU_ftype UAM_GetSProvInputTCDU;

    /**
     * Set the Test Control Data Unit with SafetyProviderSAPI Outputs
     */
    UAM_SetSProvOutputTCDU_ftype UAM_SetSProvOutputTCDU;
#endif
#ifdef __GNUC__
    /**
     * Set the parameters of the SafetyProviderUTA Mapper
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetSafetyProviderUTAParam(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        const UAM_SafetyProviderUTAParam_type* const
            pzParam /**< Pointer to the parameter set of the SafetyProviderUTA Mapper */
    );

    /**
     * Get the Test Control Data Unit with SafetyProviderSAPI Inputs
     */
    UAM_API UAM_ErrorCode WINAPI UAM_GetSProvInputTCDU(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        uint16* const pwLengthOfSafetyData, /**< (Expected) length of the serialized safety-related application data */
        uint16* const pwLengthOfNonSafetyData, /**< (Expected) length of the serialized non-safety application data */
        UAM_SProvInputTCDU_type* const
            pzTcdu /**< Pointer to the Test Control Data Unit with SafetyProviderSAPI Inputs */
    );

    /**
     * Set the Test Control Data Unit with SafetyProviderSAPI Outputs
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetSProvOutputTCDU(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        const UAM_SProvOutputTCDU_type* const
            pzTcdu /**< Pointer to the Test Control Data Unit with SafetyProviderSAPI Outputs */
    );

#endif
    /*---------------------*/
    /*  V A R I A B L E S  */
    /*---------------------*/

#ifdef __cplusplus
}
#endif /* extern C */

#endif /* __UAM_SAFETYPROVIDERUTA_H */
