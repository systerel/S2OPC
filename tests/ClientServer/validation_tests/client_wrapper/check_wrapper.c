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
 * \brief Entry point for client wrapper tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdlib.h>

#include "libs2opc_client.h"
#include "libs2opc_client_config_custom.h"
#include "libs2opc_common_config.h"
#include "libs2opc_request_builder.h"

#include "sopc_helper_askpass.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_mutexes.h"
#include "sopc_pki_stack.h"

#define VALID_ENDPOINT_URL "opc.tcp://localhost:4841"
#define INVALID_ENDPOINT_URL "opc.tcp://localhost:5841"
#define MSG_SECURITY_MODE OpcUa_MessageSecurityMode_None
#define REQ_SECURITY_POLICY SOPC_SecurityPolicy_None

#define SERVER_CERTIFICATE_PATH "./server_public/server_4k_cert.der"
#define CLIENT_CERTIFICATE_PATH "./client_public/client_4k_cert.der"
#define CLIENT_KEY_PATH "./client_private/encrypted_client_4k_key.pem"
#define ENCRYPTED_CLIENT_KEY true

#define INVALID_SERVER_CERTIFICATE_PATH "./innexistent/path/not_a_real_file.der"

// Define number of read values in read request to force multi chunk use in request and response:
// use max buffer size for 1 chunk and encoded size of a ReadValueId / DataValue which is 18 bytes in this test
#define NB_READ_VALUES ((SOPC_DEFAULT_TCP_UA_MAX_BUFFER_SIZE / 18) + 1)

static SOPC_ClientConnection* invalid_scConnection = NULL;
static SOPC_ClientHelper_Subscription* check_subscription_callback = NULL;
static SOPC_Mutex check_counter_mutex;
static SOPC_Condition check_counter_condition;
static SOPC_DataValue check_counter_data_value;

/************************************************/
/*             Client Callback getter           */
/************************************************/
static bool SOPC_GetClientUserKeyPassword(char** outPassword)
{
    ck_assert_ptr_nonnull(outPassword);
    return SOPC_TestHelper_AskPass_FromEnv(outPassword);
}

/************************************************/
/*            Client Callback handler           */
/************************************************/

static void handle_failedConnEvent(SOPC_ClientConnection* config,
                                   SOPC_ClientConnectionEvent event,
                                   SOPC_StatusCode status)
{
    // Shall be called only when bad configuration is given to connection API
    ck_assert_ptr_eq(config, invalid_scConnection);
    ck_assert_int_eq(event, SOPC_ClientConnectionEvent_Disconnected);
    SOPC_UNUSED_ARG(status);
}

static void handle_subscriptionNotification(const SOPC_ClientHelper_Subscription* subscription,
                                            SOPC_StatusCode status,
                                            SOPC_EncodeableType* notificationType,
                                            uint32_t nbNotifElts,
                                            const void* notification,
                                            uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(subscription);
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(notificationType);
    SOPC_UNUSED_ARG(nbNotifElts);
    SOPC_UNUSED_ARG(notification);
    SOPC_UNUSED_ARG(monitoredItemCtxArray);
}

static void handle_subscriptionNotification_datachange(const SOPC_ClientHelper_Subscription* subscription,
                                                       SOPC_StatusCode status,
                                                       SOPC_EncodeableType* notificationType,
                                                       uint32_t nbNotifElts,
                                                       const void* notification,
                                                       uintptr_t* monitoredItemCtxArray)
{
    SOPC_UNUSED_ARG(monitoredItemCtxArray);
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Mutex_Lock(&check_counter_mutex));
    ck_assert_ptr_eq(check_subscription_callback, subscription);
    ck_assert(SOPC_IsGoodStatus(status));
    ck_assert_int_ge(nbNotifElts, 1);
    ck_assert_ptr_nonnull(notification);
    ck_assert_ptr_nonnull(notificationType);
    ck_assert_int_eq(OpcUa_DataChangeNotification_EncodeableType.TypeId, notificationType->TypeId);
    const OpcUa_DataChangeNotification* dataChange = (const OpcUa_DataChangeNotification*) notification;
    ck_assert_int_ge(dataChange->NoOfMonitoredItems, 1);
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_DataValue_Copy(&check_counter_data_value, &dataChange->MonitoredItems[0].Value));

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Mutex_Unlock(&check_counter_mutex));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Condition_SignalAll(&check_counter_condition));
}

/************************************************/
START_TEST(test_wrapper_initialize_clear)
{
    /* simple initialization */
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* double Clear shall not fail*/
    SOPC_ClientConfigHelper_Clear();
    SOPC_ClientConfigHelper_Clear();

    /* double initialization shall fail */
    status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE, status);

    SOPC_ClientConfigHelper_Clear();
}
END_TEST

