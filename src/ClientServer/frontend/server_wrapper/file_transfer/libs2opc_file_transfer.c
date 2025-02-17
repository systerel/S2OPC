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
 * \brief API implementation to manage the OPC UA FileTransfer.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_file_transfer.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"
#include "sopc_assert.h"
#include "sopc_crypto_provider.h"
#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"

/**
 * \brief Number of variables for a FileType Object
 */
#define FT_NB_VARIABLE 4

/**
 * \brief Number of methods for a FileType Object
 */
#define FT_NB_METHOD 6

/**
 * \brief The invalid internal value for the file handle
 */
#define FT_INVALID_HANDLE_VALUE 0

/**
 * \brief The invalid value for the file open mode
 */
#define FT_INVALID_OPEN_MODE 0u

/**
 * \brief Bit masks value for opening mode
 */
#define READ_MASK 0x01u
#define WRITE_MASK 0x02u
#define ERASE_EXISTING_MASK 0x04u
#define APPEND_MASK 0x08u

/**
 * \brief Indexs to store variable into a list
 */
#define OPEN_COUNT_VAR_IDX 0u
#define SIZE_VAR_IDX 1u
#define WRITABLE_VAR_IDX 2u
#define USER_WRITABLE_VAR_IDX 3u

/**
 * \brief Indexs to store method into a list
 */
#define OPEN_METHOD_IDX 0u
#define CLOSE_METHOD_IDX 1u
#define READ_METHOD_IDX 2u
#define WRITE_METHOD_IDX 3u
#define GETPOS_METHOD_IDX 4u
#define SETPOS_METHOD_IDX 5u

/**
 * \brief Max size for a file
 */
#define MAX_SIZE_FILE 1000000u // 1 Mo. Limit = UINT32_MAX

/**
 * \brief Tombstone value for the dictionary
 */
static const uintptr_t DICT_TOMBSTONE = UINTPTR_MAX;

/************************
---- STATIC VARIABLE ----
************************/
/**
 * \brief Dictionary for storing files with their nodeIds
 */
static SOPC_Dict* g_objectId_to_file = NULL;

/**
 * \brief Method call manager pointer for registering file methods
 */
static SOPC_MethodCallManager* g_methodCallManager = NULL;

/* FileType type definition methods nodeId */
const SOPC_NodeId fileType_metOpenId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_FileType_Open);
const SOPC_NodeId fileType_metCloseId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_FileType_Close);
const SOPC_NodeId fileType_metReadId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_FileType_Read);
const SOPC_NodeId fileType_metWriteId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_FileType_Write);
const SOPC_NodeId fileType_metGetPosId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_FileType_GetPosition);
const SOPC_NodeId fileType_metSetPosId = SOPC_NODEID_NS0_NUMERIC(OpcUaId_FileType_SetPosition);

/**
 * \brief structure to manage FileType object
 */
struct _SOPC_FileType
{
    SOPC_FileHandle handle;                    /* The handle of the file send to the client. */
    SOPC_NodeId* pNodeId;                      /* The nodeId of the FileType object. */
    SOPC_OpenMode mode;                        /* Open mode receives from the client (bit mask). */
    SOPC_Buffer* pTmpFileContent;              /* Temporary file content */
    SOPC_ByteString* pPermFileContent;         /* Permanent file content */
    SOPC_NodeId* pVariableIds[FT_NB_VARIABLE]; /* list of variable nodeId associated at the FileType object. */
    SOPC_NodeId* pMethodIds[FT_NB_METHOD];     /* list of method nodeId associated at the FileType object. */
    SOPC_Mutex mutCallback;                    /* Mutex to cover concurrent Callback access */
    SOPC_FileTransfer_UserClose_Callback* pFunc_UserCloseCallback; /* The Method Close Callback */
    SOPC_FileTransfer_UserOpen_Callback* pFunc_UserOpenCallback;   /* The Method Open Callback */
};

/**
 * \brief structure to manage FileType Configuration
 */
struct _SOPC_FileType_Config
{
    SOPC_NodeId* pFileNodeId;        /* The nodeId of the FileType object. */
    SOPC_NodeId* pMetOpenId;         /* The nodeId of the Open method. */
    SOPC_NodeId* pMetCloseId;        /* The nodeId of the Close method. */
    SOPC_NodeId* pMetReadId;         /* The nodeId of the Read method. */
    SOPC_NodeId* pMetWriteId;        /* The nodeId of the Write method. */
    SOPC_NodeId* pMetGetPosId;       /* The nodeId of the GetPosition method. */
    SOPC_NodeId* pMetSetPosId;       /* The nodeId of the SetPosition method. */
    SOPC_NodeId* pVarSizeId;         /* The nodeId of the Size variable. */
    SOPC_NodeId* pVarOpenCountId;    /* The nodeId of the OpenCount variable. */
    SOPC_NodeId* pVarUserWritableId; /* The nodeId of the UserWritable variable. */
    SOPC_NodeId* pVarWritableId;     /* The nodeId of the Writable variable. */
};

/*****************************
---- FileTransfer methods ----
*****************************/
static SOPC_StatusCode FileTransfer_Method_Open(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param);

static SOPC_StatusCode FileTransfer_Method_Close(const SOPC_CallContext* callContextPtr,
                                                 const SOPC_NodeId* objectId,
                                                 uint32_t nbInputArgs,
                                                 const SOPC_Variant* inputArgs,
                                                 uint32_t* nbOutputArgs,
                                                 SOPC_Variant** outputArgs,
                                                 void* param);
static SOPC_StatusCode FileTransfer_Method_Read(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param);
static SOPC_StatusCode FileTransfer_Method_Write(const SOPC_CallContext* callContextPtr,
                                                 const SOPC_NodeId* objectId,
                                                 uint32_t nbInputArgs,
                                                 const SOPC_Variant* inputArgs,
                                                 uint32_t* nbOutputArgs,
                                                 SOPC_Variant** outputArgs,
                                                 void* param);
static SOPC_StatusCode FileTransfer_Method_GetPos(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param);
static SOPC_StatusCode FileTransfer_Method_SetPos(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param);

static SOPC_FileType* FileType_Dict_Get(const SOPC_NodeId* objectId, bool* found);

static SOPC_ReturnStatus FileTransfer_DictInsertFile(SOPC_FileType* pFile);

/*****************************************************************************************
******************************************************************************************
*---------------------------- FILETYPE_CONFIG FUNCTIONS -------------------------------- *
******************************************************************************************
*****************************************************************************************/

/**
 * \brief Initializes the FileType_Config structure
 * \param fileConfig Pointer of FileType_Config structure to initialize
 */
static void FileType_Config_Initialize(SOPC_FileType_Config* fileConfig)
{
    if (fileConfig != NULL)
    {
        fileConfig->pFileNodeId = NULL;
        fileConfig->pMetOpenId = NULL;
        fileConfig->pMetCloseId = NULL;
        fileConfig->pMetReadId = NULL;
        fileConfig->pMetWriteId = NULL;
        fileConfig->pMetGetPosId = NULL;
        fileConfig->pMetSetPosId = NULL;
        fileConfig->pVarSizeId = NULL;
        fileConfig->pVarOpenCountId = NULL;
        fileConfig->pVarUserWritableId = NULL;
        fileConfig->pVarWritableId = NULL;
    }
}

