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

/** \file sopc_askpass.c
 *
 * \brief A platform independent API to ask for passwords.
 */

#include <stdlib.h>

#include "sopc_askpass.h"
#include "sopc_assert.h"
#include "sopc_helper_string.h" // TODO: remove

/* TODO: Remove the environment variable and implement this function as described in its header file. */
bool SOPC_AskPass_FromTerminal(char** outPassword)
{
    SOPC_ASSERT(NULL != outPassword);
    /*
        we will free the password after calling ::SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd
        so we have to copy it.
    */
    char* _outPassword = getenv("TEST_PASSWORD_PRIVATE_KEY");
    *outPassword = SOPC_strdup(_outPassword); // Do a copy
    return NULL != *outPassword;
}
