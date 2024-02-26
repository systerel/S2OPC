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
 * \file sopc_log_manager.h
 *
 *  \brief Provide circular logging.
 */

#ifndef SOPC_CIRCULAR_LOG_FILE_H_
#define SOPC_CIRCULAR_LOG_FILE_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct SOPC_CircularLogFile SOPC_CircularLogFile;

/**
 * \brief structure containing the file system log configuration
 */
typedef struct SOPC_CircularLogFile_Configuration
{
    const char* logDirPath;  /**< path of the log directory (can be empty) */
    const char* logFileName; /**< Name of log file (without extension) */
    uint32_t logMaxBytes;    /**< max bytes per log file */
    uint16_t logMaxFiles;    /**< max number of log files */
} SOPC_CircularLogFile_Configuration;

/**
 * Create a circular output text file.
 * @param config The files configuration
 * @return A new allocated object, that must be freed by caller using ::SOPC_CircularLogFile_Delete after use
 */
SOPC_CircularLogFile* SOPC_CircularLogFile_Create(const SOPC_CircularLogFile_Configuration* config);

/**
 * \brief Get the name of the current log file.
 * \param pFile  An existing circular file
 * \return Name of current output log file or NULL if not applicable.
 * The returned value must be deallocated by the caller.
 */
char* SOPC_CircularLogFile_GetFileName(const SOPC_CircularLogFile* pFile);

/**
 * Writes a line in the circular files, and if the limit size is reached:
 * - close current file
 * - creates and prepare next file
 * @param pFile The circular file to write into
 * @param line The line to write in files.
 */
void SOPC_CircularLogFile_PutLine(SOPC_CircularLogFile* pFile, const char* line);

/**
 * Clears a circular log file object, previously created by ::SOPC_CircularLogFile_Create
 * @param ppFile The file to clear
 */
void SOPC_CircularLogFile_Delete(SOPC_CircularLogFile** ppFile);

#endif /* SOPC_CIRCULAR_LOG_FILE_H_ */