START_TEST(test_wrapper_create_configuration)
{
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* configuration of a valid endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);

    /* check multiple configurations */
    secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection("Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE,
                                                                      REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection("Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE,
                                                                      REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection("Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE,
                                                                      REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection("Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE,
                                                                      REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection("Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE,
                                                                      REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);

    SOPC_ClientConfigHelper_Clear();

    /* configure a connection without wrapper being initialized */
    secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection("Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE,
                                                                      REQ_SECURITY_POLICY);
    ck_assert_ptr_null(secureConnConfig);
}
END_TEST

START_TEST(test_wrapper_create_connection)
{
    /* simple initialization */
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* configuration of a valid endpoint */
    SOPC_SecureConnection_Config* valid_secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(valid_secureConnConfig);

    /* configuration of an invalid endpoint */
    SOPC_SecureConnection_Config* invalid_secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", INVALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(invalid_secureConnConfig);

    /* connect using a valid configuration */

    SOPC_ClientConnection* valid_scConnection = NULL;
    status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ClientHelper_Disconnect(&valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Related issue https://gitlab.com/systerel/S2OPC/-/issues/1512

    // /* connect multiple times using a valid configuration */
    // SOPC_ClientConnection* scConnection[5] = {NULL, NULL, NULL, NULL, NULL};
    // status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &scConnection[0]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);
    // status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &scConnection[1]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);
    // status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &scConnection[2]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);
    // status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &scConnection[3]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);
    // status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &scConnection[4]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);

    // status = SOPC_ClientHelper_Disconnect(&scConnection[0]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);
    // status = SOPC_ClientHelper_Disconnect(&scConnection[1]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);
    // status = SOPC_ClientHelper_Disconnect(&scConnection[2]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);
    // status = SOPC_ClientHelper_Disconnect(&scConnection[3]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);
    // status = SOPC_ClientHelper_Disconnect(&scConnection[4]);
    // ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* connect using an invalid configuration */
    status = SOPC_ClientHelper_Connect(invalid_secureConnConfig, &handle_failedConnEvent, &invalid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_CLOSED, status);
    ck_assert_ptr_null(invalid_scConnection);

    /* Connect when no failure event callback is set */
    status = SOPC_ClientHelper_Connect(valid_secureConnConfig, NULL, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(valid_scConnection);

    /* Connect when secure connection config is NULL */
    status = SOPC_ClientHelper_Connect(NULL, handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(valid_scConnection);

    /* Connect when secure connection is NULL */
    status = SOPC_ClientHelper_Connect(valid_secureConnConfig, handle_failedConnEvent, (SOPC_ClientConnection**) NULL);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(valid_scConnection);

    SOPC_ClientConfigHelper_Clear();

    /* connect without wrapper being initialized */
    status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE, status);
}
END_TEST

START_TEST(test_wrapper_security_connection)
{
    /* Set certificate when toolkit not initialized */
    ck_assert_uint_eq(SOPC_STATUS_INVALID_STATE, SOPC_ClientConfigHelper_SetKeyCertPairFromPath(
                                                     CLIENT_CERTIFICATE_PATH, CLIENT_KEY_PATH, ENCRYPTED_CLIENT_KEY));

    /* simple initialization */
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Create and set PKI */
    SOPC_PKIProvider* pki = NULL;
    SOPC_PKIPermissive_Create(&pki);
    ck_assert_ptr_nonnull(pki);

    /* Set PKI */
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_ClientConfigHelper_SetPKIprovider(pki));

    /* Create secure connection before setting client Key Cert */
    ck_assert_ptr_null(SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, OpcUa_MessageSecurityMode_SignAndEncrypt, SOPC_SecurityPolicy_Basic256Sha256));

    /* Set invalid parameter callback */
    ck_assert_uint_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(NULL));

    /* Set callback to retrieve password of client key */
    ck_assert_uint_eq(SOPC_STATUS_OK,
                      SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(SOPC_GetClientUserKeyPassword));

    /* Invalid parameters */
    ck_assert_uint_eq(SOPC_STATUS_INVALID_PARAMETERS,
                      SOPC_ClientConfigHelper_SetKeyCertPairFromPath(NULL, CLIENT_KEY_PATH, ENCRYPTED_CLIENT_KEY));
    ck_assert_uint_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_ClientConfigHelper_SetKeyCertPairFromPath(
                                                          CLIENT_CERTIFICATE_PATH, NULL, ENCRYPTED_CLIENT_KEY));

    /* Set client Key Cert pair */
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_ClientConfigHelper_SetKeyCertPairFromPath(
                                          CLIENT_CERTIFICATE_PATH, CLIENT_KEY_PATH, ENCRYPTED_CLIENT_KEY));

    /* configuration of a valid endpoint in sign and encrypt mode */
    SOPC_SecureConnection_Config* valid_secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, OpcUa_MessageSecurityMode_SignAndEncrypt, SOPC_SecurityPolicy_Basic256Sha256);
    ck_assert_ptr_nonnull(valid_secureConnConfig);

    /* configuration of a valid endpoint in sign and encrypt mode */
    SOPC_SecureConnection_Config* invalid_cert_path_secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, OpcUa_MessageSecurityMode_SignAndEncrypt, SOPC_SecurityPolicy_Basic256Sha256);
    ck_assert_ptr_nonnull(invalid_cert_path_secureConnConfig);

    /* Set valid path to server certificate */
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_SecureConnectionConfig_SetServerCertificateFromPath(
                                          valid_secureConnConfig, SERVER_CERTIFICATE_PATH));

    /* Invalid parameters */
    ck_assert_uint_eq(SOPC_STATUS_INVALID_PARAMETERS,
                      SOPC_SecureConnectionConfig_SetServerCertificateFromPath(NULL, SERVER_CERTIFICATE_PATH));
    ck_assert_uint_eq(SOPC_STATUS_INVALID_PARAMETERS,
                      SOPC_SecureConnectionConfig_SetServerCertificateFromPath(valid_secureConnConfig, NULL));

    /* Invalid path */
    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_SecureConnectionConfig_SetServerCertificateFromPath(
                                          invalid_cert_path_secureConnConfig, INVALID_SERVER_CERTIFICATE_PATH));

    /* connect using a valid configuration */
    SOPC_ClientConnection* valid_scConnection = NULL;
    status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    status = SOPC_ClientHelper_Disconnect(&valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* connect using an invalid configuration */
    status =
        SOPC_ClientHelper_Connect(invalid_cert_path_secureConnConfig, &handle_failedConnEvent, &invalid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);
    ck_assert_ptr_null(invalid_scConnection);

    SOPC_ClientConfigHelper_Clear();
}
END_TEST

