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

typedef struct s_Cerkey_suck // serialized for unit test certificate and key
{
    SOPC_SerializedCertificate* Certificate = NULL;
    SOPC_SerializedAsymmetricKey* Key = NULL;
    SOPC_SerializedCertificate* authCertificate = NULL;
    SOPC_PKIProvider* pkiProvider = NULL;
} t_CerKey;

typedef enum
{
    SESSION_CONN_FAILED = -1,
    SESSION_CONN_PENDING,
    SESSION_CONN_CONNECTED,
} SessionConnectedState;

bool debug = true;
uint32_t session = 0;

SessionConnectedState scState = SESSION_CONN_PENDING;
SOPC_SecureChannel_Config* scConfig = NULL;

#endif // FUZZ_MGR__RECEIVE_MSG_BUFFER
