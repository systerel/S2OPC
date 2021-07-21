/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/*============================================================================
 * DESCRIPTION
 *===========================================================================*/
/** \file Provides an abstraction to SPDU REQUESTS and RESPONSE sending and receiving
 * - Module shall first be initialized once using UAM_SpduEncoder_Initialize
 * - Multiple SPDU can be created (must be done both for sending or receiving) using
 *    UAM_SpduEncoder_CreateSpduRequest or UAM_SpduEncoder_CreateSpduResponse
 * - Current content of SPDUs can be read using UAM_SpduEncoder_GetRequest and
 *    UAM_SpduEncoder_GetResponse
 * - Current content of SPDUs can be modified using UAM_SpduEncoder_SetRequest and
 *    UAM_SpduEncoder_SetResponse
 * - Module shall eventually be cleaned once using UAM_SpduEncoder_Clear
 */

#ifndef SOPC_UAM_SPDUENCODERS_H_
#define SOPC_UAM_SPDUENCODERS_H_

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "uam.h"
#include "uam_ns.h"

#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"

/*============================================================================
  GLOBAL CONSTANTS
 *===========================================================================*/

/** These values contain the numeric part of the NodeId reserved for identifying
 * Extended objects. They SHALL be equal on both sides so that receivers can decode
 * the object correctly.
 * Note: The values are purely arbitrary here, and may be modified (TODO : TBC). They do not need to
 * be known at UAS or application level. It is only used internally by S2OPC to identify the
 * encoding/decoding procedures
 */
#define OpcUaId_SPDURequest 10000001
#define OpcUaId_SPDURequest_Encoding_DefaultBinary 10000002
#define OpcUaId_SPDURequest_Encoding_DefaultXml 10000003

#define OpcUaId_SPDUResponse 10000011
#define OpcUaId_SPDUResponse_Encoding_DefaultBinary 10000012
#define OpcUaId_SPDUResponse_Encoding_DefaultXml 10000013

/*============================================================================
 * TYPES DEFINITIONS
 *===========================================================================*/

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

/**
 * \brief This function shall be called once before all other functions of this module
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus UAM_SpduEncoder_Initialize(void);

/**
 * \brief Create and registers a SPDU request. This function can be called both by
 *    PROVIDER or CONSUMER. This creates an extension Object Encoder/decoder
 * \pre UAM_SpduEncoder_Initialize must have been called
 * \param dwNumericId The numeric part of the OPC NodeId defining the Extension Object Identifier.
 *    It shall be identical on both sides (PROV + CONS)
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus UAM_SpduEncoder_CreateSpduRequest(const UAM_SpduRequestHandle dwNumericId);

/**
 * \brief Create and registers a SPDU Response. This function can be called both by
 *    PROVIDER or CONSUMER.
 * \pre UAM_SpduEncoder_Initialize must have been called
 * \param dwNumericId The numeric part of the OPC NodeId defining the Extension Object Identifier.
 *    It shall be identical on both sides (PROV + CONS)
 * \param safeDataLength The length of safe data field
 * \param unsafeDataLength The length of non-safe data field
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus UAM_SpduEncoder_CreateSpduResponse(const UAM_SpduResponseHandle dwNumericId,
                                                     const UAS_UInt16 safeDataLength,
                                                     const UAS_UInt16 unsafeDataLength);

/**
 * \brief Get a copy to the Request SPDU. This function ensures thread-safety access to the data
 *    by locking the cache during copy, and returning a copy.
 *    It will typically read by a provider.
 * \pre UAM_SpduEncoder_Initialize must have been called
 * \param[in] dwHandle The numeric part of the OPC NodeId defining the request.
 * \param[out] pzSpdu The read SPDU, in case the return value is SOPC_STATUS_OK. Undefined otherwise
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus UAM_SpduEncoder_GetRequest(const UAM_SpduRequestHandle dwNumericId, UAS_RequestSpdu_type* pzSpdu);

/**
 * \brief Get a copy to the Response SPDU. This function ensures thread-safety access to the data
 *    by locking the cache during copy, and returning a copy.
 *    It will typically read by a provider.
 * \pre UAM_SpduEncoder_Initialize must have been called
 * \param[in] dwHandle The numeric part of the OPC NodeId defining the Response.
 * \param[out] pzSpdu The read SPDU, in case the return value is SOPC_STATUS_OK. Undefined otherwise.
 *          The content of SAFE and Non-SAFE buffer are COPIED into pzSpdu. They shall
 *          be allocated and released by caller. The allocated sizes must match exactly the
 *          values provided in call to \ref UAM_SpduEncoder_GetRequest.
 *
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus UAM_SpduEncoder_GetResponse(const UAM_SpduResponseHandle dwNumericId, UAS_ResponseSpdu_type* pzSpdu, size_t* puSafeSize, size_t*puNonSafeSize);

/**
 * \brief
 *  Retreives the sizes of safe & non safe fields
 */
SOPC_ReturnStatus UAM_SpduEncoder_GetResponseSizes(const UAM_SpduResponseHandle dwNumericId, size_t* puSafeSize, size_t*puNonSafeSize);

/**
 * \brief Set the content of the Request SPDU. This function ensures thread-safety access to the data
 *    by locking the cache during copy.
 *    It will typically written by a consumer.
 * \pre UAM_SpduEncoder_Initialize must have been called
 * \param[in] dwHandle The numeric part of the OPC NodeId defining the request.
 * \param[in] pzSpdu The SPDU to write.
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus UAM_SpduEncoder_SetRequest(const UAM_SpduRequestHandle dwNumericId,
                                             const UAS_RequestSpdu_type* pzSpdu);

/**
 * \brief Set the content of the Response SPDU. This function ensures thread-safety access to the data
 *    by locking the cache during copy.
 *    It will typically written by a provider.
 * \pre UAM_SpduEncoder_Initialize must have been called
 * \param[in] dwHandle The numeric part of the OPC NodeId defining the request.
 * \param[in] pzSpdu The SPDU to write. The data pointed to in Safe and non Safe buffers are copied and can be freed
 *      after call.
 * \return SOPC_STATUS_OK in case of success
 */
SOPC_ReturnStatus UAM_SpduEncoder_SetResponse(const UAM_SpduResponseHandle dwNumericId,
                                              const UAS_ResponseSpdu_type* pzSpdu);

/**
 * \brief Cleans the module.
 *
 * \pre UAM_SpduEncoder_Initialize must have been called
 */
void UAM_SpduEncoder_Clear(void);

#endif /* SOPC_UAM_SPDUENCODERS_H_ */
