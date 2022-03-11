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
 * \brief A client executable using the s2opc_clientwrapper subscription lib
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

#include "libs2opc_client.h"

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
#define MAX_KEEP_ALIVE_COUNT 2
/* Lifetime Count of subscriptions */
#define MAX_LIFETIME_COUNT 1000
/* Number of targetted publish token */
#define PUBLISH_N_TOKEN 2

/* Path to the certificate authority */
#define PATH_CACERT_PUBL "./trusted/cacert.der"
/* Path to the CA CRL */
#define PATH_CACRL_PUBL "./revoked/cacrl.der"
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

/* Callbacks */
static void disconnect_callback(const SOPC_LibSub_ConnectionId c_id);
static void datachange_callback(const SOPC_LibSub_ConnectionId c_id,
                                const SOPC_LibSub_DataId d_id,
                                const SOPC_LibSub_Value* value);

/* Main subscribing client */
int main(int argc, char* const argv[])
{
    cmd_line_options_t options;
    if (!parse_options(&options, argc, argv))
    {
        free_options(&options);
        return 1;
    }

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_LibSub_StaticCfg cfg_cli = {.host_log_callback = Helpers_LoggerStdout,
                                     .disconnect_callback = disconnect_callback,
                                     .toolkit_logger = {.level = SOPC_LOG_LEVEL_INFO,
                                                        .log_path = "./client_subscription_logs/",
                                                        .maxBytes = 1048576,
                                                        .maxFiles = 50}};
    SOPC_LibSub_ConnectionCfg cfg_con = {.server_url = options.endpoint_url,
                                         .security_policy = SECURITY_POLICY,
                                         .security_mode = SECURITY_MODE,
                                         .disable_certificate_verification = options.disable_certificate_verification,
                                         .path_cert_auth = NULL,
                                         .path_cert_srv = NULL,
                                         .path_cert_cli = NULL,
                                         .path_key_cli = NULL,
                                         .path_crl = NULL,
                                         .policyId = options.policyId,
                                         .username = options.username,
                                         .password = options.password,
                                         .publish_period_ms = options.publish_period,
                                         .n_max_keepalive = options.n_max_keepalive,
                                         .n_max_lifetime = MAX_LIFETIME_COUNT,
                                         .data_change_callback = datachange_callback,
                                         .timeout_ms = TIMEOUT_MS,
                                         .sc_lifetime = SC_LIFETIME_MS,
                                         .token_target = options.token_target,
                                         .generic_response_callback = NULL,
                                         .expected_endpoints = NULL};
    SOPC_LibSub_ConfigurationId cfg_id = 0;
    SOPC_LibSub_ConnectionId con_id = 0;

    if (cfg_con.security_mode != OpcUa_MessageSecurityMode_None)
    {
        cfg_con.path_cert_srv = PATH_SERVER_PUBL;
        cfg_con.path_cert_cli = PATH_CLIENT_PUBL;
        cfg_con.path_key_cli = PATH_CLIENT_PRIV;
    }
    if (!cfg_con.disable_certificate_verification)
    {
        cfg_con.path_cert_auth = PATH_CACERT_PUBL;
        cfg_con.path_crl = PATH_CACRL_PUBL;
    }

    Helpers_Log(SOPC_LOG_LEVEL_INFO, SOPC_LibSub_GetVersion());
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Connecting to \"%s\"", cfg_con.server_url);

    if (SOPC_STATUS_OK != SOPC_LibSub_Initialize(&cfg_cli))
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not initialize library.");
        free_options(&options);
        return 2;
    }

    status = SOPC_LibSub_ConfigureConnection(&cfg_con, &cfg_id);
    if (SOPC_STATUS_OK != status)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not configure connection.");
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_LibSub_Configured();
        if (SOPC_STATUS_OK != status)
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not configure the toolkit.");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_LibSub_Connect(cfg_id, &con_id);
        if (SOPC_STATUS_OK != status)
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not connect with given configuration id.");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        Helpers_Log(SOPC_LOG_LEVEL_INFO, "Connected.");
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_LibSub_AttributeId* lAttrIds = calloc((size_t)(options.node_ids_size), sizeof(SOPC_LibSub_AttributeId));
        assert(NULL != lAttrIds);
        for (int i = 0; i < options.node_ids_size; ++i)
        {
            lAttrIds[i] = SOPC_LibSub_AttributeId_Value;
        }
        SOPC_LibSub_DataId* lDataId = calloc((size_t)(options.node_ids_size), sizeof(SOPC_LibSub_DataId));
        assert(NULL != lDataId);
        status = SOPC_LibSub_AddToSubscription(con_id, (const char* const*) options.node_ids, lAttrIds,
                                               options.node_ids_size, lDataId);
        if (SOPC_STATUS_OK != status)
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not create monitored items.");
        }
        else
        {
            for (int i = 0; i < options.node_ids_size; ++i)
            {
                Helpers_Log(SOPC_LOG_LEVEL_INFO, "Created MonIt for \"%s\" with data_id %" PRIu32 ".",
                            options.node_ids[i], lDataId[i]);
            }
        }
        free(lAttrIds);
        free(lDataId);
    }

    if (SOPC_STATUS_OK == status)
    {
        SOPC_Sleep(1000 * 1000);
    }

    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Closing the connections.");
    status = SOPC_LibSub_Disconnect(con_id);
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Closing the Toolkit.");
    SOPC_LibSub_Clear();
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Toolkit closed.");

    free_options(&options);

    if (SOPC_STATUS_OK != status)
    {
        return 3;
    }
    return 0;
}

