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

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>

#include <zephyr/kernel.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/socket.h>

#include "sopc_assert.h"
#include "sopc_dict.h"
#include "sopc_macros.h"

#include "p_sopc_multicast.h"

/* Max socket based on max connections allowed by zephyr */

#ifndef CONFIG_NET_L2_ETHERNET
#error "CONFIG_NET_L2_ETHERNET is required for S2OPC"
#endif

/***************************************************
 * DECLARATION OF LOCAL VARIABLES
 **************************************************/
/** A dictionary {Socket : struct net_if_mcast_addr*} */
static SOPC_Dict* dictMCast = NULL;

/***************************************************
 * DECLARATION OF LOCAL FUNCTIONS
 **************************************************/
/** Hash for key of type 'int' */
static uint64_t socketKeyHash(const uintptr_t data);
/** Equality for key of type 'int' */
static bool socketKeyEqual(const uintptr_t a, const uintptr_t b);
static void linkSockToMCast(SOPC_Socket sock, struct net_if_mcast_addr* mcAddress);
static struct net_if_mcast_addr* getAndUnlinkSockFromMCast(SOPC_Socket sock);

#define SOCKET_TO_KEY(s) ((uintptr_t)(s))
#define KEY_TO_SOCKET(k) ((int) (k))

static uintptr_t NO_KEY = SOCKET_TO_KEY(SOPC_INVALID_SOCKET);
static uintptr_t EMPTY_KEY = SOCKET_TO_KEY(-2);

/***************************************************
 * IMPLEMENTATION OF LOCAL FUNCTIONS
 **************************************************/
static uint64_t socketKeyHash(const uintptr_t data)
{
    return (uint64_t) KEY_TO_SOCKET(data);
}

/***************************************************/
static bool socketKeyEqual(const uintptr_t a, const uintptr_t b)
{
    return KEY_TO_SOCKET(a) == KEY_TO_SOCKET(b);
}

/***************************************************/
static void linkSockToMCast(SOPC_Socket sock, struct net_if_mcast_addr* mcAddress)
{
    uintptr_t key = SOCKET_TO_KEY(sock->sock);
    if (NULL == dictMCast)
    {
        dictMCast = SOPC_Dict_Create(NO_KEY, &socketKeyHash, &socketKeyEqual, NULL, NULL);
        SOPC_ASSERT(NULL != dictMCast);
        SOPC_Dict_SetTombstoneKey(dictMCast, EMPTY_KEY);
    }
    bool res = SOPC_Dict_Insert(dictMCast, key, (uintptr_t) mcAddress);
    SOPC_ASSERT(res);
}

/***************************************************/
static struct net_if_mcast_addr* getAndUnlinkSockFromMCast(SOPC_Socket sock)
{
    if (NULL == dictMCast)
    {
        return 0;
    }
    uintptr_t key = SOCKET_TO_KEY(sock->sock);

    struct net_if_mcast_addr* result = (struct net_if_mcast_addr*) SOPC_Dict_Get(dictMCast, key, NULL);
    SOPC_Dict_Remove(dictMCast, key);
    if (SOPC_Dict_Size(dictMCast) == 0)
    {
        SOPC_Dict_Delete(dictMCast);
        dictMCast = NULL;
    }
    return result;
}

/***************************************************
 * IMPLEMENTATION OF EXTERN FUNCTIONS
 **************************************************/
SOPC_ReturnStatus P_MULTICAST_AddIpV4Membership(SOPC_Socket sock, const SOPC_Socket_AddressInfo* multicast)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOT_SUPPORTED;
    SOPC_ASSERT(NULL != sock);
    struct net_if* ptrNetIf = NULL;
    if (AF_INET == multicast->addrInfo.ai_family)
    {
        // Retrieve IPV4 address
        struct in_addr* multiAddr = &((struct sockaddr_in*) &multicast->addrInfo._ai_addr)->sin_addr;
        // Check that this is a multicast address
        if (!net_ipv4_is_addr_mcast(multiAddr))
        {
            status = SOPC_STATUS_INVALID_PARAMETERS;
        }
        else
        {
            // Ensure we can find the Ethernet device
            ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
            if (ptrNetIf != NULL)
            {
                status = SOPC_STATUS_OK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            // Join a membership
            struct net_if_mcast_addr* mcAddr = net_if_ipv4_maddr_add(ptrNetIf, multiAddr);
            if (NULL == mcAddr)
            {
                status = SOPC_STATUS_NOK;
            }
            else
            {
                net_if_ipv4_maddr_join(ptrNetIf, mcAddr);
                // The link between "sock" and "mcAddr" must be stored to allow further Droping membership
                linkSockToMCast(sock, mcAddr);
            }
        }
    }
    return status;
}

/***************************************************/
SOPC_ReturnStatus P_MULTICAST_DropIpV4Membership(SOPC_Socket sock, const SOPC_Socket_AddressInfo* multicast)
{
    SOPC_ASSERT(NULL != sock);
    // Leave a membership
    struct net_if_mcast_addr* mcAddr = getAndUnlinkSockFromMCast(sock);
    struct net_if* ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));

    if (NULL == mcAddr || NULL == ptrNetIf)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    struct in_addr* multiAddr = NULL;
    if (NULL != multicast)
    {
        multiAddr = &((struct sockaddr_in*) &multicast->addrInfo._ai_addr)->sin_addr;
    }
    else
    {
        multiAddr = &mcAddr->address.in_addr;
    }

    net_if_ipv4_maddr_leave(ptrNetIf, mcAddr);
    bool ret = net_if_ipv4_maddr_rm(ptrNetIf, multiAddr);
    return (ret ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
}

/***************************************************/
SOPC_ReturnStatus SOPC_UDP_Socket_Set_MulticastTTL(SOPC_Socket sock, uint8_t TTL_scope)
{
    SOPC_UNUSED_ARG(sock);
    // In zephyr the TTL is common to interface
    struct net_if* ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
    if (NULL == ptrNetIf)
    {
        return SOPC_STATUS_NOK;
    }
    net_if_ipv4_set_ttl(ptrNetIf, TTL_scope);
    return SOPC_STATUS_OK;
}