/**
 * \brief Clears the FileType_Config structure.
 * \param fileConfig Pointer of FileType_Config structure to clear.
 */
static void FileType_Config_Clear(SOPC_FileType_Config* fileConfig)
{
    if (NULL != fileConfig)
    {
        SOPC_NodeId_Clear(fileConfig->pFileNodeId);
        SOPC_NodeId_Clear(fileConfig->pMetOpenId);
        SOPC_NodeId_Clear(fileConfig->pMetCloseId);
        SOPC_NodeId_Clear(fileConfig->pMetReadId);
        SOPC_NodeId_Clear(fileConfig->pMetWriteId);
        SOPC_NodeId_Clear(fileConfig->pMetGetPosId);
        SOPC_NodeId_Clear(fileConfig->pMetSetPosId);
        SOPC_NodeId_Clear(fileConfig->pVarSizeId);
        SOPC_NodeId_Clear(fileConfig->pVarOpenCountId);
        SOPC_NodeId_Clear(fileConfig->pVarUserWritableId);
        SOPC_NodeId_Clear(fileConfig->pVarWritableId);
        SOPC_Free(fileConfig->pFileNodeId);
        SOPC_Free(fileConfig->pMetOpenId);
        SOPC_Free(fileConfig->pMetCloseId);
        SOPC_Free(fileConfig->pMetReadId);
        SOPC_Free(fileConfig->pMetWriteId);
        SOPC_Free(fileConfig->pMetGetPosId);
        SOPC_Free(fileConfig->pMetSetPosId);
        SOPC_Free(fileConfig->pVarSizeId);
        SOPC_Free(fileConfig->pVarOpenCountId);
        SOPC_Free(fileConfig->pVarUserWritableId);
        SOPC_Free(fileConfig->pVarWritableId);
        FileType_Config_Initialize(fileConfig);
    }
}

/**
 * \brief Deletes a FileType_Config.
 * \param file  The pointer of SOPC_FileType_Config object to delete.
 */
static void FileType_Config_Delete(SOPC_FileType_Config** fileConfig)
{
    if (NULL != fileConfig && NULL != *fileConfig)
    {
        FileType_Config_Clear(*fileConfig);
        SOPC_Free(*fileConfig);
        *fileConfig = NULL;
    }
}

/**
 * \brief Creates the FileType_Config structure.
 * \return Pointer of FileType_Config structure created.
 */
static inline SOPC_FileType_Config* FileType_Config_Create(void)
{
    SOPC_FileType_Config* fileConfig = SOPC_Calloc(1, sizeof(SOPC_FileType_Config));
    if (fileConfig != NULL)
    {
        FileType_Config_Initialize(fileConfig);
        fileConfig->pFileNodeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pMetOpenId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pMetCloseId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pMetReadId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pMetWriteId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pMetGetPosId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pMetSetPosId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pVarSizeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pVarOpenCountId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pVarUserWritableId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        fileConfig->pVarWritableId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        if (fileConfig->pFileNodeId == NULL || fileConfig->pMetOpenId == NULL || fileConfig->pMetCloseId == NULL ||
            fileConfig->pMetReadId == NULL || fileConfig->pMetWriteId == NULL || fileConfig->pMetGetPosId == NULL ||
            fileConfig->pMetSetPosId == NULL || fileConfig->pVarSizeId == NULL || fileConfig->pVarOpenCountId == NULL ||
            fileConfig->pVarUserWritableId == NULL || fileConfig->pVarWritableId == NULL)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:FileConfig:Create: Failed to create a file config object.");
            FileType_Config_Delete(&fileConfig);
        }
    }
    return fileConfig;
}

SOPC_FileType_Config* SOPC_FileType_Config_CreateFromNodeId(const SOPC_NodeId* fileId,
                                                            const SOPC_NodeId* metOpenId,
                                                            const SOPC_NodeId* metCloseId,
                                                            const SOPC_NodeId* metReadId,
                                                            const SOPC_NodeId* metWriteId,
                                                            const SOPC_NodeId* metGetPosId,
                                                            const SOPC_NodeId* metSetPosId,
                                                            const SOPC_NodeId* varSizeId,
                                                            const SOPC_NodeId* varOpenCountId,
                                                            const SOPC_NodeId* varUserWritableId,
                                                            const SOPC_NodeId* varWritableId)
{
    SOPC_FileType_Config* fileConfig = FileType_Config_Create();
    if (fileConfig != NULL)
    {
        SOPC_ReturnStatus status = SOPC_NodeId_Copy(fileConfig->pFileNodeId, fileId);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pMetOpenId, metOpenId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pMetCloseId, metCloseId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pMetReadId, metReadId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pMetWriteId, metWriteId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pMetGetPosId, metGetPosId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pMetSetPosId, metSetPosId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pVarSizeId, varSizeId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pVarOpenCountId, varOpenCountId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pVarUserWritableId, varUserWritableId);
        }
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_NodeId_Copy(fileConfig->pVarWritableId, varWritableId);
        }
        if (status != SOPC_STATUS_OK)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:FileConfig:ConfigureFromNodeId: Failed to configure a FileType_Config "
                                   "object from nodeId.");
            FileType_Config_Delete(&fileConfig);
        }
    }
    return fileConfig;
}

SOPC_FileType_Config* SOPC_FileType_Config_CreateFromStringNodeId(const char* fileStrNodeId,
                                                                  const char* metOpenId,
                                                                  const char* metCloseId,
                                                                  const char* metReadId,
                                                                  const char* metWriteId,
                                                                  const char* metGetPosId,
                                                                  const char* metSetPosId,
                                                                  const char* varSizeId,
                                                                  const char* varOpenCountId,
                                                                  const char* varUserWritableId,
                                                                  const char* varWritableId)
{
    SOPC_FileType_Config* fileConfig = NULL;
    SOPC_NodeId* fileNodeId = SOPC_NodeId_FromCString(fileStrNodeId);
    SOPC_NodeId* metOpenNodeId = SOPC_NodeId_FromCString(metOpenId);
    SOPC_NodeId* metCloseNodeId = SOPC_NodeId_FromCString(metCloseId);
    SOPC_NodeId* metReadNodeId = SOPC_NodeId_FromCString(metReadId);
    SOPC_NodeId* metWriteNodeId = SOPC_NodeId_FromCString(metWriteId);
    SOPC_NodeId* metGetPosNodeId = SOPC_NodeId_FromCString(metGetPosId);
    SOPC_NodeId* metSetPosNodeId = SOPC_NodeId_FromCString(metSetPosId);
    SOPC_NodeId* varSizeNodeId = SOPC_NodeId_FromCString(varSizeId);
    SOPC_NodeId* varOpenCountNodeId = SOPC_NodeId_FromCString(varOpenCountId);
    SOPC_NodeId* varUserWritableNodeId = SOPC_NodeId_FromCString(varUserWritableId);
    SOPC_NodeId* varWritableNodeId = SOPC_NodeId_FromCString(varWritableId);

    if (fileNodeId != NULL && metOpenNodeId != NULL && metCloseNodeId != NULL && metReadNodeId != NULL &&
        metWriteNodeId != NULL && metGetPosNodeId != NULL && metSetPosNodeId != NULL && varSizeNodeId != NULL &&
        varOpenCountNodeId != NULL && varUserWritableNodeId != NULL && varWritableNodeId != NULL)
    {
        fileConfig = SOPC_FileType_Config_CreateFromNodeId(
            fileNodeId, metOpenNodeId, metCloseNodeId, metReadNodeId, metWriteNodeId, metGetPosNodeId, metSetPosNodeId,
            varSizeNodeId, varOpenCountNodeId, varUserWritableNodeId, varWritableNodeId);
    }
    SOPC_NodeId_Clear(fileNodeId);
    SOPC_Free(fileNodeId);
    SOPC_NodeId_Clear(metOpenNodeId);
    SOPC_Free(metOpenNodeId);
    SOPC_NodeId_Clear(metCloseNodeId);
    SOPC_Free(metCloseNodeId);
    SOPC_NodeId_Clear(metReadNodeId);
    SOPC_Free(metReadNodeId);
    SOPC_NodeId_Clear(metWriteNodeId);
    SOPC_Free(metWriteNodeId);
    SOPC_NodeId_Clear(metGetPosNodeId);
    SOPC_Free(metGetPosNodeId);
    SOPC_NodeId_Clear(metSetPosNodeId);
    SOPC_Free(metSetPosNodeId);
    SOPC_NodeId_Clear(varSizeNodeId);
    SOPC_Free(varSizeNodeId);
    SOPC_NodeId_Clear(varOpenCountNodeId);
    SOPC_Free(varOpenCountNodeId);
    SOPC_NodeId_Clear(varUserWritableNodeId);
    SOPC_Free(varUserWritableNodeId);
    SOPC_NodeId_Clear(varWritableNodeId);
    SOPC_Free(varWritableNodeId);

    return fileConfig;
}

