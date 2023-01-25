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

#include <conio.h>

#include "sopc_askpass.h"
#include "sopc_mem_alloc.h"

// additional character to catch end character '\n' or '\r'
#define PWD_BUFFER_ADDITIONAL_CHARACTERS 1

bool SOPC_AskPass_CustomPromptFromTerminal(char* prompt, char** outPassword)
{
    if (NULL == prompt || NULL == outPassword)
    {
        return false;
    }

    char* pwd = SOPC_Calloc(sizeof(char), SOPC_PASSWORD_MAX_LENGTH + PWD_BUFFER_ADDITIONAL_CHARACTERS);
    if (NULL == pwd)
    {
        return false;
    }

    // Display prompt
    _cputs(prompt);

    bool end = false;
    bool stop = false;
    int index = 0;
    while (!end && !stop && index < SOPC_PASSWORD_MAX_LENGTH + PWD_BUFFER_ADDITIONAL_CHARACTERS)
    {
        pwd[index] = (char) _getch();
        if (pwd[index] == '\003')
        { // EOF
            stop = true;
        }
        else if (pwd[index] == '\n' || pwd[index] == '\r')
        {
            end = true;
        }
        else if (pwd[index] == '\b' && index > 0) // backspace
        {
            index--;
        }
        else
        {
            index++;
        }
    }

    if (!end || stop)
    {
        SOPC_Free(pwd);
        pwd = NULL;
    }
    else
    {
        pwd[index] = '\0';
        *outPassword = pwd;
    }

    _putch('\r');
    _putch('\n');

    return (NULL != pwd);
}
