#include <assert.h>
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

#include "sopc_file_transfer.h"

#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "sopc_builtintypes.h"
#include "sopc_dict.h"
#include "sopc_hash.h"
#include "sopc_mem_alloc.h"

#include "opcua_statuscodes.h"

void ft_assert_fonction(bool exp, const char* file, int line, const char* msg, ...)
{
    char error_msg[STR_BUFF_SIZE];
    va_list ap;
    va_start(ap, msg);
    vsnprintf(error_msg, sizeof(error_msg), msg, ap);
    va_end(ap);

    if (exp == false)
    {
        printf("Failed on %s at line %d\n", file, line);
        printf("Error: %s\n", error_msg);
        abort();
    }
}

static void filetype_free(void* value);
static void cstring_free(void* value);
static bool cstring_equal(const void* a, const void* b);
static uint64_t cstring_hash(const void* cstring);
static bool handle_equal(const void* a, const void* b);
static uint64_t handle_hash(const void* handle);
static FT_FileHandle generate_random_handle(void);
static bool check_openModeArg(FT_OpenMode mode);
static SOPC_StatusCode opcuaMode_to_CMode(FT_OpenMode mode, char* Cmode);
static SOPC_StatusCode local_write_open_count(FT_FileType_t file);
static SOPC_StatusCode local_write_size(FT_FileType_t file);
static SOPC_StatusCode local_write_default_Writable(FT_FileType_t file);
static SOPC_StatusCode local_write_default_UserWritable(FT_FileType_t file);
static void local_write_init(const void* key, const void* value, void* user_data);

static SOPC_StatusCode SOPC_FileTransfer_Method_Open(const SOPC_CallContext* callContextPtr,
                                                     const SOPC_NodeId* objectId,
                                                     uint32_t nbInputArgs,
                                                     const SOPC_Variant* inputArgs,
                                                     uint32_t* nbOutputArgs,
                                                     SOPC_Variant** outputArgs,
                                                     void* param);

static SOPC_StatusCode SOPC_FileTransfer_Method_Close(const SOPC_CallContext* callContextPtr,
                                                      const SOPC_NodeId* objectId,
                                                      uint32_t nbInputArgs,
                                                      const SOPC_Variant* inputArgs,
                                                      uint32_t* nbOutputArgs,
                                                      SOPC_Variant** outputArgs,
                                                      void* param);

static SOPC_StatusCode SOPC_FileTransfer_Method_Read(const SOPC_CallContext* callContextPtr,
                                                     const SOPC_NodeId* objectId,
                                                     uint32_t nbInputArgs,
                                                     const SOPC_Variant* inputArgs,
                                                     uint32_t* nbOutputArgs,
                                                     SOPC_Variant** outputArgs,
                                                     void* param);

static SOPC_StatusCode SOPC_FileTransfer_Method_Write(const SOPC_CallContext* callContextPtr,
                                                      const SOPC_NodeId* objectId,
                                                      uint32_t nbInputArgs,
                                                      const SOPC_Variant* inputArgs,
                                                      uint32_t* nbOutputArgs,
                                                      SOPC_Variant** outputArgs,
                                                      void* param);

static SOPC_StatusCode SOPC_FileTransfer_Method_GetPos(const SOPC_CallContext* callContextPtr,
                                                       const SOPC_NodeId* objectId,
                                                       uint32_t nbInputArgs,
                                                       const SOPC_Variant* inputArgs,
                                                       uint32_t* nbOutputArgs,
                                                       SOPC_Variant** outputArgs,
                                                       void* param);

static SOPC_StatusCode SOPC_FileTransfer_Method_SetPos(const SOPC_CallContext* callContextPtr,
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

static bool check_openModeArg(FT_OpenMode mode)
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
        if (mode != (ERASE_EXISTING_MASK + WRITE_MASK))
        {
            ok2 = false;
        }
    }
    return ok1 && ok2;
}

static void filetype_free(void* value)
{
    if (value != NULL)
    {
        SOPC_FileTransfer_FileType_Delete((FT_FileType_t**) &value);
    }
}

static void cstring_free(void* value)
{
    if (value != NULL)
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
    return (*(const FT_FileHandle*) a == *(const FT_FileHandle*) b);
}

static uint64_t handle_hash(const void* handle)
{
    uint64_t hash = SOPC_DJBHash((const uint8_t*) handle, (size_t) strlen(handle));
    return hash;
}

