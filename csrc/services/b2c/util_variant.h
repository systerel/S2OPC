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
 * Utils to produce some Variants for basic C types: int, float, strings...
 */

#ifndef UTIL_VARIANT_H_
#define UTIL_VARIANT_H_

#include "constants.h"

#include "sopc_types.h"

/**
 * The returned Variant is malloced and shall be freed by the consumer
 *  (and only the following malloc, not the pnid, so don't use SOPC_*_Clear).
 */
constants__t_Variant_i util_variant__new_Variant_from_NodeId(SOPC_NodeId* pnid);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 */
constants__t_Variant_i util_variant__new_Variant_from_NodeClass(OpcUa_NodeClass ncl);

constants__t_Variant_i util_variant__new_Variant_from_QualifiedName(SOPC_QualifiedName* qn);

constants__t_Variant_i util_variant__new_Variant_from_LocalizedText(SOPC_LocalizedText* lt);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 */
constants__t_Variant_i util_variant__new_Variant_from_Indet(void);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 */
constants__t_Variant_i util_variant__new_Variant_from_Variant(SOPC_Variant* pvara);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 */
constants__t_Variant_i util_variant__new_Variant_from_Byte(uint8_t i);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 */
constants__t_Variant_i util_variant__new_Variant_from_uint32(uint32_t i);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 */
constants__t_Variant_i util_variant__new_Variant_from_int64(int64_t i);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 */
constants__t_Variant_i util_variant__new_Variant_from_int32(int32_t i);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 */
constants__t_Variant_i util_variant__new_Variant_from_double(double f);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 * The string is not copied.
 */
constants__t_Variant_i util_variant__new_Variant_from_ByteString(SOPC_ByteString buf);

/**
 * Quick and dirty print.
 * I don't do arrays...
 */
void util_variant__print_SOPC_Variant(SOPC_Variant* pvar);

#endif /* UTIL_VARIANT_H_ */
