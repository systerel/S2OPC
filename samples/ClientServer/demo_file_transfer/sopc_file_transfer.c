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

#include "sopc_builtintypes.h"
#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_mem_alloc.h"

#include "opcua_statuscodes.h"

/**
 * \brief Number of method per FileType Object
 */
#define NB_FILE_TYPE_METHOD 6

/**
 * \brief Number of variable to be update by method for a FileType Object
 */
#define NB_VARIABLE 4

/**
 * \brief A buffer size to manage C string
 */
#define STR_BUFF_SIZE 100

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
 * \brief Default value for UserWritable variable of FileType Object
 */
#define VAR_USER_WRITABLE_DEFAULT true
/**
 * \brief Default value for Writable variable of FileType Object
 */
#define VAR_WRITABLE_DEFAULT true

/**
 * \brief File handle type (to send to client)
 */
typedef uint32_t SOPC_FileHandle;

/**
 * \brief Open mode type (receive from the client, bit mask)
 */
typedef SOPC_Byte SOPC_OpenMode;

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
 * \brief Function to free a C string (used for dictionary management purposes).
 * \param value The C string
 */
static void cstring_free(void* value);

/**
 * \brief Function to compare two C string (used for dictionary management purposes).
 * \param a first C string
 * \param b second C string
 * \return true if equal else false
 */
static bool cstring_equal(const void* a, const void* b);

/**
 * \brief Function to hash a C string (used for dictionary management purposes).
 * \param cstring the C string to hash
 * \return the hash result
 */
static uint64_t cstring_hash(const void* cstring);

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

/************************************/
/* STATIC VARIABLE */
/************************************/
static SOPC_Dict* g_objectId_to_file = NULL;
static SOPC_Dict* g_str_objectId_to_file = NULL;
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

static void cstring_free(void* value)
{
    if (NULL != value)
    {
        SOPC_Free(value);
    }
}

static bool cstring_equal(const void* a, const void* b)
{
    return (strcmp((const char*) a, (const char*) b) == 0);
}

