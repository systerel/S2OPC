/**
 * OPC Foundation OPC UA Safety Stack
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief OPC UA Safety data type definitions and constant definitions
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines macros and data types for the OPC UA Safety protocol.
 *
 * Safety-Related: yes
 */

#ifndef INC_UASTYPES_H

#define INC_UASTYPES_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_stdtypes.h"

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/** @name Logical and bit operators
 *  These macros define the alternative spellings to be used instead of logical
 *  and bit operators.
 */
/**@{*/
/*lint -save -e960 */
#define AND &&     /**< Logical AND                    */
#define AND_EQ &=  /**< Bitwise AND assignment         */
#define BITAND &   /**< Bitwise AND                    */
#define BITNOT ~   /**< Bitwise NOT                    */
#define BITOR |    /**< Bitwise OR                     */
#define BITXOR XOR /**< Bitwise XOR                    */
#define COMPL ~    /**< Bitwise NOT (One's Complement) */
#define EQ ==      /**< Equal to                       */
#define NOT !      /**< Logical negation               */
#define NOT_EQ !=  /**< Not equal to                   */
#define OR ||      /**< Logical OR                     */
#define OR_EQ |=   /**< Bitwise OR assignment          */
#define XOR ^      /**< Bitwise XOR                    */
#define XOR_EQ ^=  /**< Bitwise XOR assignment         */
/*lint -restore */
/**@}*/

/**
 * Length of the CRC in octets.
 */
#define UAS_CRC_LENGTH 0x04u

/** @name Length of process data.
 *  These macros define the limits for the length of serialized process data.
 */
/**@{*/
#define UAS_SAFETY_DATA_LENGTH_MAX 1500u /**< Maximum allowed length of serialized process data */
#define UAS_SAFETY_DATA_LENGTH_MIN 1u    /**< Minimum allowed length of serialized process data */
/**@}*/

/**
 * Fail-safe values (FSV) are issued instead of process values when the system
 * is set to a fail-safe state.
 */
#define UAS_FSV 0x00u

/** @name Bit positions in SPDU.Flags
 *  Use the following definitions to access the flags in SPDUs.
 */
/**@{*/
#define UAS_BITPOS_ACTIVATE_FSV 1u        /**< Bit position of signal 'ActivateFSV'          in ResponseSPDU.Flags */
#define UAS_BITPOS_COMMUNICATION_ERROR 0u /**< Bit position of signal 'CommunicationError'   in RequestSPDU.Flags */
#define UAS_BITPOS_FSV_ACTIVATED 2u       /**< Bit position of signal 'FSV_Activated'        in RequestSPDU.Flags */
#define UAS_BITPOS_OPERATOR_ACK_PROVIDER                                        \
    0u /**< Bit position of signal 'OperatorAckProvider'  in ResponseSPDU.Flags \
        */
#define UAS_BITPOS_OPERATOR_ACK_REQUESTED                                                                         \
    1u                                    /**< Bit position of signal 'OperatorAckRequested' in RequestSPDU.Flags \
                                           */
#define UAS_BITPOS_TEST_MODE_ACTIVATED 2u /**< Bit position of signal 'TestModeActivated'    in ResponseSPDU.Flags */
/**@}*/

/** @name MNR limit values
 *  Use the following definitions for the limits of the Monitoring number (MNR).
 */
/**@{*/

/**
 * Value 0xFFFFFFFF is the maximum value for MNR.
 */
#define UAS_MNR_MAX 0xFFFFFFFFu

/**
 * Value 0x100 is the start value for MNR, also used after wrap-around.
 * The values 0 to 0xFF are reserved for future use.
 */
#define UAS_MNR_MIN 0x100u

/**@}*/

/** @name SafetyProviderLevel
 *  These macros define the values for the SafetyProviderLevel
 */
/**@{*/
#define UAS_SAFETY_LEVEL_1 0x01u /**< SafetyProviderLevel up to SIL1 */
#define UAS_SAFETY_LEVEL_2 0x02u /**< SafetyProviderLevel up to SIL2 */
#define UAS_SAFETY_LEVEL_3 0x03u /**< SafetyProviderLevel up to SIL3 */
#define UAS_SAFETY_LEVEL_4 0x04u /**< SafetyProviderLevel up to SIL4 */
/**@}*/

/** @name SafetyProviderLevel_ID
 *  These macros define the values for the SafetyProviderLevel_ID
 */
