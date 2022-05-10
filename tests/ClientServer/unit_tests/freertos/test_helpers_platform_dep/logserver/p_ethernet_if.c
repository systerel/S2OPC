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

#include "p_ethernet_if.h"
#include "sopc_macros.h"

static QueueHandle_t gEthernetReady = NULL;

static struct netif gnetif;
static ip4_addr_t localAdress = {.addr = 0};
static ip4_addr_t localNetmsk = {.addr = 0};
static ip4_addr_t localGatewy = {.addr = 0};

static ethernetif_config_t fsl_enet_config = {
    .phyAddress = BOARD_ENET0_PHY_ADDRESS,
    .clockName = kCLOCK_CoreSysClk,
    .macAddress = PHY_MAC_ADDRESS,
};

static void CB_P_ETHERNET_IF_Initialize(void* arg)
{
    SOPC_UNUSED_ARG(arg);

    tcpip_init(NULL, NULL);

    IP4_ADDR(&localAdress, IP_ADDRESS_0, IP_ADDRESS_1, IP_ADDRESS_2, IP_ADDRESS_3);
    IP4_ADDR(&localNetmsk, IP_MASK_0, IP_MASK_1, IP_MASK_2, IP_MASK_3);
    IP4_ADDR(&localGatewy, IP_GW_0, IP_GW_1, IP_GW_2, IP_GW_3);

    netif_add(&gnetif, &localAdress, &localNetmsk, &localGatewy, &fsl_enet_config, &ethernetif0_init, &tcpip_input);

    netif_set_default(&gnetif);

    if (netif_is_link_up(&gnetif))
    {
        netif_set_up(&gnetif);
    }
    else
    {
        netif_set_down(&gnetif);
    }

    xSemaphoreGive(gEthernetReady);

    vTaskDelete(NULL);
}

eEthernetIfResult P_ETHERNET_IF_IsReady(uint32_t uwTimeOutMs)
{
    TickType_t xTimeToWait = 0;
    if (uwTimeOutMs >= (portMAX_DELAY / (configTICK_RATE_HZ * ((uint32_t) 1000))))
    {
        xTimeToWait = portMAX_DELAY;
    }
    else
    {
        xTimeToWait = pdMS_TO_TICKS(uwTimeOutMs);
    }
    if (xSemaphoreTake(gEthernetReady, xTimeToWait) == pdPASS)
    {
        xSemaphoreGive(gEthernetReady);
        return ETHERNET_IF_RESULT_OK;
    }
    return ETHERNET_IF_RESULT_NOK;
}

eEthernetIfResult P_ETHERNET_IF_GetIp(ip_addr_t* pAdressInfo)
{
    if (xSemaphoreTake(gEthernetReady, 0) == pdPASS)
    {
        xSemaphoreGive(gEthernetReady);
        SOPC_UNUSED_RESULT(memcpy(pAdressInfo, &gnetif.ip_addr, sizeof(ip_addr_t)));
        return ETHERNET_IF_RESULT_OK;
    }
    return ETHERNET_IF_RESULT_NOK;
}

eEthernetIfResult P_ETHERNET_IF_Initialize(void)
{
    eEthernetIfResult result = ETHERNET_IF_RESULT_OK;

    gEthernetReady = xSemaphoreCreateBinary();
    xSemaphoreTake(gEthernetReady, 0);

    if (NULL ==
        sys_thread_new("lwip_init", CB_P_ETHERNET_IF_Initialize, NULL, DEFAULT_THREAD_STACKSIZE, DEFAULT_THREAD_PRIO))
    {
        result = ETHERNET_IF_RESULT_NOK;
    }

    return result;
}
