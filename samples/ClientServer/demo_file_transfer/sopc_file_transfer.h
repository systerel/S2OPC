#include "libs2opc_server.h"
#include "sopc_builtintypes.h"

typedef struct SOPC_FileType SOPC_FileType;

/**
 * \brief User callback for the closing processing
 * \param file The pointer to the structure of the FileType object that gathers the file data. This parameter is
 * optional and may not be used by the user. \return In the function code, the user must return SOPC_STATUT_OK if no
 * error otherwise SOPC_STATUT_NOK.
 */
typedef SOPC_ReturnStatus (*SOPC_FileTransfer_UserClose_Callback)(SOPC_FileType* file);

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
 * \return SOPC_STATUS_OK if no error
 */
SOPC_ReturnStatus SOPC_FileTransfer_Initialize(void);

/**
 * \brief Adding a FileType object to the API from the address space information.
 * \note This function shall be call after <SOPC_FileTransfer_Initialize> and before <SOPC_FileTransfer_StartServer>
 * \param config The structure which gather FileType configuration data
 * \return SOPC_STATUS_OK if no error
 */
SOPC_ReturnStatus SOPC_FileTransfer_Add_File(const SOPC_FileType_Config config);

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
 * \brief Function to get the temporary file path name created by the API.
 * \param file A pointer to the structure of the FileType object.
 * \param name The output name of the temporary file path.
 * \return SOPC_STATUS_OK if no error otherwise SOPC_STATUS_NOK
 */
SOPC_ReturnStatus SOPC_FileTransfer_Get_TmpPath(SOPC_FileType* file, char* name);

/**
 * \brief Function to write a single value of variable (array are not supported).
 * \param CnodeId The nodeId of the variable to write.
 * \param UserBuiltInId The type of the variable.
 * \param pUserValue A pointer allocated to write the value with the same type as the UserBuiltInId parameter.
 * \return SOPC_STATUS_OK if no error otherwise SOPC_STATUS_NOK
 */
SOPC_ReturnStatus SOPC_FileTransfer_WriteVariable(const char* CnodeId, SOPC_BuiltinId UserBuiltInId, void* pUserValue);

/**
 * \brief Function to read a single value of variable (array are not supported).
 * \note This is a blocking function.
 * \param CnodeId The nodeId of the variable to write.
 * \param UserBuiltInId The type of the variable.
 * \param pUserValue A pointer allocated to read the value.
 * \param timeout Timeout in milliseconds (wait until service response is received)
 * \return SOPC_STATUS_OK if no error otherwise SOPC_STATUS_NOK
 */
SOPC_ReturnStatus SOPC_FileTransfer_ReadVariable(const char* CnodeId, void* pUserValue, uint32_t timeout);
/**
 * \brief Uninitialize the API (Free the memory)
 */
void SOPC_FileTransfer_Clear(void);