static FT_FileHandle generate_random_handle(void)
{
    /* Initialisation of the seed to generate random handle */
    srand((unsigned int) time(NULL));
    FT_FileHandle gen = (uint32_t) rand();
    return gen;
}

static SOPC_StatusCode opcuaMode_to_CMode(FT_OpenMode mode, char* Cmode)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus;
    if (Cmode != NULL)
    {
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
    }
    else
    {
        status = OpcUa_BadInvalidArgument;
    }
    return status;
}

static SOPC_StatusCode SOPC_FileTransfer_Method_Open(const SOPC_CallContext* callContextPtr,
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
        /* avoid hard indentation level */
        return result_code;
    }

    SOPC_Byte mode = inputArgs->Value.Byte;
    bool mode_ok = check_openModeArg(mode);
    if (!mode_ok)
    {
        /* avoid hard indentation level */
        return result_code;
    }

    bool found = false;
    FT_FileType_t* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (file->is_open)
        {
            /* Clients can open the same file several times for read */
            /* A request to open for writing shall return Bad_NotWritable when the file is already opened */
            if ((file->mode == READ_MASK) && (mode != READ_MASK))
            {
                /* avoid hard indentation level */
                return OpcUa_BadNotWritable;
            }
            /* A request to open for reading shall return Bad_NotReadable when the file is already opened for writing.
             */
            if ((file->mode == WRITE_MASK) || (file->mode == APPEND_MASK) || (file->mode == (APPEND_MASK + WRITE_MASK)))
            {
                if ((mode != WRITE_MASK) && (mode != APPEND_MASK) && (mode != (APPEND_MASK + WRITE_MASK)))
                {
                    /* avoid hard indentation level */
                    return OpcUa_BadNotReadable;
                }
            }
            result_code = SOPC_FileTransfer_Delete_TmpFile(file);
            if (SOPC_GoodGenericStatus != result_code)
            {
                /* avoid hard indentation level */
                return result_code;
            }
        }
        assert(SOPC_Dict_Insert(g_handle_to_file, &file->handle, file) == true);
        file->mode = mode;
        result_code = SOPC_FileTransfer_Create_TmpFile(file);
        if (SOPC_GoodGenericStatus == result_code)
        {
            result_code = SOPC_FileTransfer_Open_TmpFile(file);
            if (SOPC_GoodGenericStatus == result_code)
            {
                file->handle = file->handle + 1;
                file->is_open = true;
                file->open_count++;
                /* Start local service on variables */
                result_code = local_write_open_count(*file);
                struct stat sb;
                if (fstat(fileno(file->fp), &sb) != -1)
                {
                    file->size_in_byte = (uint64_t) sb.st_size;
                    result_code = local_write_size(*file);
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
                        result_code = OpcUa_BadOutOfMemory;
                    }
                }
            }
        }
    }
    else
    {
        result_code = OpcUa_BadNotFound;
    }
    return result_code;
}

static SOPC_StatusCode SOPC_FileTransfer_Method_Close(const SOPC_CallContext* callContextPtr,
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
        /* avoid hard indentation level */
        return result_code;
    }

    if ((inputArgs->BuiltInTypeId != SOPC_UInt32_Id))
    {
        /* avoid hard indentation level */
        return result_code;
    }

    FT_FileHandle handle = inputArgs->Value.Uint32;
    result_code = SOPC_FileTransfer_Close_TmpFile(handle, objectId);
    return result_code;
}

static SOPC_StatusCode SOPC_FileTransfer_Method_Read(const SOPC_CallContext* callContextPtr,
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
        /* avoid hard indentation level */
        return result_code;
    }

    if ((inputArgs[0].BuiltInTypeId != SOPC_UInt32_Id) || (inputArgs[1].BuiltInTypeId != SOPC_Int32_Id))
    {
        /* avoid hard indentation level */
        return result_code;
    }

    FT_FileHandle handle = inputArgs[0].Value.Uint32;
    int32_t length = inputArgs[1].Value.Int32;

    SOPC_Variant* v = SOPC_Variant_Create();
    if (NULL != v)
    {
        v->ArrayType = SOPC_VariantArrayType_SingleValue;
        v->BuiltInTypeId = SOPC_ByteString_Id;
        SOPC_ByteString_Initialize(&v->Value.Bstring);
        result_code = SOPC_FileTransfer_Read_TmpFile(handle, length, &(v->Value.Bstring), objectId);
    }
    else
    {
        result_code = OpcUa_BadOutOfMemory;
    }
    *nbOutputArgs = 1;
    *outputArgs = v;

    bool found = false;
    FT_FileType_t* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        struct stat sb;
        if (fstat(fileno(file->fp), &sb) != -1)
        {
            file->size_in_byte = (uint64_t) sb.st_size;
            result_code = local_write_size(*file);
        }
    }
    return result_code;
}

