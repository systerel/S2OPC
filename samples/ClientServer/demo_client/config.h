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
 * \brief A configuration file header, which gathers the static configuration of the demo clients.
 *
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "sopc_crypto_profiles.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/* Secure Channel configuration */
extern char* ENDPOINT_URL; // See config.c, possible override by main
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define GATEWAY_SERVER_URI ""

/* Secure Channel lifetime */
#define SC_LIFETIME 60000
/* Active wait sleep, in ms */
#define SLEEP_LENGTH 200

#define PATH_CLIENT_PUBL "./client_public/client_4k_cert.der"
#define PATH_CLIENT_PRIV "./client_private/client_4k_key.pem"
#define PATH_SERVER_PUBL "./server_public/server_4k_cert.der"
#define PATH_CACERT_PUBL "./trusted/cacert.der"
#define PATH_CACRL "./revoked/cacrl.der"

/* The policyId of the chosen IdentityToken */
#define ANONYMOUS_POLICY_ID "anonymous"

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
