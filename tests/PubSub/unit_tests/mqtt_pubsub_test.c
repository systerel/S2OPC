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
 * \brief Entry point for tests. Tests use libcheck.
 * https://libcheck.github.io/check/doc/check_html/check_3.html
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdlib.h>

#include "sopc_atomic.h"
#include "sopc_common.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mqtt_transport_layer.h"

static const char* URI_MQTT_BROKER = "127.0.0.1:1883";
static const char* BAD_PORT_URI_MQTT_BROKER = "127.0.0.1:18";
static const char* MQTT_LIB_TOPIC_NAME[MQTT_LIB_MAX_NB_TOPIC_NAME] = {"test1", "test2", "test3"};
#define NB_TOPIC 3
static const char* USERNAME = "user1";
static const char* PASSWORD = "password";
/* Arbitrary network message */
#define ENCODED_DATA_SIZE 35
#define WAIT_CONNECTION 100
#define TRY_IF_CONNECTED 5
uint8_t encoded_network_msg_data[ENCODED_DATA_SIZE] = {0x71, 0x2E, 0x03, 0x2A, 0x00, 0xE8, 0x03, 0x00, 0x00,
                                                       // Payload header/Message Count & DSM WriterIds
                                                       0x01, 0xFF, 0x00,
                                                       // DSM1 (Flags, nbFields)
                                                       0x01, 0x05, 0x00,
                                                       // DSM1 = vars 1..5
                                                       0x07, 0x2E, 0x34, 0xB8, 0x00, 0x03, 0xEF, 0x05, 0x54, 0xFD, 0x0A,
                                                       0x8F, 0xC2, 0xF5, 0x3D, 0x07, 0xBC, 0xA4, 0x05, 0x00};

SOPC_Buffer encoded_network_msg = {ENCODED_DATA_SIZE, ENCODED_DATA_SIZE,       ENCODED_DATA_SIZE, 0,
                                   ENCODED_DATA_SIZE, encoded_network_msg_data};

#define VERBOSE 0 // Use 1 for verbose mode (debug)

