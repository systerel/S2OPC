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

/** \file
 *
 * \brief Internal API to manage the ServerConfiguration according the Push model.
 */

#ifndef SOPC_PUSH_SERVER_CONFIG_
#define SOPC_PUSH_SERVER_CONFIG_

#include "sopc_builtintypes.h"

/**
 * \brief Get the rejected list of all group.
 *
 * \param[out] ppBsCertArray A valid pointer to the newly created ByteString array.
 * \param[out] pLength The length of \p ppThumbprintArray
 *
 * \return Return SOPC_GoodGenericStatus if successful.
 */
SOPC_StatusCode PushServer_GetRejectedList(SOPC_ByteString** ppBsCertArray, uint32_t* pLengthArray);

/**
 * \brief Export the rejected list.
 *
 * \param bEraseExiting Define if the existing certificate shall be deleted or include.
 *
 * \return SOPC_STATUS_OK if successful.
 */
SOPC_StatusCode PushServer_ExportRejectedList(const bool bEraseExisting);

#endif /* SOPC_PUSH_SERVER_CONFIG_ */