START_TEST(test_wrapper_disconnect)
{
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* configuration of a valid endpoint */
    SOPC_SecureConnection_Config* valid_secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(valid_secureConnConfig);

    /* connect using a valid configuration */

    SOPC_ClientConnection* valid_scConnection = NULL;
    status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* disconnect a valid endpoint */
    status = SOPC_ClientHelper_Disconnect(&valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* disconnect a NULL handler secure connection */
    status = SOPC_ClientHelper_Disconnect(NULL);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    /* disconnect a secure connection set to NULL  */
    SOPC_ClientConnection* scConnection = NULL;
    status = SOPC_ClientHelper_Disconnect(&scConnection);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    /* disconnect an already closed connection */
    status = SOPC_ClientHelper_Disconnect(&valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    status = SOPC_ClientHelper_Connect(valid_secureConnConfig, &handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_ClientConfigHelper_Clear();

    /* disconnect after wrapper has been closed */
    status = SOPC_ClientHelper_Disconnect(&valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE, status);
}
END_TEST

START_TEST(test_wrapper_create_subscription)
{
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Connecting to a valid endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    SOPC_ClientConnection* valid_scConnection = NULL;
    status = SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* create a valid subscription */
    OpcUa_CreateSubscriptionRequest* subscriptionRequest = SOPC_CreateSubscriptionRequest_CreateDefault();
    ck_assert_ptr_nonnull(subscriptionRequest);
    SOPC_ClientHelper_Subscription* subscription = SOPC_ClientHelper_CreateSubscription(
        valid_scConnection, subscriptionRequest, handle_subscriptionNotification, (uintptr_t) NULL);
    ck_assert_ptr_nonnull(subscription);

    /* Call the subscription a second time */
    subscriptionRequest = SOPC_CreateSubscriptionRequest_CreateDefault();
    SOPC_ClientHelper_Subscription* second_subscription = SOPC_ClientHelper_CreateSubscription(
        valid_scConnection, subscriptionRequest, handle_subscriptionNotification, (uintptr_t) NULL);
    ck_assert_ptr_nonnull(second_subscription);

    /* Unsubscribe and delete the subscription */
    status = SOPC_ClientHelper_DeleteSubscription(&second_subscription);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_ClientHelper_DeleteSubscription(&subscription);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    subscriptionRequest = SOPC_CreateSubscriptionRequest_CreateDefault();
    /* Invalid parameters */
    SOPC_ClientHelper_Subscription* invalid_subscription = SOPC_ClientHelper_CreateSubscription(
        NULL, subscriptionRequest, handle_subscriptionNotification, (uintptr_t) NULL);
    ck_assert_ptr_null(invalid_subscription);

    invalid_subscription = SOPC_ClientHelper_CreateSubscription(valid_scConnection, NULL,
                                                                handle_subscriptionNotification, (uintptr_t) NULL);
    ck_assert_ptr_null(invalid_subscription);

    subscriptionRequest = SOPC_CreateSubscriptionRequest_CreateDefault();
    invalid_subscription =
        SOPC_ClientHelper_CreateSubscription(valid_scConnection, subscriptionRequest, NULL, (uintptr_t) NULL);
    ck_assert_ptr_null(invalid_subscription);

    /* Disconnect */
    status = SOPC_ClientHelper_Disconnect(&valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    // Create a subscription after disconnect
    subscriptionRequest = SOPC_CreateSubscriptionRequest_CreateDefault();
    ck_assert_ptr_nonnull(subscriptionRequest);

    subscription = SOPC_ClientHelper_CreateSubscription(valid_scConnection, subscriptionRequest,
                                                        handle_subscriptionNotification, (uintptr_t) NULL);
    ck_assert_ptr_null(subscription);

    status = SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_ClientConfigHelper_Clear();

    subscriptionRequest = SOPC_CreateSubscriptionRequest_CreateDefault();
    ck_assert_ptr_nonnull(subscriptionRequest);
    subscription = SOPC_ClientHelper_CreateSubscription(valid_scConnection, subscriptionRequest,
                                                        handle_subscriptionNotification, (uintptr_t) NULL);
    ck_assert_ptr_null(subscription);
}
END_TEST

START_TEST(test_wrapper_add_monitored_items)
{
    /* array of node ids */
    char* nodeIds1[1] = {"ns=0;s=Counter"}; // value increments itself
    const size_t nbMonitoredItems1 = 1;
    uint32_t monitoredItemsIds1[1];
    char* nodeIds2[1] = {"ns=0;i=1013"};
    const size_t nbMonitoredItems2 = 1;
    uint32_t monitoredItemsIds2[1];
    char* nodeIds3[3] = {"ns=0;i=1009", "ns=0;i=1011", "ns=0;i=1001"};
    const size_t nbMonitoredItems3 = 3;
    uint32_t monitoredItemsIds3[3];
    char* nodeIds3_plus_unknown[4] = {"ns=0;i=1009", "ns=0;i=1011", "ns=0;i=1001", "ns=1;s=Invalid_NodeId"};
    const size_t nbMonitoredItems3_plus_unknown = 4;
    uint32_t monitoredItemsIds3_plus_unknown[4];
    char* nodeIds2_plus_2_unknown[4] = {"ns=1;s=Invalid_NodeId", "ns=0;i=1009", "ns=0;i=1011", "ns=1;s=Invalid_NodeId"};
    const size_t nbMonitoredItems2_plus_2_unknown = 4;
    uint32_t monitoredItemsIds2_plus_2_unknown[4];
    uint32_t subscriptionId = 0; // Let the toolkit select the subscriptionId automaticcaly

    OpcUa_CreateMonitoredItemsResponse response;
    OpcUa_CreateMonitoredItemsResponse_Initialize(&response);

    // Initialize toolkit
    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* Connecting to a valid endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    SOPC_ClientConnection* valid_scConnection = NULL;
    status = SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* create a valid subscription */
    OpcUa_CreateSubscriptionRequest* subscriptionRequest = SOPC_CreateSubscriptionRequest_CreateDefault();
    ck_assert_ptr_nonnull(subscriptionRequest);

    SOPC_ClientHelper_Subscription* subscription = SOPC_ClientHelper_CreateSubscription(
        valid_scConnection, subscriptionRequest, handle_subscriptionNotification, (uintptr_t) NULL);
    ck_assert_ptr_nonnull(subscription);

    /* Create one monitored item */
    OpcUa_CreateMonitoredItemsRequest* monitoredItemsRequest =
        SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(subscriptionId, nbMonitoredItems1, nodeIds1,
                                                                  OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(monitoredItemsRequest);

    /* Invalid Parameters */
    status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(NULL, monitoredItemsRequest, (const uintptr_t*) NULL,
                                                                 (OpcUa_CreateMonitoredItemsResponse*) &response);
    ck_assert_uint_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(subscription, NULL, (const uintptr_t*) NULL,
                                                                 (OpcUa_CreateMonitoredItemsResponse*) &response);
    ck_assert_uint_eq(SOPC_STATUS_INVALID_PARAMETERS, status);

    /* Create subscription monitored items */
    monitoredItemsRequest = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
        subscriptionId, nbMonitoredItems1, nodeIds1, OpcUa_TimestampsToReturn_Both);
    status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(
        subscription, monitoredItemsRequest, (const uintptr_t*) NULL, (OpcUa_CreateMonitoredItemsResponse*) &response);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems1, response.NoOfResults);
    ck_assert_ptr_nonnull(response.Results);
    ck_assert(SOPC_IsGoodStatus(response.Results[0].StatusCode));
    monitoredItemsIds1[0] = response.Results[0].MonitoredItemId;
    OpcUa_CreateMonitoredItemsResponse_Clear(&response);

    /* add one more monitored item */
    monitoredItemsRequest = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
        subscriptionId, nbMonitoredItems2, nodeIds2, OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(monitoredItemsRequest);

    OpcUa_CreateMonitoredItemsResponse_Initialize(&response);
    status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(
        subscription, monitoredItemsRequest, (const uintptr_t*) NULL, (OpcUa_CreateMonitoredItemsResponse*) &response);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems2, response.NoOfResults);
    ck_assert_ptr_nonnull(response.Results);
    ck_assert(SOPC_IsGoodStatus(response.Results[0].StatusCode));
    monitoredItemsIds2[0] = response.Results[0].MonitoredItemId;
    OpcUa_CreateMonitoredItemsResponse_Clear(&response);

    /* add multiple monitored items */
    monitoredItemsRequest = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
        subscriptionId, nbMonitoredItems3, nodeIds3, OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(monitoredItemsRequest);

    OpcUa_CreateMonitoredItemsResponse_Initialize(&response);
    status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(
        subscription, monitoredItemsRequest, (const uintptr_t*) NULL, (OpcUa_CreateMonitoredItemsResponse*) &response);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems3, response.NoOfResults);
    ck_assert_ptr_nonnull(response.Results);
    ck_assert(SOPC_IsGoodStatus(response.Results[0].StatusCode));
    ck_assert(SOPC_IsGoodStatus(response.Results[1].StatusCode));
    ck_assert(SOPC_IsGoodStatus(response.Results[2].StatusCode));
    monitoredItemsIds3[0] = response.Results[0].MonitoredItemId;
    monitoredItemsIds3[1] = response.Results[1].MonitoredItemId;
    monitoredItemsIds3[2] = response.Results[2].MonitoredItemId;
    OpcUa_CreateMonitoredItemsResponse_Clear(&response);

    /* add multiple monitored items with 1 unkown node id*/
    monitoredItemsRequest = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
        subscriptionId, nbMonitoredItems3_plus_unknown, nodeIds3_plus_unknown, OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(monitoredItemsRequest);

    OpcUa_CreateMonitoredItemsResponse_Initialize(&response);
    status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(
        subscription, monitoredItemsRequest, (const uintptr_t*) NULL, (OpcUa_CreateMonitoredItemsResponse*) &response);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems3_plus_unknown, response.NoOfResults);
    ck_assert_ptr_nonnull(response.Results);
    ck_assert(SOPC_IsGoodStatus(response.Results[0].StatusCode));
    ck_assert(SOPC_IsGoodStatus(response.Results[1].StatusCode));
    ck_assert(SOPC_IsGoodStatus(response.Results[2].StatusCode));
    ck_assert(!SOPC_IsGoodStatus(response.Results[3].StatusCode));
    monitoredItemsIds3_plus_unknown[0] = response.Results[0].MonitoredItemId;
    monitoredItemsIds3_plus_unknown[1] = response.Results[1].MonitoredItemId;
    monitoredItemsIds3_plus_unknown[2] = response.Results[2].MonitoredItemId;
    monitoredItemsIds3_plus_unknown[3] = response.Results[3].MonitoredItemId;
    OpcUa_CreateMonitoredItemsResponse_Clear(&response);

    /* add multiple monitored items with 2 unkown node id*/
    monitoredItemsRequest = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(
        subscriptionId, nbMonitoredItems2_plus_2_unknown, nodeIds2_plus_2_unknown, OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(monitoredItemsRequest);

    OpcUa_CreateMonitoredItemsResponse_Initialize(&response);
    status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(
        subscription, monitoredItemsRequest, (const uintptr_t*) NULL, (OpcUa_CreateMonitoredItemsResponse*) &response);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems2_plus_2_unknown, response.NoOfResults);
    ck_assert_ptr_nonnull(response.Results);
    ck_assert(!SOPC_IsGoodStatus(response.Results[0].StatusCode));
    ck_assert(SOPC_IsGoodStatus(response.Results[1].StatusCode));
    ck_assert(SOPC_IsGoodStatus(response.Results[2].StatusCode));
    ck_assert(!SOPC_IsGoodStatus(response.Results[3].StatusCode));
    monitoredItemsIds2_plus_2_unknown[0] = response.Results[0].MonitoredItemId;
    monitoredItemsIds2_plus_2_unknown[1] = response.Results[1].MonitoredItemId;
    monitoredItemsIds2_plus_2_unknown[2] = response.Results[2].MonitoredItemId;
    monitoredItemsIds2_plus_2_unknown[3] = response.Results[3].MonitoredItemId;
    OpcUa_CreateMonitoredItemsResponse_Clear(&response);

    /* Delete Monitored Item */
    OpcUa_DeleteMonitoredItemsResponse deleteMIresp;
    OpcUa_DeleteMonitoredItemsResponse_Initialize(&deleteMIresp);
    OpcUa_DeleteMonitoredItemsRequest* deleteMIreq =
        SOPC_DeleteMonitoredItemsRequest_Create(0, nbMonitoredItems1, (const uint32_t*) monitoredItemsIds1);
    ck_assert_ptr_nonnull(deleteMIreq);

    status = SOPC_ClientHelper_Subscription_DeleteMonitoredItems(subscription, deleteMIreq, &deleteMIresp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems1, deleteMIresp.NoOfResults);
    ck_assert_ptr_nonnull(deleteMIresp.Results);
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[0]));
    OpcUa_DeleteMonitoredItemsResponse_Clear(&deleteMIresp);

    /* Delete Monitored Item */
    OpcUa_DeleteMonitoredItemsResponse_Initialize(&deleteMIresp);
    deleteMIreq = SOPC_DeleteMonitoredItemsRequest_Create(0, nbMonitoredItems2, (const uint32_t*) monitoredItemsIds2);
    ck_assert_ptr_nonnull(deleteMIreq);

    status = SOPC_ClientHelper_Subscription_DeleteMonitoredItems(subscription, deleteMIreq, &deleteMIresp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems2, deleteMIresp.NoOfResults);
    ck_assert_ptr_nonnull(deleteMIresp.Results);
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[0]));
    OpcUa_DeleteMonitoredItemsResponse_Clear(&deleteMIresp);

    /* Delete Monitored Item */
    OpcUa_DeleteMonitoredItemsResponse_Initialize(&deleteMIresp);
    deleteMIreq = SOPC_DeleteMonitoredItemsRequest_Create(0, nbMonitoredItems2_plus_2_unknown,
                                                          (const uint32_t*) monitoredItemsIds2_plus_2_unknown);
    ck_assert_ptr_nonnull(deleteMIreq);

    status = SOPC_ClientHelper_Subscription_DeleteMonitoredItems(subscription, deleteMIreq, &deleteMIresp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems2_plus_2_unknown, deleteMIresp.NoOfResults);
    ck_assert_ptr_nonnull(deleteMIresp.Results);
    ck_assert(!SOPC_IsGoodStatus(deleteMIresp.Results[0]));
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[1]));
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[2]));
    ck_assert(!SOPC_IsGoodStatus(deleteMIresp.Results[3]));
    OpcUa_DeleteMonitoredItemsResponse_Clear(&deleteMIresp);

    /* Delete Monitored Item */
    OpcUa_DeleteMonitoredItemsResponse_Initialize(&deleteMIresp);
    deleteMIreq = SOPC_DeleteMonitoredItemsRequest_Create(0, nbMonitoredItems3, (const uint32_t*) monitoredItemsIds3);
    ck_assert_ptr_nonnull(deleteMIreq);

    status = SOPC_ClientHelper_Subscription_DeleteMonitoredItems(subscription, deleteMIreq, &deleteMIresp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems3, deleteMIresp.NoOfResults);
    ck_assert_ptr_nonnull(deleteMIresp.Results);
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[0]));
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[1]));
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[2]));
    OpcUa_DeleteMonitoredItemsResponse_Clear(&deleteMIresp);

    /* Delete Monitored Item */
    OpcUa_DeleteMonitoredItemsResponse_Initialize(&deleteMIresp);
    deleteMIreq = SOPC_DeleteMonitoredItemsRequest_Create(0, nbMonitoredItems3_plus_unknown,
                                                          (const uint32_t*) monitoredItemsIds3_plus_unknown);
    ck_assert_ptr_nonnull(deleteMIreq);

    status = SOPC_ClientHelper_Subscription_DeleteMonitoredItems(subscription, deleteMIreq, &deleteMIresp);
    ck_assert_int_eq((int32_t) nbMonitoredItems3_plus_unknown, deleteMIresp.NoOfResults);
    ck_assert_ptr_nonnull(deleteMIresp.Results);
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[0]));
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[1]));
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[2]));
    ck_assert(!SOPC_IsGoodStatus(deleteMIresp.Results[3]));
    OpcUa_DeleteMonitoredItemsResponse_Clear(&deleteMIresp);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_DeleteSubscription(&subscription));

    status = SOPC_ClientHelper_Disconnect(&valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    SOPC_ClientConfigHelper_Clear();
}
END_TEST

