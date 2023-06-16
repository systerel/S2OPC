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

#if !defined(SOPC_WITH_EXPAT) || SOPC_WITH_EXPAT
#include <stdio.h>
#include <string.h>

#include "sopc_assert.h"
#include "sopc_helper_expat.h"
#include "sopc_mem_alloc.h"

static bool is_whitespace_char(char c)
{
    switch (c)
    {
    case ' ':
    case '\t':
    case '\n':
        return true;
    default:
        return false;
    }
}

static const char* strip_whitespace(char* s, size_t len)
{
    char* end = s + len - 1;
    for (; ((*s) != '\0') && is_whitespace_char(*s); ++s)
    {
    }
    for (; (end >= s) && is_whitespace_char(*end); --end)
    {
    }
    *(end + 1) = '\0';
    return s;
}

bool SOPC_HelperExpat_CharDataAppend(SOPC_HelperExpatCtx* ctx, const char* data, size_t len)
{
    size_t required_cap = ctx->char_data_len + len + 1;

    if (required_cap > ctx->char_data_cap)
    {
        size_t cap = 2 * ctx->char_data_cap;

        while (cap < required_cap)
        {
            cap = 2 * cap;
        }

        char* dataBuff = SOPC_Realloc(ctx->char_data_buffer, ctx->char_data_cap * sizeof(char), cap * sizeof(char));

        if (dataBuff == NULL)
        {
            LOG_MEMORY_ALLOCATION_FAILURE;
            return false;
        }

        ctx->char_data_buffer = dataBuff;
        ctx->char_data_cap = cap;
    }

    memcpy(ctx->char_data_buffer + ctx->char_data_len, data, len);
    ctx->char_data_len += len;
    ctx->char_data_buffer[ctx->char_data_len] = '\0';

    return true;
}

void SOPC_HelperExpat_CharDataReset(SOPC_HelperExpatCtx* ctx)
{
    ctx->char_data_buffer[0] = '\0';
    ctx->char_data_len = 0;
}

const char* SOPC_HelperExpat_CharDataStripped(SOPC_HelperExpatCtx* ctx)
{
    return strip_whitespace(ctx->char_data_buffer, ctx->char_data_len);
}

void SOPC_HelperExpat_PushSkipTag(SOPC_HelperExpatCtx* ctx, const char* name)
{
    SOPC_ASSERT(0 == ctx->skip_tag[0]);
    SOPC_ASSERT(strlen(name) < SKIP_TAG_LEN);
    strncpy(ctx->skip_tag, name, SKIP_TAG_LEN - 1);
}

bool SOPC_HelperExpat_IsSkipTagActive(SOPC_HelperExpatCtx* ctx)
{
    return ctx->skip_tag[0] != 0;
}

bool SOPC_HelperExpat_PopSkipTag(SOPC_HelperExpatCtx* ctx, const char* name)
{
    if (SOPC_HelperExpat_IsSkipTagActive(ctx) && strcmp(ctx->skip_tag, name) == 0)
    {
        ctx->skip_tag[0] = 0;
        return true;
    }
    return false;
}

const char* SOPC_HelperExpat_GetAttr(SOPC_HelperExpatCtx* ctx, const char* attrName, const XML_Char** attrs)
{
    for (size_t i = 0; attrs[i]; ++i)
    {
        const char* attr = attrs[i];

        if (strcmp(attrName, attr) == 0)
        {
            const char* attr_val = attrs[++i];

            if (attr_val == NULL)
            {
                LOG_XML_ERRORF(ctx->parser, "Missing value for %s attribute", attrName);
                return NULL;
            }
            return attr_val;
        }
        else
        {
            ++i; // Skip value of unknown attribute
        }
    }
    return NULL;
}

#endif /* SOPC_WITH_EXPAT */
