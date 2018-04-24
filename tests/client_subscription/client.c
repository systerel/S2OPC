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

#include "libs2opc_client.h"
#include "sopc_crypto_profiles.h"

/* Secure Channel configuration */
#define ENDPOINT_URL "opc.tcp://localhost:4841"
/* Security Policy is None or Basic256 or Basic256Sha256 */
#define SECURITY_POLICY SOPC_SecurityPolicy_None_URI
/* Security Mode is None or Sign or SignAndEncrypt */
#define SECURITY_MODE OpcUa_MessageSecurityMode_None

/* Callbacks */
void log_callback(const s2opc_client_log_level_t log_level, cst_string_t text);
void disconnect_callback(const s2opc_client_connection_id_t c_id);

/* Main subscribing client */
int main(void)
{
    s2opc_client_static_cfg_t cfg_cli = {.host_log_callback = log_callback, .disconnect_callback = disconnect_callback};
    s2opc_client_connect_cfg_t cfg_con = {
        .server_url = ENDPOINT_URL,
        .timeout_ms = 10000 /* TODO: change timeout */,
        .identification_cfg.username = "foobar".identification_cfg.password = "foobar"};
    s2opc_client_connection_id_t con_id = 0;

    if (s2opc_client_no_error != s2opc_client_initialize(&cfg_cli))
    {
        log_callback(s2opc_client_log_error, "Could not initialize library");
        return 1;
    }

    if (s2opc_client_no_error != s2opc_client_configure_connection(&cfg_con, &con_id))
    {
        log_callback(s2opc_client_log_error, "Could not configure connection");
        return 2;
    }

    if (s2opc_client_no_error != s2opc_client_configured())
    {
        log_callback(s2opc_client_log_error, "Could not configure the toolkit");
        return 3;
    }

    if(s2opc_client_no_error != s2opc_client_connect()
    {
        log_callback(s2opc_client_log_error, "Could not configure the toolkit");
        return 4;
    }

    return 0;
}

void log_callback(const s2opc_client_log_level_t log_level, cst_string_t text)
{
    printf("Level %i: %s\n", log_level, text);
}

void disconnect_callback(const s2opc_client_connection_id_t c_id)
{
    char sz[128];

    snprintf(sz, sizeof(sz) / sizeof(sz[0]), "Client %" PRIi64 " disconnected", c_id);
    log_callback(s2opc_client_log_info, sz);
}
