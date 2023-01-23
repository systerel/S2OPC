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

#ifndef S2OPC_PIKEOS_SIGNAL_H_
#define S2OPC_PIKEOS_SIGNAL_H_

#define SIGTERM 0
#define SIGINT 1

typedef void (*sig_t)(int);

/* Don't do anything just return NULL */
sig_t signal(int __sig, sig_t __handler);

/* Don't do anything*/
void exit(int status);

#endif // S2OPC_PIKEOS_SIGNAL_H_
