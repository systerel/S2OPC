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
 * \brief A client executable using the client_subscription library.
 *
 */

#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libs2opc_client_cmds.h"

/* Secure Channel configuration */
#define DEFAULT_ENDPOINT_URL "opc.tcp://localhost:4841"
/* Security Policy is None or Basic256 or Basic256Sha256 */
#define SECURITY_POLICY SOPC_SecurityPolicy_None_URI
/* Security Mode is None or Sign or SignAndEncrypt */
#define SECURITY_MODE OpcUa_MessageSecurityMode_None

/* Connection global timeout */
#define TIMEOUT_MS 10000
/* Secure Channel lifetime */
#define SC_LIFETIME_MS 3600000
/* Default publish period */
#define PUBLISH_PERIOD_MS 500
/* Default max keep alive count */
#define MAX_KEEP_ALIVE_COUNT 30
/* Lifetime Count of subscriptions */
#define MAX_LIFETIME_COUNT 1000
/* Number of targetted publish token */
#define PUBLISH_N_TOKEN 3

/* Path to the certificate authority */
#define PATH_CACERT_PUBL "./trusted/cacert.der"
/* Path to the server certificate */
#define PATH_SERVER_PUBL "./server_public/server_2k_cert.der"
/* Path to the client certificate */
#define PATH_CLIENT_PUBL "./client_public/client_2k_cert.der"
/* Path to the client private key */
#define PATH_CLIENT_PRIV "./client_private/client_2k_key.pem"

/* Default policy Id */
#define POLICY_ID "anonymous"

/* Command line helpers and arguments */
enum
{
    OPT_HELP,
    OPT_ENDPOINT,
    OPT_POLICYID,
    OPT_USERNAME,
    OPT_PASSWORD,
    OPT_PUBLISH_PERIOD,
    OPT_TOKEN_TARGET,
    OPT_DISABLE_SECU,
    OPT_KEEPALIVE,
} cmd_line_option_values_t;
typedef struct
{
    char* endpoint_url;
    char* policyId;
    char* username;
    char* password;
    char* publish_period_str;
    int64_t publish_period;
    char* token_target_str;
    uint16_t token_target;
    int node_ids_size;
    char** node_ids;
    bool disable_certificate_verification;
    char* n_max_keepalive_str;
    uint32_t n_max_keepalive;
} cmd_line_options_t;
static bool parse_options(cmd_line_options_t* o, int argc, char* const* argv);
static void free_options(cmd_line_options_t* o);
static void print_usage(const char* exe);

static void datachange_callback(const int32_t c_id,
                                const char* node_id,
                                const SOPC_DataValue* value)
{
    char sz[1024];
    size_t n;

    //TODO use format constant for node_id if it exists
    n = (size_t) snprintf(sz, sizeof(sz) / sizeof(sz[0]),
                          "Client %" PRIu32 " data change:\n  value id %s\n  new value ", c_id, node_id);


    if (NULL == value)
    {
        snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, "NULL");
    }
    else {
        // TODO is there a generic function to print/format a variant ?
        SOPC_Variant variant = value->Value;
        //TODO print value
        if (SOPC_Boolean_Id == variant.BuiltInTypeId)
        {
            snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, (bool) variant.Value.Boolean ? "true" : "false");
        }
        else if (SOPC_UInt64_Id == variant.BuiltInTypeId)
        {
            snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, "%" PRIu64, (int64_t) variant.Value.Uint64);
        }
        //else if (SOPC_Int64_Id == variant.BuiltInTypeId)
        //{
        //    snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, "%" PRIi64, (int64_t) variant.Value);
        //}
        //else
        //{
        //    snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, "%s", (SOPC_LibSub_CstString) variant.Value);
        //}
    }

    printf("%s",sz);
}

