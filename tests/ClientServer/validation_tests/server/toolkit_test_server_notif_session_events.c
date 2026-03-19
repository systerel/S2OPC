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

#include <stdio.h>
#include <stdlib.h> /* getenv, exit */
#include <string.h>

#include "toolkit_test_server_notif_session_events.h"

#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_helper_askpass.h"
#include "sopc_helper_string.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_threads.h"

#include "libs2opc_common_config.h"
#include "libs2opc_server.h"
#include "libs2opc_server_config.h"

// Server endpoints and PKI configuration
#define XML_SERVER_CFG_PATH "./S2OPC_Server_UACTT_Config.xml"
// Server address space configuration
#define XML_ADDRESS_SPACE_PATH "./S2OPC_Demo_NodeSet.xml"
// User credentials and authorizations
#define XML_USERS_CFG_PATH "./S2OPC_Users_Demo_Config.xml"

static int32_t atomicStopped = false;

static void SOPC_ServerStopped_Cb(SOPC_ReturnStatus status)
{
    SOPC_UNUSED_ARG(status);
    SOPC_Atomic_Int_Set(&atomicStopped, true);
}

/*---------------------------------------------------------------------------
 *                          Server initialization
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_Initialize(void)
{
    // Due to issue in certification tool for View Basic 005/015/020 number of chunks shall be the same and at least 12
    SOPC_Common_EncodingConstants encConf = SOPC_Common_GetDefaultEncodingConstants();
    encConf.receive_max_nb_chunks = 12;
    encConf.send_max_nb_chunks = 12;
    bool res = SOPC_Common_SetEncodingConstants(encConf);
    SOPC_ASSERT(res);

    // Get default log config and set the custom path
    SOPC_Log_Configuration logConfiguration = SOPC_Common_GetDefaultLogConfiguration();
    logConfiguration.logSysConfig.fileSystemLogConfig.logDirPath = "./toolkit_test_server_notif_session_events_logs/";
    logConfiguration.logLevel = SOPC_LOG_LEVEL_DEBUG;

    SOPC_ReturnStatus status = SOPC_CommonHelper_Initialize(&logConfiguration, NULL);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_Initialize();
    }
    if (SOPC_STATUS_OK != status)
    {
        printf("<Test_Server_Toolkit_Notif_Session_Events: Failed initializing\n");
    }
    else
    {
        printf("<Test_Server_Toolkit_Notif_Session_Events: initialized\n");
    }
    return status;
}

/*---------------------------------------------------------------------------
 *                             Server configuration
 *---------------------------------------------------------------------------*/

static SOPC_ReturnStatus Server_LoadServerConfiguration(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    const char* xml_server_config_path = NULL;
    const char* xml_address_space_config_path = NULL;
    const char* xml_users_config_path = NULL;

    // Define a callback to retrieve the server key password (from environment variable)
    status = SOPC_ServerConfigHelper_SetKeyPasswordCallback(&SOPC_TestHelper_AskPass_FromEnv);

    xml_server_config_path = XML_SERVER_CFG_PATH;
    xml_address_space_config_path = XML_ADDRESS_SPACE_PATH;
    xml_users_config_path = XML_USERS_CFG_PATH;

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_ConfigureFromXML(xml_server_config_path, xml_address_space_config_path,
                                                          xml_users_config_path, NULL);
    }

    return status;
}

/*---------------------------------------------------------------------------
 *                 Session event notifications verification
 *---------------------------------------------------------------------------*/

static int32_t nbFailures = 0;
static int32_t nbReceivedEvents = 0;
static const Test_ExpectedEvent** expectedEvents = NULL;
static int32_t nbExpectedEvents = 0;

static int32_t currentNbFailures = 0;

static void set_expected_session_events_test(const Test_ExpectedEvent** expectedEventsList, size_t nbEvents)
{
    SOPC_Atomic_Int_Set(&nbReceivedEvents, 0);
    nbExpectedEvents = (int32_t) nbEvents;
    expectedEvents = expectedEventsList;
    currentNbFailures = SOPC_Atomic_Int_Get(&nbFailures);
}

static SOPC_ReturnStatus check_session_events_test(uint32_t maxWait_ms)
{
    uint32_t nb100ms = maxWait_ms / 100;
    while (nb100ms > 0 && SOPC_Atomic_Int_Get(&nbReceivedEvents) < nbExpectedEvents)
    {
        SOPC_Sleep(100);
        nb100ms -= 1;
    }
    return (SOPC_Atomic_Int_Get(&nbReceivedEvents) == nbExpectedEvents &&
            currentNbFailures == SOPC_Atomic_Int_Get(&nbFailures))
               ? SOPC_STATUS_OK
               : SOPC_STATUS_NOK;
}

