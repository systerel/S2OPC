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
/** \file This file contains the UAM PubSub implementation specific interface
 */

#ifndef SOPC_UAM_NS_IMPL_H_
#define SOPC_UAM_NS_IMPL_H_ 1

/*============================================================================
 * INCLUDES
 *===========================================================================*/

#include "sopc_builtintypes.h"
#include "sopc_common.h"
#include "sopc_enum_types.h"
#include "uam_ns.h"
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
 * \brief Shall be called before the module is used
 * \pre user must have initialized SKS parameters before this call (typically 'SOPC_LocalSKS_init')
 * \return The Result status code
 */
SOPC_ReturnStatus UAM_NS_Impl_Initialize(const char * pubSubXmlConfigFile);

/**
 * \brief Clear the module
 */
void UAM_NS_Impl_Clear(void);

#endif
