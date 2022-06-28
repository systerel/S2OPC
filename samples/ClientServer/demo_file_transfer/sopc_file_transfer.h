#include "libs2opc_server.h"
#include "sopc_builtintypes.h"

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
} SOPC_FileType_Config;

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
