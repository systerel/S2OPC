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
 *  \brief A log manager providing circular logging, multiple logging categories and levels with thread-safe accesses.
 */

#ifndef SOPC_LOG_MANAGER_H_
#define SOPC_LOG_MANAGER_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct SOPC_Log_Instance SOPC_Log_Instance;

typedef enum
{
    SOPC_LOG_LEVEL_ERROR = 0,
    SOPC_LOG_LEVEL_WARNING = 1,
    SOPC_LOG_LEVEL_INFO = 2,
    SOPC_LOG_LEVEL_DEBUG = 3
} SOPC_Log_Level;

/*
 * \brief Initializes the logger manager: generate unique file name prefix for execution
 * */
void SOPC_Log_Initialize(void);

/*
 * \brief Creates a new log file and log instance and prints the starting timestamp
 *
 * \param logDirPath   Absolute or relative path of the directory to be used for logs (shall exist and terminate with
 * directory separator)
 * \param logFileName  The file name to be used without extension. A prefix, an integer suffix and extension will be
 * added automatically.
 * \param category     A category name if the log file is used for several categories or NULL. Truncated if more than 9
 * characters.
 * \param maxBytes     A maximum amount of bytes by log file before opening a new file incrementing the integer suffix.
 * It is a best effort value (amount verified after each print).
 * \param maxFiles     A maximum number of files to be used, when reached the older log file is overwritten
 * (starting with *_00001.log)
 *
 * \return             The log instance to be used to add traces
 * */
SOPC_Log_Instance* SOPC_Log_CreateInstance(
    const char* logDirPath,
    const char* logFileName,
    const char* category,
    uint32_t maxBytes,  // New file created when maxBytes reached (after printing latest trace)
    uint16_t maxFiles); // Old logs overwritten when maxFiles reached

/*
 * \brief Creates a new log instance using the same log file than existing log instance and prints the starting
 * timestamp. It provides the way to have several categories with different levels of log in the same log file.
 *
 * \param pLogInst  An existing log instance used to print in the same log file
 * \param category  Category for the new log instance in the log file (should be unique in log file)
 *
 * \return          The log instance to be used to add traces
 */
SOPC_Log_Instance* SOPC_Log_CreateInstanceAssociation(SOPC_Log_Instance* pLogInst, const char* category);

/*
 * \brief Defines the active log level for the given log instance (default: ERROR):
 * - ERROR: display only ERROR level
 * - WARNING: display ERROR + WARNING levels
 * - INFO: display ERROR + WARNING + INFO levels
 * - DEBUG: display ERROR + WARNING + INFO + DEBUG levels
 *
 * \param pLogInst  An existing log instance
 * \param level     The level to be activated for the log instance
 */
bool SOPC_Log_SetLogLevel(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level);

/*
 * \brief Activates the console output for logged traces (same active level as log file)
 *
 * \param pLogInst  An existing log instance
 * \param activate  Flag to activate / deactivate the console output
 */
bool SOPC_Log_SetConsoleOutput(SOPC_Log_Instance* pLogInst, bool activate);

/*
 * \brief Returns the file path prefix for the given log instance.
 * It complies with the following format: <logDirPath><startExecutionDate><logFileName>_
 * - <logDirPath>: the path provided to SOPC_Log_CreateInstance
 * - <startExecutionDate>: the starting execution date set by SOPC_Log_Initialize (or UNINIT_LOG if not initialized)
 * - <logFileName>: the log file name provided to SOPC_Log_CreateInstance
 * Note: the complete file path is then the returned prefix followed by <NNNNN>.log with N digits starting to 00001
 *
 * \param pLogInst  An existing log instance
 * \return          The generic file path prefix of the log files for the given instance (to be deallocated by caller)
 */
char* SOPC_Log_GetFilePathPrefix(SOPC_Log_Instance* pLogInst);

/*
 * \brief Logs a trace with the given level
 *
 * \param pLogInst  An existing log instance already started
 * \param level     The log level corresponding to the given trace
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Log_Trace(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, const char* format, ...);

/*
 * \brief Logs a trace with the given level
 *
 * \param pLogInst  An existing log instance already started
 * \param level     The log level corresponding to the given trace
 * \param format    String specifying how subsequent arguments are converted for output
 * \param args      Arguments used by the string specifying the output
 */
void SOPC_Log_VTrace(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, const char* format, va_list args);

/*
 * \brief Stops allowing to log traces in the given log instance. Log file is closed when last log instance is stopped.
 *
 * \param ppLogInst  An existing log instance already started. Pointer set to NULL after call.
 */
void SOPC_Log_ClearInstance(SOPC_Log_Instance** ppLogInst);

/*
 * \brief Clears the logger manager: clear unique file name prefix for execution
 * */
void SOPC_Log_Clear(void);

#endif /* SOPC_LOG_MANAGER_H_ */
