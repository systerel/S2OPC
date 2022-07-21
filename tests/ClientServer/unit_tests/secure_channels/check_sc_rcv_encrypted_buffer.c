/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/** \file
 *
 * \brief Entry point for tools tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <assert.h>
#include <check.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> /* EXIT_* */

#include "check_sc_rcv_helpers.h"
#include "hexlify.h"
#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_crypto_profiles.h"
#include "sopc_encoder.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_pki_stack.h"
#include "sopc_secure_channels_api.h"
#include "sopc_time.h"
#include "sopc_toolkit_config.h"
#include "sopc_toolkit_config_constants.h"
#include "sopc_types.h"
#include "stub_sc_sopc_sockets_api.h"

#include "opcua_identifiers.h"
#include "opcua_statuscodes.h"

static const char* sEndpointUrl = "opc.tcp://localhost:8888/myEndPoint";

static const uint32_t pendingRequestHandle = 1000;

static SOPC_SecureChannel_Config scConfig;

// Configuration SC idx provided on configuration (used also as socket / scIdx)
uint32_t scConfigIdx = 0;

static SOPC_PKIProvider pki;
static int32_t pkiValidationAsked = false;
static SOPC_SerializedCertificate *crt_cli = NULL, *crt_srv = NULL, *crt_ca = NULL;
static SOPC_SerializedAsymmetricKey* priv_cli = NULL;

static SOPC_ReturnStatus Check_Client_Closed_SC_Helper(SOPC_StatusCode status)
{
    return Check_Client_Closed_SC(scConfigIdx, scConfigIdx, scConfigIdx, pendingRequestHandle, status);
}

static SOPC_ReturnStatus PKIStub_ValidateAnything(const SOPC_PKIProvider* pPKI,
                                                  const SOPC_CertificateList* pToValidate,
                                                  uint32_t* error)
{
    SOPC_UNUSED_ARG(pPKI);
    SOPC_UNUSED_ARG(pToValidate);
    SOPC_UNUSED_ARG(error);

    SOPC_Atomic_Int_Set(&pkiValidationAsked, true);
    return SOPC_STATUS_OK;
}

static void PKIStub_Free(SOPC_PKIProvider* pPKI)
{
    SOPC_UNUSED_ARG(pPKI);
}

static void clearToolkit(void)
{
    SOPC_Toolkit_Clear();
    Check_SC_Clear();

    // Free all allocated resources
    SOPC_KeyManager_SerializedCertificate_Delete(crt_cli);
    SOPC_KeyManager_SerializedCertificate_Delete(crt_srv);
    SOPC_KeyManager_SerializedCertificate_Delete(crt_ca);
    SOPC_KeyManager_SerializedAsymmetricKey_Delete(priv_cli);
}

