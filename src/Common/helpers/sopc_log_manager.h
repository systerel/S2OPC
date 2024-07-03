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

#include "sopc_circular_log_file.h"

typedef struct SOPC_Log_Instance SOPC_Log_Instance;

typedef enum SOPC_Log_Level
{
    SOPC_LOG_LEVEL_ERROR = 0,
    SOPC_LOG_LEVEL_WARNING = 1,
    SOPC_LOG_LEVEL_INFO = 2,
    SOPC_LOG_LEVEL_DEBUG = 3
} SOPC_Log_Level;

/**
 * \brief Log event callback.
 * \param[in] timestampUtc  String pointer containing the event timestamp (UTC time)
 * \param[in] category  String pointer containing the category. Can be NULL, when
 *                      not related to any category,
 * \param[in] level     The level of the log (Already filtered out if below current log level),
 * \param[in] line      Non-null string pointer, containing the full log line,
 *                      including NULL-terminating character but excluding any newline character,
 *                      so that it can be specificly defined for each platform.
 *                      The line has already been filtered (level) and formatted by logger core.
 *                      In all cases, the line is truncated to SOPC_Log_UserMaxLogLen characters.
 * */
typedef void SOPC_Log_UserDoLog(const char* timestampUtc,
                                const char* category,
                                const SOPC_Log_Level level,
                                const char* const line);

/**
 * \brief structure containing the file log configuration
 */
typedef struct SOPC_LogSystem_File_Configuration
{
    const char* logDirPath; /**< path of the log directory ending with directory separator if not empty */
    uint32_t logMaxBytes;   /**< max bytes per log file */
    uint16_t logMaxFiles;   /**< max number of log files */
} SOPC_LogSystem_File_Configuration;

/**
 * \brief structure containing the user system log configuration
 */
typedef struct SOPC_LogSystem_User_Configuration
{
    SOPC_Log_UserDoLog* doLog; /**< Log event user callback */
} SOPC_LogSystem_User_Configuration;

/**
 * \brief log system discriminant
 */
typedef enum SOPC_Log_System
{
    SOPC_LOG_SYSTEM_FILE,  /**< file system logger */
    SOPC_LOG_SYSTEM_USER,  /**< user-implemented system logger */
    SOPC_LOG_SYSTEM_NO_LOG /**< no system logger */
} SOPC_Log_System;

/**
 * \brief Provides possible logging configurations
 */
typedef union SOPC_Log_SystemConfiguration
{
    SOPC_LogSystem_File_Configuration fileSystemLogConfig; /**< log file system configuration */
    SOPC_LogSystem_User_Configuration userSystemLogConfig; /**< log user system configuration */
} SOPC_Log_SystemConfiguration;

/**
 * \brief Defines logging configuration
 */
typedef struct SOPC_Log_Configuration
{
    SOPC_Log_Level logLevel;   /**< default log level */
    SOPC_Log_System logSystem; /**< discriminant of the log system configuration used in \a logSysConfig */
    SOPC_Log_SystemConfiguration logSysConfig; /**< The specific parameters of logging (depending on \a logSystem) */
} SOPC_Log_Configuration;

/**
 * \brief Creates a new log file and log instance and prints the starting timestamp
 *
 * \param pConf        A non-NULL pointer to the file instance configuration
 * \param category     A category name if the log file is used for several categories or NULL. Truncated if more than 9
 * characters.
 *
 * \return             The log instance to be used to add traces
 * */
SOPC_Log_Instance* SOPC_Log_CreateFileInstance(const SOPC_CircularLogFile_Configuration* pConf, const char* category);

/**
 * \brief Creates a new log instance for user mode
 *
 * \param category     A category name if the log file is used for several categories or NULL. Truncated if more than 9
 * characters.
 * \param logCallback  The user logging callback. If set to NULL, no user event is called.
 *
 * \return             The log instance to be used to add traces
 * */
SOPC_Log_Instance* SOPC_Log_CreateUserInstance(const char* category, SOPC_Log_UserDoLog* logCallback);

/**
 * \brief Creates a new log instance using the same log file than existing log instance and prints the starting
 * timestamp. It provides the way to have several categories with different levels of log in the same log file.
 *
 * \param pLogInst  An existing log instance used to print in the same log file
 * \param category  Category for the new log instance in the log file (should be unique in log file)
 *
 * \return          The log instance to be used to add traces
 * \note            The new instance must always be cleared before the parent reference instance.
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

/**
 * \brief getter for the log level of an instance
 *
 * \param pLogInst  An existing log instance
 *
 * \return the log level for the specified instance
 */
SOPC_Log_Level SOPC_Log_GetLogLevel(SOPC_Log_Instance* pLogInst);

/**
 * \brief Activates the console output for logged traces (same active level as log file)
 *
 * \param pLogInst  An existing log instance
 * \param activate  Flag to activate / deactivate the console output
 */
bool SOPC_Log_SetConsoleOutput(SOPC_Log_Instance* pLogInst, bool activate);

/**
 * \brief Get the name of the current log file.
 * \param pLogInst  An existing log instance
 * \return Name of current output log file or NULL if not applicable.
 * The returned value must be deallocated be caller.
 */
char* SOPC_Log_GetCurrentFilename(const SOPC_Log_Instance* pLogInst);

/**
 * \brief Logs a trace with the given level
 *
 * \param pLogInst  An existing log instance already started
 * \param level     The log level corresponding to the given trace
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Log_Trace(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, const char* format, ...);

/**
 * \brief Logs a trace with the given level
 *
 * \param pLogInst  An existing log instance already started
 * \param level     The log level corresponding to the given trace
 * \param format    String specifying how subsequent arguments are converted for output
 * \param args      Arguments used by the string specifying the output
 */
void SOPC_Log_VTrace(SOPC_Log_Instance* pLogInst, SOPC_Log_Level level, const char* format, va_list args);

/**
 * \brief Stops allowing to log traces in the given log instance. Log file is closed when last log instance is stopped.
 *
 * \param ppLogInst  An existing log instance already started. Pointer set to NULL after call.
 */
void SOPC_Log_ClearInstance(SOPC_Log_Instance** ppLogInst);

#endif /* SOPC_LOG_MANAGER_H_ */
