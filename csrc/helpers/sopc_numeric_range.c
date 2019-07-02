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

#include "sopc_numeric_range.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "sopc_array.h"
#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"

typedef enum
{
    TOKEN_START,
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_DIGIT,
    TOKEN_COMMA,
    TOKEN_COLON,
} token_type_t;

struct parse_ctx_t
{
    const char* data;
    size_t data_len;

    size_t idx;

    token_type_t last_token;
    size_t token_len;
};

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool lex_digit(struct parse_ctx_t* ctx)
{
    ctx->token_len = 0;

    while (ctx->idx < ctx->data_len && is_digit(ctx->data[ctx->idx]))
    {
        ctx->token_len++;
        ctx->idx++;
    }

    return true;
}

static token_type_t lex_helper(struct parse_ctx_t* ctx)
{
    if (ctx->last_token == TOKEN_EOF)
    {
        return TOKEN_EOF;
    }

    if (ctx->idx == ctx->data_len)
    {
        ctx->last_token = TOKEN_EOF;
        ctx->token_len = 0;
        return TOKEN_EOF;
    }

    const char c = ctx->data[ctx->idx];

    if (is_digit(c))
    {
        lex_digit(ctx);
    }
    else if (c == ':')
    {
        ctx->last_token = TOKEN_COLON;
        ctx->token_len = 1;
        ctx->idx++;
    }
    else if (c == ',')
    {
        ctx->last_token = TOKEN_COMMA;
        ctx->token_len = 1;
        ctx->idx++;
    }
    else
    {
        // Unknown token
        ctx->last_token = TOKEN_ERROR;
        ctx->token_len = 0;
    }

    return ctx->last_token;
}

static token_type_t lex(struct parse_ctx_t* ctx, bool consume)
{
    size_t idx = ctx->idx;
    token_type_t res = lex_helper(ctx);

    if (!consume)
    {
        ctx->idx = idx;
    }

    return res;
}

static bool parse_index(struct parse_ctx_t* ctx, uint32_t* val)
{
    char buf[21];
    const char* start = ctx->data + ctx->idx;
    lex_digit(ctx);

    if (ctx->token_len == 0 || ctx->token_len >= (sizeof(buf) / sizeof(char)))
    {
        return false;
    }

    memcpy(buf, start, sizeof(char) * ctx->token_len);
    buf[ctx->token_len] = '\0';

    return SOPC_strtouint32_t(buf, val, 10, '\0') == SOPC_STATUS_OK;
}

static bool parse_dimension(struct parse_ctx_t* ctx, SOPC_Dimension* result)
{
    if (!parse_index(ctx, &result->start))
    {
        return false;
    }

    if (lex(ctx, false) == TOKEN_COLON)
    {
        lex(ctx, true);

        if (!parse_index(ctx, &result->end) || result->end <= result->start)
        {
            return false;
        }
    }
    else
    {
        result->end = result->start;
    }

    return true;
}

static SOPC_ReturnStatus parse_one_dimension(struct parse_ctx_t* ctx, SOPC_Array* dimensions, bool* has_more)
{
    size_t dim_idx = SOPC_Array_Size(dimensions);

    if (!SOPC_Array_Append_Values(dimensions, NULL, 1))
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_Dimension* dim = SOPC_Array_Get_Ptr(dimensions, dim_idx);

    if (!parse_dimension(ctx, dim))
    {
        return SOPC_STATUS_NOK;
    }

    token_type_t next = lex(ctx, true);

    if (next == TOKEN_COMMA)
    {
        *has_more = true;
        return SOPC_STATUS_OK;
    }
    else if (next == TOKEN_EOF)
    {
        *has_more = false;
        return SOPC_STATUS_OK;
    }
    else
    {
        return SOPC_STATUS_NOK;
    }
}

static SOPC_ReturnStatus parse_dimensions(struct parse_ctx_t* ctx, SOPC_Array* dimensions)
{
    for (bool has_more = true; has_more;)
    {
        SOPC_ReturnStatus status = parse_one_dimension(ctx, dimensions, &has_more);

        if (status != SOPC_STATUS_OK)
        {
            return status;
        }
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_NumericRange_Parse(const char* text, SOPC_NumericRange** result)
{
    struct parse_ctx_t ctx = {
        .data = text,
        .data_len = strlen(text),
        .idx = 0,
        .last_token = TOKEN_START,
        .token_len = 0,
    };

    SOPC_Array* dimensions = SOPC_Array_Create(sizeof(SOPC_Dimension), 1, NULL);
    SOPC_NumericRange* range = SOPC_Calloc(1, sizeof(SOPC_NumericRange));

    if (dimensions == NULL || range == NULL)
    {
        SOPC_Array_Delete(dimensions);
        SOPC_Free(range);
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    SOPC_ReturnStatus status = parse_dimensions(&ctx, dimensions);

    range->n_dimensions = SOPC_Array_Size(dimensions);

    if (status == SOPC_STATUS_OK && range->n_dimensions == 0)
    {
        status = SOPC_STATUS_NOK;
    }

    if (status != SOPC_STATUS_OK)
    {
        SOPC_Array_Delete(dimensions);
        SOPC_Free(range);
        return status;
    }

    range->dimensions = SOPC_Array_Into_Raw(dimensions);
    *result = range;

    return SOPC_STATUS_OK;
}

void SOPC_NumericRange_Delete(SOPC_NumericRange* range)
{
    if (range == NULL)
    {
        return;
    }

    SOPC_Free(range->dimensions);
    SOPC_Free(range);
}
