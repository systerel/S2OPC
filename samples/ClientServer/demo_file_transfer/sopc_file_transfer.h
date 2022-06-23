#include "libs2opc_server.h"
#include "sopc_builtintypes.h"

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
 * \brief Bit masks for opening mode
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
 * \brief File handle type
 */
typedef uint32_t FT_FileHandle;

/**
 * \brief Open mode type
 */
typedef SOPC_Byte FT_OpenMode;

/**
 * \brief structure to manage FileType object
 */
typedef struct FT_FileType_t
{
    SOPC_NodeId* node_id;
    FT_FileHandle handle;                        // The handle of the file
    SOPC_String* path;                           // the file path
    SOPC_NodeId* methodIds[NB_FILE_TYPE_METHOD]; // list of method associeted
    FT_OpenMode mode;
    bool is_open;
    SOPC_String* tmp_path; // The path with the ramdom tmp name created
    FILE* fp;
    SOPC_NodeId* variableIds[NB_VARIABLE];
    uint16_t open_count;
    uint64_t size_in_byte;
} FT_FileType_t;

/**
 * \brief Structure to gather FilType configuration data
 */
typedef struct SOPC_FileType_Config
{
    const char* file_path; /*!< The path to store the temporary file. This one has to include the prefix name of the
                              temporary file. */
    const char* fileType_nodeId;    /*!< The nodeId of the FileType object. */
    const char* met_openId;         /*!< The nodeId of the Open method. */
    const char* met_closeId;        /*!< The nodeId of the Close method. */
    const char* met_readId;         /*!< The nodeId of the Read method. */
    const char* met_writeId;        /*!< The nodeId of the Write method. */
    const char* met_getposId;       /*!< The nodeId of the GetPosistion method. */
    const char* met_setposId;       /*!< The nodeId of the SetPosition method. */
    const char* var_sizeId;         /*!< The nodeId of the Size variable. */
    const char* var_openCountId;    /*!< The nodeId of the OpenCount variable. */
    const char* var_userWritableId; /*!< The nodeId of the UserWritable variable. */
    const char* var_writableId;     /*!< The nodeId of the Writable variable. */
} SOPC_FileType_Config;

/**
 * \brief Create a FileType object.
 * \note Memory allocation, need to call SOPC_FileTransfer_FileType_Delete after use.
 * \return The FileType object structure allocated
 */
FT_FileType_t* SOPC_FilteTransfer_FileType_Create(void);

/**
 * \brief Initialize a FileType object with its default value.
 * \note Random value is use to generate the file handle.
 * \param filetype The pointer on the FileType object structure to initialize.
 */
void SOPC_FilteTransfer_FileType_Initialize(FT_FileType_t* filetype);

/**
 * \brief Clear the FileType structure fields.
 * \param filetype The pointer on the FileType object structure to clear.
 */
void SOPC_FileTransfer_FileType_Clear(FT_FileType_t* filetype);

/**
 * \brief Delete the FileType structure (clear and free).
 * \param filetype Pointer on the FileType object structure to delete.
 */
void SOPC_FileTransfer_FileType_Delete(FT_FileType_t** filetype);

/**
 * \brief Create a temporary file at the path location (path is the field of the FileType structure)
 * \note This function set the field tmp_path with the field path + random suffix.
 * \param file Pointer on the FileType object structure.
 * \return SOPC_GoodGenericStatus if no error
 */
SOPC_StatusCode SOPC_FileTransfer_Create_TmpFile(FT_FileType_t* file);

/**
 * \brief Open the temporary file and lock it.
 * \param file Pointer on the FileType object structure.
 * \return SOPC_GoodGenericStatus if no error
 */
SOPC_StatusCode SOPC_FileTransfer_Open_TmpFile(FT_FileType_t* file);

/**
 * \brief Close the temporary file and unlock it.
 * \note This function is usefull for the Close method implementation.
 * \param handle The handle of the file to close
 * \param objectId The nodeID of the FileType object on the address space.
 * \return SOPC_GoodGenericStatus if no error
 */
SOPC_StatusCode SOPC_FileTransfer_Close_TmpFile(FT_FileHandle handle, const SOPC_NodeId* objectId);

/**
 * \brief Delete the temporary file (removed from the memory location).
 * \param file Pointer on the FileType object structure.
 * \return SOPC_GoodGenericStatus if no error
 */
SOPC_StatusCode SOPC_FileTransfer_Delete_TmpFile(FT_FileType_t* file);

/**
 * \brief Read into the temporary file (from the current position).
 * \note This function is usefull for the Read method implementation.
 * \param handle The handle of the file to read.
 * \param length The byte number to read
 * \param msg The output buffer
 * \param objectId The nodeID of the FileType object on the address space.
 * \return SOPC_GoodGenericStatus if no error
 */
SOPC_StatusCode SOPC_FileTransfer_Read_TmpFile(FT_FileHandle handle,
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
SOPC_StatusCode SOPC_FileTransfer_Write_TmpFile(FT_FileHandle handle,
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
SOPC_StatusCode SOPC_FileTransfer_SetPos_TmpFile(FT_FileHandle handle, const SOPC_NodeId* objectId, uint64_t posOff);

/**
 * \brief Get the file position of temporary file.
 * \note This function is usefull for the GetPosition method implementation.
 * \param handle The handle of the file.
 * \param objectId The nodeID of the FileType object on the address space.
 * \param pos The output position to get.
 * \return SOPC_GoodGenericStatus if no error
 */
SOPC_StatusCode SOPC_FileTransfer_GetPos_TmpFile(FT_FileHandle handle, const SOPC_NodeId* objectId, uint64_t* pos);

/**
 * \brief Initialise the API.
 * \note Memory allocation, need to call SOPC_FileTransfer_Clear after use.
 * \return SOPC_STATUS_OK if no error
 */
SOPC_ReturnStatus SOPC_FileTransfer_Initialize(void);

/**
 * \brief Adding a FileType object to the API from the address space information.
 * \note This function shall be call after <SOPC_FileTransfer_Initialize>.
 * \param config The structure which gather FileType configuration data
 * \return SOPC_STATUS_OK if no error
 */
SOPC_ReturnStatus SOPC_FileTransfer_Add_File(const SOPC_FileType_Config config);

/**
 * \brief Start the server asynchronously
 * \param ServerStoppedCallback  callback called when server will stop (on purpose or due to endpoint opening isssue)
 * \return SOPC_STATUS_OK in case of success. SOPC_STATUS_INVALID_STATE if the configuration is not possible (toolkit
 * not initialized, server already started). SOPC_STATUS_NOK if at least one local write of variable failed for one
 * fileType object on the address space.
 */
SOPC_ReturnStatus SOPC_FileTransfer_StartServer(SOPC_ServerStopped_Fct* ServerStoppedCallback);

/**
 * \brief Function to get the temporary file path name created by the API.
 * \param node_id The nodeId of the file object.
 * \param name The output name of the temporary file path.
 * \return SOPC_STATUS_OK if no error
 */
SOPC_ReturnStatus SOPC_FileTransfer_Get_TmpPath(const char* node_id, char* name);

/**
 * \brief Uninitialize the API (Free the memory)
 */
void SOPC_FileTransfer_Clear(void);
