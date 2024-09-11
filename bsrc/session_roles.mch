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
    session_roles

SEES
    constants,
    constants_statuscodes_bs

OPERATIONS
    p_roles <-- get_user_roles(p_user, p_locales) =
    PRE
        p_user : t_user_i &
        p_user : t_user &
        p_locales : t_LocaleIds_i &
        p_locales : t_LocaleIds
    THEN
        p_roles :: t_sessionRoles_i
    END

END
