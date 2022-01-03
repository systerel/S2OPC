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

#include "sopc_common_constants.h"
#include "sopc_eth_sockets.h"

#include "p_sockets.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <limits.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"

struct SOPC_ETH_Socket_ReceiveAddressInfo
{
    struct sockaddr_ll addr;
    bool recvMulticast;
    bool recvForDest;
    unsigned char recvDestAddr[6];
    bool recvFromSource;
    unsigned char recvSourceAddr[6];
};

struct SOPC_ETH_Socket_SendAddressInfo
{
    struct sockaddr_ll addr;
    struct ifreq sendSrcMACaddr;
};

static bool SOPC_ETH_Socket_AddMembership(Socket sock, struct packet_mreq* mreq)
{
    int res = setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, mreq, sizeof(*mreq));
    return res >= 0;
}

static SOPC_ReturnStatus SOPC_ETH_Socket_AddMemberships(Socket sock,
                                                        const SOPC_ETH_Socket_ReceiveAddressInfo* multicastAddrInfo)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == multicastAddrInfo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    assert(multicastAddrInfo->recvMulticast);

    struct packet_mreq mreq;
    memset(&mreq, 0, sizeof(mreq));
    if (!multicastAddrInfo->recvForDest)
    {
        mreq.mr_type = PACKET_MR_ALLMULTI; // on all multicast addresses (least significant bit of first byte == 1)
    }
    else
    {
        mreq.mr_type = PACKET_MR_MULTICAST; // => on given address
        mreq.mr_alen = ETH_ALEN;
        memcpy(mreq.mr_address, multicastAddrInfo->recvDestAddr, ETH_ALEN);
    }

    uint32_t counter = 0;
    bool atLeastOneItfSuccess = false;

    if (multicastAddrInfo->addr.sll_ifindex > 0)
    {
        // Receive interface index is specified, only set membership on this one
        counter = 1;
        mreq.mr_ifindex = multicastAddrInfo->addr.sll_ifindex;
        atLeastOneItfSuccess = SOPC_ETH_Socket_AddMembership(sock, &mreq);

        return SOPC_STATUS_OK;
    }
    else
    {
        // No receive interface index is specified, set membership on all compatible interfaces

        struct ifaddrs* ifap = NULL;
        int result = getifaddrs(&ifap);

        if (0 != result)
        {
            return SOPC_STATUS_NOT_SUPPORTED;
        }

        for (struct ifaddrs* ifa = ifap; ifa != NULL; ifa = ifa->ifa_next)
        {
            bool optionSet = false;
            int setOptStatus = 0;
            if (ifa->ifa_addr)
            {
                if (multicastAddrInfo->addr.sll_family == ifa->ifa_addr->sa_family)
                {
                    unsigned int ifindex = if_nametoindex(ifa->ifa_name);
                    if (ifindex > 0 && ifindex <= INT_MAX)
                    {
                        mreq.mr_ifindex = (int) ifindex;
                        setOptStatus = SOPC_ETH_Socket_AddMembership(sock, &mreq);
                        counter++;
                        optionSet = true;
                    }
                }
            }

            if (optionSet)
            {
                if (setOptStatus < 0)
                {
                    SOPC_CONSOLE_PRINTF("SOPC_ETH_Socket_AddMembership failure (error='%s') on interface %s\n",
                                        strerror(errno), ifa->ifa_name);
                }
                else
                {
                    atLeastOneItfSuccess = true;
                }
            }
        }

        freeifaddrs(ifap);

        if (0 == counter)
        {
            return SOPC_STATUS_NOT_SUPPORTED;
        }
        else
        {
            if (atLeastOneItfSuccess)
            {
                return SOPC_STATUS_OK;
            }
            else
            {
                return SOPC_STATUS_NOK;
            }
        }
    }
}

