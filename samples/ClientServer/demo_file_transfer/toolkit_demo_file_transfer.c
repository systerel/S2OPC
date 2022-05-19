
/**
 * A practice work to use S2OPC server library
 */

#include <assert.h>
#include <inttypes.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

#include "opcua_statuscodes.h"
#include "sopc_mem_alloc.h"

/************************************/
/* FIRST API */
/************************************/

#include <unistd.h>
#include <stdarg.h>
#include <sys/file.h>
#include "sopc_builtintypes.h"
#include "sopc_dict.h"
#include "sopc_hash.h"


void ft_assert_fonction(bool exp, const char* file, int line, const char* msg, ...);

#define NB_FILE_TYPE_METHOD 6
#define STR_BUFF_SIZE 100

#define FileTransfer_UnknownMode 0u
#define READ_MASK 0x01u
#define WRITE_MASK 0x02u
#define ERASE_EXISTING_MASK 0x04u
#define APPEND_MASK 0x08u

#define OPEN_METHOD_IDX 0u
#define CLOSE_METHOD_IDX 1u
#define READ_METHOD_IDX 2u
#define WRITE_METHOD_IDX 3u
#define GETPOS_METHOD_IDX 4u
#define SETPOS_METHOD_IDX 5u

#define FT_ASSERT(exp, msg, ...) (ft_assert_fonction(exp, __FILE__, __LINE__, msg, __VA_ARGS__))

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
/************************************/
/* TYPES DEFINITION */
/************************************/

// The FT_FileHandle type
typedef uint32_t FT_FileHandle;
typedef SOPC_Byte FT_OpenMode;

typedef struct _FT_FileHandle_Set_t
{
    uint32_t length;
    FT_FileHandle* elems;
} FT_FileHandle_Set_t;

// Structure to organize a File Object
typedef struct _FT_FileType_t
{
    SOPC_NodeId* node_id;
    FT_FileHandle handle;                        // The handle of the file
    SOPC_String* path;                           // the file path
    SOPC_NodeId* methodIds[NB_FILE_TYPE_METHOD]; // list of method associeted
    FT_OpenMode mode;
    bool is_open;
    SOPC_String* tmp_path;
    FILE* fp;
} FT_FileType_t;

static void filetype_free(void* value);
static void cstring_free(void* value);
static bool cstring_equal(const void* a, const void* b);
static uint64_t cstring_hash(const void* cstring);
static bool handle_equal(const void* a, const void* b);
static uint64_t handle_hash(const void* handle);
static FT_FileHandle_Set_t* filehandles_create(void);
static void filehandles_delete(FT_FileHandle_Set_t** handles);
static uint8_t filehandles_add(FT_FileHandle_Set_t* handles, FT_FileHandle handle);
static FT_FileHandle generate_handle_from_set(FT_FileHandle_Set_t* handles);
static bool check_openModeArg(FT_OpenMode mode);
static SOPC_StatusCode opcuaMode_to_CMode(FT_OpenMode mode, char *Cmode);

void SOPC_FileTransfer_FileHandle_Clear(FT_FileHandle* filehandle);
void SOPC_FileTransfer_OpenMode_Clear(FT_OpenMode* mode);

FT_FileType_t* SOPC_FilteTransfer_FileType_Create(void);
void SOPC_FilteTransfer_FileType_Initialize(FT_FileType_t* filetype);
void SOPC_FileTransfer_FileType_Clear(FT_FileType_t* filetype);
void SOPC_FileTransfer_FileType_Delete(FT_FileType_t** filetype);
SOPC_StatusCode SOPC_FileTransfer_Create_TmpFile(FT_FileType_t *file);
SOPC_StatusCode SOPC_FileTransfer_Open_TmpFile(FT_FileType_t *file);
SOPC_StatusCode SOPC_FildeTransfer_Delete_TmpFile(FT_FileType_t *file);
SOPC_StatusCode SOPC_FileTransfer_Close_TmpFile(FT_FileHandle* handle);
SOPC_StatusCode SOPC_FileTransfer_Write_TmpFile(FT_FileHandle* handle, SOPC_ByteString* msg);
SOPC_StatusCode SOPC_FileTransfer_Read_TmpFile(FT_FileHandle* handle, SOPC_ByteString* msg);

