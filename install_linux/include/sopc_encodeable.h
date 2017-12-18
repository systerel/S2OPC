/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "sopc_toolkit_constants.h"

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

#endif /* SOPC_ENCODEABLE_H_ */
