/**
 *  \file sopc_run.h
 *
 *  \brief API to trigger message reception and events in single threaded mode
 *         Note: only necessary for single threaded mode
 *
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

#include "sopc_base_types.h"

#ifndef SOPC_RUN_H_
#define SOPC_RUN_H_

/**
 *  \brief Trigger the reception of messages and call callback associated with related events
 *         (connection, disconnection, service request, service response, etc.)
 *
 *  \param msecTimeout The channel to initialize
 *  \return            STATUS_NOK if no message received before timeout or failed to treat received message,
 *                     STATUS_OK otherwise
 */
SOPC_StatusCode SOPC_TreatReceivedMessages(uint32_t msecTimeout);

#endif /* SOPC_RUN_H_ */
