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

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "p_sopc_sockets.h"
#include "p_sopc_udp_sockets_custom.h"
#include "sopc_assert.h"
#include "sopc_atomic.h"
#include "sopc_date_time.h"
#include "sopc_helper_endianness_cfg.h"
#include "sopc_macros.h"
#include "sopc_network_layer.h"
#include "sopc_udp_sockets.h"

/* Publisher cycle time in nano seconds */
#define DEFAULT_CYCLE_TIME 1000000
/* Transmit wake delay */
#define DEFAULT_WAKE_DELAY 500000
#define DEFAULT_SO_PRIORITY 3
#define DEFAULT_MCAST_PORT "4840"
#define DEFAULT_MCAST_ADDR "232.1.2.100"
/* Ethernet interface selection */
#define DEFAULT_ETH_INTERFACE "enp1s0"

static int32_t stopPublisher = false;
static uint64_t cycle_time = DEFAULT_CYCLE_TIME;
static uint64_t wake_delay = DEFAULT_WAKE_DELAY;
static int so_priority = DEFAULT_SO_PRIORITY;
static char* interface = DEFAULT_ETH_INTERFACE;
static char* mcastPort = DEFAULT_MCAST_PORT;
static char* mcastAddr = DEFAULT_MCAST_ADDR;

static const SOPC_DataSet_LL_UadpDataSetMessageContentMask default_Uapd_DSM_Mask = {
    .validFlag = true,
    .fieldEncoding = DataSet_LL_FieldEncoding_Variant,
    .dataSetMessageSequenceNumberFlag = false,
    .statusFlag = false,
    .configurationVersionMajorVersionFlag = false,
    .configurationVersionMinorFlag = false,
    .dataSetMessageType = DataSet_LL_MessageType_KeyFrame,
    .timestampFlag = false,
    .picoSecondsFlag = false};

static void Test_StopSignal(int sig)
{
    /* avoid unused parameter compiler warning */
    SOPC_UNUSED_ARG(sig);

    if (SOPC_Atomic_Int_Get(&stopPublisher) != false)
    {
        exit(1);
    }
    else
    {
        SOPC_Atomic_Int_Set(&stopPublisher, true);
    }
}

static void Timestamp_Normalize(struct timespec* timestamp)
{
    while (timestamp->tv_nsec > 999999999)
    {
        timestamp->tv_sec += 1;
        timestamp->tv_nsec -= 1000000000;
    }

    while (timestamp->tv_nsec < 0)
    {
        timestamp->tv_sec -= 1;
        timestamp->tv_nsec += 1000000000;
    }
}

