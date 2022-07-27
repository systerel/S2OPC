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
    SOPC_NodeId* node_id;   /*!< The nodeId of the FileType object into the adress space. */
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
 * \brief Read into the temporary file (from the current position).
 * \note This function is usefull for the Read method implementation.
 * \param handle The handle of the file to read.
 * \param length The byte number to read
 * \param msg The output buffer
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
 * \param msg The input buffer to write
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
static SOPC_StatusCode local_write_open_count(SOPC_FileType file);

/**
 * \brief Function to update the Size variable of a FileType object on the address space (local write request)
 * \param file The FileType structure object
 * \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode local_write_size(SOPC_FileType file);

/**
 * \brief Function to write the default value of Writable variable for a FileType object on the address space (local
 * write request) \param file The FileType structure object \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode local_write_default_Writable(SOPC_FileType file);

/**
 * \brief Function to write the default value of UserWritable variable for a FileType object on the address space (local
 * write request) \param file The FileType structure object \return SOPC_GoodGenericStatus if no error
 */
static SOPC_StatusCode local_write_default_UserWritable(SOPC_FileType file);

/**
 * \brief Function used with SOPC_Dict_ForEach
 * \note Purpose: For each FileType registered into the API, intialized each variable of the object on the address
 * space. \param key not used here \param value the pointer on the SOPC_FileType \param user_data use to get
 * SOPC_StatusCode (SOPC_STATUT_OK if no error)
 */
static void local_write_init(const void* key, const void* value, void* user_data);

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

