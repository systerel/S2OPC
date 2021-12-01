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
 * \brief Entry point for threads tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 */

#include <check.h>
#include <stdio.h>

#include "check_helpers.h"

#include "sopc_common_constants.h"
#include "sopc_helper_string.h"
#include "sopc_log_manager.h"
#include "sopc_mem_alloc.h"
#include "sopc_time.h"

#define MAX_LINE_LENGTH 100
#define LINE_PREFIX_LENGTH 26

START_TEST(test_logger_levels)
{
    SOPC_Log_Instance* otherLog = NULL;
    bool res = false;
    FILE* refLogFile = NULL;
    FILE* genLogFile = NULL;
    int ires = 0;
    char* filePathPrefix = NULL;
    char* filePath = NULL;
    char refFilePath[17];
    char refLogLine[MAX_LINE_LENGTH];
    char* refLogLineBis = NULL;
    char genLogLine[MAX_LINE_LENGTH];
    char* genLogLineBis = NULL;

    otherLog = SOPC_Log_CreateFileInstance("", "AnotherLogFile", "Other", 10000, 2); // Another testLog file
    ck_assert(otherLog != NULL);

    res = SOPC_Log_SetConsoleOutput(otherLog, true);
    ck_assert(res != false);

    res = SOPC_Log_SetLogLevel(otherLog, SOPC_LOG_LEVEL_ERROR);
    ck_assert(res != false);
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_ERROR, "Printing in another log %d %s", 1000, "using template");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_WARNING, "No warning printed !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_INFO, "No info printed !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_DEBUG, "No debug printed !");

    // wait 1 ms to have different timestamps
    SOPC_Sleep(1);

    SOPC_Log_SetConsoleOutput(otherLog, false);
    res = SOPC_Log_SetLogLevel(otherLog, SOPC_LOG_LEVEL_WARNING);
    ck_assert(res != false);
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_WARNING, "Warning printed !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_INFO, "No info printed !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_DEBUG, "No debug printed !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_ERROR, "It is another log");

    // wait 1 ms to have different timestamps
    SOPC_Sleep(1);

    res = SOPC_Log_SetLogLevel(otherLog, SOPC_LOG_LEVEL_INFO);
    ck_assert(res != false);
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_WARNING, "Warning printed again !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_INFO, "Info printed !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_DEBUG, "No debug printed !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_ERROR, "Still another log !");

    // wait 1 ms to have different timestamps
    SOPC_Sleep(1);

    res = SOPC_Log_SetLogLevel(otherLog, SOPC_LOG_LEVEL_DEBUG);
    ck_assert(res != false);
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_WARNING, "Warning printed again !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_INFO, "Info printed 2 times !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_DEBUG, "Debug printed !");
    SOPC_Log_Trace(otherLog, SOPC_LOG_LEVEL_ERROR, "Error in the end");

    filePathPrefix = SOPC_Log_GetFilePathPrefix(otherLog);
    ck_assert(filePathPrefix != NULL);
    ck_assert(strlen(filePathPrefix) > 0);

    // Close the otherLog instance
    SOPC_Log_ClearInstance(&otherLog);

    // Check testLog log file content is the expected content
    SOPC_Free(filePath);
    filePath = SOPC_Malloc((strlen(filePathPrefix) + 10) * sizeof(char)); // 9 + '\0'
    ck_assert(filePath != NULL);
    ires = sprintf(filePath, "%s00000.log", filePathPrefix);
    ck_assert(ires > 0);
    strcpy(refFilePath, "logAnother.ref");
    // Open the generated log and reference log files
    refLogFile = fopen(refFilePath, "r");
    ck_assert(refLogFile != NULL);
    genLogFile = fopen(filePath, "r");
    ck_assert(genLogFile != NULL);

    printf("Comparing log files: %s / %s\n", refFilePath, filePath);

    refLogLineBis = refLogLine;
    genLogLineBis = genLogLine;
    while (NULL != refLogLineBis && NULL != genLogLineBis)
    {
        refLogLineBis = fgets(refLogLine, MAX_LINE_LENGTH, refLogFile);
        genLogLineBis = fgets(genLogLine, MAX_LINE_LENGTH, genLogFile);

        if (refLogLineBis != NULL && genLogLineBis != NULL)
        {
            // Compare logs excluding the timestamp prefix
            ires = strcmp(&refLogLine[LINE_PREFIX_LENGTH], &genLogLine[LINE_PREFIX_LENGTH]);
            printf("Comparing (result == %d):\n- %s- %s", ires, &refLogLine[LINE_PREFIX_LENGTH],
                   &genLogLine[LINE_PREFIX_LENGTH]);
            ck_assert(ires == 0);
        }
        else
        {
            ck_assert(refLogLineBis == NULL && genLogLineBis == NULL);
        }
    }

    fclose(refLogFile);
    fclose(genLogFile);

    SOPC_Free(filePathPrefix);
    SOPC_Free(filePath);
}
END_TEST