static SOPC_StatusCode SOPC_FileTransfer_Method_Write(const SOPC_CallContext* callContextPtr,
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
        /* avoid hard indentation level */
        return result_code;
    }
    if ((inputArgs[0].BuiltInTypeId != SOPC_UInt32_Id) || (inputArgs[1].BuiltInTypeId != SOPC_ByteString_Id))
    {
        /* avoid hard indentation level */
        return result_code;
    }
    FT_FileHandle handle = inputArgs[0].Value.Uint32;
    SOPC_ByteString data = inputArgs[1].Value.Bstring;
    result_code = SOPC_FileTransfer_Write_TmpFile(handle, &data, objectId);

    return result_code;
}

static SOPC_StatusCode SOPC_FileTransfer_Method_GetPos(const SOPC_CallContext* callContextPtr,
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
        /* avoid hard indentation level */
        return result_code;
    }
    if ((inputArgs->BuiltInTypeId != SOPC_UInt32_Id))
    {
        /* avoid hard indentation level */
        return result_code;
    }

    FT_FileHandle handle = inputArgs->Value.Uint32;
    SOPC_Variant* v = SOPC_Variant_Create();
    if (NULL != v)
    {
        v->ArrayType = SOPC_VariantArrayType_SingleValue;
        v->BuiltInTypeId = SOPC_UInt64_Id;
        SOPC_UInt64_Initialize(&v->Value.Uint64);
        result_code = SOPC_FileTransfer_GetPos_TmpFile(handle, objectId, &(v->Value.Uint64));
    }
    else
    {
        result_code = OpcUa_BadOutOfMemory;
    }
    *nbOutputArgs = 1;
    *outputArgs = v;
    return result_code;
}

static SOPC_StatusCode SOPC_FileTransfer_Method_SetPos(const SOPC_CallContext* callContextPtr,
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
        /* avoid hard indentation level */
        return result_code;
    }
    if ((inputArgs[0].BuiltInTypeId != SOPC_UInt32_Id) || (inputArgs[1].BuiltInTypeId != SOPC_UInt64_Id))
    {
        /* avoid hard indentation level */
        return result_code;
    }
    FT_FileHandle handle = inputArgs[0].Value.Uint32;
    uint64_t pos = inputArgs[1].Value.Uint64;
    result_code = SOPC_FileTransfer_SetPos_TmpFile(handle, objectId, pos);
    return result_code;
}

FT_FileType_t* SOPC_FilteTransfer_FileType_Create(void)
{
    FT_FileType_t* filetype = NULL;
    filetype = SOPC_Malloc(sizeof(FT_FileType_t));
    if (NULL != filetype)
    {
        SOPC_FilteTransfer_FileType_Initialize(filetype);
    }
    return filetype;
}

