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
.toolkitSrcCommit = "aa42cdee98a49c83e49669784fddf78467fa2d23",
.toolkitDockerId = "sha256:bbbb6dad58c377f0d341279687e84b7d6d69972d7032f13885f0af4502094c2e",
.toolkitBuildDate = "2019-09-18",
};
#endif
