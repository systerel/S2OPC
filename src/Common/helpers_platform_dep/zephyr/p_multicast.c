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

#include "sopc_mutexes.h"
#include "sopc_raw_sockets.h"
#include "sopc_threads.h"
#include "sopc_time.h"

#include "p_multicast.h"

#include "p_sockets.h"

typedef enum E_MULTICAST_STATUS
{
    MULTCAST_STATUS_NOT_INITIALIZED,
    MULTCAST_STATUS_INITIALIZING,
    MULTCAST_STATUS_INITIALIZED,
    MULTCAST_STATUS_LOCKED,
    MULTCAST_STATUS_SIZE = INT32_MAX
} eMulticastStatus;

#define P_MULTICAST_DEBUG (0)

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

static volatile eMulticastStatus priv_MulticastStatus = MULTCAST_STATUS_NOT_INITIALIZED;

// *** Multicast variable definitions ***
typedef struct mCastRegistered
{
    int sock[MAX_ZEPHYR_SOCKET]; // Socket associated to mcast addr
    struct in_addr addr;         // Mcast addr
    bool bIsJoined;              // Mcast in use
} mCastRegistered __attribute__((aligned(4)));

static mCastRegistered tabMCast[MAX_MCAST];

// *** multicast private functions ***

static void P_MULTICAST_enet_add_mcast(struct net_if* ptrNetIf, struct in_addr* pAddr);
static void P_MULTICAST_enet_rm_mcast(struct net_if* ptrNetIf, struct in_addr* pAddr);

void P_MULTICAST_Initialize(void)
{
#if P_MULTICAST_DEBUG == 1
    printk("\r\nP_MCAST initialize called\r\n");
#endif

    eMulticastStatus expectedStatus = MULTCAST_STATUS_NOT_INITIALIZED;
    eMulticastStatus desiredStatus = MULTCAST_STATUS_INITIALIZING;

    bool bTransition = __atomic_compare_exchange(&priv_MulticastStatus, //
                                                 &expectedStatus,       //
                                                 &desiredStatus,        //
                                                 false,                 //
                                                 __ATOMIC_SEQ_CST,      //
                                                 __ATOMIC_SEQ_CST);     //

    if (bTransition)
    {
        for (uint32_t iIter = 0; iIter < MAX_MCAST; iIter++)
        {
            tabMCast[iIter].bIsJoined = false;
            tabMCast[iIter].addr.s_addr = 0;
            for (uint32_t iIterSock = 0; iIterSock < MAX_ZEPHYR_SOCKET; iIterSock++)
            {
                tabMCast[iIter].sock[iIterSock] = SOPC_INVALID_SOCKET;
            }
        }

        desiredStatus = MULTCAST_STATUS_INITIALIZED;
        __atomic_store(&priv_MulticastStatus, &desiredStatus, __ATOMIC_SEQ_CST);
    }

    __atomic_load(&priv_MulticastStatus, &expectedStatus, __ATOMIC_SEQ_CST);
    while (MULTCAST_STATUS_INITIALIZED > expectedStatus)
    {
        k_yield();
        __atomic_load(&priv_MulticastStatus, &expectedStatus, __ATOMIC_SEQ_CST);
    }
}

// *** Multicast private function definitions

