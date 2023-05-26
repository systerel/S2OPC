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
 * \brief High level interface to configure an OPC UA client
 *
 */

#ifndef LIBS2OPC_CLIENT_CONFIG_H_
#define LIBS2OPC_CLIENT_CONFIG_H_

#include <stdbool.h>
#include "sopc_enums.h"
#include "sopc_user_app_itf.h"
/**
 * \brief Initialize the S2OPC client frontend configuration
 *        Call to ::SOPC_ClientConfigHelper_Initialize is required before any other operation
 *        and shall be done after a call to ::SOPC_CommonHelper_Initialize
 *
 * The default log configuration is provided by the ::SOPC_Common_GetDefaultLogConfiguration function. \n
 * By default, the log configuration is : \n
 *  .logLevel     = \a SOPC_LOG_LEVEL_INFO \n
 *  .logSystem    = \a SOPC_LOG_SYSTEM_FILE \n
 *  .logSysConfig = {.fileSystemLogConfig = {.logDirPath = "", .logMaxBytes = 1048576, .logMaxFiles = 50}}
 *
 * \result SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_STATE in case of double initialization.
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_Initialize(void);

/**
 * \brief Clear the S2OPC client frontend configuration
 *        It shall be done before a call to ::SOPC_CommonHelper_Clear
 */
void SOPC_ClientConfigHelper_Clear(void);

/**
 * \brief
 *   Structure reserved for future use in order to custom the configuration through XML.
 *   e.g.: PKI provider alternative, etc.
 */
typedef struct SOPC_ConfigClientXML_Custom SOPC_ConfigClientXML_Custom;

/**
 * \brief Configure client from XML configuration files for: client connections
 *
 * If not used or used partially, see libs2opc_client_config_custom.h to manually configure through API.
 *
 * \param clientConfigPath        Path to server configuration XML file (s2opc_clientserver_config.xsd schema)
 * \param customConfig            Shall be NULL. Reserved for future customization of configuration from XML
 *                                (PKI provider, etc.).
 * \param nbScConfigs[out]        Number of secure connection configurations parsed in the XML configuration
 * \param scConfig[out]           Pointer to the array of secure connection configurations of length \p nbScConfigs.
 *
 *
 * \return     SOPC_STATUS_OK in case of success,
 *             otherwise SOPC_STATUS_INVALID_PARAMETERS if a path is invalid or all paths are NULL or
 *             SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *             (toolkit not initialized, server already started).
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_ConfigureFromXML(const char* clientConfigPath,
                                                           SOPC_ConfigClientXML_Custom* customConfig,
                                                           size_t* nbScConfigs,
                                                           SOPC_SecureConnection_Config*** scConfigArray);

/**
 * \brief Returns the secure connection configuration which has the given user defined identifier.
 *        The user defined identifier is the id attribute of the connection in XML configuration.
 *
 * \note If several connection have the same identifier, the first match will be returned.
 *
 * \param userDefinedId  The identifier searched for in secure connections configured
 *
 * \return The (first) secure connection configuration for which configured id match \p userDefinedId
 *         or NULL if none is found.
 *
 */
SOPC_SecureConnection_Config* SOPC_ClientConfigHelper_GetConfigFromId(const char* userDefinedId);

/**
 * \brief Type of callback to provide asynchronous service response
 *
 * \param response     An asynchronous response to a local service request sent using
 *                     ::SOPC_ClientHelperNew_ServiceAsync
 *                     Response will be a pointer to the service response corresponding to sent request:
 *                     \li ::OpcUa_ReadResponse
 *                     \li ::OpcUa_WriteResponse
 *                     \li ::OpcUa_BrowseResponse
 *                     \li ::OpcUa_GetEndpointsResponse
 *                     \li ::OpcUa_FindServersResponse
 *                     \li ::OpcUa_FindServersOnNetworkResponse
 *                     \li ::OpcUa_RegisterServer2Response
 *
 *                     In case of service failure the response type is always ::OpcUa_ServiceFault,
 *                     in this case the \c response.encodeableType points to ::OpcUa_ServiceFault_EncodeableType
 *                     and ::SOPC_IsGoodStatus(\c response.ResponseHeader.ServiceResult) is \c false.
 *                     In case of sending failure the response is NULL and only the userContext is provided.
 *
 * \param userContext  The context that was provided with the corresponding request provided on
 *                     ::SOPC_ClientHelperNew_ServiceAsync call
 *
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call (other instance of response, data change notification, etc.).
 */