START_TEST(test_wrapper_add_monitored_items_callback_called)
{
    char* nodeIds[1] = {"ns=0;s=Counter"};
    const size_t nbMonitoredItems1 = 1;
    const uint32_t subscriptionId = 0; // Let the toolkit select the subscriptionId automaticcaly
    uint32_t monitoredItemsIds1[1];
    OpcUa_CreateMonitoredItemsResponse response;
    OpcUa_CreateMonitoredItemsResponse_Initialize(&response);

    SOPC_ReturnStatus status = SOPC_ClientConfigHelper_Initialize();
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Mutex_Initialization(&check_counter_mutex));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Condition_Init(&check_counter_condition));

    /* Connecting to a valid endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    SOPC_ClientConnection* valid_scConnection = NULL;
    status = SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &valid_scConnection);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    /* create a valid subscription */
    OpcUa_CreateSubscriptionRequest* subRequest = SOPC_CreateSubscriptionRequest_CreateDefault();
    ck_assert_ptr_nonnull(subRequest);

    /* create a subscription */
    check_subscription_callback = SOPC_ClientHelper_CreateSubscription(
        valid_scConnection, subRequest, handle_subscriptionNotification_datachange, (uintptr_t) NULL);
    ck_assert_ptr_nonnull(check_subscription_callback);

    /* Create one monitored item */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Mutex_Lock(&check_counter_mutex));
    OpcUa_CreateMonitoredItemsRequest* monitoredItemsRequest =
        SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(subscriptionId, nbMonitoredItems1, nodeIds,
                                                                  OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(monitoredItemsRequest);

    status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(check_subscription_callback, monitoredItemsRequest,
                                                                 (const uintptr_t*) NULL,
                                                                 (OpcUa_CreateMonitoredItemsResponse*) &response);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq((int32_t) nbMonitoredItems1, response.NoOfResults);
    ck_assert_ptr_nonnull(response.Results);
    ck_assert(SOPC_IsGoodStatus(response.Results[0].StatusCode));
    monitoredItemsIds1[0] = response.Results[0].MonitoredItemId;
    OpcUa_CreateMonitoredItemsResponse_Clear(&response);

    /* verify that callback is called correctly */
    /* use a mutex and a condition to wait until datachange has been received (use a 1.2 sec timeout)*/
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_Mutex_UnlockAndTimedWaitCond(&check_counter_condition, &check_counter_mutex, 1200));

    /* check datavalue */
    ck_assert_int_eq(SOPC_UInt64_Id, check_counter_data_value.Value.BuiltInTypeId);
    uint64_t first_value = check_counter_data_value.Value.Value.Uint64;

    /* reset global values */
    check_counter_data_value.Value.BuiltInTypeId = SOPC_Int16_Id;

    /* verify that callback is called correctly once again*/
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_Mutex_UnlockAndTimedWaitCond(&check_counter_condition, &check_counter_mutex, 1200));
    /* verify datachange callback arguments again */
    /* check connection id */
    /* check datavalue */
    ck_assert_int_eq(SOPC_UInt64_Id, check_counter_data_value.Value.BuiltInTypeId);
    ck_assert_uint_ne(first_value, check_counter_data_value.Value.Value.Uint64);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_Mutex_Unlock(&check_counter_mutex));

    /* Delete Monitored Item */
    OpcUa_DeleteMonitoredItemsResponse deleteMIresp;
    OpcUa_DeleteMonitoredItemsResponse_Initialize(&deleteMIresp);
    OpcUa_DeleteMonitoredItemsRequest* deleteMIreq =
        SOPC_DeleteMonitoredItemsRequest_Create(0, nbMonitoredItems1, (const uint32_t*) monitoredItemsIds1);
    ck_assert_ptr_nonnull(deleteMIreq);
    status =
        SOPC_ClientHelper_Subscription_DeleteMonitoredItems(check_subscription_callback, deleteMIreq, &deleteMIresp);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    ck_assert_int_eq(deleteMIresp.NoOfResults, (int32_t) nbMonitoredItems1);
    ck_assert(SOPC_IsGoodStatus(deleteMIresp.Results[0]));
    OpcUa_DeleteMonitoredItemsResponse_Clear(&deleteMIresp);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_DeleteSubscription(&check_subscription_callback));

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_Disconnect(&valid_scConnection));

    SOPC_ClientConfigHelper_Clear();
}
END_TEST