SOPC_ReturnStatus SOPC_FileTransfer_Initialize(void);
void SOPC_FileTransfer_Clear(void);
SOPC_ReturnStatus SOPC_FileTransfer_Add_File(const char* nodeId,
                                             const char* path,
                                             const char* openId,
                                             const char* closeId,
                                             const char* readId,
                                             const char* writeId,
                                             const char* getposId,
                                             const char* setposId);

static SOPC_StatusCode SOPC_FileTransfer_Method_Open(const SOPC_CallContext* callContextPtr, // to be set into static when used
                                                     const SOPC_NodeId* objectId,
                                                     uint32_t nbInputArgs,
                                                     const SOPC_Variant* inputArgs,
                                                     uint32_t* nbOutputArgs,
                                                     SOPC_Variant** outputArgs,
                                                     void* param);

/************************************/
/* STATIC VARIABLE */
/************************************/
SOPC_Dict* g_objectId_to_file = NULL;
SOPC_Dict* g_str_objectId_to_file = NULL;
SOPC_Dict* g_handle_to_file = NULL;
FT_FileHandle_Set_t* g_handles = NULL;
SOPC_MethodCallManager* g_method_call_manager = NULL;

const char* UserMethodIds[6] = {"Method Open",  "Method Close",  "Method Read",
                                "Method Write", "Method SetPos", "Method GetPos"};

static bool check_openModeArg(FT_OpenMode mode)
{
    uint8_t shall_be_reserved = 0u;
    bool ok = true;
    if (shall_be_reserved != (mode >> 4u) || shall_be_reserved == mode)
    {
        ok = false;
    }
    //  EraseExisting bit can only be set if the file is opened for writing
    if (mode & ERASE_EXISTING_MASK)
    {
        if (mode != (ERASE_EXISTING_MASK + WRITE_MASK))
        {
            ok = false;
        }
    }
    return ok;
}

static FT_FileHandle_Set_t* filehandles_create(void)
{
    FT_FileHandle_Set_t* handles = SOPC_Malloc(sizeof(FT_FileHandle_Set_t));
    handles->elems = NULL;
    handles->length = 0;
    return handles;
}

static void filehandles_delete(FT_FileHandle_Set_t** handles)
{
    if (handles != NULL && NULL != *handles)
    {
        FT_FileHandle_Set_t *handles_tmp = *handles;
        SOPC_Free(handles_tmp->elems);
        SOPC_Free(*handles);
    }
    *handles = NULL; 
}

static uint8_t filehandles_add(FT_FileHandle_Set_t* handles, FT_FileHandle handle)
{
    FT_ASSERT(handles != NULL, "handles global set needs to be initialize", NULL);
    bool is_present = false;
    if (handles->elems == NULL)
    {
        handles->elems = SOPC_Malloc(sizeof(FT_FileHandle));
        handles->elems[0] = handle;
        handles->length++;
    }
    else
    {
        for (uint32_t i = 0; i < handles->length; i++)
        {
            if (handles->elems[i] == handle)
            {
                is_present = true;
            }
        }
        if (is_present == false)
        {
            FT_FileHandle* new_elems = SOPC_Malloc((handles->length + 1) * sizeof(FT_FileHandle));
            memcpy(new_elems, handles->elems, (size_t) handles->length);
            handles->length++;
            new_elems[handles->length] = handle;
            SOPC_Free(handles->elems);
            handles->elems = new_elems;
        }
    }
    return (is_present == false);
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
    return (*(const FT_FileHandle*)a == *(const FT_FileHandle*)b);
}

