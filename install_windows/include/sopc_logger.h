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
 * \file sopc_logger.h
 *
 *  \brief Specialized logger for the Toolkit
 */

#ifndef SOPC_LOGGER_H_
#define SOPC_LOGGER_H_

#include <stdbool.h>
#include <stdint.h>

#include "sopc_log_manager.h"

#ifdef __GNUC__
#define ATTR_FORMAT(archetype, string_index, first) __attribute__((format(archetype, string_index, first)))
#else
#define ATTR_FORMAT(archetype, string_index, first)
#endif

#define LOGGER_FUNC_FORMAT ATTR_FORMAT(printf, 1, 2)

/*
 * \brief Initializes the logger and create the necessary log file(s)
 *
 * \param logDirPath   Absolute or relative path of the directory to be used for logs (shall exist and terminate with
 * directory separator)
 * \param maxBytes     A maximum amount of bytes by log file before opening a new file incrementing the integer suffix.
 * It is a best effort value (amount verified after each print).
 * \param maxFiles     A maximum number of files to be used, when reached the older log file is overwritten
 * (starting with *_00001.log)
 *
 * */
bool SOPC_Logger_Initialize(const char* logDirPath, uint32_t maxBytes, uint16_t maxFiles);

/*
 * \brief Defines the active log level for the given log instance (default: ERROR):
 * - ERROR: display only ERROR level
 * - WARNING: display ERROR + WARNING levels
 * - INFO: display ERROR + WARNING + INFO levels
 * - DEBUG: display ERROR + WARNING + INFO + DEBUG levels
 *
 * \param level     The level to be activated for the log instance
 */
void SOPC_Logger_SetTraceLogLevel(SOPC_Log_Level level);

/*
 * \brief Activates the console output for logged traces (same active level as log file)
 *
 * \param activate  Flag to activate / deactivate the console output
 */
void SOPC_Logger_SetConsoleOutput(bool activate);

/*
 * \brief Log a trace with the error level
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceError(const char* format, ...) LOGGER_FUNC_FORMAT;

/*
 * \brief Log a trace with the warning level
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceWarning(const char* format, ...) LOGGER_FUNC_FORMAT;

/*
 * \brief Log a trace with the info level
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceInfo(const char* format, ...) LOGGER_FUNC_FORMAT;

/*
 * \brief Log a trace with the debug level
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceDebug(const char* format, ...) LOGGER_FUNC_FORMAT;

/*
 * \brief Log a trace for the security audit log
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceSecurityAudit(const char* format, ...) LOGGER_FUNC_FORMAT;

/*
 * \brief Log a warning trace for the security audit log
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceSecurityAuditWarning(const char* format, ...) LOGGER_FUNC_FORMAT;

/*
 * \brief Log a trace for the OPC UA audit log
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceOpcUaAudit(const char* format, ...) LOGGER_FUNC_FORMAT;

/*
 * \brief Log a warning trace for the OPC UA audit log
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceOpcUaAuditWarning(const char* format, ...) LOGGER_FUNC_FORMAT;

/*
 * \brief Clears the logger and close the current log files
 * */
void SOPC_Logger_Clear(void);

#endif /* SOPC_LOGGER_H_ */