/**@{*/
#define UAS_SAFETY_LEVEL_1_ID 0x11912881 /**< SafetyProviderLevel_ID for SafetyProviderLevel up to SIL1 */
#define UAS_SAFETY_LEVEL_2_ID 0x647C4654 /**< SafetyProviderLevel_ID for SafetyProviderLevel up to SIL2 */
#define UAS_SAFETY_LEVEL_3_ID 0xDEAA9DEE /**< SafetyProviderLevel_ID for SafetyProviderLevel up to SIL3 */
#define UAS_SAFETY_LEVEL_4_ID 0xAB47F33B /**< SafetyProviderLevel_ID for SafetyProviderLevel up to SIL4 */
/**@}*/

/**@{*/
/* This Macro is used to shift an unsigned value of 8 bits and store the result in a 32 bit unsigned value */
#define UAS_UINT16_SHIFT8(value) ((UAS_UInt16)(((UAS_UInt16)(value)) << 8))
/**@}*/

/**@{*/
/* This Macro is used to shift an unsigned value of 8 bits and store the result in a 32 bit unsigned value */
#define UAS_UINT32_SHIFT8(value) (((UAS_UInt32)(value)) << 8)
/**@}*/

/* This Macro is used to shift an unsigned value of 16 bits and store the result in a 32 bit unsigned value */
/**@{*/
#define UAS_UINT32_SHIFT16(value) (((UAS_UInt32)(value)) << 16)
/**@}*/

/* This Macro is used to shift an unsigned value of 24 bits and store the result in a 32 bit unsigned value */
/**@{*/
#define UAS_UINT32_SHIFT24(value) (((UAS_UInt32)(value)) << 24)
/**@}*/

/* This Macro is used to shift an unsigned value and store the result in a 64 bit unsigned value */
/**@{*/
#define UAS_UINT64_SHIFT(value, bits) ((UAS_UInt64)(((UAS_UInt64)(value)) << (bits)))
/**@}*/

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/**
 * Data type definition for universal unique identifier version4
 * (UUIDv4, also called globally unique identifier (GUID))  attributes.
 */
typedef struct UAS_GUID_struct
{
#if 1                   // tbd
    UAS_UInt32 dwPart1; /**< attribute part1 */
    UAS_UInt32 dwPart2; /**< attribute part2 */
    UAS_UInt32 dwPart3; /**< attribute part3 */
    UAS_UInt32 dwPart4; /**< attribute part4 */
#else
    UAS_UInt32 dwData1;
    UAS_UInt16 wData2;
    UAS_UInt16 wData3;
    UAS_UInt8 abyData4[8];
#endif
} UAS_GUID_type;

/**
 * Data type definition for the SPDU_ID.
 */
typedef struct UAS_SPDUID_struct
{
    UAS_UInt32 dwPart1; /**< attribute part1 */
    UAS_UInt32 dwPart2; /**< attribute part2 */
    UAS_UInt32 dwPart3; /**< attribute part3 */
} UAS_SPDUID_type;

/**
 * Identifier of the safety layer diagnostic messages.
 */
typedef enum UAS_SafetyDiagIdentifier_enum
{
    /** The SafetyConsumer did not detect any error. */
    UAS_DIAG_OK = 0x00,
    /** The SafetyConsumer has discarded a message due to an incorrect ID. */
    UAS_DIAG_SD_ID_ERR_IGN = 0x01,
    /** Mismatch of SafetyBaseID. The SafetyConsumer has switched to fail-safe substitute values due to an incorrect ID.
       Operator acknowledgment is required. */
    UAS_DIAG_SD_ID_ERR_OA = 0x11,
    /** Mismatch of SafetyProviderID. The SafetyConsumer has switched to fail-safe substitute values due to an incorrect
       ID. Operator acknowledgment is required. */
    UAS_DIAG_SD_ID_ERR_OA_0 = 0x12,
    /** Mismatch of safety data structure or identifier. The SafetyConsumer has switched to fail-safe substitute values
       due to an incorrect ID. Operator acknowledgment is required. */
    UAS_DIAG_SD_ID_ERR_OA_1 = 0x13,
    /** The SafetyConsumer has discarded a message due to a CRC error (data corruption). */
    UAS_DIAG_CRC_ERR_IGN = 0x04,
    /** The SafetyConsumer has switched to fail-safe substitute values due to a CRC error (data corruption). Operator
       acknowledgment is required. */
    UAS_DIAG_CRC_ERR_OA = 0x14,
    /** The SafetyConsumer has discarded a message due to an incorrect ConsumerID. */
    UAS_DIAG_COID_ERR_IGN = 0x05,
    /** The SafetyConsumer has switched to fail-safe substitute values due to an incorrect consumer ID. Operator
       acknowledgment is required. */
    UAS_DIAG_COID_ERR_OA = 0x15,
    /** The SafetyConsumer has discarded a message due to an incorrect monitoring number. */
    UAS_DIAG_MNR_ERR_IGN = 0x06,
    /** The SafetyConsumer has switched to fail-safe substitute values due to an incorrect monitoring number. Operator
       acknowledgment is required. */
    UAS_DIAG_MNR_ERR_OA = 0x16,
    /** The SafetyConsumer has switched to fail-safe substitute values due to timeout. */
    UAS_DIAG_COMM_ERR_TO = 0x07,
    /** The SafetyConsumer has switched to fail-safe substitute values at the request of the safety application. */
    UAS_DIAG_APPL_ERR_TO = 0x08,
    /** The SafetyConsumer has switched to fail-safe substitute values at the request of the SafetyProvider. Operator
       acknowledgment is required. */
    UAS_DIAG_FSV_REQUESTED = 0x20
} UAS_SafetyDiagIdentifier_type;