/*****************************************************************************************
******************************************************************************************
*------------------------------- FILE TYPE FUNCTIONS ----------------------------------- *
******************************************************************************************
*****************************************************************************************/

/**
 * \brief Initializes the FileType structure
 * \param file Pointer of FileType structure to initialize
 */
static void FileType_Initialize(SOPC_FileType* file)
{
    if (file != NULL)
    {
        file->handle = FT_INVALID_HANDLE_VALUE;
        file->pNodeId = NULL;
        file->mode = 0;
        file->pTmpFileContent = NULL;
        file->pPermFileContent = NULL;
        for (int i = 0; i < FT_NB_VARIABLE; i++)
        {
            file->pVariableIds[i] = NULL;
        }
        for (int i = 0; i < FT_NB_METHOD; i++)
        {
            file->pMethodIds[i] = NULL;
        }
        file->pFunc_UserCloseCallback = NULL;
        file->pFunc_UserOpenCallback = NULL;
    }
}

/**
 * \brief Clears the FileType structure
 * \param file Pointer of FileType structure to clear
 */
static void FileType_Clear(SOPC_FileType* file)
{
    if (file != NULL)
    {
        SOPC_NodeId_Clear(file->pNodeId);
        SOPC_Free(file->pNodeId);
        SOPC_Buffer_Delete(file->pTmpFileContent);
        SOPC_ByteString_Delete(file->pPermFileContent);
        for (int i = 0; i < FT_NB_VARIABLE; i++)
        {
            SOPC_NodeId_Clear(file->pVariableIds[i]);
            SOPC_Free(file->pVariableIds[i]);
        }
        for (int i = 0; i < FT_NB_METHOD; i++)
        {
            SOPC_NodeId_Clear(file->pMethodIds[i]);
            SOPC_Free(file->pMethodIds[i]);
        }
        SOPC_Mutex_Clear(&file->mutCallback);
        FileType_Initialize(file);
    }
}

/**
 * \brief Deletes a FileType.
 * \param ppFile  The pointer of FileType object to delete.
 */
static void FileType_Delete(SOPC_FileType** ppFile)
{
    if (NULL != ppFile && NULL != *ppFile)
    {
        FileType_Clear(*ppFile);
        SOPC_Free((*ppFile)->pNodeId);
        SOPC_Free((*ppFile)->pPermFileContent);
        SOPC_Free(*ppFile);
        *ppFile = NULL;
    }
}

/**
 * \brief Adds the FileType methods of the \p file object to the g_methodCallManager.
 * \return SOPC_STATUS_OK on success, error code otherwise
 */
static SOPC_ReturnStatus FileType_AddMethod(const SOPC_FileType* file)
{
    char* strFailedMethod = "";
    SOPC_ReturnStatus status = (NULL == file) ? SOPC_STATUS_INVALID_PARAMETERS : SOPC_STATUS_OK;
    if (g_methodCallManager == NULL)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(g_methodCallManager, file->pMethodIds[OPEN_METHOD_IDX],
                                                  &FileTransfer_Method_Open, "Open", NULL);
        strFailedMethod = status ? NULL : "Open";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(g_methodCallManager, file->pMethodIds[CLOSE_METHOD_IDX],
                                                  &FileTransfer_Method_Close, "Close", NULL);
        strFailedMethod = status ? NULL : "Close";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(g_methodCallManager, file->pMethodIds[READ_METHOD_IDX],
                                                  &FileTransfer_Method_Read, "Read", NULL);
        strFailedMethod = status ? NULL : "Read";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(g_methodCallManager, file->pMethodIds[WRITE_METHOD_IDX],
                                                  &FileTransfer_Method_Write, "Write", NULL);
        strFailedMethod = status ? NULL : "Write";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(g_methodCallManager, file->pMethodIds[GETPOS_METHOD_IDX],
                                                  &FileTransfer_Method_GetPos, "GetPosition", NULL);
        strFailedMethod = status ? NULL : "GetPosition";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(g_methodCallManager, file->pMethodIds[SETPOS_METHOD_IDX],
                                                  &FileTransfer_Method_SetPos, "SetPosition", NULL);
        strFailedMethod = status ? NULL : "SetPosition";
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:FileConfig:AddMethod: Unable to add %s method", strFailedMethod);
    }
    return status;
}

