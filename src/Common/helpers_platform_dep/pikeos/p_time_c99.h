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

#ifndef PIKEOS_TIME_H
#define PIKEOS_TIME_H

#include <stdint.h>
#include <stdio.h>

struct tm
{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long __tm_gmtoff;
    const char* __tm_zone;
};

/* Represent time in second */
typedef int64_t time_t;

/************************************************
 *               Public API                      *
 *************************************************/

size_t strftime(char* strBuffer, size_t maxSize, const char* format, const struct tm* pTime);

/* MUSL implementation */

time_t mktime(struct tm* tp);

/* Timezone for the pikeos machine cannot be easily access, so we decide to not take in account Timezone.
 * This mean that localtime_r and gmtime_r has the same behavior and will fill struct tm as if you were in UTC + 0.
 * Impact in S2OPC code is minimal since absolute date time is only used to print logs. */
struct tm* localtime_r(const time_t* restrict t, struct tm* restrict tm);
struct tm* gmtime_r(const time_t* restrict t, struct tm* restrict tm);
struct tm* gmtime(const time_t* restrict t);

/************************************************
 *               Internal API                    *
 *************************************************/

int __month_to_secs(int, int);
long long __year_to_secs(long long, int*);
long long __tm_to_secs(const struct tm*);
int __secs_to_tm(long long, struct tm*);
void __secs_to_zone(long long, int, int*, long*, long*, const char**);

#endif // PIKEOS_TIME_H