static char* getClientUserIdFromUserToken(const SOPC_ExtensionObject* token)
{
    if (NULL == token)
    {
        return NULL;
    }

    char* result = NULL;

    if (SOPC_ExtObjBodyEncoding_Object == token->Encoding &&
        &OpcUa_AnonymousIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        result = SOPC_strdup("ANONYMOUS_RESERVED");
    }
    else if (SOPC_ExtObjBodyEncoding_Object == token->Encoding &&
             &OpcUa_UserNameIdentityToken_EncodeableType == token->Body.Object.ObjType)
    {
        OpcUa_UserNameIdentityToken* userToken = (OpcUa_UserNameIdentityToken*) (token->Body.Object.Value);

        result = SOPC_String_GetCString(&userToken->UserName);
    }
    else
    {
        SOPC_ASSERT(false && "not implemented for other user token types in test");
        return NULL;
    }

    return result;
}

static char* getClientUserId(const SOPC_User* user)
{
    if (NULL == user)
    {
        return NULL;
    }
    if (SOPC_User_IsAnonymous(user))
    {
        return SOPC_strdup("ANONYMOUS_RESERVED");
    }
    else if (SOPC_User_IsUsername(user))
    {
        const char* userId = SOPC_String_GetRawCString(SOPC_User_GetUsername(user));

        return SOPC_strdup(userId);
    }
    else
    {
        SOPC_ASSERT(false && "not implemented for other user token types in test");
        return NULL;
    }
}

static const char* getSessionEventName(SOPC_ServerSessionEvent sessionEvent)
{
    return SESSION_CREATION == sessionEvent     ? "SESSION_CREATION"
           : SESSION_ACTIVATION == sessionEvent ? "SESSION_ACTIVATION"
           : SESSION_INACTIVE == sessionEvent   ? "SESSION_INACTIVE"
           : SESSION_CLOSURE == sessionEvent    ? "SESSION_CLOSURE"
                                                : "INVALID_SESSION_EVENT";
}

static bool checkBothNullOrEqual(const char* str1, const char* str2)
{
    if (str1 == str2)
    {
        return true;
    }
    else if (NULL != str1 && NULL != str2)
    {
        return 0 == strcmp(str1, str2);
    }
    else
    {
        return false;
    }
}

