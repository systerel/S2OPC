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
 * \brief Interface of the SafetyProviderUT Mapper
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the SafetyProviderUT Mapper.
 *
 * Safety-Related: no
 */

#ifndef __UAM_SAFETYPROVIDERUTA_H
#define __UAM_SAFETYPROVIDERUTA_H

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
     * \brief Function type for setting the parameters of the SafetyProviderUT Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSafetyProviderUTParam_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        const UAM_SafetyProviderUTParam_type* const
            pzParam /**< [in] Pointer to the parameter set of the SafetyProviderUT Mapper */
    );

    /**
     * \brief Function type for browsing the parameters of the corresponding SafetyProviderUTA Mapper
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_BrowseSafetyProviderUTAParam_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        UAM_SafetyProviderUTAParam_type* const
            pzParam /**< [out] Pointer to the parameter set of the SafetyProviderUTA Mapper */
    );

    /**
     * \brief Function type for getting the Test Control Data Unit with SafetyProviderSAPI Outputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_GetSProvOutputTCDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        UAM_SProvOutputTCDU_type* const
            pzSpdu /**< [out] Pointer to the Test Control Data Unit with SafetyProviderSAPI Outputs */
    );

    /**
     * \brief Function type for setting the Test Control Data Unit with SafetyProviderSAPI Inputs
     * \return UAM_ERRORCODE_OK if succeed, Errorcode if error occured
     */
    UAM_API_TYPE typedef UAM_ErrorCode WINAPI UAM_SetSProvInputTCDU_ftype(
        const UAM_InstanceId_type dwInstanceId, /**< [in] Instance ID */
        uint16 wLengthOfSafetyData,             /**< [in] Length of the serialized safety-related application data */
        uint16 wLengthOfNonSafetyData,          /**< [in] Length of the serialized non-safety application data */
        const UAM_SProvInputTCDU_type* const
            pzSpdu /**< [in] Pointer to the Test Control Data Unit with SafetyProviderSAPI Inputs */
    );

    /*---------------------*/
    /*  F U N C T I O N S  */
    /*---------------------*/

#ifdef _WIN32
    /**
     * Set the parameters of the SafetyProviderUT Mapper
     */
    UAM_SetSafetyProviderUTParam_ftype UAM_SetSafetyProviderUTParam;

    /**
     * Browse the parameters of the corresponding SafetyProviderUTA Mapper
     */
    UAM_BrowseSafetyProviderUTAParam_ftype UAM_BrowseSafetyProviderUTAParam;

    /**
     * Get the Test Control Data Unit with SafetyProviderSAPI Outputs
     */
    UAM_GetSProvOutputTCDU_ftype UAM_GetSProvOutputTCDU;

    /**
     * Set the Test Control Data Unit with SafetyProviderSAPI Inputs
     */
    UAM_SetSProvInputTCDU_ftype UAM_SetSProvInputTCDU;
#endif
#ifdef __GNUC__
    /**
     * Set the parameters of the SafetyProviderUT Mapper
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetSafetyProviderUTParam(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        const UAM_SafetyProviderUTParam_type* const
            pzParam /**< Pointer to the parameter set of the SafetyProviderUT Mapper */
    );

    /**
     * Browse the parameters of the corresponding SafetyProviderUTA Mapper
     */
    UAM_API UAM_ErrorCode WINAPI UAM_BrowseSafetyProviderUTAParam(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        UAM_SafetyProviderUTAParam_type* const
            pzParam /**< Pointer to the parameter set of the SafetyProviderUTA Mapper */
    );

    /**
     * Get the Test Control Data Unit with SafetyProviderSAPI Outputs
     */
    UAM_API UAM_ErrorCode WINAPI UAM_GetSProvOutputTCDU(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        UAM_SProvOutputTCDU_type* const
            pzSpdu /**< Pointer to the Test Control Data Unit with SafetyProviderSAPI Outputs */
    );

    /**
     * Set the Test Control Data Unit with SafetyProviderSAPI Inputs
     */
    UAM_API UAM_ErrorCode WINAPI UAM_SetSProvInputTCDU(
        const UAM_InstanceId_type dwInstanceId, /**< Instance ID */
        uint16 wLengthOfSafetyData,             /**< Length of the serialized safety-related application data */
        uint16 wLengthOfNonSafetyData,          /**< Length of the serialized non-safety application data */
        const UAM_SProvInputTCDU_type* const
            pzSpdu /**< Pointer to the Test Control Data Unit with SafetyProviderSAPI Inputs */
    );
#endif
    /*---------------------*/
    /*  V A R I A B L E S  */
    /*---------------------*/

#ifdef __cplusplus
}
#endif /* extern C */

#endif /* __UAM_SAFETYPROVIDERUT_H */
