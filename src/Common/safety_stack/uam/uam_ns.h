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
/** \file This file contains the UAM interface from non-safe user point of view
 * So as to distinguish messages on COMMUNICATION side from those on SAFE side,
 * the frames from COMMUNCATION are named "Messages", whereas those on SAFE side
 * are named "SPDU"
 */

#ifndef SOPC_UAM_NS_H_
#define SOPC_UAM_NS_H_ 1

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_enum_types.h"
#include "uam.h"
#include "uas.h"

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/

/**
 * A Handle (identifier) for a SPDU request
 */
typedef UAS_UInt32 UAM_SpduRequestHandle;
/**
 * A Handle (identifier) for a SPDU response
 */
typedef UAS_UInt32 UAM_SpduResponseHandle;

typedef struct UAM_NS_Configuration_struct
{
    UAM_RedundancySetting_type eRedundancyType;
    /** Session handle. */
    UAM_SessionHandle dwHandle;
    bool bIsProvider;
    /** User-parameters */
    UAM_SpduRequestHandle uUserRequestId;
    UAM_SpduResponseHandle uUserResponseId;
} UAM_NS_Configuration_type;

/*============================================================================
 * EXPORTED CONSTANTS
 *===========================================================================*/

/** The OPC UA namespace used */
#define UAM_NAMESPACE 1

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

/**
 *  \brief Convert a UAS return code to SOPC_ReturnStatus
 *  \param bUasCode The UAS internal code
 *  \return the equivalent SOPC return code
 */
SOPC_ReturnStatus UAM_NS_UasToSopcCode(UAS_UInt8 bUasCode);

/**
 * \brief Shall be called before the module is used
 */
SOPC_ReturnStatus UAM_NS_Initialize(void);

/**
 * \brief Create a SPDU
 * \param pzConfig A pointer to the configuration. The pointer can be freed after call, but not the data pointed
 *      by pzConfig->pUserParams
 * \return false in case of error.
 */
bool UAM_NS_CreateSpdu(const UAM_NS_Configuration_type* const pzConfig);

/**
 * \brief Call this function when a SPDU request message has been received on communication side. This will trigger a
 * further call to UAM_NS2S_SendSpduImpl.
 * \param pzExt The extension object received
 */
void UAM_NS_RequestMessageReceived(UAM_SessionHandle dwHandle);

/**
 * \brief Call this function when a SPDU response message has been received on communication side. This will trigger a
 * further call to UAM_NS2S_SendSpduImpl.
 * \param pzExt The extension object received
 */
void UAM_NS_ResponseMessageReceived(UAM_SessionHandle dwHandle);

/**
 * \brief Call this function to poll reception of SPDU from SAFE component
 * \param pzExt The extension object received
 */
void UAM_NS_CheckSpduReception(UAM_SessionHandle dwHandle);
/**
 * \brief Stop all safety consumers and Producer. Removes all memory allocations
 */
void UAM_NS_Clear(void);

#endif
