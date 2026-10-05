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

MACHINE
    subscription_diagnostics_bs

SEES
    constants

DEFINITIONS
    d_init ==
    currentSubscriptionCount := 0 ||
    cumulatedSubscriptionCount := 0 ||
    publishingIntervalCount := 0 ||
    a_publishingIntervalNbSubscriptions :( a_publishingIntervalNbSubscriptions = t_opcua_duration_i*{0})

ABSTRACT_VARIABLES
    currentSubscriptionCount,
    cumulatedSubscriptionCount,
    publishingIntervalCount,
    a_publishingIntervalNbSubscriptions

INVARIANT
    currentSubscriptionCount : NAT &
    cumulatedSubscriptionCount : NAT &

    // Number of subscriptions associated to each currently used publishing interval
    a_publishingIntervalNbSubscriptions : t_opcua_duration_i --> NAT &

    publishingIntervalCount = card(a_publishingIntervalNbSubscriptions|>>{0}) &
    currentSubscriptionCount <= cumulatedSubscriptionCount

INITIALISATION
    d_init

OPERATIONS

    subscription_created (p_publishInterval) =
    PRE
        p_publishInterval : t_opcua_duration_i &
        p_publishInterval /= c_opcua_duration_indet
    THEN
        currentSubscriptionCount := currentSubscriptionCount + 1 ||
        cumulatedSubscriptionCount := cumulatedSubscriptionCount + 1 ||
        a_publishingIntervalNbSubscriptions :=
            a_publishingIntervalNbSubscriptions <+ {p_publishInterval |-> a_publishingIntervalNbSubscriptions(p_publishInterval) + 1} ||
        publishingIntervalCount :=
            card(a_publishingIntervalNbSubscriptions <+ {p_publishInterval |-> a_publishingIntervalNbSubscriptions(p_publishInterval) + 1} |>> {0})
    END
    ;

    subscription_deleted (p_publishInterval) =
    PRE
        p_publishInterval : t_opcua_duration_i &
        p_publishInterval : dom(a_publishingIntervalNbSubscriptions) &
        currentSubscriptionCount > 0
    THEN
        currentSubscriptionCount := currentSubscriptionCount - 1 ||
        a_publishingIntervalNbSubscriptions :=
            a_publishingIntervalNbSubscriptions <+ { p_publishInterval |-> a_publishingIntervalNbSubscriptions(p_publishInterval) - 1} ||
        publishingIntervalCount :=
            card(a_publishingIntervalNbSubscriptions <+ { p_publishInterval |-> a_publishingIntervalNbSubscriptions(p_publishInterval) - 1}|>>{0})
    END
    ;

    subscription_modified (p_oldPublishInterval, p_newPublishInterval) =
    PRE
        p_oldPublishInterval : t_opcua_duration_i &
        p_newPublishInterval : t_opcua_duration_i &
        p_oldPublishInterval : dom(a_publishingIntervalNbSubscriptions) &
        p_newPublishInterval : dom(a_publishingIntervalNbSubscriptions) &
        a_publishingIntervalNbSubscriptions(p_oldPublishInterval) > 0 &
        p_newPublishInterval /= c_opcua_duration_indet
    THEN
        IF p_oldPublishInterval /= p_newPublishInterval
        THEN
            a_publishingIntervalNbSubscriptions := a_publishingIntervalNbSubscriptions <+
                {p_oldPublishInterval |-> a_publishingIntervalNbSubscriptions(p_oldPublishInterval) - 1,
                 p_newPublishInterval |-> a_publishingIntervalNbSubscriptions(p_newPublishInterval) + 1}
        END
    END
    ;

    subscription_diagnostics_server_stopped =
    BEGIN
        d_init
    END
    ;

    subscription_diagnostics_bs_UNINITIALISATION =
    BEGIN
        d_init
    END

END
