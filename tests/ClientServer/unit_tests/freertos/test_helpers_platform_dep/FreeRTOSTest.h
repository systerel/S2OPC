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

#ifndef FREE_RTOS_TEST_H
#define FREE_RTOS_TEST_H

void FREE_RTOS_TEST_API_S2OPC_THREAD(void* ptr);
void FREE_RTOS_TEST_S2OPC_SERVER(void* ptr);
void FREE_RTOS_TEST_S2OPC_CLIENT(void* ptr);
void FREE_RTOS_TEST_S2OPC_TIME(void* ptr);
void FREE_RTOS_TEST_S2OPC_CHECK_THREAD(void* ptr);
void FREE_RTOS_TEST_S2OPC_UDP_SOCKET_API(void* ptr);
void FREE_RTOS_TEST_S2OPC_UDP_SOCKET_API_LB(void* ptr);
void FREE_RTOS_TEST_S2OPC_PUBSUB(void* ptr);
void FREE_RTOS_TEST_S2OPC_USECASE_PUBSUB_SYNCHRO(void* ptr);

#endif
