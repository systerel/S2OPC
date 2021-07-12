/**
 * OPC Foundation OPC UA Safety Stack
 *
 * \file
 * \author
 *    Copyright 2021 (c) ifak e.V.Magdeburg
 *    Copyright 2021 (c) Elke Hintze
 *
 * \brief OPC UA Safety interface definition.
 *
 * \date      2021-05-14
 * \revision  0.2
 * \status    in work
 *
 * Defines the interface of the OPC UA Safety stack.
 *
 * Safety-Related: yes
 */

#ifndef INC_UAS_H

#define INC_UAS_H

/*--------------------------------------------------------------------------*/
/******************************* I M P O R T ********************************/
/*--------------------------------------------------------------------------*/

#include "uas_def.h"
#include "uas_stdtypes.h"
#include "uas_type.h"

/*--------------------------------------------------------------------------*/
/******************************* E X P O R T ********************************/
/*--------------------------------------------------------------------------*/

/*---------------*/
/*  M A C R O S  */
/*---------------*/

/** @name Error codes of the UAS manager
 *  These macros define the return values of the functions.
 */
/**@{*/
#define UAS_OK (UASRVAL_NO_ERROR)                                      /**< Function result OK                 */
#define UAS_STATE_ERR (UASRVAL_UASMNGR_ID + UASRVAL_STATE_ERR)         /**< Function not allowed in that state */
#define UAS_MEMORY_ERR (UASRVAL_UASMNGR_ID + UASRVAL_MEMORY_ERR)       /**< Not enough memory available        */
#define UAS_POINTER_ERR (UASRVAL_UASMNGR_ID + UASRVAL_POINTER_ERR)     /**< Pointer is NULL                    */
#define UAS_FCT_PARAM_ERR (UASRVAL_UASMNGR_ID + UASRVAL_FCT_PARAM_ERR) /**< Parameter is faulty                */
#define UAS_DEVICE_ERR (UASRVAL_UASMNGR_ID + UASRVAL_DEVICE_ERR)       /**< Device error                       */
#define UAS_NOT_IMPL (UASRVAL_UASMNGR_ID + UASRVAL_NOT_IMPL)           /**< Function not implemented yet       */
#define UAS_NO_DATA (UASRVAL_UASMNGR_ID + UASRVAL_NO_DATA)             /**< No data available                  */
#define UAS_COMM_ERR (UASRVAL_UASMNGR_ID + UASRVAL_COMM_ERR)           /**< Communication error                */
#define UAS_TIMER_ERR (UASRVAL_UASMNGR_ID + UASRVAL_TIMER_ERR)         /**< Timer error                        */
#define UAS_SOFT_ERR (UASRVAL_UASMNGR_ID + UASRVAL_SOFT_ERR)           /**< Soft error                         */
#define UAS_SYNC_ERR (UASRVAL_UASMNGR_ID + UASRVAL_SYNC_ERR)           /**< Synchronization error              */
#define UAS_DEFAULT_ERR (UASRVAL_UASMNGR_ID + UASRVAL_DEFAULT_ERR)     /**< Function didn't set return value   */
/**@}*/

/*-------------*/
/*  T Y P E S  */
/*-------------*/

/**
 * Data type for the UAS system time (timestamp)
 */
typedef UAS_UInt32 UAS_Timestamp_type;

/**
 * This data type contains local diagnostic information.
 */
typedef enum UAS_ParameterError_enum
{
    /** The SafetyConsumer did not detect any error. */
    UAS_PARAMETER_OK = 0x00,
    /** SafetyDataLength is invalid. */
    UAS_INVALID_SAFETY_DATA_LENGTH,
    /** NonSafetyDataLength is invalid. */
    UAS_INVALID_NON_SAFETY_DATA_LENGTH,
    /** NULL Pointer to SafetyData. */
    UAS_INVALID_SAFETY_DATA_POINTER,
    /** NULL Pointer to NonSafetyData. */
    UAS_INVALID_NON_SAFETY_DATA_POINTER,
    /** SafetyBaseID is invalid. */
    UAS_INVALID_SAFETY_BASE_ID,
    /** SafetyProviderID is invalid. */
    UAS_INVALID_SAFETY_PROVIDER_ID,
    /** SafetyConsumerID is invalid. */
    UAS_INVALID_SAFETY_CONSUMER_ID,
    /** SafetyProviderLevel is invalid. */
    UAS_INVALID_SAFETY_PROVIDER_LEVEL,
    /** Signature of the SafetyData structure is invalid. */
    UAS_INVALID_SAFETY_STRUCTURE_SIGNATURE,
    /** SafetyConsumerTimeout is invalid. */
    UAS_INVALID_SAFETY_CONSUMER_TIMEOUT,
    /**wSafetyErrorIntervalLimit is invalid. */
    UAS_INVALID_SAFETY_ERROR_INTERVAL_LIMIT,
} UAS_ParameterError_type;