START_TEST(test_logger_categories_and_files)
{
    // For 1 line of testLog
    // 23 characters used for compact timestamps + 2 for brackets + 1 blank
    // 9 characters used for category + 1 blank
    // 8 characters used for Error level (including 1 space) => Not present in start stop
    // 1 character for newline
    // => 37 characters + 8 level + trace length

    // Start message => 37 + 9 trace = 46

    SOPC_Log_Instance* testLog = NULL;
    SOPC_Log_Instance* testLog2 = NULL;
    SOPC_Log_Instance* testLog3 = NULL;
    FILE* refLogFile = NULL;
    FILE* genLogFile = NULL;
    int ires = 0;
    char* filePathPrefix = NULL;
    char* filePath = NULL;
    char refFilePath[17];
    char refLogLine[MAX_LINE_LENGTH];
    char* refLogLineBis = NULL;
    char genLogLine[MAX_LINE_LENGTH];
    char* genLogLineBis = NULL;
    char idx = 0;

    testLog = SOPC_Log_CreateFileInstance("./not_existing_path/", "testLogFile", "Category1", 340, 3);
    ck_assert(testLog == NULL);
    testLog = SOPC_Log_CreateFileInstance("", "testLogFile", "Category1", 0, 3);
    ck_assert(testLog == NULL);
    testLog = SOPC_Log_CreateFileInstance("", "testLogFile", "Category1", 340, 0);
    ck_assert(testLog == NULL);

    testLog =
        SOPC_Log_CreateFileInstance("", "TestLogFile", "Category1",
                                    340, // 100 bytes reserved for final line (indicating next file) => 240 of trace
                                    3);  // 3 files => generate enough content to have 3 files
    ck_assert(testLog != NULL);

    testLog2 = SOPC_Log_CreateInstanceAssociation(testLog, "Category2");
    ck_assert(testLog2 != NULL);

    testLog3 = SOPC_Log_CreateInstanceAssociation(testLog, "Category9999"); // Truncated after 9 characters
    ck_assert(testLog3 != NULL);

    SOPC_Log_Trace(testLog, SOPC_LOG_LEVEL_ERROR, "This is a test "); // 1st message (60 characters for each)

    SOPC_Log_Trace(testLog2, SOPC_LOG_LEVEL_ERROR, "Always a test !"); // 2 messages

    SOPC_Log_Trace(testLog3, SOPC_LOG_LEVEL_ERROR,
                   "Again a test !!"); // 3 messages => 2nd file of first testLog creation !

    SOPC_Log_Trace(testLog2, SOPC_LOG_LEVEL_ERROR, "2nd file test !"); // 4 messages

    SOPC_Log_Trace(testLog2, SOPC_LOG_LEVEL_ERROR, "message in 2nd!"); // 5 messages

    SOPC_Log_Trace(testLog, SOPC_LOG_LEVEL_ERROR, "This is a test "); // 6 messages

    SOPC_Log_Trace(testLog, SOPC_LOG_LEVEL_ERROR,
                   "This is test 2!"); // 7 messages => 3rd file of first testLog creation

    SOPC_Log_Trace(testLog3, SOPC_LOG_LEVEL_ERROR, "This is the end"); // last message of first testLog

    // Retrieve the prefix file path of testLog files
    filePathPrefix = SOPC_Log_GetFilePathPrefix(testLog);
    ck_assert(filePathPrefix != NULL);
    ck_assert(strlen(filePathPrefix) > 0);

    // Close the testLog instances
    SOPC_Log_ClearInstance(&testLog);
    SOPC_Log_ClearInstance(&testLog2);
    SOPC_Log_ClearInstance(&testLog3);

    // Check testLog log file content is the expected content
    filePath = SOPC_Malloc((strlen(filePathPrefix) + 10) * sizeof(char)); // 9 + '\0'
    ck_assert(filePath != NULL);
    ires = sprintf(filePath, "%s00000.log", filePathPrefix);
    ck_assert(ires > 0);
    strcpy(refFilePath, "logTest.ref1");

    for (idx = 1; idx <= 3; idx++)
    {
        // Open the generated log and reference log files
        refFilePath[11] = (char) (48 + idx); // 48 => '0'
        refLogFile = fopen(refFilePath, "r");
        ck_assert(refLogFile != NULL);
        filePath[strlen(filePathPrefix) + 4] = (char) (48 + idx - 1); // 48 => '0'
        genLogFile = fopen(filePath, "r");
        ck_assert(genLogFile != NULL);

        printf("Comparing log files: %s / %s\n", refFilePath, filePath);

        refLogLineBis = refLogLine;
        genLogLineBis = genLogLine;
        while (NULL != refLogLineBis && NULL != genLogLineBis)
        {
            refLogLineBis = fgets(refLogLine, MAX_LINE_LENGTH, refLogFile);
            genLogLineBis = fgets(genLogLine, MAX_LINE_LENGTH, genLogFile);

            if (refLogLineBis != NULL && genLogLineBis != NULL)
            {
                // Compare logs excluding the timestamp prefix
                ires = strcmp(&refLogLine[LINE_PREFIX_LENGTH], &genLogLine[LINE_PREFIX_LENGTH]);
                if (ires != 0)
                {
                    // It shall be the continue in next file line
                    ires = memcmp(&refLogLine[LINE_PREFIX_LENGTH], "LOG CONTINUE IN NEXT FILE: ", 27);
                    if (ires == 0)
                    {
                        ires = memcmp(&genLogLine[LINE_PREFIX_LENGTH], "LOG CONTINUE IN NEXT FILE: ", 27);
                    }
                }
                printf("Comparing (result == %d):\n- %s- %s", ires, &refLogLine[LINE_PREFIX_LENGTH],
                       &genLogLine[LINE_PREFIX_LENGTH]);
                ck_assert(ires == 0);
            }
            else
            {
                ck_assert(refLogLineBis == NULL && genLogLineBis == NULL);
            }
        }
        fclose(refLogFile);
        fclose(genLogFile);
    }

    SOPC_Free(filePathPrefix);
    SOPC_Free(filePath);
}
END_TEST