static void filetransfer_results_set_service_result(SOPC_Boolean res);
static SOPC_Boolean filetransfer_results_get_service_result(void);
static void filetransfer_results_set_service_status(SOPC_Boolean res);
static SOPC_Boolean filetransfer_results_get_service_status(void);

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
static int32_t g_valid_service_result = false;
static int32_t g_service_status = false;

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
    uint64_t hash = SOPC_DJBHash((const uint8_t*) handle, (size_t) strlen(handle));
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
    SOPC_StatusCode result_code = OpcUa_BadInvalidArgument;
    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: Bad inputs arguments");
        return result_code;
    }

    SOPC_Byte mode = inputArgs->Value.Byte;
    bool mode_ok = check_openModeArg(mode);
    if (!mode_ok)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: OpenMode %d is unknown", mode);
        return result_code;
    }

    bool found = false;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:Method_Open: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
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
            /* A request to open for reading shall return Bad_NotReadable when the file is already opened for writing.
             */
            if ((WRITE_MASK == file->mode) || (APPEND_MASK == file->mode) || ((APPEND_MASK + WRITE_MASK) == file->mode))
            {
                if ((WRITE_MASK != mode) && (APPEND_MASK != mode) && ((APPEND_MASK + WRITE_MASK) != mode))
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "FileTransfer:Method_Open: file is open in write mode, it cannot be opened in read mode");
                    return OpcUa_BadNotReadable;
                }
            }
            /* Deviation from the OPC UA specification: an opening followed by a closing, otherwise the file is deleted
             */
            result_code = FileTransfer_Delete_TmpFile(file);
            if (SOPC_GoodGenericStatus != result_code)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:Method_Open: unable to deleted tmp file");
                return result_code;
            }
        }
        /* g_handle_to_file is reserved for future use (deviation from the OPC UA specification: Currently we don't
         * support multiple handles for the same file)*/
        file->handle = generate_random_handle();
        bool res = SOPC_Dict_Insert(g_handle_to_file, &file->handle, file);
        SOPC_ASSERT(true == res);
        file->mode = mode;
        result_code = FileTransfer_FileType_Create_TmpFile(file);
        if (SOPC_GoodGenericStatus == result_code)
        {
            result_code = FileTransfer_Open_TmpFile(file);
            if (SOPC_GoodGenericStatus == result_code)
            {
                file->is_open = true;
                /* OpenCount indicates the number of currently valid file handles on the file.
                as we do not support multiple handlers, this one is maintained at 1 */
                file->open_count = 1;
                /* Start local service on variables */
                result_code = local_write_open_count(*file);
                if (SOPC_GoodGenericStatus != result_code)
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "FileTransfer:Method_Open: unable to make a local write request for the OpenCount variable");
                }
                struct stat sb;
                int ret = fstat(fileno(file->fp), &sb);
                if (-1 != ret)
                {
                    file->size_in_byte = (uint64_t) sb.st_size;
                    result_code = local_write_size(*file);
                    if (SOPC_GoodGenericStatus != result_code)
                    {
                        SOPC_Logger_TraceError(
                            SOPC_LOG_MODULE_CLIENTSERVER,
                            "FileTransfer:Method_Open: unable to make a local write request for the Size variable");
                    }
                }
                else
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "FileTransfer:Method_Open: unable to get stat on the tmp file");
                }
                /* End local service on variables */
                if (SOPC_GoodGenericStatus == result_code)
                {
                    SOPC_Variant* v = SOPC_Variant_Create();
                    if (NULL != v)
                    {
                        v->ArrayType = SOPC_VariantArrayType_SingleValue;
                        v->BuiltInTypeId = SOPC_UInt32_Id;
                        SOPC_UInt32_Initialize(&v->Value.Uint32);
                        v->Value.Uint32 = (uint32_t) file->handle;
                        *nbOutputArgs = 1;
                        *outputArgs = v;
                        result_code = SOPC_GoodGenericStatus;
                    }
                    else
                    {
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "FileTransfer:Method_Open: unable to create a variant");
                        result_code = OpcUa_BadOutOfMemory;
                    }
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:Method_Open: unable to open the tmp file");
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Open: unable to create the tmp file");
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Method_Open: unable to retieve the tmp file in the API");
        result_code = OpcUa_BadNotFound;
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
    (void) callContextPtr;
    (void) nbOutputArgs;
    (void) outputArgs;
    (void) param;
    SOPC_StatusCode result_code = OpcUa_BadInvalidArgument;

    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Close: bad inputs arguments");
        return result_code;
    }

    if ((SOPC_UInt32_Id != inputArgs->BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Close: bad BuiltInTypeId argument");
        return result_code;
    }

    SOPC_FileHandle handle = inputArgs->Value.Uint32;
    result_code = FileTransfer_Close_TmpFile(handle, objectId);
    if (SOPC_GoodGenericStatus != result_code)
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
    SOPC_StatusCode result_code = OpcUa_BadInvalidArgument;

    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: bad inputs arguments");
        return result_code;
    }

    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_Int32_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: bad BuiltInTypeId arguments");
        return result_code;
    }

    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    int32_t length = inputArgs[1].Value.Int32;

    SOPC_Variant* v = SOPC_Variant_Create(); // Free by the Method Call Manager
    if (NULL != v)
    {
        v->ArrayType = SOPC_VariantArrayType_SingleValue;
        v->BuiltInTypeId = SOPC_ByteString_Id;
        SOPC_ByteString_Initialize(&v->Value.Bstring);
        result_code = FileTransfer_Read_TmpFile(handle, length, &(v->Value.Bstring), objectId);
        if (SOPC_GoodGenericStatus == result_code)
        {
            *nbOutputArgs = 1;
            *outputArgs = v;
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Read: error while reading tmp file");
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: unable to create a variant");
        result_code = OpcUa_BadOutOfMemory;
    }
    if (SOPC_GoodGenericStatus == result_code)
    {
        bool found = false;
        SOPC_ASSERT(g_objectId_to_file != NULL &&
                    "FileTransfer:Method_Read: API not initialized with <SOPC_FileTransfer_Initialize>");
        SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
        if (found)
        {
            struct stat sb;
            int res = fstat(fileno(file->fp), &sb);
            if (-1 != res)
            {
                file->size_in_byte = (uint64_t) sb.st_size;
                result_code = local_write_size(*file);
                if (SOPC_GoodGenericStatus != result_code)
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "FileTransfer:Method_Read: unable to make a local write request for Size variable");
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:Method_Read: unable to get stat on the tmp file");
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_Read: unable to retrieve FileType in the API");
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
    SOPC_StatusCode result_code = OpcUa_BadInvalidArgument;

    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Write: bad inputs arguments");
        return result_code;
    }
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_ByteString_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Write: bad BuiltInTypeId arguments");
        return result_code;
    }
    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    SOPC_ByteString data = inputArgs[1].Value.Bstring;
    result_code = FileTransfer_Write_TmpFile(handle, &data, objectId);
    if (SOPC_GoodGenericStatus != result_code)
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
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;
    SOPC_StatusCode result_code = OpcUa_BadInvalidArgument;

    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_GetPos: bad inputs arguments");
        return result_code;
    }
    if (SOPC_UInt32_Id != inputArgs->BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_GetPos: bad BuiltInTypeId argument");
        return result_code;
    }

    SOPC_FileHandle handle = inputArgs->Value.Uint32;
    SOPC_Variant* v = SOPC_Variant_Create(); // Free by the Method Call Manager
    if (NULL != v)
    {
        v->ArrayType = SOPC_VariantArrayType_SingleValue;
        v->BuiltInTypeId = SOPC_UInt64_Id;
        SOPC_UInt64_Initialize(&v->Value.Uint64);
        result_code = FileTransfer_GetPos_TmpFile(handle, objectId, &(v->Value.Uint64));
        if (SOPC_GoodGenericStatus == result_code)
        {
            *nbOutputArgs = 1;
            *outputArgs = v;
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Method_GetPos: error while retrieving the position of the tmp file");
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_GetPos: unable to create a variant");
        result_code = OpcUa_BadOutOfMemory;
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
    SOPC_StatusCode result_code = OpcUa_BadInvalidArgument;

    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_SetPos: bad inputs arguments");
        return result_code;
    }
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_UInt64_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_SetPos: bad BuiltInTypeId arguments");
        return result_code;
    }
    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    uint64_t pos = inputArgs[1].Value.Uint64;
    result_code = FileTransfer_SetPos_TmpFile(handle, objectId, pos);
    if (SOPC_GoodGenericStatus != result_code)
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
    SOPC_ASSERT(NULL != filetype && "SOPC_FileType pointer needs to be initialize");
    filetype->node_id = NULL;
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
        SOPC_NodeId_Clear(filetype->node_id);
        SOPC_String_Delete(filetype->path);
        SOPC_String_Delete(filetype->tmp_path);
        for (int i = 0; i < NB_FILE_TYPE_METHOD; i++)
        {
            SOPC_NodeId_Clear(filetype->methodIds[i]);
        }
        for (int i = 0; i < NB_VARIABLE; i++)
        {
            SOPC_NodeId_Clear(filetype->variableIds[i]);
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
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (NULL != g_objectId_to_file || NULL != g_method_call_manager)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:Init: The FileTransfer API is already initialized.");
        status = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        g_objectId_to_file = SOPC_NodeId_Dict_Create(true, filetype_free);
        if (NULL == g_objectId_to_file)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Init: unable to create dictionary <g_objectId_to_file>");
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        g_method_call_manager = SOPC_MethodCallManager_Create();
        if (NULL == g_method_call_manager)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Init: unable to create the MethodCallManager");
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        g_handle_to_file = SOPC_Dict_Create(NULL, handle_hash, handle_equal, NULL, NULL);
        if (NULL == g_handle_to_file)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Init: unable to create dictionary <g_handle_to_file>");
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            status = SOPC_HelperConfigServer_SetMethodCallManager(g_method_call_manager);
            if (SOPC_STATUS_OK != status)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:Init: error while configuring the MethodCallManager");
            }
            SOPC_Dict_SetTombstoneKey(g_handle_to_file, &g_tombstone_key);
        }
    }
    return status;
}