static bool set_mac_addr_from_string(unsigned char* addr, const char* MACaddress)
{
    assert(NULL != addr);
    if (strlen(MACaddress) != 17)
    {
        return false;
    }
    bool res = true;
    for (int i = 0; i < 6 && res; i++)
    {
        uint8_t addressByte = 0;
        // TODO: do not use end character, use SOPC_strtouint instead but we need base 16 instead of 10
        SOPC_ReturnStatus status = SOPC_strtouint8_t(MACaddress + i * 3, &addressByte, 16, i < 5 ? '-' : '\0');
        res = SOPC_STATUS_OK == status;
        if (res)
        {
            addr[i] = addressByte;
        }
    }
    return res;
}

static bool set_itfindex_from_string(struct sockaddr_ll* addr, const char* interfaceName)
{
    assert(NULL != addr);
    int tmpSock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (tmpSock == SOPC_INVALID_SOCKET)
    {
        return false;
    }
    struct ifreq req;
    memset(&req, 0, sizeof(req));
    strncpy(req.ifr_name, interfaceName, IFNAMSIZ - 1);
    int res = ioctl(tmpSock, SIOCGIFINDEX, &req);
    close(tmpSock);
    if (res < 0)
    {
        return false;
    }
    else
    {
        addr->sll_ifindex = req.ifr_ifindex;
    }
    return true;
}

static bool set_mac_addr_from_interface(struct ifreq* ifreq, const char* interfaceName)
{
    assert(NULL != ifreq);
    int tmpSock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (tmpSock == SOPC_INVALID_SOCKET)
    {
        return false;
    }
    strncpy(ifreq->ifr_name, interfaceName, IFNAMSIZ - 1);
    int res = ioctl(tmpSock, SIOCGIFHWADDR, ifreq);
    close(tmpSock);
    if (res < 0)
    {
        return false;
    }
    return true;
}