static uint64_t cstring_hash(const void* cstring)
{
    uint64_t hash = SOPC_DJBHash((const uint8_t*) cstring, (size_t) strlen(cstring));
    return hash;
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

    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: Bad inputs arguments");
        /* avoid hard indentation level */
        return result_code;
    }

    SOPC_Byte mode = inputArgs->Value.Byte;
    bool mode_ok = check_openModeArg(mode);
    if (!mode_ok)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Open: OpenMode %d is unknown", mode);
        /* avoid hard indentation level */
        return result_code;
    }

    bool found = false;
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
                    "FileTransfer:Method_Open: file is open into reading mode, can't open for writing");
                /* avoid hard indentation level */
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
                        "FileTransfer:Method_Open: file is open into writing mode, can't open for reading");
                    /* avoid hard indentation level */
                    return OpcUa_BadNotReadable;
                }
            }
            result_code = FileTransfer_Delete_TmpFile(file);
            if (SOPC_GoodGenericStatus != result_code)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:Method_Open: unable to deleted tmp file");
                /* avoid hard indentation level */
                return result_code;
            }
        }
        bool res = SOPC_Dict_Insert(g_handle_to_file, &file->handle, file);
        SOPC_ASSERT(true == res);
        file->mode = mode;
        result_code = FileTransfer_FileType_Create_TmpFile(file);
        if (SOPC_GoodGenericStatus == result_code)
        {
            result_code = FileTransfer_Open_TmpFile(file);
            if (SOPC_GoodGenericStatus == result_code)
            {
                file->handle = file->handle + 1;
                file->is_open = true;
                file->open_count++;
                /* Start local service on variables */
                result_code = local_write_open_count(*file);
                if (SOPC_GoodGenericStatus != result_code)
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "FileTransfer:Method_Open: unable to make a local write request for OpenCount variable");
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
                            "FileTransfer:Method_Open: unable to make a local write request for Size variable");
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
        /* avoid hard indentation level */
        return result_code;
    }

    if ((SOPC_UInt32_Id != inputArgs->BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Close: bad BuiltInTypeId argument");
        /* avoid hard indentation level */
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
    SOPC_StatusCode result_code = OpcUa_BadInvalidArgument;

    if ((2 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: bad inputs arguments");
        /* avoid hard indentation level */
        return result_code;
    }

    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_Int32_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Read: bad BuiltInTypeId arguments");
        /* avoid hard indentation level */
        return result_code;
    }

    SOPC_FileHandle handle = inputArgs[0].Value.Uint32;
    int32_t length = inputArgs[1].Value.Int32;

    SOPC_Variant* v = SOPC_Variant_Create();
    if (NULL != v)
    {
        v->ArrayType = SOPC_VariantArrayType_SingleValue;
        v->BuiltInTypeId = SOPC_ByteString_Id;
        SOPC_ByteString_Initialize(&v->Value.Bstring);
        result_code = FileTransfer_Read_TmpFile(handle, length, &(v->Value.Bstring), objectId);
        if (SOPC_GoodGenericStatus != result_code)
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
    *nbOutputArgs = 1;
    *outputArgs = v;

    bool found = false;
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
        /* avoid hard indentation level */
        return result_code;
    }
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_ByteString_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_Write: bad BuiltInTypeId arguments");
        /* avoid hard indentation level */
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
    SOPC_StatusCode result_code = OpcUa_BadInvalidArgument;
    printf("GetPos\n");

    if ((1 != nbInputArgs) || (NULL == inputArgs) || (NULL == objectId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_GetPos: bad inputs arguments");
        /* avoid hard indentation level */
        return result_code;
    }
    if (SOPC_UInt32_Id != inputArgs->BuiltInTypeId)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_GetPos: bad BuiltInTypeId argument");
        /* avoid hard indentation level */
        return result_code;
    }

    SOPC_FileHandle handle = inputArgs->Value.Uint32;
    SOPC_Variant* v = SOPC_Variant_Create();
    if (NULL != v)
    {
        v->ArrayType = SOPC_VariantArrayType_SingleValue;
        v->BuiltInTypeId = SOPC_UInt64_Id;
        SOPC_UInt64_Initialize(&v->Value.Uint64);
        result_code = FileTransfer_GetPos_TmpFile(handle, objectId, &(v->Value.Uint64));
        if (SOPC_GoodGenericStatus != result_code)
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
    *nbOutputArgs = 1;
    *outputArgs = v;
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
        /* avoid hard indentation level */
        return result_code;
    }
    if ((SOPC_UInt32_Id != inputArgs[0].BuiltInTypeId) || (SOPC_UInt64_Id != inputArgs[1].BuiltInTypeId))
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer:Method_SetPos: bad BuiltInTypeId arguments");
        /* avoid hard indentation level */
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
    filetype->handle = generate_random_handle();
    filetype->path = SOPC_String_Create();
    filetype->tmp_path = SOPC_String_Create();
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
}

static void FileTransfer_FileType_Clear(SOPC_FileType* filetype)
{
    SOPC_NodeId_Clear(filetype->node_id);
    if (NULL != filetype)
    {
        filetype->handle = 0;
        SOPC_String_Clear(filetype->path);
        filetype->path = NULL;
        SOPC_String_Clear(filetype->tmp_path);
        filetype->tmp_path = NULL;
        for (int i = 0; i < NB_FILE_TYPE_METHOD; i++)
        {
            SOPC_NodeId_Clear(filetype->methodIds[i]);
            filetype->methodIds[i] = NULL;
        }
        for (int i = 0; i < NB_VARIABLE; i++)
        {
            SOPC_NodeId_Clear(filetype->variableIds[i]);
            filetype->variableIds[i] = NULL;
        }
        filetype->mode = FileTransfer_UnknownMode;
        filetype->is_open = false;
        filetype->fp = NULL;
        filetype->open_count = 0;
        filetype->size_in_byte = 0;
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
    if (NULL != g_objectId_to_file || NULL != g_str_objectId_to_file || NULL != g_method_call_manager)
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
        g_str_objectId_to_file = SOPC_Dict_Create(NULL, cstring_hash, cstring_equal, cstring_free, NULL);
        if (NULL == g_str_objectId_to_file)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:Init: unable to create dictionary <g_str_objectId_to_file>");
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
    SOPC_Dict_Delete(g_str_objectId_to_file);
    g_str_objectId_to_file = NULL;
    SOPC_Dict_Delete(g_handle_to_file);
    g_handle_to_file = NULL;
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();
}

SOPC_ReturnStatus SOPC_FileTransfer_Add_File(const SOPC_FileType_Config config)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_FileType* file;
    bool status_nok = false;

    if (NULL != config.fileType_nodeId && NULL != config.file_path && NULL != config.met_openId &&
        NULL != config.met_closeId && NULL != config.met_readId && NULL != config.met_writeId &&
        NULL != config.met_getposId && NULL != config.met_setposId && NULL != config.var_openCountId &&
        NULL != config.var_sizeId && NULL != config.var_userWritableId && NULL != config.var_writableId)
    {
        file = FileTransfer_FileType_Create();
        if (NULL == file)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create FileType structure");
            status_nok = true;
        }
        file->mode = FileTransfer_UnknownMode;
        file->node_id = SOPC_NodeId_FromCString(config.fileType_nodeId, (int32_t) strlen(config.fileType_nodeId));
        if (NULL == file->node_id)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to create NodeId from a C string for the FileType");
            status_nok = true;
        }
        status = SOPC_String_CopyFromCString(file->path, config.file_path);
        if (SOPC_STATUS_OK == status)
        {
            file->methodIds[OPEN_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_openId, (int32_t) strlen(config.met_openId));
            if (NULL != file->methodIds[OPEN_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[OPEN_METHOD_IDX],
                                                          &FileTransfer_Method_Open, "Open", NULL);
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:AddFile: unable to create NodeId from a C string for Open method");
                status_nok = true;
            }
            file->methodIds[CLOSE_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_closeId, (int32_t) strlen(config.met_closeId));
            if (NULL != file->methodIds[CLOSE_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[CLOSE_METHOD_IDX],
                                                          &FileTransfer_Method_Close, "Close", NULL);
            }
            else
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:AddFile: unable to create NodeId from a C string for Close method");
                status_nok = true;
            }
            file->methodIds[READ_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_readId, (int32_t) strlen(config.met_readId));
            if (NULL != file->methodIds[READ_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[READ_METHOD_IDX],
                                                          &FileTransfer_Method_Read, "Read", NULL);
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "FileTransfer:AddFile: unable to create NodeId from a C string for Read method");
                status_nok = true;
            }
            file->methodIds[WRITE_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_writeId, (int32_t) strlen(config.met_writeId));
            if (NULL != file->methodIds[WRITE_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[WRITE_METHOD_IDX],
                                                          &FileTransfer_Method_Write, "Write", NULL);
            }
            else
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:AddFile: unable to create NodeId from a C string for Write method");
                status_nok = true;
            }
            file->methodIds[GETPOS_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_getposId, (int32_t) strlen(config.met_getposId));
            if (NULL != file->methodIds[GETPOS_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[GETPOS_METHOD_IDX],
                                                          &FileTransfer_Method_GetPos, "GetPosition", NULL);
            }
            else
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:AddFile: unable to create NodeId from a C string for GetPosition method");
                status_nok = true;
            }

            file->methodIds[SETPOS_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_setposId, (int32_t) strlen(config.met_setposId));
            if (NULL != file->methodIds[SETPOS_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[SETPOS_METHOD_IDX],
                                                          &FileTransfer_Method_SetPos, "SetPosition", NULL);
            }
            else
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:AddFile: unable to create NodeId from a C string for SetPosition method");
                status_nok = true;
            }

            file->variableIds[SIZE_VAR_IDX] =
                SOPC_NodeId_FromCString(config.var_sizeId, (int32_t) strlen(config.var_sizeId));
            if (NULL == file->variableIds[SIZE_VAR_IDX])
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:AddFile: unable to create NodeId from a C string for Size variable");
                status_nok = true;
            }
            file->variableIds[OPEN_COUNT_VAR_IDX] =
                SOPC_NodeId_FromCString(config.var_openCountId, (int32_t) strlen(config.var_openCountId));
            if (NULL == file->variableIds[OPEN_COUNT_VAR_IDX])
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:AddFile: unable to create NodeId from a C string for OpenCount variable");
                status_nok = true;
            }
            file->variableIds[WRITABLE_VAR_IDX] =
                SOPC_NodeId_FromCString(config.var_writableId, (int32_t) strlen(config.var_writableId));
            if (NULL == file->variableIds[WRITABLE_VAR_IDX])
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:AddFile: unable to create NodeId from a C string for Writable variable");
                status_nok = true;
            }
            file->variableIds[USER_WRITABLE_VAR_IDX] =
                SOPC_NodeId_FromCString(config.var_userWritableId, (int32_t) strlen(config.var_userWritableId));
            if (NULL == file->variableIds[USER_WRITABLE_VAR_IDX])
            {
                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "FileTransfer:AddFile: unable to create NodeId from a C string for UserWritable variable");
                status_nok = true;
            }

            /* g_str_objectId_to_file only for debuging with string key */
            char* str_key = SOPC_Malloc(strlen(config.fileType_nodeId));
            memcpy(str_key, config.fileType_nodeId, (size_t) strlen(config.fileType_nodeId));
            bool res;
            res = SOPC_Dict_Insert(g_objectId_to_file, file->node_id, file);
            SOPC_ASSERT(true == res);
            res = SOPC_Dict_Insert(g_str_objectId_to_file, str_key, file);
            SOPC_ASSERT(true == res);
            res = SOPC_Dict_Insert(g_handle_to_file, &file->handle, file);
            SOPC_ASSERT(true == res);
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "FileTransfer:AddFile: unable to set file path from a C string");
        }
    }
    if (status_nok)
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:AddFile: The fields of the config argument must be initialized");
        status = SOPC_STATUS_NOK;
    }
    return status;
}

