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
 * \brief A client executable using the client_subscription library.
 *
 */

#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs2opc_client.h"

/* Secure Channel configuration */
#define ENDPOINT_URL "opc.tcp://localhost:4841"
/* Security Policy is None or Basic256 or Basic256Sha256 */
#define SECURITY_POLICY SOPC_SecurityPolicy_None_URI
/* Security Mode is None or Sign or SignAndEncrypt */
#define SECURITY_MODE OpcUa_MessageSecurityMode_None

/* Connection global timeout */
#define TIMEOUT_MS 10000
/* Secure Channel lifetime */
#define SC_LIFETIME_MS 3600000
/* Publish period */
#define PUBLISH_PERIOD_MS 500
/* Number of targetted publish token */
#define PUBLISH_N_TOKEN 3

/* Path to the certificate authority */
#define PATH_CACERT_PUBL "./trusted/cacert.der"
/* Path to the server certificate */
#define PATH_SERVER_PUBL "./server_public/server_4k.der"
/* Path to the client certificate */
#define PATH_CLIENT_PUBL "./client_public/client_4k.der"
/* Path to the client private key */
#define PATH_CLIENT_PRIV "./client_private/client_4k.key"

/* Command line helpers and arguments */
enum
{
    OPT_HELP,
    OPT_ENDPOINT,
    OPT_USERNAME,
    OPT_PASSWORD,
    OPT_PUBLISH_PERIOD,
    OPT_TOKEN_TARGET,
} cmd_line_option_values_t;
typedef struct
{
    char* endpoint_url;
    char* username;
    char* password;
    char* publish_period_str;
    int64_t publish_period;
    char* token_target_str;
    uint16_t token_target;
    int node_ids_sz;
    char** node_ids;
} cmd_line_options_t;
static bool parse_options(cmd_line_options_t* o, int argc, char** argv);
static void print_usage(const char* exe);

/* usleep declaration. The unistd.h declaration is conditioned by unwanted defines. Consequently, this may fail. */
#include <unistd.h>
extern int usleep(__useconds_t __useconds);

/* Callbacks */
void log_callback(const SOPC_Log_Level log_level, SOPC_LibSub_CstString text);
void disconnect_callback(const SOPC_LibSub_ConnectionId c_id);
void datachange_callback(const SOPC_LibSub_ConnectionId c_id,
                         const SOPC_LibSub_DataId d_id,
                         const SOPC_LibSub_Value* value);

/* Main subscribing client */
int main(int argc, char* argv[])
{
    cmd_line_options_t options;
    if (!parse_options(&options, argc, argv))
    {
        return 1;
    }

    SOPC_LibSub_StaticCfg cfg_cli = {.host_log_callback = log_callback, .disconnect_callback = disconnect_callback};
    SOPC_LibSub_ConnectionCfg cfg_con = {.server_url = options.endpoint_url,
                                         .security_policy = SECURITY_POLICY,
                                         .security_mode = SECURITY_MODE,
                                         .path_cert_auth = PATH_CACERT_PUBL,
                                         .path_cert_srv = NULL,
                                         .path_cert_cli = NULL,
                                         .path_key_cli = NULL,
                                         .path_crl = NULL,
                                         .username = options.username,
                                         .password = options.password,
                                         .publish_period_ms = options.publish_period,
                                         .data_change_callback = datachange_callback,
                                         .timeout_ms = TIMEOUT_MS,
                                         .sc_lifetime = SC_LIFETIME_MS,
                                         .token_target = options.token_target};
    SOPC_LibSub_ConfigurationId cfg_id = 0;
    SOPC_LibSub_ConnectionId con_id = 0;
    SOPC_LibSub_DataId d_id = 0;

    Helpers_Log(SOPC_LOG_LEVEL_INFO, SOPC_LibSub_GetVersion());
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Connecting to \"%s\"", cfg_con.server_url);

    if (SOPC_STATUS_OK != SOPC_LibSub_Initialize(&cfg_cli))
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not initialize library.");
        return 1;
    }

    if (SOPC_STATUS_OK != SOPC_LibSub_ConfigureConnection(&cfg_con, &cfg_id))
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not configure connection.");
        return 2;
    }

    if (SOPC_STATUS_OK != SOPC_LibSub_Configured())
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not configure the toolkit.");
        return 3;
    }

    if (SOPC_STATUS_OK != SOPC_LibSub_Connect(cfg_id, &con_id))
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not connect with given configuration id.");
        return 4;
    }

    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Connected.");

    for (int i = 0; i < options.node_ids_sz; ++i)
    {
        if (SOPC_STATUS_OK !=
            SOPC_LibSub_AddToSubscription(con_id, options.node_ids[i], SOPC_LibSub_AttributeId_Value, &d_id))
        {
            Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Could not create monitored item.");
            return 5;
        }
        else
        {
            Helpers_Log(SOPC_LOG_LEVEL_INFO, "created MonIt for \"%s\" with data_id %" PRIu32 ".", options.node_ids[i],
                        d_id);
        }
    }

    usleep(10 * 1000000);
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Closing the connection.");
    SOPC_LibSub_Disconnect(con_id);
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Closing the Toolkit.");
    SOPC_LibSub_Clear();
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Toolkit closed.");

    return 0;
}

