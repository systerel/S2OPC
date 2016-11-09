/*
 *  \file sopc_run.h
 *
 *  \brief API to trigger message reception and events in single threaded mode
 *         Note: only necessary for single threaded mode
 *
 *  Created on: Nov 9, 2016
 *      Author: VMO (Systerel)
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
