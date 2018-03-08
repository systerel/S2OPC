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

#ifndef CONFIG_H_
#define CONFIG_H_

#include "sopc_crypto_profiles.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/* Secure Channel configuration */
#define ENDPOINT_URL "opc.tcp://localhost:4841"
/* Security Policy is None or Basic256 or Basic256Sha256 */
#define SECURITY_POLICY SOPC_SecurityPolicy_Basic256Sha256_URI
/* Security Mode is None or Sign or SignAndEncrypt */
#define SECURITY_MODE OpcUa_MessageSecurityMode_SignAndEncrypt

/* Secure Channel lifetime */
#define SC_LIFETIME 60000
/* Active wait sleep, in ms */
#define SLEEP_LENGTH 200

#define PATH_CLIENT_PUBL "./client_public/client_4k.der"
#define PATH_CLIENT_PRIV "./client_private/client_4k.key"
#define PATH_SERVER_PUBL "./server_public/serveur_4k.der"
#define PATH_CACERT_PUBL "./trusted/cacert.der"

SOPC_SecureChannel_Config* Config_NewSCConfig(const char* reqSecuPolicyUri, OpcUa_MessageSecurityMode msgSecurityMode);
void Config_DeleteSCConfig(SOPC_SecureChannel_Config** ppscConfig);

#endif /* CONFIG_H_ */