SOPC_FileType* SOPC_FileType_Create(SOPC_FileType_Config* fileConfig)
{
    // Check parameter
    if (NULL == fileConfig)
    {
        return NULL;
    }
    // Build FileType structure
    SOPC_FileType* file = SOPC_Calloc(1, sizeof(SOPC_FileType));
    SOPC_ReturnStatus status = (NULL == file) ? SOPC_STATUS_OUT_OF_MEMORY : SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        FileType_Initialize(file);
        // Forward FileType_Config nodeIds into FileType
        file->pNodeId = fileConfig->pFileNodeId;
        file->pVariableIds[SIZE_VAR_IDX] = fileConfig->pVarSizeId;
        file->pVariableIds[OPEN_COUNT_VAR_IDX] = fileConfig->pVarOpenCountId;
        file->pVariableIds[WRITABLE_VAR_IDX] = fileConfig->pVarWritableId;
        file->pVariableIds[USER_WRITABLE_VAR_IDX] = fileConfig->pVarUserWritableId;
        file->pMethodIds[OPEN_METHOD_IDX] = fileConfig->pMetOpenId;
        file->pMethodIds[CLOSE_METHOD_IDX] = fileConfig->pMetCloseId;
        file->pMethodIds[READ_METHOD_IDX] = fileConfig->pMetReadId;
        file->pMethodIds[WRITE_METHOD_IDX] = fileConfig->pMetWriteId;
        file->pMethodIds[GETPOS_METHOD_IDX] = fileConfig->pMetGetPosId;
        file->pMethodIds[SETPOS_METHOD_IDX] = fileConfig->pMetSetPosId;
        FileType_Config_Initialize(fileConfig);
        file->pTmpFileContent = SOPC_Buffer_CreateResizable(1, MAX_SIZE_FILE);
        if (NULL == file->pTmpFileContent)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->pPermFileContent = SOPC_ByteString_Create();
        if (NULL == file->pPermFileContent)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Mutex_Initialization(&file->mutCallback);
    }

    // Add to Dict
    if (SOPC_STATUS_OK == status)
    {
        status = FileTransfer_DictInsertFile(file);
    }

    // Add method
    if (SOPC_STATUS_OK == status)
    {
        status = FileType_AddMethod(file);
    }

    // Delete FileConfig
    if (SOPC_STATUS_OK == status)
    {
        FileType_Config_Delete(&fileConfig);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:FileType:Create: Failed to create a file object.");
        FileType_Delete(&file);
    }
    return file;
}

/**********************
---- Getter/Setter ----
**********************/

SOPC_NodeId* SOPC_FileType_GetFileNodeId(SOPC_FileType* file)
{
    return NULL == file ? NULL : file->pNodeId;
}

SOPC_FileType* SOPC_FileType_GetFileFromNodeId(const SOPC_NodeId* fileNodeId)
{
    SOPC_FileType* fileType = NULL;
    bool found = false;
    if (NULL != fileNodeId)
    {
        fileType = FileType_Dict_Get(fileNodeId, &found);
    }
    return found ? fileType : NULL;
}

SOPC_OpenMode SOPC_FileType_GetMode(const SOPC_FileType* file)
{
    return NULL == file ? FT_INVALID_OPEN_MODE : file->mode;
}

SOPC_FileHandle SOPC_FileType_GetHandle(const SOPC_FileType* file)
{
    return NULL == file ? FT_INVALID_HANDLE_VALUE : file->handle;
}

SOPC_ByteString* SOPC_FileType_GetCopyPermFileContent(const SOPC_FileType* file)
{
    SOPC_ByteString* bs = SOPC_ByteString_Create();
    SOPC_ReturnStatus status = SOPC_ByteString_Copy(bs, file->pPermFileContent);
    if (status != SOPC_STATUS_OK)
    {
        SOPC_ByteString_Delete(bs);
        return NULL;
    }
    return bs;
}

SOPC_Buffer* SOPC_FileType_GetCopyTmpFileContent(SOPC_FileType* file)
{
    uint32_t length = file->pTmpFileContent->length;
    SOPC_Buffer* buf = SOPC_Buffer_Create(length);
    SOPC_ReturnStatus status = SOPC_Buffer_Copy(buf, file->pTmpFileContent);
    if (status != SOPC_STATUS_OK)
    {
        SOPC_Buffer_Delete(buf);
        return NULL;
    }
    return buf;
}

SOPC_ReturnStatus SOPC_FileType_SetUserOpenCb(SOPC_FileType* file, SOPC_FileTransfer_UserOpen_Callback* userOpenCb)
{
    if (NULL == file || NULL == userOpenCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        SOPC_Mutex_Lock(&file->mutCallback);
        file->pFunc_UserOpenCallback = userOpenCb;
        SOPC_Mutex_Unlock(&file->mutCallback);
    }
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_FileType_SetUserCloseCb(SOPC_FileType* file, SOPC_FileTransfer_UserClose_Callback* userCloseCb)
{
    if (NULL == file || NULL == userCloseCb)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    else
    {
        SOPC_Mutex_Lock(&file->mutCallback);
        file->pFunc_UserCloseCallback = userCloseCb;
        SOPC_Mutex_Unlock(&file->mutCallback);
    }
    return SOPC_STATUS_OK;
}

/*****************************************************************************************
******************************************************************************************
*------------------------------ FILE TRANSFER FUNCTIONS -------------------------------- *
******************************************************************************************
*****************************************************************************************/

// TODO : Switch to thread-safe dictionary, once available

/**
 * \brief Gets the FileType object in the dictionary from the file nodeId.
 */
static SOPC_FileType* FileType_Dict_Get(const SOPC_NodeId* objectId, bool* found)
{
    return (SOPC_FileType*) SOPC_Dict_Get(g_objectId_to_file, (const uintptr_t) objectId, found);
}

/**
 * \brief Frees the FileType object.
 */
static void FileType_Free(uintptr_t value)
{
    if (NULL != (void*) value)
    {
        FileType_Delete((SOPC_FileType**) &value);
    }
}

void SOPC_FileTransfer_Clear(void)
{
    // Delete dictionary => Delete all file inside
    SOPC_Dict_Delete(g_objectId_to_file);
    g_objectId_to_file = NULL;
    g_methodCallManager = NULL;
}

SOPC_ReturnStatus SOPC_FileTransfer_Initialize(SOPC_MethodCallManager* methodCallManager)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Check already initialized
    if (NULL != g_objectId_to_file && NULL != g_methodCallManager)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Init: The FileTransfer module is already initialized.");
        status = SOPC_STATUS_INVALID_STATE;
    }

    // Create Dictionary (NodeId <-> FileType)
    if (SOPC_STATUS_OK == status)
    {
        g_objectId_to_file = SOPC_NodeId_Dict_Create(true, FileType_Free);
        if (NULL == g_objectId_to_file)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Init: Unable to create dictionary <g_objectId_to_file>");
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        SOPC_Dict_SetTombstoneKey(g_objectId_to_file, DICT_TOMBSTONE);
    }

    // Save method call manager pointer
    if (SOPC_STATUS_OK == status)
    {
        g_methodCallManager = methodCallManager;
    }

    // Register methods of the type FileType.
    char* strFailedMethod = "";
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(methodCallManager, &fileType_metOpenId, &FileTransfer_Method_Open,
                                                  "Open", NULL);
        strFailedMethod = status ? NULL : "Open";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(methodCallManager, &fileType_metCloseId, &FileTransfer_Method_Close,
                                                  "Close", NULL);
        strFailedMethod = status ? NULL : "Close";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(methodCallManager, &fileType_metReadId, &FileTransfer_Method_Read,
                                                  "Read", NULL);
        strFailedMethod = status ? NULL : "Read";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(methodCallManager, &fileType_metWriteId, &FileTransfer_Method_Write,
                                                  "Write", NULL);
        strFailedMethod = status ? NULL : "Write";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(methodCallManager, &fileType_metGetPosId, &FileTransfer_Method_GetPos,
                                                  "GetPosition", NULL);
        strFailedMethod = status ? NULL : "GetPosition";
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_MethodCallManager_AddMethod(methodCallManager, &fileType_metSetPosId, &FileTransfer_Method_SetPos,
                                                  "SetPosition", NULL);
        strFailedMethod = status ? NULL : "SetPosition";
    }

    if (status != SOPC_STATUS_OK)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:AddTypeMethod: Unable to add %s method",
                               strFailedMethod);
        SOPC_FileTransfer_Clear();
    }

    return status;
}

