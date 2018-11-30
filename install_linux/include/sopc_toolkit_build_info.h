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
.toolkitSrcCommit = "9863bbebb3a9909af0e317bc1e1070775717bd38",
.toolkitDockerId = "sha256:a5c83c23530d5f845927f53926eee4546d473c5ed1f44e0a3e52f9942726a0a5",
.toolkitBuildDate = "2018-11-30",
};
#endif
