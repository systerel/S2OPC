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
/** \file This file contains the UAM interface from safe user point of view
 */

#ifndef SOPC_UAM_S_H_
#define SOPC_UAM_S_H_ 1

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "uam.h"
#include "uas.h"

#include <stdbool.h>

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/

typedef struct UAM_S_Configuration_struct
{
    UAM_RedundancySetting_type eRedundancyType;
    /** Session handle. */
    UAM_SessionHandle dwHandle;
    bool bIsProvider;
} UAM_S_Configuration_type;

typedef struct UAM_SafetyConfiguration_struct
{
    /** Session handle. */
    UAM_SessionHandle dwHandle;
    /** IN: Length of SafetyData*/
    UAS_UInt16 wSafetyDataLength;
    /** IN: Length of NonSafetyData*/
    UAS_UInt16 wNonSafetyDataLength;
} UAM_SafetyConfiguration_type;

/**
 *  Redefine interface types with UAS so that the names match actual significance from
 *  application point of view (input of UAS is Output of application and vice-versa)
 */
typedef UAS_SafetyProviderSAPIO_type UAM_S_ProviderSAPI_Input;
typedef UAS_SafetyProviderSAPII_type UAM_S_ProviderSAPI_Output;
typedef UAS_SafetyConsumerSAPIO_type UAM_S_ConsumerSAPI_Input;
typedef UAS_SafetyConsumerSAPII_type UAM_S_ConsumerSAPI_Output;

/**
 * A Handle for Provider connections
 */
typedef UAS_UInt8 UAM_S_ProviderHandle;

/**
 * A Handle for Consumer connections
 */
typedef UAS_UInt8 UAM_S_ConsumerHandle;

/**
 * The callback type for provider user-application level cycle.
 * Will be called for each provider within UAM_Cycle
 * \param pzConfiguration The applicative configuration.
 * \param pzAppInputs The applicative input.
 * \param pzAppOutputs The applicative outputs. They are set to previous cycle values and may
 *          be updated only if required.
 * \return true in case of success.
 */
typedef bool (*UAM_S_pfProviderApplicationCycle)(const UAM_SafetyConfiguration_type* const pzConfiguration,
                                               const UAM_S_ProviderSAPI_Input* pzAppInputs,
                                               UAM_S_ProviderSAPI_Output* pzAppOutputs);

/**
 * The callback type for consumer user-application level cycle.
 * Will be called for each provider within UAM_Cycle.
 * \param pzConfiguration The applicative configuration.
 * \param pzAppInputs The applicative input.
 * \param pzAppOutputs The applicative outputs. They are set to previous cycle values and may
 *          be updated only if required.
 * \return true in case of success.
 */
typedef bool (*UAM_S_pfConsumerApplicationCycle)(const UAM_SafetyConfiguration_type* const pzConfiguration,
                                               const UAM_S_ConsumerSAPI_Input* pzAppInputs,
                                               UAM_S_ConsumerSAPI_Output* pzAppOutputs);

/*============================================================================
 * EXPORTED CONSTANTS
 *===========================================================================*/


/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

/**
 * \brief Shall be called before the module is used
 */
void UAM_S_Initialize(void);

/**
 * \brief Helper function for set-up of a Safety provider. Calls byUAS_InitSafetyProvider
 * \param[in] pzInstanceConfiguration. Configuration of safety provider.
 *          The dwHandle filed shall be identical to Session handle provided by Non safe
 *          in call to UAM_NS_CreateSpdu
 * \param[in] pzSPI. Pointer to an existing SPI configuration. Shall not be NULL.
 *      Can be freed/released after called
 * \param[in] pfProviderCycle. Pointer to the user cycle application.
 *      Will be called for each provider within UAM_Cycle.
 * \return UAS_OK in case of success.
 */
UAS_UInt8 UAM_S_InitSafetyProvider(const UAM_SafetyConfiguration_type* const pzInstanceConfiguration,
                                         const UAS_SafetyProviderSPI_type* const pzSPI,
                                         UAM_S_pfProviderApplicationCycle pfProviderCycle,
                                         UAM_S_ProviderHandle* phHandle);

