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

#ifndef FREE_RTOS_SHELL_H
#define FREE_RTOS_SHELL_H

/**
 *  Read from UART and echo back every character caught.
 *  Simple emulation of "DEL", left and right keys.
 *  The function blocks until the line is validated (Return Key)
 *
 *  @return a new allocated string with the content of input. It must be freed by caller
 */
char* SOPC_Shell_ReadLine(void);

/**
 *   Printf-like function that outputs the result on the default serial line.
 *   Each line is limited to 80 chars. The function is thread-safe, implying that multiple calls in
 *   different threads may be blocking.
 */
void SOPC_Shell_Printf(const char* msg, ...);

#endif /* FREE_RTOS_SHELL_H */
