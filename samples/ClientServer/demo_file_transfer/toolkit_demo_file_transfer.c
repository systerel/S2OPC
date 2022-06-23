
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
#include "sopc_file_transfer.h"
#include "sopc_mem_alloc.h"
#include "sopc_platform_time.h"
#include "sopc_assert.h"
#include "sopc_logger.h"


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

static void ServerStoppedCallback(SOPC_ReturnStatus status)
{
    (void) status;
    SOPC_FileTransfer_Clear();
    printf("******* Server stopped\n");
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
    assert(SOPC_STATUS_OK == status);
    if (SOPC_STATUS_OK == status)
    {
        status = Server_LoadServerConfigurationFromPaths();
    }
    assert(SOPC_STATUS_OK == status);

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
                                             .fileType_nodeId = "ns=1;i=15017",
                                             .met_openId = "ns=1;i=15048",
                                             .met_closeId = "ns=1;i=15051",
                                             .met_readId = "ns=1;i=15053",
                                             .met_writeId = "ns=1;i=15056",
                                             .met_getposId = "ns=1;i=15058",
                                             .met_setposId = "ns=1;i=15061",
                                             .var_sizeId = "ns=1;i=15018",
                                             .var_openCountId = "ns=1;i=15021",
                                             .var_userWritableId = "ns=1;i=15020",
                                             .var_writableId = "ns=1;i=15019"};

        printf("******* File added ...\n");
        status = SOPC_FileTransfer_Add_File(config);
        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to add file into server\n");
            SOPC_FileTransfer_Clear();
            return 1;
        }

        status = SOPC_FileTransfer_StartServer(ServerStoppedCallback);

        if (SOPC_STATUS_OK != status)
        {
            printf("******* Failed to start server ...\n");
            SOPC_FileTransfer_Clear();
            return 1;
        }

        printf("******* Start server ...\n");

        /********************/
        /* USER CODE BEGING */
        /********************/

        char name[STR_BUFF_SIZE];
        while (1)
        {
            status = SOPC_FileTransfer_Get_TmpPath(config.fileType_nodeId, name);
            if (SOPC_STATUS_OK == status)
            {
                printf("<toolkit_demo_file_transfer> Tmp file path name = '%s'\n", name);
            }
            SOPC_Sleep(500);
        }

        /********************/
        /* END USER CODE   */
        /********************/
    }

    return 0;
}