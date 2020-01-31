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

#include <direct.h>
#include <errno.h>

SOPC_FileSystem_CreationResult SOPC_FileSystem_mkdir(const char* directoryPath)
{
    int res = _mkdir(directoryPath);
    if (res == 0)
    {
        return SOPC_FileSystem_Creation_OK;
    }
    else if (res == -1)
    {
        switch (errno)
        {
        case ENOENT:
            return SOPC_FileSystem_Creation_Error_PathPrefixInvalid;
        case EEXIST:
            return SOPC_FileSystem_Creation_Error_PathAlreadyExists;
        case EACCES:
            return SOPC_FileSystem_Creation_Error_PathPermisionDenied;
        default:
            return SOPC_FileSystem_Creation_Error_PathResolutionIssue;
        }
    }
    else
    {
        return SOPC_FileSystem_Creation_Error_UnknownIssue;
    }
}

SOPC_FileSystem_RemoveResult SOPC_FileSystem_rmdir(const char* directoryPath)
{
    int res = _rmdir(directoryPath);
    if (res == 0)
    {
        return SOPC_FileSystem_Remove_OK;
    }
    else if (res == -1)
    {
        switch (errno)
        {
        case ENOENT:
            return SOPC_FileSystem_Remove_Error_PathInvalid;
        case ENOTEMPTY:
            return SOPC_FileSystem_Remove_Error_PathNotEmpty;
        case EACCES:
            return SOPC_FileSystem_Remove_Error_PathPermisionDenied;
        default:
            return SOPC_FileSystem_Remove_Error_UnknownIssue;
        }
    }
    else
    {
        return SOPC_FileSystem_Remove_Error_UnknownIssue;
    }
}
