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

/**
 *  \file
 *
 *  \brief A platform independent API for file system operations.
 */

#ifndef SOPC_FILESYSTEM_H_
#define SOPC_FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>

typedef enum
{
    SOPC_FileSystem_Creation_OK = 0,
    SOPC_FileSystem_Creation_Error_PathAlreadyExists,
    SOPC_FileSystem_Creation_Error_PathPrefixInvalid,
    SOPC_FileSystem_Creation_Error_PathResolutionIssue,
    SOPC_FileSystem_Creation_Error_PathPermisionDenied,
    SOPC_FileSystem_Creation_Error_UnknownIssue
} SOPC_FileSystem_CreationResult;

typedef enum
{
    SOPC_FileSystem_Remove_OK = 0,
    SOPC_FileSystem_Remove_Error_PathNotEmpty,
    SOPC_FileSystem_Remove_Error_PathInvalid,
    SOPC_FileSystem_Remove_Error_PathPermisionDenied,
    SOPC_FileSystem_Remove_Error_UnknownIssue
} SOPC_FileSystem_RemoveResult;

/**
 * \brief Request to create a directory with the given path in the file system.
 *        Only the last item of the path can be created, the path prefix shall exist.
 *
 * \param directoryPath    The directory path for which last item (regarding path delimiter) is the directory to create.
 *                         Path delimiter is still dependent on O.S. but '/' path delimiter should be supported for
 *                         relative paths.
 *
 * \return                 SOPC_Creation_OK if directory creation succeeded or SOPC_Error_* value in other
 *                         cases
 */
SOPC_FileSystem_CreationResult SOPC_FileSystem_mkdir(const char* directoryPath);

/**
 * \brief Request to delete a directory with the given path in the file system.
 *        Only empty directory can be deleted.
 *
 * \param directoryPath    The directory path in which last item (regarding path delimiter) is the directory to remove.
 *                         Path delimiter is still dependent on O.S. but '/' path delimiter should be supported for
 *                         relative paths.
 *
 * \return                 SOPC_Remove_OK if directory creation succeeded or SOPC_Error_* value in other
 *                         cases
 */
SOPC_FileSystem_RemoveResult SOPC_FileSystem_rmdir(const char* directoryPath);

/**
 * \brief Simulates a file open using an memory buffer rather than an actual file.
 * refer to "fmemopen" linux manpage for precise specification
 * \param  buf Non-NULL pointer to the in/out buffer
 * \param  size The size of the \p buf
 * \param  opentype The file mode: with the same convention as usual "fopen".
 * \warning TODO This feature is only available on LINUX implementation. A Rework of the API might be needed
 *      for full portability.
 */
FILE* SOPC_FileSystem_fmemopen(void* buf, size_t size, const char* opentype);

#endif // SOPC_FILESYSTEM_H_