SOPC_ReturnStatus SOPC_ETH_Socket_CreateSendAddressInfo(const char* interfaceName,
                                                        const char* destMACaddr,
                                                        SOPC_ETH_Socket_SendAddressInfo** sendAddInfo)
{
    // TODO: retrieve MAC address already parsed and add optional VLAN parameters
    if (NULL == interfaceName || NULL == destMACaddr || NULL == sendAddInfo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ETH_Socket_SendAddressInfo* addrInfo = SOPC_Calloc(1, sizeof(*addrInfo));
    if (NULL == addrInfo)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    addrInfo->addr.sll_family = AF_PACKET;
    addrInfo->addr.sll_protocol = htons(ETH_P_ALL);

    /* From man packet:
    When you send packets, it is enough to specify sll_family,
    sll_addr, sll_halen, sll_ifindex, and sll_protocol.  The other
    fields should be 0.  sll_hatype and sll_pkttype are set on
    received packets for your information.
    */
    // sll_ifindex is the interface on source side
    // sll_addr is the MAC address. For multicast: least significant bit of first byte == 1
    bool res = set_itfindex_from_string(&addrInfo->addr, interfaceName);
    if (res)
    {
        // Note: source MAC address needed for ethernet header
        res = set_mac_addr_from_interface(&addrInfo->sendSrcMACaddr, interfaceName);
    }
    if (res)
    {
        res = set_mac_addr_from_string(addrInfo->addr.sll_addr, destMACaddr);
    }

    if (!res)
    {
        SOPC_Free(addrInfo);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *sendAddInfo = addrInfo;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_ETH_Socket_CreateReceiveAddressInfo(const char* interfaceName,
                                                           bool recvMulticast,
                                                           const char* destMACaddr,
                                                           const char* sourceMACaddr,
                                                           SOPC_ETH_Socket_ReceiveAddressInfo** recvAddInfo)
{
    // TODO: retrieve MAC addresses already parsed and add optional VLAN parameters
    if (NULL == recvAddInfo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    SOPC_ETH_Socket_ReceiveAddressInfo* addrInfo = SOPC_Calloc(1, sizeof(*addrInfo));
    if (NULL == addrInfo)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }
    addrInfo->addr.sll_family = AF_PACKET;
    addrInfo->addr.sll_protocol = htons(ETH_P_ALL);

    /* From man packet:
    sll_ifindex is the interface index of the interface (see
    netdevice(7)); 0 matches any interface (only permitted for
    binding).  sll_hatype is an ARP type as defined in the
    <linux/if_arp.h> include file.
    */
    // Reminder: sll_hatype and sll_pkttype are set on received packets for your information.

    bool res = true;
    if (NULL != interfaceName)
    {
        res = set_itfindex_from_string(&addrInfo->addr, interfaceName);
    }

    if (res && NULL != destMACaddr)
    {
        // Parse Multicast destination address or Unicast source address to record
        res = set_mac_addr_from_string(addrInfo->recvDestAddr, destMACaddr);
        if (res && recvMulticast)
        {
            // Check multicast bit set (least significant bit of first byte)
            res = (addrInfo->recvDestAddr[0] & 0x01) != 0;
        }
    }
    if (res && NULL != sourceMACaddr)
    {
        // Parse Multicast destination address or Unicast source address to record
        res = set_mac_addr_from_string(addrInfo->recvSourceAddr, sourceMACaddr);
    }
    if (!res)
    {
        SOPC_Free(addrInfo);
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    addrInfo->recvMulticast = recvMulticast;
    addrInfo->recvForDest = NULL != destMACaddr;
    addrInfo->recvFromSource = NULL != sourceMACaddr;

    *recvAddInfo = addrInfo;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_ETH_Socket_CreateToReceive(SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                                  bool setNonBlocking,
                                                  Socket* sock)
{
    if (NULL == sock || NULL == receiveAddrInfo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    *sock = socket(receiveAddrInfo->addr.sll_family, SOCK_RAW, receiveAddrInfo->addr.sll_protocol);
    if (SOPC_INVALID_SOCKET == *sock)
    {
        return SOPC_STATUS_NOK;
    }

    int res = 0;
    if (setNonBlocking)
    {
        res = fcntl(*sock, F_SETFL, O_NONBLOCK);
    }
    SOPC_ReturnStatus status = (res == 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK);

    if (SOPC_STATUS_OK == status)
    {
        res = bind(*sock, (struct sockaddr*) &receiveAddrInfo->addr, sizeof(receiveAddrInfo->addr));
        status = (res == 0 ? SOPC_STATUS_OK : SOPC_STATUS_NOK);
    }

    if (SOPC_STATUS_OK == status && receiveAddrInfo->recvMulticast)
    {
        status = SOPC_ETH_Socket_AddMemberships(*sock, receiveAddrInfo);
    }

    if (SOPC_STATUS_OK != status)
    {
        close(*sock);
        *sock = SOPC_INVALID_SOCKET;

        return status;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_ETH_Socket_CreateToSend(SOPC_ETH_Socket_SendAddressInfo* sendAddrInfo,
                                               bool setNonBlocking,
                                               Socket* sock)
{
    if (NULL == sock || NULL == sendAddrInfo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }
    *sock = socket(sendAddrInfo->addr.sll_family, SOCK_RAW, sendAddrInfo->addr.sll_protocol);
    if (SOPC_INVALID_SOCKET == *sock)
    {
        return SOPC_STATUS_NOK;
    }

    int setOptStatus = 0;
    if (setNonBlocking)
    {
        setOptStatus = fcntl(*sock, F_SETFL, O_NONBLOCK);
    }

    if (setOptStatus < 0)
    {
        SOPC_ETH_Socket_Close(sock);
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_ETH_Socket_SendTo(Socket sock,
                                         const SOPC_ETH_Socket_SendAddressInfo* sendAddrInfo,
                                         uint16_t etherType,
                                         SOPC_Buffer* buffer)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == sendAddrInfo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // note: No VLAN management for now, should be done using EtherType 0x801 and VLAN tag if active.
    //       Change header size if VLAN needed.
    SOPC_Buffer* sendBuffer = SOPC_Buffer_Create(ETHERNET_HEADER_SIZE + buffer->length);
    if (NULL == sendBuffer)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    // Encode the Ethernet header
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    // Destination MAC
    status = SOPC_Buffer_Write(sendBuffer, (const uint8_t*) sendAddrInfo->addr.sll_addr, ETH_ALEN);

    // Source MAC
    if (SOPC_STATUS_OK == status)
    {
        status =
            SOPC_Buffer_Write(sendBuffer, (const uint8_t*) sendAddrInfo->sendSrcMACaddr.ifr_hwaddr.sa_data, ETH_ALEN);
    }

    if (SOPC_STATUS_OK == status)
    {
        uint16_t nboEtherType = htons(etherType);
        status = SOPC_Buffer_Write(sendBuffer, (const uint8_t*) &nboEtherType, (uint32_t) sizeof(uint16_t));
    }

    if (SOPC_STATUS_OK == status)
    {
        memcpy(sendBuffer->data + ETHERNET_HEADER_SIZE, buffer->data, buffer->length);
        sendBuffer->length = ETHERNET_HEADER_SIZE + buffer->length;

        ssize_t sent = sendto(sock, sendBuffer->data, sendBuffer->length, 0,
                              (const struct sockaddr*) &sendAddrInfo->addr, (socklen_t) sizeof(sendAddrInfo->addr));

        if (sent < 0)
        {
            status = SOPC_STATUS_NOK;
        }
        if (sent < (ssize_t)(sendBuffer->length))
        {
            status = SOPC_STATUS_WOULD_BLOCK;
        }
    }

    SOPC_Buffer_Delete(sendBuffer);

    return status;
}

SOPC_ReturnStatus SOPC_ETH_Socket_ReceiveFrom(Socket sock,
                                              const SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                              bool checkEtherType,
                                              uint16_t etherType,
                                              SOPC_Buffer* buffer)
{
    if (SOPC_INVALID_SOCKET == sock || NULL == receiveAddrInfo || NULL == buffer ||
        buffer->current_size < sizeof(struct ethhdr))
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    // Loop until expected MAC address (source if unicast, dest if multicast) && expected protocol if provided
    bool bres = false;
    while (!bres)
    {
        ssize_t recv_len = recv(sock, buffer->data, buffer->current_size, 0);
        if (recv_len < 0)
        {
            return SOPC_STATUS_NOK;
        }

        buffer->length = (uint32_t) recv_len;
        if (buffer->length < sizeof(struct ethhdr))
        {
            // Ethernet header incomplet
            return SOPC_STATUS_OUT_OF_MEMORY;
        }

        // Check Ethernet header
        bool tmpRes = true;

        if (receiveAddrInfo->recvForDest)
        {
            // Check dest MAC
            int res = memcmp(receiveAddrInfo->recvDestAddr, buffer->data, ETH_ALEN * sizeof(char));
            tmpRes &= (0 == res);
        }

        if (receiveAddrInfo->recvFromSource)
        {
            // Check source MAC
            int res = memcmp(receiveAddrInfo->recvSourceAddr, buffer->data + ETH_ALEN, ETH_ALEN * sizeof(char));
            tmpRes &= (0 == res);
        }

        // note: No VLAN management for now, check it by using EtherType 0x801 (or 0x88A8 for double-tag)
        if (checkEtherType)
        {
            // Check EtherType
            uint16_t* nboEthType = (uint16_t*) (void*) (buffer->data + 2 * ETH_ALEN);
            tmpRes &= (ntohs(*nboEthType) == etherType);
        }

        if (tmpRes && buffer->length == buffer->current_size)
        {
            // The message could be incomplete
            return SOPC_STATUS_OUT_OF_MEMORY;
        }

        bres = tmpRes;
    }

    return SOPC_STATUS_OK;
}

void SOPC_ETH_Socket_Close(Socket* sock)
{
    if (NULL != sock)
    {
        close(*sock);
        *sock = SOPC_INVALID_SOCKET;
    }
}
