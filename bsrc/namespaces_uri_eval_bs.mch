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

MACHINE
    namespaces_uri_eval_bs

SEES
    constants

ABSTRACT_VARIABLES
    a_variant_value_uri

INVARIANT
    a_variant_value_uri : t_Variant +-> t_NamespaceUri

INITIALISATION
    a_variant_value_uri :: t_Variant +-> t_NamespaceUri

OPERATIONS
    p_bres <-- compare_namespaceUri_and_value_uri(p_namespaceUri, p_variant) =
    PRE
        p_namespaceUri : t_NamespaceUri &
        p_variant : t_Variant_i &
        p_variant : t_Variant
    THEN
        p_bres := bool(a_variant_value_uri(p_variant) = p_namespaceUri)
    END

END