void SOPC_FilteTransfer_FileType_Initialize(FT_FileType_t* filetype)
{
    FT_ASSERT(filetype != NULL, "FT_FileType_t pointer needs to be initialize", NULL);
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

void SOPC_FileTransfer_FileType_Clear(FT_FileType_t* filetype)
{
    SOPC_NodeId_Clear(filetype->node_id);
    if (filetype != NULL)
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

void SOPC_FileTransfer_FileType_Delete(FT_FileType_t** filetype)
{
    if (filetype != NULL && NULL != *filetype)
    {
        SOPC_FileTransfer_FileType_Clear(*filetype);
        SOPC_Free(*filetype);
        *filetype = NULL;
    }
}

SOPC_ReturnStatus SOPC_FileTransfer_Initialize(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (g_objectId_to_file != NULL || g_str_objectId_to_file != NULL || g_method_call_manager != NULL)
    {
        // Protection of double init
        status = OpcUa_BadInvalidState;
    }
    else
    {
        g_objectId_to_file = SOPC_NodeId_Dict_Create(true, filetype_free);
        if (g_objectId_to_file == NULL)
        {
            status = OpcUa_BadOutOfMemory;
        }
        g_str_objectId_to_file = SOPC_Dict_Create(NULL, cstring_hash, cstring_equal, cstring_free, NULL);
        if (g_str_objectId_to_file == NULL)
        {
            status = OpcUa_BadOutOfMemory;
        }
        g_method_call_manager = SOPC_MethodCallManager_Create();
        if (g_method_call_manager == NULL)
        {
            status = OpcUa_BadOutOfMemory;
        }
        g_handle_to_file = SOPC_Dict_Create(NULL, handle_hash, handle_equal, NULL, NULL);
        if (g_handle_to_file == NULL)
        {
            status = OpcUa_BadOutOfMemory;
        }
        else
        {
            status = SOPC_HelperConfigServer_SetMethodCallManager(g_method_call_manager);
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
    FT_FileType_t* file;
    bool status_nok = false;

    if (NULL != config.fileType_nodeId && NULL != config.file_path && NULL != config.met_openId &&
        NULL != config.met_closeId && NULL != config.met_readId && NULL != config.met_writeId &&
        NULL != config.met_getposId && NULL != config.met_setposId && NULL != config.var_openCountId &&
        NULL != config.var_sizeId && NULL != config.var_userWritableId && NULL != config.var_writableId)
    {
        file = SOPC_FilteTransfer_FileType_Create();
        file->node_id = SOPC_NodeId_FromCString(config.fileType_nodeId, (int32_t) strlen(config.fileType_nodeId));
        file->mode = FileTransfer_UnknownMode;
        status = SOPC_String_CopyFromCString(file->path, config.file_path);
        if (SOPC_STATUS_OK == status)
        {
            file->methodIds[OPEN_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_openId, (int32_t) strlen(config.met_openId));
            if (NULL != file->methodIds[OPEN_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[OPEN_METHOD_IDX],
                                                          &SOPC_FileTransfer_Method_Open, "Open", NULL);
            }
            else
            {
                status_nok = true;
            }
            file->methodIds[CLOSE_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_closeId, (int32_t) strlen(config.met_closeId));
            if (NULL != file->methodIds[CLOSE_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[CLOSE_METHOD_IDX],
                                                          &SOPC_FileTransfer_Method_Close, "Close", NULL);
            }
            else
            {
                status_nok = true;
            }
            file->methodIds[READ_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_readId, (int32_t) strlen(config.met_readId));
            if (NULL != file->methodIds[READ_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[READ_METHOD_IDX],
                                                          &SOPC_FileTransfer_Method_Read, "Read", NULL);
            }
            else
            {
                status_nok = true;
            }
            file->methodIds[WRITE_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_writeId, (int32_t) strlen(config.met_writeId));
            if (NULL != file->methodIds[WRITE_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[WRITE_METHOD_IDX],
                                                          &SOPC_FileTransfer_Method_Write, "Write", NULL);
            }
            else
            {
                status_nok = true;
            }
            file->methodIds[GETPOS_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_getposId, (int32_t) strlen(config.met_getposId));
            if (NULL != file->methodIds[GETPOS_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[GETPOS_METHOD_IDX],
                                                          &SOPC_FileTransfer_Method_GetPos, "GetPosition", NULL);
            }
            else
            {
                status_nok = true;
            }

            file->methodIds[SETPOS_METHOD_IDX] =
                SOPC_NodeId_FromCString(config.met_setposId, (int32_t) strlen(config.met_setposId));
            if (NULL != file->methodIds[SETPOS_METHOD_IDX])
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[SETPOS_METHOD_IDX],
                                                          &SOPC_FileTransfer_Method_SetPos, "SetPosition", NULL);
            }
            else
            {
                status_nok = true;
            }

            file->variableIds[SIZE_VAR_IDX] =
                SOPC_NodeId_FromCString(config.var_sizeId, (int32_t) strlen(config.var_sizeId));
            if (NULL == file->variableIds[SIZE_VAR_IDX])
            {
                status_nok = true;
            }
            file->variableIds[OPEN_COUNT_VAR_IDX] =
                SOPC_NodeId_FromCString(config.var_openCountId, (int32_t) strlen(config.var_openCountId));
            if (NULL == file->variableIds[OPEN_COUNT_VAR_IDX])
            {
                status_nok = true;
            }
            file->variableIds[WRITABLE_VAR_IDX] =
                SOPC_NodeId_FromCString(config.var_writableId, (int32_t) strlen(config.var_writableId));
            if (NULL == file->variableIds[WRITABLE_VAR_IDX])
            {
                status_nok = true;
            }
            file->variableIds[USER_WRITABLE_VAR_IDX] =
                SOPC_NodeId_FromCString(config.var_userWritableId, (int32_t) strlen(config.var_userWritableId));
            if (NULL == file->variableIds[USER_WRITABLE_VAR_IDX])
            {
                status_nok = true;
            }

            /* g_str_objectId_to_file only for debuging with string key */
            char* str_key = SOPC_Malloc(strlen(config.fileType_nodeId));
            memcpy(str_key, config.fileType_nodeId, (size_t) strlen(config.fileType_nodeId));
            assert(SOPC_Dict_Insert(g_objectId_to_file, file->node_id, file) == true);
            assert(SOPC_Dict_Insert(g_str_objectId_to_file, str_key, file) == true);
            assert(SOPC_Dict_Insert(g_handle_to_file, &file->handle, file) == true);
        }
    }
    if (status_nok)
    {
        status = SOPC_STATUS_NOK;
    }
    return status;
}

SOPC_StatusCode SOPC_FileTransfer_Create_TmpFile(FT_FileType_t* file)
{
    SOPC_StatusCode status = OpcUa_BadOutOfMemory;
    if (file != NULL)
    {
        if ((file->node_id != NULL) && (file->path != NULL) && (file->tmp_path != NULL))
        {
            char tmp_file_path[STR_BUFF_SIZE];

            memset(tmp_file_path, 0, sizeof(tmp_file_path));

            sprintf(tmp_file_path, "%s-XXXXXX", SOPC_String_GetCString(file->path));

            int filedes = mkstemp(tmp_file_path);
            FT_ASSERT(filedes >= 1, "creation of tmp file %s failed", file->path);
            FT_ASSERT(close(filedes) == 0, "closing of tmp file %s failed", file->path);

            if (SOPC_STATUS_OK == SOPC_String_CopyFromCString(file->tmp_path, (const char*) tmp_file_path))
            {
                status = SOPC_GoodGenericStatus;
            }
        }
    }
    return status;
}

SOPC_StatusCode SOPC_FileTransfer_Open_TmpFile(FT_FileType_t* file)
{
    SOPC_StatusCode status;
    char Cmode[5] = {0};
    bool mode_is_ok = check_openModeArg(file->mode);
    if (mode_is_ok)
    {
        if (file != NULL)
        {
            if (file->fp == NULL)
            {
                status = opcuaMode_to_CMode(file->mode, Cmode);
                if (SOPC_GoodGenericStatus == status)
                {
                    file->fp = fopen(SOPC_String_GetCString(file->tmp_path), Cmode);
                    FT_ASSERT(file->fp != NULL, "file %s can't be open", SOPC_String_GetCString(file->tmp_path));
                    FT_ASSERT(flock(fileno(file->fp), LOCK_SH) == 0, "file %s can't be lock",
                              SOPC_String_GetCString(file->tmp_path));
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

SOPC_StatusCode SOPC_FileTransfer_Close_TmpFile(FT_FileHandle handle, const SOPC_NodeId* objectId)
{
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    FT_FileType_t* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (file->handle == handle)
        {
            status = SOPC_GoodGenericStatus;
            if (file->is_open)
            {
                status = OpcUa_BadOutOfMemory;
                if ((file->fp != NULL) && (file->tmp_path != NULL))
                {
                    FT_ASSERT(flock(fileno(file->fp), LOCK_UN) == 0, "file %s can't be unlock", file->tmp_path);
                    FT_ASSERT(fclose(file->fp) == 0, "file %s can't be closed", file->tmp_path);
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

SOPC_StatusCode SOPC_FileTransfer_Delete_TmpFile(FT_FileType_t* file)
{
    SOPC_StatusCode status = OpcUa_BadOutOfMemory;
    if (file != NULL)
    {
        if (file->fp != NULL)
        {
            FT_ASSERT(flock(fileno(file->fp), LOCK_UN) == 0, "file %s can't be unlock", file->tmp_path);
            FT_ASSERT(fclose(file->fp) == 0, "file %s can't be closed", file->tmp_path);
            FT_ASSERT(remove(SOPC_String_GetCString(file->tmp_path)) == 0, "file %s can't be remove", file->tmp_path);
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

SOPC_StatusCode SOPC_FileTransfer_Read_TmpFile(FT_FileHandle handle,
                                               int32_t length,
                                               SOPC_ByteString* msg,
                                               const SOPC_NodeId* objectId)
{
    char buffer[length];
    memset(buffer, 0, sizeof(buffer));
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    size_t read_count;
    FT_FileType_t* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found && (length > 0))
    {
        if (file->handle == handle)
        {
            /* check if File was not opened for read access */
            if ((file->is_open == true) && ((file->mode == WRITE_MASK) || (file->mode == APPEND_MASK) ||
                                            (file->mode == (APPEND_MASK + WRITE_MASK))))
            {
                /* avoid hard indentation level */
                return OpcUa_BadInvalidState;
            }
            status = OpcUa_BadOutOfMemory;
            if (msg != NULL)
            {
                if (file->fp != NULL)
                {
                    read_count = fread(buffer, 1, (size_t) length, file->fp);
                    if ((read_count < (size_t) length) && (feof(file->fp) == 0))
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

SOPC_StatusCode SOPC_FileTransfer_Write_TmpFile(FT_FileHandle handle, SOPC_ByteString* msg, const SOPC_NodeId* objectId)
{
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    FT_FileType_t* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (file->handle == handle)
        {
            status = OpcUa_BadInvalidState;
            /* check if File was not opened for write access */
            if ((file->is_open == true) && (file->mode != READ_MASK))
            {
                status = OpcUa_BadOutOfMemory;
                if (msg != NULL)
                {
                    if ((file->fp != NULL) && (msg->Length != -1))
                    {
                        size_t ret;
                        char buffer[msg->Length];
                        memcpy(buffer, msg->Data, (size_t) msg->Length);
                        /* If ret != masg->Length then file might be locked and thus not writable */
                        status = OpcUa_BadNotWritable;
                        ret = fwrite(buffer, 1, (size_t) msg->Length, file->fp);
                        if (ret == (size_t) msg->Length)
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

SOPC_StatusCode SOPC_FileTransfer_GetPos_TmpFile(FT_FileHandle handle, const SOPC_NodeId* objectId, uint64_t* pos)
{
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    FT_FileType_t* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (file->handle == handle)
        {
            status = SOPC_GoodGenericStatus;
            *pos = 0;
            if (file->fp != NULL)
            {
                long int ret;
                ret = ftell(file->fp);
                if (ret == -1L)
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

SOPC_StatusCode SOPC_FileTransfer_SetPos_TmpFile(FT_FileHandle handle, const SOPC_NodeId* objectId, uint64_t posOff)
{
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    FT_FileType_t* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
    if (found)
    {
        if (file->handle == handle)
        {
            status = SOPC_GoodGenericStatus;
            if (file->fp != NULL)
            {
                int ret;
                ret = fseek(file->fp, (long int) posOff, SEEK_SET);
                if (ret != 0)
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
    FT_FileType_t* file = SOPC_Dict_Get(g_str_objectId_to_file, node_id, &found);
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
        if (file->tmp_path->Length < 0)
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
    const FT_FileType_t* file = value;
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

static SOPC_StatusCode local_write_open_count(FT_FileType_t file)
{
    FT_ASSERT(file.variableIds[OPEN_COUNT_VAR_IDX] != NULL,
              "OpenCount variable nodeId shall be added with <SOPC_FileTransfer_Add_File>", NULL);
    SOPC_ReturnStatus status;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (pReq == NULL)
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

static SOPC_StatusCode local_write_size(FT_FileType_t file)
{
    FT_ASSERT(file.variableIds[SIZE_VAR_IDX] != NULL,
              "Size variable nodeId shall be added with <SOPC_FileTransfer_Add_Variable_To_File>", NULL);
    SOPC_ReturnStatus status;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (pReq == NULL)
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

static SOPC_StatusCode local_write_default_Writable(FT_FileType_t file)
{
    FT_ASSERT(file.variableIds[WRITABLE_VAR_IDX] != NULL,
              "Writable variable nodeId shall be added with <SOPC_FileTransfer_Add_File>", NULL);
    SOPC_ReturnStatus status;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (pReq == NULL)
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

static SOPC_StatusCode local_write_default_UserWritable(FT_FileType_t file)
{
    FT_ASSERT(file.variableIds[USER_WRITABLE_VAR_IDX] != NULL,
              "UserWritable variable nodeId shall be added with <SOPC_FileTransfer_Add_File>", NULL);
    SOPC_ReturnStatus status;
    OpcUa_WriteRequest* pReq = SOPC_WriteRequest_Create(1);
    if (pReq == NULL)
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