void SOPC_FileTransfer_Clear(void)
{
    SOPC_Dict_Delete(g_objectId_to_file);
    g_objectId_to_file = NULL;
    SOPC_Dict_Delete(g_handle_to_file);
    g_handle_to_file = NULL;
    SOPC_MethodCallManager_Free(g_method_call_manager);
    g_method_call_manager = NULL;
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();
}

SOPC_ReturnStatus SOPC_FileTransfer_Add_File(const SOPC_FileType_Config* config)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_FileType* file;
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
    file->mode = FileTransfer_UnknownMode;
    file->pFunc_UserCloseCallback = config->pFunc_UserCloseCallback;
    file->node_id = SOPC_NodeId_FromCString(config->fileType_nodeId, (int32_t) strlen(config->fileType_nodeId));
    if (NULL == file->node_id)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:AddFile: unable to create NodeId from a C string for the FileType");
        status = SOPC_STATUS_NOK;
    }
    file->path = SOPC_String_Create();
    if (NULL == file->path)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:AddFile: unable to create the path string");
        status = SOPC_STATUS_NOK;
    }
    file->tmp_path = SOPC_String_Create();
    if (NULL == file->tmp_path)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:AddFile: unable to create the tmp_path string");
        status = SOPC_STATUS_NOK;
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
        res = SOPC_Dict_Insert(g_objectId_to_file, file->node_id, file);
        if (false == res)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to insert file into dictionary");
            FileTransfer_FileType_Delete(&file); // The FileType will not be deleted throught <SOPC_FileTransfer_Clear>
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK != status)
    {
        SOPC_FileTransfer_Clear(); // Unitialize the API
        status = SOPC_STATUS_NOK;
    }

    return status;
}

SOPC_ReturnStatus SOPC_FileTransfer_Add_MethodItems(SOPC_MethodCallFunc_Ptr methodFunc,
                                                    char* methodName,
                                                    const char* CnodeId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_NodeId* node_id;
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
    if ((NULL != file->node_id) && (NULL != file->path) && (NULL != file->tmp_path))
    {
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
                                       "FileTransfer:CreateTmpFile: the snprintf function has failed (file '%s')",
                                       Cpath);
                status = OpcUa_BadUnexpectedError;
            }
        }
        if (0 == (status & SOPC_GoodStatusOppositeMask))
        {
            filedes = mkstemp(tmp_file_path);
            if (0 > filedes)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:CreateTmpFile: the mkstemp function has failed (file '%s')",
                                       Cpath);
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
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:CreateTmpFile: the FileType object is not initialize in the API");
        status = OpcUa_BadUnexpectedError;
    }

    SOPC_Free(tmp_file_path);
    tmp_file_path = NULL;
    return status;
}

