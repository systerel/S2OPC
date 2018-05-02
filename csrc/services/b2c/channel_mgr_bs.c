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

/*------------------------
   Exported Declarations
  ------------------------*/
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "sopc_secure_channels_api.h"

#include "sopc_crypto_profiles.h"
#include "sopc_types.h"

#include "channel_mgr_bs.h"

#include "util_b2c.h"

#include "sopc_services_api.h"
#include "sopc_toolkit_config_internal.h"

#include "channel_mgr_1.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void channel_mgr_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void channel_mgr_bs__is_valid_channel_config_idx(const constants__t_channel_config_idx_i channel_mgr_bs__p_config_idx,
                                                 t_bool* const channel_mgr_bs__bres)
{
    SOPC_SecureChannel_Config* res = NULL;

    if (channel_mgr_bs__p_config_idx > 0 && channel_mgr_bs__p_config_idx <= constants__t_channel_config_idx_i_max)
    {
        res = SOPC_ToolkitClient_GetSecureChannelConfig((uint32_t) channel_mgr_bs__p_config_idx);
        if (NULL == res)
        {
            res = SOPC_ToolkitServer_GetSecureChannelConfig((uint32_t) channel_mgr_bs__p_config_idx);
        }
    }
    else
    {
        assert(false);
    }

    if (NULL == res)
    {
        *channel_mgr_bs__bres = false;
    }
    else
    {
        *channel_mgr_bs__bres = true;
    }
}

void channel_mgr_bs__is_valid_endpoint_config_idx(const constants__t_endpoint_config_idx_i channel_mgr_bs__p_config_idx,
                                                  t_bool* const channel_mgr_bs__bres)
{
    SOPC_Endpoint_Config* res = NULL;

    if (channel_mgr_bs__p_config_idx > 0 && channel_mgr_bs__p_config_idx <= constants__t_endpoint_config_idx_i_max)
    {
        res = SOPC_ToolkitServer_GetEndpointConfig((uint32_t) channel_mgr_bs__p_config_idx);
    }
    else
    {
        assert(false);
    }

    if (NULL == res)
    {
        *channel_mgr_bs__bres = false;
    }
    else
    {
        *channel_mgr_bs__bres = true;
    }
}

void channel_mgr_bs__prepare_cli_open_secure_channel(
    const constants__t_channel_config_idx_i channel_mgr_bs__p_config_idx)
{
    SOPC_SecureChannel_Config* config =
        SOPC_ToolkitClient_GetSecureChannelConfig((uint32_t) channel_mgr_bs__p_config_idx);
    if (NULL != config)
    {
        SOPC_SecureChannels_EnqueueEvent(SC_CONNECT, (uint32_t) channel_mgr_bs__p_config_idx, NULL, 0);
    }
    // else: will be checked in B model in next instruction and open avoided
}

void channel_mgr_bs__finalize_close_secure_channel(const constants__t_channel_i channel_mgr_bs__channel)
{
    SOPC_SecureChannels_EnqueueEvent(SC_DISCONNECT, (uint32_t) channel_mgr_bs__channel, NULL, 0);
}

void channel_mgr_bs__last_connected_channel_lost()
{
    SOPC_Services_EnqueueEvent(SE_TO_SE_SC_ALL_DISCONNECTED, 0, NULL, 0);
}

void channel_mgr_bs__send_channel_msg_buffer(const constants__t_channel_i channel_mgr_bs__channel,
                                             const constants__t_byte_buffer_i channel_mgr_bs__buffer,
                                             const constants__t_request_context_i channel_mgr_bs__request_context)
{
    SOPC_SecureChannels_EnqueueEvent(SC_SERVICE_SND_MSG, (uint32_t) channel_mgr_bs__channel, channel_mgr_bs__buffer,
                                     channel_mgr_bs__request_context);
}

extern void channel_mgr_bs__define_SecurityPolicy(const constants__t_channel_i channel_mgr_bs__p_channel)
{
    // Nothing to do since config is static, it is only for B model
    (void) channel_mgr_bs__p_channel;
}

extern void channel_mgr_bs__reset_SecurityPolicy(const constants__t_channel_i channel_mgr_bs__channel)
{
    // Nothing to do since config is static, it is only for B model
    (void) channel_mgr_bs__channel;
}

void channel_mgr_bs__get_SecurityPolicy(const constants__t_channel_i channel_mgr_bs__channel,
                                        constants__t_SecurityPolicy* const channel_mgr_bs__secpol)
{
    SOPC_SecureChannel_Config* pSCCfg = NULL;
    uint32_t scConfigIdx = 0;

    channel_mgr_1__get_channel_info(channel_mgr_bs__channel, (int32_t*) &scConfigIdx);

    pSCCfg = SOPC_ToolkitServer_GetSecureChannelConfig(scConfigIdx);
    if (pSCCfg == NULL)
    {
        pSCCfg = SOPC_ToolkitClient_GetSecureChannelConfig(scConfigIdx);
    }
    assert(pSCCfg != NULL);

    util_channel__SecurityPolicy_C_to_B(pSCCfg->reqSecuPolicyUri, channel_mgr_bs__secpol);
}

void channel_mgr_bs__channel_do_nothing(const constants__t_channel_i channel_mgr_bs__channel)
{
    (void) channel_mgr_bs__channel;
}
