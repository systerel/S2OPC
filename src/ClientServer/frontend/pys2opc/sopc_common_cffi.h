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
 * This file is an excerpt from sopc_common.h.
 * It should not be included in a generic project.
 * See s2opc_headers.h
 */

typedef struct SOPC_LogSystem_File_Configuration
{
    const char* logDirPath; /**< path of the log directory */
    uint32_t logMaxBytes;   /**< max bytes per log file */
    uint16_t logMaxFiles;   /**< max number of log files */
} SOPC_LogSystem_File_Configuration;

typedef enum SOPC_Log_System
{
    SOPC_LOG_SYSTEM_FILE /**< file system logger */
} SOPC_Log_System;

typedef union SOPC_Log_SystemConfiguration {
    SOPC_LogSystem_File_Configuration fileSystemLogConfig; /**< log file system configuration */
} SOPC_Log_SystemConfiguration;

typedef struct SOPC_Log_Configuration
{
    SOPC_Log_Level logLevel;                   /**< default log level */
    SOPC_Log_System logSystem;                 /**< discriminant for the log system configuration */
    SOPC_Log_SystemConfiguration logSysConfig; /**< log system configuration */
} SOPC_Log_Configuration;

SOPC_ReturnStatus SOPC_Common_Initialize(SOPC_Log_Configuration logConfiguration);
bool SOPC_Common_IsInitialized(void);
SOPC_Log_Configuration SOPC_Common_GetDefaultLogConfiguration(void);
SOPC_ReturnStatus SOPC_Common_SetLogLevel(SOPC_Log_Level level);
void SOPC_Common_Clear(void);