/**
 * \brief Helper function for set-up of a Safety consumer. Calls byUAS_InitSafetyConsumer
 * \param[in] pzInstanceConfiguration. Data pointed by zSPI and pzInstanceConfiguration can be released after call.
 * \param[in] pzSPI. Pointer to an existing SPI configuration. Shall not be NULL.
 *      Can be freed/released after called
 * \param[in] pfConsumerCycle. Pointer to the user cycle application.
 *      Will be called for each provider within UAM_Cycle.
 * \return UAS_OK in case of success.
 */
UAS_UInt8 UAM_S_InitSafetyConsumer(const UAM_SafetyConfiguration_type* const pzInstanceConfiguration,
                                         const UAS_SafetyConsumerSPI_type* const pzSPI,
                                         UAM_S_pfConsumerApplicationCycle pfConsumerCycle,
                                         UAM_S_ProviderHandle* phHandle);

/**
 * \brief Start all safety consumers and Producer.
 * \post No more calls to  UAM_InitSafetyProvider or UAM_InitSafetyConsumer are allowed
 * \return UAS_OK in case of success.
 */
UAS_UInt8 UAM_S_StartSafety(void);

/**
 * \brief Stop all safety consumers and Producer. Removes all memory allocations
 */
void UAM_S_Clear(void);

/**
 * \brief This is the user-application code cyclic caller
 * \pre UAM_StartSafety shall have been called
 * \pre All registered Providers and Consumers cyclic event are called, and must return true in case of success
 * \see UAM_pfConsumerApplicationCycle
 * \see UAM_pfProviderApplicationCycle
 * \return UAS_OK in case of success.
 * TODO: should be removed in the end
 */
UAS_UInt8 UAM_S_Cycle(void);

/**
 * \brief retrieve the UAS Provider object using its handle
 * \pre UAM_StartSafety shall have been called
 * \return NULL if the handle is incorrect. The provider object otherwise
 */
UAS_SafetyProvider_type* UAM_S_GetProvider(const UAM_S_ProviderHandle hHandle);

/**
 * \brief retrieve the UAS Consumer object using its handle
 * \pre UAM_StartSafety shall have been called
 * \return NULL if the handle is incorrect. The Consumer object otherwise
 */
UAS_SafetyConsumer_type* UAM_S_GetConsumer(const UAM_S_ConsumerHandle hHandle);


/*============================================================================
 * LOGS FEATURES
 *===========================================================================*/

/**
 * log levels.
 */
typedef enum
{
    UAM_S_LOG_DEFAULT, /**< Default messages are logged */
    UAM_S_LOG_ERROR,   /**< Error messages are logged   */
    UAM_S_LOG_WARN,    /**< Warn messages are logged    */
    UAM_S_LOG_INFO,    /**< Info messages are logged    */
    UAM_S_LOG_DEBUG,   /**< Debug messages are logged   */
    UAM_S_LOG_ALL,     /**< All messages are logged     */
} UAM_S_LOG_LEVEL;

/**
 * Note: SAFE application cannot use variadic parameters, so that the DEBUG features
 * provide ease functions to display different basic parameters
 */
void UAM_S_DoLog(const UAM_S_LOG_LEVEL level, const char* txt);
void UAM_S_DoLog_UHex32 (const UAM_S_LOG_LEVEL level, const char* txt, const UAS_UInt32 u32);
void UAM_S_DoLog_UInt32 (const UAM_S_LOG_LEVEL level, const char* txt, const UAS_UInt32 u32);
void UAM_S_DoLog_Int32 (const UAM_S_LOG_LEVEL level, const char* txt, const UAS_Int32 s32);
void UAM_S_DoLog_Text (const UAM_S_LOG_LEVEL level, const char* txt, const char* ptxt);
void UAM_S_DoLog_Pointer (const UAM_S_LOG_LEVEL level, const char* txt, const void* pAddr);

#endif