typedef void SOPC_ServiceAsyncResp_Fct(SOPC_EncodeableType* type, const void* response, uintptr_t userContext);

/**
 * \brief Define the service response callback to be used.
 *
 * This is optional if not used or only synchronous version used.
 * This shall be defined before starting the server and using ::SOPC_ClientHelperNew_ServiceAsync.
 *
 * \param asyncRespCb  The service asynchronous response callback to be used
 *
 * \return SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS if \p asyncRespCb is invalid
 *         or SOPC_STATUS_INVALID_STATE if the configuration is not possible
 *         (toolkit not initialized, client connection initiated).
 *
 * \warning The callback function shall not do anything blocking or long treatment since it will block any other
 *          callback call (other instance of data change notification, service sync/async response, etc.).
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_SetServiceAsyncResponse(SOPC_ServiceAsyncResp_Fct* asyncRespCb);

/**
 * \brief Type of callback to retrieve username and password for session activation
 *
 * \param[out] outUserName   the newly allocated username which shall be used for session activation, it shall be a
 *                           zero-terminated string in case of success.
 * \param[out] outPassword   the newly allocated password which shall be a zero-terminated string in case
 *                           of success.
 *
 * \return true in case of success, otherwise false.
 *
 * \warning The implementation of the user callback must free the \p outUserName and  \p outPassword
 *          and set them it back to NULL in case of failure.
 */
typedef bool SOPC_GetClientUserNamePassword_Fct(char** outUserName, char** outPassword);

/**
 * \brief Defines the callback to retrieve the username and password to activate the client session.
 *        This is necessary if a username token type is used
 *        and shall be defined before starting client and loading its configuration.
 *
 * \param getClientUsernamePassword  The callback to retrieve the password associated to the given username
 *
 * \return  SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_STATE or SOPC_STATUS_INVALID_PARAMETERS otherwise.
 *
 * \note    This function must be called before the configuration of the secure channel.
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_SetUserNamePasswordCallback(
    SOPC_GetClientUserNamePassword_Fct* getClientUsernamePassword);

/**
 * \brief Type of callback to retrieve password for decryption of the user private key
 *
 * \param      userCertThumb user certificate thumbprint containing the public key paired
 *                           with the private key to decrypt
 * \param[out] outPassword   the newly allocated password which shall be a zero-terminated string in case
 *                           of success
 *
 * \return true in case of success, otherwise false.
 *
 * \warning The implementation of the callback must free the \p outPassword and set it back to NULL in case of failure.
 */
typedef bool SOPC_GetClientUserKeyPassword_Fct(const char* userCertThumb, char** outPassword);

/**
 * \brief Defines the callback to retrieve password for decryption of the user X509 token private key.
 *
 * This is optional but if used it shall be defined before starting client and loading its configuration.
 *
 * \param getClientX509userKeyPassword  The callback to retrieve the user X509 token private key password
 *
 * \return  SOPC_STATUS_OK in case of success, SOPC_STATUS_INVALID_STATE or SOPC_STATUS_INVALID_PARAMETERS otherwise.
 *
 * \note    This function must be called before the configuration of the secure channel.
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_SetUserKeyPasswordCallback(
    SOPC_GetClientUserKeyPassword_Fct* getClientX509userKeyPassword);

/**
 * \brief Type of callback to retrieve password for decryption of the client application private key
 *        or the user x509 token private key.
 *
 * \param[out] outPassword   out parameter, the newly allocated password which shall be a zero-terminated string in case
 *                           of success.
 *
 * \return true in case of success, otherwise false.
 *
 * \warning The implementation of the user callback must free the \p outPassword and set it back to NULL in case of
 * failure.
 */
typedef bool SOPC_GetPassword_Fct(char** outPassword);

/**
 * \brief Defines the callback to retrieve password for decryption of the client private key.
 *
 * This is optional but if used it shall be defined before starting client and loading its configuration.
 *
 * \param getClientKeyPassword  The callback to retrieve the password
 *
 * \return  SOPC_STATUS_OK in case of success, otherwise SOPC_STATUS_INVALID_PARAMETERS if \p getClientKeyPassword is
 *          invalid.
 *
 * \note    This function must be called before the configuration of the secure channel.
 */
SOPC_ReturnStatus SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(SOPC_GetPassword_Fct* getClientKeyPassword);

#endif /* LIBS2OPC_CLIENT_CONFIG_H_ */
