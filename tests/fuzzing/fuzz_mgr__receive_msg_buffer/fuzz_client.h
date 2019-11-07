#ifndef FUZZ_MGR__RECEIVE_MSG_BUFFER_CLIENT
#define FUZZ_MGR__RECEIVE_MSG_BUFFER_CLIENT

#define NB_SESSIONS 1
#define DESIGNATE_NEW(T, ...) memcpy(SOPC_Malloc(sizeof(T)), &(T const){__VA_ARGS__}, sizeof(T))

#include "fuzz_main.h"

// Prototypage
SOPC_ReturnStatus Wait_response_client();
OpcUa_WriteRequest* newWriteRequest_client(const char* buff, size_t len);
SOPC_ReturnStatus AddSecureChannelconfig_client();

SOPC_ReturnStatus Setup_client();
SOPC_ReturnStatus Run_client(char* buff, size_t len);
SOPC_ReturnStatus Teardown_client();

extern OpcUa_WriteRequest* pWriteReq;

extern t_CerKey ck_cli;
extern uint32_t session;
extern uintptr_t sessionContext[2];
extern SessionConnectedState scState;
extern uint32_t channel_config_idx;

#endif // FUZZ_MGR__RECEIVE_MSG_BUFFER_CLIENT
