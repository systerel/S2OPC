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

#define LOGGER_FUNC_FORMAT_ENUM ATTR_FORMAT(printf, 2, 3)
#define LOGGER_FUNC_FORMAT ATTR_FORMAT(printf, 1, 2)

/**
 * \brief enumerate to define log modules
 */
typedef enum SOPC_Log_Module
{
    SOPC_LOG_MODULE_COMMON,       /**< Common log module */
    SOPC_LOG_MODULE_CLIENTSERVER, /**< ClientServer log module */
    SOPC_LOG_MODULE_PUBSUB        /**< PubSub log module */
} SOPC_Log_Module;

/*
 * \brief Initializes the logger system
 *
 * \param logConfiguration   Global log configuration
 *
 * */
bool SOPC_Logger_Initialize(const SOPC_Log_Configuration* const logConfiguration);

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
 * \brief getter for the log level
 *
 * \return the trace log level
 */
SOPC_Log_Level SOPC_Logger_GetTraceLogLevel(void);

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
void SOPC_Logger_TraceError(SOPC_Log_Module logModule, const char* format, ...) LOGGER_FUNC_FORMAT_ENUM;

/*
 * \brief Log a trace with the warning level
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceWarning(SOPC_Log_Module logModule, const char* format, ...) LOGGER_FUNC_FORMAT_ENUM;

/*
 * \brief Log a trace with the info level
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceInfo(SOPC_Log_Module logModule, const char* format, ...) LOGGER_FUNC_FORMAT_ENUM;

/*
 * \brief Log a trace with the debug level
 *
 * \param format    String specifying how subsequent arguments are converted for output
 */
void SOPC_Logger_TraceDebug(SOPC_Log_Module logModule, const char* format, ...) LOGGER_FUNC_FORMAT_ENUM;

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
