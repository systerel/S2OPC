#ifndef FUZZ_MGR__RECEIVE_MSG_BUFFER_SERVER
#define FUZZ_MGR__RECEIVE_MSG_BUFFER_SERVER

#include <stdlib.h>
#include "fuzz_mgr__receive_msg_buffer.h"

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"

// def! setup and teardown
SOPC_ReturnStatus Setup_serv(void); // Server initialization
void StopSignal_serv(int sig);      // Catch the sigint and call Teardown_serv function
void Teardown_serv();               // Free memory
// !endef

typedef enum
{
    AS_LOADER_EMBEDDED,
} AddressSpaceLoader;

#endif // FUZZ_MGR__RECEIVE_MSG_BUFFER_SERVER
