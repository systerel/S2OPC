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

#ifndef SOPC_PUBSUB_HELPERS_H_
#define SOPC_PUBSUB_HELPERS_H_

#include <stdbool.h>

#include "sopc_builtintypes.h"
#include "sopc_pubsub_conf.h"
#include "sopc_udp_sockets.h"

/*
 * Parse a multicast address and return an address info to be used to send data on a
 * socket.
 *
 * Limitation: IP are limited to IPv4 for now (detection function missing)
 *
 * \param address             Cstring address with format opc.udp://<IP>:<PORT>
 * \param[out] multicastAddr  the multicast address. Must be cleared by caller with ::SOPC_Socket_AddrInfoDelete
 *                            after use.
 *
 * \return true in case of success, false otherwise
 */
bool SOPC_PubSubHelpers_ParseAddressUDP(const char* address, SOPC_Socket_AddressInfo** multicastAddr);

/**
 * Check if the variant is compatible (value type and value rank) with the field meta data
 *
 * \param fieldMetaData  the field meta data with which compliance shall be verified
 * \param variant        the variant to check
 * \param out_isBad      output flag to indicate that the variant is compatible (\return true) because it is Bad
 *                       status value instead of expected value type, it then indicates value status only
 *
 * Note:  \p out_isBad can be NULL if information is not needed
 * Note2: if the builtin type in the field meta data is Null builtin type, always \return true
 *
 * \return true if variant is compatible with the field meta data, false otherwise
 */
bool SOPC_PubSubHelpers_IsCompatibleVariant(const SOPC_FieldMetaData* fieldMetaData,
                                            const SOPC_Variant* variant,
                                            bool* out_isBad);

// TODO: to be defined in S2OPC
// Note: only suitable for TCP / UDP / MQTT prefixes
bool SOPC_Helper_URI_ParseUri_WithPrefix(const char* prefix,
                                         const char* uri,
                                         size_t* hostnameLength,
                                         size_t* portIdx,
                                         size_t* portLength);

#endif /* SOPC_PUBSUB_HELPERS_H_ */