static uint64_t handle_hash(const void* handle)
{
    uint64_t hash = SOPC_DJBHash((const uint8_t*) handle, (size_t) strlen(handle));
    return hash;
}


static FT_FileHandle generate_handle_from_set(FT_FileHandle_Set_t* handles)
{
    FT_ASSERT(handles != NULL, "handles global set needs to be initialize", NULL);
    FT_FileHandle gen = (uint32_t) rand();
    bool handle_added = filehandles_add(handles, gen);
    while (handle_added == false)
    {
        gen = (uint32_t) rand();
        handle_added = filehandles_add(handles, gen);
    }
    return gen;
}

static SOPC_StatusCode opcuaMode_to_CMode(FT_OpenMode mode, char *Cmode)
{
    SOPC_StatusCode status = SOPC_GoodGenericStatus; 
    if (Cmode != NULL)
    {
        switch(mode)
        {
            case 1:
                snprintf(Cmode, 2, "r"); // reading
                break;
            case 2:
                snprintf(Cmode, 2,  "w"); // writing
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

void SOPC_FileTransfer_FileHandle_Clear(FT_FileHandle* filehandle)
{
    if (filehandle != NULL)
    {
        *filehandle = 0;
    }
}

void SOPC_FileTransfer_OpenMode_Clear(FT_OpenMode* mode)
{
    if (mode != NULL)
    {
        *mode = 0;
    }
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
    filetype->handle = 0;
    filetype->path = SOPC_String_Create();
    filetype->tmp_path = SOPC_String_Create();
    for (int i = 0; i < NB_FILE_TYPE_METHOD; i++)
    {
        filetype->methodIds[i] = NULL;
    }
    filetype->mode = FileTransfer_UnknownMode;
    filetype->is_open = false;
    filetype->fp = NULL;
}

void SOPC_FileTransfer_FileType_Clear(FT_FileType_t* filetype)
{
    SOPC_NodeId_Clear(filetype->node_id);
    if (filetype != NULL)
    {
        SOPC_FileTransfer_FileHandle_Clear(&filetype->handle);
        SOPC_String_Clear(filetype->path);
        SOPC_String_Clear(filetype->tmp_path);
        for (int i = 0; i < NB_FILE_TYPE_METHOD; i++)
        {
            SOPC_NodeId_Clear(filetype->methodIds[i]);
        }
        SOPC_FileTransfer_OpenMode_Clear(&filetype->mode);
        filetype->is_open = false;
        filetype->fp = NULL;
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
    if (g_objectId_to_file != NULL || g_str_objectId_to_file != NULL || g_handles != NULL || g_method_call_manager != NULL)
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
        g_handles = filehandles_create();
        if (g_handles == NULL)
        {
            status = OpcUa_BadOutOfMemory;
        }
        g_method_call_manager = SOPC_MethodCallManager_Create();
        if(g_method_call_manager == NULL)
        {
            status = OpcUa_BadOutOfMemory;
        }
        g_handle_to_file = SOPC_Dict_Create(NULL, handle_hash, handle_equal, NULL, NULL);
        if(g_handle_to_file == NULL)
        {
            status = OpcUa_BadOutOfMemory; 
        }
        else
        {
            status = SOPC_HelperConfigServer_SetMethodCallManager(g_method_call_manager);
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
    filehandles_delete((FT_FileHandle_Set_t**) &g_handles);
    SOPC_HelperConfigServer_Clear();
    SOPC_CommonHelper_Clear();
}

SOPC_ReturnStatus SOPC_FileTransfer_Add_File(const char* nodeId,
                                             const char* path,
                                             const char* openId,
                                             const char* closeId,
                                             const char* readId,
                                             const char* writeId,
                                             const char* getposId,
                                             const char* setposId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    FT_FileType_t* file;
    bool status_nok = false;

    if (NULL != nodeId && NULL != path && NULL != openId && NULL != closeId && NULL != readId && NULL != writeId &&
        NULL != getposId && NULL != setposId)
    {
        file = SOPC_FilteTransfer_FileType_Create();
        file->node_id = SOPC_NodeId_FromCString(nodeId, (int32_t) strlen(nodeId));
        file->handle = generate_handle_from_set(g_handles);
        file->mode = FileTransfer_UnknownMode;
        status = SOPC_String_CopyFromCString(file->path, path);
        if (SOPC_STATUS_OK == status)
        {
            file->methodIds[OPEN_METHOD_IDX] = SOPC_NodeId_FromCString(openId, (int32_t) strlen(openId));
            if (NULL != file->methodIds)
            {
                status = SOPC_MethodCallManager_AddMethod(g_method_call_manager, file->methodIds[OPEN_METHOD_IDX],
                                                        &SOPC_FileTransfer_Method_Open, "Open", NULL);
            }
            else
            {
                status_nok = true;
            }
            file->methodIds[CLOSE_METHOD_IDX] = SOPC_NodeId_FromCString(closeId, (int32_t) strlen(closeId));
            file->methodIds[READ_METHOD_IDX] = SOPC_NodeId_FromCString(readId, (int32_t) strlen(readId));
            file->methodIds[WRITE_METHOD_IDX] = SOPC_NodeId_FromCString(writeId, (int32_t) strlen(writeId));
            file->methodIds[SETPOS_METHOD_IDX] = SOPC_NodeId_FromCString(getposId, (int32_t) strlen(getposId));
            file->methodIds[GETPOS_METHOD_IDX] = SOPC_NodeId_FromCString(setposId, (int32_t) strlen(setposId));

            char* str_key = SOPC_Malloc(strlen(nodeId));
            memcpy(str_key, nodeId, (size_t) strlen(nodeId));
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
    printf("Registration is OK\n");
    SOPC_StatusCode result_code = OpcUa_BadInvalidArgument;
    SOPC_Byte mode = inputArgs->Value.Byte;
    bool mode_ok = check_openModeArg(mode);

    if (1 == nbInputArgs && NULL != inputArgs && NULL != objectId && mode_ok)
    {
        bool found = false;
        FT_FileType_t* file = SOPC_Dict_Get(g_objectId_to_file, objectId, &found);
        if (found)
        {
            file->mode = mode;
            if (file->is_open)
            {
                result_code = SOPC_FildeTransfer_Delete_TmpFile(file);
                if (SOPC_GoodGenericStatus == result_code)
                {
                    result_code = SOPC_FileTransfer_Create_TmpFile(file);
                    if (SOPC_GoodGenericStatus == result_code)
                    {
                        result_code = SOPC_FileTransfer_Open_TmpFile(file);
                    }
                }
            }
            if (SOPC_GoodGenericStatus != result_code)
            {
                /* avoid hard indentation level */
                return result_code;
            }
            file->is_open = true;
            // TODO: requête à envoyer pour set la property open_count.

            result_code = SOPC_FileTransfer_Create_TmpFile(file);
            if (SOPC_GoodGenericStatus == result_code)
            {
                result_code = SOPC_FileTransfer_Open_TmpFile(file);
                if (SOPC_GoodGenericStatus == result_code)
                {
                    SOPC_Variant* v = SOPC_Variant_Create();
                    if (NULL != v)
                    {
                        v->ArrayType = SOPC_VariantArrayType_SingleValue;
                        v->BuiltInTypeId = SOPC_UInt32_Id;
                        SOPC_UInt32_Initialize(&v->Value.Uint32);
                        v->Value.Uint32 = file->handle;
                        *nbOutputArgs = 1;
                        *outputArgs = v;
                        result_code = SOPC_GoodGenericStatus;
                    }
                    else
                    {
                        result_code = OpcUa_BadOutOfMemory;
                    }
                }
                else
                {
                    result_code = OpcUa_BadUnexpectedError;
                }
            }
            else
            {
                result_code = OpcUa_BadUnexpectedError;
            }
        }
        else
        {
            result_code = OpcUa_BadNotFound;
        }
    }
    return result_code;
}

static SOPC_ReturnStatus Server_LoadServerConfigurationFromPaths(void)
{
    // Server endpoints and PKI configuration
    const char* xml_server_cfg_path =
        "/home/rba/PROJECTS/C838_S2OPC/GIT/S2OPC/RBA_DATA/RBA_S2OPC_Server_Demo_Config.xml";
    // Server address space configuration
    const char* xml_address_space_path = "/home/rba/PROJECTS/C838_S2OPC/GIT/S2OPC/RBA_DATA/RBA_address_space.xml";
    // User credentials and authorizations
    const char* xml_users_cfg_path = "/home/rba/PROJECTS/C838_S2OPC/GIT/S2OPC/RBA_DATA/RBA_S2OPC_Users_Demo_Config.xml";

    return SOPC_HelperConfigServer_ConfigureFromXML(xml_server_cfg_path, xml_address_space_path, xml_users_cfg_path,
                                                    NULL);
}

SOPC_StatusCode SOPC_FileTransfer_Create_TmpFile(FT_FileType_t *file)
{
    SOPC_StatusCode status = OpcUa_BadOutOfMemory;
    if ((file != NULL) && (file->node_id != NULL) && (file->path != NULL) && (file->tmp_path != NULL))
    {
        char tmp_file_path[STR_BUFF_SIZE];

        memset(tmp_file_path, 0, sizeof(tmp_file_path));

        sprintf(tmp_file_path, "%s-XXXXXX", SOPC_String_GetCString(file->path));

        int filedes = mkstemp(tmp_file_path);
        FT_ASSERT(filedes >= 1, "creation of tmp file %s failed", file->path);
        FT_ASSERT(close(filedes) == 0, "closing of tmp file %s failed", file->path);

        if (SOPC_STATUS_OK == SOPC_String_CopyFromCString(file->tmp_path, (const char *)tmp_file_path))
        {
            status = SOPC_GoodGenericStatus;
        }
    }
    return status;
}

SOPC_StatusCode SOPC_FileTransfer_Open_TmpFile(FT_FileType_t *file)
{
    SOPC_StatusCode status;
    char Cmode[5] = {0};
    bool mode_is_ok = check_openModeArg(file->mode);
    if (mode_is_ok)
    {
        if ((file != NULL) && (file->fp == NULL))
        {
            status = opcuaMode_to_CMode(file->mode, Cmode);
            if (SOPC_GoodGenericStatus == status)
            {
                file->fp = fopen(SOPC_String_GetCString(file->tmp_path), Cmode);
                FT_ASSERT(file->fp != NULL, "file %s can't be open", SOPC_String_GetCString(file->tmp_path));
                FT_ASSERT(flock(fileno(file->fp), LOCK_SH) == 0, "file %s can't be lock", SOPC_String_GetCString(file->tmp_path));
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

SOPC_StatusCode SOPC_FildeTransfer_Delete_TmpFile(FT_FileType_t *file)
{
    SOPC_StatusCode status = OpcUa_BadOutOfMemory;
    if (file != NULL && file->fp != NULL)
    {
        FT_ASSERT(flock(fileno(file->fp), LOCK_UN) == 0, "file %s can't be unlock", file->tmp_path);
        FT_ASSERT(fclose(file->fp) == 0, "file %s can't be closed", file->tmp_path);
        FT_ASSERT(remove(SOPC_String_GetCString(file->tmp_path)) == 0, "file %s can't be remove", file->tmp_path);
        file->fp = NULL;
        status = SOPC_GoodGenericStatus;
    }
    return status; 
}

SOPC_StatusCode SOPC_FileTransfer_Close_TmpFile(FT_FileHandle* handle)
{

    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    FT_FileType_t* file = SOPC_Dict_Get(g_handle_to_file, handle, &found);
    if (found)
    {
        status = OpcUa_BadOutOfMemory;
        if ((file != NULL) && (file->fp != NULL) && (file->tmp_path != NULL))
        {
            // TODO: Send notification to the user applciation with FileType information 
            FT_ASSERT(fclose(file->fp) == 0, "file %s can't be closed", file->tmp_path);
        }
    }
    return status;
}

SOPC_StatusCode SOPC_FileTransfer_Write_TmpFile(FT_FileHandle* handle, SOPC_ByteString* msg)
{
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    FT_FileType_t* file = SOPC_Dict_Get(g_handle_to_file, handle, &found);
    if (found)
    {
        //printf("Succes\n")
        status = OpcUa_BadOutOfMemory;
        int ret;
        if (file != NULL && file->fp != NULL)
        {
            ret = fprintf(file->fp, "%s", (const char *)msg->Data);
            if(ret != EOF)
            {
                status = SOPC_GoodGenericStatus;
            }
        }
    }
    return status; 
}


SOPC_StatusCode SOPC_FileTransfer_Read_TmpFile(FT_FileHandle* handle, SOPC_ByteString* msg)
{
    (void)msg; 
    SOPC_StatusCode status = OpcUa_BadInvalidArgument;
    bool found = false;
    FT_FileType_t* file = SOPC_Dict_Get(g_handle_to_file, handle, &found);
    if (found)
    {
        status = OpcUa_BadOutOfMemory;
        if (file != NULL && file->fp != NULL)
        {
        // Do treatment 
        }
    }
    return status; 
}


int main(int argc, char* argv[])
{
    printf("******* API test\n");
    // Note: avoid unused parameter warning from compiler
    (void) argc;
    (void) argv;

    SOPC_Log_Configuration* log_conf = NULL;
    SOPC_ReturnStatus status;
    SOPC_StatusCode status_code;
    status = SOPC_CommonHelper_Initialize(log_conf);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }
    assert(SOPC_STATUS_OK == status);
    if (SOPC_STATUS_OK == status)
    {
        // status = Server_LoadServerConfigurationFromFiles(argv[0]);
        status = Server_LoadServerConfigurationFromPaths();
    }
    assert(SOPC_STATUS_OK == status);

    /* Finalize the server configuration */
    if (SOPC_STATUS_OK == status)
    {
        printf("******* Code begin ...\n");

        status = SOPC_FileTransfer_Initialize();
        FT_ASSERT(SOPC_STATUS_OK == status, "during file transfer intialization", NULL);

        const char* file_id = "ns=1;i=15017";
        const char* path = "/tmp/myfile";
        const char* open_id = "ns=1;i=15048";
        SOPC_FileTransfer_Add_File(file_id, path, open_id, "", "", "", "", "");

        bool found = false;
        FT_FileType_t* file = SOPC_Dict_Get(g_str_objectId_to_file, file_id, &found);
        if (!found)
        {
            printf("Not found\n");
        }
        else
        {
            file->mode = 6;
            status_code = SOPC_FileTransfer_Create_TmpFile(file);
            FT_ASSERT(status_code == SOPC_GoodGenericStatus, "during tmp file creation", NULL);
            status_code = SOPC_FileTransfer_Open_TmpFile(file);
            FT_ASSERT(status_code == SOPC_GoodGenericStatus, "during tmp file opening", NULL);
            SOPC_ByteString* msg = SOPC_ByteString_Create(void);
            status_code = SOPC_String_InitializeFromCString((SOPC_String*) msg, "My first writing into file");
            FT_ASSERT(status_code == SOPC_GoodGenericStatus, "during tmp file opening", NULL);
            status_code = SOPC_FileTransfer_Write_TmpFile(&file->handle, );
            status = SOPC_ServerHelper_Serve(true);
        }
    }

    SOPC_FileTransfer_Clear();
    printf("******* Server stopped\n");

    if (SOPC_STATUS_OK != status)
    {
        return 1;
    }

    return 0;
}