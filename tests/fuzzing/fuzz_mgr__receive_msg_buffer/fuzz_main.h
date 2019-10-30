#ifndef FUZZ_MGR__RECEIVE_MSG_BUFFER
#define FUZZ_MGR__RECEIVE_MSG_BUFFER

#define ENDPOINT_URL "opc.tcp://localhost:4841"
#define APPLICATION_URI "urn:S2OPC:localhost"
#define PRODUCT_URI "urn:S2OPC:localhost"

#include <stdbool.h>

#include "sopc_crypto_profiles.h"
#include "sopc_types.h"

#define ENDPOINT_URL "opc.tcp://localhost:4841"

void Fuzz_Event_Fct(SOPC_App_Com_Event event, uint32_t idOrStatus, void* param, uintptr_t appContext);
SOPC_ReturnStatus SOPC_EpConfig_serv();

typedef struct s_Cerkey
{
    SOPC_SerializedCertificate* Certificate;
    SOPC_SerializedAsymmetricKey* Key;
    SOPC_SerializedCertificate* authCertificate;
    SOPC_PKIProvider* pkiProvider;
} t_CerKey;

typedef enum
{
    SESSION_CONN_FAILED = -1,
    SESSION_CONN_CLOSED,
    SESSION_CONN_NEW,
    SESSION_CONN_CONNECTED,
	SESSION_CONN_MSG_RECEIVED,
} SessionConnectedState;

extern bool debug;
extern int32_t sendFailures;
extern bool secuActive;
extern SOPC_SecureChannel_Config scConfig;

extern SOPC_Endpoint_Config epConfig;
extern SOPC_S2OPC_Config output_s2opcConfig;

#endif // FUZZ_MGR__RECEIVE_MSG_BUFFER