static SOPC_StatusCode FileTransfer_FileType_Create_TmpFile(SOPC_FileType* file)
{
    SOPC_StatusCode status = OpcUa_BadOutOfMemory;
    if (NULL != file)
    {
        if ((NULL != file->node_id) && (NULL != file->path) && (NULL != file->tmp_path))
        {
            char tmp_file_path[STR_BUFF_SIZE];

            memset(tmp_file_path, 0, sizeof(tmp_file_path));

            sprintf(tmp_file_path, "%s-XXXXXX", SOPC_String_GetCString(file->path));

            int filedes = mkstemp(tmp_file_path);
            if (1 > filedes)
            {
                char* str = SOPC_String_GetCString(file->path);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer: creation of tmp file %s failed",
                                       str);
                SOPC_ASSERT(1 <= filedes && "creation of tmp file failed");
            }
            int res = close(filedes);
            if (0 != res)
            {
                char* str = SOPC_String_GetCString(file->path);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer: closing of tmp file %s failed",
                                       str);
                SOPC_ASSERT(0 == res && "closing of tmp file failed");
            }

            if (SOPC_STATUS_OK == SOPC_String_CopyFromCString(file->tmp_path, (const char*) tmp_file_path))
            {
                status = SOPC_GoodGenericStatus;
            }
        }
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Open_TmpFile(SOPC_FileType* file)
{
    SOPC_StatusCode status;
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
                    file->fp = fopen(SOPC_String_GetCString(file->tmp_path), Cmode);
                    if (NULL == file->fp)
                    {
                        char* str = SOPC_String_GetCString(file->tmp_path);
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer: file %s can't be open",
                                               str);
                    }
                    SOPC_ASSERT(NULL != file->fp && "tmp file can't be open");
                }
            }
            else
            {
                status = OpcUa_BadOutOfMemory;
            }
        }
        else
        {
            status = OpcUa_BadOutOfMemory;
        }
    }
    else
    {
        status = OpcUa_BadInvalidArgument;
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Close_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId)
{
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (handle == file->handle)
        {
            status = SOPC_GoodGenericStatus;
            if (file->is_open)
            {
                status = OpcUa_BadOutOfMemory;
                if ((NULL != file->fp) && (NULL != file->tmp_path))
                {
                    int res = fclose(file->fp);
                    if (0 != res)
                    {
                        char* str = SOPC_String_GetCString(file->tmp_path);
                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer: file %s can't be closed",
                                               str);
                        SOPC_ASSERT(0 == res && "file can't be closed");
                    }
                    file->fp = NULL;
                    file->is_open = false;
                    SOPC_Dict_Remove(g_handle_to_file, &file->handle);
                    /* Free and creat a new tmp_path */
                    SOPC_String_Clear(file->tmp_path);
                    file->tmp_path = NULL;
                    file->tmp_path = SOPC_String_Create();
                    status = SOPC_GoodGenericStatus;
                }
            }
        }
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Delete_TmpFile(SOPC_FileType* file)
{
    SOPC_StatusCode status = OpcUa_BadOutOfMemory;
    if (NULL != file)
    {
        if (NULL != file->fp)
        {
            int res = fclose(file->fp);
            if (0 != res)
            {
                char* str = SOPC_String_GetCString(file->tmp_path);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer: tmp file %s can't be closed", str);
                SOPC_ASSERT(0 == res && "tmp file can't be closed");
            }
            res = remove(SOPC_String_GetCString(file->tmp_path));
            if (0 != res)
            {
                char* str = SOPC_String_GetCString(file->tmp_path);
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "FileTransfer: tmp file %s can't be removed", str);
                SOPC_ASSERT(0 == res && "tmp file can't be remove");
            }
            file->fp = NULL;
            file->is_open = false;
            /* Free and creat a new tmp_path */
            SOPC_String_Clear(file->tmp_path);
            file->tmp_path = NULL;
            file->tmp_path = SOPC_String_Create();
            status = SOPC_GoodGenericStatus;
        }
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Read_TmpFile(SOPC_FileHandle handle,
                                                 int32_t length,
                                                 SOPC_ByteString* msg,
                                                 const SOPC_NodeId* objectId)
{
    char buffer[length];
    memset(buffer, 0, sizeof(buffer));
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    size_t read_count;
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found && (length > 0))
    {
        if (handle == file->handle)
        {
            /* check if File was not opened for read access */
            if ((file->is_open == true) && ((file->mode == WRITE_MASK) || (file->mode == APPEND_MASK) ||
                                            (file->mode == (APPEND_MASK + WRITE_MASK))))
            {
                /* avoid hard indentation level */
                return OpcUa_BadInvalidState;
            }
            status = OpcUa_BadOutOfMemory;
            if (NULL != msg)
            {
                if (NULL != file->fp)
                {
                    read_count = fread(buffer, 1, (size_t) length, file->fp);
                    int end_of_file = feof(file->fp);
                    if ((read_count < (size_t) length) && (0 == end_of_file))
                    {
                        status = OpcUa_BadUnexpectedError;
                    }
                    else
                    {
                        if (SOPC_STATUS_OK != SOPC_String_CopyFromCString(msg, (const char*) buffer))
                        {
                            status = OpcUa_BadUnexpectedError;
                        }
                        else
                        {
                            status = SOPC_GoodGenericStatus;
                        }
                    }
                }
            }
        }
    }
    return status;
}

