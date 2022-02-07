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

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>

#include "kernel.h"
#include "net/ethernet.h"
#include "net/net_if.h"
#include "net/socket.h"

#include "sopc_assert.h"
#include "sopc_mutexes.h"
#include "sopc_raw_sockets.h"
#include "sopc_threads.h"
#include "sopc_time.h"

#include "p_multicast.h"

#include "p_sockets.h"

// Uncomment one of the following section to allow/disable debug logs
#if 1
// NO DEBUG
#define P_MULTICAST_DEBUG(text, ...) ((void) text)
#define P_MULTICAST_DEBUG_ADDR_PRINT(text, pAddr, sock) ((void) text)

#else
// USE DEBUG
#define P_MULTICAST_DEBUG printk
#define P_MULTICAST_DEBUG_ADDR_PRINT(text, pAddr, sock)                   \
    do                                                                    \
    {                                                                     \
        char addr_str[32];                                                \
        inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));            \
        printk("\r\n" text ", Addr = %s, sock = %d\r\n", addr_str, sock); \
    } while (0)

#endif

/* Max socket based on max connections allowed by zephyr */

#ifdef CONFIG_NET_MAX_CONN
#define MAX_ZEPHYR_SOCKET (CONFIG_NET_MAX_CONN - 2)
#else
#define MAX_ZEPHYR_SOCKET 4
#endif

/* Constant definitions MAX_MCAST, based on value allowed by zephyr configuration */
#ifdef CONFIG_NET_IF_MCAST_IPV4_ADDR_COUNT
#define MAX_MCAST CONFIG_NET_IF_MCAST_IPV4_ADDR_COUNT
#else
#define MAX_MCAST 16
#endif

static Mutex priv_Mutex;

// *** Multicast variable definitions ***
typedef struct mCastRegistered
{
    int sock[MAX_ZEPHYR_SOCKET]; // Socket associated to mcast addr
    struct in_addr addr;         // Mcast addr
    bool bIsJoined;              // Mcast in use
} mCastRegistered;

static mCastRegistered tabMCast[MAX_MCAST];

// *** multicast private functions ***

static inline void P_MULTICAST_register_MCast(const struct in_addr* pAddr);
static inline void P_MULTICAST_unregister_MCast(const struct in_addr* pAddr);

void initialize(void)
{
    static bool priv_MulticastInitialized = false;

    P_MULTICAST_DEBUG("\r\nP_MCAST initialize called\r\n");
    if (priv_MulticastInitialized)
    {
        return;
    }

    for (uint32_t iIter = 0; iIter < MAX_MCAST; iIter++)
    {
        tabMCast[iIter].bIsJoined = false;
        tabMCast[iIter].addr.s_addr = 0;
        for (uint32_t iIterSock = 0; iIterSock < MAX_ZEPHYR_SOCKET; iIterSock++)
        {
            tabMCast[iIter].sock[iIterSock] = SOPC_INVALID_SOCKET;
        }
    }

    Mutex_Initialization(&priv_Mutex);
    priv_MulticastInitialized = true;
}

// *** Multicast private function definitions

