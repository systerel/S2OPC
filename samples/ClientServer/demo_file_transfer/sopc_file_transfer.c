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

#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "sopc_assert.h"
#include "sopc_file_transfer.h"
#include "sopc_logger.h"

#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "opcua_statuscodes.h"
#include "sopc_atomic.h"
#include "sopc_builtintypes.h"
#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_mem_alloc.h"
#include "sopc_platform_time.h"

/**
 * \brief Number of method per FileType Object
 */
#define NB_FILE_TYPE_METHOD 6

/**
 * \brief Number of variable to be update by method for a FileType Object
 */
#define NB_VARIABLE 4

/**
 * \brief The invalid internal value for the file handle
 */
#define INVALID_HANDLE_VALUE 0

/**
 * \brief File handle type (to send to client)
 */
typedef uint32_t SOPC_FileHandle;

/**
 * \brief Open mode type (receive from the client, bit mask)
 */
typedef SOPC_Byte SOPC_OpenMode;

/**
 * \brief A buffer size to manage C string
 */
#define STR_MARGIN_SIZE 50

/**
 * \brief Value to check if the opening mode is unknown
 */
#define FileTransfer_UnknownMode 0u

/**
 * \brief Bit masks value for opening mode
 */
#define READ_MASK 0x01u
#define WRITE_MASK 0x02u
#define ERASE_EXISTING_MASK 0x04u
#define APPEND_MASK 0x08u

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
 * \brief Indexs to store variable into a list
 */
#define OPEN_COUNT_VAR_IDX 0u
#define SIZE_VAR_IDX 1u
#define WRITABLE_VAR_IDX 2u
#define USER_WRITABLE_VAR_IDX 3u

/**
 * \brief structure to manage FileType object
 */
typedef struct SOPC_FileType
{
    SOPC_FileHandle handle; /*!< The handle of the file send to the client. */
    SOPC_String* path;      /*!< the file path where the tmp file will created (shall include the prefix name, example:
                               /tmp/my_file). */
    SOPC_NodeId* methodIds[NB_FILE_TYPE_METHOD]; /*!< list of method nodeId associated at the FileType object. */
    SOPC_OpenMode mode;                          /*!< Open mode receives from the client (bit mask). */
    bool is_open;                                /*!< boolean to known if the file is already open. */
    SOPC_String* tmp_path; /*!< The path with the ramdom tmp name created (path + random suffix, example:
                              /tmp/my_file-7moC1F). */
    FILE* fp;              /*!< File pointer. */
    SOPC_NodeId* variableIds[NB_VARIABLE]; /*!< list of variable nodeId associated at the FileType object. */
    uint16_t open_count;   /*!< The number of times the Open method has been called since the server started. */
    uint64_t size_in_byte; /*!< The size in byte of the file, updated after a read operation from the client. */
    SOPC_FileTransfer_UserClose_Callback pFunc_UserCloseCallback; /*!< The Method Close Callback */
} SOPC_FileType;

/**
 * \brief Create a FileType object.
 * \note Memory allocation, need to call FileTransfer_FileType_Delete after use.
 * \return The FileType object structure allocated
 */
static SOPC_FileType* FileTransfer_FileType_Create(void);

/**
 * \brief Initialize a FileType object with its default value.
 * \note Random value is use to generate the file handle.
 * \param filetype The pointer on the FileType object structure to initialize.
 */
static void FileTransfer_FileType_Initialize(SOPC_FileType* filetype);

/**
 * \brief Clear the FileType structure fields.
 * \param filetype The pointer on the FileType object structure to clear.
 */
static void FileTransfer_FileType_Clear(SOPC_FileType* filetype);

/**
 * \brief Delete the FileType structure (clear and free).
 * \param filetype Pointer on the FileType object structure to delete.
 */
static void FileTransfer_FileType_Delete(SOPC_FileType** filetype);

/**
 * \brief Create a temporary file at the path location (path is the field of the FileType structure)
 * \note This function set the field tmp_path with the field path + random suffix.
 * \param file Pointer on the FileType object structure.
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode FileTransfer_FileType_Create_TmpFile(SOPC_FileType* file);

/**
 * \brief Open the temporary file and lock it.
 * \param file Pointer on the FileType object structure.
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode FileTransfer_Open_TmpFile(SOPC_FileType* file);

/**
 * \brief Close the temporary file and unlock it.
 * \note This function is usefull for the Close method implementation.
 * \param handle The handle of the file to close
 * \param objectId The nodeID of the FileType object on the address space.
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode FileTransfer_Close_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId);

/**
 * \brief Delete the temporary file (removed from the memory location).
 * \param file Pointer on the FileType object structure.
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode FileTransfer_Delete_TmpFile(SOPC_FileType* file);

/**
 * \brief Function to reset the internal data of the FileType structure in case of closing or deleted tmp file.
 * \param file The FileType structure object.
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode FileTransfer_Reset_FileType_Data(SOPC_FileType* file);

/**
 * \brief Read into the temporary file (from the current position).
 * \note This function is usefull for the Read method implementation.
 * \param handle The handle of the file to read.
 * \param length The byte number to read.
 * \param msg The output buffer that is allocated by this function (must be freed by the caller).
 * \param objectId The nodeID of the FileType object on the address space.
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode FileTransfer_Read_TmpFile(SOPC_FileHandle handle,
                                                 int32_t length,
                                                 SOPC_ByteString* msg,
                                                 const SOPC_NodeId* objectId);

/**
 * \brief Write the temporary file (from the current position).
 * \note This function is usefull for the Write method implementation.
 * \param handle The handle of the file to write.
 * \param msg The input buffer to write (pre-allocated)
 * \param objectId The nodeID of the FileType object on the address space.
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode FileTransfer_Write_TmpFile(SOPC_FileHandle handle,
                                                  SOPC_ByteString* msg,
                                                  const SOPC_NodeId* objectId);

/**
 * \brief Set the file position of temporary file.
 * \note This function is usefull for the SetPosition method implementation.
 * \param handle The handle of the file.
 * \param objectId The nodeID of the FileType object on the address space.
 * \param posOff The offset position to set
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode FileTransfer_SetPos_TmpFile(SOPC_FileHandle handle,
                                                   const SOPC_NodeId* objectId,
                                                   uint64_t posOff);

/**
 * \brief Get the file position of temporary file.
 * \note This function is usefull for the GetPosition method implementation.
 * \param handle The handle of the file.
 * \param objectId The nodeID of the FileType object on the address space.
 * \param pos The output position to get.
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode FileTransfer_GetPos_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId, uint64_t* pos);

/**
 * \brief Function to free SOPC_FileType object structure (used for dictionary management purposes).
 * \param value The FileType object
 */
static void filetype_free(void* value);

/**
 * \brief Function to compare two SOPC_FileHandle (used for dictionary management purposes).
 * \param a first SOPC_FileHandle
 * \param b second SOPC_FileHandle
 * \return true if equal else false
 */
