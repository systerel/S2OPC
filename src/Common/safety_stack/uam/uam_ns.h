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
 *
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
typedef struct UAM_NS_Configuration_struct
{
    UAM_RedundancySetting_type eRedundancyType;
    /** The numeric part of the OPC NodeId defining the SPDU from NonSafe App to Safe App. */
    UAS_UInt32 dwInputHandle;
    /** The numeric part of the OPC NodeId defining the SPDU from Safe App to NonSafe App. */
    UAS_UInt32 dwOutputHandle;
} UAM_NS_Configuration_type;

typedef UAS_UInt8 UAM_NS_SpduHandle;

/*============================================================================
 * EXPORTED CONSTANTS
 *===========================================================================*/

/** The OPC UA namespace used */
#define UAM_NAMESPACE 1

/*============================================================================
 * EXTERNAL SERVICES
 *===========================================================================*/

/**
 * \brief Shall be called before the module is used
 * \param eRedundancyType The kind of redundancy used between SAFE & NON-SAFE
 */
void UAM_NS_Initialize(void);

/**
 * \brief Create a SPDU
 * \param pzConfig TODO
 * \return UAM_NoHandle in case of error.
 */
UAM_NS_SpduHandle UAM_NS_CreateSpdu(const UAM_NS_Configuration_type* const pzConfig);

/**
 * \brief Stop all safety consumers and Producer. Removes all memory allocations
 */
void UAM_NS_Clear(void);

#endif