// Register multicast address to L1 and associate socket to this one.
// Multicast address is added, if not already added, to ethernet interface (L2)
SOPC_ReturnStatus P_MULTICAST_join_or_leave_mcast_group(int32_t sock, const struct in_addr* pAddr, bool bJoin)
{
    mCastRegistered* mcastAddr = NULL;
    int* mcSock = NULL;

    initialize();

    P_MULTICAST_DEBUG("\r\nP_MULTICAST: Enter join_mcast_group\r\n");

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (SOPC_INVALID_SOCKET == sock || NULL == pAddr)
    {
        P_MULTICAST_DEBUG("\r\nP_MULTICAST: Enter join_mcast_group invalid socket\r\n");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!net_ipv4_is_addr_mcast(pAddr))
    {
        P_MULTICAST_DEBUG("\r\nP_MULTICAST: Enter join_mcast_group not mcast addr\r\n");
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    Mutex_Lock(&priv_Mutex);
    // Search for already registered mcast
    for (uint32_t i = 0; i < MAX_MCAST && (NULL == mcastAddr); i++)
    {
        if ((tabMCast[i].addr.s_addr == pAddr->s_addr) && (tabMCast[i].addr.s_addr != 0))
        {
            mcastAddr = &tabMCast[i];
        }
    }

    // If not registered mcast, add it if join request
    if (mcastAddr == NULL && bJoin)
    {
        for (uint32_t i = 0; i < MAX_MCAST && (NULL == mcastAddr); i++)
        {
            if (tabMCast[i].addr.s_addr == 0)
            {
                P_MULTICAST_DEBUG_ADDR_PRINT("P_SOCKET_UDP: add new mcast ip (L2) to allow bind socket", pAddr, sock);

                // Register L2 multicast to allow bind
                struct net_if* ptrNetIf = NULL;

#if defined(CONFIG_NET_L2_ETHERNET)
                ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
                if (ptrNetIf != NULL)
                {
                    net_if_ipv4_maddr_add(ptrNetIf, pAddr);
                }
#endif

                tabMCast[i].addr.s_addr = pAddr->s_addr;
                tabMCast[i].bIsJoined = false;

                for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
                {
                    tabMCast[i].sock[j] = SOPC_INVALID_SOCKET;
                }

                mcastAddr = &tabMCast[i];
            }
        }
    }

    // Search for already registered socket
    if (NULL != mcastAddr)
    {
        for (uint32_t j = 0; (j < MAX_ZEPHYR_SOCKET) && (mcSock == NULL); j++)
        {
            if ((mcastAddr->sock[j] == sock) && (mcastAddr->sock[j] != SOPC_INVALID_SOCKET))
            {
                mcSock = &mcastAddr->sock[j];
            }
        }
    }

    // If socket not found, add it
    if (mcSock == NULL && bJoin)
    {
        for (uint32_t j = 0; (j < MAX_ZEPHYR_SOCKET) && (mcSock == NULL); j++)
        {
            if (SOPC_INVALID_SOCKET == mcastAddr->sock[j])
            {
                mcastAddr->sock[j] = sock;
                mcSock = &mcastAddr->sock[j];

                P_MULTICAST_DEBUG_ADDR_PRINT("P_MULTICAST: register socket for mcast ip", pAddr, sock);
            }
        }
    }

    // Verify mcast is already registered by L1
    if ((NULL != mcastAddr) && (NULL != mcSock))
    {
        P_MULTICAST_DEBUG_ADDR_PRINT("P_MULTICAST: verify if mcast ip is already joined or not", pAddr, sock);

        // Join mcast not already joined by other socket and join request
        if (!mcastAddr->bIsJoined && bJoin)
        {
            P_MULTICAST_DEBUG_ADDR_PRINT("P_MULTICAST: register mcast ip to L1 (first register)", pAddr, sock);

            // Register L1 multicast
            P_MULTICAST_register_MCast(pAddr);
            mcastAddr->bIsJoined = true;
        }
        else
        {
            // Leave request, verify if at least one sock is joining this mcast.
            // If not, remove from L1 and L2
            if (!bJoin)
            {
                *mcSock = SOPC_INVALID_SOCKET;
                mcSock = NULL;

                for (uint32_t j = 0; (j < MAX_ZEPHYR_SOCKET) && (NULL == mcSock); j++)
                {
                    if (mcastAddr->sock[j] != SOPC_INVALID_SOCKET)
                    {
                        mcSock = &mcastAddr->sock[j];
                    }
                }
                P_MULTICAST_DEBUG_ADDR_PRINT("P_MULTICAST: remove socket for mcast ip", pAddr, sock);

                if (NULL == mcSock)
                {
                    P_MULTICAST_DEBUG_ADDR_PRINT("P_MULTICAST: remove mcast ip (L1 and L2) because no more joined",
                                                 pAddr, sock);

                    // Unregister L1 and L2 multicast
                    P_MULTICAST_unregister_MCast(pAddr);

                    mcastAddr->bIsJoined = false;
                    mcastAddr->addr.s_addr = 0;
                }
            }
        }
    }
    else
    {
        status = SOPC_STATUS_NOK;
    }

    Mutex_Unlock(&priv_Mutex);
    P_MULTICAST_DEBUG("\r\nP_MULTICAST: Exit join_mcast_group\r\n");

    return status;
}

// Check if address is a known multicast address registered with socket in parameter
bool P_MULTICAST_soft_filter(int32_t sock, const struct in_addr* pAddr)
{
    uint32_t indexMacast = 0;
    bool bIsFound = false;
    bool bSockIsFound = false;
    bool bResult = false;

    initialize();

    P_MULTICAST_DEBUG("\r\nP_MULTICAST: Enter verify mcast group\r\n");

    if (SOPC_INVALID_SOCKET == sock || NULL == pAddr)
    {
        return false;
    }

    if (!net_ipv4_is_addr_mcast(pAddr))
    {
        return false;
    }

    Mutex_Lock(&priv_Mutex);

    // Search mcast
    for (uint32_t i = 0; (i < MAX_MCAST) && (!bIsFound); i++)
    {
        if ((tabMCast[i].addr.s_addr == pAddr->s_addr) && (tabMCast[i].addr.s_addr != 0))
        {
            bIsFound = true;
            indexMacast = i;
        }
    }

    // Search sock
    if (bIsFound)
    {
        for (uint32_t j = 0; (j < MAX_ZEPHYR_SOCKET) && (!bSockIsFound); j++)
        {
            if ((tabMCast[indexMacast].sock[j] == sock) && (tabMCast[indexMacast].sock[j] != SOPC_INVALID_SOCKET))
            {
                bSockIsFound = true;
            }
        }
    }

    Mutex_Unlock(&priv_Mutex);
    bResult = (bIsFound && bSockIsFound);

    P_MULTICAST_DEBUG("\r\nP_MULTICAST: Exit verify mcast group\r\n");
    return bResult;
}

// Remove socket from all mcast.
void P_MULTICAST_remove_sock_from_mcast(int32_t sock)
{
    P_MULTICAST_DEBUG("\r\nP_MULTICAST: Enter remove_sock_from_mcast\r\n");
    if (SOPC_INVALID_SOCKET == sock)
    {
        return;
    }
    struct in_addr addr;
    addr.s_addr = 0;

    Mutex_Lock(&priv_Mutex);
    // Remove socket from mcast and remove mcast if necessary
    for (uint32_t i = 0; i < MAX_MCAST; i++)
    {
        for (uint32_t j = 0; j < MAX_ZEPHYR_SOCKET; j++)
        {
            if (tabMCast[i].addr.s_addr != 0)
            {
                (void) P_MULTICAST_join_or_leave_mcast_group(sock, &addr, false);
            }
        }
    }
    Mutex_Unlock(&priv_Mutex);
    P_MULTICAST_DEBUG("\r\nP_MULTICAST: Exit remove_sock_from_mcast\r\n");
}

static inline void P_MULTICAST_register_MCast(const struct in_addr* pAddr)
{
    (void) pAddr;

#if defined(CONFIG_NET_L2_ETHERNET)

    struct net_if* ptrNetIf = NULL;
    ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
    if (ptrNetIf != NULL)
    {
        struct net_if_mcast_addr* mcAddr = net_if_ipv4_maddr_add(ptrNetIf, pAddr);
        net_if_ipv4_maddr_join(mcAddr);
        SOPC_ASSERT(NULL != mcAddr);
        P_MULTICAST_DEBUG("\r\nP_MULTICAST: Call ethernet mcast\r\n");
    }

#endif
}

static inline void P_MULTICAST_unregister_MCast(const struct in_addr* pAddr)
{
    (void) pAddr;

#if defined(CONFIG_NET_L2_ETHERNET)

    struct net_if* ptrNetIf = NULL;
    ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
    if (ptrNetIf != NULL)
    {
        bool ret = net_if_ipv4_maddr_rm(ptrNetIf, pAddr);
        SOPC_ASSERT(ret);
    }

#endif
}
