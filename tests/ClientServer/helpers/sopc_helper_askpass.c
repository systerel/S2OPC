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

/** \file sopc_helper_askpass.c
 *
 * \brief An API to ask for passwords.
 */

#include <stdio.h>
#include <stdlib.h>

#include "sopc_assert.h"
#include "sopc_helper_askpass.h"
#include "sopc_helper_string.h"

bool SOPC_TestHelper_AskPass_FromEnv(char** outPassword)
{
    SOPC_ASSERT(NULL != outPassword);
    /*
        We have to make a copy here because in any case, we will free the password and not distinguish if it come
        from environement or terminal after calling ::SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile_WithPwd
    */
    char* _outPassword = getenv(PASSWORD_ENV_NAME);
    if (NULL == _outPassword)
    {
        printf("<SOPC_TestHelper_AskPass_FromEnv: The following environment variable is missing: %s\n",
               PASSWORD_ENV_NAME);
        return false;
    }
    *outPassword = SOPC_strdup(_outPassword); // Do a copy
    return NULL != *outPassword;
}