START_TEST(test_logger_circular)
{
    // For 1 line of testLog
    // 23 characters used for compact timestamps + 2 for brackets + 1 blank
    // 9 characters used for category + 1 blank
    // 8 characters used for Error level (including 1 space) => Not present in start stop
    // 1 character for newline
    // => 37 characters + 8 level + trace length

    // Start message => 37 + 9 trace = 46

    SOPC_Log_Instance* circularLog = NULL;
    bool res = false;
    FILE* refLogFile = NULL;
    FILE* genLogFile = NULL;
    int ires = 0;
    char* filePathPrefix = NULL;
    char* filePath = NULL;
    char refFilePath[17];
    char refLogLine[MAX_LINE_LENGTH];
    char* refLogLineBis = NULL;
    char genLogLine[MAX_LINE_LENGTH];
    char* genLogLineBis = NULL;
    char idx = 0;

    circularLog = SOPC_Log_CreateFileInstance("", "CircularLogFile", "Circular1",
                                              340, // Same as testLog
                                              2);  // TestLog - 1 => first log file overwritten

    // Start circular log
    res = SOPC_Log_SetLogLevel(circularLog, SOPC_LOG_LEVEL_INFO);
    ck_assert(res != false);
    // Simulate a start line in circular log to generate same amount of bytes
    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_INFO, "LOG START");

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR, "This is a test "); // 1st message (60 characters for each)

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR, "Always a test !"); // 2 messages

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR,
                   "Again a test !!"); // 3 messages => 2nd file of first testLog creation !

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR, "2nd file test !"); // 4 messages

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR, "message in 2nd!"); // 5 messages

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR, "This is a test "); // 6 messages

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR,
                   "This is test 2!"); // 7 messages => 3rd file of first testLog creation !

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR, "3rd file test !"); // 8 messages

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR, "message in 2nd!"); // 9 messages

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR, "This is a test "); // 10 messages
    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR,
                   "This is test 3!"); // 11 messages => overwrite first circularLog file

    SOPC_Log_Trace(circularLog, SOPC_LOG_LEVEL_ERROR, "This is the end"); // last message of circularLog

    // Check circularLog log file content is the expected content
    filePathPrefix = SOPC_Log_GetFilePathPrefix(circularLog);
    ck_assert(filePathPrefix != NULL);
    ck_assert(strlen(filePathPrefix) > 0);

    // Close the circular log instance
    SOPC_Log_ClearInstance(&circularLog);

    // Check testLog log file content is the expected content
    SOPC_Free(filePath);
    filePath = SOPC_Malloc((strlen(filePathPrefix) + 10) * sizeof(char)); // 9 + '\0'
    ck_assert(filePath != NULL);
    ires = sprintf(filePath, "%s00000.log", filePathPrefix);
    ck_assert(ires > 0);
    strcpy(refFilePath, "logCircular.ref1");

    for (idx = 0; idx <= 2; idx++)
    {
        // Open the generated log and reference log files
        refFilePath[15] = (char) (48 + idx); // 48 => '0'
        refLogFile = fopen(refFilePath, "r");
        ck_assert(refLogFile != NULL);
        filePath[strlen(filePathPrefix) + 4] = (char) (48 + idx); // 48 => '0'
        genLogFile = fopen(filePath, "r");
        ck_assert(genLogFile != NULL);

        printf("Comparing log files: %s / %s\n", refFilePath, filePath);

        refLogLineBis = refLogLine;
        genLogLineBis = genLogLine;
        while (NULL != refLogLineBis && NULL != genLogLineBis)
        {
            refLogLineBis = fgets(refLogLine, MAX_LINE_LENGTH, refLogFile);
            genLogLineBis = fgets(genLogLine, MAX_LINE_LENGTH, genLogFile);

            if (refLogLineBis != NULL && genLogLineBis != NULL)
            {
                // Compare logs excluding the timestamp prefix
                ires = strcmp(&refLogLine[LINE_PREFIX_LENGTH], &genLogLine[LINE_PREFIX_LENGTH]);
                if (ires != 0)
                {
                    // It shall be the continue in next file line
                    ires = memcmp(&refLogLine[LINE_PREFIX_LENGTH], "LOG CONTINUE IN NEXT FILE: ", 27);
                    if (ires == 0)
                    {
                        ires = memcmp(&genLogLine[LINE_PREFIX_LENGTH], "LOG CONTINUE IN NEXT FILE: ", 27);
                    }
                }
                printf("Comparing (result == %d):\n- %s- %s", ires, &refLogLine[LINE_PREFIX_LENGTH],
                       &genLogLine[LINE_PREFIX_LENGTH]);
                ck_assert(ires == 0);
            }
            else
            {
                ck_assert(refLogLineBis == NULL && genLogLineBis == NULL);
            }
        }
        fclose(refLogFile);
        fclose(genLogFile);
    }

    SOPC_Free(filePathPrefix);
    SOPC_Free(filePath);
}
END_TEST

