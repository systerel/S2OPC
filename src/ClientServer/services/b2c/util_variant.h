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
 * Utils to produce some Variants from built-in types: int, float, strings...
 */

#ifndef UTIL_VARIANT_H_
#define UTIL_VARIANT_H_

#include "sopc_types.h"

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_NodeId(SOPC_NodeId* pnid, bool deepCopy);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_NodeClass(OpcUa_NodeClass ncl);

SOPC_Variant* util_variant__new_Variant_from_QualifiedName(SOPC_QualifiedName* qn, bool deepCopy);

SOPC_Variant* util_variant__new_Variant_from_LocalizedText(SOPC_LocalizedText* lt, bool deepCopy);
/* Asserts noOfRolePermissions > 0 */
SOPC_Variant* util_variant__new_Variant_from_RolePermissions(OpcUa_RolePermissionType* rolePermissionsArray,
                                                             int32_t noOfRolePermissions);

/**
 * On success, returns a deep copy of the input variant with new locales, and free the input variant.
 * On fail, returns the input variant.
 * If the caller is not interested in the result, he can provide NULL for the *success* parameter.
 */
SOPC_Variant* util_variant__set_PreferredLocalizedText_from_LocalizedText_Variant(SOPC_Variant** v,
                                                                                  char** preferredLocales,
                                                                                  bool* success);

// Same function as previous one but source variant is not modified and destination already allocated
bool util_variant__copy_PreferredLocalizedText_from_LocalizedText_Variant(SOPC_Variant* dest,
                                                                          const SOPC_Variant* src,
                                                                          char** preferredLocales);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_Indet(void);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_Variant(const SOPC_Variant* pvara, bool deepCopy);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_Bool(bool b);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_Byte(uint8_t i);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_uint32(uint32_t i);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_int64(int64_t i);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_int32(int32_t i);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_double(double f);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_ByteString(SOPC_ByteString* bs, bool deepCopy);

/**
 * The returned Variant is allocated and shall be deleted (or freed + cleared) by consumer
 */
SOPC_Variant* util_variant__new_Variant_from_ExtensionObject(SOPC_ExtensionObject* extObj, bool deepCopy);

/**
 * The \p source variant content is copied into \p dest applying:
 * - the preferred LocalIds if defined and source is a LocalizedText value
 * - the index range if defined and applicable to source value
 *
 * Note: in case an error occurred, the \p dest value is cleared abd set to a StatusCode value indicating the error.
 *
 * The returned status is either SOPC_STATUS_OK when \p dest value has been set from \p source value successfully,
 * otherwise the \p dest value has been set to an OPC UA Bad status indicating the error that occured.
 */
SOPC_ReturnStatus util_variant__copy_and_apply_locales_and_index_range(SOPC_Variant* destVal,
                                                                       const SOPC_Variant* source,
                                                                       char** preferredLocalesIds,
                                                                       const SOPC_NumericRange* indexRange);

#endif /* UTIL_VARIANT_H_ */