static bool handle_equal(const void* a, const void* b);

/**
 * \brief Function to hash a SOPC_FileHandle (used for dictionary management purposes).
 * \param handle the SOPC_FileHandle to hash
 * \return the hash result
 */
static uint64_t handle_hash(const void* handle);

/**
 * \brief Function to generate a random SOPC_FileHandle (current time is used as seed).
 * \note The handle generated is different form 0 which is the invalid internal value.
 * \return the SOPC_FileHandle generate
 */
static SOPC_FileHandle generate_random_handle(void);

/**
 * \brief Function to check if the opening file mode bit mask receive from the client is valid.
 * \param mode The SOPC_OpenMode (bit mask)
 * \return true if valid else false
 */
static bool check_openModeArg(SOPC_OpenMode mode);

/**
 * \brief Function to convert the opening file mode bit mask into a C string mode compatible with fopen function
 * \param mode The SOPC_OpenMode (bit mask)
 * \param Cmode The C string result
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode opcuaMode_to_CMode(SOPC_OpenMode mode, char* Cmode);

/**
 * \brief Function to update the OpenCount variable of a FileType object on the address space (local write request)
 * \param file The FileType structure object
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode local_write_open_count(const SOPC_FileType* file);

/**
 * \brief Function to update the Size variable of a FileType object on the address space (local write request)
 * \param file The FileType structure object
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode local_write_size(const SOPC_FileType* file);

/**
 * \brief Function to write the default value of Writable variable for a FileType object on the address space (local
 * write request)
 * \param file The FileType structure object
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode local_write_default_Writable(const SOPC_FileType* file);

/**
 * \brief Function to write the default value of UserWritable variable for a FileType object on the address space (local
 * write request)
 * \param file The FileType structure object
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode local_write_default_UserWritable(const SOPC_FileType* file);

/**
 * \brief Function used with SOPC_Dict_ForEach. Purpose: For each FileType registered into the API, intialized each
 * variable of the object on the address space.
 * \param key not used here
 * \param value the pointer on the SOPC_FileType
 * \param user_data use to get SOPC_StatusCode (SOPC_STATUT_OK if no error)
 */
static void local_write_all(const void* key, const void* value, void* user_data);

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

/**
 * \brief The asynchronous local service response callback
 */
static void AsyncRespCb_Fct(SOPC_EncodeableType* type, void* response, uintptr_t userContext);

/************************************/
/* STATIC VARIABLE */
/************************************/
static SOPC_Dict* g_objectId_to_file = NULL;
/* g_handle_to_file is reserved for future use (deviation from the OPC UA specification: Currently we don't support
 * multiple handles for the same file)*/
static SOPC_Dict* g_handle_to_file = NULL;
static int32_t g_tombstone_key = -1;
static SOPC_MethodCallManager* g_method_call_manager = NULL;

static bool check_openModeArg(SOPC_OpenMode mode)
{
    uint8_t shall_be_reserved = 0u;
    bool ok1 = true;
    bool ok2 = true;
    if (shall_be_reserved != (mode >> 4u) || shall_be_reserved == mode)
    {
        ok1 = false;
    }
    //  EraseExisting bit can only be set if the file is opened for writing
    if (mode & ERASE_EXISTING_MASK)
    {
        if ((ERASE_EXISTING_MASK + WRITE_MASK) != mode)
        {
            ok2 = false;
        }
    }
    return ok1 && ok2;
}

static void filetype_free(void* value)
{
    if (NULL != value)
    {
        FileTransfer_FileType_Delete((SOPC_FileType**) &value);
    }
}

static bool handle_equal(const void* a, const void* b)
{
    return (*(const SOPC_FileHandle*) a == *(const SOPC_FileHandle*) b);
}

static uint64_t handle_hash(const void* handle)
{
    uint64_t hash = SOPC_DJBHash((const uint8_t*) handle, (size_t) sizeof(SOPC_FileHandle));
    return hash;
}

static SOPC_FileHandle generate_random_handle(void)
{
    /* Initialisation of the seed to generate random handle */
    srand((unsigned int) time(NULL));
    SOPC_FileHandle gen = (uint32_t) rand();
    while (INVALID_HANDLE_VALUE == gen)
    {
        gen = (uint32_t) rand();
    }
    return gen;
}

