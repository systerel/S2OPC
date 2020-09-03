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

/**
 *  \file sopc_encodeable.h
 *
 *  \brief Encodeable object services
 */

#ifndef SOPC_ENCODEABLE_H_
#define SOPC_ENCODEABLE_H_

#include <stdint.h>

#include "sopc_builtintypes.h"
#include "sopc_encodeabletype.h"

/**
 *  \brief           Instantiate and initialize an encodeable object of the given encodeable type
 *
 *  \param encTyp    Encodeable type of the encodeable object to instantiate and initialize
 *  \param encObject Pointer to be set with the address of the newly created encodeable object
 *  \return          SOPC_SOPC_STATUS_OK if creation succeeded
 */
SOPC_ReturnStatus SOPC_Encodeable_Create(SOPC_EncodeableType* encTyp, void** encObject);

/**
 *  \brief           Instantiate and initialize an encodeable object of the given encodeable type
 *
 *  \param encTyp    Encodeable type of the encodeable object to deallocate
 *  \param encObject Pointer to the address of the encodeable object to delete (set to NULL if operation succeded)
 *  \return          SOPC_SOPC_STATUS_OK if deletion succeeded
 */
SOPC_ReturnStatus SOPC_Encodeable_Delete(SOPC_EncodeableType* encTyp, void** encObject);

/**
 *  \brief           Create an encodeable object of the given encodeable type and set it in
 *                   the given extension object
 *
 *  \param extObject Extension object in which the newly created encodeable object must be set
 *  \param encTyp    Encodeable type of the encodeable object to instantiate and initialize
 *  \param encObject Pointer to be set with the address of the newly created encodeable object
 *  \return          SOPC_SOPC_STATUS_OK if creation and extension setting succeeded
 */
SOPC_ReturnStatus SOPC_Encodeable_CreateExtension(SOPC_ExtensionObject* extObject,
                                                  SOPC_EncodeableType* encTyp,
                                                  void** encObject);

/**
 *  \brief           Moves content of \p srcObj to \p destObj, i.e. copy \p srcObj structure content to \p destObj and
 *                   reset \p srcObj. Both parameters shall be EncodeableObject with same SOPC_EncodeableType.
 *
 *  \param destObj   Empty and initialized encodeable object in which content of \p srcObj will be copied.
 *  \param srcObj    Source encodeable object from which content will be copied into \p destObj and then reset.
 *  \return          SOPC_SOPC_STATUS_OK if move operation succeeded
 */
SOPC_ReturnStatus SOPC_Encodeable_Move(void* destObj, void* srcObj);

#endif /* SOPC_ENCODEABLE_H_ */
