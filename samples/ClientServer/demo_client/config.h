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

#include <stdbool.h>

#include "argparse.h"
#include "sopc_crypto_profiles.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"

/**
 * \brief Type of callback to retrieve a password.
 *
 * \param[out] outPassword   out parameter, the newly allocated password which shall be a zero-terminated string in case
 *                           of success.
 *
 * \return true in case of success, otherwise false.
 *
 * \warning The implementation of the user callback must free the \p outPassword and set it back to NULL in case of
 * failure.
 */
typedef bool Config_GetPassword_Fct(char** outPassword);

/* Connection configuration variables with default values */

/* Secure Channel configuration */
extern char* ENDPOINT_URL;
extern int NONE;
extern int ENCRYPT;

/* Secure Channel lifetime */
extern uint32_t SC_LIFETIME;

extern char* APPLICATION_NAME;
extern char* APPLICATION_URI;

extern char* PATH_CLIENT_PUBL;
extern char* PATH_CLIENT_PRIV;
extern char* PATH_SERVER_PUBL;
extern char* PATH_PKI_STORE;
extern char* PATH_ISSUED;

extern char* USER_POLICY_ID;
extern char* USER_NAME;
extern char* PATH_USER_PUBL;
extern char* PATH_USER_PRIV;

extern char* SESSION_NAME;

/* Options to include in command line tool for connection management */
extern struct argparse_option CONN_OPTIONS[16];

/* Active wait sleep, in ms */
#define SLEEP_LENGTH 200

/* The policyId of the chosen IdentityToken */
#define ANONYMOUS_POLICY_ID "anonymous"

/* For register server service only */
#define DEFAULT_APPLICATION_URI "urn:S2OPC:localhost"
#define DEFAULT_PRODUCT_URI "urn:S2OPC:localhost"
#define GATEWAY_SERVER_URI ""

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

/**
 * \brief Retrieve the serialized version of the user X509 token certificate and key
 *        for which paths have been provided through command line arguments.
 * \param[out] userX509cert  A reference to the pointer in which the serialized certificate will be set
 * \param[out] userX509key   A reference to the pointer in which the serialized key will be set.
 *                           If the key is expected to be encrypted the password will be asked interactively
 *                           unless ::Config_Client_SetUserKeyPassword_Fct has been called to do otherwise.
 *
 * \return true in case of success and false otherwise
 */
bool Config_Client_GetUserCertAndKey(SOPC_SerializedCertificate** userX509cert,
                                     SOPC_SerializedAsymmetricKey** userX509key);

/**
 * \brief Change the default callback which retrieve password for decryption of the client private key.
 *
 * \param getClientKeyPassword  The callback to retrieve the password
 *
 * \note   This function is useful for validation tests
 */
void Config_Client_SetClientKeyPassword_Fct(Config_GetPassword_Fct* getClientKeyPassword);

/**
 * \brief Change the default callback which retrieve password for decryption of the user private key.
 *
 * \param getUserKeyPassword  The callback to retrieve the password
 *
 */
void Config_Client_SetUserKeyPassword_Fct(Config_GetPassword_Fct* getUserKeyPassword);

/**
 * \brief Function to retrieve the user password when username token type is for establishing connection session.
 *        The user password is requested through the terminal.
 *
 * \return session user password as a zero-terminated string or NULL if no username defined or terminal input failed.
 *
 * \note   The returned C string must be freed by user.
 */
char* Config_Client_GetUserPassword(void);

#endif /* CONFIG_H_ */
