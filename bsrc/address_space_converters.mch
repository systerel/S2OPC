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
    address_space_converters

SEES
    address_space, constants

OPERATIONS
    var <-- conv_NodeId_to_Variant(nid) =
    PRE
        nid : t_NodeId_i
    THEN
        var :: t_Variant_i
    END
    ;
    var <-- conv_NodeClass_to_Variant(nc) = 
    PRE
        nc : t_NodeId_i
    THEN
        var :: t_Variant_i
    END    

END