/**
 * \brief Insert a FileType object into the FileTransfer Dictionary.
 * \param pFile The file object
 * \return SOPC_STATUS_OK on success, error code otherwise
 */
static SOPC_ReturnStatus FileTransfer_DictInsertFile(SOPC_FileType* pFile)
{
    SOPC_ReturnStatus status = pFile != NULL ? SOPC_STATUS_OK : SOPC_STATUS_INVALID_PARAMETERS;
    if (SOPC_STATUS_OK == status)
    {
        SOPC_NodeId* nodeId = SOPC_Calloc(1, sizeof(SOPC_NodeId));
        if (nodeId != NULL)
        {
            status = SOPC_NodeId_Copy(nodeId, pFile->pNodeId);
        }
        if (SOPC_STATUS_OK == status)
        {
            // Insert file into Dictionary
            bool res = SOPC_Dict_Insert(g_objectId_to_file, (uintptr_t) nodeId, (uintptr_t) pFile);
            if (false == res)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:AddFile: Unable to insert file into dictionary");
                status = SOPC_STATUS_NOK;
            }
        }
        else
        {
            SOPC_NodeId_Clear(nodeId);
            SOPC_Free(nodeId);
        }
    }
    return status;
}

void SOPC_FileType_Remove(SOPC_FileType* pFile)
{
    SOPC_Dict_Remove(g_objectId_to_file, (uintptr_t) pFile->pNodeId);
}

/*****************************************************************************************
******************************************************************************************
*--------------------------------- UTILITY FUNCTIONS ----------------------------------- *
******************************************************************************************
*****************************************************************************************/

/**
 * \brief Checks the validity of the OpenMode.
 * \param mode OpenMode to check.
 * \return true if the OpenMode is valid, false otherwise.
 */
static bool Check_OpenModeValidity(SOPC_OpenMode mode)
{
    uint8_t shall_be_reserved = 0u;
    if (shall_be_reserved != (mode >> 4u) || shall_be_reserved == mode)
    {
        return false;
    }
    // EraseExisting bit can only be set if the file is opened for writing
    if ((mode & ERASE_EXISTING_MASK) && !(mode & WRITE_MASK))
    {
        return false;
    }
    // Append bit cannot be set alone
    if ((mode & APPEND_MASK) && !(mode & ~APPEND_MASK))
    {
        return false;
    }
    return true;
}

static uint32_t BuffToInt(const SOPC_ExposedBuffer* pBuff)
{
    SOPC_ASSERT(NULL != pBuff);
    uint32_t out = FT_INVALID_HANDLE_VALUE;
    out = (uint32_t)(pBuff[0] + (pBuff[1] << 8) + (pBuff[2] << 16) + (pBuff[3] << 24));
    return out;
}

/**
 * \brief Function to generate a random SOPC_FileHandle.
 * \note The handle generated is different form 0 which is the invalid internal value.
 * \return the SOPC_FileHandle generate (FT_INVALID_HANDLE_VALUE in case of error)
 */
static SOPC_FileHandle GenerateRandomHandle(void)
{
    {
        SOPC_FileHandle gen = FT_INVALID_HANDLE_VALUE;
        SOPC_ExposedBuffer* pBuff = NULL;
        SOPC_CryptoProvider* pCrypto = SOPC_CryptoProvider_Create(SOPC_SecurityPolicy_None_URI);
        SOPC_ReturnStatus status = SOPC_CryptoProvider_GenerateRandomBytes(pCrypto, 4, &pBuff);
        if (SOPC_STATUS_OK == status)
        {
            gen = BuffToInt(pBuff);
            if (FT_INVALID_HANDLE_VALUE == gen)
            {
                gen = 1;
            }
        }
        SOPC_Free(pBuff);
        SOPC_CryptoProvider_Free(pCrypto);
        return gen;
    }
}

/**
 * \brief Function to increment or decrement the OpenCount variable of a FileType object.
 * \param callContextPtr  context provided by server on connection/session associated to method call.
 * \param file            file object.
 * \param command         true if increment, false if decrement.
 * \return SOPC_GoodGenericStatus in case of success, OpcUa_BadInvalidState otherwise.
 */
static inline SOPC_StatusCode FileType_OpenCount_Inc_Dec(const SOPC_CallContext* callContextPtr,
                                                         const SOPC_FileType* file,
                                                         bool command)
{
    SOPC_StatusCode result_code = SOPC_GoodGenericStatus;
    SOPC_DataValue* dv = NULL;
    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    // SOPC_AddressSpaceAccess_ReadValue makes a copy of the dataValue into dv (allocation).
    result_code = SOPC_AddressSpaceAccess_ReadValue(addSpAccess, file->pVariableIds[OPEN_COUNT_VAR_IDX], NULL, &dv);
    if (!SOPC_IsGoodStatus(result_code) || SOPC_UInt16_Id != dv->Value.BuiltInTypeId ||
        SOPC_VariantArrayType_SingleValue != dv->Value.ArrayType)
    {
        result_code = OpcUa_BadInvalidState;
    }
    if (SOPC_IsGoodStatus(result_code))
    {
        if (command)
        {
            dv->Value.Value.Uint16++;
        }
        else
        {
            dv->Value.Value.Uint16--;
        }
        SOPC_DateTime ts = 0;
        result_code = SOPC_AddressSpaceAccess_WriteValue(addSpAccess, file->pVariableIds[OPEN_COUNT_VAR_IDX], NULL,
                                                         &dv->Value, NULL, &ts, NULL);
        if (!SOPC_IsGoodStatus(result_code))
        {
#ifdef FILE_TRANSFER_LOG
            char* errorNodeId = SOPC_NodeId_ToCString(file->pNodeId);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:WriteOpenCount: Write Open Count variable on file object '%s' failed "
                                   "with status code : %d",
                                   errorNodeId, result_code);
            SOPC_Free(errorNodeId);
#endif
            result_code = OpcUa_BadInvalidState;
        }
    }
    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);
    return result_code;
}

/**
 * \brief Writes the Size variable of a FileType object.
 *
 * \param callContextPtr  context provided by server on connection/session associated to method call.
 * \param file            file object.
 * \param value_size      size to write in the Size variable of FileType object.
 * \param optStatus       (Optional) The status code to associate with the value written,
 *                        NULL if previous value shall be kept.
 *
 * \return SOPC_GoodGenericStatus in case of success, OpcUa_BadInvalidState otherwise.
 */

static inline SOPC_StatusCode FileType_WriteSizeWithStatus(const SOPC_CallContext* callContextPtr,
                                                           const SOPC_FileType* file,
                                                           uint64_t value_size,
                                                           SOPC_StatusCode optStatus)
{
    SOPC_StatusCode result_code = SOPC_GoodGenericStatus;
    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    SOPC_DateTime ts = 0;
    SOPC_Variant var = {.BuiltInTypeId = SOPC_UInt64_Id,
                        .ArrayType = SOPC_VariantArrayType_SingleValue,
                        .Value = {.Uint64 = value_size}};
    result_code = SOPC_AddressSpaceAccess_WriteValue(addSpAccess, file->pVariableIds[SIZE_VAR_IDX], NULL, &var,
                                                     &optStatus, &ts, NULL);
    if (!SOPC_IsGoodStatus(result_code))
    {
#ifdef FILE_TRANSFER_LOG
        char* errorNodeId = SOPC_NodeId_ToCString(file->pNodeId);
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "FileTransfer:WriteOpenCount: Write Size variable on file object '%s' failed with status code : %d",
            errorNodeId, result_code);
        SOPC_Free(errorNodeId);
#endif
        result_code = OpcUa_BadInvalidState;
    }
    return result_code;
}

