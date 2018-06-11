/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \file
 *
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdint.h>
#include <stdlib.h>

#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_log_manager.h"
#include "sopc_time.h" /* SOPC_Sleep */
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"

#include "toolkit_helpers.h"

#define SLEEP_TIME 10
#define TEST_TIMEOUT 10000

START_TEST(test_time_conversion)
{
    /* Thu Sep 21 00:00:00 1905 UTC, unix timestamp is -2028499761.000000 */
    ck_assert(Helpers_OPCTimeToNTP(96159738390000000ULL) == 775194519791468544ULL);
    /* Tue Jan  3 19:44:21 1978 UTC, unix timestamp is 252701061.000000 */
    ck_assert(Helpers_OPCTimeToNTP(118971746610000000ULL) == 10572877445889785856ULL);
    /* Thu Nov 30 04:57:25 2034 UTC, unix timestamp is 2048471845.694287 */
    ck_assert(Helpers_OPCTimeToNTP(136929454456942870ULL) == 18285654237264005879ULL);
    /* Tue Nov 30 04:57:25 2055 UTC, unix timestamp is 2711159845.694287 */
    ck_assert(Helpers_OPCTimeToNTP(143556334456942870ULL) == 2685133451006102263ULL);
    /* Fri May  4 17:34:36 2018 UTC, unix timestamp is 1525448076.741346 */
    ck_assert(Helpers_OPCTimeToNTP(131699216767413460ULL) == 16039284254580464121ULL);
}
END_TEST

/* Subscription test */

static SOPC_LibSub_ConnectionId con_id = 0;
static SOPC_LibSub_DataId dat_id = 0;
static bool bFirstValue = true;
static int64_t iFirstValue = 0;
static int32_t bValueChanged = 0; /* TODO: use sopc_atomic.h */
static int32_t bDisconnected = 0;

static void disconnect_callback(const SOPC_LibSub_ConnectionId c_id)
{
    /* This is not just assert(false), as the disconnection shall happen when closing the lib */
    ck_assert(c_id == con_id);
    bDisconnected = 1; /* TODO: use SOPC_Atomit_Int_Set */
}

static void datachange_callback(const SOPC_LibSub_ConnectionId c_id,
                                const SOPC_LibSub_DataId d_id,
                                const SOPC_LibSub_Value* value)
{
    ck_assert(c_id == con_id);
    ck_assert(d_id == dat_id);
    ck_assert(value->type == SOPC_LibSub_DataType_integer);
    ck_assert(value->quality == 0);

    if (bFirstValue)
    {
        bFirstValue = false;
        iFirstValue = *(int64_t*) (value->value);
    }
    else if (0 == bValueChanged)
    {
        bValueChanged = 1; /* TODO: use SOPC_Atomit_Int_Set */
        ck_assert(*(int64_t*) (value->value) == iFirstValue + 1);
    }
}

START_TEST(test_subscription)
{
    SOPC_LibSub_StaticCfg cfg_cli = {.host_log_callback = Helpers_LoggerStdout,
                                     .disconnect_callback = disconnect_callback};
    SOPC_LibSub_ConnectionCfg cfg_con = {.server_url = "opc.tcp://localhost:4841",
                                         .security_policy = SOPC_SecurityPolicy_None_URI,
                                         .security_mode = OpcUa_MessageSecurityMode_None,
                                         .path_cert_auth = "./trusted/cacert.der",
                                         .path_cert_srv = NULL,
                                         .path_cert_cli = NULL,
                                         .path_key_cli = NULL,
                                         .path_crl = NULL,
                                         .username = NULL,
                                         .password = NULL,
                                         .publish_period_ms = 100,
                                         .data_change_callback = datachange_callback,
                                         .timeout_ms = TEST_TIMEOUT,
                                         .sc_lifetime = 60000,
                                         .token_target = 3};
    SOPC_LibSub_ConfigurationId cfg_id = 0;

    ck_assert(SOPC_LibSub_Initialize(&cfg_cli) == SOPC_STATUS_OK);
    ck_assert(SOPC_LibSub_ConfigureConnection(&cfg_con, &cfg_id) == SOPC_STATUS_OK);
    ck_assert(SOPC_LibSub_Configured() == SOPC_STATUS_OK);
    ck_assert(SOPC_LibSub_Connect(cfg_id, &con_id) == SOPC_STATUS_OK);
    ck_assert(SOPC_LibSub_AddToSubscription(con_id, "s=Counter", SOPC_LibSub_AttributeId_Value, &dat_id) ==
              SOPC_STATUS_OK);

    /* Wait for deconnection, failed assert, or subscription success */
    int iCnt = 0;
    /* TODO: use SOPC_Atomic_Int_Get */
    while (iCnt * SLEEP_TIME <= TEST_TIMEOUT && bValueChanged == 0 && bDisconnected == 0)
    {
        SOPC_Sleep(SLEEP_TIME);
    }

    ck_assert(bDisconnected == 0);
    ck_assert(SOPC_LibSub_Disconnect(con_id) == SOPC_STATUS_OK);
    SOPC_LibSub_Clear();
}
END_TEST

Suite* tests_make_suite_libsub(void)
{
    Suite* s = NULL;
    TCase *tc_time = NULL, *tc_libsub;

    s = suite_create("Client subscription library");

    tc_time = tcase_create("Time");
    tcase_add_test(tc_time, test_time_conversion);
    suite_add_tcase(s, tc_time);

    tc_libsub = tcase_create("LibSub");
    tcase_add_test(tc_libsub, test_subscription);
    suite_add_tcase(s, tc_libsub);

    return s;
}

int main(void)
{
    int number_failed = 0;
    SRunner* sr = NULL;

    sr = srunner_create(tests_make_suite_libsub());
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