static SOPC_StatusCode opcuaMode_to_CMode(SOPC_OpenMode mode, char* Cmode)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus;

    SOPC_ASSERT(NULL != Cmode && "Cmode is not initialize");
    switch (mode)
    {
    case 1:
        snprintf(Cmode, 2, "r"); // reading
        break;
    case 2:
        snprintf(Cmode, 2, "w"); // writing
        break;
    case 3:
        snprintf(Cmode, 3, "r+"); // Reading and writing
        break;
    case 6:
        snprintf(Cmode, 3, "w+"); // Reading and writing with erase if existing
        break;
    case 8:
        snprintf(Cmode, 2, "a"); // writing into appening mode
        break;
    case 9:
        snprintf(Cmode, 3, "a+"); // reading and writing
        break;
    case 10:
        snprintf(Cmode, 2, "a"); // writing into appening mode
        break;
    case 11:
        snprintf(Cmode, 3, "a+"); // reading and writing
        break;
    default:
        status = OpcUa_BadInvalidArgument;
        break;
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Method_Open(const SOPC_CallContext* callContextPtr,
                                                const SOPC_NodeId* objectId,
                                                uint32_t nbInputArgs,
                                                const SOPC_Variant* inputArgs,
                                                uint32_t* nbOutputArgs,
                                                SOPC_Variant** outputArgs,
                                                void* param)
{
    (void) callContextPtr;
    (void) param;
    SOPC_StatusCode result_code = SOPC_GoodGenericStatus;
    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: Bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }

    SOPC_Byte mode = inputArgs->Value.Byte;
    bool mode_ok = check_openModeArg(mode);
    if (!mode_ok)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: OpenMode %d is unknown", mode);
        return OpcUa_BadInvalidArgument;
    }

    bool found = false;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:Method_Open: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (false == found)
    {
        char* C_objectId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Open: unable to retieve the tmp file in the API from nodeId '%s'",
                               C_objectId);
        SOPC_Free(C_objectId);
        return OpcUa_BadNotFound;
    }

    if (file->is_open)
    {
        /* Clients can open the same file several times for read */
        /* A request to open for writing shall return Bad_NotWritable when the file is already opened */
        if ((READ_MASK == file->mode) && (READ_MASK != mode))
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "FileTransfer:Method_Open: file is open in read mode, it cannot be opened in write mode");
            return OpcUa_BadNotWritable;
        }
        /* A request to open for reading shall return Bad_NotReadable when the file is already opened for writing.*/
        if ((0 != (file->mode & (WRITE_MASK | APPEND_MASK))) && (0 == (file->mode & READ_MASK)) &&
            (0 == (mode & (WRITE_MASK | APPEND_MASK))))
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "FileTransfer:Method_Open: file is open in write mode, it cannot be opened in read mode");
            return OpcUa_BadNotReadable;
        }
        /* Deviation from the OPC UA specification: an opening followed by a closing, otherwise the file is deleted.*/
        result_code = FileTransfer_Delete_TmpFile(file);
        if (0 != (result_code & SOPC_GoodStatusOppositeMask))
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Open: unable to deleted tmp file");
            return result_code;
        }
    }

    /* g_handle_to_file is reserved for future use (deviation from the OPC UA specification: Currently we don't support
     * multiple handles for the same file)*/
    file->handle = generate_random_handle();
    bool res = SOPC_Dict_Insert(g_handle_to_file, &file->handle, file);
    if (false == res)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Open: unable to insert file into g_handle_to_file dictionary");
        return OpcUa_BadUnexpectedError;
    }

    file->mode = mode;
    result_code = FileTransfer_FileType_Create_TmpFile(file);
    if (0 != (result_code & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: unable to create the tmp file");
        return OpcUa_BadUnexpectedError;
    }

    result_code = FileTransfer_Open_TmpFile(file);
    if (0 != (result_code & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: unable to open the tmp file");
        return OpcUa_BadUnexpectedError;
    }

    file->is_open = true;
    /* OpenCount indicates the number of currently valid file handles on the file.
    as we do not support multiple handlers, this one is maintained at 1 */
    file->open_count = 1;
    /* Start local service on variables */
    result_code = local_write_open_count(file);
    if (0 != (result_code & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "FileTransfer:Method_Open: unable to make a local write request for the OpenCount variable");
        return OpcUa_BadUnexpectedError;
    }

    int filedes = fileno(file->fp);
    if (-1 == filedes)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Open: the fileno function has failed");
        return OpcUa_BadUnexpectedError;
    }

    struct stat sb;
    int ret = fstat(filedes, &sb);
    if (-1 == ret)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Open: unable to get stat on the tmp file");
        return OpcUa_BadUnexpectedError;
    }

    file->size_in_byte = (uint64_t) sb.st_size;
    result_code = local_write_size(file);
    if (0 != (result_code & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Open: unable to make a local write request for the Size variable");
        return OpcUa_BadUnexpectedError;
    }

    /* End local service on variables */
    SOPC_Variant* v = SOPC_Variant_Create();
    if (NULL == v)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: unable to create a variant");
        return OpcUa_BadUnexpectedError;
    }

    v->ArrayType = SOPC_VariantArrayType_SingleValue;
    v->BuiltInTypeId = SOPC_UInt32_Id;
    SOPC_UInt32_Initialize(&v->Value.Uint32);
    v->Value.Uint32 = (uint32_t) file->handle;
    *nbOutputArgs = 1;
    *outputArgs = v;
    result_code = SOPC_GoodGenericStatus;

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
    (void) callContextPtr;
    (void) nbOutputArgs;
    (void) outputArgs;
    (void) param;

    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Close: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }

    if ((SOPC_UInt32_Id != inputArgs->BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Close: bad BuiltInTypeId argument");
        return OpcUa_BadInvalidArgument;
    }

    SOPC_FileHandle handle = inputArgs->Value.Uint32;
    SOPC_StatusCode result_code = FileTransfer_Close_TmpFile(handle, objectId);
    if (0 != (result_code & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Close: error while closing tmp file");
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
    (void) callContextPtr;
    (void) param;

    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:Method_Read: API not initialized with <SOPC_FileTransfer_Initialize>");

    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }

    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_Int32_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: bad BuiltInTypeId arguments");
        return OpcUa_BadInvalidArgument;
    }

    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    int32_t length = inputArgs[1].Value.Int32;

    SOPC_Variant* v = SOPC_Variant_Create(); // Free by the Method Call Manager
    if (NULL == v)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: unable to create a variant");
        return OpcUa_BadResourceUnavailable;
    }

    v->ArrayType = SOPC_VariantArrayType_SingleValue;
    v->BuiltInTypeId = SOPC_ByteString_Id;
    SOPC_ByteString_Initialize(&v->Value.Bstring);
    SOPC_StatusCode result_code = FileTransfer_Read_TmpFile(handle, length, &(v->Value.Bstring), objectId);
    if (0 != (result_code & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: error while reading tmp file");
    }

    SOPC_FileType* file = NULL;
    if (0 == (result_code & SOPC_GoodStatusOppositeMask))
    {
        *nbOutputArgs = 1;
        *outputArgs = v;

        bool found = false;
        file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
        if (false == found)
        {
            char* C_objectId = SOPC_NodeId_ToCString(objectId);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Read: unable to retrieve FileType in the API from nodeId %s",
                                   C_objectId);
            SOPC_Free(C_objectId);
            result_code = OpcUa_BadUnexpectedError;
        }
    }

    int filedes = -1;
    if (0 == (result_code & SOPC_GoodStatusOppositeMask))
    {
        filedes = fileno(file->fp);
        if (-1 == filedes)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Read: the fileno function has failed");
            result_code = OpcUa_BadResourceUnavailable;
        }
    }

    struct stat sb;
    if (0 == (result_code & SOPC_GoodStatusOppositeMask))
    {
        int res = fstat(filedes, &sb);
        if (-1 == res)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Read: unable to get stat on the tmp file");
            result_code = OpcUa_BadResourceUnavailable;
        }
    }

    if (0 == (result_code & SOPC_GoodStatusOppositeMask))
    {
        file->size_in_byte = (uint64_t) sb.st_size;
        result_code = local_write_size(file);
        if (0 != (result_code & SOPC_GoodStatusOppositeMask))
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Read: unable to make a local write request for Size variable");
            result_code = OpcUa_BadUnexpectedError;
        }
    }

    return result_code;
}

static SOPC_StatusCode FileTransfer_Method_Write(const SOPC_CallContext* callContextPtr,
                                                 const SOPC_NodeId* objectId,
                                                 uint32_t nbInputArgs,
                                                 const SOPC_Variant* inputArgs,
                                                 uint32_t* nbOutputArgs,
                                                 SOPC_Variant** outputArgs,
                                                 void* param)
{
    (void) callContextPtr;
    (void) nbOutputArgs;
    (void) outputArgs;
    (void) param;

    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Write: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }

    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_ByteString_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Write: bad BuiltInTypeId arguments");
        return OpcUa_BadInvalidArgument;
    }

    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    SOPC_ByteString data = inputArgs[1].Value.Bstring;
    SOPC_StatusCode result_code = FileTransfer_Write_TmpFile(handle, &data, objectId);
    if (0 != (result_code & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Write: error while writing tmp file");
    }

    return result_code;
}