/**
 * \brief Reads/Writes the Writable/UserWritable variables of a FileType object.
 *
 * \param callContextPtr  context provided by server on connection/session associated to method call.
 * \param file            file object.
 * \param[out] writable   the result of Writable reading.
 *
 * \return SOPC_GoodGenericStatus in case of success, error code otherwise.
 *
 * \note  if Read: Writable = NULL -> Write: Writable/UserWritable = True
 *        else Read: Writable -> put in \p writable
 */
static inline SOPC_StatusCode FileType_ReadWrite_Writable(const SOPC_CallContext* callContextPtr,
                                                          const SOPC_FileType* file,
                                                          bool* writable)
{
    SOPC_DataValue* dv = NULL;
    SOPC_AddressSpaceAccess* addSpAccess = SOPC_CallContext_GetAddressSpaceAccess(callContextPtr);
    // SOPC_AddressSpaceAccess_ReadValue makes a copy of the dataValue into dv (allocation).
    SOPC_StatusCode result_code =
        SOPC_AddressSpaceAccess_ReadValue(addSpAccess, file->pVariableIds[WRITABLE_VAR_IDX], NULL, &dv);
    if (SOPC_IsGoodStatus(result_code) || SOPC_VariantArrayType_SingleValue == dv->Value.ArrayType)
    {
        if (SOPC_Boolean_Id == dv->Value.BuiltInTypeId) // Writable defined -> write the result to the output value
        {
            *writable = (bool) dv->Value.Value.Boolean;
        }
        else if (SOPC_Null_Id == dv->Value.BuiltInTypeId) // Writable variable undefined -> Set to true
        {
            *writable = true; // First, set output value to true
            // Write true in Writable and UserWritable values in addressSpace
            SOPC_DateTime ts = 0;
            SOPC_Variant var = {.BuiltInTypeId = SOPC_Boolean_Id,
                                .ArrayType = SOPC_VariantArrayType_SingleValue,
                                .Value = {.Boolean = true}};
            result_code = SOPC_AddressSpaceAccess_WriteValue(addSpAccess, file->pVariableIds[WRITABLE_VAR_IDX], NULL,
                                                             &var, SOPC_GoodGenericStatus, &ts, NULL);
            if (SOPC_IsGoodStatus(result_code))
            {
                result_code = SOPC_AddressSpaceAccess_WriteValue(addSpAccess, file->pVariableIds[USER_WRITABLE_VAR_IDX],
                                                                 NULL, &var, SOPC_GoodGenericStatus, &ts, NULL);
            }
            if (!SOPC_IsGoodStatus(result_code))
            {
#ifdef FILE_TRANSFER_LOG
                char* errorNodeId = SOPC_NodeId_ToCString(file->pNodeId);
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:ReadWriteWritable: Write Writable variable on file object '%s' failed "
                    "with status code : %d",
                    errorNodeId, result_code);
                SOPC_Free(errorNodeId);
#endif
            }
        }
    }
    else
    {
#ifdef FILE_TRANSFER_LOG
        char* errorNodeId = SOPC_NodeId_ToCString(file->pNodeId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:ReadWriteWritable: Read Writable variable on file object '%s' failed "
                               "with status code : %d",
                               errorNodeId, result_code);
        SOPC_Free(errorNodeId);
#endif
    }
    SOPC_DataValue_Clear(dv);
    SOPC_Free(dv);
    return result_code;
}
/*****************************************************************************************
******************************************************************************************
*------------------------------ METHODS IMPLEMENTATION --------------------------------- *
******************************************************************************************
*****************************************************************************************/

static SOPC_StatusCode FileTransfer_Method_Open(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param)
{
    SOPC_UNUSED_ARG(param);
    SOPC_StatusCode result_code = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Check input arguments
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: Bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    // Get the file object
    bool found = false;
    SOPC_FileType* file = FileType_Dict_Get(objectId, &found);
    if (true != found)
    {
#ifdef FILE_TRANSFER_LOG
        char* errorNodeId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Open: Unable to retrieve the file object from nodeId '%s'",
                               errorNodeId);
        SOPC_Free(errorNodeId);
#endif
        return OpcUa_BadNotFound;
    }
    // Decode argument
    SOPC_Byte mode = inputArgs->Value.Byte;

    // Check open mode validity
    bool mode_ok = Check_OpenModeValidity(mode);
    if (!mode_ok)
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: OpenMode %" PRIu8 " is invalid",
                               mode);
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Check Writable
    bool writable = true;
    result_code = FileType_ReadWrite_Writable(callContextPtr, file, &writable);
    if (SOPC_IsGoodStatus(result_code))
    {
        if (!writable && (mode & WRITE_MASK)) // File not writable, Read only.
        {
            return OpcUa_BadNotWritable;
        }
    }
    else // Failure to Read/Write Writable variable
    {
        return OpcUa_BadInvalidState;
    }

    // Check file already open
    if (file->handle != FT_INVALID_HANDLE_VALUE)
    {
        if (mode & WRITE_MASK)
        {
            return OpcUa_BadNotWritable; // File already open in any mode => Not Writable
        }
        else if ((mode & READ_MASK) && (file->mode & WRITE_MASK))
        {
            return OpcUa_BadNotReadable; // File already open in a write mode => Not Readable
        }
        else
        {
            return OpcUa_BadNotSupported; // File already open with in any mode
        }
    }

    // Check open mode user authorization
    if (file->pFunc_UserOpenCallback != NULL)
    {
        bool mode_userOk = true;
        SOPC_ByteString* bs = SOPC_ByteString_Create();
        SOPC_Mutex_Lock(&file->mutCallback);
        file->pFunc_UserOpenCallback(mode, &mode_userOk, bs);
        SOPC_Mutex_Unlock(&file->mutCallback);
        // User refuses opening mode
        if (mode_userOk == false)
        {
            SOPC_ByteString_Delete(bs);
            return OpcUa_BadInvalidArgument;
        }
        // User authorizes opening mode and gives an input ByteString.
        if (bs->Length > 0 && bs->Length < (int32_t) MAX_SIZE_FILE) // Protect buffer (file->pTmpFileContent) overload
        {
            // Move user ByteString (bs) in internal file structure
            SOPC_ByteString_Delete(file->pPermFileContent);
            file->pPermFileContent = bs;
        }
        else
        {
            if (bs->Length > (int32_t) MAX_SIZE_FILE)
            {
#ifdef FILE_TRANSFER_LOG
                SOPC_Logger_TraceWarning(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:Method_Open: The size of the ByteString transmitted in UserOpen_Callback is "
                    "too large (must be less than MAX_SIZE_FILE = %d). Internal ByteString saved is used",
                    MAX_SIZE_FILE);
#endif
            }
            SOPC_ByteString_Delete(bs);
        }
    }

    // EraseExisting case
    if (mode & ERASE_EXISTING_MASK)
    {
        SOPC_ByteString_Clear(file->pPermFileContent);
        SOPC_Buffer_Reset(file->pTmpFileContent);
    }
    // Load file content (pPermFileContent) in temporary buffer (pTmpFileContent)
    else if (file->pTmpFileContent != NULL && file->pTmpFileContent->data != NULL)
    {
        SOPC_Buffer_Reset(file->pTmpFileContent);
        if (file->pPermFileContent != NULL && file->pPermFileContent->Data != NULL &&
            file->pPermFileContent->Length > 0)
        {
            status = SOPC_Buffer_Write(file->pTmpFileContent, file->pPermFileContent->Data,
                                       (uint32_t) file->pPermFileContent->Length);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_Buffer_SetPosition(file->pTmpFileContent, 0);
            }
            if (status != SOPC_STATUS_OK)
            {
#ifdef FILE_TRANSFER_LOG
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:Method_Open: Internal file failed to load");
#endif
                return OpcUa_BadUnexpectedError;
            }
        }
    }

    // Append case
    if (mode & APPEND_MASK)
    {
        status = SOPC_STATUS_NOK;
        if (file->pTmpFileContent != NULL)
        {
            status = SOPC_Buffer_SetPosition(file->pTmpFileContent, file->pTmpFileContent->length);
        }
        if (status != SOPC_STATUS_OK)
        {
#ifdef FILE_TRANSFER_LOG
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Open: Unable to set buffer position");
#endif
            return OpcUa_BadUnexpectedError;
        }
    }

    // Increment OpenCount
    result_code = FileType_OpenCount_Inc_Dec(callContextPtr, file, true);
    // Write Size
    if (SOPC_IsGoodStatus(result_code))
    {
        result_code = OpcUa_BadUnexpectedError;
        if (file->pTmpFileContent != NULL)
        {
            result_code = FileType_WriteSizeWithStatus(callContextPtr, file, (uint64_t) file->pTmpFileContent->length,
                                                       SOPC_GoodGenericStatus);
        }
    }

    // Output
    if (SOPC_IsGoodStatus(result_code))
    {
        // Generate a file handle
        SOPC_FileHandle handle = GenerateRandomHandle();
        // Set output arguments
        SOPC_Variant* pVariant = SOPC_Variant_Create();
        if (NULL == pVariant)
        {
#ifdef FILE_TRANSFER_LOG
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Open: Unable to create output variant status = %d", status);
#endif
            return OpcUa_BadUnexpectedError;
        }
        pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
        pVariant->BuiltInTypeId = SOPC_UInt32_Id;
        pVariant->Value.Uint32 = handle;
        *nbOutputArgs = 1;
        *outputArgs = pVariant;

        // Internal save of file handle and mode
        file->handle = handle;
        file->mode = mode;
    }
    return result_code;
}