static SOPC_StatusCode FileTransfer_Open_TmpFile(SOPC_FileType* file)
{
    SOPC_StatusCode status;
    int res;
    char Cmode[5] = {0};
    bool mode_is_ok = check_openModeArg(file->mode);
    if (mode_is_ok)
    {
        if (NULL != file)
        {
            if (NULL == file->fp)
            {
                status = opcuaMode_to_CMode(file->mode, Cmode);
                if (SOPC_GoodGenericStatus == status)
                {
                    file->fp = fopen(SOPC_String_GetRawCString(file->tmp_path), Cmode);
                    if (NULL == file->fp)
                    {
                        const char* str = SOPC_String_GetRawCString(file->tmp_path);
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "FileTransfer:OpenTmpFile: the fopen function has failed (file '%s')",
                                               str);
                        SOPC_ASSERT(NULL != file->fp && "tmp file can't be open");
                    }
                    res = flock(fileno(file->fp), LOCK_SH);
                    if (0 != res)
                    {
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "FileTransfer:OpenTmpFile: unable to lock the file");
                        SOPC_ASSERT(0 == res && "the tmp file can't be locked");
                    }
                }
                else
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "FileTransfer:OpenTmpFile: unable to decode mode to fopen function");
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:OpenTmpFile: the file pointer is already initialized");
                status = OpcUa_BadOutOfMemory;
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:OpenTmpFile: the FileType object is not initialized in the API");
            status = OpcUa_BadOutOfMemory;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:OpenTmpFile: bad openning mode");
        status = OpcUa_BadInvalidArgument;
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Close_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId)
{
    SOPC_StatusCode status;
    int res;
    bool found = false;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:CloseTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if ((handle == file->handle) && (INVALID_HANDLE_VALUE != handle))
        {
            status = SOPC_GoodGenericStatus;
            if (file->is_open)
            {
                if ((NULL != file->fp) && (NULL != file->tmp_path))
                {
                    res = flock(fileno(file->fp), LOCK_UN);
                    if (0 != res)
                    {
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "FileTransfer:CloseTmpFile: unable to unlock the file");
                        SOPC_ASSERT(0 == res && "the tmp file can't be unlocked");
                    }
                    res = fclose(file->fp);
                    if (0 != res)
                    {
                        const char* str = SOPC_String_GetRawCString(file->tmp_path);
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "FileTransfer:CloseTmpFile: the fclose function has failed (file '%s')",
                                               str);
                        SOPC_ASSERT(0 == res && "file can't be closed");
                    }
                    /* User close callback */
                    if (NULL != file->pFunc_UserCloseCallback)
                    {
                        file->pFunc_UserCloseCallback(SOPC_String_GetRawCString(file->tmp_path));
                    }
                    file->fp = NULL;
                    file->is_open = false;
                    file->size_in_byte = 0;
                    file->open_count = 0;
                    local_write_open_count(*file);
                    /* Remove the file handle in the API and invalid it */
                    /* g_handle_to_file is reserved for future use (deviation from the OPC UA specification: Currently
                     * we don't support multiple handles for the same file)*/
                    SOPC_Dict_Remove(g_handle_to_file, &file->handle);
                    file->handle = INVALID_HANDLE_VALUE;
                    /* Free and creat a new tmp_path */
                    SOPC_String_Delete(file->tmp_path);
                    file->tmp_path = NULL;
                    file->tmp_path = SOPC_String_Create();
                }
                else
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "FileTransfer:CloseTmpFile: the file pointer or the file path are not initialized");
                    status = OpcUa_BadOutOfMemory;
                }
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:CloseTmpFile: unexpected file handle");
            status = OpcUa_BadInvalidArgument;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:CloseTmpFile: unable to retrieve file in the API");
        status = OpcUa_BadUnexpectedError;
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Delete_TmpFile(SOPC_FileType* file)
{
    SOPC_StatusCode status = OpcUa_BadOutOfMemory;
    int res;
    if (NULL != file)
    {
        if (NULL != file->fp)
        {
            res = flock(fileno(file->fp), LOCK_UN);
            if (0 != res)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:DeleteTmpFile: unable to unlock the file");
                SOPC_ASSERT(0 == res && "the tmp file can't be unlocked");
            }
            res = fclose(file->fp);
            if (0 != res)
            {
                const char* str = SOPC_String_GetRawCString(file->tmp_path);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:DeleteTmpFile: the fclose function has failed (file '%s')", str);
                SOPC_ASSERT(0 == res && "tmp file can't be closed");
            }
            res = remove(SOPC_String_GetRawCString(file->tmp_path));
            if (0 != res)
            {
                const char* str = SOPC_String_GetRawCString(file->tmp_path);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:DeleteTmpFile: the remove function has failed (file '%s')", str);
                SOPC_ASSERT(0 == res && "tmp file can't be remove");
            }
            file->fp = NULL;
            file->is_open = false;
            file->size_in_byte = 0;
            file->open_count = 0;
            /* Remove the file handle in the API and invalid it */
            /* g_handle_to_file is reserved for future use (deviation from the OPC UA specification: Currently we don't
             * support multiple handles for the same file)*/
            SOPC_Dict_Remove(g_handle_to_file, &file->handle);
            file->handle = INVALID_HANDLE_VALUE;
            /* Free and creat a new tmp_path */
            SOPC_String_Delete(file->tmp_path);
            file->tmp_path = NULL;
            file->tmp_path = SOPC_String_Create();
            status = SOPC_GoodGenericStatus;
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:DeleteTmpFile: the file pointer is not initialized");
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:DeleteTmpFile: the FileType object is not initialized in the API");
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Read_TmpFile(SOPC_FileHandle handle,
                                                 int32_t length,
                                                 SOPC_ByteString* msg,
                                                 const SOPC_NodeId* objectId)
{
    SOPC_StatusCode status;
    SOPC_ReturnStatus sopc_status;
    bool found = false;
    size_t read_count;
    char* buffer = NULL;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:ReadTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (0 >= length)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "FileTransfer:ReadTmpFile: only positive values are allowed for the length argument");
            return OpcUa_BadInvalidArgument;
        }
        if ((handle == file->handle) && (INVALID_HANDLE_VALUE != handle))
        {
            /* check if File was not opened for read access */
            if ((file->is_open == true) && ((file->mode == WRITE_MASK) || (file->mode == APPEND_MASK) ||
                                            (file->mode == (APPEND_MASK + WRITE_MASK))))
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:ReadTmpFile: file has not been opened for read access");
                return OpcUa_BadInvalidState;
            }

            buffer = SOPC_Calloc((size_t)(length + 1), sizeof(char));

            if (NULL != msg)
            {
                if (NULL != file->fp)
                {
                    read_count = fread(buffer, 1, (size_t) length, file->fp);
                    int end_of_file = feof(file->fp);
                    if ((read_count < (size_t) length) && (0 == end_of_file))
                    {
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "FileTransfer:ReadTmpFile: the fread function has failed");
                        status = OpcUa_BadUnexpectedError;
                    }
                    else
                    {
                        sopc_status = SOPC_String_CopyFromCString(msg, (const char*) buffer);
                        if (SOPC_STATUS_OK != sopc_status)
                        {
                            SOPC_Logger_TraceError(
                                SOPC_LOG_MODULE_CLIENTSERVER,
                                "FileTransfer:ReadTmpFile: the SOPC_String_CopyFromCString function has failed");
                            status = OpcUa_BadUnexpectedError;
                        }
                        else
                        {
                            status = SOPC_GoodGenericStatus;
                        }
                    }
                }
                else
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "FileTransfer:ReadTmpFile: the file pointer is not initialized");
                    status = OpcUa_BadOutOfMemory;
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:ReadTmpFile: ByteString msg has not been allocated");
                status = OpcUa_BadOutOfMemory;
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:ReadTmpFile: unexpected file handle");
            status = OpcUa_BadInvalidArgument;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:ReadTmpFile: unable to retrieve file in the API");
        status = OpcUa_BadUnexpectedError;
    }

    SOPC_Free(buffer);
    return status;
}

