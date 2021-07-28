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
/** \file This file contains the interface used to send data from Safe to NonSafe
 * It only provides the interface that shall be implemented by a user-application, which
 * makes it independent from technical means available on each target.
 * For example, a possible implementation may use shared memories, shares files, sockets, ...
 * IT shall provide reverse encoding/decoding functions than those defined in "uam_ns2s_itf.h"
 */

#ifndef SOPC_UAM_S2NS_H_
#define SOPC_UAM_S2NS_H_ 1

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "uam.h"
#include "uam_s.h"
#include "uas.h"

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/
typedef void (*UAM_S2NS_SpduReceptionEvent) (const UAM_SessionId dwSessionId, const void* pData, UAM_S_Size sLen);

/*============================================================================
 * EXPORTED CONSTANTS
 *===========================================================================*/

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

/**
 * \brief Will be called once on initialization.
 * \param dwSessionId The session ID, as defined in call to UAM_NS_CreateSpdu
 */
void UAM_S2NS_Initialize(void);

/**
 * \brief Will be called once per SPDU couple on initialization.
 * \param dwSessionId The session ID, as defined in call to UAM_NS_CreateSpdu
 */
void UAM_S2NS_InitializeSpdu (const UAM_SessionId dwSessionId);

/**
 * \brief Implementation of a SPDU sending from Safe to Non-Safe partition
 * \param dwSessionId The session ID, as defined in call to UAM_NS_CreateSpdu
 * \param pData The data to be sent. Shall point to at least sLen bytes.
 * \param sLen The data length
 */
void UAM_S2NS_SendSpduImpl(const UAM_SessionId dwSessionId, const void* const pData, const UAM_S_Size sLen);

/**
 * \brief Implementation of SPDUs reception on Safe from Non-Safe partition.
 *      The call shall not be blocking and proceed to reading all SPDUs.
 * \param pfMessageProcess The function to call for each received message.
 */
void UAM_S2NS_ReceiveAllSpdusFromNonSafe(UAM_S2NS_SpduReceptionEvent pfMessageProcess);

/**
 * \brief Will be called once  on cleanup.
 */
void UAM_S2NS_Clear(void);

#endif