static SOPC_StatusCode FileTransfer_Method_Close(const SOPC_CallContext* callContextPtr,
                                                 const SOPC_NodeId* objectId,
                                                 uint32_t nbInputArgs,
                                                 const SOPC_Variant* inputArgs,
                                                 uint32_t* nbOutputArgs,
                                                 SOPC_Variant** outputArgs,
                                                 void* param)
{
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_UNUSED_ARG(param);

    SOPC_StatusCode result_code = SOPC_GoodGenericStatus;
    // Check input arguments
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Close: Bad inputs arguments");
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Get the file object
    bool found = false;
    SOPC_FileType* file = FileType_Dict_Get(objectId, &found);
    if (true != found)
    {
#ifdef FILE_TRANSFER_LOG
        char* errorNodeId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Close: Unable to retrieve the file object from nodeId '%s'",
                               errorNodeId);
        SOPC_Free(errorNodeId);
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Decode arguments
    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;

    // Check handle
    if (handle != file->handle)
    {
        return OpcUa_BadInvalidArgument;
    }

    // Save File
    if (file->pTmpFileContent->length > INT32_MAX)
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Close: impossible to save internally, the file is larger than "
                               "an INT32_MAX. file size = %d",
                               file->pTmpFileContent->length);
#endif
        return OpcUa_BadUnexpectedError;
    }
    bool localSave = true;
    SOPC_ByteString* bs = SOPC_ByteString_Create();
    if (bs != NULL)
    {
        // Put temporary buffer into a ByteString (bs)
        SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(file->pTmpFileContent, 0);
        if (SOPC_STATUS_OK == status && file->pTmpFileContent->length > 0)
        {
            bs->Data = (SOPC_Byte*) SOPC_Calloc(file->pTmpFileContent->length, sizeof(SOPC_Byte));
            status = SOPC_Buffer_Read(bs->Data, file->pTmpFileContent, file->pTmpFileContent->length);
            bs->Length = (int32_t) file->pTmpFileContent->length;
        }
        if (SOPC_STATUS_OK == status)
        {
            // Give ByteString (bs) to User
            if (file->pFunc_UserCloseCallback != NULL)
            {
                SOPC_Mutex_Lock(&file->mutCallback);
                file->pFunc_UserCloseCallback(bs, &localSave);
                SOPC_Mutex_Unlock(&file->mutCallback);
            }
            // User choice : save ByteString (bs) locally
            if (localSave)
            {
                SOPC_ByteString_Delete(file->pPermFileContent);
                file->pPermFileContent = bs;
            }
            else
            {
                SOPC_ByteString_Delete(bs);
            }
        }
        else
        {
            SOPC_ByteString_Delete(bs);
#ifdef FILE_TRANSFER_LOG
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Close: Unable to save file content. status = %d", status);
#endif
            return OpcUa_BadUnexpectedError;
        }
    }

    // Close file handle
    file->handle = 0;

    // Decrement OpenCount
    result_code = FileType_OpenCount_Inc_Dec(callContextPtr, file, false);

    // Write Size
    if (SOPC_IsGoodStatus(result_code))
    {
        // If the file is not backed up locally, the length of the temporary file is retained (in size variable) but the
        // OpcUa_BadNotSupported status is indicated.
        result_code = FileType_WriteSizeWithStatus(callContextPtr, file, (uint64_t) file->pTmpFileContent->length,
                                                   (localSave ? SOPC_GoodGenericStatus : OpcUa_BadNotSupported));
    }

    return result_code;
}

