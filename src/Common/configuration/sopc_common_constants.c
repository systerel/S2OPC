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

#include "sopc_common_constants.h"

#include "sopc_atomic.h"

#include <assert.h>

bool SOPC_Internal_Common_Constants_RuntimeCheck(void)
{
    bool res = (sizeof(uintptr_t) == sizeof(void*));
    assert(res && "uintptr_t has not same size as void* which is expected for other language bindings");
    return res;
}

static SOPC_Common_EncodingConstants globalEncodingConfig = {
    .buffer_size = SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE,
    .receive_max_msg_size = SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH,
    .receive_max_nb_chunks = SOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS,
    .send_max_msg_size = SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH,
    .send_max_nb_chunks = SOPC_DEFAULT_SEND_MAX_NB_CHUNKS,

    .max_string_length = SOPC_DEFAULT_MAX_STRING_LENGTH,
    .max_array_length = SOPC_DEFAULT_MAX_ARRAY_LENGTH,
    .max_nested_diag_info = SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL,
    .max_nested_struct = SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL};

int32_t globalEncodingConfigSet = false;

SOPC_Common_EncodingConstants SOPC_Common_GetDefaultEncodingConstants(void)
{
    return (SOPC_Common_EncodingConstants){.buffer_size = SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE,
                                           .receive_max_msg_size = SOPC_DEFAULT_RECEIVE_MAX_MESSAGE_LENGTH,
                                           .receive_max_nb_chunks = SOPC_DEFAULT_RECEIVE_MAX_NB_CHUNKS,
                                           .send_max_msg_size = SOPC_DEFAULT_SEND_MAX_MESSAGE_LENGTH,
                                           .send_max_nb_chunks = SOPC_DEFAULT_SEND_MAX_NB_CHUNKS,

                                           .max_string_length = SOPC_DEFAULT_MAX_STRING_LENGTH,
                                           .max_array_length = SOPC_DEFAULT_MAX_ARRAY_LENGTH,
                                           .max_nested_diag_info = SOPC_DEFAULT_MAX_DIAG_INFO_NESTED_LEVEL,
                                           .max_nested_struct = SOPC_DEFAULT_MAX_STRUCT_NESTED_LEVEL};
}

const SOPC_Common_EncodingConstants* SOPC_Internal_Common_GetEncodingConstants(void)
{
    return &globalEncodingConfig;
}

bool SOPC_Common_SetEncodingConstants(SOPC_Common_EncodingConstants config)
{
    // Check same constraints that the one on default values

    if ((config.receive_max_msg_size != 0 && config.receive_max_msg_size < config.buffer_size) ||
        (config.send_max_msg_size != 0 && config.send_max_msg_size < config.buffer_size))
    {
        return false;
    }

    if ((0 == config.receive_max_msg_size && 0 == config.receive_max_nb_chunks) ||
        (0 == config.send_max_msg_size && 0 == config.send_max_nb_chunks))
    {
        return false;
    }

    if (!SOPC_Atomic_Int_Get(&globalEncodingConfigSet))
    {
        SOPC_Atomic_Int_Set(&globalEncodingConfigSet, true);

        // Enforce definition of max message size
        if (0 == config.send_max_msg_size)
        {
            assert(0 != config.send_max_nb_chunks);
            config.send_max_msg_size = config.send_max_nb_chunks * config.buffer_size;
        }
        if (0 == config.receive_max_msg_size)
        {
            assert(0 != config.receive_max_msg_size);
            config.receive_max_msg_size = config.receive_max_nb_chunks * config.buffer_size;
        }

        globalEncodingConfig = config;
        return true;
    }
    else
    {
        return false;
    }
}
