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
 * \brief OPC UA Safety Mapper data type definitions and constant definitions
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines macros and data types for the OPC UA Safety Mapper.
 *
 * Safety-Related: no
 */

#ifndef __UAM_TYPES_H
#define __UAM_TYPES_H

#include "uam_lib.h"
#include "uam_stdtypes.h"
#undef __cplusplus // TODO JCH : parsing error
#ifdef __cplusplus
extern "C"
{
#endif

/** @name Error codes of the OPC UA Safety Mapper (UAM)
 *  These macros define the return values of the functions.
 */
/**@{*/
#define UAM_ERRORCODE_OK 0x00000000                  /**< Function result OK                 */
#define UAM_ERRORCODE_BAD 0x80000000                 /**< Unspecific Error */
#define UAM_ERRORCODE_BAD_NOT_IMPLEMENTED 0x80000001 /**< Function didn't set return value   */
#define UAM_ERRORCODE_BAD_PARAMETER 0x80000002       /**< At least one of paased parameter wrong or invalid   */
#define UAM_ERRORCODE_BAD_HAS_UNSENT_DATA 0x80000003 /**< Not all data are sent due to lack of memory */
#define UAM_ERRORCODE_BAD_NULL_POINTER 0x80000004    /**< t least one of the passed pointers is NULL. */
#define UAM_ERRORCODE_BAD_STATE 0x80000005           /**< Function not allowed in that state.   */
#define UAM_ERRORCODE_NO_DATA 0x80000006             /**< No data available   */
#define UAM_ERRORCODE_BAD_UA_STACK_ERROR 0x80000007  /**< if error is from ua stack*/
#define UAM_ERRORCODE_BAD_DEVICE 0x80000008          /**< A local device interface returns an error.   */
#define UAM_ERRORCODE_BAD_NO_MEMORY 0x80000009       /**< Not enough memory available   */
#define UAM_ERRORCODE_BAD_COMMUNICATION 0x80000010   /**< Communication Error   */
#define UAM_ERRORCODE_BAD_TIMER 0x80000011           /**< Timer Error  */
#define UAM_ERRORCODE_BAD_SYNCRONIZATION 0x80000012  /**< Synchoronization Error   */

    /**@}*/

    /**
     * Identifier of the mapper instance types.
     */
    typedef enum UAM_InstanceType_enum
    {
        /** The instance is not configured. */
        UAM_NOT_USED = 0x00,
        /** The instance is an SafetyProvider Mapper. */
        UAM_SAFETY_PROVIDER = 0x01,
        /** The instance is an SafetyConsumer Mapper. */
        UAM_SAFETY_CONSUMER = 0x02,
        /** The instance is an SafetyProviderUT Mapper. */
        UAM_SAFETY_PROVIDER_UT = 0x03,
        /** The instance is an SafetyConsumerUT Mapper. */
        UAM_SAFETY_CONSUMER_UT = 0x04,
        /** The instance is an SafetyProviderUTA Mapper. */
        UAM_SAFETY_PROVIDER_UTA = 0x05,
        /** The instance is an SafetyConsumerUTA Mapper. */
        UAM_SAFETY_CONSUMER_UTA = 0x06,
    } UAM_InstanceType_type;

    /** @name ID types
     *  Data type definitions for IDs used in mapper.
     */
    /**@{*/

    typedef uint32 UAM_InstanceId_type; /**< Instance ID */
    typedef uint32 UAM_SafetyId_type;   /**< Safety ID   */

    /*TODO not in doc*/
    typedef uint32 UAM_ErrorCode;                        /**< Error Code   */
    typedef UAS_GUID_type UAM_GUID_type;                 /**< GUID type   */
    typedef UAS_RequestSpdu_type UAM_RequestSPDU_type;   /**< RequestSPDU   */
    typedef UAS_ResponseSpdu_type UAM_ResponseSPDU_type; /**< ResponseSPDU   */
    /**@}*/

    /**
     * This structure contains the configuration of an UA node
     */
    typedef struct UAM_NodeAddress_type
    {
        /** Node ID */
        char* pszNodeId;
        /** Index of the name space */
        uint32 dwNameSpaceIndex;
        /** Index of namespface*/
        /** TODO:@elke, is node address correct*/
        uint16 nsIndex;
        /** Node ID */
        uint32 nodeId;
        /** tbd */
    } UAM_NodeAddress_type;

    /**
     * This structure contains the configuration of an UAM instance
     */
    typedef struct UAM_InstanceData_type
    {
        /** ID of the UAM instance. */
        UAM_InstanceId_type dwInstanceId;
        /** Type of the UAM instance */
        UAM_InstanceType_type nInstanceType;
        /** OPC UA Node address */
        UAM_NodeAddress_type zNodeAddress;
        /** Length of SafetyData */
        uint16 wSafetyDataLength;
        /** Length of NonSafetyData */
        uint16 wNonSafetyDataLength;
        /** tbd */
    } UAM_InstanceData_type;

    /**
     * Data type definition for the Test Control Data Unit with SafetyProviderSAPI Inputs
     */
    typedef struct UAM_SProvInputTCDU_struct
    {
        /** Inputs of the Safety Application Program Interface (SAPI) of the SafetyProvider */
        UAS_SafetyProviderSAPII_type zSapiInputs;
        /** Test step */
        uint16 wTestStep;
    } UAM_SProvInputTCDU_type;

    /**
     * Data type definition for the Test Control Data Unit with SafetyProviderSAPI Outputs
     */
    typedef struct UAM_SProvOutputTCDU_struct
    {
        /** Outputs of the Safety Application Program Interface (SAPI) of the SafetyProvider */
        UAS_SafetyProviderSAPIO_type zSapiOutputs;
        /** Test step */
        uint16 wTestStep;
    } UAM_SProvOutputTCDU_type;

    /**
     * Data type definition for the Test Control Data Unit with SafetyConsumerSAPI Inputs
     */
    typedef struct UAM_SConsInputTCDU_struct
    {
        /** Inputs of the Safety Application Program Interface (SAPI) of the SafetyConsumer */
        UAS_SafetyConsumerSAPII_type zSapiInputs;
        /** Test step */
        uint16 wTestStep;
    } UAM_SConsInputTCDU_type;

    /**
     * Data type definition for the Test Control Data Unit with SafetyConsumerSAPI Outputs
     */
    typedef struct UAM_SConsOutputTCDU_struct
    {
        /** Outputs of the Safety Application Program Interface (SAPI) of the SafetyConsumer */
        UAS_SafetyConsumerSAPIO_type zSapiOutputs;
        /** Identifier of the diagnostic message. */
        UAS_SafetyDiagIdentifier_type nSafetyDiagnosticId;
        /** Test step */
        uint16 wTestStep;
    } UAM_SConsOutputTCDU_type;

    /**
     * Data type definition for the parameters of the SafetyProvider Mapper
     */
    typedef struct UAM_SafetyProviderParam_struct
    {
        /** Safety Parameter Interface (SPI) of the SafetyProvider */
        UAS_SafetyProviderSPI_type zSpi;
        /** tbd */
    } UAM_SafetyProviderParam_type;

    /**
     * Data type definition for the parameters of the SafetyConsumer Mapper
     */
    typedef struct UAM_SafetyConsumerParam_struct
    {
        /** Safety Parameter Interface (SPI) of the SafetyConsumer */
        UAS_SafetyConsumerSPI_type zSpi;
        /** tbd */
    } UAM_SafetyConsumerParam_type;

    /**
     * Data type definition for the parameters of the SafetyProviderUT Mapper
     */
    typedef struct UAM_SafetyProviderUTParam_struct
    {
        /** Instance ID of the Test Channel Mapper used by the Lower Tester (LT) */
        const UAM_InstanceId_type nTestChannelId;
        /** tbd ... */
    } UAM_SafetyProviderUTParam_type;

    /**
     * Data type definition for the parameters of the SafetyConsumerUT Mapper
     */
    typedef struct UAM_SafetyConsumerParamUT_struct
    {
        /** Instance ID of the Test Channel Mapper used by the Lower Tester (LT) */
        const UAM_InstanceId_type nTestChannelId;
        /** tbd ... */
    } UAM_SafetyConsumerUTParam_type;

    /**
     * Data type definition for the parameters of the SafetyProviderUTA Mapper
     */
    typedef struct UAM_SafetyProviderUTAParam_struct
    {
        /** Instance ID of the Test Channel Mapper used by the Lower Tester (LT) */
        UAM_InstanceId_type nTestChannelId;
        /** Indicates whether the UTA is configured for the SafetyProvider test */
        uint8 bIsConfigured;
        /** Node ID of the SafetyProvider (IUT) to be tested */
        // tbd: UTA does not know the NodeID --> mapper has to find the NodeID by itself using the handle
        // UAM_NodeAddress_type zRelatedNodeID;
        /** Value for the SafetyData to be used for the SafetyProvider test */
        uint8* pbySafetyDataValue;
        /** Value for the NonSafetyData to be used for the SafetyProvider test */
        uint8* pbyNonSafetyDataValue;
        /** Indicates whether the SafetyProvider supports dynamic IDs via the SAPI */
        uint8 bDynamicSystemsSupported;
        /** Value for SAPI.SafetyProviderID to be used for the SafetyProvider test */
        uint32 dwDynamicProviderIdValue;
        /** Value for SAPI.SafetyBaseID to be used for the SafetyProvider test */
        UAS_GUID_type zDynamicBaseIdValue;
        /** Value for SAPI.SafetyConsumerID to be used for the SafetyProvider test */
        uint32 dwDynamicConsumerIdValue;
        /** tbd ... */
    } UAM_SafetyProviderUTAParam_type;

    /**
     * Data type definition for the parameters of the SafetyConsumerUTA Mapper
     */
    typedef struct UAM_SafetyConsumerParamUTA_struct
    {
        /** Instance ID of the Test Channel Mapper used by the Lower Tester (LT) */
        UAM_InstanceId_type nTestChannelId;
        /** Indicates whether the UTA is configured for the SafetyProvider test */
        uint8 bIsConfigured;
        /** Node ID of the SafetyProvider (IUT) to be tested */
        UAM_NodeAddress_type zRelatedNodeID;
        /** Value for the SafetyData to be used for the SafetyProvider test */
        uint8* pbySafetyDataValue;
        /** Value for the NonSafetyData to be used for the SafetyProvider test */
        uint8* pbyNonSafetyDataValue;
        /** Indicates whether the SafetyProvider supports dynamic IDs via the SAPI */
        uint8 bDynamicSystemsSupported;
        /** Value for SAPI.SafetyProviderID to be used for the SafetyProvider test */
        uint32 dwDynamicProviderIdValue;
        /** Value for SAPI.SafetyBaseID to be used for the SafetyProvider test */
        UAS_GUID_type zDynamicBaseIdValue;
        /** Value for SAPI.SafetyConsumerID to be used for the SafetyProvider test */
        uint32 dwDynamicConsumerIdValue;
        /** tbd ... */
    } UAM_SafetyConsumerUTAParam_type;

#ifdef __cplusplus
}
#endif

#endif /*__UAM_TYPES_H*/
