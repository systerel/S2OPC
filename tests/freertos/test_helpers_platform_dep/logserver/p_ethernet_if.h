/*
 * p_ethernet_if.h
 *
 *  Created on: 6 juin 2019
 *      Author: nottin
 */

#ifndef HELPERS_PLATFORM_DEP_FREERTOS_P_ETHERNET_IF_H_
#define HELPERS_PLATFORM_DEP_FREERTOS_P_ETHERNET_IF_H_

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

#include "ethernetif.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "lwipopts.h"
#include "netif/etharp.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/tcpip.h"

#define IP_ADDRESS_0 192
#define IP_ADDRESS_1 168
#define IP_ADDRESS_2 1
#define IP_ADDRESS_3 102

#define IP_MASK_0 255
#define IP_MASK_1 255
#define IP_MASK_2 255
#define IP_MASK_3 0

#define IP_GW_0 192
#define IP_GW_1 168
#define IP_GW_2 1
#define IP_GW_3 100

#define PHY_MAC_ADDRESS                    \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }

typedef enum E_ETHERNET_IF_RESULT
{
    ETHERNET_IF_RESULT_OK,
    ETHERNET_IF_RESULT_NOK
} eEthernetIfResult;

eEthernetIfResult P_ETHERNET_IF_Initialize(void);
eEthernetIfResult P_ETHERNET_IF_IsReady(void);
eEthernetIfResult P_ETHERNET_IF_GetIp(ip_addr_t* pAdressInfo);

#endif /* HELPERS_PLATFORM_DEP_FREERTOS_P_ETHERNET_IF_H_ */