static SOPC_StatusCode FileTransfer_Method_GetPos(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param)
{
    (void) callContextPtr;
    (void) param;
    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;
    SOPC_StatusCode result_code = SOPC_GoodGenericStatus;

    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_GetPos: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }
    if (SOPC_UInt32_Id != inputArgs->BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_GetPos: bad BuiltInTypeId argument");
        return OpcUa_BadInvalidArgument;
    }

    SOPC_FileHandle handle = inputArgs->Value.Uint32;
    SOPC_Variant* v = SOPC_Variant_Create(); // Free by the Method Call Manager
    if (NULL == v)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_GetPos: unable to create a variant");
        return OpcUa_BadResourceUnavailable;
    }

    v->ArrayType = SOPC_VariantArrayType_SingleValue;
    v->BuiltInTypeId = SOPC_UInt64_Id;
    SOPC_UInt64_Initialize(&v->Value.Uint64);
    result_code = FileTransfer_GetPos_TmpFile(handle, objectId, &(v->Value.Uint64));
    if (0 != (result_code & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_GetPos: error while retrieving the position of the tmp file");
    }

    if (0 == (result_code & SOPC_GoodStatusOppositeMask))
    {
        *nbOutputArgs = 1;
        *outputArgs = v;
    }

    return result_code;
}

static SOPC_StatusCode FileTransfer_Method_SetPos(const SOPC_CallContext* callContextPtr,
                                                  const SOPC_NodeId* objectId,
                                                  uint32_t nbInputArgs,
                                                  const SOPC_Variant* inputArgs,
                                                  uint32_t* nbOutputArgs,
                                                  SOPC_Variant** outputArgs,
                                                  void* param)
{
    (void) callContextPtr;
    (void) nbOutputArgs;
    (void) outputArgs;
    (void) param;

    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_SetPos: bad inputs arguments");
        return OpcUa_BadInvalidArgument;
    }

    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_UInt64_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_SetPos: bad BuiltInTypeId arguments");
        return OpcUa_BadInvalidArgument;
    }

    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    uint64_t pos = inputArgs[1].Value.Uint64;
    SOPC_StatusCode result_code = FileTransfer_SetPos_TmpFile(handle, objectId, pos);
    if (0 != (result_code & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_SetPos: error while setting the position of the tmp file");
    }

    return result_code;
}

SOPC_FileType* FileTransfer_FileType_Create(void)
{
    SOPC_FileType* filetype = NULL;
    filetype = SOPC_Malloc(sizeof(SOPC_FileType));
    if (NULL != filetype)
    {
        FileTransfer_FileType_Initialize(filetype);
    }
    return filetype;
}

static void FileTransfer_FileType_Initialize(SOPC_FileType* filetype)
{
    SOPC_ASSERT(NULL != filetype && "SOPC_FileType pointer must be initialized");
    filetype->handle = INVALID_HANDLE_VALUE;
    filetype->path = NULL;
    filetype->tmp_path = NULL;
    for (int i = 0; i < NB_FILE_TYPE_METHOD; i++)
    {
        filetype->methodIds[i] = NULL;
    }
    for (int i = 0; i < NB_VARIABLE; i++)
    {
        filetype->variableIds[i] = NULL;
    }
    filetype->mode = FileTransfer_UnknownMode;
    filetype->is_open = false;
    filetype->fp = NULL;
    filetype->open_count = 0;
    filetype->size_in_byte = 0;
    filetype->pFunc_UserCloseCallback = NULL;
}

static void FileTransfer_FileType_Clear(SOPC_FileType* filetype)
{
    if (NULL != filetype)
    {
        SOPC_String_Delete(filetype->path);
        SOPC_String_Delete(filetype->tmp_path);
        // filetype->methodIds[i] Free by the MethodCallManager
        for (int i = 0; i < NB_VARIABLE; i++)
        {
            SOPC_NodeId_Clear(filetype->variableIds[i]);
            SOPC_Free(filetype->variableIds[i]);
        }
        FileTransfer_FileType_Initialize(filetype);
    }
}

static void FileTransfer_FileType_Delete(SOPC_FileType** filetype)
{
    if (NULL != filetype && NULL != *filetype)
    {
        FileTransfer_FileType_Clear(*filetype);
        SOPC_Free(*filetype);
        *filetype = NULL;
    }
}

SOPC_ReturnStatus SOPC_FileTransfer_Initialize(void)
{
    if (NULL != g_objectId_to_file || NULL != g_method_call_manager)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Init: The FileTransfer API is already initialized.");
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (SOPC_STATUS_OK == status)
    {
        g_objectId_to_file = SOPC_NodeId_Dict_Create(true, filetype_free);
        if (NULL == g_objectId_to_file)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Init: unable to create dictionary <g_objectId_to_file>");
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        g_method_call_manager = SOPC_MethodCallManager_Create();
        if (NULL == g_method_call_manager)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Init: unable to create the MethodCallManager");
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        g_handle_to_file = SOPC_Dict_Create(NULL, handle_hash, handle_equal, NULL, NULL);
        if (NULL == g_handle_to_file)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Init: unable to create dictionary <g_handle_to_file>");
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetMethodCallManager(g_method_call_manager);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Init: error while configuring the MethodCallManager");
        }
        SOPC_Dict_SetTombstoneKey(g_handle_to_file, &g_tombstone_key);
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_FileTransfer_Clear();
    }
    return status;
}

void SOPC_FileTransfer_Clear(void)
{
    SOPC_Dict_Delete(g_objectId_to_file);
    g_objectId_to_file = NULL;
    SOPC_Dict_Delete(g_handle_to_file);
    g_handle_to_file = NULL;
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();
    // MethodCallManager free by SOPC_CommonHelper_Clear->SOPC_S2OPC_Config_Clear->SOPC_ServerConfig_Clear
    g_method_call_manager = NULL;
}