START_TEST(test_wrapper_unsubscribe)
{
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientConfigHelper_Initialize());

    /* Invalid parameters */
    SOPC_ClientHelper_Subscription* subscription = NULL;
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_ClientHelper_DeleteSubscription(NULL));
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_ClientHelper_DeleteSubscription(&subscription));

    /* Connecting to a valid endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    SOPC_ClientConnection* scConnection = NULL;
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &scConnection));

    /* create a valid subscription */
    OpcUa_CreateSubscriptionRequest* subscriptionRequest = SOPC_CreateSubscriptionRequest_CreateDefault();
    ck_assert_ptr_nonnull(subscriptionRequest);

    subscription = SOPC_ClientHelper_CreateSubscription(scConnection, subscriptionRequest,
                                                        handle_subscriptionNotification, (uintptr_t) NULL);
    ck_assert_ptr_nonnull(subscription);

    /* Delete subscription */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_DeleteSubscription(&subscription));
    ck_assert_ptr_null(subscription);

    /* Delete once again */
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_ClientHelper_DeleteSubscription(&subscription));
    ck_assert_ptr_null(subscription);

    // Cannot create a subscription after deleting one before. https://gitlab.com/systerel/S2OPC/-/issues/1513
    // /* create subscription again */
    // subscription = SOPC_ClientHelper_CreateSubscription(scConnection, subscriptionRequest,
    //                                                        handle_subscriptionNotification, (uintptr_t) NULL);
    // ck_assert_ptr_nonnull(subscription);

    /* disconnect */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_Disconnect(&scConnection));

    /* Unsubscribe after disconnect */
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_ClientHelper_DeleteSubscription(&subscription));

    SOPC_ClientConfigHelper_Clear();

    /* delete subscription after toolkit is closed*/
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS, SOPC_ClientHelper_DeleteSubscription(&subscription));
}
END_TEST