/* This structure contains the configuration and interface data */
/* of an UAS SafetyProvider instance                            */
typedef struct UAS_SafetyProvider_struct
{
    /** IN: Handle of the UAS instance. */
    UAS_UInt32 dwHandle;
    /** IN: Length of SafetyData*/
    UAS_UInt16 wSafetyDataLength;
    /** IN: Length of NonSafetyData*/
    UAS_UInt16 wNonSafetyDataLength;
    /** IN: Safety Provider SPI. */
    UAS_SafetyProviderSPI_type zSPI;
    /** IN: SAPI Inputs of the SafetyProvider. */
    UAS_SafetyProviderSAPII_type zInputSAPI;
    /** OUT: SAPI Outputs of the SafetyProvider. */
    UAS_SafetyProviderSAPIO_type zOutputSAPI;
    /** IN: RequestSPUD */
    UAS_RequestSpdu_type zRequestSPDU;
    /** OUT: ResponseSPUD */
    UAS_ResponseSpdu_type zResponseSPDU;
    /*  IN/OUT: Sync flag with the communication task */
    UAS_Bool bCommDone;
    /*  IN/OUT: Sync flag with the application task */
    UAS_Bool bAppDone;
} UAS_SafetyProvider_type;

/* This structure contains the configuration and interface data */
/* of an UAS SafetyConsumer instance                            */
typedef struct UAS_SafetyConsumer_struct
{
    /** IN: Handle of the UAS instance. */
    UAS_UInt32 dwHandle;
    /** IN: Length of SafetyData*/
    UAS_UInt16 wSafetyDataLength;
    /** IN: Length of NonSafetyData*/
    UAS_UInt16 wNonSafetyDataLength;
    /** IN: SPI of the Safety SafetyConsumer. */
    UAS_SafetyConsumerSPI_type zSPI;
    /** IN: SAPI Inputs of the SafetyConsumer. */
    UAS_SafetyConsumerSAPII_type zInputSAPI;
    /** OUT: SAPI Outputs of the SafetyConsumer. */
    UAS_SafetyConsumerSAPIO_type zOutputSAPI;
    /** OUT: DI of the SafetyConsumer. */
    UAS_SafetyConsumerDI_type zDI;
    /** OUT: RequestSPUD */
    UAS_RequestSpdu_type zRequestSPDU;
    /** IN: ResponseSPUD */
    UAS_ResponseSpdu_type zResponseSPDU;
    /*  IN/OUT: Sync flag with the communication task */
    UAS_Bool bCommDone;
    /*  IN/OUT: Sync flag with the application task */
    UAS_Bool bAppDone;
} UAS_SafetyConsumer_type;

/*----------------------------------------*/
/*  F U N C T I O N S  ( SafetyProvider ) */
/*----------------------------------------*/

/**
 * Initialization of a SafetyProvider instance
 */
UAS_UInt8 byUAS_InitSafetyProvider(
    /** IN: Pointer to the data structure of the SafetyProvider instance */
    UAS_SafetyProvider_type* pzInstanceData,
    /** IN: Index (Number) of the SafetyProvider instance */
    UAS_UInt16 uInstanceIndex,
    /** Pointer to an output variable for the result of the parameter check */
    UAS_ParameterError_type* pnResult);

/**
 * Change parameters of a SafetyProvider instance
 */
