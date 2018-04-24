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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_log_manager.h"
#include "sopc_toolkit_constants.h"

#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"

/* Secure Channel configuration */
#define ENDPOINT_URL "opc.tcp://localhost:4841"
/* Security Policy is None or Basic256 or Basic256Sha256 */
#define SECURITY_POLICY SOPC_SecurityPolicy_None_URI
/* Security Mode is None or Sign or SignAndEncrypt */
#define SECURITY_MODE OpcUa_MessageSecurityMode_None

/* Callbacks */
void log_callback(const SOPC_Log_Level log_level, SOPC_LibSub_CstString text);
void disconnect_callback(const SOPC_LibSub_ConnectionId c_id);

/* Main subscribing client */
int main(void)
{
    SOPC_LibSub_StaticCfg cfg_cli = {.host_log_callback = log_callback, .disconnect_callback = disconnect_callback};
    SOPC_LibSub_ConnectionCfg cfg_con = {
        .server_url = ENDPOINT_URL,
        .timeout_ms = 10000 /* TODO: change timeout */,
        .identification_cfg.username = "foobar".identification_cfg.password = "foobar"};
    SOPC_LibSub_ConnectionId con_id = 0;

    if (SOPC_STATUS_OK != SOPC_LibSub_Initialize(&cfg_cli))
    {
        log_callback(SOPC_LOG_LEVEL_ERROR, "Could not initialize library");
        return 1;
    }

    if (SOPC_STATUS_OK != SOPC_LibSub_ConfigureConnection(&cfg_con, &con_id))
    {
        log_callback(SOPC_LOG_LEVEL_ERROR, "Could not configure connection");
        return 2;
    }

    if (SOPC_STATUS_OK != SOPC_LibSub_Configured())
    {
        log_callback(SOPC_LOG_LEVEL_ERROR, "Could not configure the toolkit");
        return 3;
    }

    if (SOPC_STATUS_OK != SOPC_LibSub_Connect())
    {
        log_callback(SOPC_LOG_LEVEL_ERROR, "Could not configure the toolkit");
        return 4;
    }

    return 0;
}

void log_callback(const SOPC_Log_Level log_level, SOPC_LibSub_CstString text)
{
    printf("Level %i: %s\n", log_level, text);
}

void disconnect_callback(const SOPC_LibSub_ConnectionId c_id)
{
    char sz[128];

    snprintf(sz, sizeof(sz) / sizeof(sz[0]), "Client %" PRIi64 " disconnected", c_id);
    log_callback(SOPC_log_info, sz);
}
