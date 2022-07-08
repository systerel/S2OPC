
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
#include "sopc_assert.h"
#include "sopc_file_transfer.h"
#include "sopc_logger.h"
#include "sopc_mem_alloc.h"
#include "sopc_platform_time.h"

#define BUFF_SIZE 100u

/*-----------------------
 * Logger configuration :
 *-----------------------*/

/* Set the log path and set directory path built on executable name prefix */
static char* Server_ConfigLogPath(const char* logDirName)
{
    char* logDirPath = NULL;

    size_t logDirPathSize = strlen(logDirName) + 7; // <logDirName> + "_logs/" + '\0'

    logDirPath = SOPC_Malloc(logDirPathSize * sizeof(char));

    if (NULL != logDirPath &&
        (int) (logDirPathSize - 1) != snprintf(logDirPath, logDirPathSize, "%s_logs/", logDirName))
    {
        SOPC_Free(logDirPath);
        logDirPath = NULL;
    }

    return logDirPath;
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

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

// static SOPC_ReturnStatus Server_LoadServerConfigurationFromFiles(void)
// {
//     /* Retrieve XML configuration file path from environment variables TEST_SERVER_XML_CONFIG,
//      * TEST_SERVER_XML_ADDRESS_SPACE and TEST_USERS_XML_CONFIG.
//      *
//      * In case of success returns the file path otherwise display error message and return failure status.
//      */

//     const char* xml_server_config_path = getenv("TEST_SERVER_XML_CONFIG");
//     const char* xml_address_space_config_path = getenv("TEST_SERVER_XML_ADDRESS_SPACE");
//     const char* xml_users_config_path = getenv("TEST_USERS_XML_CONFIG");

//     if (NULL == xml_server_config_path || NULL == xml_address_space_config_path || NULL == xml_users_config_path)
//     {
//         printf("Error: an XML server configuration file path shall be provided\n");
//         return SOPC_STATUS_INVALID_PARAMETERS;
//     }

//     return SOPC_HelperConfigServer_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
//                                                     xml_users_config_path, NULL);
// }

static SOPC_StatusCode RemoteExecution_Method_Test(const SOPC_CallContext* callContextPtr,
                                                   const SOPC_NodeId* objectId,
                                                   uint32_t nbInputArgs,
                                                   const SOPC_Variant* inputArgs,
                                                   uint32_t* nbOutputArgs,
                                                   SOPC_Variant** outputArgs,
                                                   void* param)
{
    /********************/
    /* USER CODE BEGING */
    /********************/

    /* avoid unused parameter compiler warning */
    (void) callContextPtr;
    (void) objectId;
    (void) param;
    (void) nbInputArgs;
    (void) inputArgs;
    (void) param;
    /* The list of output argument shall be empty if the statusCode Severity is Bad (Table 65 – Call Service Parameters
     * / spec V1.05)*/
    *nbOutputArgs = 0;
    *outputArgs = NULL;

    SOPC_StatusCode status = SOPC_GoodGenericStatus;

    SOPC_Variant* v = SOPC_Variant_Create(); // Free by the Method Call Manager
    if (NULL != v)
    {
        v->ArrayType = SOPC_VariantArrayType_SingleValue;
        v->BuiltInTypeId = SOPC_Boolean_Id;
        v->Value.Boolean = true;

        *nbOutputArgs = 1;
        *outputArgs = v;
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "FileTransfer:UserMethod_Test: unable to create a variant");
        status = OpcUa_BadUnexpectedError;
    }

    printf("<toolkit_demo_file_transfer> Test of the user method: successful!\n");

    return status;

    /********************/
    /* END USER CODE   */
    /********************/
}

static void ServerStoppedCallback(SOPC_ReturnStatus status)
{
    (void) status;
    SOPC_FileTransfer_Clear();
    printf("******* Server stopped\n");
}

/*
 * User Close method callback definition.
 */
static SOPC_ReturnStatus UserCloseCallback(SOPC_FileType* file)
{
    /********************/
    /* USER CODE BEGING */
    /********************/
    SOPC_ReturnStatus status;
    char name[BUFF_SIZE];
    status = SOPC_FileTransfer_Get_TmpPath(file, name);
    if (SOPC_STATUS_OK == status)
    {
        printf("<toolkit_demo_file_transfer> Tmp file path name = '%s'\n", name);
    }
    return status;
    /********************/
    /* END USER CODE   */
    /********************/
}

/*
 * User Server callback definition used for address space modification by client.
 */
