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

#include "sopc_macros.h"
#include "sopc_mqtt_transport_layer.h"

static const char* URI_MQTT_BROKER = "127.0.0.1:1883";
static const char* MQTT_LIB_TOPIC_NAME[MQTT_LIB_MAX_NB_TOPIC_NAME] = {"test1", "test2", "test3"};
static const uint16_t NB_TOPIC = 3;

/* Arbitrary network message */
#define ENCODED_DATA_SIZE 35
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

static void cbMessageArrivedTest(uint8_t* data, uint16_t size, void* user)
{
    SOPC_UNUSED_ARG(user);
    ck_assert_int_eq(ENCODED_DATA_SIZE, size);
    for (uint16_t i = 0; i < size; i++)
    {
        ck_assert_uint_eq(encoded_network_msg_data[i], data[i]);
    }
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
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_Initialize_Client(contextClient, URI_MQTT_BROKER, NULL, NULL, MQTT_LIB_TOPIC_NAME, NB_TOPIC + 2,
                                         cbMessageArrivedTest, NULL);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    SOPC_MQTT_Release_Client(contextClient);
}
END_TEST

START_TEST(test_username_noPassword_fail)
{
    MqttContextClient* contextClient;
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_Initialize_Client(contextClient, URI_MQTT_BROKER, "randomUsername", NULL, MQTT_LIB_TOPIC_NAME,
                                         NB_TOPIC + 2, cbMessageArrivedTest, NULL);
    ck_assert_int_eq(SOPC_STATUS_NOK, status);
    SOPC_MQTT_Release_Client(contextClient);
}
END_TEST

START_TEST(test_publisher_send)
{
    MqttContextClient* contextClient;
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_Initialize_Client(contextClient, URI_MQTT_BROKER, NULL, NULL, NULL, 0, NULL, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Sleep(100); // Let time to mqtt Client to connect
    for (int i = 0; i < NB_TOPIC; i++)
    {
        status = SOPC_MQTT_Send_Message(contextClient, MQTT_LIB_TOPIC_NAME[i], encoded_network_msg);
        ck_assert_int_eq(SOPC_STATUS_OK, status);
    }
    SOPC_MQTT_Release_Client(contextClient);
}
END_TEST

START_TEST(test_callback_subscription)
{
    MqttContextClient* contextClient;
    SOPC_ReturnStatus status = SOPC_MQTT_Create_Client(&contextClient);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    status = SOPC_MQTT_Initialize_Client(contextClient, URI_MQTT_BROKER, NULL, NULL, MQTT_LIB_TOPIC_NAME, NB_TOPIC,
                                         cbMessageArrivedTest, NULL);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_Sleep(100); // Let time to mqtt Client to connect
    status = SOPC_MQTT_Send_Message(contextClient, MQTT_LIB_TOPIC_NAME[0], encoded_network_msg);
    ck_assert_int_eq(SOPC_STATUS_OK, status);
    SOPC_MQTT_Release_Client(contextClient);
}
END_TEST

int main(void)
{
    int number_failed;
    Suite* suite = suite_create("MQTT PubSub module test");
    SRunner* sr;

    TCase* tc_client = tcase_create("Mqtt client creation and initialisation");
    suite_add_tcase(suite, tc_client);
    tcase_add_test(tc_client, test_create_client);
    tcase_add_test(tc_client, test_difference_topic_nbTopic_fail);
    tcase_add_test(tc_client, test_username_noPassword_fail);

    TCase* tc_publisher_client = tcase_create("Mqtt publisher");
    suite_add_tcase(suite, tc_publisher_client);
    tcase_add_test(tc_publisher_client, test_publisher_send);

    TCase* tc_subscriber_client = tcase_create("Mqtt subscriber");
    suite_add_tcase(suite, tc_subscriber_client);
    tcase_add_test(tc_subscriber_client, test_callback_subscription);

    sr = srunner_create(suite);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
