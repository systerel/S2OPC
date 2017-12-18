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

/**
 *  \file sopc_time.h
 *
 *  \brief Tools for time management
 */

#ifndef SOPC_TIME_H_
#define SOPC_TIME_H_

#include "sopc_builtintypes.h"

/**
 * Time reference type (milliseconds)
 */
typedef uint64_t SOPC_TimeReference;

/**
 *  \brief Suspend current thread execution for (at least) a microsecond interval
 *
 *  \param microsecs  The microsecond interval value for which execution must be suspended
 */
void SOPC_Sleep(unsigned int microsecs);

/**
 * \brief return the current time in DateTime format which is nanoseconds from 1601/01/01 00:00:00 UTC
 *
 * Note: since the clock is not monotonic, it should not be used to measure elapsed time
 *
 * \return the current time in DateTime format
 *
 */
SOPC_DateTime SOPC_Time_GetCurrentTimeUTC(void);

/**
 * \brief return the current time reference
 *
 * Note: clock is monotonic if SOPC_MONOTONIC_CLOCK is true
 *
 * \return the current time reference
 *
 */
SOPC_TimeReference SOPC_TimeReference_GetCurrent(void);

/**
 * \brief return the time reference corresponding to the given time reference incremented by the given duration in
 * milliseconds
 *
 * \param timeRef the time reference to be incremented
 * \param ms      the duration in milliseconds to use for increment
 *
 * \return the new time reference incremented by the given duration or with the maximum value in case of overflow
 *         or NULL incase timerRef == NULL or new memory allocation failed
 *
 */
SOPC_TimeReference SOPC_TimeReference_AddMilliseconds(SOPC_TimeReference timeRef, uint64_t ms);

/**
 * \brief return the comparison of given time references
 *
 * \param left  the left time reference operand (NULL pointer considered less than any other value)
 * \param right the right time reference operand (NULL pointer considered less than any other value)
 *
 * \return -1 if \p left < \p right operand, 0 if \p left = \p right and 1 if \p left > right
 *
 */
int8_t SOPC_TimeReference_Compare(SOPC_TimeReference left, SOPC_TimeReference right);

#endif /* SOPC_TIME_H_ */