static void establishSC(void)
{
    printf("\nSTART UNIT TEST\n");

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_Event* serviceEvent = NULL;
    SOPC_Event* socketEvent = NULL;
    int res = 0;
    SOPC_Buffer* buffer = NULL;
    char hexOutput[512];

    // Endpoint URL
    SOPC_String stEndpointUrl;
    SOPC_String_Initialize(&stEndpointUrl);
    // Policy security:
    char* pRequestedSecurityPolicyUri = SOPC_SecurityPolicy_Basic256Sha256_URI;

    // Message security mode: None
    OpcUa_MessageSecurityMode messageSecurityMode = OpcUa_MessageSecurityMode_SignAndEncrypt;

    printf(">>SC_Rcv_Buffer Init: Security mode ='SignAndEncrypt'\n");

    // Paths to client certificate/key and server certificate
    // Client certificate name
    char* certificateLocation = "./check_sc_rcv_encrypted_buffer_data/check_sc_client_2k_cert.der";
    // Server certificate name
    char* certificateSrvLocation = "./check_sc_rcv_encrypted_buffer_data/check_sc_server_2k_cert.der";
    // Client private key
    char* keyLocation = "./check_sc_rcv_encrypted_buffer_data/check_sc_client_2k_key.pem";

    if (SOPC_STATUS_OK == status)
    {
        printf("SC_Rcv_Buffer Init: used paths for keys and certificates ='%s', '%s' and '%s'\n", keyLocation,
               certificateLocation, certificateSrvLocation);

        // The certificates: load
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(certificateLocation, &crt_cli);
        if (SOPC_STATUS_OK != status)
        {
            printf("SC_Rcv_Buffer Init: Failed to load client certificate\n");
        }
        else
        {
            printf("SC_Rcv_Buffer Init: Client certificate loaded\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_KeyManager_SerializedCertificate_CreateFromFile(certificateSrvLocation, &crt_srv);
        if (SOPC_STATUS_OK != status)
        {
            printf("SC_Rcv_Buffer Init: Failed to load server certificate\n");
        }
        else
        {
            printf("SC_Rcv_Buffer Init: Server certificate loaded\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        // Private key: load
        status = SOPC_KeyManager_SerializedAsymmetricKey_CreateFromFile(keyLocation, &priv_cli);
        if (SOPC_STATUS_OK != status)
        {
            printf("SC_Rcv_Buffer Init: Failed to load private key\n");
        }
        else
        {
            printf("SC_Rcv_Buffer Init: Client private key loaded\n");
        }
    }

    // Init a stub PKI only to check validate certificate function is called
    // (we do not want to validate the loaded certificates since they may have expired)
    if (SOPC_STATUS_OK == status)
    {
        // The pki function pointer shall be const after this init
        SOPC_GCC_DIAGNOSTIC_IGNORE_CAST_CONST
        *((SOPC_PKIProvider_Free_Func**) (&pki.pFnFree)) = &PKIStub_Free;
        *((SOPC_FnValidateCertificate**) (&pki.pFnValidateCertificate)) = &PKIStub_ValidateAnything;
        SOPC_GCC_DIAGNOSTIC_RESTORE
        pki.pTrustedIssuerRootsList = NULL;
        pki.pIssuedCertsList = NULL;
        pki.pUntrustedIssuerRootsList = NULL;
        pki.pCertRevocList = NULL;
        pki.pUserData = NULL;
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
        logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./check_sc_rcv_encrypted_buffer_logs/";
        logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
        status = SOPC_Common_Initialize(logConfiguration);
    }
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Init toolkit configuration
    status = SOPC_Toolkit_Initialize(NULL);
    if (SOPC_STATUS_OK != status)
    {
        printf("SC_Rcv_Buffer Init: Failed initializing toolkit\n");
    }
    else
    {
        printf("SC_Rcv_Buffer Init: Toolkit initialized\n");
    }
    ck_assert(SOPC_STATUS_OK == status);

    Check_SC_Init();

    memset(&scConfig, 0, sizeof(SOPC_SecureChannel_Config));
    scConfig.isClientSc = true;
    scConfig.msgSecurityMode = messageSecurityMode;
    scConfig.reqSecuPolicyUri = pRequestedSecurityPolicyUri;
    scConfig.requestedLifetime = 100000;
    scConfig.url = sEndpointUrl;
    scConfig.crt_cli = crt_cli;
    scConfig.crt_srv = crt_srv;
    scConfig.key_priv_cli = priv_cli;
    scConfig.pki = &pki;

    scConfigIdx = SOPC_ToolkitClient_AddSecureChannelConfig(&scConfig);
    ck_assert(scConfigIdx != 0);

    printf("SC_Rcv_Buffer Init: request connection to SC layer\n");
    SOPC_SecureChannels_EnqueueEvent(SC_CONNECT, scConfigIdx, (uintptr_t) NULL, 0);

    // Retrieve socket event
    printf("SC_Rcv_Buffer Init: Checking correct socket creation event received\n");
    socketEvent = Check_Socket_Event_Received(SOCKET_CREATE_CLIENT, scConfigIdx,
                                              0); // scConfigIdx == scIdx since there is only 1 SC

    ck_assert(socketEvent != NULL);

    res = strcmp(sEndpointUrl, (char*) socketEvent->params);
    if (res != 0)
    {
        printf("SC_Rcv_Buffer: Unexpected SOCKET_CREATE_CLIENT params\n");
        ck_assert(false);
    }
    SOPC_Free(socketEvent);
    socketEvent = NULL;

    // Simulate event from socket
    SOPC_EventHandler_Post(socketsEventHandler, SOCKET_CONNECTION, scConfigIdx, (uintptr_t) NULL, scConfigIdx);
    printf("SC_Rcv_Buffer Init: Simulating socket connection\n");

    printf("SC_Rcv_Buffer Init: Checking correct HEL message requested to be sent\n");

    // Check expected HEL message requested to be sent on socket
    status = Check_Expected_Sent_Message(scConfigIdx,
                                         // Expected HEL message
                                         "48454c464300000000000000ffff0000ffff0000fbff040005000000230000006f70632e74637"
                                         "03a2f2f6c6f63616c686f73743a383838382f6d79456e64506f696e74",
                                         false, 0, 0);
    ck_assert(SOPC_STATUS_OK == status);

    printf("SC_Rcv_Buffer Init: Simulate correct ACK message received\n");

    // Simulate ACK message received on socket
    status = Simulate_Received_Message(scConfigIdx,
                                       // Expected ACK message received
                                       "41434b461c00000000000000ffff0000ffff0000fbff040005000000");

    ck_assert(SOPC_STATUS_OK == status);
    printf("SC_Rcv_Buffer Init: Checking correct OPN message requested to be sent\n");

    // Check expected OPN request message to be sent on socket
    status = Check_Expected_Sent_Message(
        scConfigIdx,
        // Expected OPN message
        "4f504e46920600000000000039000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f5365637572697479506f6c"
        "6963792342617369633235365368613235362d0400003082042930820311a003020102020105300d06092a864886f70d01010b05003081"
        "88310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533110300e060355040a0c07"
        "494e474f5043533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f5043532043413122302006092a8648"
        "86f70d0109011613696e676f70637340737973746572656c2e6672301e170d3137313130323136313535385a170d313831313032313631"
        "3535385a3068310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533110300e0603"
        "55040a0c07494e474f5043533110300e060355040b0c07494e474f5043533117301506035504030c0e494e474f50435320434c49454e54"
        "30820122300d06092a864886f70d01010105000382010f003082010a0282010100ef5a09ea13fe0a325df423390268de4eba8f41a8f146"
        "dd888578000b83db5b7a702f2fb013bf259f4d5f045c38bcbc2361c4e85eda9504041dcec33d17fe31f905c81d12236eba2157472c5b26"
        "686b72aa33eaba7bb3e8e9aa66c2996236eb7c5c3e881afcdcae380511df907fa053a8a14ac27284f5bef513a1042e6ab65349b3a617a2"
        "1a044c118abc96501fe023c17fe0a3e9f4fb857c008fa86d7d9577ef9cb729b278770e1bca0b0a559c685679a703ee438b1053e029e901"
        "a437502ae7c4889b2304740c8d9aef62c128d1f39ed3d7dae105e23df6354ff096943405e697d8c35ef1c7f89c21f06037273fef17ee7d"
        "37fd05dd870c1af7b3d003d8a8890203010001a381bc3081b9301d0603551d0e04160414fa8d89278964f0c1e5a75e8aa19c2edfe46c6f"
        "48301f0603551d2304183016801431ae8e65336406e4379b518fc3379dce459bb61f300f0603551d130101ff040530030101ff300b0603"
        "551d0f040403020106302b0603551d1104243022861575726e3a494e474f5043533a6c6f63616c686f737482096c6f63616c686f737430"
        "2c06096086480186f842010d041f161d4f70656e53534c2047656e657261746564204365727469666963617465300d06092a864886f70d"
        "01010b05000382010100845c2ed4d6fa383de6398388bb2f0b9e7be01b197a40633bdffe56c6dfdc043bd6bf09176f8b0f962a36361a86"
        "5b42cdde8e5996da786bd5dad191fb98e991d8cf7d49e7a3bac0e1bdb92092fd194668f3f93c442ab4e612895a409dba03bbae5477989c"
        "a51e210127db6eb9e2500be93bfd7f8cc142c6a6b9b984d82ec5831052060455c9e14e3414ae4e29c440689d0b8ac4a3b0be5d7b47eaee"
        "2bc71a52e9a388d856081ef5edd8321c6bd5e48d688e6e0b6038fe96f759d0c67e6d9a66afa3bee92133a6a64d98894502692555cb7373"
        "c735f6b523e5380b5b9abe119b692db8e5d13d0f54c055cc7ee0b4828122d8b8deaef9e6a68af9ca3839a48caf85140000008bbe166057"
        "56eebcb6e10756bc17c57df3f1ef01"
        "0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000",
        true, 1170, 512); // ignore all encrypted data

    // Note: the content of the OPN request depends on the sender (client) and receiver (server) certificate loaded
    // here. To define the expected data it may be captured from TCP packet capture to be known as a correct exchange
    // with same certificates or may be defined "manually" from spec 1.03 part 6. The ciphered part has been set to zero
    // here.

    ck_assert(SOPC_STATUS_OK == status);
    printf("SC_Rcv_Buffer Init: Simulate correct OPN message response received\n");

    /* Check PKI validate function was not called until OPN response sent */
    ck_assert(false == SOPC_Atomic_Int_Get(&pkiValidationAsked));

    // Simulate OPN resp. message received on socket
    status = Simulate_Received_Message(
        scConfigIdx,
        /* Expected OPN resp. message received:
         * - SC id = 3321932765 <=> ddab00c6
         * - secu token = 190349500 <=> bc80580b
         * - SeqNum = 1
         * - RequestId = 1
         */
        "4f504e4692060000ddab00c639000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f5365637572697479506f6c"
        "6963792342617369633235365368613235362d0400003082042930820311a003020102020106300d06092a864886f70d01010b05003081"
        "88310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533110300e060355040a0c07"
        "494e474f5043533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f5043532043413122302006092a8648"
        "86f70d0109011613696e676f70637340737973746572656c2e6672301e170d3137313130323136353835345a170d313831313032313635"
        "3835345a3068310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533110300e0603"
        "55040a0c07494e474f5043533110300e060355040b0c07494e474f5043533117301506035504030c0e494e474f50435320534552564552"
        "30820122300d06092a864886f70d01010105000382010f003082010a0282010100b245842f18a25870b4bdcf162ca5b2d48cb531f0c5ba"
        "5d509e5051b892fe535d6dbfbafc6f3e430af250468659476cfd8381c57a9dc4063d14ef48b6ede87b1a15e3560a8a759a4cda397a7103"
        "4c812142c743d16685b6fe2f2bb3a56570a88f87452121ecb6bce7865e14ef83a5bd436661995a8a0635d3ba9b22324bd2d06f3cf11741"
        "8d6e9e2fa3e1d317c77ba00972e622d602d8f1c07a720ba7d064150ee0533f256ccc0bb6fa0447dcacc974627c54bb544c60cbef5aeb70"
        "ebfe8c07acaf64f315088e8461a4af2f9d8846065c416d0c1cc4075e4baa2d7f44c1be2bd1e2d8dec1f9796b8727cac05f5e931069192f"
        "ae496298a549bae6df699e2638270203010001a381bc3081b9301d0603551d0e0416041449da0583068c3c894ac32777d92ba84e8763f7"
        "22301f0603551d2304183016801431ae8e65336406e4379b518fc3379dce459bb61f300f0603551d130101ff040530030101ff300b0603"
        "551d0f040403020106302b0603551d1104243022861575726e3a494e474f5043533a6c6f63616c686f737482096c6f63616c686f737430"
        "2c06096086480186f842010d041f161d4f70656e53534c2047656e657261746564204365727469666963617465300d06092a864886f70d"
        "01010b0500038201010053e982480661e4bffb4af3f1b37d663e4191a3b0cfb590e9f50008634c63d3b44297b7544dee44f103f589c15f"
        "59b080e037603309e0afe6b9d294f61ad85f5de27275983d6d81353e97dfc30bbbcc3b359a8720206bc298bc9f0da97a4dbd24c0c07a96"
        "07071c3d729ad2849ee0c3d2b410ee9e066aef3a9bf3e755f19270808c5efbaf46a8c8ca7d290162a818c1e6abba08c973e4d6857487a6"
        "2e0a34a61930150e2e2639fc5ec8234f5a86a38bd9e7454f8157732f6028adbcb1525770671c0443724270d4bf8fa530193e033712a2e2"
        "f2b8581d3dcaa974cb3504c475582322b8f0ffb212a714fea2b6016eb928ef523ec65f0b160063ba207c45caa2d7140000005d30ae4f95"
        "6a2e2ee184e2214af19bbecf9792f876656981224ccdddce8a7b0287fdd91ce351f459dc15ddcdf94010ad76d2b53b79ae816e1216c589"
        "1d87ae40a133ac188f450a87eb1ef4007c8d58817428511f07645eda35d813ea900b10e7eb51d6880f188738c3fb52ce884539193890b9"
        "36904b58e172a9571cfe6f659a54e701a522a8a2ba2c0c33eed07672bb524c00bd9423db5e5b8e1bdc6cd2a71aee68f657fbc52b8a8399"
        "27de09254977dfcefb9c4be0c0b7a00faec3b6f577904478291863faa8b5ea89705da8f84eacedcbdd93152fa0aaf877593d75ebb8b434"
        "d7b045702670791c6b1b11dfe7234b896a2cc5772813a73b3308f1a63564c5f7cbade4f3ea840aac09b9d0d2337a5e4095db708dcaa955"
        "c3e19a6b64854e434d190d349bd62c681b8cbd6354d3bdf402b42324821f42d495a206df466360b60a296c4d820ca1516dc59e953f86f2"
        "cfc4e9732c28258b82e315f543c3abb2817f9d38a8c8c0f261580ae5ce87265155d859e1a90fd2553c0f3926224da5ddc28c44881916ab"
        "baf721e11618f5d3e4e452c8b1b8d415b467c2ae2d0fa9652ad3c3d980c744922019e721264833fee951a0c63cf2bbd0e1e8ad0a458ccc"
        "3b6ace58dd39360c2c8c26ee571b856b2cf7d867c692f4dba00fa1e3d927cde64f6e78d7a021e799f7946ffd20c56b0b1f5b111ad5ff8c"
        "d6be2503af8344ea3bde3e944f3a0795deed71e09aa940ed19ba37fcca16f942");

    // Note: the content of the OPN response depends on the sender (client) and receiver (server) certificate loaded
    // here but the SC id and security token shall be set in a deterministic way for the rest of the test.
    // To define the message data it may be captured from TCP packet capture to be known as a correct exchange with
    // same certificates and pre-determined SC id and security token. It may be done by instrumenting a server
    // code to force those values or by defining "manually" the non-ciphered data + ciphering "manually" the rest.

    // Retrieve expected service event
    ck_assert(SOPC_STATUS_OK == status);
    printf("SC_Rcv_Buffer Init: Checking correct connection established event received by services\n");
    serviceEvent = Check_Service_Event_Received(SC_CONNECTED, scConfigIdx, scConfigIdx);

    ck_assert(NULL != serviceEvent);
    SOPC_Free(serviceEvent);
    serviceEvent = NULL;

    /* Check the PKI validate function was called during OPN treatment */
    ck_assert(true == SOPC_Atomic_Int_Get(&pkiValidationAsked));
    SOPC_Atomic_Int_Set(&pkiValidationAsked, false);

    printf("SC_Rcv_Buffer: request to send an empty MSG and retrieve requestId associated\n");
    buffer = SOPC_Buffer_Create(1000); // Let 24 bytes reserved for the header
    ck_assert(buffer != NULL);

    status = SOPC_Buffer_SetDataLength(buffer, 24);
    ck_assert(SOPC_STATUS_OK == status);

    SOPC_SecureChannels_EnqueueEvent(SC_SERVICE_SND_MSG, scConfigIdx, (uintptr_t) buffer, pendingRequestHandle);

    socketEvent = Check_Socket_Event_Received(SOCKET_WRITE, scConfigIdx, 0);
    ck_assert_ptr_nonnull(socketEvent);
    ck_assert_ptr_nonnull((void*) socketEvent->params);

    buffer = (SOPC_Buffer*) socketEvent->params;
    res = hexlify(buffer->data, hexOutput, buffer->length);

    ck_assert((uint32_t) res == buffer->length);

    // Check typ = MSG final = F
    res = memcmp(hexOutput, "4d534746", 8);
    ck_assert(res == 0);

    // retrieve requestId in MSG
    status = SOPC_Buffer_SetPosition(buffer, 20);
    ck_assert(SOPC_STATUS_OK == status);

    SOPC_Buffer_Delete(buffer);
    SOPC_Free(socketEvent);
    socketEvent = NULL;

    ck_assert(SOPC_STATUS_OK == status);
}

START_TEST(test_unexpected_hel_msg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected HEL message received on socket\n");

    // Simulate HEL msg received on connected SC
    status = Simulate_Received_Message(scConfigIdx,
                                       "48454c464300000000000000ffff0000ffff0000ffff000001000000230000006f70632e7463703"
                                       "a2f2f6c6f63616c686f73743a383838382f6d79456e64506f696e74");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_ack_msg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected ACK message received on socket\n");

    // Simulate ACK msg received on connected SC
    status = Simulate_Received_Message(scConfigIdx, "41434b461c00000000000000ffff0000ffff0000ffff000001000000");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_opn_req_msg_replay)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected OPN req message received on socket (replay client msg)\n");

    // Simulate OPN req msg received on connected SC (replay: same as sent by client)
    status = Simulate_Received_Message(
        scConfigIdx,
        "4f504e46920600000000000039000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f5365637572697479506f6c"
        "6963792342617369633235365368613235362d0400003082042930820311a003020102020105300d06092a864886f70d01010b05003081"
        "88310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533110300e060355040a0c07"
        "494e474f5043533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f5043532043413122302006092a8648"
        "86f70d0109011613696e676f70637340737973746572656c2e6672301e170d3137313130323136313535385a170d313831313032313631"
        "3535385a3068310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533110300e0603"
        "55040a0c07494e474f5043533110300e060355040b0c07494e474f5043533117301506035504030c0e494e474f50435320434c49454e54"
        "30820122300d06092a864886f70d01010105000382010f003082010a0282010100ef5a09ea13fe0a325df423390268de4eba8f41a8f146"
        "dd888578000b83db5b7a702f2fb013bf259f4d5f045c38bcbc2361c4e85eda9504041dcec33d17fe31f905c81d12236eba2157472c5b26"
        "686b72aa33eaba7bb3e8e9aa66c2996236eb7c5c3e881afcdcae380511df907fa053a8a14ac27284f5bef513a1042e6ab65349b3a617a2"
        "1a044c118abc96501fe023c17fe0a3e9f4fb857c008fa86d7d9577ef9cb729b278770e1bca0b0a559c685679a703ee438b1053e029e901"
        "a437502ae7c4889b2304740c8d9aef62c128d1f39ed3d7dae105e23df6354ff096943405e697d8c35ef1c7f89c21f06037273fef17ee7d"
        "37fd05dd870c1af7b3d003d8a8890203010001a381bc3081b9301d0603551d0e04160414fa8d89278964f0c1e5a75e8aa19c2edfe46c6f"
        "48301f0603551d2304183016801431ae8e65336406e4379b518fc3379dce459bb61f300f0603551d130101ff040530030101ff300b0603"
        "551d0f040403020106302b0603551d1104243022861575726e3a494e474f5043533a6c6f63616c686f737482096c6f63616c686f737430"
        "2c06096086480186f842010d041f161d4f70656e53534c2047656e657261746564204365727469666963617465300d06092a864886f70d"
        "01010b05000382010100845c2ed4d6fa383de6398388bb2f0b9e7be01b197a40633bdffe56c6dfdc043bd6bf09176f8b0f962a36361a86"
        "5b42cdde8e5996da786bd5dad191fb98e991d8cf7d49e7a3bac0e1bdb92092fd194668f3f93c442ab4e612895a409dba03bbae5477989c"
        "a51e210127db6eb9e2500be93bfd7f8cc142c6a6b9b984d82ec5831052060455c9e14e3414ae4e29c440689d0b8ac4a3b0be5d7b47eaee"
        "2bc71a52e9a388d856081ef5edd8321c6bd5e48d688e6e0b6038fe96f759d0c67e6d9a66afa3bee92133a6a64d98894502692555cb7373"
        "c735f6b523e5380b5b9abe119b692db8e5d13d0f54c055cc7ee0b4828122d8b8deaef9e6a68af9ca3839a48caf85140000008bbe166057"
        "56eebcb6e10756bc17c57df3f1ef0000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"
        "0000000000000000000000000000000000000000000000000000000000000000");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpSecureChannelUnknown); // invalid SC ID == 0
    ck_assert(SOPC_STATUS_OK == status);

    /* Check the PKI validate function was not called during OPN treatment
     * Note: this is due to the fact that SC id == 0 whereas it should be the one defined by server previously */
    ck_assert(false == SOPC_Atomic_Int_Get(&pkiValidationAsked));
}
END_TEST

START_TEST(test_unexpected_opn_resp_msg_replay)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected OPN resp message received on socket (replay server msg)\n");

    // Simulate OPN resp msg received on connected SC (replay: same as sent by server the first time)
    status = Simulate_Received_Message(
        scConfigIdx,
        "4f504e4692060000ddab00c639000000687474703a2f2f6f7063666f756e646174696f6e2e6f72672f55412f5365637572697479506f6c"
        "6963792342617369633235365368613235362d0400003082042930820311a003020102020106300d06092a864886f70d01010b05003081"
        "88310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533110300e060355040a0c07"
        "494e474f5043533110300e060355040b0c07494e474f5043533113301106035504030c0a494e474f5043532043413122302006092a8648"
        "86f70d0109011613696e676f70637340737973746572656c2e6672301e170d3137313130323136353835345a170d313831313032313635"
        "3835345a3068310b3009060355040613024652310c300a06035504080c03494446310e300c06035504070c0550415249533110300e0603"
        "55040a0c07494e474f5043533110300e060355040b0c07494e474f5043533117301506035504030c0e494e474f50435320534552564552"
        "30820122300d06092a864886f70d01010105000382010f003082010a0282010100b245842f18a25870b4bdcf162ca5b2d48cb531f0c5ba"
        "5d509e5051b892fe535d6dbfbafc6f3e430af250468659476cfd8381c57a9dc4063d14ef48b6ede87b1a15e3560a8a759a4cda397a7103"
        "4c812142c743d16685b6fe2f2bb3a56570a88f87452121ecb6bce7865e14ef83a5bd436661995a8a0635d3ba9b22324bd2d06f3cf11741"
        "8d6e9e2fa3e1d317c77ba00972e622d602d8f1c07a720ba7d064150ee0533f256ccc0bb6fa0447dcacc974627c54bb544c60cbef5aeb70"
        "ebfe8c07acaf64f315088e8461a4af2f9d8846065c416d0c1cc4075e4baa2d7f44c1be2bd1e2d8dec1f9796b8727cac05f5e931069192f"
        "ae496298a549bae6df699e2638270203010001a381bc3081b9301d0603551d0e0416041449da0583068c3c894ac32777d92ba84e8763f7"
        "22301f0603551d2304183016801431ae8e65336406e4379b518fc3379dce459bb61f300f0603551d130101ff040530030101ff300b0603"
        "551d0f040403020106302b0603551d1104243022861575726e3a494e474f5043533a6c6f63616c686f737482096c6f63616c686f737430"
        "2c06096086480186f842010d041f161d4f70656e53534c2047656e657261746564204365727469666963617465300d06092a864886f70d"
        "01010b0500038201010053e982480661e4bffb4af3f1b37d663e4191a3b0cfb590e9f50008634c63d3b44297b7544dee44f103f589c15f"
        "59b080e037603309e0afe6b9d294f61ad85f5de27275983d6d81353e97dfc30bbbcc3b359a8720206bc298bc9f0da97a4dbd24c0c07a96"
        "07071c3d729ad2849ee0c3d2b410ee9e066aef3a9bf3e755f19270808c5efbaf46a8c8ca7d290162a818c1e6abba08c973e4d6857487a6"
        "2e0a34a61930150e2e2639fc5ec8234f5a86a38bd9e7454f8157732f6028adbcb1525770671c0443724270d4bf8fa530193e033712a2e2"
        "f2b8581d3dcaa974cb3504c475582322b8f0ffb212a714fea2b6016eb928ef523ec65f0b160063ba207c45caa2d7140000005d30ae4f95"
        "6a2e2ee184e2214af19bbecf9792f876656981224ccdddce8a7b0287fdd91ce351f459dc15ddcdf94010ad76d2b53b79ae816e1216c589"
        "1d87ae40a133ac188f450a87eb1ef4007c8d58817428511f07645eda35d813ea900b10e7eb51d6880f188738c3fb52ce884539193890b9"
        "36904b58e172a9571cfe6f659a54e701a522a8a2ba2c0c33eed07672bb524c00bd9423db5e5b8e1bdc6cd2a71aee68f657fbc52b8a8399"
        "27de09254977dfcefb9c4be0c0b7a00faec3b6f577904478291863faa8b5ea89705da8f84eacedcbdd93152fa0aaf877593d75ebb8b434"
        "d7b045702670791c6b1b11dfe7234b896a2cc5772813a73b3308f1a63564c5f7cbade4f3ea840aac09b9d0d2337a5e4095db708dcaa955"
        "c3e19a6b64854e434d190d349bd62c681b8cbd6354d3bdf402b42324821f42d495a206df466360b60a296c4d820ca1516dc59e953f86f2"
        "cfc4e9732c28258b82e315f543c3abb2817f9d38a8c8c0f261580ae5ce87265155d859e1a90fd2553c0f3926224da5ddc28c44881916ab"
        "baf721e11618f5d3e4e452c8b1b8d415b467c2ae2d0fa9652ad3c3d980c744922019e721264833fee951a0c63cf2bbd0e1e8ad0a458ccc"
        "3b6ace58dd39360c2c8c26ee571b856b2cf7d867c692f4dba00fa1e3d927cde64f6e78d7a021e799f7946ffd20c56b0b1f5b111ad5ff8c"
        "d6be2503af8344ea3bde3e944f3a0795deed71e09aa940ed19ba37fcca16f942");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecurityChecksFailed); // invalid SN / request Id
    ck_assert(SOPC_STATUS_OK == status);

    /* Check the PKI validate function was called during OPN treatment */
    ck_assert(true == SOPC_Atomic_Int_Get(&pkiValidationAsked));
    SOPC_Atomic_Int_Set(&pkiValidationAsked, false);
}
END_TEST