static SOPC_Dataset_LL_NetworkMessage* UDP_Pub_Test_Get_NetworkMessage(void)
{
    SOPC_Dataset_LL_NetworkMessage* nm = SOPC_Dataset_LL_NetworkMessage_Create(1, 1);
    SOPC_Dataset_LL_NetworkMessage_Header* header = SOPC_Dataset_LL_NetworkMessage_GetHeader(nm);
    SOPC_Dataset_LL_DataSetMessage* dsm = SOPC_Dataset_LL_NetworkMessage_Get_DataSetMsg_At(nm, 0);
    SOPC_Dataset_LL_NetworkMessage_SetVersion(header, 1);
    SOPC_Dataset_LL_DataSetMsg_Allocate_DataSetField_Array(dsm, 5);
    SOPC_Dataset_LL_NetworkMessage_Set_PublisherId_UInt32(header, 15300);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupId(nm, 1245);
    SOPC_Dataset_LL_NetworkMessage_Set_GroupVersion(nm, 963852);
    SOPC_Dataset_LL_DataSetMsg_Set_WriterId(dsm, 123);
    SOPC_Dataset_LL_DataSetMsg_Set_ContentMask(dsm, &default_Uapd_DSM_Mask);

    SOPC_Variant variant;
    // variant 1
    SOPC_Variant_Initialize(&variant);
    variant.BuiltInTypeId = SOPC_UInt32_Id;
    variant.ArrayType = SOPC_VariantArrayType_SingleValue;
    variant.Value.Uint32 = 12071982;
    bool res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, &variant, 0);
    SOPC_ASSERT(res);
    // variant 2
    SOPC_Variant_Initialize(&variant);
    variant.BuiltInTypeId = SOPC_Byte_Id;
    variant.ArrayType = SOPC_VariantArrayType_SingleValue;
    variant.Value.Byte = 239;
    res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, &variant, 1);
    SOPC_ASSERT(res);
    // variant 3
    SOPC_Variant_Initialize(&variant);
    variant.BuiltInTypeId = SOPC_UInt16_Id;
    variant.ArrayType = SOPC_VariantArrayType_SingleValue;
    variant.Value.Uint16 = 64852;
    res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, &variant, 2);
    SOPC_ASSERT(res);
    // variant 4
    SOPC_Variant_Initialize(&variant);
    variant.BuiltInTypeId = SOPC_DateTime_Id;
    variant.ArrayType = SOPC_VariantArrayType_SingleValue;
    variant.Value.Date = SOPC_Time_GetCurrentTimeUTC();
    res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, &variant, 3);
    SOPC_ASSERT(res);
    // variant 5
    SOPC_Variant_Initialize(&variant);
    variant.BuiltInTypeId = SOPC_UInt32_Id;
    variant.ArrayType = SOPC_VariantArrayType_SingleValue;
    variant.Value.Uint32 = 369852;
    res = SOPC_Dataset_LL_DataSetMsg_Set_DataSetField_Variant_At(dsm, &variant, 4);
    SOPC_ASSERT(res);

    return nm;
}

static void usage(char* progname)
{
    printf(
        " Usage : %s [options]\n"
        "  -i [name]          use network interface 'name' (default %s)\n"
        "  -c [num]           period in nanoseconds (default %d)\n"
        "  -d [num]           delta from wake up to txtime in nanoseconds (default %d)\n"
        "  -p [num]           set SO_PRIORITY to 'num' (default %d)\n"
        "  -h                 print this message and exit\n"
        "  -a [addr]          multicast address used (default %s)\n"
        "  -u [port]          udp port used (default %s)\n"
        "  \n",
        progname, DEFAULT_ETH_INTERFACE, DEFAULT_CYCLE_TIME, DEFAULT_WAKE_DELAY, DEFAULT_SO_PRIORITY,
        DEFAULT_MCAST_ADDR, DEFAULT_MCAST_PORT);
}