static void Test_SessionEventCallback(const SOPC_CallContext* callCtxPtr,
                                      SOPC_ServerSessionEvent sessionEvent,
                                      SOPC_SessionId id,
                                      SOPC_StatusCode opStatus)
{
    int32_t localNbReceivedEvents = SOPC_Atomic_Int_Get(&nbReceivedEvents);
    if (localNbReceivedEvents >= nbExpectedEvents)
    {
        printf("Received unexpected number of session events: unexpected %s\n", getSessionEventName(sessionEvent));
        nbFailures++;
        return;
    }
    const Test_ExpectedEvent* expectedEvent = expectedEvents[localNbReceivedEvents];

    const char* peerInfo = SOPC_CallContext_GetClientIpPort(callCtxPtr);
    const OpcUa_ApplicationDescription* appDesc = SOPC_CallContext_GetClientApplicationDesc(callCtxPtr);
    const SOPC_User* user = SOPC_CallContext_GetUser(callCtxPtr);
    const char* sessionName = SOPC_CallContext_GetSessionName(callCtxPtr);
    char* failedUserId = getClientUserIdFromUserToken(SOPC_CallContext_GetFailedActivationUserToken(callCtxPtr));

    bool failure = false;
    if (expectedEvent->event != sessionEvent)
    {
        printf("Received session event %s while expecting %s\n", getSessionEventName(sessionEvent),
               getSessionEventName(expectedEvent->event));
        failure = true;
    }

    if (!failure && expectedEvent->id != id)
    {
        printf("Received session event with id=%" PRIu32 " while expecting id=%" PRIu32 "\n", id, expectedEvent->id);
        failure = true;
    }

    if (!checkBothNullOrEqual(expectedEvent->sessionName, sessionName))
    {
        printf("Received session event with session name=%s while expecting session name=%s\n",
               NULL == sessionName ? "<NULL>" : sessionName,
               NULL == expectedEvent->sessionName ? "<NULL>" : expectedEvent->sessionName);
        failure = true;
    }

    if ((const void*) expectedEvent->appName != (const void*) appDesc && NULL != appDesc &&
        !checkBothNullOrEqual(expectedEvent->appName, SOPC_String_GetRawCString(&appDesc->ApplicationName.defaultText)))
    {
        printf("Received session event with application name=%s while expecting application name=%s\n",
               NULL == appDesc ? "<NULL>" : SOPC_String_GetRawCString(&appDesc->ApplicationName.defaultText),
               NULL == expectedEvent->appName ? "<NULL>" : expectedEvent->appName);
        failure = true;
    }

    if (expectedEvent->isNullClientHostInfo && NULL != peerInfo)
    {
        printf("Received session event with client host info=%s while expecting no client host info\n", peerInfo);
        failure = true;
    }
    else if (!expectedEvent->isNullClientHostInfo && (NULL == peerInfo || strlen(peerInfo) < 1))
    {
        printf("Received session event with no client host info while expecting client host info\n");
        failure = true;
    }

    if (expectedEvent->opStatus != opStatus)
    {
        printf("Received session event with status=0x%" PRIx32 " while expecting status=0x%" PRIx32 "\n", opStatus,
               expectedEvent->opStatus);
        failure = true;
    }

    if (!failure && SESSION_ACTIVATION == expectedEvent->event && expectedEvent->opStatus != SOPC_GoodGenericStatus)
    {
        if (!checkBothNullOrEqual(expectedEvent->userId, failedUserId))
        {
            printf("Received session activation event with failed user id=%s while expecting failed user id=%s\n",
                   NULL == failedUserId ? "<NULL>" : failedUserId,
                   NULL == expectedEvent->userId ? "<NULL>" : expectedEvent->userId);
            failure = true;
        }
    }
    else
    {
        char* userId = getClientUserId(user);
        if (!checkBothNullOrEqual(expectedEvent->userId, userId))
        {
            printf("Received session event with user id=%s while expecting user id=%s\n",
                   NULL == userId ? "<NULL>" : userId,
                   NULL == expectedEvent->userId ? "<NULL>" : expectedEvent->userId);
            failure = true;
        }
        SOPC_Free(userId);
    }

    SOPC_Free(failedUserId);

    SOPC_Atomic_Int_Add(&nbReceivedEvents, 1);
    if (failure)
    {
        SOPC_Atomic_Int_Add(&nbFailures, 1);
    }
}

/*---------------------------------------------------------------------------
 *                             Server main function
 *---------------------------------------------------------------------------*/