UAS_UInt8 byUAS_ChangeSafetyProviderSPI(
    /** IN: Index (Number) of the SafetyProvider instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyProvider instance */
    UAS_UInt32 dwHandle,
    /** IN: New SPI parameters of the SafetyProvider instance. */
    UAS_SafetyProviderSPI_type* pzSPI,
    /** Pointer to an output variable for the result of the parameter check */
    UAS_ParameterError_type* pnResult);

/**
 * Start of a SafetyProvider instance
 */
UAS_UInt8 byUAS_StartSafetyProvider(
    /** IN: Index (Number) of the SafetyProvider instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyProvider instance */
    UAS_UInt32 dwHandle);

/**
 * Execution of a SafetyProvider instance
 */
UAS_UInt8 byUAS_ExecuteSafetyProvider(
    /** IN: Index (Number) of the SafetyProvider instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyProvider instance */
    UAS_UInt32 dwHandle);

/**
 * Stop of a SafetyProvider instance
 */
UAS_UInt8 byUAS_StopSafetyProvider(
    /** IN: Index (Number) of the SafetyProvider instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyProvider instance */
    UAS_UInt32 dwHandle);

/**
 * De-initialization of a SafetyProvider instance
 */
UAS_UInt8 byUAS_ResetSafetyProvider(
    /** IN: Index (Number) of the SafetyProvider instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyProvider instance */
    UAS_UInt32 dwHandle);

/*----------------------------------------*/
/*  F U N C T I O N S  ( SafetyConsumer ) */
/*----------------------------------------*/

/**
 * Initialization of a SafetyConsumer instance
 */
UAS_UInt8 byUAS_InitSafetyConsumer(
    /** IN: Pointer to the data structure of the SafetyConsumer instance */
    UAS_SafetyConsumer_type* pzInstance,
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex,
    /** Pointer to an output variable for the result of the parameter check */
    UAS_ParameterError_type* pnResult);

/**
 * Change parameters of a SafetyConsumer instance
 */
UAS_UInt8 byUAS_ChangeSafetyConsumerSPI(
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyConsumer instance */
    UAS_UInt32 dwHandle,
    /** IN: Pointer to new SPI parameters of the SafetyConsumer instance. */
    UAS_SafetyConsumerSPI_type* pzSPI,
    /** Pointer to an output variable for the result of the parameter check */
    UAS_ParameterError_type* pnResult);

/**
 * Change watchdog of a SafetyConsumer instance
 */
UAS_UInt8 byUAS_ChangeSafetyConsumerTimeout(
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyConsumer instance */
    UAS_UInt32 dwHandle,
    /** IN: New timeout value of the SafetyConsumer instance. */
    UAS_UInt32 dwSafetyConsumerTimeout,
    /** Pointer to an output variable for the result of the parameter check */
    UAS_ParameterError_type* pnResult);

/**
 * Start of a SafetyConsumer instance
 */
UAS_UInt8 byUAS_StartSafetyConsumer(
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyConsumer instance */
    UAS_UInt32 dwHandle);

/**
 * Execution of a SafetyConsumer instance
 */
UAS_UInt8 byUAS_ExecuteSafetyConsumer(
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyConsumer instance */
    UAS_UInt32 dwHandle);

/**
 * Stop of a SafetyConsumer instance
 */
UAS_UInt8 byUAS_StopSafetyConsumer(
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyConsumer instance */
    UAS_UInt32 dwHandle);

/**
 * De-initialization of a SafetyConsumer instance
 */
UAS_UInt8 byUAS_ResetSafetyConsumer(
    /** IN: Index (Number) of the SafetyConsumer instance */
    UAS_UInt16 uInstanceIndex,
    /** IN: Handle of of the SafetyConsumer instance */
    UAS_UInt32 dwHandle);

/*---------------------*/
/*  V A R I A B L E S  */
/*---------------------*/

/**
 * Array of SafetyProvider data sets
 * Each data set contains the interfaces of one SafetyProvider
 */
extern UAS_SafetyProvider_type azUAS_SafetyProviders[UASDEF_MAX_SAFETYPROVIDERS];

/**
 * Array of SafetyConsumer data sets
 * Each data set contains the interfaces of one SafetyConsumer
 */
extern UAS_SafetyConsumer_type azUAS_SafetyConsumers[UASDEF_MAX_SAFETYCONSUMERS];

#endif /* INC_UAS_H */
