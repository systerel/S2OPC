/* ========================================================================
 * Copyright (c) 2005-2016 The OPC Foundation, Inc. All rights reserved.
 *
 * OPC Foundation MIT License 1.00
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * The complete license agreement can be found here:
 * http://opcfoundation.org/License/MIT/1.00/
 *
 * Modifications: adaptation for S2OPC project
 * ======================================================================*/

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/
/** \file Provides an abstraction to UAS module, so that it can be easily used
 * by an application, without specific knowledge on how it is actually implemented
 * at transport level.
 *  \note this module is not thread-safe, and no concurrent calls to either of its function
 *  shall be done.
 *
 */

#ifndef SOPC_UAM_H_
#define SOPC_UAM_H_ 1

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_enum_types.h"
#include "uas.h"

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/
typedef UAS_UInt32 UAM_SessionHandle;

/**
 * A Handle (identifier) for a SPDU request
 */
typedef UAS_UInt32 UAM_SpduRequestHandle;
/**
 * A Handle (identifier) for a SPDU response
 */
typedef UAS_UInt32 UAM_SpduResponseHandle;

/**
 *  Redefine interface types with UAS so that the names match actual significance from
 *  application point of view (input of UAS is Output of application and vice-versa)
 */
typedef UAS_SafetyProviderSAPIO_type UAM_ProviderSAPI_Input;
typedef UAS_SafetyProviderSAPII_type UAM_ProviderSAPI_Output;
typedef UAS_SafetyConsumerSAPIO_type UAM_ConsumerSAPI_Input;
typedef UAS_SafetyConsumerSAPII_type UAM_ConsumerSAPI_Output;

typedef struct UAM_SafetyConfiguration_struct
{
    /* TODO: dwRequestHandle & dwResponseHandle cannot be shared to SAFE partition.
     * It shall be known by NonSafe only
     */
    /** The numeric part of the OPC NodeId defining the request. */
    UAS_UInt32 dwRequestHandle;
    /** The numeric part of the OPC NodeId defining the response. */
    UAS_UInt32 dwResponseHandle;
    /** IN: Length of SafetyData*/
    UAS_UInt16 wSafetyDataLength;
    /** IN: Length of NonSafetyData*/
    UAS_UInt16 wNonSafetyDataLength;
} UAM_SafetyConfiguration_type;

/**
 * The callback type for provider user-application level cycle.
 * Will be called for each provider within UAM_Cycle
 * \param pzConfiguration The applicative configuration.
 * \param pzAppInputs The applicative input.
 * \param pzAppOutputs The applicative outputs. They are set to previous cycle values and may
 *          be updated only if required.
 * \return true in case of success.
 */
typedef bool (*UAM_pfProviderApplicationCycle)(const UAM_SafetyConfiguration_type* const pzConfiguration,
                                               const UAM_ProviderSAPI_Input* pzAppInputs,
                                               UAM_ProviderSAPI_Output* pzAppOutputs);

/**
 * The callback type for consumer user-application level cycle.
 * Will be called for each provider within UAM_Cycle.
 * \param pzConfiguration The applicative configuration.
 * \param pzAppInputs The applicative input.
 * \param pzAppOutputs The applicative outputs. They are set to previous cycle values and may
 *          be updated only if required.
 * \return true in case of success.
 */
typedef bool (*UAM_pfConsumerApplicationCycle)(const UAM_SafetyConfiguration_type* const pzConfiguration,
                                               const UAM_ConsumerSAPI_Input* pzAppInputs,
                                               UAM_ConsumerSAPI_Output* pzAppOutputs);

/**
 * A Handle for Provider connections
 */
typedef UAS_UInt8 UAM_ProviderHandle;

/**
 * A Handle for Consumer connections
 */
typedef UAS_UInt8 UAM_ConsumerHandle;

/**
 * Safe Channel Redundancy settings
 */
typedef enum UAM_RedundancySetting_enum
{
    /** There is only one SAFE channel */
    UAM_RDD_SINGLE_CHANNEL,
    /** There are two diversified channel. */
    UAM_RDD_DUAL_CHANNEL,
} UAM_RedundancySetting_type;

