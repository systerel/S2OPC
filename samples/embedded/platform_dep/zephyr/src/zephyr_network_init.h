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

#ifndef ZEPHYR_NETWORK_INIT_H_
#define ZEPHYR_NETWORK_INIT_H_

#include <stdbool.h>

/** \brief
 * Configure the network for ZEPHYR with LOOPBACK and ETH0
 * \param overrideEthAddr Ip address to set-up eth0 card.
 *  \a CONFIG_SOPC_ETH_ADDRESS is used if NULL.
 */
bool Network_Initialize(const char* overrideEthAddr);

#endif /* ZEPHYR_NETWORK_INIT_H_ */