/**
 * Data type definition for the Safety Parameter Interface (SPI) of the SafetyProvider.
 * The SafetyProviderSPI represents the parameters of the Safety communication layer
 * management of the SafetyProvider.
 */
typedef struct UAS_SafetyProviderSPI_struct
{
    /** Provider-ID of the SafetyProvider, see Clause 3.2.26 and Clause 11.1.1. of the OPC UA Safety Specification. */
    UAS_UInt32 dwSafetyProviderId;
    /** Base-ID of the SafetyProvider, which is normally used, see Clause 3.2.25. and Clause 11.1.1.of the OPC UA Safety
     * Specification. For dynamic systems, the safety application program can overwrite this ID by providing a non-zero
     * value at the input SafetyBaseID of the SafetyProvider’ s SAPI. */
    UAS_GUID_type zSafetyBaseId;
    /** SafetyProviderLevel is a constant value depending on the safety implementation, see UASDEF_SAFETY_LEVEL */
    /** Signature of the SafetyData structure, for calculation see Clause 8.1.3.4 of the OPC UA Safety Specification. */
    UAS_UInt32 dwSafetyStructureSignature;
    /** SafetyStructureSignatureVersion is not used in the UAS */
} UAS_SafetyProviderSPI_type;

/**
 * Data type definition for the Safety Application Program Interface (SAPI) inputs of the SafetyProvider.
 * The SafetyProvider SAPI represents the Safety communication layer services of the SafetyProvider.
 */
typedef struct UAS_SafetyProviderSAPII_struct
{
    /** This input is used to accept the user data which is then transmitted as SafetyData
     * in the SPDU.
     * NOTE: Whenever a new MNR is received from a SafetyConsumer, the state machine of the
     * SafetyProvider will read a new value of the SafetyData from its corresponding
     * Safety Application and use it until the next MNR is received.
     * NOTE: If no valid user data is available at the Safety Application, ActivateFSV
     * is expected to be set to "1" by the Safety Application. */
    UAS_UInt8* pbySerializedSafetyData;
    /** Used to consistently transmit non-safety data values (e.g. diagnostic information)
     * together with safe data. */
    UAS_UInt8* pbySerializedNonSafetyData;
    /** By setting this input to "1" the remote SafetyConsumer is informed (by Bit 2 in
     * ResponseSPDU.Flags) that the SafetyData are test data, and is not to be used for
     * safety related decisions. */
    UAS_Bool bEnableTestMode;
    /** This input to is used to implement an operator acknowledgment on the provider side.
     * The value will be forwarded to the consumer, where it can be used to trigger a return
     * from fail-safe substitute values (FSV) to actual process values (PV). */
    UAS_Bool bOperatorAckProvider;
    /** By setting this input to '1' the SafetyConsumer is instructed (via Bit 1 in
     * ResponseSPDU.Flags) to deliver Fail-safe Substitute Values (FSV) instead of PV to the
     * safety application program.
     * NOTE: If the replacement of process values by FSV should be controllable in a more
     * fine-grained way, this can be realized by using qualifiers within the SafetyData. */
    UAS_Bool bActivateFsv;
    /** By changing this input to a non-zero-value, the SafetyProvider uses this variable
     * instead of the SPI-Parameter SafetyProviderID. If it is changed to '0', the parameter
     * SafetyProviderID will become activated. */
    UAS_UInt32 dwSafetyProviderId;
    /** By changing this input to a non-zero-value, the SafetyProvider uses this variable
     * instead of the SPI-Parameter SafetyBaseID. If it is changed to '0', the parameter
     * SafetyBaseID will become activated. */
    UAS_GUID_type zSafetyBaseId;
} UAS_SafetyProviderSAPII_type;