// Register multicast address to L1 and associate socket to this one.
// Multicast address is added, if not already added, to ethernet interface (L2)
SOPC_ReturnStatus P_MULTICAST_join_or_leave_mcast_group(int32_t sock, struct in_addr* pAddr, bool bJoin)
{
    uint32_t indexMacast = 0;
    uint32_t indexMasock = 0;
    bool bIsFound = false;
    bool bSockIsFound = false;

    eMulticastStatus expectedStatus = MULTCAST_STATUS_INITIALIZED;
    eMulticastStatus desiredStatus = MULTCAST_STATUS_LOCKED;

#if P_MULTICAST_DEBUG == 1
    printk("\r\nP_MULTICAST: Enter join_mcast_group\r\n");
#endif

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (SOPC_INVALID_SOCKET == sock || NULL == pAddr)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (!net_ipv4_is_addr_mcast(pAddr))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    do
    {
        status = SOPC_STATUS_OK;
        expectedStatus = MULTCAST_STATUS_INITIALIZED;
        desiredStatus = MULTCAST_STATUS_LOCKED;

        bool bTransition = __atomic_compare_exchange(&priv_MulticastStatus, //
                                                     &expectedStatus,       //
                                                     &desiredStatus,        //
                                                     false,                 //
                                                     __ATOMIC_SEQ_CST,      //
                                                     __ATOMIC_SEQ_CST);     //

        if (bTransition)
        {
            // Search for already registered mcast
            for (uint32_t i = 0; i < MAX_MCAST && !bIsFound; i++)
            {
                if ((tabMCast[i].addr.s_addr == pAddr->s_addr) && (tabMCast[i].addr.s_addr != 0))
                {
                    bIsFound = true;
                    indexMacast = i;
                }
            }

            // If not registered mcast, add it if join request
            if (!bIsFound && bJoin)
            {
                for (uint32_t i = 0; i < MAX_MCAST && !bIsFound; i++)
                {
                    if (tabMCast[i].addr.s_addr == 0)
                    {
#if P_MULTICAST_DEBUG == 1
                        {
                            char addr_str[32];
                            inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));
                            printk(
                                "\r\nP_SOCKET_UDP: add new mcast ip = %s (L2) for to allow bind socket = %d"
                                "\r\n",   //
                                addr_str, //
                                sock      //
                            );            //
                        }
#endif
                        // Register L2 multicast to allow bind
                        struct net_if* ptrNetIf = NULL;

#if defined(CONFIG_NET_L2_ETHERNET)
                        ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
#endif
                        if (ptrNetIf != NULL)
                        {
                            net_if_ipv4_maddr_add(ptrNetIf, pAddr);
                        }

                        tabMCast[i].addr.s_addr = pAddr->s_addr;
                        tabMCast[i].bIsJoined = false;

                        for (int j = 0; j < MAX_ZEPHYR_SOCKET; j++)
                        {
                            tabMCast[i].sock[j] = SOPC_INVALID_SOCKET;
                        }

                        bIsFound = true;
                        indexMacast = i;
                        indexMasock = 0;
                    }
                }
            }

            // Search for already registered socket
            if (bIsFound)
            {
                for (uint32_t j = 0; (j < MAX_ZEPHYR_SOCKET) && (!bSockIsFound); j++)
                {
                    if ((tabMCast[indexMacast].sock[j] == sock) &&
                        (tabMCast[indexMacast].sock[j] != SOPC_INVALID_SOCKET))
                    {
                        bSockIsFound = true;
                        indexMasock = j;
                    }
                }
            }

            // If socket not found, add it
            if (!bSockIsFound && bJoin)
            {
                for (uint32_t j = 0; (j < MAX_ZEPHYR_SOCKET) && (!bSockIsFound); j++)
                {
                    if (SOPC_INVALID_SOCKET == tabMCast[indexMacast].sock[j])
                    {
                        tabMCast[indexMacast].sock[j] = sock;
                        bSockIsFound = true;
                        indexMasock = j;
#if P_MULTICAST_DEBUG == 1
                        {
                            char addr_str[32];
                            inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));
                            printk("\r\nP_MULTICAST: register socket = %d for mcast ip = %s\r\n", //
                                   sock,                                                          //
                                   addr_str);                                                     //
                        }
#endif
                    }
                }
            }

            // Verify mcast is already registered by L1
            if (bIsFound && bSockIsFound)
            {
#if P_MULTICAST_DEBUG == 1
                {
                    char addr_str[32];
                    inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));
                    printk("\r\nP_MULTICAST: verify for socket = %d if mcast ip = %s is already joined or not\r\n", //
                           sock,                                                                                    //
                           addr_str);                                                                               //
                }
#endif

                // Join mcast not already joined by other socket and join request
                if (!tabMCast[indexMacast].bIsJoined && bJoin)
                {
#if P_MULTICAST_DEBUG == 1
                    {
                        char addr_str[32];
                        inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));
                        printk("\r\nP_MULTICAST: register mcast ip = %s to L1 (first register) for socket = %d\r\n", //
                               addr_str,                                                                             //
                               sock);                                                                                //
                    }
#endif
                    // Register L1 multicast
                    struct net_if* ptrNetIf = NULL;
#if defined(CONFIG_NET_L2_ETHERNET)
                    ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
#endif
                    if (ptrNetIf != NULL)
                    {
                        P_MULTICAST_enet_add_mcast(ptrNetIf, pAddr);
#if P_MULTICAST_DEBUG == 1
                        printk("\r\nP_MULTICAST: Call ethernet mcast\r\n");
#endif
                    }
                    tabMCast[indexMacast].bIsJoined = true;
                }
                else
                {
                    // Leave request, verify if at least one sock is joining this mcast.
                    // If not, remove from L1 and L2
                    if (!bJoin)
                    {
                        tabMCast[indexMacast].sock[indexMasock] = SOPC_INVALID_SOCKET;
                        bSockIsFound = false;

                        for (uint32_t j = 0; (j < MAX_ZEPHYR_SOCKET) && (!bSockIsFound); j++)
                        {
                            if (tabMCast[indexMacast].sock[j] != SOPC_INVALID_SOCKET)
                            {
                                bSockIsFound = true;
                                indexMasock = j;
                            }
                        }
#if P_MULTICAST_DEBUG == 1
                        {
                            char addr_str[32];
                            inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));
                            printk("\r\nP_MULTICAST: remove socket = %d for mcast ip = %s\r\n", //
                                   sock,                                                        //
                                   addr_str                                                     //
                            );                                                                  //
                        }
