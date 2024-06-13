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
 * \brief This pubsub sample is used to benchmark publisher and subscriber internall process.
 *
 * Publisher :
 * The publisher emmits a NetworkMessage which is composed of 8 DataSetMessages. Each DataSetMessage is compose of 7
 * DataSetField which are UInt32 type.
 *
 * The main thread periodically updates the data sent by the publisher
 *
 * Subscriber :
 * The subscriber expects a NetworkMessage which is composed of 8 DataSetMessages. Each DataSetMessage must be compose
 * of 7 DataSetField of UInt32 type.
 *
 * The data is stored in cache
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pubsub.h"
#include "sopc_assert.h"

#define KIND_BENCHMARK_DEFAULT PUBLISHER_BENCH
#define MAX_BUFFER_SIZE 1024

typedef enum benchmark_kind_t
{
    PUBLISHER_BENCH,
    SUBSCRIBER_BENCH
} benchmark_kind_t;

benchmark_kind_t benchKind = KIND_BENCHMARK_DEFAULT;
int priority = 99;

char benchKindBuff[64] = {0};
/*************************************************/

static char* enum_bench_type_to_char(benchmark_kind_t var)
{
    int res = 0;
    if (var == PUBLISHER_BENCH)
    {
        res = sprintf(benchKindBuff, "PUBLISHER_BENCH");
        SOPC_ASSERT(res == strlen("PUBLISHER_BENCH"));
        return benchKindBuff;
    }
    else if (var == SUBSCRIBER_BENCH)
    {
        res = sprintf(benchKindBuff, "SUBSCRIBER_BENCH");
        SOPC_ASSERT(res == strlen("SUBSCRIBER_BENCH"));
        return benchKindBuff;
    }
    else
    {
        res = sprintf(benchKindBuff, "UNKNOWN");
        SOPC_ASSERT(res == strlen("UNKNOWN"));
        return benchKindBuff;
    }
}

/*************************************************/

static bool benchmark(void)
{
    // Initialize S2OPC toolkit
    bool res = PubSub_common_init();

    if (res)
    {
        if (PUBLISHER_BENCH == benchKind)
        {
            res = PubSub_publisher_bench(priority);
        }
        else if (SUBSCRIBER_BENCH == benchKind)
        {
            res = PubSub_subscriber_bench(priority);
        }
        else
        {
            printf("ERROR: Unknown kind of bench. Check help of program\n");
            res = false;
        }
    }
    PubSub_common_clear();
    return res;
}

/*************************************************/
static int check_priority(int prio)
{
    if (prio < 0 || prio > 99)
    {
        printf("value %d is not a priority accepted\n", prio);
        printf("Possible value are between 0 and 99 include\n");
        return -1;
    }
    else if (prio > 0)
    {
        uid_t uid = getuid();
        if (0 != uid)
        {
            printf("When thread priority is superior than 0 this program must be launch with sudo priviliege\n");
            return -2;
        }
    }
    return 0;
}

/*************************************************/

static void usage(char* fct_name)
{
    printf("%s [options] \n", fct_name);
    printf("\t\t\t launch S2OPC publisher or subscriber\n");
    printf("\n\t\t\t -k [pub|sub] kind of benchark launch publisher or a subscriber, by default %s\n",
           enum_bench_type_to_char(KIND_BENCHMARK_DEFAULT));
    printf("\t\t\t    notes: Subscriber benchmark create a publisher process running in a different CPU\n");
    printf("\t\t\t -h print this help message\n");
    printf("\t\t\t -p priority of the publisher/subscriber thread, by default %d\n", priority);
}

int main(int argc, char* argv[])
{
    char* progname;
    /* Process the command line arguments. */
    progname = strrchr(argv[0], '/');
    progname = progname ? 1 + progname : argv[0];
    int opt;
    char buffer[MAX_BUFFER_SIZE] = {0};
    while ((opt = getopt(argc, argv, "hk:p:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            usage(progname);
            return 0;
            break;
        case 'k':
            strncpy(buffer, optarg, MAX_BUFFER_SIZE - 1);
            break;
        case 'p':
            priority = atoi(optarg);
            break;
        case '?':
        default:
            usage(progname);
            return -1;
        }
    }

    size_t len = strlen(buffer);
    if (0 != len)
    {
        if (0 == strncmp(buffer, "pub", strlen("pub")))
        {
            benchKind = PUBLISHER_BENCH;
        }
        else if (0 == strncmp(buffer, "sub", strlen("sub")))
        {
            benchKind = SUBSCRIBER_BENCH;
        }
        else
        {
            printf("value %s not supported with -k option\n", buffer);
            printf("Possible values are pub or sub\n");
            return -1;
        }
    }

    int res = check_priority(priority);
    if (0 != res)
    {
        return res;
    }

    benchmark();
    return 0;
}