static SOPC_StatusCode FileTransfer_Write_TmpFile(SOPC_FileHandle handle,
                                                  SOPC_ByteString* msg,
                                                  const SOPC_NodeId* objectId)
{
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (handle == file->handle)
        {
            status = OpcUa_BadInvalidState;
            /* check if File was not opened for write access */
            if ((file->is_open) && (READ_MASK != file->mode))
            {
                status = OpcUa_BadOutOfMemory;
                if (NULL != msg)
                {
                    if ((NULL != file->fp) && (-1 != msg->Length))
                    {
                        size_t ret;
                        char buffer[msg->Length];
                        memcpy(buffer, msg->Data, (size_t) msg->Length);
                        /* If ret != masg->Length then file might be locked and thus not writable */
                        status = OpcUa_BadNotWritable;
                        ret = fwrite(buffer, 1, (size_t) msg->Length, file->fp);
                        if ((size_t) msg->Length == ret)
                        {
                            status = SOPC_GoodGenericStatus;
                        }
                    }
                }
            }
        }
    }
    return status;
}

static SOPC_StatusCode FileTransfer_GetPos_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId, uint64_t* pos)
{
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (handle == file->handle)
        {
            status = SOPC_GoodGenericStatus;
            *pos = 0;
            if (NULL != file->fp)
            {
                long int ret;
                ret = ftell(file->fp);
                if (-1L == ret)
                {
                    status = OpcUa_BadUnexpectedError;
                }
                else
                {
                    *pos = (uint64_t) ret;
                }
            }
        }
    }
    return status;
}

