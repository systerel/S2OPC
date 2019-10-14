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

/** \file
 *
 * \brief Include this file and call SOPC_IEEE_Check to assert IEEE-754 compliance.
 *
 */

#ifndef SOPC_IEEE_CHECK_H_
#define SOPC_IEEE_CHECK_H_

#include <float.h>
#include <stdbool.h>

/**
 * \brief Checks the floating-point compliance to IEEE-754 standard.
 *
 * Most of the verifications are done at compile time with \#if statements,
 * but some of them must be done at execution time.
 *
 * \return  true if floating-point arithmetics are compliant to IEEE-754.
 */
bool SOPC_IEEE_Check(void);

#if FLT_MAX_EXP != 128
#error "Compiler float definition differs from IEEE-754 standard"
#endif

#if FLT_MIN_EXP != (-125)
#error "Compiler float definition differs from IEEE-754 standard"
#endif

#if FLT_MANT_DIG != 24
#error "Compiler float definition differs from IEEE-754 standard"
#endif

#if DBL_MAX_EXP != 1024
#error "Compiler double definition differs from IEEE-754 standard"
#endif

#if DBL_MIN_EXP != (-1021)
#error "Compiler double definition differs from IEEE-754 standard"
#endif

#if DBL_MANT_DIG != 53
#error "Compiler double definition differs from IEEE-754 standard"
#endif

#endif
