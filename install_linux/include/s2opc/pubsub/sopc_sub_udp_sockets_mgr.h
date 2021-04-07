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

#ifndef SOPC_SUB_UDP_SOCKETS_MGR_H_
#define SOPC_SUB_UDP_SOCKETS_MGR_H_

#include "sopc_raw_sockets.h"

typedef void (*SOPC_UDP_ReadyToReceive)(void* sockContext, Socket sock);
typedef void (*SOPC_UDP_PeriodicTick)(void* ctx);

/**
 * Initialize the sockets manager for the given sockets (with custom context), the data received callback and tick
 * callback (to check keep alive timeout).
 * A dedicated thread is started to check when sockets have data to read and to call the periodic callback.
 *
 * \param sockContextArray  array of context pointers (shall have \p nbSockets elements)
 * \param socketArray       array of sockets defined to receive data as subscriber (shall have \p nbSockets elements)
 * \param nbSockets         the number of sockets (and sockets context)
 * \param callback          the callback called when a socket has data to read available, socket and its associated
 *                           context are provided by caller
 * \param tickCb            the callback called in a periodic way (each 500ms) to could check keep alive timeout
 * \param tickCbCtx         the callback context pointer provided on call to \p tickCb callback
 * */
void SOPC_UDP_SocketsMgr_Initialize(void** sockContextArray,
                                    Socket* socketArray,
                                    uint16_t nbSockets,
                                    SOPC_UDP_ReadyToReceive callback,
                                    SOPC_UDP_PeriodicTick tickCb,
                                    void* tickCbCtx);

/**
 * Awaits the dedicated thread to stop, clear the context and return.
 */
void SOPC_UDP_SocketsMgr_Clear(void);

#endif /* SOPC_SUB_UDP_SOCKETS_MGR_H_ */
