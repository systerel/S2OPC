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

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* usleep */

#include "libs2opc_client.h"

/* Secure Channel configuration */
#define ENDPOINT_URL "opc.tcp://localhost:4841"
/* Security Policy is None or Basic256 or Basic256Sha256 */
#define SECURITY_POLICY SOPC_SecurityPolicy_None_URI
/* Security Mode is None or Sign or SignAndEncrypt */
#define SECURITY_MODE OpcUa_MessageSecurityMode_None
/* Node to subscribe to */
#define NODE_ID "s=Counter"

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

/* Callbacks */
void log_callback(const SOPC_Log_Level log_level, SOPC_LibSub_CstString text);
void disconnect_callback(const SOPC_LibSub_ConnectionId c_id);
void datachange_callback(const SOPC_LibSub_ConnectionId c_id,
                         const SOPC_LibSub_DataId d_id,
                         const SOPC_LibSub_Value* value);

/* Main subscribing client */
int main(void)
{
    SOPC_LibSub_StaticCfg cfg_cli = {.host_log_callback = log_callback, .disconnect_callback = disconnect_callback};
    SOPC_LibSub_ConnectionCfg cfg_con = {.server_url = ENDPOINT_URL,
                                         .security_policy = SECURITY_POLICY,
                                         .security_mode = SECURITY_MODE,
                                         .path_cert_auth = PATH_CACERT_PUBL,
                                         .path_cert_srv = NULL,
                                         .path_cert_cli = NULL,
                                         .path_key_cli = NULL,
                                         .path_crl = NULL,
                                         .username = NULL,
                                         .password = NULL,
                                         .publish_period_ms = PUBLISH_PERIOD_MS,
                                         .data_change_callback = datachange_callback,
                                         .timeout_ms = TIMEOUT_MS,
                                         .sc_lifetime = SC_LIFETIME_MS,
                                         .token_target = PUBLISH_N_TOKEN};
    SOPC_LibSub_ConfigurationId cfg_id = 0;
    SOPC_LibSub_ConnectionId con_id = 0;
    SOPC_LibSub_DataId d_id = 0;

    log_callback(SOPC_LOG_LEVEL_INFO, SOPC_LibSub_GetVersion());

    if (SOPC_STATUS_OK != SOPC_LibSub_Initialize(&cfg_cli))
    {
        log_callback(SOPC_LOG_LEVEL_ERROR, "Could not initialize library");
        return 1;
    }

    if (SOPC_STATUS_OK != SOPC_LibSub_ConfigureConnection(&cfg_con, &cfg_id))
    {
        log_callback(SOPC_LOG_LEVEL_ERROR, "Could not configure connection");
        return 2;
    }

    if (SOPC_STATUS_OK != SOPC_LibSub_Configured())
    {
        log_callback(SOPC_LOG_LEVEL_ERROR, "Could not configure the toolkit");
        return 3;
    }

    if (SOPC_STATUS_OK != SOPC_LibSub_Connect(cfg_id, &con_id))
    {
        log_callback(SOPC_LOG_LEVEL_ERROR, "Could not connect with given con_id");
        return 4;
    }

    printf("# Info: Connected.\n");

    if (SOPC_STATUS_OK != SOPC_LibSub_AddToSubscription(con_id, NODE_ID, SOPC_LibSub_AttributeId_Value, &d_id))
    {
        log_callback(SOPC_LOG_LEVEL_ERROR, "Could not create monitored item");
        return 5;
    }
    else
    {
        /* TODO: log */
        printf("# Info: created MonIt for \"%s\" with data_id %" PRIu32 ".\n", NODE_ID, d_id);
    }

    usleep(10 * 1000000);
    SOPC_LibSub_Clear();

    return 0;
}

void log_callback(const SOPC_Log_Level log_level, SOPC_LibSub_CstString text)
{
    printf("Level %i: %s\n", log_level, text);
}

void disconnect_callback(const SOPC_LibSub_ConnectionId c_id)
{
    char sz[128];

    snprintf(sz, sizeof(sz) / sizeof(sz[0]), "Client %" PRIu32 " disconnected", c_id);
    log_callback(SOPC_LOG_LEVEL_INFO, sz);
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
