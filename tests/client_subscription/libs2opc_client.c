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
 * \brief A client library that supports and automates the subscription.
 *
 */

#include <stdbool.h>

#include "sopc_singly_linked_list.h"
#include "sopc_toolkit_config.h"
#include "sopc_user_app_itf.h"

#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_log_manager.h"
#include "sopc_toolkit_constants.h"
#include "sopc_types.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"

#include "state_machine.h"
#include "toolkit_helpers.h"

/* =========
 * Internals
 * =========
 */

/* Client structures */

/* Global library variables */
static bool bLibInitialized = false;
static bool bLibConfigured = false;
static SOPC_LibSub_LogCbk cbkLog = NULL;
static SOPC_LibSub_DisconnectCbk cbkDisco = NULL;
static SOPC_SLinkedList* pListConfig = NULL; /* IDs are configuration_id, value is SecureChannel_Config */
static SOPC_SLinkedList* pListClient = NULL; /* IDs are connection_id, value is a StaMac */

/* Event callback */
void ToolkitEventCallback(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext);

/* ==================
 * API implementation
 * ==================
 */

SOPC_LibSub_CstString SOPC_LibSub_GetVersion(void)
{
    return "Subscribe library v" SOPC_LIBSUB_VERSION " on S2OPC Toolkit v" SOPC_TOOLKIT_VERSION;
}

SOPC_ReturnStatus SOPC_LibSub_Initialize(const SOPC_LibSub_StaticCfg* pCfg)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pCfg || NULL == pCfg->host_log_callback || NULL == pCfg->disconnect_callback)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }
    else if (bLibInitialized || bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        pListConfig = SOPC_SLinkedList_Create(0);
        pListClient = SOPC_SLinkedList_Create(0);
        if (NULL == pListConfig || NULL == pListClient)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        cbkLog = pCfg->host_log_callback;
        cbkDisco = pCfg->disconnect_callback;
        bLibInitialized = true;
        status = SOPC_Toolkit_Initialize(ToolkitEventCallback);
    }

    if (!bLibInitialized)
    {
        /* Clean partial mallocs */
        SOPC_SLinkedList_Delete(pListConfig);
        pListConfig = NULL;
        SOPC_SLinkedList_Delete(pListClient);
        pListClient = NULL;
    }

    return status;
}

SOPC_ReturnStatus SOPC_LibSub_ConfigureConnection(const SOPC_LibSub_ConnectionCfg* pCfg,
                                                  SOPC_LibSub_ConfigurationId* pCfgId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_SecureChannel_Config* pscConfig = NULL;
    uint32_t cfgId = 0;

    if (!bLibInitialized || bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (SOPC_STATUS_OK == status && (NULL == pCfg || NULL == pCfgId))
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Create the new configuration */
    if (SOPC_STATUS_OK == status)
    {
        status = Helpers_NewSCConfigFromLibSubCfg(pCfg->server_url, pCfg->security_policy, pCfg->security_mode,
                                                  pCfg->path_cert_auth, pCfg->path_cert_srv, pCfg->path_cert_cli,
                                                  pCfg->path_key_cli, pCfg->path_crl, pCfg->sc_lifetime, &pscConfig);
    }

    /* Add it to the Toolkit */
    if (SOPC_STATUS_OK == status)
    {
        cfgId = SOPC_ToolkitClient_AddSecureChannelConfig(pscConfig);
        if (0 == cfgId)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* Append it to the internal list */
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_SLinkedList_Append(pListConfig, cfgId, pscConfig) == NULL)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    /* Handle the id to the client */
    if (SOPC_STATUS_OK == status)
    {
        *pCfgId = cfgId;
    }

    return status;
}

SOPC_ReturnStatus SOPC_LibSub_Configured(void)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!bLibInitialized || bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }
    else
    {
        status = SOPC_Toolkit_Configured();
        if (SOPC_STATUS_OK == status)
        {
            bLibConfigured = true;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_LibSub_Connect(const SOPC_LibSub_ConfigurationId cfg_id, SOPC_LibSub_ConnectionId* cli_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!bLibInitialized || !bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    status = SOPC_STATUS_NOK;
    return status;
}

SOPC_ReturnStatus SOPC_LibSub_AddToSubscription(const SOPC_LibSub_ConnectionId c_id, SOPC_LibSub_DataId* d_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!bLibInitialized || !bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    status = SOPC_STATUS_NOK;
    return status;
}

SOPC_ReturnStatus SOPC_LibSub_Disconnect(const SOPC_LibSub_ConnectionId c_id)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!bLibInitialized || !bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    status = SOPC_STATUS_NOK;
    return status;
}

/* ========================
 * Internal implementations
 * ========================
 */

void ToolkitEventCallback(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext) {}