int main(int argc, char* argv[])
{
    // Install signal handler to close the server gracefully when server needs to stop
    signal(SIGINT, Test_StopSignal);
    signal(SIGTERM, Test_StopSignal);

    // Parse argument
    int opt = 0;
    char* progname = strrchr(argv[0], '/');
    progname = progname ? 1 + progname : argv[0];
    while ((opt = getopt(argc, argv, "hi:c:d:p:a:u:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            usage(progname);
            return 0;
            break;
        case 'i':
            interface = optarg;
            break;
        case 'c':
            cycle_time = (uint64_t) atol(optarg);
            break;
        case 'd':
            wake_delay = (uint64_t) atol(optarg);
            break;
        case 'p':
            so_priority = atoi(optarg);
            break;
        case 'a':
            mcastAddr = optarg;
            break;
        case 'u':
            mcastPort = optarg;
            break;
        case '?':
            usage(progname);
            return 1;
            break;
        default:
            printf("Unknown option %c\n", opt);
            usage(progname);
            return 1;
            break;
        }
    }

    uint64_t txtime = 0;
    int clockErr = 0;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_Socket sock = SOPC_INVALID_SOCKET;
    struct timespec sleepTime;
    struct pollfd sockErrPoll;

    SOPC_Helper_Endianness_Check();
    SOPC_Socket_AddressInfo* multicastAddr = SOPC_UDP_SocketAddress_Create(false, mcastAddr, mcastPort);
    status = SOPC_UDP_Socket_CreateToSend(multicastAddr, NULL, true, &sock);
    if (SOPC_STATUS_NOK == status)
    {
        return -1;
    }

    status = SOPC_UDP_SO_TXTIME_Socket_Option((const char*) interface, sock, (uint32_t) so_priority);
    if (SOPC_STATUS_INVALID_PARAMETERS == status)
    {
        printf("Invalid parameters\n");
        return -1;
    }
    else if (SOPC_STATUS_NOK == status)
    {
        printf("Failed to add TXTIME socket option\n");
        return -1;
    }

    SOPC_Dataset_LL_NetworkMessage* nm = UDP_Pub_Test_Get_NetworkMessage();
    if (NULL == nm)
    {
        return -1;
    }

    SOPC_Buffer* buffer = NULL;
    SOPC_Buffer* buffer_payload = NULL;
    SOPC_NetworkMessage_Error_Code errorCode =
        SOPC_UADP_NetworkMessage_Encode_Buffers(nm, NULL, &buffer, &buffer_payload);
    SOPC_ASSERT(SOPC_NetworkMessage_Error_Code_None == errorCode);
    SOPC_ASSERT(NULL != buffer);
    SOPC_ASSERT(NULL != buffer_payload);

    errorCode = SOPC_UADP_NetworkMessage_BuildFinalMessage(NULL, buffer, &buffer_payload);
    SOPC_ASSERT(SOPC_NetworkMessage_Error_Code_None == errorCode);
    SOPC_ASSERT(NULL != buffer);
    SOPC_ASSERT(NULL == buffer_payload);

    int returnCode = 0;
    /* Get current time and start 1 second in future and add wake tx delay */
    clock_gettime(CLOCKID, &sleepTime);
    sleepTime.tv_sec += 1;
    sleepTime.tv_nsec = ONE_SEC - (long) wake_delay;
    Timestamp_Normalize(&sleepTime);
    txtime = (uint64_t)(sleepTime.tv_sec * ONE_SEC + sleepTime.tv_nsec);
    txtime += wake_delay;
    memset(&sockErrPoll, 0, sizeof(sockErrPoll));
    sockErrPoll.fd = sock->sock;
    if (SOPC_STATUS_OK == status && buffer != NULL)
    {
        printf("\nFirst packet txtime %" PRIu64 "\n", txtime);
        while (0 == returnCode && SOPC_STATUS_OK == status && SOPC_Atomic_Int_Get(&stopPublisher) == false)
        {
            clockErr = clock_nanosleep(CLOCKID, TIMER_ABSTIME, &sleepTime, NULL);
            switch (clockErr)
            {
            case 0:
                status = SOPC_TX_UDP_send(sock, buffer->data, buffer->length, txtime, (const char*) mcastAddr,
                                          (const char*) mcastPort);
                if (status == SOPC_STATUS_NOK)
                {
                    printf("TX buffer send failed\n");
                }
                sleepTime.tv_nsec += (long) cycle_time;
                Timestamp_Normalize(&sleepTime);
                txtime += cycle_time;
                if (poll(&sockErrPoll, 1, 0) && sockErrPoll.revents & POLLERR) // 0x008
                {
                    if (SOPC_TX_UDP_Socket_Error_Queue(sock))
                    {
                        // Operation error identifier
                        returnCode = -ECANCELED;
                    }
                }
                break;
            case EINTR:
                continue;
            default:
                fprintf(stderr, "Clock_nanosleep returned %d: %s", clockErr, strerror(clockErr));
                returnCode = clockErr;
            }
        }
    }
    else
    {
        returnCode = 1;
    }

    SOPC_Dataset_LL_NetworkMessage_Delete(nm);
    SOPC_Buffer_Delete(buffer);
    SOPC_UDP_Socket_Close(&sock);
    SOPC_UDP_SocketAddress_Delete(&multicastAddr);

    return returnCode;
}