static SOPC_StatusCode FileTransfer_Write_TmpFile(SOPC_FileHandle handle,
                                                  SOPC_ByteString* msg,
                                                  const SOPC_NodeId* objectId)
{
    SOPC_StatusCode status;
    bool found = false;
    char* buffer = NULL;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:WriteTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if ((handle == file->handle) && (INVALID_HANDLE_VALUE != handle))
        {
            /* check if File was not opened for write access */
            if ((file->is_open) && (READ_MASK != file->mode))
            {
                if (NULL != msg)
                {
                    if (NULL != file->fp)
                    {
                        /* Writing an empty or null ByteString returns a Good result code without any affect on the
                         * file. */
                        if (-1 == msg->Length)
                        {
                            return SOPC_GoodGenericStatus;
                        }
                        size_t ret;
                        buffer = SOPC_Malloc((size_t) msg->Length);
                        memcpy(buffer, msg->Data, (size_t) msg->Length);
                        /* If ret != msg->Length then file might be locked and thus not writable */
                        ret = fwrite(buffer, 1, (size_t) msg->Length, file->fp);
                        if ((size_t) msg->Length == ret)
                        {
                            status = SOPC_GoodGenericStatus;
                        }
                        else
                        {
                            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                                   "FileTransfer:WriteTmpFile: the fwrite function has failed");
                            status = OpcUa_BadNotWritable;
                        }
                    }
                    else
                    {
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "FileTransfer:WriteTmpFile: the file pointer is not initialized");
                        status = OpcUa_BadOutOfMemory;
                    }
                }
                else
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "FileTransfer:WriteTmpFile: ByteString msg has not been allocated");
                    status = OpcUa_BadOutOfMemory;
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:WriteTmpFile: file has not been opened for write access");
                status = OpcUa_BadInvalidState;
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:WriteTmpFile: unexpected file handle");
            status = OpcUa_BadInvalidArgument;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteTmpFile: unable to retrieve file in the API");
        status = OpcUa_BadUnexpectedError;
    }

    SOPC_Free(buffer);
    return status;
}

static SOPC_StatusCode FileTransfer_GetPos_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId, uint64_t* pos)
{
    SOPC_StatusCode status;
    bool found = false;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:GetPosTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        status = OpcUa_BadInvalidArgument;
        if ((handle == file->handle) && (INVALID_HANDLE_VALUE != handle))
        {
            *pos = 0;
            if (NULL != file->fp)
            {
                long int ret;
                ret = ftell(file->fp);
                if (-1L == ret)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "FileTransfer:GetPosTmpFile: the ftell function has failed");
                    status = OpcUa_BadUnexpectedError;
                }
                else
                {
                    *pos = (uint64_t) ret;
                    status = SOPC_GoodGenericStatus;
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:GetPosTmpFile: the file pointer is not initialized");
                status = OpcUa_BadOutOfMemory;
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:GetPosTmpFile: unexpected file handle");
            status = OpcUa_BadInvalidArgument;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:GetPosTmpFile: unable to retrieve file in the API");
        status = OpcUa_BadUnexpectedError;
    }
    return status;
}

static SOPC_StatusCode FileTransfer_SetPos_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId, uint64_t posOff)
{
    SOPC_StatusCode status;
    bool found = false;
    SOPC_ASSERT(g_objectId_to_file != NULL &&
                "FileTransfer:SetPosTmpFile: API not initialized with <SOPC_FileTransfer_Initialize>");
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if ((handle == file->handle) && (INVALID_HANDLE_VALUE != handle))
        {
            status = SOPC_GoodGenericStatus;
            if (NULL != file->fp)
            {
                int ret;
                ret = fseek(file->fp, (long int) posOff, SEEK_SET);
                if (0 != ret)
                {
                    SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                           "FileTransfer:SetPosTmpFile: the fseek function has failed");
                    status = OpcUa_BadUnexpectedError;
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:SetPosTmpFile: The file pointer is not initialized");
                status = OpcUa_BadOutOfMemory;
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:SetPosTmpFile: unexpected file handle");
            status = OpcUa_BadInvalidArgument;
        }
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:SetPosTmpFile: unable to retrieve file in the API");
        status = OpcUa_BadUnexpectedError;
    }
    return status;
}

static void local_write_init(const void* key, const void* value, void* user_data)
{
    (void) key;
    const SOPC_FileType* file = value;
    SOPC_ReturnStatus* status = user_data;
    SOPC_StatusCode res;
    res = local_write_default_UserWritable(*file);
    if (SOPC_GoodGenericStatus != res)
    {
        *status = SOPC_STATUS_NOK;
    }
    res = local_write_default_Writable(*file);
    if (SOPC_GoodGenericStatus != res)
    {
        *status = SOPC_STATUS_NOK;
    }
    res = local_write_open_count(*file);
    if (SOPC_GoodGenericStatus != res)
    {
        *status = SOPC_STATUS_NOK;
    }
    res = local_write_size(*file);
    if (SOPC_GoodGenericStatus != res)
    {
        *status = SOPC_STATUS_NOK;
    }
}

SOPC_ReturnStatus SOPC_FileTransfer_StartServer(SOPC_ServerStopped_Fct* ServerStoppedCallback)
{
    SOPC_ReturnStatus status;
    status = SOPC_HelperConfigServer_SetLocalServiceAsyncResponse(&AsyncRespCb_Fct);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_StartServer(ServerStoppedCallback);
    }
    if (SOPC_STATUS_OK == status)
    {
        /* Initialize each variables for each each FileType added into the API */
        SOPC_Dict_ForEach(g_objectId_to_file, &local_write_init, &status);
    }
    return status;
}

