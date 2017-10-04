/*
 *  Copyright (C) 2017 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 *
 * Utils to produce some Variants for basic C types: int, float, strings...
 */

#ifndef _util_variant_h
#define _util_variant_h


#include "constants.h"

#include "sopc_toolkit_constants.h"
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
constants__t_Variant_i util_variant__new_Variant_from_uint32(uint32_t i);

/**
 * The returned Variant is malloced and shall be freed by the consumer.
 */
constants__t_Variant_i util_variant__new_Variant_from_int64(int64_t i);

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
void util_variant__print_SOPC_Variant(SOPC_Variant *pvar);

#endif /* _util_variant_h */
