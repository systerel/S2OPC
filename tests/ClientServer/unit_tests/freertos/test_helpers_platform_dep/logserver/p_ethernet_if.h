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

#ifndef P_ETHERNET_IF_H
#define P_ETHERNET_IF_H

#include <board.h>
#include <inttypes.h> /* stdlib includes */
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h" /* freeRtos includes */
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "p_channel.h"
#include "p_synchronisation.h" /* synchronisation include */
#include "p_threads.h"
#include "p_utils.h" /* private list include */

#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "lwipopts.h"
#include "netif/etharp.h"

#include "ethernetif.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"

#include "EthernetConfig.h"

// ************Private API**************

typedef enum E_ETHERNET_IF_RESULT
{
    ETHERNET_IF_RESULT_OK,
    ETHERNET_IF_RESULT_NOK
} eEthernetIfResult;

eEthernetIfResult P_ETHERNET_IF_Initialize(void);
eEthernetIfResult P_ETHERNET_IF_IsReady(uint32_t timeoutMS);
eEthernetIfResult P_ETHERNET_IF_GetIp(ip_addr_t* pAdressInfo);

// ************Public API**************

#endif /* P_ETHERNET_IF_H */