static void disconnect_callback(const SOPC_LibSub_ConnectionId c_id)
{
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Client %" PRIu32 " disconnected.", c_id);
}

static void datachange_callback(const SOPC_LibSub_ConnectionId c_id,
                                const SOPC_LibSub_DataId d_id,
                                const SOPC_LibSub_Value* value)
{
    char sz[1024];
    size_t n;

    n = (size_t) snprintf(sz, sizeof(sz) / sizeof(sz[0]),
                          "Client %" PRIu32 " data change:\n  value id %" PRIu32 "\n  new value ", c_id, d_id);
    if (NULL == value || NULL == value->value)
    {
        snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, "NULL");
    }
    else if (SOPC_LibSub_DataType_bool == value->type)
    {
        snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, *(bool*) value->value ? "true" : "false");
    }
    else if (SOPC_LibSub_DataType_integer == value->type)
    {
        snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, "%" PRIi64, *(int64_t*) value->value);
    }
    else
    {
        snprintf(sz + n, sizeof(sz) / sizeof(sz[0]) - n, "%s", (SOPC_LibSub_CstString) value->value);
    }

    Helpers_LoggerStdout(SOPC_LOG_LEVEL_INFO, sz);
}

#define FOREACH_OPT(x)                                                                                            \
    /* name of flag, is flag required, does flag requires argument, internal flag value, field of the C struct */ \
    x("endpoint", false, required_argument, OPT_ENDPOINT, endpoint_url)                                           \
        x("policy-id", false, required_argument, OPT_POLICYID, policyId)                                          \
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
        assert(NULL != o->field);                    \
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

#define CHECK_REQUIRED_STR_OPT(name, req, arg_req, val, field)            \
    if (req && o->field == NULL)                                          \
    {                                                                     \
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Missing option: --" name "."); \
        print_usage(argv[0]);                                             \
        return false;                                                     \
    }

    FOREACH_OPT(CHECK_REQUIRED_STR_OPT)

#undef CHECK_REQUIRED_STR_OPT

#define CONVERT_STR_OPT(name, type, default_val)                                     \
    if (o->name##_str != NULL)                                                       \
    {                                                                                \
        char* endptr;                                                                \
        o->name = (type) strtoul(o->name##_str, &endptr, 10);                        \
                                                                                     \
        if (*endptr != '\0')                                                         \
        {                                                                            \
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Invalid name: %s.\n", o->name##_str); \
            return false;                                                            \
        }                                                                            \
    }                                                                                \
    else                                                                             \
    {                                                                                \
        o->name = default_val;                                                       \
    }

    if (NULL == o->endpoint_url)
    {
        o->endpoint_url = malloc(strlen(DEFAULT_ENDPOINT_URL) + 1);
        assert(NULL != o->endpoint_url);
        strcpy(o->endpoint_url, DEFAULT_ENDPOINT_URL);
    }
    if (NULL == o->policyId)
    {
        o->policyId = malloc(strlen(POLICY_ID) + 1);
        assert(NULL != o->policyId);
        strcpy(o->policyId, POLICY_ID);
    }
    CONVERT_STR_OPT(publish_period, int64_t, PUBLISH_PERIOD_MS)
    CONVERT_STR_OPT(token_target, uint16_t, PUBLISH_N_TOKEN)
    CONVERT_STR_OPT(n_max_keepalive, uint32_t, MAX_KEEP_ALIVE_COUNT)

#undef CONVERT_STR_OPT

    o->node_ids_size = argc - optind;
    if (o->node_ids_size < 1)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "No node to subscribe to were specified.");
        print_usage(argv[0]);
        return false;
    }
    o->node_ids = malloc(sizeof(char*) * (size_t)(o->node_ids_size));
    if (NULL == o->node_ids)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Out of memory.");
        return false;
    }
    for (int i = 0; i < o->node_ids_size; ++i)
    {
        o->node_ids[i] = malloc(strlen(argv[optind + i]) + 1);
        assert(NULL != o->node_ids[i]);
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