static void UserWriteNotificationCallback(const SOPC_CallContext* callContextPtr,
                                          OpcUa_WriteValue* writeValue,
                                          SOPC_StatusCode writeStatus)
{
    /********************/
    /* USER CODE BEGING */
    /********************/
    const SOPC_User* user = SOPC_CallContext_GetUser(callContextPtr);
    const char* writeSuccess = (SOPC_STATUS_OK == writeStatus ? "success" : "failure");
    char* sNodeId = SOPC_NodeId_ToCString(&writeValue->NodeId);

    printf("Write notification (%s) on node '%s' by user '%s'\n", writeSuccess, sNodeId, SOPC_User_ToCString(user));
    SOPC_Free(sNodeId);
    /********************/
    /* END USER CODE   */
    /********************/
}

int main(int argc, char* argv[])
{
    printf("******* API test\n");

    // Note: avoid unused parameter warning from compiler
    (void) argc;

    /* Configure the server logger:
     * DEBUG traces generated in ./<argv[0]_logs/ */
    SOPC_Log_Configuration logConfig = SOPC_Common_GetDefaultLogConfiguration();
    logConfig.logLevel = SOPC_LOG_LEVEL_DEBUG;
    char* logDirPath = Server_ConfigLogPath(argv[0]);
    logConfig.logSysConfig.fileSystemLogConfig.logDirPath = logDirPath;

    SOPC_ReturnStatus status;
    SOPC_StatusCode status_code;
    (void) status_code;
    status = SOPC_CommonHelper_Initialize(&logConfig);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_Initialize();
    }
    if (SOPC_STATUS_OK == status)
    {
        /* status = Server_LoadServerConfigurationFromFiles(); */
        status = Server_LoadServerConfigurationFromPaths();
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_HelperConfigServer_SetWriteNotifCallback(&UserWriteNotificationCallback);
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to configure the @ space modification notification callback\n");
        }
    }

    /* Finalize the server configuration */
    if (SOPC_STATUS_OK == status)
    {
        printf("******* Code begin ...\n");

        status = SOPC_FileTransfer_Initialize();
        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER, "API: FileTransfer intialization failed");
            SOPC_ASSERT(SOPC_STATUS_OK == status && "during file transfer intialization");
        }

        const SOPC_FileType_Config config = {.file_path = "/tmp/myfile",
                                             .fileType_nodeId = "ns=1;i=15042",
                                             .met_openId = "ns=1;i=15049",
                                             .met_closeId = "ns=1;i=15052",
                                             .met_readId = "ns=1;i=15054",
                                             .met_writeId = "ns=1;i=15138",
                                             .met_getposId = "ns=1;i=15140",
                                             .met_setposId = "ns=1;i=15143",
                                             .var_sizeId = "ns=1;i=15043",
                                             .var_openCountId = "ns=1;i=15047",
                                             .var_userWritableId = "ns=1;i=15045",
                                             .var_writableId = "ns=1;i=15044",
                                             .pFunc_UserCloseCallback = &UserCloseCallback};

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_FileTransfer_Add_File(config);
            if (SOPC_STATUS_OK != status)
            {
                printf("******* Failed to add file into server\n");
                SOPC_FileTransfer_Clear();
                return 1;
            }
        }
        printf("******* File added ...\n");

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_FileTransfer_Add_MethodItems(&RemoteExecution_Method_Test, "RemoteExecution_Method_Test",
                                                       "ns=1;i=15790");
            if (SOPC_STATUS_OK != status)
            {
                printf("******* Failed to add UserMethod_Test to the server ...\n");
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_FileTransfer_StartServer(ServerStoppedCallback);
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to start server ...\n");
            SOPC_FileTransfer_Clear();
            return 1;
        }
        printf("******* Start server ...\n");
    }

    /********************/
    /* USER CODE BEGING */
    /********************/

    SOPC_Boolean var_executable = true;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_FileTransfer_WriteVariable("ns=1;i=15792", SOPC_Boolean_Id, &var_executable);
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to write Executable variable (RemoteReset node)\n");
        }
    }

    SOPC_String* var_operationState = SOPC_String_Create();
    SOPC_String* var_operationState_readback = SOPC_String_Create();

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_CopyFromCString(var_operationState, "This is a test");
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_FileTransfer_WriteVariable("ns=1;i=15626", SOPC_String_Id, var_operationState);
        }
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to write OperationState variable (Items node)\n");
        }
    }
    status = SOPC_FileTransfer_ReadVariable("ns=1;i=15626", var_operationState_readback, 5000u);
    if (SOPC_STATUS_OK != status)
    {
        printf("******* ReadBack OperationState variable (failure)\n");
    }
    else
    {
        printf("******* ReadBack on OperationState (success): %s\n",
               SOPC_String_GetCString(var_operationState_readback));
    }

    SOPC_String_Delete(var_operationState);
    SOPC_String_Delete(var_operationState_readback);
    var_operationState = NULL;
    var_operationState_readback = NULL;

    while (1)
    {
        SOPC_Sleep(500);
    }

    /********************/
    /* END USER CODE   */
    /********************/
    return 0;
}