/*============================================================================
 * EXPORTED CONSTANTS
 *===========================================================================*/
/** Specific value of UAM_ProviderHandle or UAM_ConsumerHandle indicating an invalid handle */
#define UAM_NoHandle (0xFFu)

/** The OPC UA namespace used */
#define UAM_NAMESPACE 1

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

/**
 * \brief Shall be called before the module is used
 */
void UAM_Initialize(void);

/**
 * \brief Helper function for set-up of a Safety provider. Calls byUAS_InitSafetyProvider
 * \param[in] pzInstanceConfiguration. Configuration of safety provider.
 * \param[in] pzSPI. Pointer to an existing SPI configuration. Shall not be NULL.
 *      Can be freed/released after called
 * \param[in] pfProviderCycle. Pointer to the user cycle application.
 *      Will be called for each provider within UAM_Cycle.
 * \return SOPC_STATUS_OK in case of success.
 */
SOPC_ReturnStatus UAM_InitSafetyProvider(const UAM_SafetyConfiguration_type* const pzInstanceConfiguration,
                                         const UAS_SafetyProviderSPI_type* const pzSPI,
                                         UAM_pfProviderApplicationCycle pfProviderCycle,
                                         UAM_ProviderHandle* phHandle);

/**
 * \brief Helper function for set-up of a Safety consumer. Calls byUAS_InitSafetyConsumer
 * \param[in] pzInstanceConfiguration. Data pointed by zSPI and pzInstanceConfiguration can be released after call.
 * \param[in] pzSPI. Pointer to an existing SPI configuration. Shall not be NULL.
 *      Can be freed/released after called
 * \param[in] pfConsumerCycle. Pointer to the user cycle application.
 *      Will be called for each provider within UAM_Cycle.
 * \return SOPC_STATUS_OK in case of success.
 */
SOPC_ReturnStatus UAM_InitSafetyConsumer(const UAM_SafetyConfiguration_type* const pzInstanceConfiguration,
                                         const UAS_SafetyConsumerSPI_type* const pzSPI,
                                         UAM_pfConsumerApplicationCycle pfConsumerCycle,
                                         UAM_ProviderHandle* phHandle);

/**
 * \brief Start all safety consumers and Producer.
 * \post No more calls to  UAM_InitSafetyProvider or UAM_InitSafetyConsumer are allowed
 * \return SOPC_STATUS_OK in case of success.
 */
SOPC_ReturnStatus UAM_StartSafety(void);

/**
 * \brief Stop all safety consumers and Producer. Removes all memory allocations
 */
void UAM_Clear(void);

/**
 * \brief This is the user-application code cyclic caller
 * \pre UAM_StartSafety shall have been called
 * \pre All registered Providers and Consumers cyclic event are called, and must return true in case of success
 * \see UAM_pfConsumerApplicationCycle
 * \see UAM_pfProviderApplicationCycle
 * \return SOPC_STATUS_OK in case of success.
 *
 */
SOPC_ReturnStatus UAM_Cycle(void);

/**
 *  \brief Convert a UAS return code to SOPC_ReturnStatus
 *  \param bUasCode The UAS internal code
 *  \return the equivalent SOPC return code
 */
SOPC_ReturnStatus UAM_UasToSopcCode(UAS_UInt8 bUasCode);

/**
 * \brief retrieve the UAS Provider object using its handle
 * \pre UAM_StartSafety shall have been called
 * \return NULL if the handle is incorrect. The provider object otherwise
 */
UAS_SafetyProvider_type* UAM_GetProvider(const UAM_ProviderHandle hHandle);

/**
 * \brief retrieve the UAS Consumer object using its handle
 * \pre UAM_StartSafety shall have been called
 * \return NULL if the handle is incorrect. The Consumer object otherwise
 */
UAS_SafetyConsumer_type* UAM_GetConsumer(const UAM_ConsumerHandle hHandle);
#endif