/**
 * Data type definition for the Safety Application Program Interface (SAPI) outputs of the SafetyProvider.
 * The SafetyProvider SAPI represents the Safety communication layer services of the SafetyProvider.
 */
typedef struct UAS_SafetyProviderSAPIO_struct
{
    /** This output indicates the request for operator acknowledgment. */
    UAS_Bool bOperatorAckRequested;
    /** This output yields the ConsumerID used in the last access to this SafetyProvider
     * by a SafetyConsumer.
     * NOTE: All safety-related checks are executed by OPC UA Safety. The safety application
     * is not required to check this SafetyConsumerID. */
    UAS_UInt32 dwSafetyConsumerId;
    /** This output yields the monitoring number (MNR). It is updated whenever a new request
     * comes in from the SafetyConsumer.
     * NOTE: All safety-related checks are executed by OPC UA Safety. The safety application
     * is not required to check this Monitoring number. */
    UAS_UInt32 dwMonitoringNumber;
} UAS_SafetyProviderSAPIO_type;

/**
 * Data type definition for the (safety-related) flags from the SafetyProvider
 */
typedef struct UAS_SafetyProviderFlags_struct
{
    /** Activation of fail-safe values by the safety application at the SafetyProvider,
     * hereby forwarded to the SafetyConsumer. */
    UAS_Bool bActivateFSV;
    /** Operator acknowledgment at the provider, hereby forwarded to the SafetyConsumer. */
    UAS_Bool bOperatorAckProvider;
    /** Enabling and disabling of test mode in the SafetyProvider, hereby forwarded to the SafetyConsumer. */
    UAS_Bool TestModeActivated;
} UAS_SafetyProviderFlags_type;

/**
 * Data type definition for the Safety Parameter Interface (SPI) of the SafetyConsumer.
 * The SafetyConsumer SPI represents the parameters of the Safety communication layer
 * management of the SafetyConsumer.
 */
typedef struct UAS_SafetyConsumerSPI_struct
{
    /** The SafetyProviderID of the SafetyProvider this SafetyConsumer normally connects to.
     * For dynamic systems, the safety application program can overwrite this ID by providing
     * a non-zero value at the input SafetyProviderID of the safety Consumer's SAPI. */
    UAS_UInt32 dwSafetyProviderId;
    /** The default SafetyBaseID of the SafetyProvider this SafetyConsumer uses to make a connection.
     * For dynamic systems, the safety application program can overwrite this ID by providing
     * a non-zero value at the input SafetyBaseID of the SafetyConsumer's SAPI. */
    UAS_GUID_type zSafetyBaseId;
    /** ID of the SafetyConsumer. */
    UAS_UInt32 dwSafetyConsumerId;
    /** SafetyConsumer's expectation on the maximal SIL the SafetyProvider implementation
     * (hardware & software) is capable of. */
    UAS_UInt8 bySafetyProviderLevel;
    /** Signature over the SafetyData structure. */
    UAS_UInt32 dwSafetyStructureSignature;
    /** SafetyStructureSignatureVersion is not used in the UAS */
    /** SafetyStructureIdentifier is not used in the UAS */
    /** Watchdog-time in microseconds. Whenever the SafetyConsumer sends a request to a
     * SafetyProvider, its watchdog timer is set to this value.
     * The expiration of this timer prior to receiving an error-free reply by the
     * SafetyProvider indicates an unacceptable delay. */
    // JC REVIEW : no unit specified! (ms, us, ns ?)
    UAS_UInt32 dwSafetyConsumerTimeout;
    /** This parameter controls whether an operator acknowledgment (OA) is necessary in case of
     * errors of type 'unacceptable delay' or 'loss', or when the SafetyProvider has activated
     * FSV (ActivateFSV).
     * 1: FSV are provided at the output SafetyData of the SAPI until OA.
     * 0: PV are provided at SafetyData of the SAPI as soon as the communication is free
     *    of errors. In case of ActivateFSV the values change from FSV to PV as soon as
     *    ActivateFSV returns to '0'.
     * Note: This parameter does not have an influence on the behavior of the SafetyConsumer
     *       following the detection of other types of communication errors, such as data corruption.
     *       For these types of errors, OA is mandatory. */
    UAS_Bool bSafetyOperatorAckNecessary;
    /** Error interval time in minutes. The parameter SafetyErrorIntervalLimit determines the minimum
     * distance two consecutive communication errors must have for not triggering a switch
     * to FSV in the SafetyConsumer. It affects the availability and the PFH of this
     * OPC UA Safety link. */
    UAS_UInt16 wSafetyErrorIntervalLimit;
} UAS_SafetyConsumerSPI_type;

