/*
 *  Copyright (C) 2018 Systerel and others.
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

#ifndef SOPC_SECURE_CHANNELS_API_INTERNAL_H_
#define SOPC_SECURE_CHANNELS_API_INTERNAL_H_

#include <stdint.h>
#include "sopc_secure_channels_api.h"

/* Secure channel internal event enqueue function
 * IMPORTANT NOTE: non-internal events use will cause an assertion error */
void SOPC_SecureChannels_EnqueueInternalEvent(SOPC_SecureChannels_InputEvent scEvent,
                                              uint32_t id,
                                              void* params,
                                              uintptr_t auxParam);

/* Secure channel internal event enqueue function: event will be enqueued as next to be treated (only for close SC
 * situation) Note: it is important to close the SC as soon as possible in order to avoid any treatment of new messages
 * on a SC to be closed. IMPORTANT NOTE: non-internal events will be refused */
void SOPC_SecureChannels_EnqueueInternalEventAsNext(SOPC_SecureChannels_InputEvent scEvent,
                                                    uint32_t id,
                                                    void* params,
                                                    uintptr_t auxParam);

#endif /* SOPC_SECURE_CHANNELS_API_INTERNAL_H_ */
