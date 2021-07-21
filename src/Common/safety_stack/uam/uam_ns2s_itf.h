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
/** \file This file contains the interface used to send data from NonSafe to Safe
 * It only provides the interface that shall be implemented by a user-application, which
 * makes it independent from technical means available on each target.
 * For example, a possible implementation may use shared memories, shares files, sockets, ...
 */

#ifndef SOPC_UAM_NS2S_H_
#define SOPC_UAM_NS2S_H_ 1

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "uam.h"
#include "uas.h"

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/

/*============================================================================
 * EXPORTED CONSTANTS
 *===========================================================================*/

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

/**
 * \brief Will be called once per SPDU couple on initialization.
 * \param dwHandle The session handle, as defined in call to UAM_NS_CreateSpdu
 * \return true in case of success
 */
bool UAM_NS2S_Initialize(const UAM_SessionHandle dwHandle);

/**
 * \brief Implementation of a SPDU sending from Non-Safe to Safe partition
 * \param dwHandle The session handle, as defined in call to UAM_NS_CreateSpdu
 * \param pData The data to be sent? Shall point to at least sLen bytes.
 * \param sLen The data length
 */
void UAM_NS2S_SendSpduImpl(const UAM_SessionHandle dwHandle, const void* const pData, const size_t sLen);

/**
 * \brief Implementation of a SPDU reception on Non-Safe from Safe partition.
 *      The call shall not be blocking.
 * \param dwHandle The session handle, as defined in call to UAM_NS_CreateSpdu
 * \param pData A non-null pointer that points to an area where message can be received.
 * \param sMaxLen The maximum buffer length
 * \param[out] sReadLen Return the length of read buffer (0 in case of error)
 */
void UAM_NS2S_ReceiveSpduImpl (const UAM_SessionHandle dwHandle, void* pData, size_t sMaxLen, size_t* sReadLen);

/**
 * \brief Will be called once  on cleanup.
 */
void UAM_NS2S_Clear(void);


#endif
