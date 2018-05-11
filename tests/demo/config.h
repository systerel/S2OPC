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
 * \brief A configuration file header, which gathers the static configuration of the demo clients.
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "sopc_crypto_profiles.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/* Secure Channel configuration */
#define ENDPOINT_URL "opc.tcp://localhost:4841"
/* Security Policy is None or Basic256 or Basic256Sha256 */
#define SECURITY_POLICY SOPC_SecurityPolicy_None_URI
/* Security Mode is None or Sign or SignAndEncrypt */
#define SECURITY_MODE OpcUa_MessageSecurityMode_None

/* Secure Channel lifetime */
#define SC_LIFETIME 60000
/* Active wait sleep, in ms */
#define SLEEP_LENGTH 200

#define PATH_CLIENT_PUBL "./client_public/client_4k_cert.der"
#define PATH_CLIENT_PRIV "./client_private/client_4k_key.pem"
#define PATH_SERVER_PUBL "./server_public/server_4k_cert.der"
#define PATH_CACERT_PUBL "./trusted/cacert.der"

/**
 * \brief   Creates a new SecureChannel configuration, which shall be freed with Config_DeleteSCConfig().
 *  When security policy is not None, the underlying certificates and PKI provider are created.
 *
 * \param reqSecuPolicyUri  The requested security policy URI.
 * \param msgSecurityMode   The requested security mode.
 *
 * \return  The created configuration
 */
SOPC_SecureChannel_Config* Config_NewSCConfig(const char* reqSecuPolicyUri, OpcUa_MessageSecurityMode msgSecurityMode);

/**
 * \brief   Frees the secure channel configuration, and the underlying certificates/PKI, if any.
 *
 * This function acts as a garbage collector: once the last configuration using certificates and PKI
 *  is freed, the function also frees the these certificates and PKI. Hence, references to these shall
 *  not be used after calls to Config_DeleteSCConfig.
 *
 * \param ppscConfig  The SecureChannel configuration to free.
 */
void Config_DeleteSCConfig(SOPC_SecureChannel_Config** ppscConfig);

#endif /* CONFIG_H_ */