void log_callback(const SOPC_Log_Level log_level, SOPC_LibSub_CstString text)
{
    Helpers_LoggerStdout(log_level, text);
}

void disconnect_callback(const SOPC_LibSub_ConnectionId c_id)
{
    Helpers_Log(SOPC_LOG_LEVEL_INFO, "Client %" PRIu32 " disconnected.", c_id);
}

void datachange_callback(const SOPC_LibSub_ConnectionId c_id,
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

    log_callback(SOPC_LOG_LEVEL_INFO, sz);
}

static bool parse_options(cmd_line_options_t* o, int argc, char** argv)
{
    memset(o, 0, sizeof(cmd_line_options_t));

#define FOREACH_OPT(x)                                                                                            \
    /* name of flag, is flag required, does flag requires argument, internal flag value, field of the C struct */ \
    x("endpoint", false, required_argument, OPT_ENDPOINT, endpoint_url)                                           \
        x("username", false, required_argument, OPT_USERNAME, username)                                           \
            x("password", false, required_argument, OPT_PASSWORD, password)                                       \
                x("publish-period", false, required_argument, OPT_PUBLISH_PERIOD, publish_period_str)             \
                    x("token-target", false, required_argument, OPT_TOKEN_TARGET, token_target_str)

#define OPT_DEFINITION(name, req, arg_req, val, field) {name, arg_req, NULL, val},

    static struct option long_options[] = {{"help", no_argument, NULL, OPT_HELP}, FOREACH_OPT(OPT_DEFINITION)};

#undef OPT_DEFINITION

#define STR_OPT_CASE(name, req, arg_req, val, field) \
    case val:                                        \
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
        o->endpoint_url = ENDPOINT_URL;
    }
    CONVERT_STR_OPT(publish_period, int64_t, PUBLISH_PERIOD_MS)
    CONVERT_STR_OPT(token_target, uint16_t, PUBLISH_N_TOKEN)

#undef CONVERT_STR_OPT
#undef FOREACH_OPT

    o->node_ids_sz = argc - optind;
    if (o->node_ids_sz < 1)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "No node to subscribe to were specified.");
        print_usage(argv[0]);
        return false;
    }
    o->node_ids = malloc(sizeof(char*) * (size_t) o->node_ids_sz);
    if (NULL == o->node_ids)
    {
        Helpers_Log(SOPC_LOG_LEVEL_ERROR, "Out of memory.");
        return false;
    }
    for (int i = 0; i < o->node_ids_sz; ++i)
    {
        o->node_ids[i] = malloc(strlen(argv[optind + i]));
        strcpy(o->node_ids[i], argv[optind + i]);
    }

    return true;
}

static void print_usage(const char* exe)
{
    printf("Usage: %s [options] NODE_ID [NODE_ID...]\n", exe);
    printf("Options:\n");
    printf("  --endpoint URL      URL of the endpoint to connect to\n");
    printf("  --username UNAME    Username of the session user\n");
    printf("  --password PWD      Password of the session user\n");
    printf("  --publish-period MILLISEC  Subscription publish cycle, in ms\n");
    printf("  --token-target N    Number of PublishRequests available to the server\n");
    printf("\nNODE_ID are the nodes to subscribe to.\n");
}