static void AsyncRespCb_Fct(SOPC_EncodeableType* type, void* response, uintptr_t userContext)
{
    if (type == &OpcUa_ReadResponse_EncodeableType)
    {
        SOPC_ReturnStatus status = SOPC_STATUS_OK;
        OpcUa_ReadResponse* readResp = (OpcUa_ReadResponse*) response;
        SOPC_VariantValue value = readResp->Results->Value.Value;
        switch (readResp->Results->Value.BuiltInTypeId)
        {
        case SOPC_Null_Id:
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:ReadVariable: SOPC_Null_Id is not supported");
            status = SOPC_STATUS_NOK;
            break;
        case SOPC_Boolean_Id:
            *(SOPC_Boolean*) userContext = value.Boolean;
            break;
        case SOPC_SByte_Id:
            *(SOPC_SByte*) userContext = value.Sbyte;
            break;
        case SOPC_Byte_Id:
            *(SOPC_Byte*) userContext = value.Byte;
            break;
        case SOPC_Int16_Id:
            *(int16_t*) userContext = value.Int16;
            break;
        case SOPC_UInt16_Id:
            *(uint16_t*) userContext = value.Uint16;
            break;
        case SOPC_Int32_Id:
            *(int32_t*) userContext = value.Int32;
            break;
        case SOPC_UInt32_Id:
            *(uint32_t*) userContext = value.Uint32;
            break;
        case SOPC_Int64_Id:
            *(int64_t*) userContext = value.Int64;
            break;
        case SOPC_UInt64_Id:
            *(uint64_t*) userContext = value.Uint64;
            break;
        case SOPC_Float_Id:
            *(float*) userContext = value.Floatv;
            break;
        case SOPC_Double_Id:
            *(double*) userContext = value.Doublev;
            break;
        case SOPC_String_Id:
            status = SOPC_String_Copy((SOPC_String*) userContext, &value.String);
            break;
        case SOPC_DateTime_Id:
            status = SOPC_DateTime_CopyAux((SOPC_DateTime*) userContext, &value.Date);
            break;
        case SOPC_Guid_Id:
            status = SOPC_Guid_Copy((SOPC_Guid*) userContext, value.Guid);
            break;
        case SOPC_ByteString_Id:
            status = SOPC_ByteString_Copy((SOPC_ByteString*) userContext, &value.Bstring);
            break;
        case SOPC_XmlElement_Id:
            status = SOPC_XmlElement_Copy((SOPC_XmlElement*) userContext, &value.XmlElt);
            break;
        case SOPC_NodeId_Id:
            status = SOPC_NodeId_Copy((SOPC_NodeId*) userContext, value.NodeId);
            break;
        case SOPC_ExpandedNodeId_Id:
            status = SOPC_ExpandedNodeId_Copy((SOPC_ExpandedNodeId*) userContext, value.ExpNodeId);
            break;
        case SOPC_StatusCode_Id:
            status = SOPC_StatusCode_CopyAux((SOPC_StatusCode*) userContext, &value.Status);
            break;
        case SOPC_QualifiedName_Id:
            status = SOPC_QualifiedName_Copy((SOPC_QualifiedName*) userContext, value.Qname);
            break;
        case SOPC_LocalizedText_Id:
            status = SOPC_LocalizedText_Copy((SOPC_LocalizedText*) userContext, value.LocalizedText);
            break;
        case SOPC_ExtensionObject_Id:
            status = SOPC_ExtensionObject_Copy((SOPC_ExtensionObject*) userContext, value.ExtObject);
            break;
        case SOPC_DataValue_Id:
            status = SOPC_DataValue_Copy((SOPC_DataValue*) userContext, value.DataValue);
            break;
        case SOPC_Variant_Id:
            // Part 6 Table 14 (v1.03): "The value shall not be a Variant
            // but it could be an array of Variants."
            // Note: Variant is not encoded in S2OPC stack for this case
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:ReadVariable: SOPC_Variant_Id is not supported");
            status = SOPC_STATUS_NOK;
            break;
        case SOPC_DiagnosticInfo_Id:
            status = SOPC_DiagnosticInfo_Copy((SOPC_DiagnosticInfo*) userContext, value.DiagInfo);
            break;
        default:
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:ReadVariable: UserBuiltInId value is not supported");
            status = SOPC_STATUS_NOK;
            break;
        }
        if (SOPC_STATUS_OK == status)
        {
            filetransfer_results_set_service_status(true);
        }
        else
        {
            filetransfer_results_set_service_status(false);
        }
        filetransfer_results_set_service_result(true);
    }
}

static SOPC_StatusCode local_write_open_count(SOPC_FileType file)
{
    SOPC_ASSERT(NULL != file.variableIds[OPEN_COUNT_VAR_IDX] &&
                "OpenCount variable nodeId shall be added with <SOPC_FileTransfer_Add_File>");
    SOPC_ReturnStatus status;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (NULL == pReq)
    {
        return OpcUa_BadUnexpectedError;
    }
    SOPC_NodeId* nodeId = file.variableIds[OPEN_COUNT_VAR_IDX];
    SOPC_DataValue dataValue = {.Value = {.BuiltInTypeId = SOPC_UInt16_Id,
                                          .ArrayType = SOPC_VariantArrayType_SingleValue,
                                          .Value.Uint16 = file.open_count},
                                .Status = SOPC_GoodGenericStatus};

    status = SOPC_WriteRequest_SetWriteValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dataValue);
    if (SOPC_STATUS_OK != status)
    {
        return OpcUa_BadUnexpectedError;
    }
    status = SOPC_ServerHelper_LocalServiceAsync(pReq, 1);

    if (SOPC_STATUS_OK != status)
    {
        return OpcUa_BadUnexpectedError;
    }

    return SOPC_GoodGenericStatus;
}