#endif
                        if (!bSockIsFound)
                        {
#if P_MULTICAST_DEBUG == 1
                            {
                                char addr_str[32];
                                inet_ntop(AF_INET, pAddr, addr_str, sizeof(addr_str));
                                printk(
                                    "\r\nP_MULTICAST: remove mcast ip = %s (L1 and L2) for socket = %d because no more "
                                    "joined \r\n", //
                                    addr_str,      //
                                    sock           //
                                );                 //
                            }
#endif
                            // Unregister L1 and L2 multicast
                            struct net_if* ptrNetIf = NULL;
#if defined(CONFIG_NET_L2_ETHERNET)
                            ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
#endif
                            if (ptrNetIf != NULL)
                            {
                                net_if_ipv4_maddr_rm(ptrNetIf, pAddr);
                                P_MULTICAST_enet_rm_mcast(ptrNetIf, pAddr);
                            }

                            tabMCast[indexMacast].bIsJoined = false;
                            tabMCast[indexMacast].addr.s_addr = 0;
                        }
                    }
                }
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }

            desiredStatus = MULTCAST_STATUS_INITIALIZED;
            __atomic_store(&priv_MulticastStatus, &desiredStatus, __ATOMIC_SEQ_CST);
        }
        else
        {
            if (MULTCAST_STATUS_LOCKED == expectedStatus)
            {
                status = SOPC_STATUS_NOK;
                k_yield();
            }
            else
            {
                if (MULTCAST_STATUS_NOT_INITIALIZED == expectedStatus)
                {
                    status = SOPC_STATUS_INVALID_STATE;
                }
            }
        }
    } while ((MULTCAST_STATUS_INITIALIZED != expectedStatus) && (status != SOPC_STATUS_INVALID_STATE));

    return status;
}

// Check if address is a knowned multicast address registered with socket in parameter
bool P_MULTICAST_soft_filter(int32_t sock, struct in_addr* pAddr)
{
    uint32_t indexMacast = 0;
    uint32_t indexMasock = 0;
    bool bIsFound = false;
    bool bSockIsFound = false;
    bool bResult = false;
    eMulticastStatus expectedStatus = MULTCAST_STATUS_INITIALIZED;
    eMulticastStatus desiredStatus = MULTCAST_STATUS_LOCKED;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

#if P_MULTICAST_DEBUG == 1
    printk("\r\nP_MULTICAST: Enter verify mcast group\r\n");
#endif

    if (SOPC_INVALID_SOCKET == sock || NULL == pAddr)
    {
        return false;
    }

    if (!net_ipv4_is_addr_mcast(pAddr))
    {
        return false;
    }

    do
    {
        status = SOPC_STATUS_OK;
        expectedStatus = MULTCAST_STATUS_INITIALIZED;
        desiredStatus = MULTCAST_STATUS_LOCKED;

        bool bTransition = __atomic_compare_exchange(&priv_MulticastStatus, //
                                                     &expectedStatus,       //
                                                     &desiredStatus,        //
                                                     false,                 //
                                                     __ATOMIC_SEQ_CST,      //
                                                     __ATOMIC_SEQ_CST);     //

        if (bTransition)
        {
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
                    if ((tabMCast[indexMacast].sock[j] == sock) &&
                        (tabMCast[indexMacast].sock[j] != SOPC_INVALID_SOCKET))
                    {
                        bSockIsFound = true;
                        indexMasock = j;
                    }
                }
            }

            desiredStatus = MULTCAST_STATUS_INITIALIZED;
            __atomic_store(&priv_MulticastStatus, &desiredStatus, __ATOMIC_SEQ_CST);
        }
        else
        {
            if (MULTCAST_STATUS_LOCKED == expectedStatus)
            {
                status = SOPC_STATUS_NOK;
                k_yield();
            }
            else
            {
                if (MULTCAST_STATUS_NOT_INITIALIZED == expectedStatus)
                {
                    status = SOPC_STATUS_INVALID_STATE;
                }
            }
        }
    } while ((MULTCAST_STATUS_INITIALIZED != expectedStatus) && (status != SOPC_STATUS_INVALID_STATE));

    bResult = (bIsFound && bSockIsFound);