static SOPC_StatusCode FileTransfer_Method_Read(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);

    // Check input arguments
    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: Bad inputs arguments");
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Get the file object
    bool found = false;
    SOPC_FileType* file = FileType_Dict_Get(objectId, &found);
    if (true != found)
    {
#ifdef FILE_TRANSFER_LOG
        char* errorNodeId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Read: Unable to retrieve the file object from nodeId '%s'",
                               errorNodeId);
        SOPC_Free(errorNodeId);
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Decode arguments
    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    int32_t length = inputArgs[1].Value.Int32;
    // Negative length are not allowed
    if (length < 0)
    {
        return OpcUa_BadInvalidArgument;
    }

    // Check handle/mode
    if (handle != file->handle)
    {
        return OpcUa_BadInvalidArgument;
    }
    if (!(file->mode & READ_MASK))
    {
        return OpcUa_BadInvalidState;
    }

    // Read Buffer
    uint32_t lengthToRead = length > (int32_t)(file->pTmpFileContent->length - file->pTmpFileContent->position)
                                ? (file->pTmpFileContent->length - file->pTmpFileContent->position)
                                : (uint32_t) length;
    SOPC_Byte* data = SOPC_Calloc(lengthToRead, sizeof(SOPC_Byte));
    SOPC_ReturnStatus status = SOPC_Buffer_Read(data, file->pTmpFileContent, lengthToRead);
    if (status != SOPC_STATUS_OK)
    {
        SOPC_Free(data);
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Read: Unable to read buffer. status = %d", status);
#endif
        return OpcUa_BadUnexpectedError;
    }

    // Set output arguments
    SOPC_Variant* pVariant = SOPC_Variant_Create();
    if (NULL == pVariant)
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Read: Unable to create output variant. status = %d", status);
#endif
        return OpcUa_BadUnexpectedError;
    }
    pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
    pVariant->BuiltInTypeId = SOPC_ByteString_Id;
    pVariant->Value.Bstring.Data = data;
    pVariant->Value.Bstring.Length = (int32_t) lengthToRead;
    *nbOutputArgs = 1;
    *outputArgs = pVariant;
    return SOPC_GoodGenericStatus;
}

static SOPC_StatusCode FileTransfer_Method_Write(const SOPC_CallContext* callContextPtr,
                                                 const SOPC_NodeId* objectId,
                                                 uint32_t nbInputArgs,
                                                 const SOPC_Variant* inputArgs,
                                                 uint32_t* nbOutputArgs,
                                                 SOPC_Variant** outputArgs,
                                                 void* param)
{
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_UNUSED_ARG(param);

    // Check input arguments
    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Write: Bad inputs arguments");
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Get the file object
    bool found = false;
    SOPC_FileType* file = FileType_Dict_Get(objectId, &found);
    if (true != found)
    {
#ifdef FILE_TRANSFER_LOG
        char* errorNodeId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Write: Unable to retrieve the file object from nodeId '%s'",
                               errorNodeId);
        SOPC_Free(errorNodeId);
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Decode arguments
    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    SOPC_ByteString bs_data = inputArgs[1].Value.Bstring;

    // Check handle/mode
    if (handle != file->handle)
    {
        return OpcUa_BadInvalidArgument;
    }
    if (!(file->mode & WRITE_MASK))
    {
        return OpcUa_BadInvalidState;
    }

    // Write Buffer
    if (bs_data.Data != NULL && bs_data.Length > 0)
    {
        // cast int32 -> uint32 OK, because int32 positivity is verified
        SOPC_ReturnStatus status = SOPC_Buffer_Write(file->pTmpFileContent, bs_data.Data, (uint32_t) bs_data.Length);
        if (status != SOPC_STATUS_OK)
        {
#ifdef FILE_TRANSFER_LOG
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Write: Unable to write buffer. status = %d", status);
#endif
            return OpcUa_BadUnexpectedError;
        }
    }

    // Write Size
    SOPC_StatusCode return_status = FileType_WriteSizeWithStatus(
        callContextPtr, file, (uint64_t) file->pTmpFileContent->length, SOPC_GoodGenericStatus);

    return return_status;
}

static SOPC_StatusCode FileTransfer_Method_GetPos(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(param);

    // Check input arguments
    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_GetPos: Bad inputs arguments");
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Get the file object
    bool found = false;
    SOPC_FileType* file = FileType_Dict_Get(objectId, &found);
    if (true != found)
    {
#ifdef FILE_TRANSFER_LOG
        char* errorNodeId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_GetPosition: Unable to retrieve the file object from nodeId '%s'",
                               errorNodeId);
        SOPC_Free(errorNodeId);
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Decode arguments
    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;

    // Check handle
    if (handle != file->handle)
    {
        return OpcUa_BadInvalidArgument;
    }

    // Get Buffer position
    uint32_t position = 0;
    SOPC_ReturnStatus status = SOPC_Buffer_GetPosition(file->pTmpFileContent, &position);
    if (status != SOPC_STATUS_OK)
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_GetPosition: Unable to get buffer position. status = %d", status);
#endif
        return OpcUa_BadUnexpectedError;
    }

    // Set output arguments
    SOPC_Variant* pVariant = SOPC_Variant_Create();
    if (NULL == pVariant)
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_GetPosition: Unable to create output variant. status = %d", status);
#endif
        return OpcUa_BadUnexpectedError;
    }
    pVariant->ArrayType = SOPC_VariantArrayType_SingleValue;
    pVariant->BuiltInTypeId = SOPC_UInt64_Id;
    pVariant->Value.Uint64 = (uint64_t) position;
    *nbOutputArgs = 1;
    *outputArgs = pVariant;
    return SOPC_GoodGenericStatus;
}

static SOPC_StatusCode FileTransfer_Method_SetPos(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param)
{
    SOPC_UNUSED_ARG(callContextPtr);
    SOPC_UNUSED_ARG(nbOutputArgs);
    SOPC_UNUSED_ARG(outputArgs);
    SOPC_UNUSED_ARG(param);

    // Check input arguments
    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_SetPos: Bad inputs arguments");
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Get the file object
    bool found = false;
    SOPC_FileType* file = FileType_Dict_Get(objectId, &found);
    if (true != found)
    {
#ifdef FILE_TRANSFER_LOG
        char* errorNodeId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_SetPosition: Unable to retrieve the file object from nodeId '%s'",
                               errorNodeId);
        SOPC_Free(errorNodeId);
#endif
        return OpcUa_BadInvalidArgument;
    }

    // Decode arguments
    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    uint64_t position_u64 = inputArgs[1].Value.Uint64;

    // Check handle
    if (handle != file->handle)
    {
        return OpcUa_BadInvalidArgument;
    }
    if (position_u64 > UINT32_MAX)
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_SetPosition: Unable to set position higher than UINT32_MAX");
#endif
        return OpcUa_BadNotSupported;
    }

    // Set Buffer position
    uint32_t postion_u32 = (uint32_t) position_u64;
    postion_u32 = postion_u32 > file->pTmpFileContent->length ? file->pTmpFileContent->length : postion_u32;
    SOPC_ReturnStatus status = SOPC_Buffer_SetPosition(file->pTmpFileContent, postion_u32);
    if (status != SOPC_STATUS_OK)
    {
#ifdef FILE_TRANSFER_LOG
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_SetPosition: Unable to set buffer position. status = %d", status);
#endif
        return OpcUa_BadUnexpectedError;
    }
    return SOPC_GoodGenericStatus;
}

/*****************************************************************************************
******************************************************************************************
*-------------------------------- API METHODS ACCESS ----------------------------------- *
******************************************************************************************
*****************************************************************************************/

SOPC_StatusCode SOPC_FileTransfer_CloseMethodAccess(const SOPC_CallContext* callContextPtr,
                                                    const SOPC_NodeId* objectId,
                                                    uint32_t nbInputArgs,
                                                    const SOPC_Variant* inputArgs,
                                                    uint32_t* nbOutputArgs,
                                                    SOPC_Variant** outputArgs,
                                                    void* param)
{
    return FileTransfer_Method_Close(callContextPtr, objectId, nbInputArgs, inputArgs, nbOutputArgs, outputArgs, param);
}