static SOPC_StatusCode local_write_size(SOPC_FileType file)
{
    SOPC_ASSERT(NULL != file.variableIds[SIZE_VAR_IDX] &&
                "Size variable nodeId shall be added with <SOPC_FileTransfer_Add_Variable_To_File>");
    SOPC_ReturnStatus status;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (NULL == pReq)
    {
        return OpcUa_BadUnexpectedError;
    }
    SOPC_NodeId* nodeId = file.variableIds[SIZE_VAR_IDX];
    SOPC_DataValue dataValue = {.Value = {.BuiltInTypeId = SOPC_UInt64_Id,
                                          .ArrayType = SOPC_VariantArrayType_SingleValue,
                                          .Value.Uint64 = file.size_in_byte},
                                .Status = SOPC_GoodGenericStatus};

    status = SOPC_WriteRequest_SetWriteValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dataValue);
    if (SOPC_STATUS_OK != status)
    {
        return OpcUa_BadUnexpectedError;
    }
    status = SOPC_ServerHelper_LocalServiceAsync(pReq, 1);

    if (SOPC_STATUS_OK != status)
    {
        return OpcUa_BadUnexpectedError;
    }

    return SOPC_GoodGenericStatus;
}

static SOPC_StatusCode local_write_default_Writable(SOPC_FileType file)
{
    SOPC_ASSERT(NULL != file.variableIds[WRITABLE_VAR_IDX] &&
                "Writable variable nodeId shall be added with <SOPC_FileTransfer_Add_File>");
    SOPC_ReturnStatus status;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (NULL == pReq)
    {
        return OpcUa_BadUnexpectedError;
    }
    SOPC_NodeId* nodeId = file.variableIds[WRITABLE_VAR_IDX];
    SOPC_DataValue dataValue = {.Value = {.BuiltInTypeId = SOPC_Boolean_Id,
                                          .ArrayType = SOPC_VariantArrayType_SingleValue,
                                          .Value.Boolean = VAR_WRITABLE_DEFAULT},
                                .Status = SOPC_GoodGenericStatus};

    status = SOPC_WriteRequest_SetWriteValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dataValue);
    if (SOPC_STATUS_OK != status)
    {
        return OpcUa_BadUnexpectedError;
    }
    status = SOPC_ServerHelper_LocalServiceAsync(pReq, 1);

    if (SOPC_STATUS_OK != status)
    {
        return OpcUa_BadUnexpectedError;
    }

    return SOPC_GoodGenericStatus;
}

static SOPC_StatusCode local_write_default_UserWritable(SOPC_FileType file)
{
    SOPC_ASSERT(NULL != file.variableIds[USER_WRITABLE_VAR_IDX] &&
                "UserWritable variable nodeId shall be added with <SOPC_FileTransfer_Add_File>");
    SOPC_ReturnStatus status;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (NULL == pReq)
    {
        return OpcUa_BadUnexpectedError;
    }
    SOPC_NodeId* nodeId = file.variableIds[USER_WRITABLE_VAR_IDX];
    SOPC_DataValue dataValue = {.Value = {.BuiltInTypeId = SOPC_Boolean_Id,
                                          .ArrayType = SOPC_VariantArrayType_SingleValue,
                                          .Value.Boolean = VAR_USER_WRITABLE_DEFAULT},
                                .Status = SOPC_GoodGenericStatus};

    status = SOPC_WriteRequest_SetWriteValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dataValue);
    if (SOPC_STATUS_OK != status)
    {
        return OpcUa_BadUnexpectedError;
    }
    status = SOPC_ServerHelper_LocalServiceAsync(pReq, 1);

    if (SOPC_STATUS_OK != status)
    {
        return OpcUa_BadUnexpectedError;
    }

    return SOPC_GoodGenericStatus;
}