/**
 * Data type definition for the Safety Application Program Interface (SAPI) inputs of the SafetyConsumer.
 * The SafetyConsumer SAPI represents the Safety communication layer services of the SafetyConsumer.
 */
typedef struct UAS_SafetyConsumerSAPII_struct
{
    /** By changing this input to "0" the SafetyConsumer will change each and every variable
     * of the SafetyData to "0" and stop sending requests to the SafetyProvider.
     * When changing Enable to "1" the SafetyConsumer will restart safe communication.
     * The variable can be used to delay the start of the OPC UA Safety communication,
     * after power on until "OPC UA connection ready" is set. The delay time is not
     * monitored while enable is set to '0'. */
    UAS_Bool bEnable;
    /** After an indication of OperatorAckRequested this input can be used to signal an
     * operator acknowledgment. By changing this input from '0' to "1" (rising edge)
     * the SafetyConsumer is instructed to switch SafetyData from FSV to PV.
     * OperatorAckConsumer is processed only if this rising edge arrives after
     * OperatorAckRequested was set to '1'.
     * If a rising edge of OperatorAckConsumer arrives before OperatorAckRequested
     * becomes 1, this rising edge is ignored. */
    UAS_Bool bOperatorAckConsumer;
    /** By changing this input to a non-zero value, the SafetyConsumer uses this variable
     * instead of the SPI-Parameter SafetyProviderID. This input is only read in the first
     * cycle, or when a rising edge occurs at the input Enable. If it is changed to '0',
     * the parameter SafetyProviderID will become activated. */
    UAS_UInt32 dwSafetyProviderId;
    /** By changing this input to a non-zero-value the SafetyConsumer uses this variable
     * instead of the SPI-Parameter SafetyBaseID. This input is only read in the first
     * cycle, or when a rising edge occurs at the input Enable. If it is changed to '0',
     * the SPI-parameter SafetyBaseID will become activated. */
    UAS_GUID_type zSafetyBaseId;
    /** By changing this input to a non-zero-value the SafetyConsumer uses this variable
     * instead of the SPI-Parameter SafetyConsumerID. This input is only read in the first
     * cycle, or when a rising edge occurs at the input Enable. If it is changed to '0',
     * the SPI-parameter SafetyConsumerID will become activated. */
    UAS_UInt32 dwSafetyConsumerId;
} UAS_SafetyConsumerSAPII_type;

/**
 * Data type definition for the Safety Application Program Interface (SAPI) outputs of the SafetyConsumer.
 * The SafetyConsumer SAPI represents the Safety communication layer services of the SafetyConsumer.
 */
typedef struct UAS_SafetyConsumerSAPIO_struct
{
    /** This output either delivers the process values received from the SafetyProvider
     * in the SPDU field SafetyData, or FSV. */
    UAS_UInt8* pbySerializedSafetyData;
    /** Used to consistently transmit non-safety data values (e.g. diagnostic information)
     * together with safe data. */
    UAS_UInt8* pbySerializedNonSafetyData;
    /** This output indicates via "1", that on the output SafetyData FSV (all binary "0")
     * are provided.
     * NOTE: If an application needs different FSV than 'all binary 0', it is expected
     *       to use appropriate constants and ignore the output of SafetyData whenever
     *       FSV_Activated is set.
     * NOTE: If the ResponseSPDU is checked with error: ActivateFSV is set. */
    UAS_Bool bFsvActivated;
    /** This output indicates the request for operator acknowledgment.
     * The bit is set to '1' by the SafetyConsumer, when three conditions are met:
     * 1. Too many communication errors were detected in the past, so the SafetyConsumer
     *    decided to switch to fail-safe substitute values.
     * 2. Currently, no communication errors occur, and hence operator acknowledgment
     *    is possible.
     * 3. Operator acknowledgment (rising edge at input OperatorAckConsumer) has not yet
     *    occurred.
     * The bit is reset to '0' when a rising edge at OperatorAckConsumer is detected. */
    UAS_Bool bOperatorAckRequested;
    /** This output indicates that an operator acknowledgment has taken place on the
     * SafetyProvider. If operator acknowledgment at the SafetyProvider should be allowed,
     * this output is connected to OperatorAckConsumer.
     * NOTE: If the ResponseSPDU is checked with error, this output remains last value. */
    UAS_Bool bOperatorAckProvider;
    /** The safety application program is expected to evaluate this output for determining
     * whether the communication partner is in test mode or not. A value of '1' indicates
     * that the communication partner (source of data) is in test mode, e.g. during
     * commissioning. Data coming from a device in test mode may be used for testing but
     * is not intended to be used to control safety-critical processes. A value of '0'
     * represents the 'normal' safety-related mode.
     * Motivation: Test mode enables the programmer and commissioner to validate the
     *             safety application using test data.
     * NOTE: If the ResponseSPDU is checked with error: TestModeActivated is reset. */
    UAS_Bool bTestModeActivated;
} UAS_SafetyConsumerSAPIO_type;

