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

#include "util_b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void subscription_core_bs__INITIALISATION(void) {}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void subscription_core_bs__compute_create_subscription_revised_params(
    const constants__t_session_i subscription_core_bs__p_session,
    const constants__t_opcua_duration_i subscription_core_bs__p_reqPublishInterval,
    const t_entier4 subscription_core_bs__p_reqLifetimeCount,
    const t_entier4 subscription_core_bs__p_reqMaxKeepAlive,
    const t_entier4 subscription_core_bs__p_maxNotificationsPerPublish,
    constants__t_opcua_duration_i* const subscription_core_bs__revisedPublishInterval,
    t_entier4* const subscription_core_bs__revisedLifetimeCount,
    t_entier4* const subscription_core_bs__revisedMaxKeepAlive,
    t_entier4* const subscription_core_bs__revisedMaxNotificationsPerPublish)
{
    (void) subscription_core_bs__p_session;
    if (subscription_core_bs__p_reqPublishInterval < SOPC_MIN_SUBSCRIPTION_INTERVAL_DURATION)
    {
        *subscription_core_bs__revisedPublishInterval = SOPC_MIN_SUBSCRIPTION_INTERVAL_DURATION;
    }
    else if (subscription_core_bs__p_reqPublishInterval > SOPC_MAX_SUBSCRIPTION_INTERVAL_DURATION)
    {
        *subscription_core_bs__revisedPublishInterval = SOPC_MAX_SUBSCRIPTION_INTERVAL_DURATION;
    }
    else
    {
        *subscription_core_bs__revisedPublishInterval = subscription_core_bs__p_reqPublishInterval;
    }

    if (subscription_core_bs__p_reqMaxKeepAlive < SOPC_MIN_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedMaxKeepAlive = SOPC_MIN_LIFETIME_PUBLISH_INTERVALS;
    }
    else if (subscription_core_bs__p_reqMaxKeepAlive > SOPC_MAX_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedMaxKeepAlive = SOPC_MAX_LIFETIME_PUBLISH_INTERVALS;
    }
    else
    {
        *subscription_core_bs__revisedMaxKeepAlive = subscription_core_bs__p_reqMaxKeepAlive;
    }

    if (subscription_core_bs__p_reqLifetimeCount < SOPC_MIN_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedLifetimeCount = SOPC_MIN_LIFETIME_PUBLISH_INTERVALS;
    }
    else if (subscription_core_bs__p_reqLifetimeCount > SOPC_MAX_LIFETIME_PUBLISH_INTERVALS)
    {
        *subscription_core_bs__revisedLifetimeCount = SOPC_MAX_LIFETIME_PUBLISH_INTERVALS;
    }
    else if (subscription_core_bs__p_reqLifetimeCount < 3 * *subscription_core_bs__revisedMaxKeepAlive)
    {
        *subscription_core_bs__revisedLifetimeCount = 3 * *subscription_core_bs__revisedMaxKeepAlive;
    }
    else
    {
        *subscription_core_bs__revisedLifetimeCount = subscription_core_bs__p_reqLifetimeCount;
    }

    if (subscription_core_bs__p_maxNotificationsPerPublish > SOPC_MAX_OPERATIONS_PER_MSG)
    {
        *subscription_core_bs__revisedMaxNotificationsPerPublish = SOPC_MAX_OPERATIONS_PER_MSG;
    }
    else
    {
        *subscription_core_bs__revisedMaxNotificationsPerPublish = subscription_core_bs__p_maxNotificationsPerPublish;
    }
}