static char* SOPC_Check_Logger_lastUserCategory = NULL;
static char* SOPC_Check_Logger_lastUserLog_Dated = NULL;
static char* SOPC_Check_Logger_lastUserLog = NULL;
static bool SOPC_Check_Logger_userLogCalled = false;
static FILE* SOPC_Check_Logger_userFile = NULL;
static const char* SOPC_Check_Logger_dateSample = "[YYYY/MM/DD HH:MM:SS.mmm] ";
static void SOPC_Check_Logger_UserDoLog(const char* category, const char* const line)
{
    static const char* lineToShort = "Line too short!";
    if (SOPC_Check_Logger_userFile == NULL)
    {
        SOPC_Check_Logger_userFile = fopen("check_logger_user.log", "w");
    }
    fprintf(stderr, "[USER log] [%s] => <<%s>>\n", category, line);
    fprintf(SOPC_Check_Logger_userFile, "[%s] => <<%s>>\n", category, line);
    SOPC_Free(SOPC_Check_Logger_lastUserCategory);
    SOPC_Free(SOPC_Check_Logger_lastUserLog_Dated);
    SOPC_Free(SOPC_Check_Logger_lastUserLog);
    SOPC_Check_Logger_lastUserCategory = SOPC_strdup(category);
    SOPC_Check_Logger_lastUserLog_Dated = SOPC_strdup(line);
    if (strlen(line) > strlen(SOPC_Check_Logger_dateSample))
    {
        SOPC_Check_Logger_lastUserLog = SOPC_strdup(&line[strlen(SOPC_Check_Logger_dateSample)]);
    }
    else
    {
        SOPC_Check_Logger_lastUserLog = SOPC_strdup(lineToShort);
    }
    SOPC_Check_Logger_userLogCalled = true;
}

