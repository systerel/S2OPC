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

#ifndef B2C_H_
#define B2C_H_
#include <stdbool.h>
#include <stdint.h>

/* t_bool */
#ifndef t_bool_
#define t_bool_
typedef bool t_bool;
#endif /* t_bool */

/* t_entier4 */
#ifndef t_entier4_
#define t_entier4_
typedef int32_t t_entier4;
#endif /* t_entier4 */

/* MAXINT */
#if !defined(MAXINT)
#define MAXINT (2147483647)
#endif /* MAXINT */

/* MININT */
#if !defined(MININT)
#define MININT (-MAXINT)
#endif /* MININT */

#endif /* B2C_H_ */
