/*
 * p_logsrv.h
 *
 *  Created on: 8 juin 2019
 *      Author: elsin
 */

#ifndef FREERTOS_P_LOGSRV_H_
#define FREERTOS_P_LOGSRV_H_

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

#include "p_synchronisation.h" /* synchronisation include */
#include "p_threads.h"
#include "p_utils.h"           /* private list include */
#include "p_ethernet_if.h"

#include "lwip/opt.h"
#include "lwipopts.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "ethernetif.h"

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"

#define P_LOG_SRV_TIMEOUT_SELECT 100
#define P_LOG_CLT_TIMEOUT_SELECT 100
#define P_LOG_CLT_TX_POP_WAIT   100
#define P_LOG_CLT_RX_POP_WAIT   100

#define P_LOG_SRV_CALLBACK_STACK 512
#define P_LOG_CLT_MON_CALLBACK_STACK 512
#define P_LOG_CLT_TX_CALLBACK_STACK 512
#define P_LOG_CLT_RX_CALLBACK_STACK 512


#define P_LOG_FIFO_RX_DATA_SIZE         (256)
#define P_LOG_FIFO_RX_ELT_SIZE          (16)


#define P_LOG_FIFO_TX_DATA_SIZE         (256)
#define P_LOG_FIFO_TX_ELT_SIZE          (16)

typedef enum E_LOG_SERVER_STATUS
{
    E_LOG_SERVER_CLOSING,
    E_LOG_SERVER_BINDING,
    E_LOG_SERVER_ONLINE
}eLogServerStatus;

typedef enum E_LOG_CLIENT_STATUS
{
    E_LOG_CLIENT_DISCONNECTED,
    E_LOG_CLIENT_CONNECTED
}eLogClientStatus;



typedef struct T_CHANNEL tChannel;
typedef struct T_LOG_CLIENT_WORKSPACE tLogClientWks;
typedef struct T_LOG_SERVER_WORKSPACE tLogSrvWks;

tLogSrvWks* P_LOG_SRV_CreateAndStart(uint16_t port, int16_t maxClient);

#endif /* FREERTOS_P_LOGSRV_H_ */
