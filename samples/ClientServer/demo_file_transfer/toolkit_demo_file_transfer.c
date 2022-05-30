
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

int main(int argc, char* argv[])
{
    printf("******* API test\n");
    // Note: avoid unused parameter warning from compiler
    (void) argc;
    (void) argv;

    SOPC_Log_Configuration* log_conf = NULL;
    SOPC_ReturnStatus status;
    SOPC_StatusCode status_code;
    (void) status_code;
    status = SOPC_CommonHelper_Initialize(log_conf);
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
        FT_ASSERT(SOPC_STATUS_OK == status, "during file transfer intialization", NULL);

        const char* file_id = "ns=1;i=15017";
        const char* path = "/tmp/myfile";
        const char* open_id = "ns=1;i=15048";
        const char* close_id = "ns=1;i=15051";
        const char* read_id = "ns=1;i=15053";
        const char* write_id = "ns=1;i=15056";
        const char* getPos_id = "ns=1;i=15058";
        const char* setPos_id = "ns=1;i=15061";

        printf("******* File added ...\n");
        SOPC_FileTransfer_Add_File(file_id, path, open_id, close_id, read_id, write_id, getPos_id, setPos_id);
        printf("******* Start server ...\n");
        status = SOPC_ServerHelper_Serve(true);
    }

    SOPC_FileTransfer_Clear();
    printf("******* Server stopped\n");

    if (SOPC_STATUS_OK != status)
    {
        return 1;
    }

    return 0;
}