START_TEST(test_invalid_msg_typ)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate invalid message type response received\n");

    // Simulate MIL / F / size 28 / SC id = 3321932765 <=> ddab00c6 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d494c461c000000ddab00c60123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST
/*
START_TEST(test_unexpected_multi_chunks_final_value)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    // Only since we are not managing multi chunk messages
    printf("SC_Rcv_Buffer: Simulate unexpected intermediate chunk message received\n");

    // Simulate MSG / C / size 28 / SC id = 3321932765 <=> ddab00c6 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347431c000000ddab00c60123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_abort_chunk_final_value)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate unexpected abort chunk message received\n");

    // Simulate MSG / A / size 28 / SC id = 3321932765 <=> ddab00c6 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347411c000000ddab00c60123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST
*/
START_TEST(test_invalid_final_value)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate invalid isFinal value received\n");

    // Simulate MSG / M / size 28 / SC id = 3321932765 <=> ddab00c6 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d53474d1c000000ddab00c60123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTypeInvalid);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_too_large_msg_size)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    uint8_t byte = 0;
    uint32_t ui32 = 0;

    printf("SC_Rcv_Buffer: Simulate too large message size indicated (without associated content)\n");

    // Simulate MSG / F / size SOPC_TCP_UA_MAX_BUFFER_SIZE + 1 / SC id = 833084066 received on socket
    SOPC_Buffer* buffer = SOPC_Buffer_Create(1000);
    ck_assert(buffer != NULL);
    byte = 'M';
    status = SOPC_Byte_Write(&byte, buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    byte = 'S';
    status = SOPC_Byte_Write(&byte, buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    byte = 'G';
    status = SOPC_Byte_Write(&byte, buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    byte = 'F';
    status = SOPC_Byte_Write(&byte, buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    ui32 = SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE + 1;
    status = SOPC_UInt32_Write(&ui32, buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    ui32 = 833084066;
    status = SOPC_UInt32_Write(&ui32, buffer, 0);
    ck_assert(SOPC_STATUS_OK == status);
    status = SOPC_Buffer_SetDataLength(buffer, 500);
    ck_assert(SOPC_STATUS_OK == status);

    SOPC_EventHandler_Post(socketsEventHandler, SOCKET_RCV_BYTES, scConfigIdx, (uintptr_t) buffer, 0);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpMessageTooLarge);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_invalid_sc_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate invalid MSG message response received with an invalid SC ID\n");

    // Simulate MSG / F / size 80 / SC id = 3661027322 received on socket
    status =
        Simulate_Received_Message(scConfigIdx,
                                  "4d53474650000000fad736da0ad42ebb94bd33035a5b35c9a16e46557fc4eaea47d763781be2ac806e1c"
                                  "5d2e98d3c7e986287a2b3dd194cd0f5f768bbe4e704d8a8fd343d589728a8e847df679545114");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadTcpSecureChannelUnknown);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_invalid_sc_token_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf("SC_Rcv_Buffer: Simulate invalid MSG message response received with an invalid SC TOKEN ID\n");

    // Simulate MSG / F / size 28 / SC id = 3321932765 <=> ddab00c6 received on socket
    status = Simulate_Received_Message(scConfigIdx, "4d5347461c000000ddab00c60123456789abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecureChannelTokenUnknown);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_encrypted_unaligned_value)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf(
        "SC_Rcv_Buffer: Simulate invalid MSG message response received with an invalid encrypted part (not aligned)\n");

    // Simulate MSG / F / size 28 / SC id = 3321932765 <=> ddab00c6 / token = 190349500 <=> bc80580b received on socket
    // Important note: it is not aligned to be decrypted since non encrypted part size is:
    //  28 - 16 = 12 (%  SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block != 0)
    status = Simulate_Received_Message(scConfigIdx, "4d5347461c000000ddab00c6bc80580b89abcdef0123456789abcdef");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecurityChecksFailed);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

START_TEST(test_unexpected_encrypted_aligned_value)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    printf(
        "SC_Rcv_Buffer: Simulate invalid MSG message response received with an invalid encrypted part (invalid "
        "signature)\n");

    // Simulate MSG / F / size 32 / SC id = 3321932765 <=> ddab00c6 / token = 190349500 <=> bc80580b received on socket
    // Important note: it is aligned to be decrypted since non encrypted part size is:
    //  32 - 16 = 16 (%  SOPC_SecurityPolicy_Basic256Sha256_SymmLen_Block == 0)
    //  => signature is then verified but leads to the same error status than non aligned
    status = Simulate_Received_Message(scConfigIdx, "4d53474620000000ddab00c6bc80580b89abcdef0123456789abcdef01234567");
    ck_assert(SOPC_STATUS_OK == status);

    status = Check_Client_Closed_SC_Helper(OpcUa_BadSecurityChecksFailed);
    ck_assert(SOPC_STATUS_OK == status);
}
END_TEST

static Suite* tests_make_suite_invalid_encrypted_buffers(void)
{
    Suite* s;
    TCase* tc_invalid_buf;

    s = suite_create("SC layer: receive invalid encrypted buffers");
    tc_invalid_buf = tcase_create("Invalid encrypted buffers received");
    tcase_add_checked_fixture(tc_invalid_buf, establishSC, clearToolkit);
    tcase_add_test(tc_invalid_buf, test_unexpected_hel_msg);
    tcase_add_test(tc_invalid_buf, test_unexpected_ack_msg);
    tcase_add_test(tc_invalid_buf, test_unexpected_opn_req_msg_replay);
    tcase_add_test(tc_invalid_buf, test_unexpected_opn_resp_msg_replay);
    tcase_add_test(tc_invalid_buf, test_invalid_msg_typ);
    // tcase_add_test(tc_invalid_buf, test_unexpected_multi_chunks_final_value);
    // tcase_add_test(tc_invalid_buf, test_unexpected_abort_chunk_final_value);
    tcase_add_test(tc_invalid_buf, test_invalid_final_value);
    tcase_add_test(tc_invalid_buf, test_too_large_msg_size);
    tcase_add_test(tc_invalid_buf, test_invalid_sc_id);
    tcase_add_test(tc_invalid_buf, test_invalid_sc_token_id);
    tcase_add_test(tc_invalid_buf, test_unexpected_encrypted_unaligned_value);
    tcase_add_test(tc_invalid_buf, test_unexpected_encrypted_aligned_value);

    suite_add_tcase(s, tc_invalid_buf);

    return s;
}

int main(void)
{
    int number_failed;
    SRunner* sr;

    sr = srunner_create(tests_make_suite_invalid_encrypted_buffers());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
