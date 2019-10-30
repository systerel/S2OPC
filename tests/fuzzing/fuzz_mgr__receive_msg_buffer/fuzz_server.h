#ifndef FUZZ_MGR__RECEIVE_MSG_BUFFER_SERVER
#define FUZZ_MGR__RECEIVE_MSG_BUFFER_SERVER

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include "fuzz_main.h"

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"

// def! setup and teardown
SOPC_ReturnStatus Setup_serv(void);		 // Server initialization
void StopSignal_serv(int sig);     		 // Catch the sigint and call Teardown_serv function
SOPC_ReturnStatus Teardown_serv(void);   // Free memory
// !endef

typedef enum
{
    AS_LOADER_EMBEDDED,
} AddressSpaceLoader;

// These variables are global to be accessible from StopSignal_serv
extern SOPC_AddressSpace* address_space;
extern t_CerKey ck_serv;
extern volatile sig_atomic_t stopServer;
extern uint32_t epConfigIdx;
extern SOPC_UserAuthentication_Manager* authenticationManager;
extern SOPC_UserAuthorization_Manager* authorizationManager;

#endif // FUZZ_MGR__RECEIVE_MSG_BUFFER_SERVER