SOPC_ReturnStatus SOPC_FileTransfer_Add_File(const SOPC_FileType_Config* config)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_FileType* file = NULL;
    SOPC_NodeId* node_id = NULL;
    bool res = false;

    if (NULL == g_objectId_to_file && NULL == g_method_call_manager)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:AddFile: The FileTransfer API is not initialized.");
        return SOPC_STATUS_INVALID_STATE;
    }
    if (NULL == config->fileType_nodeId || NULL == config->file_path || NULL == config->met_openId ||
        NULL == config->met_closeId || NULL == config->met_readId || NULL == config->met_writeId ||
        NULL == config->met_getposId || NULL == config->met_setposId || NULL == config->var_openCountId ||
        NULL == config->var_sizeId || NULL == config->var_userWritableId || NULL == config->var_writableId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:AddFile: The fields of the config argument must be initialized");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    file = FileTransfer_FileType_Create();
    if (NULL == file)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:AddFile: unable to create FileType structure");
        status = SOPC_STATUS_NOK;
    }
    if (SOPC_STATUS_OK == status)
    {
        node_id = SOPC_NodeId_FromCString(config->fileType_nodeId, (int32_t) strlen(config->fileType_nodeId));
        if (NULL == node_id)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create NodeId from a C string for the FileType");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->mode = FileTransfer_UnknownMode;
        file->pFunc_UserCloseCallback = config->pFunc_UserCloseCallback;
        file->path = SOPC_String_Create();
        if (NULL == file->path)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create the path string");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->tmp_path = SOPC_String_Create();
        if (NULL == file->tmp_path)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create the tmp_path string");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_CopyFromCString(file->path, config->file_path);
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to set file path from a C string");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->methodIds[OPEN_METHOD_IDX] =
            SOPC_NodeId_FromCString(config->met_openId, (int32_t) strlen(config->met_openId));
        if (NULL != file->methodIds[OPEN_METHOD_IDX])
        {
            status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[OPEN_METHOD_IDX],
                                                      &FileTransfer_Method_Open, "Open", NULL);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:AddFile: unable to add Open method");
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create NodeId from a C string for Open method");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->methodIds[CLOSE_METHOD_IDX] =
            SOPC_NodeId_FromCString(config->met_closeId, (int32_t) strlen(config->met_closeId));
        if (NULL != file->methodIds[CLOSE_METHOD_IDX])
        {
            status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[CLOSE_METHOD_IDX],
                                                      &FileTransfer_Method_Close, "Close", NULL);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:AddFile: unable to add Close method");
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create NodeId from a C string for Close method");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->methodIds[READ_METHOD_IDX] =
            SOPC_NodeId_FromCString(config->met_readId, (int32_t) strlen(config->met_readId));
        if (NULL != file->methodIds[READ_METHOD_IDX])
        {
            status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[READ_METHOD_IDX],
                                                      &FileTransfer_Method_Read, "Read", NULL);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:AddFile: unable to add Read method");
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create NodeId from a C string for Read method");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->methodIds[WRITE_METHOD_IDX] =
            SOPC_NodeId_FromCString(config->met_writeId, (int32_t) strlen(config->met_writeId));
        if (NULL != file->methodIds[WRITE_METHOD_IDX])
        {
            status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[WRITE_METHOD_IDX],
                                                      &FileTransfer_Method_Write, "Write", NULL);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:AddFile: unable to add Write method");
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create NodeId from a C string for Write method");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->methodIds[GETPOS_METHOD_IDX] =
            SOPC_NodeId_FromCString(config->met_getposId, (int32_t) strlen(config->met_getposId));
        if (NULL != file->methodIds[GETPOS_METHOD_IDX])
        {
            status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[GETPOS_METHOD_IDX],
                                                      &FileTransfer_Method_GetPos, "GetPosition", NULL);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:AddFile: unable to add GetPosition method");
            }
        }
        else
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "FileTransfer:AddFile: unable to create NodeId from a C string for GetPosition method");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->methodIds[SETPOS_METHOD_IDX] =
            SOPC_NodeId_FromCString(config->met_setposId, (int32_t) strlen(config->met_setposId));
        if (NULL != file->methodIds[SETPOS_METHOD_IDX])
        {
            status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[SETPOS_METHOD_IDX],
                                                      &FileTransfer_Method_SetPos, "SetPosition", NULL);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:AddFile: unable to add SetPosition method");
            }
        }
        else
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "FileTransfer:AddFile: unable to create NodeId from a C string for SetPosition method");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->variableIds[SIZE_VAR_IDX] =
            SOPC_NodeId_FromCString(config->var_sizeId, (int32_t) strlen(config->var_sizeId));
        if (NULL == file->variableIds[SIZE_VAR_IDX])
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create NodeId from a C string for Size variable");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->variableIds[OPEN_COUNT_VAR_IDX] =
            SOPC_NodeId_FromCString(config->var_openCountId, (int32_t) strlen(config->var_openCountId));
        if (NULL == file->variableIds[OPEN_COUNT_VAR_IDX])
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "FileTransfer:AddFile: unable to create NodeId from a C string for OpenCount variable");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->variableIds[WRITABLE_VAR_IDX] =
            SOPC_NodeId_FromCString(config->var_writableId, (int32_t) strlen(config->var_writableId));
        if (NULL == file->variableIds[WRITABLE_VAR_IDX])
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "FileTransfer:AddFile: unable to create NodeId from a C string for Writable variable");
            status = SOPC_STATUS_NOK;
        }
    }
    if (SOPC_STATUS_OK == status)
    {
        file->variableIds[USER_WRITABLE_VAR_IDX] =
            SOPC_NodeId_FromCString(config->var_userWritableId, (int32_t) strlen(config->var_userWritableId));
        if (NULL == file->variableIds[USER_WRITABLE_VAR_IDX])
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "FileTransfer:AddFile: unable to create NodeId from a C string for UserWritable variable");
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        res = SOPC_Dict_Insert(g_objectId_to_file, node_id, file);
        if (false == res)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to insert file into dictionary");

            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_FileTransfer_Clear();
        FileTransfer_FileType_Delete(&file);
        SOPC_NodeId_Clear(node_id);
        SOPC_Free(node_id);
        file = NULL;
        node_id = NULL;
    }

    return status;
}

SOPC_ReturnStatus SOPC_FileTransfer_Add_MethodItems(SOPC_MethodCallFunc_Ptr methodFunc,
                                                    char* methodName,
                                                    const char* CnodeId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_NodeId* node_id = NULL;

    if (NULL == g_objectId_to_file && NULL == g_method_call_manager)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Add_MethodItems: The FileTransfer API is not initialized.");
        return SOPC_STATUS_INVALID_STATE;
    }

    if (NULL == methodFunc || NULL == CnodeId || NULL == methodName)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    node_id = SOPC_NodeId_FromCString(CnodeId, (int32_t) strlen(CnodeId));
    if (NULL == node_id)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Add_MethodItems: unable to create NodeId from a C string");
        status = SOPC_STATUS_NOK;
    }
    else
    {
        status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, node_id, methodFunc, methodName, NULL);
        if (SOPC_STATUS_NOK == status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Add_MethodItems: unable to register method '%s'", methodName);
        }
    }
    return status;
}

