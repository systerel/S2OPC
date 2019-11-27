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

#ifndef _sopc_toolkit_build_info_h
#define _sopc_toolkit_build_info_h
#include "sopc_version.h"
#include "sopc_user_app_itf.h"
#include "sopc_version.h"
const SOPC_Build_Info toolkit_build_info = {
.toolkitVersion = SOPC_TOOLKIT_VERSION,
.toolkitSrcCommit = "dc930d0824df1c82fc2c19fd2415210a414dfd6c",
.toolkitDockerId = "sha256:c1958a853af652ee12a74d6eee23e7af746711e10ff1e2a58453cf1e21f85b49",
.toolkitBuildDate = "2019-11-27",
};
#endif
