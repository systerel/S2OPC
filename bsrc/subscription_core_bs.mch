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

MACHINE
    subscription_core_bs

SEES
    constants

OPERATIONS
    
    revisedPublishInterval, revisedLifetimeCount, revisedMaxKeepAlive, revisedMaxNotificationsPerPublish
      <-- compute_create_subscription_revised_params (p_session,
                                                      p_reqPublishInterval,
                                                      p_reqLifetimeCount,
                                                      p_reqMaxKeepAlive,
                                                      p_maxNotificationsPerPublish) =
    PRE
        p_session : t_session_i &
        p_session : t_session &
        p_reqPublishInterval : t_opcua_duration_i &
        p_reqLifetimeCount : t_counter &
        p_reqMaxKeepAlive : t_counter &
        p_maxNotificationsPerPublish : t_counter
    THEN
        revisedPublishInterval,
            revisedLifetimeCount,
            revisedMaxKeepAlive,
            revisedMaxNotificationsPerPublish
            :(revisedPublishInterval : t_opcua_duration_i & revisedPublishInterval /= c_opcua_duration_indet &
                revisedLifetimeCount : t_counter &
                revisedLifetimeCount > 0 &
                revisedMaxKeepAlive : t_counter &
                revisedMaxKeepAlive > 0 &
                revisedLifetimeCount >= 3 * revisedMaxKeepAlive & /* part 4 table 86 */
                revisedMaxNotificationsPerPublish : t_counter &
                revisedMaxNotificationsPerPublish > 0
            )
    END
    
END
