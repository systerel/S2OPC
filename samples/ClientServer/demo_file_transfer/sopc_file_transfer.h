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

/** \file
 *
 * \brief API to manage the OPC UA FileTransfer.
 *
 * The API only handles temporary files. A temporary file is created with each call to the Open method.
 * If several Openings are followed without a call to the close method between them,
 * then the temporary file of the first Opening is deleted.
 * The path to store the temporary file and its prefix name must be defined by the user. The
 * user can access the name of the temporary file (prefix and random sufix) using a notification when
 * the close method is called.
 * Typical use in main (see toolkit_demo_file_transfer.c):
 *      -1 Initialise the API with <SOPC_FileTransfer_Initialize> function.
 *      -2 Configure FileType data of the address space (nodeId, methodId, variableId, close callback,
 *         tmp file path and tmp file name) with <SOPC_FileType_Config> structure.
 *      -3 Add file(s) to the API through the configuration(s) data of the address space
 *         with <SOPC_FileTransfer_Add_File> function (one call for each file).
 *      -4 Add optional user method(s) implementation with <SOPC_FileTransfer_Add_MethodItems> function.
 *      -5 Start the server asynchronously with <SOPC_FileTransfer_StartServer> function.
 *      -6 Get the name of temporary file in the closing processing
 *         (user close callback implementation: SOPC_FileTransfer_UserClose_Callback)
 *      - The user can read the values of the variables in the address space with
 *        <SOPC_FileTransfer_ReadVariable> function.
 *      - The user can write the values of the variables in the address space with
 *        <SOPC_FileTransfer_WriteVariable> function.
 *      - The user can implement a server callback definition used for address space modification
 *        by client.
 */

#include "libs2opc_server.h"
#include "sopc_builtintypes.h"

/**
 * \brief Default value for UserWritable variable of FileType Object
 */
#define VAR_USER_WRITABLE_DEFAULT true
/**
 * \brief Default value for Writable variable of FileType Object
 */
#define VAR_WRITABLE_DEFAULT true

/**
 * \brief User callback for the closing processing
 * \param tmp_file_path A pointer to the temporary file path.
 * \note After this callback, the path of the tmp file will be deallocated and the user should copy it.
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call.
 */
typedef void (*SOPC_FileTransfer_UserClose_Callback)(const char* tmp_file_path);

/**
 * \brief Structure to gather FilType configuration data
 */
typedef struct SOPC_FileType_Config
{
    char* file_path;          /*!< The path to store the temporary file. This one has to include the prefix name of the
                                 temporary file. */
    char* fileType_nodeId;    /*!< The nodeId of the FileType object. */
    char* met_openId;         /*!< The nodeId of the Open method. */
    char* met_closeId;        /*!< The nodeId of the Close method. */
    char* met_readId;         /*!< The nodeId of the Read method. */
    char* met_writeId;        /*!< The nodeId of the Write method. */
    char* met_getposId;       /*!< The nodeId of the GetPosistion method. */
    char* met_setposId;       /*!< The nodeId of the SetPosition method. */
    char* var_sizeId;         /*!< The nodeId of the Size variable. */
    char* var_openCountId;    /*!< The nodeId of the OpenCount variable. */
    char* var_userWritableId; /*!< The nodeId of the UserWritable variable. */
    char* var_writableId;     /*!< The nodeId of the Writable variable. */
    SOPC_FileTransfer_UserClose_Callback pFunc_UserCloseCallback; /*!< The Method Close Callback */
} SOPC_FileType_Config;

/**
 * \brief Initialise the API.
 * \note Memory allocation, need to call SOPC_FileTransfer_Clear after use.
 * \warning The function shall be called after SOPC_HelperConfigServer_Initialize.
 * \return SOPC_STATUS_OK if no error
 */
SOPC_ReturnStatus SOPC_FileTransfer_Initialize(void);

/**
 * \brief Adding a FileType object to the API from the address space information.
 * \note This function shall be call after <SOPC_FileTransfer_Initialize> and before <SOPC_FileTransfer_StartServer>
 * \param config The structure which gather FileType configuration data
 * \warning In case of error, the API is uninitialized (except for SOPC_STATUS_INVALID_PARAMETERS and
 * SOPC_STATUS_INVALID_STATE errors). \return SOPC_STATUS_OK if no error.
 */
SOPC_ReturnStatus SOPC_FileTransfer_Add_File(const SOPC_FileType_Config* config);

/**
 * \brief Adding a user method into the Server.
 * \note This function shall be call after <SOPC_FileTransfer_Initialize> and before <SOPC_FileTransfer_StartServer>.
 * \param methodFunc Pointer to the method function.
 * \param methodName A string to register the name of the method.
 * \param CnodeId The nodeId of the Method in the address space (C string).
 * \return SOPC_STATUS_OK if no error
 */
SOPC_ReturnStatus SOPC_FileTransfer_Add_MethodItems(SOPC_MethodCallFunc_Ptr methodFunc,
                                                    char* methodName,
                                                    const char* CnodeId);

/**
 * \brief Start the server asynchronously
 * \param ServerStoppedCallback  callback called when server will stop (on purpose or due to endpoint opening isssue)
 * \return SOPC_STATUS_OK in case of success. SOPC_STATUS_INVALID_STATE if the configuration is not possible (toolkit
 * not initialized, server already started). SOPC_STATUS_NOK if at least one local write of variable failed for one
 * fileType object on the address space.
 */
SOPC_ReturnStatus SOPC_FileTransfer_StartServer(SOPC_ServerStopped_Fct* ServerStoppedCallback);

/**
 * \brief Uninitialize the API (Free the memory)
 */
void SOPC_FileTransfer_Clear(void);
