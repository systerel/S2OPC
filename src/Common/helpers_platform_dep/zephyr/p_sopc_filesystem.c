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

#include "sopc_assert.h"
#include "sopc_filesystem.h"

SOPC_FileSystem_CreationResult SOPC_FileSystem_mkdir(const char* directoryPath)
{
    return SOPC_FileSystem_Creation_Error_UnknownIssue;
}

SOPC_FileSystem_RemoveResult SOPC_FileSystem_rmdir(const char* directoryPath)
{
    return SOPC_FileSystem_Creation_Error_UnknownIssue;
}

SOPC_FileSystem_GetDirResult SOPC_FileSystem_GetDirFilePaths(const char* directoryPath, SOPC_Array** ppFilePaths)
{
    (void) directoryPath;
    (void) ppFilePaths;
    SOPC_ASSERT(false && "NOT IMPLEMENTED");
    return SOPC_FileSystem_Creation_OK;
}
