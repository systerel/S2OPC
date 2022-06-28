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

#include "sopc_assert.h"
#include "sopc_mutexes.h"

#define MY_IP_LB ((const char*) ("127.0.0.1"))
#define MY_IP_LB_MASK ((const char*) ("255.255.255.0"))
#define MY_IP_LB_GW ((const char*) ("127.0.0.1"))

#ifndef CONFIG_SOPC_ETH_ADDRESS
#error "CONFIG_SOPC_ETH_ADDRESS is not defined!"
#endif

#ifndef CONFIG_SOPC_ETH_GW
#error "CONFIG_SOPC_ETH_GW is not defined!"
#endif

#ifndef CONFIG_SOPC_ETH_NETMASK
#error "CONFIG_SOPC_ETH_NETMASK is not defined!"
#endif

#ifndef CONFIG_NET_L2_DUMMY
#error 'CONFIG_NET_L2_DUMMY is required for this demo'
#endif

#ifndef CONFIG_NET_L2_ETHERNET
#error 'CONFIG_NET_L2_ETHERNET is required for this demo'
#endif

/* Max socket based on max connections allowed by zephyr */

#ifdef CONFIG_NET_MAX_CONN
#define MAX_ZEPHYR_SOCKET (CONFIG_NET_MAX_CONN - 2)
#else
#define MAX_ZEPHYR_SOCKET 4
#endif

typedef struct NetItf
{
    const char* name;
    const struct net_l2* l2;
} NetItf;

static const NetItf netl2Loopback = {.name = "DUMMY", .l2 = &NET_L2_GET_NAME(DUMMY)};
static const NetItf netl2ethernet = {.name = "ETHERNET", .l2 = &NET_L2_GET_NAME(ETHERNET)};

static volatile bool priv_P_SOCKET_networkConfigStatus = false;

// *** Weak p_sockets functions definitions ***

static bool configure_interface(const NetItf* itf, const char* ipAddr, const char* netmask, const char* gwIp)
{
    struct in_addr address;
    struct in_addr addressMask;
    struct in_addr addressGtw;
    struct net_if* ptrNetIf = NULL;
    struct net_if_addr* newInterface;

    net_addr_pton(AF_INET, ipAddr, (void*) &address);
    net_addr_pton(AF_INET, netmask, (void*) &addressMask);
    net_addr_pton(AF_INET, gwIp, (void*) &addressGtw);
    printk("Configure %s with %s , Mask = %s, GW= %s\r\n", itf->name, ipAddr, netmask, gwIp);

    if (NULL == net_if_ipv4_addr_lookup(&address, &ptrNetIf))
    {
        ptrNetIf = net_if_get_first_by_type(itf->l2);
        if (ptrNetIf != NULL)
        {
            SOPC_ASSERT(NULL != ptrNetIf->if_dev->dev->name);
            printk("Using interface (%s)\n", ptrNetIf->if_dev->dev->name);
        }
        if (NULL == ptrNetIf)
        {
            printk("Could not find matching network interface!\n");
            return false;
        }
        newInterface = net_if_ipv4_addr_add(ptrNetIf, &address, NET_ADDR_MANUAL, 0);
        if (NULL == newInterface)
        {
            printk("Could not add IP to network interface %s!\n", itf->name);
            return false;
        }
        printk("Added IP = <%s> to <%s> !\n", ipAddr, ptrNetIf->if_dev->dev->name);
        net_if_ipv4_set_netmask(ptrNetIf, &addressMask);
        net_if_ipv4_set_gw(ptrNetIf, &addressGtw);
    }
    return true;
}

bool Network_Initialize(const char* overrideEthAddr)
{
    if (!priv_P_SOCKET_networkConfigStatus)
    {
        if (!configure_interface(&netl2Loopback, MY_IP_LB, MY_IP_LB_MASK, MY_IP_LB_GW))
        {
            printk("Initialization of LOOPBACK failed\n");
            return false;
        }
        const char* ethAddr = (overrideEthAddr ? overrideEthAddr : CONFIG_SOPC_ETH_ADDRESS);
        if (!configure_interface(&netl2ethernet, ethAddr, CONFIG_SOPC_ETH_NETMASK, CONFIG_SOPC_ETH_GW))
        {
            printk("Initialization of ETH failed\n");
            return false;
        }

        priv_P_SOCKET_networkConfigStatus = true;
    }
    return priv_P_SOCKET_networkConfigStatus;
}
