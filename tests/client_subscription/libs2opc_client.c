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

#include <assert.h>
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
static SOPC_SLinkedList* pListConfig = NULL; /* IDs are cfgId == Toolkit cfgScId, value is SOPC_LibSub_ConnectionCfg */
static SOPC_SLinkedList* pListClient = NULL; /* IDs are cliId, value is a StaMac */
static SOPC_LibSub_ConnectionId nCreatedClient = 0;

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

void SOPC_LibSub_Clear(void)
{
    /* TODO: clear configurations */
    /* TODO: clear connected clients */
    SOPC_Toolkit_Clear();
}

SOPC_ReturnStatus SOPC_LibSub_ConfigureConnection(const SOPC_LibSub_ConnectionCfg* pCfg,
                                                  SOPC_LibSub_ConfigurationId* pCfgId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_SecureChannel_Config* pscConfig = NULL;
    uint32_t cfgId = 0;
    SOPC_LibSub_ConnectionCfg* pCfgCpy = NULL;

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

    /* Copy it to append it safely to the internal list */
    if (SOPC_STATUS_OK == status)
    {
        pCfgCpy = (SOPC_LibSub_ConnectionCfg*) malloc(sizeof(SOPC_LibSub_ConnectionCfg));
        if (NULL == pCfgCpy)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
        else
        {
            *pCfgCpy = *pCfg;
        }
    }
    /* Append it to the internal list */
    if (SOPC_STATUS_OK == status)
    {
        if (SOPC_SLinkedList_Append(pListConfig, cfgId, pCfgCpy) == NULL)
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
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

SOPC_ReturnStatus SOPC_LibSub_Connect(const SOPC_LibSub_ConfigurationId cfgId, SOPC_LibSub_ConnectionId* pCliId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_LibSub_ConnectionCfg* pCfg = NULL;
    SOPC_StaMac_Machine* pSM = NULL;

    if (!bLibInitialized || !bLibConfigured || UINT32_MAX == nCreatedClient)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    if (NULL == pCliId)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Check the configuration Id */
    if (SOPC_STATUS_OK == status)
    {
        pCfg = SOPC_SLinkedList_FindFromId(pListConfig, cfgId);
        if (pCfg == NULL)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
            /* TODO: log */
        }
    }

    /* Creates a client state machine */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_Create(cfgId, pCfg->data_change_callback, (double) pCfg->publish_period_ms,
                                    pCfg->token_target, &pSM);
    }

    /* Adds it to the list */
    if (SOPC_STATUS_OK == status)
    {
        ++nCreatedClient;
        *pCliId = nCreatedClient;
        if (pSM != SOPC_SLinkedList_Append(pListClient, *pCliId, pSM))
        {
            status = SOPC_STATUS_OUT_OF_MEMORY;
        }
    }

    /* Starts the machine */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_StartSession(pSM);
    }

    /* Wait for the subscription to be created */
    /* TODO: use Mutex and CV */
    if (SOPC_STATUS_OK == status)
    {
        while (!SOPC_StaMac_IsError(pSM) && !SOPC_StaMac_HasSubscription(pSM))
        {
            usleep(1000);
        }
        if (SOPC_StaMac_IsError(pSM))
        {
            status = SOPC_STATUS_NOK;
        }
    }

    return status;
}

SOPC_ReturnStatus SOPC_LibSub_AddToSubscription(const SOPC_LibSub_ConnectionId cliId,
                                                SOPC_LibSub_CstString szNodeId,
                                                SOPC_LibSub_AttributeId attrId,
                                                SOPC_LibSub_DataId* pDataId)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    SOPC_StaMac_Machine* pSM = NULL;
    uintptr_t appCtx = 0;

    if (!bLibInitialized || !bLibConfigured)
    {
        status = SOPC_STATUS_INVALID_STATE;
    }

    /* Finds the state machine */
    if (SOPC_STATUS_OK == status)
    {
        pSM = SOPC_SLinkedList_FindFromId(pListClient, cliId);
        if (NULL == pSM)
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
    }

    /* Create the monitored item and wait for its creation */
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_StaMac_CreateMonitoredItem(pSM, szNodeId, attrId, &appCtx, pDataId);
    }
    if (SOPC_STATUS_OK == status)
    {
        while (!SOPC_StaMac_IsError(pSM) && !SOPC_StaMac_HasMonItByAppCtx(pSM, appCtx))
        {
            usleep(1000);
        }
        if (SOPC_StaMac_IsError(pSM))
        {
            status = SOPC_STATUS_NOK;
        }
    }

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

void ToolkitEventCallback(SOPC_App_Com_Event event, uint32_t IdOrStatus, void* param, uintptr_t appContext)
{
    SOPC_SLinkedListIterator pIterCli = NULL;
    SOPC_LibSub_ConnectionId cliId = 0;
    SOPC_LibSub_ConnectionId cliIdPro = 0; /* The cliId of the machine that processed the event */
    SOPC_StaMac_Machine* pSM = NULL;
    SOPC_StaMac_Machine* pSMPro = NULL; /* The state machine that processed the event */
    bool bProcessed = false;

    /* List through known clients and call state machine event callback */
    pIterCli = SOPC_SLinkedList_GetIterator(pListClient);
    while (NULL != pIterCli)
    {
        pSM = SOPC_SLinkedList_NextWithId(&pIterCli, &cliId);
        /* Only one machine shall process the event */
        if (SOPC_StaMac_EventDispatcher(pSM, NULL, event, IdOrStatus, param, appContext))
        {
            /* TODO: remove asserts, make log, remove assert.h */
            assert(!bProcessed);
            bProcessed = true;
            cliIdPro = cliId;
            pSMPro = pSM;
        }
    }

    /* At least one machine should have processed the event */
    assert(bProcessed);

    /* Post process the event. The only interesting event is for now CLOSED. */
    if (SE_CLOSED_SESSION == event)
    {
        /* The disconnect callback shall be called after the client has been destroyed */
        assert(pSMPro == SOPC_SLinkedList_RemoveFromId(pListClient, cliIdPro));
        SOPC_StaMac_Delete(&pSMPro);
        cbkDisco(cliIdPro);
    }
}
