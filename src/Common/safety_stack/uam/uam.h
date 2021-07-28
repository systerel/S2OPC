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

#include "uas.h"

/*============================================================================
 * EXTERNAL TYPES
 *===========================================================================*/
/**
 * A Session Identifier is a unique identifier, defined by user application. Each
 * UAM_SessionHandle identifies a SPDU REQ/RESP couple. This value allows the
 * UAM Safe and NON-safe layers to map correctly items. Of course, the handle
 * provided to SAFE and NON-SAFE parts for a given SPDU couple shall be identical..
 */
typedef UAS_UInt32 UAM_SessionId;

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
#define UAM_NoHandle (0xFFu) // TODO move & rename & type!

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

#endif