static SOPC_StatusCode FileTransfer_FileType_Create_TmpFile(SOPC_FileType* file)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    SOPC_ReturnStatus return_status = SOPC_STATUS_OK;
    const char* Cpath = NULL;
    char* tmp_file_path = NULL;
    size_t size_path = 0;
    int res = -1;
    int filedes = -1;
    SOPC_ASSERT(NULL != file && "CreateTmpFile: unexpected error");

    if ((NULL == file->path) || (NULL == file->tmp_path))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:CreateTmpFile: the FileType object is not initialized in the API");
        return OpcUa_BadUnexpectedError;
    }

    Cpath = SOPC_String_GetRawCString(file->path);
    size_path =
        (size_t) file->path->Length +
        STR_MARGIN_SIZE; // Margin if the number of random digits of the mkstemp function changes in a future use.
    tmp_file_path = SOPC_Calloc(size_path, sizeof(char));
    if (NULL == tmp_file_path)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:CreateTmpFile: the calloc has failed (file '%s')", Cpath);
        status = OpcUa_BadUnexpectedError;
    }
    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        res = snprintf(tmp_file_path, size_path, "%s-XXXXXX", SOPC_String_GetRawCString(file->path));
        if (0 > res)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:CreateTmpFile: the snprintf function has failed (file '%s')", Cpath);
            status = OpcUa_BadUnexpectedError;
        }
    }
    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        filedes = mkstemp(tmp_file_path);
        if (0 > filedes)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:CreateTmpFile: the mkstemp function has failed (file '%s')", Cpath);
            status = OpcUa_BadUnexpectedError;
        }
    }
    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        res = close(filedes);
        if (0 != res)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:CreateTmpFile: the close function has failed (file '%s')", Cpath);
            status = OpcUa_BadUnexpectedError;
        }
    }
    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        return_status = SOPC_String_InitializeFromCString(file->tmp_path, (const char*) tmp_file_path);
        if (SOPC_STATUS_OK != return_status)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "FileTransfer:CreateTmpFile: the InitializeFromCString function has failed (file '%s')", Cpath);
            status = OpcUa_BadUnexpectedError;
        }
    }

    SOPC_Free(tmp_file_path);
    tmp_file_path = NULL;
    return status;
}

static SOPC_StatusCode FileTransfer_Open_TmpFile(SOPC_FileType* file)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    int res = -1;
    int filedes = -1;
    char Cmode[5] = {0};
    bool mode_is_ok = check_openModeArg(file->mode);

    if (NULL == file)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:OpenTmpFile: the FileType object is not initialized in the API");
        return OpcUa_BadUnexpectedError;
    }

    if (false == mode_is_ok)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:OpenTmpFile: bad openning mode");
        return OpcUa_BadInvalidArgument;
    }

    status = opcuaMode_to_CMode(file->mode, Cmode);
    if (0 != (status & SOPC_GoodStatusOppositeMask))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:OpenTmpFile: unable to decode mode to fopen function");
        return OpcUa_BadInvalidArgument;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        if (NULL != file->fp)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:OpenTmpFile: the file pointer is already initialized");
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        file->fp = fopen(SOPC_String_GetRawCString(file->tmp_path), Cmode);
        if (NULL == file->fp)
        {
            const char* str = SOPC_String_GetRawCString(file->tmp_path);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:OpenTmpFile: the fopen function has failed (file '%s')", str);
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        filedes = fileno(file->fp);
        if (-1 == filedes)
        {
            const char* str = SOPC_String_GetRawCString(file->tmp_path);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:OpenTmpFile: the fileno function has failed (file '%s')", str);
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        res = flock(filedes, LOCK_SH);
        if (0 != res)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:OpenTmpFile: unable to lock the file");
            status = OpcUa_BadUnexpectedError;
        }
    }

    return status;
}

static SOPC_StatusCode FileTransfer_Reset_FileType_Data(SOPC_FileType* file)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus;

    file->fp = NULL;
    file->is_open = false;
    file->size_in_byte = 0;
    file->open_count = 0;
    local_write_open_count(file);
    /* Invalid the file handle in the API */
    file->handle = INVALID_HANDLE_VALUE;
    /* Free and creat a new tmp_path */
    SOPC_String_Delete(file->tmp_path);
    file->tmp_path = NULL;
    file->tmp_path = SOPC_String_Create();
    if (NULL == file->tmp_path)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:ResetFileType: unable to create a new tmp_path string");
        status = OpcUa_BadResourceUnavailable;
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Close_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId)
{
    int res = -1;
    int filedes = -1;
    bool found = false;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:CloseTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (false == found)
    {
        char* C_objectId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:CloseTmpFile: unable to retrieve file in the API from nodeId '%s'",
                               C_objectId);
        SOPC_Free(C_objectId);
        return OpcUa_BadUnexpectedError;
    }

    if ((handle != file->handle) || (INVALID_HANDLE_VALUE == handle))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:CloseTmpFile: unexpected file handle");
        return OpcUa_BadInvalidArgument;
    }

    if ((NULL == file->fp) || (NULL == file->tmp_path))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:CloseTmpFile: the file pointer or the file path are not initialized");
        return OpcUa_BadUnexpectedError;
    }

    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    filedes = fileno(file->fp);
    if (-1 == filedes)
    {
        const char* str = SOPC_String_GetRawCString(file->tmp_path);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:CloseTmpFile: the fileno function has failed (file '%s')", str);
        status = OpcUa_BadResourceUnavailable;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        res = flock(filedes, LOCK_UN);
        if (0 != res)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:CloseTmpFile: unable to unlock the file");
            status = OpcUa_BadResourceUnavailable;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        res = fclose(file->fp);
        if (0 != res)
        {
            const char* str = SOPC_String_GetRawCString(file->tmp_path);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:CloseTmpFile: the fclose function has failed (file '%s')", str);
            status = OpcUa_BadResourceUnavailable;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        /* User close callback */
        if (NULL != file->pFunc_UserCloseCallback)
        {
            file->pFunc_UserCloseCallback(SOPC_String_GetRawCString(file->tmp_path));
        }
        status = FileTransfer_Reset_FileType_Data(file);
    }

    return status;
}