#if VERBOSE
#define DEBUG(x) printf x
// This is necessary to use a separate macro to avoid warnings from CLANG
#define VERBOSE_EXPECTED(x, y, remTimeMs, timeout)                                     \
    do                                                                                 \
    {                                                                                  \
        if ((x) == (y))                                                                \
        {                                                                              \
            DEBUG(("'" #x "= " #y "' obtained. Rem time = %d ms\n", (int) remTimeMs)); \
        }                                                                              \
        else                                                                           \
        {                                                                              \
            DEBUG(("'" #x "= " #y "' NOT obtained after %d ms\n", (int) timeout));     \
        }                                                                              \
    } while (0)

static void logCb(const char* category, const char* const line)
{
    DEBUG(("[%s]: %s\n", category ? category : "NONE", line));
}

#else
#define DEBUG(x)
#define VERBOSE_EXPECTED(x, y, remTimeMs, timeout)
#endif

#define MAX_WAIT_MS 2000

#define WAIT_FOR(x, y, timeout)                                              \
    do                                                                       \
    {                                                                        \
        static int32_t remTimeMs = (timeout);                                \
        DEBUG(("Waiting for '" #x " = " #y "' for %d ms\n", (int) timeout)); \
        while ((x) != (y) && remTimeMs >= 100)                               \
        {                                                                    \
            SOPC_Sleep(100);                                                 \
            remTimeMs -= 100;                                                \
        }                                                                    \
        VERBOSE_EXPECTED(x, y, (int) remTimeMs, (int) timeout);              \
    } while (0)

#define WAIT_AND_CHECK_EQ(x, y, timeout)        \
    do                                          \
    {                                           \
        WAIT_FOR(x, y, timeout);                \
        ck_assert_int_eq((int) (x), (int) (y)); \
    } while (0)

static int32_t nbFatalError = 0;
static int32_t nbReceived = 0;
static void onFatalError(void* userContext, const char* message)
{
    SOPC_UNUSED_ARG(userContext);
    SOPC_UNUSED_ARG(message);
    SOPC_Atomic_Int_Add(&nbFatalError, 1);
}

static void cbMessageArrivedTest(uint8_t* data, uint16_t size, void* user)
{
    SOPC_UNUSED_ARG(user);
    ck_assert_int_eq(ENCODED_DATA_SIZE, size);
    for (uint16_t i = 0; i < size; i++)
    {
        ck_assert_uint_eq(encoded_network_msg_data[i], data[i]);
    }
    SOPC_Atomic_Int_Add(&nbReceived, 1);
}

START_TEST(test_create_client)
{
    MqttContextClient* contextClient;
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_MQTT_Release_Client(contextClient);
}
END_TEST

START_TEST(test_difference_topic_nbTopic_fail)
{
    MqttContextClient* contextClient;
    SOPC_Atomic_Int_Set(&nbFatalError, 0);
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_InitializeAndConnect_Client(contextClient, URI_MQTT_BROKER, NULL, NULL, MQTT_LIB_TOPIC_NAME,
                                                   NB_TOPIC + 2, cbMessageArrivedTest, &onFatalError, NULL);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    SOPC_Sleep(500);
    ck_assert_int_eq(0, SOPC_Atomic_Int_Get(&nbFatalError));
    SOPC_MQTT_Release_Client(contextClient);
}
END_TEST

START_TEST(test_username_noPassword_fail)
{
    MqttContextClient* contextClient;
    SOPC_Atomic_Int_Set(&nbFatalError, 0);
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_InitializeAndConnect_Client(contextClient, URI_MQTT_BROKER, "randomUsername", NULL,
                                                   MQTT_LIB_TOPIC_NAME, NB_TOPIC + 2, cbMessageArrivedTest,
                                                   &onFatalError, NULL);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    SOPC_Sleep(500);
    ck_assert_int_eq(SOPC_Atomic_Int_Get(&nbFatalError), 0);
    SOPC_MQTT_Release_Client(contextClient);
}
END_TEST

START_TEST(test_connexion_fail)
{
    MqttContextClient* contextClient;
    SOPC_Atomic_Int_Set(&nbFatalError, 0);
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_InitializeAndConnect_Client(contextClient, BAD_PORT_URI_MQTT_BROKER, NULL, NULL, NULL, 0, NULL,
                                                   &onFatalError, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    WAIT_AND_CHECK_EQ(SOPC_Atomic_Int_Get(&nbFatalError), 1, MAX_WAIT_MS);
    SOPC_MQTT_Release_Client(contextClient);
}
END_TEST

START_TEST(test_publisher_send)
{
    MqttContextClient* contextClient;
    SOPC_Atomic_Int_Set(&nbFatalError, 0);
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_InitializeAndConnect_Client(contextClient, URI_MQTT_BROKER, NULL, NULL, NULL, 0, NULL,
                                                   &onFatalError, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    WAIT_AND_CHECK_EQ(SOPC_MQTT_Client_Is_Connected(contextClient), 1, MAX_WAIT_MS);

    for (int i = 0; i < NB_TOPIC; i++)
    {
        status = SOPC_MQTT_Send_Message(contextClient, MQTT_LIB_TOPIC_NAME[i], encoded_network_msg);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
    }
    SOPC_MQTT_Release_Client(contextClient);
    SOPC_Sleep(500);
    ck_assert_int_eq(SOPC_Atomic_Int_Get(&nbFatalError), 0);
}
END_TEST

START_TEST(test_callback_subscription)
{
    MqttContextClient* contextClient;
    SOPC_Atomic_Int_Set(&nbFatalError, 0);
    SOPC_Atomic_Int_Set(&nbReceived, 0);
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_InitializeAndConnect_Client(contextClient, URI_MQTT_BROKER, NULL, NULL, MQTT_LIB_TOPIC_NAME,
                                                   NB_TOPIC, cbMessageArrivedTest, &onFatalError, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    WAIT_AND_CHECK_EQ(SOPC_MQTT_Client_Is_Connected(contextClient), 1, MAX_WAIT_MS);

    status = SOPC_MQTT_Send_Message(contextClient, MQTT_LIB_TOPIC_NAME[0], encoded_network_msg);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    WAIT_AND_CHECK_EQ(nbReceived > 0, true, MAX_WAIT_MS);
    SOPC_MQTT_Release_Client(contextClient);
    SOPC_Sleep(100);
    ck_assert_int_eq(SOPC_Atomic_Int_Get(&nbFatalError), 0);
}
END_TEST

START_TEST(test_connexion_authentification)
{
    MqttContextClient* contextClient;
    SOPC_Atomic_Int_Set(&nbFatalError, 0);
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_InitializeAndConnect_Client(contextClient, URI_MQTT_BROKER, USERNAME, PASSWORD, NULL, 0, NULL,
                                                   &onFatalError, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);

    WAIT_AND_CHECK_EQ(SOPC_MQTT_Client_Is_Connected(contextClient), 1, MAX_WAIT_MS);

    for (int i = 0; i < NB_TOPIC; i++)
    {
        status = SOPC_MQTT_Send_Message(contextClient, MQTT_LIB_TOPIC_NAME[i], encoded_network_msg);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
    }
    SOPC_MQTT_Release_Client(contextClient);
    SOPC_Sleep(100);
    ck_assert_int_eq(SOPC_Atomic_Int_Get(&nbFatalError), 0);
}
END_TEST

int main(void)
{
    int number_failed;
    Suite* suite = suite_create("MQTT PubSub module test");
    SRunner* sr;

#if VERBOSE
    SOPC_Log_Configuration logCfg;
    logCfg.logSystem = SOPC_LOG_SYSTEM_USER;
    logCfg.logSysConfig.userSystemLogConfig.doLog = &logCb;
    SOPC_Common_Initialize(&logCfg, NULL);
#endif
    TCase* tc_client = tcase_create("Mqtt client creation and initialisation");
    suite_add_tcase(suite, tc_client);
    tcase_add_test(tc_client, test_create_client);
    tcase_add_test(tc_client, test_difference_topic_nbTopic_fail);
    tcase_add_test(tc_client, test_username_noPassword_fail);
    tcase_add_test(tc_client, test_connexion_fail);

    TCase* tc_publisher_client = tcase_create("Mqtt publisher");
    suite_add_tcase(suite, tc_publisher_client);
    tcase_add_test(tc_publisher_client, test_publisher_send);
    tcase_add_test(tc_publisher_client, test_connexion_authentification);

    TCase* tc_subscriber_client = tcase_create("Mqtt subscriber");
    suite_add_tcase(suite, tc_subscriber_client);
    tcase_add_test(tc_subscriber_client, test_callback_subscription);

    sr = srunner_create(suite);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
