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

#include "network_init.h"

#include <errno.h>
#include <inttypes.h>
#include <kernel.h>
#include <limits.h>
#include <assert.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <kernel.h>
#include <net/net_ip.h>
#include <net/socket.h>
#include "net/ethernet.h"
#include "net/net_if.h"

#include "sopc_mutexes.h"

#ifndef MY_IP_LB
#define MY_IP_LB ((const char*) ("127.0.0.1"))
#endif

#ifndef MY_IP_LB_MASK
#define MY_IP_LB_MASK ((const char*) ("255.255.255.0"))
#endif

#ifndef CONFIG_NET_CONFIG_MY_IPV4_ADDR
#define CONFIG_NET_CONFIG_MY_IPV4_ADDR ((const char*) ("192.168.1.102"))
#endif

#ifndef CONFIG_NET_CONFIG_MY_IPV4_NETMASK
#define CONFIG_NET_CONFIG_MY_IPV4_NETMASK ((const char*) ("255.255.255.0"))
#endif

#ifndef CONFIG_NET_CONFIG_MY_IPV4_GW
#define CONFIG_NET_CONFIG_MY_IPV4_GW ((const char*) ("192.168.1.111"))
#endif

typedef enum E_NETWORK_CONFIG_STATUS
{
    NETWORK_CONFIG_STATUS_NOT_INITIALIZED,
    NETWORK_CONFIG_STATUS_INITIALIZING,
    NETWORK_CONFIG_STATUS_INITIALIZED,
    NETWORK_CONFIG_STATUS_SIZE = INT32_MAX
} eNetworkConfigStatus;

/* Max socket based on max connections allowed by zephyr */

#ifdef CONFIG_NET_MAX_CONN
#define MAX_ZEPHYR_SOCKET (CONFIG_NET_MAX_CONN - 2)
#else
#define MAX_ZEPHYR_SOCKET 4
#endif

static volatile eNetworkConfigStatus priv_P_SOCKET_networkConfigStatus = NETWORK_CONFIG_STATUS_NOT_INITIALIZED;

static Mutex priv_lockL2; // tabMCast protection

// *** Weak p_sockets functions definitions ***

bool Network_Initialize(void)
{
    uint32_t nwStatus = 0;

    printk("\r\nNetwork initialize called\r\n");

    do
    {
        nwStatus = __sync_val_compare_and_swap(&priv_P_SOCKET_networkConfigStatus,    //
                                               NETWORK_CONFIG_STATUS_NOT_INITIALIZED, // Not initialized
                                               NETWORK_CONFIG_STATUS_INITIALIZING);   // Initializing

        if (0 == nwStatus)
        {
            assert(SOPC_STATUS_OK == Mutex_Initialization(&priv_lockL2));

            struct net_if* ptrNetIf = NULL;
            struct in_addr addressLoopBack;
            struct in_addr addressLoopBackNetMask;
            struct in_addr addressInterfaceEth;
            struct in_addr addressInterfaceEthMask;
            struct in_addr addressInterfaceEthGtw;
            net_addr_pton(AF_INET, MY_IP_LB, (void*) &addressLoopBack);
            net_addr_pton(AF_INET, MY_IP_LB_MASK, (void*) &addressLoopBackNetMask);

            net_addr_pton(AF_INET, CONFIG_NET_CONFIG_MY_IPV4_ADDR, (void*) &addressInterfaceEth);
            net_addr_pton(AF_INET, CONFIG_NET_CONFIG_MY_IPV4_NETMASK, (void*) &addressInterfaceEthMask);
            net_addr_pton(AF_INET, CONFIG_NET_CONFIG_MY_IPV4_GW, (void*) &addressInterfaceEthGtw);
            if (NULL == net_if_ipv4_addr_lookup(&addressLoopBack, &ptrNetIf))
            {
#if defined(CONFIG_NET_L2_DUMMY)
                ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(DUMMY));
#else
#error "CONFIG_NET_L2_DUMMY should be defined."
#endif
                assert(NULL != ptrNetIf);
                assert(NULL != net_if_ipv4_addr_add(ptrNetIf, &addressLoopBack, NET_ADDR_MANUAL, 0));
                net_if_ipv4_set_netmask(ptrNetIf, &addressLoopBackNetMask);
            }

            if (NULL == net_if_ipv4_addr_lookup(&addressInterfaceEth, &ptrNetIf))
            {
#if defined(CONFIG_NET_L2_ETHERNET)
                ptrNetIf = net_if_get_first_by_type(&NET_L2_GET_NAME(ETHERNET));
#else
#error "CONFIG_NET_L2_ETHERNET should be defined"
#endif
                assert(NULL != ptrNetIf);
                assert(NULL != net_if_ipv4_addr_add(ptrNetIf, &addressInterfaceEth, NET_ADDR_MANUAL, 0));
                net_if_ipv4_set_gw(ptrNetIf, &addressInterfaceEthGtw);
                net_if_ipv4_set_netmask(ptrNetIf, &addressInterfaceEthMask);
            }

            priv_P_SOCKET_networkConfigStatus = NETWORK_CONFIG_STATUS_INITIALIZED;
            nwStatus = NETWORK_CONFIG_STATUS_INITIALIZED;
        }
        else
        {
            // Initializing on going, yield and retry
            if (NETWORK_CONFIG_STATUS_INITIALIZING == nwStatus)
            {
                k_yield();
            }
        }
    } while (NETWORK_CONFIG_STATUS_INITIALIZING == nwStatus);

    // Initialized or not initialized
    if (NETWORK_CONFIG_STATUS_INITIALIZED == nwStatus)
    {
        return true;
    }
    else
    {
        return false;
    }
}
