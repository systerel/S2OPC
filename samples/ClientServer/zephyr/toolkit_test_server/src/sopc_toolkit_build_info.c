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

#include "sopc_toolkit_build_info.h"
#include "sopc_version.h"

const SOPC_Build_Info sopc_client_server_build_info = {
    .buildVersion = SOPC_TOOLKIT_VERSION,
    .buildSrcCommit = "f417af8e40c2c7c82102fe92a26cb88df9afc131",
    .buildDockerId = "sha256:a2ff15f9670d471d04872ea86f8d01502f7741c24669b024a9e16f92966a8c18",
    .buildBuildDate = "2021-03-01",
};

SOPC_Build_Info SOPC_ClientServer_GetBuildInfo(void)
{
    return sopc_client_server_build_info;
}
