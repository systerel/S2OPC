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

/**
 * \file sopc_assert.h
 * \brief Redirection of code assertions depending on user needs.
 * Assert provides one of the following indirection for the assertion "SOPC_ASSERT" macro, depending on build flags:
 * - WITH_NO_ASSERT
 *      The assertion is casted to void, so that tested variables are seen "used" by the compiler.
 *      This options aims at reducing code footprint by removing assertions. Note that this implies that SOPC_ASSERT
 *      calls must have no side-effect.
 * - WITH_USER_ASSERT
 *      The user can call (once) SOPC_Assert_Set_UserCallback to route failed asserts its own handling.
 *      This intends at providing a safety software event to set Fail-Safe behavior.
 *      If SOPC_Assert_Set_UserCallback is not called, or if the user callback returns, the failed assertion
 *      will simply call system assert with "false"
 * - Otherwise
 *      By default the system assertion is used (assert, see <assert.h>), but
 */

#ifndef SOPC_ASSERT_H_
#define SOPC_ASSERT_H_

/**
 * These macros are required to convert at build_time the __LINE__ into a string
 */
#define SOPC_PP_XSTR(x) #x
#define SOPC_PP_STR(x) SOPC_PP_XSTR(x)

#ifndef WITH_MINIMAL_FOOTPRINT
#define WITH_MINIMAL_FOOTPRINT 0
#endif

/**
 * SOPC_ASSERT_CONTEXT expands the calling context, only if WITH_MINIMAL_FOOTPRINT option is not set
 */
#if WITH_MINIMAL_FOOTPRINT
#define SOPC_ASSERT_CONTEXT(context) ""
#else
#define SOPC_ASSERT_CONTEXT(context) __FILE__ ":" SOPC_PP_STR(__LINE__) " => " #context
#endif

/**
 * SOPC_ASSERT implementation depends on build options (see module description)
 */
#ifdef WITH_NO_ASSERT

#define SOPC_ASSERT(c) ((void) (c))

#elif defined(WITH_USER_ASSERT)

#define SOPC_ASSERT(c)                                   \
    do                                                   \
    {                                                    \
        if (!(c))                                        \
        {                                                \
            SOPC_Assert_Failure(SOPC_ASSERT_CONTEXT(c)); \
        }                                                \
    } while (0)

#else

#include <assert.h>

#define SOPC_ASSERT assert

#endif

/**
 * \brief
 * User-defined implementation of Fail-Safe behavior.
 * This method shall be provided when WITH_USER_ASSERT is defined.
 * \param context The context of failed assertion.
 *      Note that when WITH_MINIMAL_FOOTPRINT option is set, this parameter will
 *      always be empty.
 * \post This function has no constraint return (may return or not).
 */
typedef void SOPC_Assert_UserCallback(const char* context);

/**
 * \brief
 *  Define the user event to call in case of Assert failure.
 * \param callback Callback event.
 */
void SOPC_Assert_Set_UserCallback(SOPC_Assert_UserCallback* callback);

/**
 * \brief
 * Called by SOPC_ASSERT in case of failure.
 * \param context The context of failed assertion.
 * \post This function does not return
 */
void SOPC_Assert_Failure(const char* context);

#endif /* SOPC_ASSERT_H_ */