START_TEST(test_logger_user)
{
    char* filePathPrefix = NULL;
    SOPC_Log_Level readLogLevel = SOPC_LOG_LEVEL_ERROR;
    static const char* userlogLine1 = "First user log line";
    static const char* userlogLine2 = "Second user log line - filtered out";
    char* userLongLine3 = NULL;
    static const char* userlogLine4 = "4th line";
    static const char* userlogLine4b = "(Warning) 4th line";
    static const char* userlogLine5 = "5th line";
    static const char* userlogLine5b = "(Warning) 5th line";
    static const char* category = "U-LOG";
    static const char* category2 = "CATEGORY2";
    char aChar = 'A';
    size_t index = 0;
    // Check user-defined logs
    SOPC_Log_Instance* userLog = NULL;
    SOPC_Log_Instance* userLog2 = NULL;

    bool res = false;

    userLog = SOPC_Log_CreateUserInstance(category, &SOPC_Check_Logger_UserDoLog);
    ck_assert(userLog != NULL);

    // Check that there is no associated log file
    filePathPrefix = SOPC_Log_GetFilePathPrefix(userLog);
    ck_assert(filePathPrefix == NULL);

    // Start user log
    res = SOPC_Log_SetLogLevel(userLog, SOPC_LOG_LEVEL_INFO);
    ck_assert(res != false);
    readLogLevel = SOPC_Log_GetLogLevel(userLog);
    ck_assert(readLogLevel == SOPC_LOG_LEVEL_INFO);

    // Check logging feature
    SOPC_Check_Logger_userLogCalled = false;
    SOPC_Log_Trace(userLog, SOPC_LOG_LEVEL_INFO, userlogLine1);
    ck_assert(SOPC_Check_Logger_userLogCalled);
    ck_assert_msg(0 == SOPC_strcmp_ignore_case(SOPC_Check_Logger_lastUserCategory, category),
                  "Was expecting CATEGORY <%s>, but found <%s>", category, SOPC_Check_Logger_lastUserCategory);
    ck_assert_msg(0 == SOPC_strcmp_ignore_case(SOPC_Check_Logger_lastUserLog, userlogLine1),
                  "Was expecting LOG LINE <%s>, but found <%s>", userlogLine1, SOPC_Check_Logger_lastUserLog);

    // Check logging filter
    SOPC_Check_Logger_userLogCalled = false;
    res = SOPC_Log_SetLogLevel(userLog, SOPC_LOG_LEVEL_WARNING);
    ck_assert(res != false);
    ck_assert_msg(0 == SOPC_strcmp_ignore_case("LOG LEVEL SET TO 'WARNING'", SOPC_Check_Logger_lastUserLog),
                  "Was expecting LOG LINE %s, but found %s", "LOG LEVEL SET TO 'WARNING'",
                  SOPC_Check_Logger_lastUserLog);
    ck_assert(SOPC_Check_Logger_userLogCalled);
    SOPC_Check_Logger_userLogCalled = false;
    SOPC_Log_Trace(userLog, SOPC_LOG_LEVEL_INFO, userlogLine2);
    ck_assert(!SOPC_Check_Logger_userLogCalled);

    // Check line limitation
    userLongLine3 = (char*) SOPC_Malloc(SOPC_LOG_MAX_USER_LINE_LENGTH + 10);
    for (index = 0; index < SOPC_LOG_MAX_USER_LINE_LENGTH + 9; index++)
    {
        userLongLine3[index] = aChar;
        aChar++;
        if (aChar >= 127)
        {
            aChar = ' ';
        }
    }
    userLongLine3[SOPC_LOG_MAX_USER_LINE_LENGTH + 9] = 0;
    SOPC_Check_Logger_userLogCalled = false;
    SOPC_Log_Trace(userLog, SOPC_LOG_LEVEL_WARNING, userLongLine3);
    SOPC_Free(userLongLine3);
    ck_assert(SOPC_Check_Logger_userLogCalled);
    ck_assert_msg(strlen(SOPC_Check_Logger_lastUserLog_Dated) == (size_t) SOPC_LOG_MAX_USER_LINE_LENGTH,
                  "Was expecting len=%zu, but found %zu", (size_t) SOPC_LOG_MAX_USER_LINE_LENGTH,
                  strlen(SOPC_Check_Logger_lastUserLog_Dated));

    // Check sections
    userLog2 = SOPC_Log_CreateInstanceAssociation(userLog, category2);
    res = SOPC_Log_SetLogLevel(userLog2, SOPC_LOG_LEVEL_DEBUG);
    ck_assert(userLog2 != NULL);

    SOPC_Check_Logger_userLogCalled = false;
    SOPC_Log_Trace(userLog2, SOPC_LOG_LEVEL_WARNING, userlogLine4);
    ck_assert(SOPC_Check_Logger_userLogCalled);
    ck_assert_msg(0 == SOPC_strcmp_ignore_case(SOPC_Check_Logger_lastUserLog, userlogLine4b),
                  "Was expecting LOG LINE %s, but found %s", userlogLine4b, SOPC_Check_Logger_lastUserLog);
    ck_assert_msg(0 == SOPC_strcmp_ignore_case(SOPC_Check_Logger_lastUserCategory, category2),
                  "Was expecting CATEGORY %s, but found %s", category2, SOPC_Check_Logger_lastUserCategory);

    // Check with console output
    SOPC_Log_SetConsoleOutput(userLog, true);
    SOPC_Check_Logger_userLogCalled = false;
    SOPC_Log_Trace(userLog2, SOPC_LOG_LEVEL_WARNING, userlogLine5);
    ck_assert(SOPC_Check_Logger_userLogCalled);
    ck_assert(0 == SOPC_strcmp_ignore_case(SOPC_Check_Logger_lastUserCategory, category2));
    ck_assert_msg(0 == SOPC_strcmp_ignore_case(SOPC_Check_Logger_lastUserLog, userlogLine5b),
                  "Was expecting LOG LINE %s, but found %s", userlogLine5b, SOPC_Check_Logger_lastUserLog);

    // Check unknown level (not taken into account)
    res = SOPC_Log_SetLogLevel(userLog, 4);
    ck_assert(false == res);
    readLogLevel = SOPC_Log_GetLogLevel(userLog);
    ck_assert(SOPC_LOG_LEVEL_WARNING == readLogLevel);

    SOPC_Check_Logger_userLogCalled = false;
    SOPC_Log_Trace(userLog, 4, "will be filtered out because level is mismatching");
    ck_assert(false == SOPC_Check_Logger_userLogCalled);

    // Close the second log instance and check first still works
    SOPC_Log_ClearInstance(&userLog2);
    ck_assert(userLog2 == NULL);

    SOPC_Check_Logger_userLogCalled = false;
    SOPC_Log_Trace(userLog2, SOPC_LOG_LEVEL_WARNING, userlogLine5);
    ck_assert(!SOPC_Check_Logger_userLogCalled);

    SOPC_Log_Trace(userLog, SOPC_LOG_LEVEL_WARNING, userlogLine5);
    ck_assert(SOPC_Check_Logger_userLogCalled);
    ck_assert(0 == SOPC_strcmp_ignore_case(SOPC_Check_Logger_lastUserCategory, category));
    ck_assert_msg(0 == SOPC_strcmp_ignore_case(SOPC_Check_Logger_lastUserLog, userlogLine5b),
                  "Was expecting LOG LINE %s, but found %s", userlogLine5b, SOPC_Check_Logger_lastUserLog);

    SOPC_Log_ClearInstance(&userLog);
}
END_TEST

static void init(void)
{
    SOPC_Log_Initialize();
}

static void clear(void)
{
    SOPC_Log_Clear();
    SOPC_Free(SOPC_Check_Logger_lastUserCategory);
    SOPC_Free(SOPC_Check_Logger_lastUserLog_Dated);
    SOPC_Free(SOPC_Check_Logger_lastUserLog);

    if (SOPC_Check_Logger_userFile != NULL)
    {
        fclose(SOPC_Check_Logger_userFile);
        SOPC_Check_Logger_userFile = NULL;
    }
}

Suite* tests_make_suite_logger(void)
{
    Suite* s;
    TCase* tc_logger;

    s = suite_create("Logger tests");
    tc_logger = tcase_create("Logger");
    tcase_add_unchecked_fixture(tc_logger, init, clear);
    tcase_add_test(tc_logger, test_logger_levels);
    tcase_add_test(tc_logger, test_logger_categories_and_files);
    tcase_add_test(tc_logger, test_logger_circular);
    tcase_add_test(tc_logger, test_logger_user);
    suite_add_tcase(s, tc_logger);

    return s;
}
