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

#include "sopc_filesystem.h"
#include "sopc_macros.h"

#include <vm.h>

SOPC_FileSystem_CreationResult SOPC_FileSystem_mkdir(const char* directoryPath)
{
    P4_e_t res = vm_dir_create(directoryPath);
    switch (res)
    {
    case P4_E_OK:
        return SOPC_FileSystem_Creation_OK;
    case P4_E_PERM:
        return SOPC_FileSystem_Creation_Error_PathPermisionDenied;
    case P4_E_EXIST:
        return SOPC_FileSystem_Creation_Error_PathAlreadyExists;
    case P4_E_NOCONTAINER:
        return SOPC_FileSystem_Creation_Error_PathPrefixInvalid;
    case P4_E_NAME:
        return SOPC_FileSystem_Creation_Error_PathResolutionIssue;
    default:
        return SOPC_FileSystem_Creation_Error_UnknownIssue;
    }
}

SOPC_FileSystem_RemoveResult SOPC_FileSystem_rmdir(const char* directoryPath)
{
    P4_e_t res = vm_unlink(directoryPath, P4_UNLINK_DIR_ONLY);
    switch (res)
    {
    case P4_E_OK:
        return SOPC_FileSystem_Remove_OK;
    case P4_E_STATE:
        return SOPC_FileSystem_Remove_Error_PathNotEmpty;
    case P4_E_NOENT || P4_E_NAME || P4_E_INVAL || P4_E_NOCONTAINER:
        return SOPC_FileSystem_Remove_Error_PathInvalid;
    case P4_E_PERM:
        return SOPC_FileSystem_Remove_Error_PathPermisionDenied;
    default:
        return SOPC_FileSystem_Remove_Error_UnknownIssue;
    }
}

SOPC_FileSystem_GetDirResult SOPC_FileSystem_GetDirFilePaths(const char* directoryPath, SOPC_Array** ppFilePaths)
{
    // Not implemented
    SOPC_UNUSED_ARG(directoryPath);
    SOPC_UNUSED_ARG(ppFilePaths);

    return SOPC_FileSystem_GetDir_Error_UnknownIssue;
}
