/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
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
