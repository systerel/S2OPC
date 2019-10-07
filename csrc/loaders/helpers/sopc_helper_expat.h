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

#ifndef SOPC_HELPER_EXPAT_H_
#define SOPC_HELPER_EXPAT_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include "expat.h"

#define SKIP_TAG_LEN 256

typedef struct SOPC_HelperExpatCtx
{
    XML_Parser parser;

    char skip_tag[SKIP_TAG_LEN]; // If set, start_tag events are ignored until this tag is closed

    // 0 terminated buffer for the char data handler (a single piece of char
    // data in the XML can be broken across many callbacks).
    char* char_data_buffer;

    // strlen of the text in char_data_buffer
    size_t char_data_len;

    // allocated size of char_data_buffer, at least char_data_len + 1 (for the
    // NULL terminator).
    size_t char_data_cap;
} SOPC_HelperExpatCtx;

#ifdef UANODESET_LOADER_LOG
#define LOG(str) fprintf(stderr, "UANODESET_LOADER: %s:%d: %s\n", __FILE__, __LINE__, (str))
#define LOG_XML_ERROR(parser, str)                                                                \
    fprintf(stderr, "UANODESET_LOADER: %s:%d: at line %lu, column %lu: %s\n", __FILE__, __LINE__, \
            XML_GetCurrentLineNumber(parser), XML_GetCurrentColumnNumber(parser), (str))

#define LOGF(format, ...) fprintf(stderr, "UANODESET_LOADER: %s:%d: " format "\n", __FILE__, __LINE__, __VA_ARGS__)
#define LOG_XML_ERRORF(parser, format, ...)                                                               \
    fprintf(stderr, "UANODESET_LOADER: %s:%d: at line %lu, column %lu: " format "\n", __FILE__, __LINE__, \
            XML_GetCurrentLineNumber(parser), XML_GetCurrentColumnNumber(parser), __VA_ARGS__)
#else
#define LOG(str)
#define LOG_XML_ERROR(parser, str)
#define LOGF(format, ...)
#define LOG_XML_ERRORF(parser, format, ...)
#endif

#define LOG_MEMORY_ALLOCATION_FAILURE LOG("Memory allocation failure")

void SOPC_HelperExpat_CharDataReset(SOPC_HelperExpatCtx* ctx);

bool SOPC_HelperExpat_CharDataAppend(SOPC_HelperExpatCtx* ctx, const char* data, size_t len);

const char* SOPC_HelperExpat_CharDataStripped(SOPC_HelperExpatCtx* ctx);

/*
 * \brief Push a tag for which all events shall be skipped until tag is closed
 * Note: assertion failure is raised if a tag is already pushed
 */
void SOPC_HelperExpat_PushSkipTag(SOPC_HelperExpatCtx* ctx, const char* name);

/*
 * \brief Check if the skip tag is still active
 * \return true if skip tag is active, false otherwise
 */
bool SOPC_HelperExpat_IsSkipTagActive(SOPC_HelperExpatCtx* ctx);

/*
 * \brief Pop skip tag if it is the one pushed previously
 * \return true if skip tag is popped, false otherwise
 */
bool SOPC_HelperExpat_PopSkipTag(SOPC_HelperExpatCtx* ctx, const char* name);

#endif /* SOPC_HELPER_EXPAT_H_ */