SOPC_ReturnStatus SOPC_FileTransfer_WriteVariable(const char* CnodeId, SOPC_BuiltinId UserBuiltInId, void* UserValue)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (NULL == pReq)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteVariable: unable to create WriteRequest");
        return SOPC_STATUS_NOK;
    }
    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(CnodeId, (int32_t) strlen(CnodeId));
    if (NULL == nodeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteVariable: unable to create SOPC_NodeId from C string");
        return SOPC_STATUS_NOK;
    }

    SOPC_DataValue dataValue;
    dataValue.Value.BuiltInTypeId = UserBuiltInId;
    dataValue.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    dataValue.Status = SOPC_GoodGenericStatus;

    switch (UserBuiltInId)
    {
    case SOPC_Null_Id:
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteVariable: SOPC_Null_Id is not supported");
        status = SOPC_STATUS_NOK;
        break;
    case SOPC_Boolean_Id:
        dataValue.Value.Value.Boolean = *(SOPC_Boolean*) UserValue;
        break;
    case SOPC_SByte_Id:
        dataValue.Value.Value.Sbyte = *(SOPC_SByte*) UserValue;
        break;
    case SOPC_Byte_Id:
        dataValue.Value.Value.Byte = *(SOPC_Byte*) UserValue;
        break;
    case SOPC_Int16_Id:
        dataValue.Value.Value.Int16 = *(int16_t*) UserValue;
        break;
    case SOPC_UInt16_Id:
        dataValue.Value.Value.Uint16 = *(uint16_t*) UserValue;
        break;
    case SOPC_Int32_Id:
        dataValue.Value.Value.Int32 = *(int32_t*) UserValue;
        break;
    case SOPC_UInt32_Id:
        dataValue.Value.Value.Uint32 = *(uint32_t*) UserValue;
        break;
    case SOPC_Int64_Id:
        dataValue.Value.Value.Int64 = *(int64_t*) UserValue;
        break;
    case SOPC_UInt64_Id:
        dataValue.Value.Value.Uint64 = *(uint64_t*) UserValue;
        break;
    case SOPC_Float_Id:
        dataValue.Value.Value.Floatv = *(float*) UserValue;
        break;
    case SOPC_Double_Id:
        dataValue.Value.Value.Doublev = *(double*) UserValue;
        break;
    case SOPC_String_Id:
        dataValue.Value.Value.String = *(SOPC_String*) UserValue;
        break;
    case SOPC_DateTime_Id:
        dataValue.Value.Value.Date = *(SOPC_DateTime*) UserValue;
        break;
    case SOPC_Guid_Id:
        dataValue.Value.Value.Guid = (SOPC_Guid*) UserValue;
        break;
    case SOPC_ByteString_Id:
        dataValue.Value.Value.Bstring = *(SOPC_ByteString*) UserValue;
        break;
    case SOPC_XmlElement_Id:
        dataValue.Value.Value.XmlElt = *(SOPC_XmlElement*) UserValue;
        break;
    case SOPC_NodeId_Id:
        dataValue.Value.Value.NodeId = (SOPC_NodeId*) UserValue;
        break;
    case SOPC_ExpandedNodeId_Id:
        dataValue.Value.Value.ExpNodeId = (SOPC_ExpandedNodeId*) UserValue;
        break;
    case SOPC_StatusCode_Id:
        dataValue.Value.Value.Status = *(SOPC_StatusCode*) UserValue;
        break;
    case SOPC_QualifiedName_Id:
        dataValue.Value.Value.Qname = (SOPC_QualifiedName*) UserValue;
        break;
    case SOPC_LocalizedText_Id:
        dataValue.Value.Value.LocalizedText = (SOPC_LocalizedText*) UserValue;
        break;
    case SOPC_ExtensionObject_Id:
        dataValue.Value.Value.ExtObject = (SOPC_ExtensionObject*) UserValue;
        break;
    case SOPC_DataValue_Id:
        dataValue.Value.Value.DataValue = (SOPC_DataValue*) UserValue;
        break;
    case SOPC_Variant_Id:
        // Part 6 Table 14 (v1.03): "The value shall not be a Variant
        // but it could be an array of Variants."
        // Note: Variant is not encoded in S2OPC stack for this case
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteVariable: SOPC_Variant_Id is not supported");
        status = SOPC_STATUS_NOK;
        break;
    case SOPC_DiagnosticInfo_Id:
        dataValue.Value.Value.DiagInfo = (SOPC_DiagnosticInfo*) UserValue;
        break;
    default:
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:WriteVariable: UserBuiltInId value is not supported");
        status = SOPC_STATUS_NOK;
        break;
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_WriteRequest_SetWriteValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL, &dataValue);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_ServerHelper_LocalServiceAsync(pReq, 1);
        }
    }

    SOPC_NodeId_Clear(nodeId);
    nodeId = NULL;
    return status;
}

static void filetransfer_results_set_service_result(SOPC_Boolean res)
{
    SOPC_Atomic_Int_Set(&g_valid_service_result, res ? true : false);
}

static SOPC_Boolean filetransfer_results_get_service_result(void)
{
    return SOPC_Atomic_Int_Get(&g_valid_service_result) == 1;
}

static void filetransfer_results_set_service_status(SOPC_Boolean res)
{
    SOPC_Atomic_Int_Set(&g_service_status, res ? true : false);
}

static SOPC_Boolean filetransfer_results_get_service_status(void)
{
    return SOPC_Atomic_Int_Get(&g_service_status) == 1;
}

SOPC_ReturnStatus SOPC_FileTransfer_ReadVariable(const char* CnodeId, void* pUserValue, uint32_t timeout)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    OpcUa_ReadRequest* pReq = SOPC_ReadRequest_Create(1u, OpcUa_TimestampsToReturn_Neither);
    const uint32_t sleepTimeout = 50;
    // Counter to stop waiting on timeout
    uint32_t loopCpt = 0;

    if (NULL == pReq)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:ReadVariable: unable to create ReadRequest");
        return SOPC_STATUS_NOK;
    }

    SOPC_NodeId* nodeId = SOPC_NodeId_FromCString(CnodeId, (int32_t) strlen(CnodeId));
    if (NULL == nodeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:ReadVariable: unable to create SOPC_NodeId from C string");
        return SOPC_STATUS_NOK;
    }

    status = SOPC_ReadRequest_SetReadValue(pReq, 0, nodeId, SOPC_AttributeId_Value, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerHelper_LocalServiceAsync(pReq, (uintptr_t) pUserValue);
    }
    if (SOPC_STATUS_OK != status)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:ReadVariable: local read asynchronous request: NOK");
    }

    /* Wait until service response is received */
    loopCpt = 0;
    while (SOPC_STATUS_OK == status && filetransfer_results_get_service_result() == false &&
           loopCpt * sleepTimeout <= timeout)
    {
        loopCpt++;
        SOPC_Sleep(sleepTimeout);
    }

    SOPC_Boolean res = filetransfer_results_get_service_status();
    if (false == res)
    {
        status = SOPC_STATUS_NOK;
    }
    // Reset expected status
    filetransfer_results_set_service_status(true);
    // Reset expected result
    filetransfer_results_set_service_result(false);

    SOPC_NodeId_Clear(nodeId);
    nodeId = NULL;

    return status;
}