/* Main subscribing client */
int main(int argc, char* const argv[])
{
    int res = 0;

    cmd_line_options_t options;
    if (!parse_options(&options, argc, argv))
    {
        free_options(&options);
        return 1;
    }

    SOPC_ClientHelper_Initialize("./client_subscription_logs/", SOPC_TOOLKIT_LOG_LEVEL_DEBUG);

    SOPC_ClientHelper_Security security = {
        .security_policy = SECURITY_POLICY,
        .security_mode = SECURITY_MODE,
        .path_cert_auth = PATH_CACERT_PUBL,
        .path_cert_srv = PATH_SERVER_PUBL,
        .path_cert_cli = PATH_CLIENT_PUBL,
        .path_key_cli = PATH_CLIENT_PRIV,
        .policyId = options.policyId,
        .username = options.username,
        .password = options.password,
    };

    int32_t connectionId = SOPC_ClientHelper_Connect(
        options.endpoint_url, security);

    if (connectionId <= 0)
    {
        res = connectionId != 0 ? connectionId : -1;
    }

    if (res == 0)
    {
        res = SOPC_ClientHelper_CreateSubscription(connectionId, datachange_callback);
    }

    if (res == 0)
    {
        assert(options.node_ids_size > 0);
        res = SOPC_ClientHelper_AddMonitoredItems(connectionId, options.node_ids, (size_t) options.node_ids_size);
    }


    if (0 == res)
    {
        //Browse server
        size_t nbBrowse = 4;
        SOPC_ClientHelper_BrowseRequest* browseRequests = (SOPC_ClientHelper_BrowseRequest*)
            calloc(nbBrowse, sizeof(SOPC_ClientHelper_BrowseRequest));
        assert(NULL != browseRequests);
        SOPC_ClientHelper_BrowseResult* browseResults = (SOPC_ClientHelper_BrowseResult*)
            calloc(nbBrowse, sizeof(SOPC_ClientHelper_BrowseResult));
        assert(NULL != browseResults);

        browseRequests[3].nodeId = "ns=0;i=85"; // Root/Objects/
        browseRequests[3].direction= 0; // forward
        browseRequests[3].referenceTypeId = ""; // all reference types (NULL?)
        browseRequests[3].includeSubtypes = true;

        browseRequests[1].nodeId = "ns=0;i=84"; // Root/
        browseRequests[1].direction= 0; // forward
        browseRequests[1].referenceTypeId = ""; // all reference types (NULL?)
        browseRequests[1].includeSubtypes = true;

        browseRequests[2].nodeId = "ns=0;i=20001"; // Root/Objects/scalar
        browseRequests[2].direction= 0; // forward
        browseRequests[2].referenceTypeId = ""; // all reference types (NULL?)
        browseRequests[2].includeSubtypes = true;

        browseRequests[0].nodeId = "ns=0;i=20008"; // Root/Objects/scalar/wo
        browseRequests[0].direction= 0; // forward
        browseRequests[0].referenceTypeId = ""; // all reference types (NULL?)
        browseRequests[0].includeSubtypes = true;

        res = SOPC_ClientHelper_Browse(connectionId, browseRequests, nbBrowse, browseResults);

        for (size_t j = 0; j < nbBrowse; j++)
        {
            printf("======================== %d ==============================\n", (int) j);
            printf("status: %d, nbOfResults: %d\n", browseResults[j].statusCode, browseResults[j].nbOfReferences);
            for (int32_t i = 0; i < browseResults[j].nbOfReferences; i++)
            {
                printf("nodeId: %s\n", browseResults[j].references[i].nodeId);
                printf("displayName: %s\n", browseResults[j].references[i].displayName);

                free(browseResults[j].references[i].nodeId);
                free(browseResults[j].references[i].displayName);
                free(browseResults[j].references[i].browseName);
                free(browseResults[j].references[i].referenceTypeId);
            }
            printf("==========================================================\n");
            free(browseResults[j].references);
        }

        free(browseRequests);
        free(browseResults);
    }

    if (res == 0)
    {
        //Read values
        sleep(3);
        SOPC_ClientHelper_ReadValue* readValues = (SOPC_ClientHelper_ReadValue*) malloc(sizeof(SOPC_ClientHelper_ReadValue) * (size_t) options.node_ids_size);
        assert(NULL != readValues);
        for (int i = 0; i < options.node_ids_size; i++)
        {
            readValues[i].nodeId = malloc(sizeof(char) * (strlen(options.node_ids[i]) + 1));
            assert(NULL != readValues[i].nodeId);
            strcpy(readValues[i].nodeId, options.node_ids[i]);
            readValues[i].attributeId = 13; // value
            readValues[i].indexRange = NULL;
        }

        SOPC_DataValue** readDataValues = malloc((size_t) options.node_ids_size * sizeof(SOPC_DataValue*));
        assert(NULL != readDataValues);

        res = SOPC_ClientHelper_Read(connectionId, readValues, (size_t) options.node_ids_size, readDataValues);

        for (int i = 0; i < options.node_ids_size; i++)
        {
            SOPC_DataValue* value = readDataValues[i];
            if (NULL == value)
            {
                printf("NULL\n");
            }
            else {
                // TODO is there a generic function to print/format a variant ?
                SOPC_Variant variant = value->Value;
                if (SOPC_Boolean_Id == variant.BuiltInTypeId)
                {
                    printf("%s\n", (bool) variant.Value.Boolean ? "true" : "false");
                }
                else if (SOPC_UInt64_Id == variant.BuiltInTypeId)
                {
                    printf("%ld\n",(int64_t) variant.Value.Uint64);
                }
            }
        }
        for (int i = 0; i < options.node_ids_size; i++)
        {
            free(readValues[i].nodeId);
            free(readDataValues[i]);
        }
        free(readValues);
        free(readDataValues);
    }

    if (0 == res)
    {
        //Write values
        sleep(3);
        SOPC_ClientHelper_WriteValue* writeValues = (SOPC_ClientHelper_WriteValue*)
            malloc(sizeof(SOPC_ClientHelper_WriteValue) * (size_t) options.node_ids_size);
        assert(writeValues != NULL);
        SOPC_StatusCode* writeResults = (SOPC_StatusCode*) malloc(sizeof(SOPC_StatusCode) * (size_t) options.node_ids_size);
        assert(writeResults != NULL);
        for (int i = 0; i < options.node_ids_size; i++)
        {
            writeValues[i].nodeId = malloc(sizeof(char) * (strlen(options.node_ids[i]) + 1));
            assert(NULL != writeValues[i].nodeId);
            strcpy(writeValues[i].nodeId, options.node_ids[i]);
            writeValues[i].indexRange = NULL;
            writeValues[i].value = malloc(sizeof(SOPC_DataValue));
            assert(writeValues[i].value != NULL);
            SOPC_DataValue_Initialize(writeValues[i].value);
            writeValues[i].value->Value.DoNotClear = false;
            writeValues[i].value->Value.BuiltInTypeId = SOPC_UInt64_Id;
            writeValues[i].value->Value.ArrayType = SOPC_VariantArrayType_SingleValue;
            writeValues[i].value->Value.Value.Uint64 = 31;
        }
        res = SOPC_ClientHelper_Write(connectionId, writeValues, (size_t) options.node_ids_size, writeResults);
        for (int i = 0; i < options.node_ids_size; i++)
        {
            if (SOPC_STATUS_OK == writeResults[i] && 0 == res)
            {
                printf("Write OK\n");
            }
            else
            {
                printf("Write Failed\n");
            }
            free(writeValues[i].nodeId);
            free(writeValues[i].value);
        }
        free(writeValues);
        free(writeResults);
    }

    if (res == 0)
    {
        sleep(10);
        SOPC_ClientHelper_Unsubscribe(connectionId);
    }


    if (connectionId > 0)
    {
        int32_t discoRes = SOPC_ClientHelper_Disconnect(connectionId);
        res = res != 0 ? res : discoRes;
    }

    SOPC_ClientHelper_Finalize();

    free_options(&options);

    return res;
}

