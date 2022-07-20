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
 * PubSub publications require a time that has finer resolution than C99 time_t.
 * The OPC UA type *Duration* is a double representing ms delays between publications.
 * It's resolution goes finer than the nanosecond resolution.
 *
 * The SOPC_RealTime type should be defined to be as precise as possible to support sub-milliseconds publications.
 *
 * Under Linux, the implementation should use CLOCK_MONOTONIC, as we want to send regular notifications,
 * even if the system time gets back or forth (e.g. because of ntp).
 */

/**
 * \brief This file provides time related ZEPHYR-specific implementations which are not directly required by
 *          SOPC core.
 *          This file may be included by ZEPHYR applications that need these services.
 * \note The ZEPHYR PtP time correction is realized while processing specific time events (\a
 *          SOPC_Time_GetCurrentTimeUTC). Natively, in S2OPC core, this function may not be called as long as no
 *          PubSub is started. As a consequence, in the case an application waits for a PtP synchronization before
 *          activating this service, it is mandatory to firstly force this synchronization by calling this
 *          functions explicitly until synchronization is good enough.
 *          A simple way to do that could be calling \a SOPC_Time_GetCurrentTimeUTC function
 *          as long as \a SOPC_RealTime_GetClockPrecision does not return sufficient precision.
 */

#ifndef SOPC_ZEPHYR_TIME_H_
#define SOPC_ZEPHYR_TIME_H_

#include <stdbool.h>
#include <stdint.h>

#include "p_time.h"

/**
 * \brief Possible time sources
 */
typedef enum
{
    SOPC_TIME_TIMESOURCE_INTERNAL,  /**< Local system time, non-ptp synchronized */
    SOPC_TIME_TIMESOURCE_PTP_SLAVE, /**< PtP time (acting as SLAVE device) */
    SOPC_TIME_TIMESOURCE_PTP_MASTER /**< PtP time (acting as MASTER device) */
} SOPC_Time_TimeSource;

/**
 * \brief returns the current time source used for time-related operations
 * (including timestamping)
 */
SOPC_Time_TimeSource SOPC_Time_GetTimeSource(void);

/**
 * \brief returns an evaluation of the system clock precision (relatively to a remote PtP MASTER)
 *      This value can be used to check that PtP clock is sufficiently synchronized before allowing
 *      some time-related events, typically like sending Publisher messages with PublishOffset constraints.
 *      The threshold value may clearly be determined experimentally depending on expected precision.
 * \return A value between 0.0 and 1.0:
 *  - 1.0 for a perfectly synchronized PtP SLAVE (typically, current time source is \a SOPC_TIME_TIMESOURCE_PTP_SLAVE
 *          and remote PtP time is perfectly synchronized and stable with S2OPC internal clock)
 *  - 0.0 if there is no PtP synchronization (not PtP SLAVE or SLAVE but with clock failed to stabilize)
 */
float SOPC_RealTime_GetClockPrecision(void);

/**
 * \brief Provides the information about clock discrepancy towards a PtP master
 * \return clock ratio discrepancy:
 *  < 1.0 when local clock is faster than actual (PtP) time
 *  > 1.0 when local clock is slower than actual (PtP) time
 */
float SOPC_RealTime_GetClockCorrection(void);

#endif /* SOPC_ZEPHYR_TIME_H_ */