static SOPC_StatusCode FileTransfer_SetPos_TmpFile(SOPC_FileHandle handle, const SOPC_NodeId* objectId, uint64_t posOff)
{
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    SOPC_FileType* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (handle == file->handle)
        {
            status = SOPC_GoodGenericStatus;
            if (NULL != file->fp)
            {
                int ret;
                ret = fseek(file->fp, (long int) posOff, SEEK_SET);
                if (0 != ret)
                {
                    status = OpcUa_BadUnexpectedError;
                }
            }
        }
    }
    return status;
}

SOPC_ReturnStatus SOPC_FileTransfer_Get_TmpPath(const char* node_id, char* name)
{
    (void) name;
    bool found = false;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    if (NULL == node_id)
    {
        return status;
    }
    status = SOPC_STATUS_NOK;
    SOPC_FileType* file = SOPC_Dict_Get(g_str_objectId_to_file, node_id, &found);
    if (!found)
    {
        printf("<FileTransfer_Get_TmpPath> Unable to retrieve the file object '%s'\n", node_id);
        return status;
    }
    if (false == file->is_open)
    {
        printf("<FileTransfer_Get_TmpPath> File object '%s' is not open yet\n", node_id);
        return status;
    }

    if (NULL == file->fp)
    {
        printf("<FileTransfer_Get_TmpPath> File object '%s' is not initialize yet\n", node_id);
        return status;
    }

    if (NULL != file->tmp_path)
    {
        if (0 > file->tmp_path->Length)
        {
            printf("<FileTransfer_Get_TmpPath> File object '%s' is not created yet\n", node_id);
            return status;
        }
    }
    else
    {
        printf("<FileTransfer_Get_TmpPath> Unexpected error\n");
        return status;
    }

    status = SOPC_STATUS_OK;
    memcpy(name, SOPC_String_GetCString(file->tmp_path), (size_t) file->tmp_path->Length + 1);
    name = SOPC_String_GetCString(file->tmp_path);
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
    status = SOPC_ServerHelper_StartServer(ServerStoppedCallback);
    if (SOPC_STATUS_OK == status)
    {
        /* Initialize each variables for each each FileType added into the API */
        SOPC_Dict_ForEach(g_objectId_to_file, &local_write_init, &status);
    }
    return status;
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
