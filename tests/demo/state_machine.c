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

#include <stdio.h>
#include <stdlib.h>

#include "sopc_toolkit_async_api.h"
#include "sopc_toolkit_config.h"

#include "config.h"
#include "state_machine.h"

StateMachine_Machine* StateMachine_Create(void)
{
    StateMachine_Machine* pSM = malloc(sizeof(StateMachine_Machine));

    if (NULL != pSM)
    {
        pSM->state = stInit;
        pSM->pscConfig = NULL;
        pSM->iscConfig = 0;
        pSM->iSession = 1;
        pSM->iSessionID = 0;
    }

    return pSM;
}

SOPC_ReturnStatus StateMachine_ConfigureToolkit(StateMachine_Machine* pSM)
{
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (NULL == pSM)
    {
        status = SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Configure the Toolkit */
    if (SOPC_STATUS_OK == status)
    {
        pSM->pscConfig = Config_NewSCConfig(SECURITY_POLICY, SECURITY_MODE);
        if (NULL == pSM->pscConfig)
        {
            status = SOPC_STATUS_NOK;
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        pSM->iscConfig = SOPC_ToolkitClient_AddSecureChannelConfig(pSM->pscConfig);
        if (0 == pSM->iscConfig)
        {
            status = SOPC_STATUS_NOK;
            printf("# Error: AddSecureChannelConfig failed.\n");
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        pSM->state = stWaitActivation;
        status = SOPC_Toolkit_Configured();
        if (SOPC_STATUS_OK == status)
        {
            printf("# Info: Toolkit configuration done.\n");
            printf("# Info: Opening Session.\n");
        }
        else
        {
            printf("# Error: Toolkit configuration failed.\n");
        }
    }

    /* Starts the async mode */
    if (SOPC_STATUS_OK == status)
    {
        SOPC_ToolkitClient_AsyncActivateSession(pSM->iscConfig, pSM->iSession);
    }

    return status;
}

bool StateMachine_IsOver(StateMachine_Machine* pSM)
{
    if (NULL != pSM)
    {
        switch (pSM->state)
        {
        case stInit:
        case stWaitActivation:
        case stWaitResponse:
            return false;
        case stWaitFinished:
        case stAbort:
        default:
            return true;
        }
    }

    return true;
}

void StateMachine_Delete(StateMachine_Machine** ppSM)
{
    if (NULL == ppSM || NULL == *ppSM)
    {
        return;
    }

    Config_DeleteSCConfig(&((*ppSM)->pscConfig));

    free(*ppSM);
    *ppSM = NULL;
}

void StateMachine_EventDispatcher(StateMachine_Machine* pSM,
                                  SOPC_App_Com_Event event,
                                  uint32_t arg,
                                  void* pParam,
                                  uintptr_t appCtx)
{
    (void) pParam;
    (void) appCtx;
    if (NULL == pSM)
    {
        return;
    }

    switch (pSM->state)
    {
    case stInit:
        printf("# Warning: Dispatching in init state machine state.\n");
        break;
    case stWaitActivation:
        switch (event)
        {
        case SE_ACTIVATED_SESSION:
            pSM->state = stWaitResponse;
            pSM->iSessionID = arg;
            printf("# Info: Session activated.\n");
            break;
        case SE_SESSION_ACTIVATION_FAILURE:
            pSM->state = stAbort;
            printf("# Error: Failed session activation.\n");
            break;
        default:
            pSM->state = stAbort;
            printf("# Error: In state Activation, unexpected event %i.\n", event);
            break;
        }
        break;
    case stWaitResponse:
        switch (event)
        {
        case SE_RCV_SESSION_RESPONSE:
            pSM->state = stWaitFinished;
            printf("# Info: Response received.\n");
            break;
        case SE_SND_REQUEST_FAILED:
            pSM->state = stAbort;
            printf("# Error: Send request failed.\n");
            break;
        default:
            pSM->state = stAbort;
            printf("# Error: In state WaitResponse, unexpected event %i.\n", event);
            break;
        }
        break;
        break;
    case stWaitFinished:
        printf("# Warning: Receiving event in wait finish state, ignoring.\n");
        break;
    case stAbort:
        printf("# Warning: Receiving event in abort state, ignoring.\n");
        break;
    default:
        pSM->state = stAbort;
        printf("# Error: Dispatching in unknown state %i, event %i.\n", pSM->state, event);
        break;
    }
}