#define FOREACH_OPT(x)                                                                                            \
    /* name of flag, is flag required, does flag requires argument, internal flag value, field of the C struct */ \
    x("endpoint", false, required_argument, OPT_ENDPOINT, endpoint_url)                                           \
        x("policyId", false, required_argument, OPT_POLICYID, policyId)                                           \
            x("username", false, required_argument, OPT_USERNAME, username)                                       \
                x("password", false, required_argument, OPT_PASSWORD, password)                                   \
                    x("publish-period", false, required_argument, OPT_PUBLISH_PERIOD, publish_period_str)         \
                        x("token-target", false, required_argument, OPT_TOKEN_TARGET, token_target_str)           \
                            x("max-keepalive-count", false, required_argument, OPT_KEEPALIVE, n_max_keepalive_str)

static bool parse_options(cmd_line_options_t* o, int argc, char* const* argv)
{
    memset(o, 0, sizeof(cmd_line_options_t));

#define OPT_DEFINITION(name, req, arg_req, val, field) {name, arg_req, NULL, val},

    static struct option long_options[] = {{"help", no_argument, NULL, OPT_HELP},
                                           {"disable-certificate-verification", no_argument, NULL, OPT_DISABLE_SECU},
                                           FOREACH_OPT(OPT_DEFINITION)};

#undef OPT_DEFINITION

#define STR_OPT_CASE(name, req, arg_req, val, field) \
    case val:                                        \
        free(o->field);                              \
        o->field = calloc(strlen(optarg) + 1, 1);    \
        strcpy(o->field, optarg);                    \
        break;

    bool parsed = false;
    while (!parsed)
    {
        switch (getopt_long(argc, argv, "", long_options, NULL))
        {
            FOREACH_OPT(STR_OPT_CASE)
        case -1:
            parsed = true;
            break;
        case OPT_DISABLE_SECU:
            o->disable_certificate_verification = true;
            break;
        case OPT_HELP:
            // fallthrough
        default:
            print_usage(argv[0]);
            return false;
        }
    }
#undef STR_OPT_CASE

#define CHECK_REQUIRED_STR_OPT(name, req, arg_req, val, field)                    \
    if (req && o->field == NULL)                                                  \
    {                                                                             \
        printf("Missing option: --" name ".\n"); \
        print_usage(argv[0]);                                                     \
        return false;                                                             \
    }

    FOREACH_OPT(CHECK_REQUIRED_STR_OPT)

#undef CHECK_REQUIRED_STR_OPT

#define CONVERT_STR_OPT(name, type, default_val)                                             \
    if (o->name##_str != NULL)                                                               \
    {                                                                                        \
        char* endptr;                                                                        \
        o->name = (type) strtoul(o->name##_str, &endptr, 10);                                \
                                                                                             \
        if (*endptr != '\0')                                                                 \
        {                                                                                    \
            printf("Invalid name: %s.\n", o->name##_str); \
            return false;                                                                    \
        }                                                                                    \
    }                                                                                        \
    else                                                                                     \
    {                                                                                        \
        o->name = default_val;                                                               \
    }

    if (NULL == o->endpoint_url)
    {
        o->endpoint_url = malloc(strlen(DEFAULT_ENDPOINT_URL) + 1);
        strcpy(o->endpoint_url, DEFAULT_ENDPOINT_URL);
    }
    if (NULL == o->policyId)
    {
        o->policyId = malloc(strlen(POLICY_ID) + 1);
        strcpy(o->policyId, POLICY_ID);
    }
    CONVERT_STR_OPT(publish_period, int64_t, PUBLISH_PERIOD_MS)
    CONVERT_STR_OPT(token_target, uint16_t, PUBLISH_N_TOKEN)
    CONVERT_STR_OPT(n_max_keepalive, uint32_t, MAX_KEEP_ALIVE_COUNT)

#undef CONVERT_STR_OPT

    o->node_ids_size = argc - optind;
    if (o->node_ids_size < 1)
    {
        printf("No node to subscribe to were specified.\n");
        print_usage(argv[0]);
        return false;
    }
    o->node_ids = malloc(sizeof(char*) * (size_t)(o->node_ids_size));
    if (NULL == o->node_ids)
    {
        printf("Out of memory.\n");
        return false;
    }
    for (int i = 0; i < o->node_ids_size; ++i)
    {
        o->node_ids[i] = malloc(strlen(argv[optind + i]) + 1);
        strcpy(o->node_ids[i], argv[optind + i]);
    }

    return true;
}

static void free_options(cmd_line_options_t* o)
{
    for (int i = 0; i < o->node_ids_size; ++i)
    {
        free(o->node_ids[i]);
    }
    free(o->node_ids);

#define FREE_STR_OPT_CASE(name, req, arg_req, val, field) free(o->field);

    FOREACH_OPT(FREE_STR_OPT_CASE)

#undef FREE_STR_OPT_CASE
}

#undef FOREACH_OPT

static void print_usage(const char* exe)
{
    printf("Usage: %s [options] NODE_ID [NODE_ID...]\n", exe);
    printf("Options:\n");
    printf("  --endpoint URL        URL of the endpoint to connect to\n");
    printf("  --policy-id POLICYID  Name of the selected UserIdentityToken policy id\n");
    printf("  --username UNAME      Username of the session user\n");
    printf("  --password PWD        Password of the session user\n");
    printf("  --publish-period MILLISEC  Subscription publish cycle, in ms\n");
    printf("  --token-target N      Number of PublishRequests available to the server\n");
    printf("  --disable-certificate-validation  For development only\n");
    printf("  --max-keepalive-count  Number of times an empty PublishResponse will not be sent\n");
    printf("\nNODE_ID are the nodes to subscribe to.\n");
}
