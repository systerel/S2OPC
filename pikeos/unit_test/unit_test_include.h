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

#ifndef UNIT_TEST_INCLUDE_H_
#define UNIT_TEST_INCLUDE_H_

void suite_test_alloc_memory(int* index);

void suite_test_thread_mutexes(int* index);

void suite_test_atomic(int* index);

void suite_test_time(int* index);

void suite_test_raw_sockets(int* index);

void suite_test_udp_sockets(int* index);

void suite_test_check_sockets(int* index);

void suite_test_check_threads(int* index);

void suite_test_server_client(int* index);

void suite_test_publisher_subsriber(int* index);
#endif // UNIT_TEST_INCLUDE_H_