/**
 * Data type definition for the (non-safety-related) flags from the SafetyConsumer
 */
typedef struct UAS_SafetyConsumerFlags_struct
{
    /** 0: No error
     * 1: An error was detected in the previous ResponseSPDU. */
    UAS_Bool bCommunicationError;
    /** Used to inform the SafetyProvider that operator acknowledgment is requested. */
    UAS_Bool bOperatorAckRequested;
    /** Is used for conformance test of SafetyConsumer.SAPI.FSV_Activated. */
    UAS_Bool bFsvActivated;
} UAS_SafetyConsumerFlags_type;

/**
 * Data type definition for the diagnostic interface
 */
typedef struct UAS_SafetyConsumerDI_struct
{
    /** Identifier of the diagnostic message. */
    UAS_SafetyDiagIdentifier_type nSafetyDiagnosticId;
    /** 0: Single error within the error interval that can be ignored.
     * 1: Permanent error within the error interval that requires failsafe handling. */
    UAS_Bool bPermanent;
} UAS_SafetyConsumerDI_type;

/**
 * Data type definition for the RequestSPDU
 */
typedef struct UAS_RequestSpdu_struct
{
    /** Safety Consumer Identifier - the identifier of the SafetyConsumer instance. */
    UAS_UInt32 dwSafetyConsumerId;
    /** Monitoring Number (MNR) of the RequestSPDU. The SafetyConsumer uses the MNR
     * to detect mistimed SPDUs, e.g. such SPDUs which are continuously repeated
     * by an erroneous network storing element. A different MNR is used in every
     * RequestSPDU of a given SafetyConsumer, and a ResponseSPDU will only be accepted,
     * if its MNR is identical to its matching RequestSPDU. */
    UAS_UInt32 dwMonitoringNumber;
    /** Non safety Flags from SafetyConsumer. */
    UAS_UInt8 byFlags;
} UAS_RequestSpdu_type;

/**
 * Data type definition for the ResponseSPDU
 */
typedef struct UAS_ResponseSpdu_struct
{
    /** Pointer to the serialized safety-related application data */
    UAS_UInt8* pbySerializedSafetyData;
    /** Safety Flags from SafetyProvider. */
    UAS_UInt8 byFlags;
    /** Safety PDU Identifier. The SPDU_ID is used by the SafetyConsumer to check
     * whether the ResponseSPDU is coming from the correct SafetyProvider. */
    UAS_SPDUID_type zSpduId;
    /** The SafetyConsumerID in the ResponseSPDU shall be a copy of the SafetyConsumerID
     * received in the corresponding RequestSPDU. */
    UAS_UInt32 dwSafetyConsumerId;
    /** Monitoring Number (MNR) of the ResponseSPDU. The MNR in the ResponseSPDU
     * shall be a copy of the MNR received in the corresponding RequestSPDU. */
    UAS_UInt32 dwMonitoringNumber;
    /** This CRC-checksum shall be used to detect data corruption. */
    UAS_UInt32 dwCrc;
    /** Pointer to the serialized non-safety application data */
    UAS_UInt8* pbySerializedNonSafetyData;
} UAS_ResponseSpdu_type;

/*---------------------*/
/*  F U N C T I O N S  */
/*---------------------*/

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

#endif /* INC_UASTYPES_H */