START_TEST(test_wrapper_read)
{
    char* nodeIds1[1] = {"ns=0;s=Counter"};
    size_t numberNodesToRead1 = 1;
    char* nodeIds2[3] = {"ns=0;s=Counter", "ns=0;i=1003", "ns=0;i=1002"};
    size_t numberNodesToRead2 = 3;
    char* nodeIds3[2] = {"ns=0;s=CounterThatShouldNotExist", "ns=0;i=1003"};
    size_t numberNodesToRead3 = 2;

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientConfigHelper_Initialize());

    /* Connecting to a valid endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    SOPC_ClientConnection* scConnection = NULL;
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &scConnection));

    /* Create a read request for one value */
    OpcUa_ReadRequest* readRequest = SOPC_ReadRequest_Create(numberNodesToRead1, OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(readRequest);

    OpcUa_ReadResponse* response = NULL;

    /* Invalid parameters */
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS,
                     SOPC_ClientHelper_ServiceSync(NULL, readRequest, (void**) &response));
    ck_assert_ptr_null(response);

    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS,
                     SOPC_ClientHelper_ServiceSync(scConnection, NULL, (void**) &response));

    readRequest = SOPC_ReadRequest_Create(numberNodesToRead1, OpcUa_TimestampsToReturn_Both);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS,
                     SOPC_ClientHelper_ServiceSync(scConnection, readRequest, (void**) NULL));

    /* read one node */
    readRequest = SOPC_ReadRequest_Create(numberNodesToRead1, OpcUa_TimestampsToReturn_Both);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ReadRequest_SetReadValueFromStrings(readRequest, 0, nodeIds1[0],
                                                                              SOPC_AttributeId_Value, NULL));

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, readRequest, (void**) &response));
    ck_assert_ptr_nonnull(response);
    ck_assert_int_eq((int32_t) numberNodesToRead1, response->NoOfResults);
    ck_assert(SOPC_IsGoodStatus(response->Results[0].Status));
    ck_assert_uint_ne(0, response->Results[0].Value.Value.Uint64);

    /* free results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    /* Create a read request for multiple values */
    readRequest = SOPC_ReadRequest_Create(numberNodesToRead2, OpcUa_TimestampsToReturn_Both);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ReadRequest_SetReadValueFromStrings(readRequest, 0, nodeIds2[0],
                                                                              SOPC_AttributeId_Value, NULL));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ReadRequest_SetReadValueFromStrings(readRequest, 1, nodeIds2[1],
                                                                              SOPC_AttributeId_Value, NULL));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ReadRequest_SetReadValueFromStrings(readRequest, 2, nodeIds2[2],
                                                                              SOPC_AttributeId_Value, NULL));

    /* read multiple nodes */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, readRequest, (void**) &response));
    ck_assert_ptr_nonnull(response);
    ck_assert_int_eq((int32_t) numberNodesToRead2, response->NoOfResults);
    ck_assert(SOPC_IsGoodStatus(response->Results[0].Status));
    ck_assert(SOPC_IsGoodStatus(response->Results[1].Status));
    ck_assert(SOPC_IsGoodStatus(response->Results[2].Status));
    ck_assert_uint_ne(0, response->Results[0].Value.Value.Uint64);
    ck_assert_double_eq(2, response->Results[1].Value.Value.Doublev);
    ck_assert_uint_eq(1000, response->Results[2].Value.Value.Uint32);

    /* free results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    /* read mix of invalid nodes and valid nodes */
    readRequest = SOPC_ReadRequest_Create(numberNodesToRead3, OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(readRequest);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ReadRequest_SetReadValueFromStrings(readRequest, 0, nodeIds3[0],
                                                                              SOPC_AttributeId_Value, NULL));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ReadRequest_SetReadValueFromStrings(readRequest, 1, nodeIds3[1],
                                                                              SOPC_AttributeId_Value, NULL));

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, readRequest, (void**) &response));
    ck_assert_ptr_nonnull(response);
    ck_assert_int_eq((int32_t) numberNodesToRead3, response->NoOfResults);
    ck_assert(!SOPC_IsGoodStatus(response->Results[0].Status));
    ck_assert(SOPC_IsGoodStatus(response->Results[1].Status));
    ck_assert_double_eq(2, response->Results[1].Value.Value.Doublev);

    /* free results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    /* read multiple nodes */
    readRequest = SOPC_ReadRequest_Create(NB_READ_VALUES, OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(readRequest);

    for (size_t i = 0; i < NB_READ_VALUES; i++)
    {
        ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ReadRequest_SetReadValueFromStrings(readRequest, i, "ns=0;s=Counter",
                                                                                  SOPC_AttributeId_Value, NULL));
    }

    ck_assert_uint_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, readRequest, (void**) &response));

    ck_assert_ptr_nonnull(response);
    ck_assert_int_ge(response->NoOfResults, 0);
    ck_assert_uint_eq(NB_READ_VALUES, (uint32_t) response->NoOfResults);
    for (size_t i = 0; i < NB_READ_VALUES; i++)
    {
        ck_assert(SOPC_IsGoodStatus(response->Results[i].Status));
        ck_assert_uint_ne(0, response->Results[i].Value.Value.Uint64);
    }

    /* free results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    /* Disconnect */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_Disconnect(&scConnection));

    /* read node after disconnect */
    readRequest = SOPC_ReadRequest_Create(numberNodesToRead2, OpcUa_TimestampsToReturn_Both);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS,
                     SOPC_ClientHelper_ServiceSync(scConnection, readRequest, (void**) &response));

    ck_assert_uint_eq(SOPC_STATUS_OK,
                      SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &scConnection));

    SOPC_ClientConfigHelper_Clear();

    /* read node after toolkit is closed */
    readRequest = SOPC_ReadRequest_Create(numberNodesToRead2, OpcUa_TimestampsToReturn_Both);
    ck_assert_ptr_nonnull(readRequest);
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE,
                     SOPC_ClientHelper_ServiceSync(scConnection, readRequest, (void**) &response));
}
END_TEST

