/*
 *  Copyright (C) 2017 Systerel and others.
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

#ifndef SOPC_TOOLKIT_CONFIG_H_
#define SOPC_TOOLKIT_CONFIG_H_

#include "sopc_user_app_itf.h"

#include "sopc_builtintypes.h"
#include "sopc_namespace_table.h"
#include "sopc_event_dispatcher_manager.h"
#include "sopc_types.h"
#include "key_manager.h"

extern SOPC_EventDispatcherManager* servicesEventDispatcherMgr;

/* Client static configuration of a Secure Channel */
typedef struct SOPC_SecureChannel_Config {
    uint8_t                   isClientSc;
    const char*               url;
    const Certificate*        crt_cli;
    const AsymmetricKey*      key_priv_cli;
    const Certificate*        crt_srv;
    const PKIProvider*        pki;
    const char*               reqSecuPolicyUri;
    uint32_t                  requestedLifetime;
    OpcUa_MessageSecurityMode msgSecurityMode;
} SOPC_SecureChannel_Config;

#define SOPC_SECURITY_MODE_NONE_MASK 0x01
#define SOPC_SECURITY_MODE_SIGN_MASK 0x02
#define SOPC_SECURITY_MODE_SIGNANDENCRYPT_MASK 0x04
#define SOPC_SECURITY_MODE_ANY_MASK 0x07

typedef struct SOPC_SecurityPolicy
{
    SOPC_String securityPolicy; /**< Security policy URI supported */
    uint16_t    securityModes;  /**< Mask of security modes supported (use combination of SECURITY_MODE_*_MASK values) */
    void*       padding;        /**< Binary compatibility */
} SOPC_SecurityPolicy;

/* Server static configuration of a Endpoint listener */
typedef struct SOPC_Endpoint_Config{
    char*                endpointURL;
    Certificate*         serverCertificate;
    AsymmetricKey*       serverKey;
    PKIProvider*         pki;
    uint8_t              nbSecuConfigs;
    SOPC_SecurityPolicy* secuConfigurations;
} SOPC_Endpoint_Config;

/**
 *  \brief  Initialize the toolkit configuration, libraries and threads
 *
 *  \param pAppFct  Pointer to applicative code function in charge of toolkit communication events
 *
 *  \return STATUS_OK if initialization succeeded,
 *  STATUS_INVALID_PARAMETER if \p pAppFct == NULL or
 *  OpcUa_BadInvalidState if toolkit already initialized and
 *  STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_Toolkit_Initialize(SOPC_ComEvent_Fct* pAppFct);

/**
 *  \brief  Define toolkit configuration as configured and lock its state until toolkit clear operation
 *
 *  \return STATUS_OK if initialization succeeded,
 *  OpcUa_BadInvalidState if toolkit is not initialized or already
 *  configured
 */
SOPC_StatusCode SOPC_Toolkit_Configured();

/**
 *  \brief  Clear the stack configuration
 */
void SOPC_Toolkit_Clear();

/**
 *  \brief Set the given Address Space for the current toolkit server
 *  (SOPC_ToolkitServer_Initialize required, !SOPC_Toolkit_Configured required).
 *  Note: only one address space can be set, further call will be refused.
 *
 *  \param adressSpace  The address space definition
 *
 *  \return STATUS_OK if configuration succeeded,
 *  OpcUa_BadInvalidState if toolkit is not initialized, already
 *  configured or address space is already set, STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_ToolkitServer_SetAddressSpaceConfig(SOPC_AddressSpace* addressSpace);


/**
 *  \brief Record the given secure channel configuration in given index
 *  (SOPC_ToolkitClient_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \return secure channel configuration index if configuration succeeded,
 *  0 if toolkit is not initialized, already
 *  configured or otherwise
 */
uint32_t SOPC_ToolkitClient_AddSecureChannelConfig(SOPC_SecureChannel_Config* scConfig);


/**
 *  \brief Return the secure channel configuration for the given index or null if not defined. (SOPC_Toolkit_Configured required)
 *
 *  \param scConfigIdx  The secure channel configuration index requested
 *
 *  \return Secure channel configuration at given index or NULL if
 *  index invalid or toolkit is not configured yet
 */
SOPC_SecureChannel_Config* SOPC_ToolkitClient_GetSecureChannelConfig(uint32_t scConfigIdx);

/**
 *  \brief Return the endpoint configuration for the given index or null if not defined. (SOPC_Toolkit_Configured required)
 *
 *  \param epConfigIdx  The endpoint configuration index requested
 *
 *  \return Endpoint configuration at given index or NULL if
 *  index invalid or toolkit is not configured yet
 */
SOPC_Endpoint_Config* SOPC_ToolkitServer_GetEndpointConfig(uint32_t epConfigIdx);

/**
 *  \brief Record the given secure channel configuration in given index
 *  (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *  Note: it is forbidden to have 2 configurations with same endpointURL
 * 
 *  \return endpoint configuration index configuration succeeded,
 *  0 if toolkit is not initialized, already
 *  configured or otherwise
 */
uint32_t SOPC_ToolkitServer_AddEndpointConfig(SOPC_Endpoint_Config* config);

/**
 *  \brief Set the given namespace table configuration (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \param nsTable  The new namespace table to be used by the stack
 *
 *  \return STATUS_OK if configuration succeeded,
 *  OpcUa_BadInvalidState if toolkit is not initialized or already
 *  configured, STATUS_INVALID_PARAMETER if \p nsTable == NULL,
 *  STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_ToolkitConfig_SetNamespaceUris(SOPC_NamespaceTable* nsTable);

/**
 *  \brief Return the encodeable types table configuration used by the stack (SOPC_Toolkit_Configured required)
 *
 *  \return Encodeable types table (terminated by NULL value) or NULL if
 *  toolkit is not configured yet
 */
SOPC_EncodeableType** SOPC_ToolkitConfig_GetEncodeableTypes();


/**
 *  \brief Add the given encodeable types to the configuration (SOPC_Toolkit_Initialize required, !SOPC_Toolkit_Configured required)
 *
 *  \param encTypesTable  The encodeable types to add to the encodeable types configuration (NULL terminated)
 *  \param nbTypes        Number of encodeable types provided in the table
 *
 *  \return STATUS_OK if configuration succeeded,
 *  OpcUa_BadInvalidState if toolkit is not initialized or already
 *  configured, STATUS_INVALID_PARAMETER if \p encTypesTable == NULL,
 *  STATUS_NOK otherwise
 */
SOPC_StatusCode SOPC_ToolkitConfig_AddTypes(SOPC_EncodeableType** encTypesTable,
                                            uint32_t              nbTypes);

/**
 *  \brief Return the namespace table configuration  (SOPC_Toolkit_Configured required)
 *
 *  \return Namespace table or NULL if toolkit is not configured yet
 */
SOPC_NamespaceTable* SOPC_ToolkitConfig_GetNamespaces();

#endif /* SOPC_TOOLKIT_CONFIG_H_ */