static SOPC_StatusCode FileTransfer_Delete_TmpFile(SOPC_FileType* file)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    int res = -1;
    int filedes = -1;
    if (NULL == file)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:DeleteTmpFile: the FileType object is not initialized in the API");
        return OpcUa_BadUnexpectedError;
    }

    if (NULL == file->fp)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:DeleteTmpFile: the file pointer is not initialized");
        return OpcUa_BadUnexpectedError;
    }

    filedes = fileno(file->fp);
    if (-1 == filedes)
    {
        const char* str = SOPC_String_GetRawCString(file->tmp_path);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:DeleteTmpFile: the fileno function has failed (file '%s')", str);
        status = OpcUa_BadResourceUnavailable;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        res = flock(filedes, LOCK_UN);
        if (0 != res)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:DeleteTmpFile: unable to unlock the file");
            status = OpcUa_BadResourceUnavailable;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        res = fclose(file->fp);
        if (0 != res)
        {
            const char* str = SOPC_String_GetRawCString(file->tmp_path);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:DeleteTmpFile: the fclose function has failed (file '%s')", str);
            status = OpcUa_BadResourceUnavailable;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        res = remove(SOPC_String_GetRawCString(file->tmp_path));
        if (0 != res)
        {
            const char* str = SOPC_String_GetRawCString(file->tmp_path);
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:DeleteTmpFile: the remove function has failed (file '%s')", str);
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        status = FileTransfer_Reset_FileType_Data(file);
    }

    return status;
}

static SOPC_StatusCode FileTransfer_Read_TmpFile(SOPC_FileHandle handle,
                                                 int32_t length,
                                                 SOPC_ByteString* msg,
                                                 const SOPC_NodeId* objectId)
{
    SOPC_StatusCode status = SOPC_STATUS_OK;
    bool found = false;
    size_t read_count = 0;
    long int old_pos = -1;
    long int last_pos = -1;
    int res = -1;
    int32_t size_available = -1;

    SOPC_ASSERT(NULL != msg && "unexpected internal error");
    SOPC_ASSERT(NULL == msg->Data && "unexpected internal error");

    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:ReadTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (false == found)
    {
        char* C_objectId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:ReadTmpFile: unable to retrieve file in the API from nodeId '%s'",
                               C_objectId);
        SOPC_Free(C_objectId);
        return OpcUa_BadUnexpectedError;
    }

    if (0 >= length)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "FileTransfer:ReadTmpFile: only positive values are allowed for the length argument, rcv len: %d", length);
        return OpcUa_BadInvalidArgument;
    }

    if ((handle != file->handle) || (INVALID_HANDLE_VALUE == handle))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:ReadTmpFile: unexpected file handle");
        return OpcUa_BadInvalidArgument;
    }

    /* check if File was not opened for read access */
    if (false == file->is_open || 0 == (file->mode & READ_MASK))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:ReadTmpFile: file has not been opened for read access");
        return OpcUa_BadInvalidState;
    }

    if (NULL == file->fp)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:ReadTmpFile: the file pointer is not initialized");
        return OpcUa_BadUnexpectedError;
    }

    /* Calculate the size to allocate */
    old_pos = ftell(file->fp);
    if (-1L == old_pos)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:ReadTmpFile: the ftell function has failed");
        return OpcUa_BadResourceUnavailable;
    }

    res = fseek(file->fp, 0, SEEK_END);
    if (0 != res)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:ReadTmpFile: the fseek function has failed");
        return OpcUa_BadResourceUnavailable;
    }

    last_pos = ftell(file->fp);
    if (-1L == last_pos)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:ReadTmpFile: the ftell function has failed");
        return OpcUa_BadResourceUnavailable;
    }

    res = fseek(file->fp, old_pos, SEEK_SET);
    if (0 != res)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:ReadTmpFile: the fseek function has failed");
        return OpcUa_BadResourceUnavailable;
    }

    size_available = (int32_t) last_pos - (int32_t) old_pos;
    if (length > size_available)
    {
        msg->Length = size_available;
    }
    else
    {
        msg->Length = length;
    }

    msg->Data = SOPC_Malloc((size_t) msg->Length);
    if (NULL == msg->Data)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:ReadTmpFile: unable to allocate memory for reading the message");
        status = OpcUa_BadResourceUnavailable;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        read_count = fread(msg->Data, 1, (size_t) msg->Length, file->fp);
        if (read_count != (size_t) msg->Length)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:ReadTmpFile: the fread function has failed");
            status = OpcUa_BadResourceUnavailable;
        }
    }

    if (0 != (status & SOPC_GoodStatusOppositeMask))
    {
        if (msg->Data != NULL)
        {
            SOPC_Free(msg->Data);
        }
        msg->Length = -1;
        msg->Data = NULL;
        msg->DoNotClear = false;
    }

    return status;
}

static SOPC_StatusCode FileTransfer_Write_TmpFile(SOPC_FileHandle handle,
                                                  SOPC_ByteString* msg,
                                                  const SOPC_NodeId* objectId)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    bool found = false;
    char* buffer = NULL;
    size_t ret = 0;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:WriteTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (false == found)
    {
        char* C_objectId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteTmpFile: unable to retrieve file in the API from nodeId '%s'",
                               C_objectId);
        SOPC_Free(C_objectId);
        return OpcUa_BadInvalidState;
    }
    if ((handle != file->handle) || (INVALID_HANDLE_VALUE == handle))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:WriteTmpFile: unexpected file handle");
        return OpcUa_BadInvalidArgument;
    }

    /* check if File was not opened for write access */
    if ((false == file->is_open) || (READ_MASK == file->mode))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteTmpFile: file has not been opened for write access");
        return OpcUa_BadInvalidState;
    }

    if (NULL == msg)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteTmpFile: invalid pointer for the ByteString msg");
        return OpcUa_BadInvalidArgument;
    }

    if (NULL == file->fp)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:WriteTmpFile: invalid file pointer");
        return OpcUa_BadInvalidState;
    }

    /* Writing an empty or null ByteString returns a Good result code without any affect on the file. */
    if (-1 == msg->Length)
    {
        return SOPC_GoodGenericStatus;
    }

    if (NULL == msg->Data)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteTmpFile: ByteString msg has not been allocated");
        return OpcUa_BadInvalidArgument;
    }

    buffer = SOPC_Malloc((size_t) msg->Length);
    if (NULL == buffer)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteTmpFile: unable to allocate memory for writing the message");
        status = OpcUa_BadResourceUnavailable;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        memcpy(buffer, msg->Data, (size_t) msg->Length);
        /* If ret != msg->Length then file might be locked and thus not writable */
        ret = fwrite(buffer, 1, (size_t) msg->Length, file->fp);
        if ((size_t) msg->Length != ret)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:WriteTmpFile: the fwrite function has failed");
            status = OpcUa_BadNotWritable;
        }
    }

    SOPC_Free(buffer);
    return status;
}

static SOPC_StatusCode FileTransfer_GetPos_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId, uint64_t* pos)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    bool found = false;
    long int ret = -1L;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:GetPosTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");

    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (false == found)
    {
        char* C_objectId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:GetPosTmpFile: unable to retrieve file in the API from nodeId '%s'",
                               C_objectId);
        SOPC_Free(C_objectId);
        return OpcUa_BadInvalidArgument;
    }

    if ((handle != file->handle) || (INVALID_HANDLE_VALUE == handle))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:GetPosTmpFile: unexpected file handle");
        status = OpcUa_BadInvalidArgument;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        if (NULL == file->fp)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:GetPosTmpFile: the file pointer is not initialized");
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        *pos = 0;
        ret = ftell(file->fp);
        if (-1L == ret)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:GetPosTmpFile: the ftell function has failed");
            status = OpcUa_BadResourceUnavailable;
        }
        else
        {
            *pos = (uint64_t) ret;
        }
    }

    return status;
}

static SOPC_StatusCode FileTransfer_SetPos_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId, uint64_t posOff)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    bool found = false;
    int ret = -1;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:SetPosTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (false == found)
    {
        char* C_objectId = SOPC_NodeId_ToCString(objectId);
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:SetPosTmpFile: unable to retrieve file in the API from nodeId '%s'",
                               C_objectId);
        SOPC_Free(C_objectId);
        return OpcUa_BadInvalidArgument;
    }
    if ((handle != file->handle) || (INVALID_HANDLE_VALUE == handle))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:SetPosTmpFile: unexpected file handle");
        status = OpcUa_BadInvalidArgument;
    }
    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        if (NULL == file->fp)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:SetPosTmpFile: The file pointer is not initialized");
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        ret = fseek(file->fp, (long int) posOff, SEEK_SET);
        if (0 != ret)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:SetPosTmpFile: the fseek function has failed");
            status = OpcUa_BadResourceUnavailable;
        }
    }
    return status;
}