START_TEST(test_wrapper_write)
{
    char* nodeIds1[1] = {"ns=0;i=1001"};
    size_t numberNodesToWrite1 = 1;

    char* nodeIds2[2] = {"ns=0;i=1001", "ns=0;i=1009"};
    size_t numberNodesToWrite2 = 2;

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientConfigHelper_Initialize());

    /* Connecting to a valid endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    SOPC_ClientConnection* scConnection = NULL;
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &scConnection));

    /* Create a write request for one value */
    OpcUa_WriteRequest* writeRequest = SOPC_WriteRequest_Create(numberNodesToWrite1);
    ck_assert_ptr_nonnull(writeRequest);

    OpcUa_WriteResponse* response = NULL;

    /* Fill data value */
    SOPC_DataValue value;
    SOPC_DataValue_Initialize(&value);
    value.Value.BuiltInTypeId = SOPC_Int64_Id;
    value.Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    value.Value.Value.Int64 = -500;

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_WriteRequest_SetWriteValueFromStrings(writeRequest, 0, nodeIds1[0],
                                                                                SOPC_AttributeId_Value, NULL, &value));

    /* write one node */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, writeRequest, (void**) &response));
    ck_assert_ptr_nonnull(response);
    ck_assert_int_eq((int32_t) numberNodesToWrite1, response->NoOfResults);
    ck_assert(SOPC_IsGoodStatus(response->Results[0]));

    /* free results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    /* Create a write request for multiple values */
    writeRequest = SOPC_WriteRequest_Create(numberNodesToWrite2);
    ck_assert_ptr_nonnull(writeRequest);

    /* Fill data value */
    SOPC_DataValue values[2];
    SOPC_DataValue* val1 = &values[0];
    SOPC_DataValue_Initialize(val1);
    val1->Value.BuiltInTypeId = SOPC_Int64_Id;
    val1->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    val1->Value.Value.Int64 = -256;

    SOPC_DataValue* val2 = &values[1];
    SOPC_DataValue_Initialize(val2);
    val2->Value.BuiltInTypeId = SOPC_Int64_Id;
    val2->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
    val2->Value.Value.Int64 = -12;

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_WriteRequest_SetWriteValueFromStrings(writeRequest, 0, nodeIds2[0],
                                                                                SOPC_AttributeId_Value, NULL, val1));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_WriteRequest_SetWriteValueFromStrings(writeRequest, 1, nodeIds2[1],
                                                                                SOPC_AttributeId_Value, NULL, val2));

    /* write multiple nodes */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, writeRequest, (void**) &response));
    ck_assert_ptr_nonnull(response);
    ck_assert_int_eq((int32_t) numberNodesToWrite2, response->NoOfResults);
    ck_assert(SOPC_IsGoodStatus(response->Results[0]));
    ck_assert(SOPC_IsGoodStatus(response->Results[1]));

    /* free results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    /* Disconnect */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_Disconnect(&scConnection));

    /* write node after disconnect */
    writeRequest = SOPC_WriteRequest_Create(numberNodesToWrite2);
    ck_assert_int_eq(SOPC_STATUS_INVALID_PARAMETERS,
                     SOPC_ClientHelper_ServiceSync(scConnection, writeRequest, (void**) &response));

    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &scConnection));

    SOPC_ClientConfigHelper_Clear();

    /* write node after toolkit is closed */
    writeRequest = SOPC_WriteRequest_Create(numberNodesToWrite2);
    ck_assert_ptr_nonnull(writeRequest);
    ck_assert_int_eq(SOPC_STATUS_INVALID_STATE,
                     SOPC_ClientHelper_ServiceSync(scConnection, writeRequest, (void**) &response));
}
END_TEST

START_TEST(test_wrapper_browse)
{
    char* nodeIds1[1] = {"ns=0;i=84"};
    size_t numberNodesToBrowse1 = 1;
    char* nodeIds2[2] = {"ns=0;i=84", "ns=0;i=85"};
    size_t numberNodesToBrowse2 = 2;

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientConfigHelper_Initialize());

    /* Connecting to a valid endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    SOPC_ClientConnection* scConnection = NULL;
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &scConnection));

    OpcUa_BrowseRequest* browseRequest = SOPC_BrowseRequest_Create(numberNodesToBrowse1, 0, NULL);
    ck_assert_ptr_nonnull(browseRequest);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(
                                         browseRequest, 0, nodeIds1[0], OpcUa_BrowseDirection_Forward, "ns=0;i=33",
                                         true, 0, OpcUa_BrowseResultMask_All));
    OpcUa_BrowseResponse* response = NULL;

    /* Browse */
    /*  Root/ - Hierarchical references */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, browseRequest, (void**) &response));
    ck_assert_int_eq((int32_t) numberNodesToBrowse1, response->NoOfResults);
    ck_assert_ptr_nonnull(response->Results);
    ck_assert_int_eq(3, response->Results[0].NoOfReferences);
    ck_assert(SOPC_IsGoodStatus(response->Results[0].StatusCode));

    ck_assert_ptr_nonnull(response->Results[0].References);
    ck_assert(response->Results[0].References[0].IsForward);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, response->Results[0].References[0].NodeId.NodeId.IdentifierType);
    ck_assert_int_eq(0, response->Results[0].References[0].NodeId.NodeId.Namespace);
    ck_assert_int_eq(85, response->Results[0].References[0].NodeId.NodeId.Data.Numeric);
    ck_assert_int_eq(1, response->Results[0].References[0].NodeClass);

    ck_assert(response->Results[0].References[1].IsForward);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, response->Results[0].References[1].NodeId.NodeId.IdentifierType);
    ck_assert_int_eq(0, response->Results[0].References[1].NodeId.NodeId.Namespace);
    ck_assert_int_eq(86, response->Results[0].References[1].NodeId.NodeId.Data.Numeric);
    ck_assert_int_eq(1, response->Results[0].References[1].NodeClass);

    ck_assert(response->Results[0].References[2].IsForward);
    ck_assert_int_eq(SOPC_IdentifierType_Numeric, response->Results[0].References[2].NodeId.NodeId.IdentifierType);
    ck_assert_int_eq(0, response->Results[0].References[2].NodeId.NodeId.Namespace);
    ck_assert_int_eq(87, response->Results[0].References[2].NodeId.NodeId.Data.Numeric);
    ck_assert_int_eq(1, response->Results[0].References[2].NodeClass);

    /* free results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    browseRequest = SOPC_BrowseRequest_Create(numberNodesToBrowse2, 0, NULL);
    ck_assert_ptr_nonnull(browseRequest);

    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(
                                         browseRequest, 0, nodeIds2[0], OpcUa_BrowseDirection_Forward, "ns=0;i=33",
                                         true, 0, OpcUa_BrowseResultMask_All));
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(
                                         browseRequest, 1, nodeIds2[1], OpcUa_BrowseDirection_Forward, "ns=0;i=33",
                                         true, 0, OpcUa_BrowseResultMask_All));
    /* Browse */
    /* Root/ and Root/Objects - Hierarchical references */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, browseRequest, (void**) &response));
    ck_assert_int_eq((int32_t) numberNodesToBrowse2, response->NoOfResults);
    ck_assert_ptr_nonnull(response->Results);
    ck_assert(SOPC_IsGoodStatus(response->Results[0].StatusCode));
    ck_assert(SOPC_IsGoodStatus(response->Results[1].StatusCode));
    ck_assert_int_eq(3, response->Results[0].NoOfReferences);
    ck_assert_int_eq(16, response->Results[1].NoOfReferences);
    ck_assert_ptr_nonnull(response->Results[0].References);
    ck_assert_ptr_nonnull(response->Results[1].References);

    /* free results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    /* FreeOPCUA server doesn't support maxReferencePerNode configuration and send one response with all nodes.
    We could update FreeOPCUA stack or change for s2opc server since this is not an interoperable test */

    // char* nodeIds3[1] = {"ns=0;i=7617"};
    // size_t numberNodesToBrowse3 = 1;

    // /* browse too many browse requests */
    // const size_t maxReferencesPerNode = 1;
    // browseRequest = SOPC_BrowseRequest_Create(numberNodesToBrowse3, maxReferencesPerNode, NULL);
    // ck_assert_int_eq(SOPC_STATUS_OK, SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(
    //                                      browseRequest, 0, nodeIds3[0], OpcUa_BrowseDirection_Forward, "ns=0;i=33",
    //                                      true, 0, OpcUa_BrowseResultMask_All));
    // /* Browse */
    // /* Root/ and Root/Objects - Hierarchical references */
    // ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, browseRequest, (void**)
    // &response)); ck_assert_int_eq((int32_t) numberNodesToBrowse3, response->NoOfResults);
    // ck_assert_ptr_nonnull(response->Results);
    // ck_assert(SOPC_IsGoodStatus(response->Results[0].StatusCode));
    // ck_assert_int_eq(1, response->Results[0].NoOfReferences);
    // ck_assert_ptr_nonnull(response->Results[0].References);
    // ck_assert_int_gt(response->Results[0].ContinuationPoint.Length, 0);

    // /* Create Browse Next Request */
    // const size_t nbContinuationPoint = 1;
    // OpcUa_BrowseNextRequest* browseNextRequest = SOPC_BrowseNextRequest_Create(true, nbContinuationPoint);

    // /* Set Continuation Point */
    // ck_assert_int_eq(SOPC_STATUS_OK, SOPC_BrowseNextRequest_SetContinuationPoint(browseNextRequest, 0,
    // &response->Results[0].ContinuationPoint));

    // /* Browse Next */
    // OpcUa_BrowseNextResponse* browseNextResponse = NULL;
    // ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_ServiceSync(scConnection, browseNextRequest,
    // browseNextResponse)); ck_assert_int_eq((int32_t) nbContinuationPoint, response->NoOfResults);
    // ck_assert_ptr_nonnull(response->Results);
    // ck_assert(SOPC_IsGoodStatus(response->Results[0].StatusCode));
    // ck_assert_int_eq(1, response->Results[0].NoOfReferences);
    // ck_assert_ptr_nonnull(response->Results[0].References);
    // ck_assert_int_eq(-1, response->Results[0].ContinuationPoint.Length);
    // ck_assert_ptr_null(response->Results[0].ContinuationPoint.Data);

    // /* free results */
    // SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);
    // SOPC_EncodeableObject_Delete(browseNextResponse->encodeableType, (void**) browseNextResponse);

    /* Disconnect */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_Disconnect(&scConnection));

    SOPC_ClientConfigHelper_Clear();
}
END_TEST