int main(int argc, char* argv[])
{
    SOPC_UNUSED_ARG(argc);
    SOPC_UNUSED_ARG(argv);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    /* Get the toolkit build information and print it */
    SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo();
    printf("S2OPC_Common       - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.commonBuildInfo.buildVersion, build_info.commonBuildInfo.buildSrcCommit,
           build_info.commonBuildInfo.buildDockerId, build_info.commonBuildInfo.buildBuildDate);
    printf("S2OPC_ClientServer - Version: %s, SrcCommit: %s, DockerId: %s, BuildDate: %s\n",
           build_info.clientServerBuildInfo.buildVersion, build_info.clientServerBuildInfo.buildSrcCommit,
           build_info.clientServerBuildInfo.buildDockerId, build_info.clientServerBuildInfo.buildBuildDate);

    /* Initialize the server library (start library threads) */
    status = Server_Initialize();

    if (SOPC_STATUS_OK == status)
    {
        status = Server_LoadServerConfiguration();
    }

    /* Define session events callback */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ServerConfigHelper_SetSessionEventNotifCallback(Test_SessionEventCallback);
    }

    /* Start the server */
    if (SOPC_STATUS_OK == status)
    {
        printf("<Test_Server_Toolkit_Notif_Session_Events: Server started\n");

        /* Run the server until error or stop server signal detected (Ctrl-C) */
        status = SOPC_ServerHelper_StartServer(&SOPC_ServerStopped_Cb);

        if (SOPC_STATUS_OK == status)
        {
            SOPC_CONSOLE_PRINTF("\n=========== Test setup complete. Starting tests\n");

            nbFailures = 0;
            SOPC_CONSOLE_PRINTF("\n=========== TC 1: Normal 'None' connection (Success case) ===========\n");
            // launch a test.
            set_expected_session_events_test(&session_AnonOK[0], sizeof(session_AnonOK) / sizeof(*session_AnonOK));
            int ret = 0;
#ifdef _WIN32
            ret = system("s2opc_read --none -n i=2259 -a 13");
#else
            ret = system("./s2opc_read --none -n i=2259 -a 13");
#endif
            SOPC_ASSERT(ret == 0);

            status = check_session_events_test(2000);
            SOPC_CONSOLE_PRINTF("\n===========> TEST TC1 %s\n", (SOPC_STATUS_OK == status) ? "PASSED" : "FAILED");

            SOPC_CONSOLE_PRINTF("\n=========== TC 2: connection with user/pass (Success case) ===========\n");
            // launch a test.
            set_expected_session_events_test(&session_UserOK[0], sizeof(session_UserOK) / sizeof(*session_UserOK));
#ifdef _WIN32
            ret = system(
                "cmd /C \"set TEST_USERNAME=user1&& set TEST_PASSWORD_USER=password&& s2opc_wrapper_connect 2\"");
#else
            ret = system("TEST_USERNAME=user1 TEST_PASSWORD_USER=password ./s2opc_wrapper_connect 2");
#endif
            SOPC_ASSERT(ret == 0);

            status = check_session_events_test(2000);
            SOPC_CONSOLE_PRINTF("\n===========> TEST TC2 %s\n", (SOPC_STATUS_OK == status) ? "PASSED" : "FAILED");

            SOPC_CONSOLE_PRINTF("\n=========== TC 3: connection with bad user (ActivateSession failed) ===========\n");
            // launch a test.
            set_expected_session_events_test(&session_UserNOK[0], sizeof(session_UserNOK) / sizeof(*session_UserNOK));
#ifdef _WIN32
            ret = system(
                "cmd /C \"set TEST_USERNAME=unknownUser&& set TEST_PASSWORD_USER=password&& s2opc_wrapper_connect 2\"");
#else
            ret = system("TEST_USERNAME=unknownUser TEST_PASSWORD_USER=password ./s2opc_wrapper_connect 2");
#endif
            SOPC_ASSERT(ret != 0);

            status = check_session_events_test(2000);
            SOPC_CONSOLE_PRINTF("\n===========> TEST TC3 %s\n", (SOPC_STATUS_OK == status) ? "PASSED" : "FAILED");

            SOPC_CONSOLE_PRINTF("\n=========== TC 4: connection with bad pass (ActivateSession failed) ===========\n");
            // launch a test.
            set_expected_session_events_test(&session_PwdNOK[0], sizeof(session_UserNOK) / sizeof(*session_UserNOK));
#ifdef _WIN32
            ret = system(
                "cmd /C \"set TEST_USERNAME=user1&& set TEST_PASSWORD_USER=PASSword&& s2opc_wrapper_connect 2\"");
#else
            ret = system("TEST_USERNAME=user1 TEST_PASSWORD_USER=PASSword ./s2opc_wrapper_connect 2");
#endif
            SOPC_ASSERT(ret != 0);

            status = check_session_events_test(2000);
            SOPC_CONSOLE_PRINTF("\n===========> TEST TC4 %s\n", (SOPC_STATUS_OK == status) ? "PASSED" : "FAILED");

            SOPC_CONSOLE_PRINTF("\n=========== End of tests. Tear down\n");
        }
        else
        {
            SOPC_CONSOLE_PRINTF("\n=========== Test setup FAILED. Tests will NOT be executed\n");
        }

        if (SOPC_STATUS_OK != status)
        {
            printf(
                "<Test_Server_Toolkit_Notif_Session_Events: Failed to run the server or end to serve with error = "
                "'%d'\n",
                status);
        }
        else
        {
            printf("<Test_Server_Toolkit_Notif_Session_Events: Server ended to serve successfully\n");
        }
    }
    else
    {
        printf(
            "<Test_Server_Toolkit_Notif_Session_Events: Error during configuration phase, see logs in %s_logs "
            "directory for "
            "details.\n",
            argv[0]);
    }

    /* Clear the server library (stop all library threads) and server configuration */
    SOPC_ServerConfigHelper_Clear();
    SOPC_CommonHelper_Clear();

    if (SOPC_STATUS_OK != status)
    {
        printf(
            "<Test_Server_Toolkit_Notif_Session_Events: Terminating with error status, see logs in %s_logs directory "
            "for "
            "details.\n",
            argv[0]);
    }
    return (status == SOPC_STATUS_OK) ? 0 : 1;
}