#if P_MULTICAST_DEBUG == 1
    printk("\r\nP_MULTICAST: Exit verify mcast group\r\n");
#endif
    return bResult;
}

// Remove socket from all mcast.
void P_MULTICAST_remove_sock_from_mcast(int32_t sock)
{
#if P_MULTICAST_DEBUG == 1
    printk("\r\nP_MULTICAST: Enter remove_sock_from_mcast\r\n");
#endif
    if (SOPC_INVALID_SOCKET == sock)
    {
        return;
    }

    // Remove socket from mcast and remove mcast if necessary
    for (uint32_t i = 0; i < MAX_MCAST; i++)
    {
        for (uint32_t j = 0; j < MAX_ZEPHYR_SOCKET; j++)
        {
            (void) P_MULTICAST_join_or_leave_mcast_group(sock, &tabMCast[i].addr, false);
        }
    }
#if P_MULTICAST_DEBUG == 1
    printk("\r\nP_MULTICAST: Exit remove_sock_from_mcast\r\n");
#endif
}

// *** Multicast private weak functions definitions

static void P_MULTICAST_enet_add_mcast(struct net_if* ptrNetIf, struct in_addr* pAddr)
{
    const struct ethernet_api* api = net_if_get_device(ptrNetIf)->driver_api;

    if (net_ipv4_is_addr_mcast(pAddr))
    {
        const struct ethernet_config ethernetconfig = {
            .filter =
                {
                    .set = 1,                                        //
                    .type = ETHERNET_FILTER_TYPE_SRC_MAC_ADDRESS,    //
                    .mac_address = {{0x01U,                          //
                                     0x00U,                          //
                                     0x5EU,                          //
                                     (pAddr->s_addr >> 8) & 0x7FU,   //
                                     (pAddr->s_addr >> 16) & 0xFFU,  //
                                     (pAddr->s_addr >> 24) & 0xFFU}} //
                },                                                   //
        };

        if (api->set_config != 0)
        {
            int res = api->set_config(net_if_get_device(ptrNetIf), //
                                      ETHERNET_CONFIG_TYPE_FILTER, //
                                      &ethernetconfig);            //
            (void) res;
#if P_MULTICAST_DEBUG == 1
            if (res != 0)
            {
                printk("\r\nP_MULTICAST: Error on set config driver api with SET ETHERNET_CONFIG_TYPE_FILTER\r\n"); //
            }
            else
            {
                printk("\r\nP_MULTICAST: Set config driver api with SET ETHERNET_CONFIG_TYPE_FILTER :-)\r\n"); //
            }
#endif
        }
        else
        {
#if P_MULTICAST_DEBUG == 1
            printk("\r\nP_MULTICAST: Error set config driver api not defined\r\n"); //
#endif
        }
    }
}

static void P_MULTICAST_enet_rm_mcast(struct net_if* ptrNetIf, struct in_addr* pAddr)
{
    const struct ethernet_api* api = net_if_get_device(ptrNetIf)->driver_api;

    if (net_ipv4_is_addr_mcast(pAddr))
    {
        const struct ethernet_config ethernetconfig = {
            .filter =
                {
                    .set = 0,                                        //
                    .type = ETHERNET_FILTER_TYPE_SRC_MAC_ADDRESS,    //
                    .mac_address = {{0x01U,                          //
                                     0x00U,                          //
                                     0x5EU,                          //
                                     (pAddr->s_addr >> 8) & 0x7FU,   //
                                     (pAddr->s_addr >> 16) & 0xFFU,  //
                                     (pAddr->s_addr >> 24) & 0xFFU}} //
                },                                                   //

        };

        if (api->set_config != 0)
        {
            int res = api->set_config(net_if_get_device(ptrNetIf), //
                                      ETHERNET_CONFIG_TYPE_FILTER, //
                                      &ethernetconfig);            //

            (void) res;
#if P_MULTICAST_DEBUG == 1
            if (res != 0)
            {
                printk("\r\nP_MULTICAST: Error on set config driver api with RESET ETHERNET_CONFIG_TYPE_FILTER\r\n"); //
            }
            else
            {
                printk("\r\nP_MULTICAST: Set config driver api with RESET ETHERNET_CONFIG_TYPE_FILTER :-)\r\n"); //
            }
#endif
        }
        else
        {
#if P_MULTICAST_DEBUG == 1
            printk("\r\nP_MULTICAST: Error set config driver api not defined\r\n"); //
#endif
        }
    }
}