START_TEST(test_wrapper_get_endpoints)
{
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientConfigHelper_Initialize());

    /* Connecting to a valid endpoint */
    SOPC_SecureConnection_Config* secureConnConfig = SOPC_ClientConfigHelper_CreateSecureConnection(
        "Test", VALID_ENDPOINT_URL, MSG_SECURITY_MODE, REQ_SECURITY_POLICY);
    ck_assert_ptr_nonnull(secureConnConfig);
    SOPC_ClientConnection* scConnection = NULL;
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_ClientHelper_Connect(secureConnConfig, &handle_failedConnEvent, &scConnection));

    /* Create valid get endpoint request */
    OpcUa_GetEndpointsRequest* valid_getEndpointRequest = SOPC_GetEndpointsRequest_Create(VALID_ENDPOINT_URL);
    ck_assert_ptr_nonnull(valid_getEndpointRequest);

    /* Create get endpoint request with no filter */
    OpcUa_GetEndpointsRequest* nofilter_getEndpointrequest = SOPC_GetEndpointsRequest_Create(NULL);
    ck_assert_ptr_nonnull(nofilter_getEndpointrequest);
    OpcUa_GetEndpointsResponse* response = NULL;

    /* Get valid Endpoint request */
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_ClientHelper_ServiceSync(scConnection, valid_getEndpointRequest, (void**) &response));
    ck_assert_int_eq(3, response->NoOfEndpoints);
    ck_assert_ptr_nonnull(response->Endpoints);

    /* free Results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    /* Get Enpoint request */
    ck_assert_int_eq(SOPC_STATUS_OK,
                     SOPC_ClientHelper_ServiceSync(scConnection, nofilter_getEndpointrequest, (void**) &response));
    ck_assert_int_eq(3, response->NoOfEndpoints);
    ck_assert_ptr_nonnull(response->Endpoints);

    /* free results */
    SOPC_EncodeableObject_Delete(response->encodeableType, (void**) &response);

    /* Disconnect */
    ck_assert_int_eq(SOPC_STATUS_OK, SOPC_ClientHelper_Disconnect(&scConnection));

    SOPC_ClientConfigHelper_Clear();
}
END_TEST

static void setup(void)
{
    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./check_wrapper_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;
    // Initialize the toolkit library and define the log configuration
    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
}

static void teardown(void)
{
    SOPC_ClientConfigHelper_Clear(); // In case of a test failed force Clearing toolkit
    SOPC_CommonHelper_Clear();
}

static Suite* tests_make_suite_wrapper(void)
{
    Suite* s = NULL;
    TCase* tc_wrapper;

    s = suite_create("Client wrapper library");

    tc_wrapper = tcase_create("WrapperC");
    // Add a teardown to guarantee the call to SOPC_ClientConfigHelper_Clear after each test
    tcase_add_checked_fixture(tc_wrapper, setup, teardown);
    tcase_add_test(tc_wrapper, test_wrapper_initialize_clear);
    tcase_add_test(tc_wrapper, test_wrapper_create_configuration);
    tcase_add_test(tc_wrapper, test_wrapper_create_connection);
    tcase_add_test(tc_wrapper, test_wrapper_security_connection);
    tcase_add_test(tc_wrapper, test_wrapper_disconnect);
    tcase_add_test(tc_wrapper, test_wrapper_create_subscription);
    tcase_add_test(tc_wrapper, test_wrapper_add_monitored_items);
    tcase_add_test(tc_wrapper, test_wrapper_add_monitored_items_callback_called);
    tcase_add_test(tc_wrapper, test_wrapper_unsubscribe);
    tcase_add_test(tc_wrapper, test_wrapper_read);
    tcase_add_test(tc_wrapper, test_wrapper_write);
    tcase_add_test(tc_wrapper, test_wrapper_browse);
    tcase_add_test(tc_wrapper, test_wrapper_get_endpoints);
    tcase_set_timeout(tc_wrapper, 0);
    suite_add_tcase(s, tc_wrapper);

    return s;
}

int main(void)
{
    int number_failed = 0;
    SRunner* sr = NULL;

    sr = srunner_create(tests_make_suite_wrapper());
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