static void local_write_all(const void* key, const void* value, void* user_data)
{
    (void) key;
    const SOPC_FileType* file = value;
    SOPC_ReturnStatus* status = user_data;
    SOPC_StatusCode res = SOPC_GoodGenericStatus;
    res = local_write_default_UserWritable(file);
    if (SOPC_GoodGenericStatus != res)
    {
        *status = SOPC_STATUS_NOK;
    }
    res = local_write_default_Writable(file);
    if (SOPC_GoodGenericStatus != res)
    {
        *status = SOPC_STATUS_NOK;
    }
    res = local_write_open_count(file);
    if (SOPC_GoodGenericStatus != res)
    {
        *status = SOPC_STATUS_NOK;
    }
    res = local_write_size(file);
    if (SOPC_GoodGenericStatus != res)
    {
        *status = SOPC_STATUS_NOK;
    }
}

SOPC_ReturnStatus SOPC_FileTransfer_StartServer(SOPC_ServerStopped_Fct* ServerStoppedCallback)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    status = SOPC_HelperConfigServer_SetLocalServiceAsyncResponse(&AsyncRespCb_Fct);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(ServerStoppedCallback);
    }
    if (SOPC_STATUS_OK == status)
    {
        /* Initialize each variables for each each FileType added into the API */
        SOPC_Dict_ForEach(g_objectId_to_file, &local_write_all, &status);
    }
    return status;
}

static void AsyncRespCb_Fct(SOPC_EncodeableType* type, void* response, uintptr_t userContext)
{
    (void) type;
    (void) response;
    (void) userContext;
}

static SOPC_StatusCode local_write_open_count(const SOPC_FileType* file)
{
    SOPC_ASSERT(NULL != file->variableIds[OPEN_COUNT_VAR_IDX] &&
                "OpenCount variable nodeId shall be added with <SOPC_FileTransfer_Add_File>");
    SOPC_ReturnStatus return_status = SOPC_STATUS_OK;
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (NULL == pReq)
    {
        return OpcUa_BadUnexpectedError;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        SOPC_NodeId* nodeId = file->variableIds[OPEN_COUNT_VAR_IDX];
        SOPC_DataValue dataValue = {.Value = {.BuiltInTypeId = SOPC_UInt16_Id,
                                              .ArrayType = SOPC_VariantArrayType_SingleValue,
                                              .Value.Uint16 = file->open_count},
                                    .Status = SOPC_GoodGenericStatus};

        return_status = SOPC_WriteRequest_SetWriteValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dataValue);
        if (SOPC_STATUS_OK != return_status)
        {
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        return_status = SOPC_ServerHelper_LocalServiceAsync(pReq, 1);
        if (SOPC_STATUS_OK != return_status)
        {
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 != (status & SOPC_GoodStatusOppositeMask))
    {
        OpcUa_WriteRequest_Clear(pReq);
    }

    return status;
}

static SOPC_StatusCode local_write_size(const SOPC_FileType* file)
{
    SOPC_ASSERT(NULL != file->variableIds[SIZE_VAR_IDX] &&
                "Size variable nodeId shall be added with <SOPC_FileTransfer_Add_Variable_To_File>");
    SOPC_ReturnStatus return_status = SOPC_STATUS_OK;
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (NULL == pReq)
    {
        return OpcUa_BadUnexpectedError;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        SOPC_NodeId* nodeId = file->variableIds[SIZE_VAR_IDX];
        SOPC_DataValue dataValue = {.Value = {.BuiltInTypeId = SOPC_UInt64_Id,
                                              .ArrayType = SOPC_VariantArrayType_SingleValue,
                                              .Value.Uint64 = file->size_in_byte},
                                    .Status = SOPC_GoodGenericStatus};

        return_status = SOPC_WriteRequest_SetWriteValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dataValue);
        if (SOPC_STATUS_OK != return_status)
        {
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        return_status = SOPC_ServerHelper_LocalServiceAsync(pReq, 1);
        if (SOPC_STATUS_OK != return_status)
        {
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 != (status & SOPC_GoodStatusOppositeMask))
    {
        OpcUa_WriteRequest_Clear(pReq);
    }

    return status;
}

static SOPC_StatusCode local_write_default_Writable(const SOPC_FileType* file)
{
    SOPC_ASSERT(NULL != file->variableIds[WRITABLE_VAR_IDX] &&
                "Writable variable nodeId shall be added with <SOPC_FileTransfer_Add_File>");
    SOPC_ReturnStatus return_status = SOPC_STATUS_OK;
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (NULL == pReq)
    {
        return OpcUa_BadUnexpectedError;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        SOPC_NodeId* nodeId = file->variableIds[WRITABLE_VAR_IDX];
        SOPC_DataValue dataValue = {.Value = {.BuiltInTypeId = SOPC_Boolean_Id,
                                              .ArrayType = SOPC_VariantArrayType_SingleValue,
                                              .Value.Boolean = VAR_WRITABLE_DEFAULT},
                                    .Status = SOPC_GoodGenericStatus};

        return_status = SOPC_WriteRequest_SetWriteValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dataValue);
        if (SOPC_STATUS_OK != return_status)
        {
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        return_status = SOPC_ServerHelper_LocalServiceAsync(pReq, 1);
        if (SOPC_STATUS_OK != return_status)
        {
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 != (status & SOPC_GoodStatusOppositeMask))
    {
        OpcUa_WriteRequest_Clear(pReq);
    }

    return status;
}

static SOPC_StatusCode local_write_default_UserWritable(const SOPC_FileType* file)
{
    SOPC_ASSERT(NULL != file->variableIds[USER_WRITABLE_VAR_IDX] &&
                "UserWritable variable nodeId shall be added with <SOPC_FileTransfer_Add_File>");
    SOPC_ReturnStatus return_status = SOPC_STATUS_OK;
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (NULL == pReq)
    {
        return OpcUa_BadUnexpectedError;
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        SOPC_NodeId* nodeId = file->variableIds[USER_WRITABLE_VAR_IDX];
        SOPC_DataValue dataValue = {.Value = {.BuiltInTypeId = SOPC_Boolean_Id,
                                              .ArrayType = SOPC_VariantArrayType_SingleValue,
                                              .Value.Boolean = VAR_USER_WRITABLE_DEFAULT},
                                    .Status = SOPC_GoodGenericStatus};

        return_status = SOPC_WriteRequest_SetWriteValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dataValue);
        if (SOPC_STATUS_OK != return_status)
        {
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 == (status & SOPC_GoodStatusOppositeMask))
    {
        return_status = SOPC_ServerHelper_LocalServiceAsync(pReq, 1);
        if (SOPC_STATUS_OK != return_status)
        {
            status = OpcUa_BadUnexpectedError;
        }
    }

    if (0 != (status & SOPC_GoodStatusOppositeMask))
    {
        OpcUa_WriteRequest_Clear(pReq);
    }

    return status;
}
