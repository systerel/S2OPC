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

#ifdef WITH_XDP_ETH_SOCKET
// Use XDP socket instead of ethernet socket to manage ethernet frames
// See https://www.kernel.org/doc/html/latest/networking/af_xdp.html

#include <linux/if_link.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <xdp/xsk.h>

struct xsk_umem_info
{
    struct xsk_ring_prod fq;
    struct xsk_ring_cons cq;
    struct xsk_umem* umem;
    void* buffer;
};

struct xsk_socket_info
{
    struct xsk_ring_cons rx;
    struct xsk_ring_prod tx;
    struct xsk_umem_info* umem;
    struct xsk_socket* xsk;
    uint32_t outstanding_tx;
};

static uint32_t opt_queue = 0; // Queue number
static const uint32_t opt_xsk_frame_size = XSK_UMEM__DEFAULT_FRAME_SIZE;
static uint32_t opt_umem_flags = 0;
// XDP_FLAGS_SKB_MODE // force SKB mode (not native: generic XDP support and copies out the data to user space).
//                       A fallback mode that works for any network device.
// XDP_FLAGS_DRV_MODE // force DRV mode (native: need compatible NIC driver)
// XDP_FLAGS_HW_MODE  // force HW mode (offload on HW: need NIC driver)
static uint32_t opt_xdp_flags = XDP_FLAGS_UPDATE_IF_NOEXIST;
static uint16_t opt_xdp_bind_flags = XDP_USE_NEED_WAKEUP;
#define NUM_FRAMES (4 * 1024)

static void* packet_buffer = NULL;
static uint64_t packet_buffer_size = 0;
static Socket* recvSock = NULL;
struct xsk_socket_info* xsk_info = NULL;
#endif

struct SOPC_ETH_Socket_ReceiveAddressInfo
{
    const char* ifname;
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

#ifndef WITH_XDP_ETH_SOCKET
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
#endif

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
#ifdef WITH_XDP_ETH_SOCKET
        addrInfo->ifname = interfaceName;
#else
        res = set_itfindex_from_string(&addrInfo->addr, interfaceName);
#endif
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

#ifdef WITH_XDP_ETH_SOCKET
static void __exit_with_error(int error, const char* file, const char* func, int line)
{
    SOPC_CONSOLE_PRINTF("%s:%s:%i: errno: %d/\"%s\"\n", file, func, line, error, strerror(error));

    exit(EXIT_FAILURE);
}

#define exit_with_error(error) __exit_with_error(error, __FILE__, __func__, __LINE__)

static struct xsk_umem_info* xsk_configure_umem(void* buffer, uint64_t size)
{
    struct xsk_umem_info* umem;
    struct xsk_umem_config cfg = {/* We recommend that you set the fill ring size >= HW RX ring size +
                                   * AF_XDP RX ring size. Make sure you fill up the fill ring
                                   * with buffers at regular intervals, and you will with this setting
                                   * avoid allocation failures in the driver. These are usually quite
                                   * expensive since drivers have not been written to assume that
                                   * allocation failures are common. For regular sockets, kernel
                                   * allocated memory is used that only runs out in OOM situations
                                   * that should be rare.
                                   */
                                  .fill_size = XSK_RING_PROD__DEFAULT_NUM_DESCS * 2,
                                  .comp_size = XSK_RING_CONS__DEFAULT_NUM_DESCS,
                                  .frame_size = opt_xsk_frame_size,
                                  .frame_headroom = XSK_UMEM__DEFAULT_FRAME_HEADROOM,
                                  .flags = opt_umem_flags};
    int ret;

    umem = SOPC_Calloc(1, sizeof(*umem));
    if (!umem)
        exit_with_error(errno);

    ret = xsk_umem__create(&umem->umem, buffer, size, &umem->fq, &umem->cq, &cfg);
    if (ret)
        exit_with_error(-ret);

    umem->buffer = buffer;
    return umem;
}

static void xsk_populate_fill_ring(struct xsk_umem_info* umem)
{
    uint32_t ret, i, idx;

    ret = xsk_ring_prod__reserve(&umem->fq, XSK_RING_PROD__DEFAULT_NUM_DESCS * 2, &idx);
    if (ret != XSK_RING_PROD__DEFAULT_NUM_DESCS * 2)
        exit_with_error((int) (ret + 1));
    for (i = 0; i < XSK_RING_PROD__DEFAULT_NUM_DESCS * 2; i++)
        *xsk_ring_prod__fill_addr(&umem->fq, idx++) = i * opt_xsk_frame_size;
    xsk_ring_prod__submit(&umem->fq, XSK_RING_PROD__DEFAULT_NUM_DESCS * 2);
}

static struct xsk_socket_info* xsk_configure_socket(const char* ifname, struct xsk_umem_info* umem, bool rx, bool tx)
{
    struct xsk_socket_config cfg;
    struct xsk_socket_info* nxsk;
    struct xsk_ring_cons* rxr;
    struct xsk_ring_prod* txr;
    int ret;

    nxsk = SOPC_Calloc(1, sizeof(*nxsk));
    if (!nxsk)
        exit_with_error(errno);

    nxsk->umem = umem;
    cfg.rx_size = XSK_RING_CONS__DEFAULT_NUM_DESCS;
    cfg.tx_size = XSK_RING_PROD__DEFAULT_NUM_DESCS;
    cfg.libbpf_flags = 0;
    cfg.xdp_flags = opt_xdp_flags;
    cfg.bind_flags = opt_xdp_bind_flags;

    rxr = rx ? &nxsk->rx : NULL;
    txr = tx ? &nxsk->tx : NULL;
    // AF_XDP sockets can be created to have exclusive ownership of the umem through xsk_socket__create()
    ret = xsk_socket__create(&nxsk->xsk, ifname, opt_queue, umem->umem, rxr, txr, &cfg);
    if (ret)
        exit_with_error(-ret);

    return nxsk;
}

static SOPC_ReturnStatus XDP_ETH_Socket_CreateToReceive(SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                                        bool setNonBlocking,
                                                        Socket* sock)
{
    (void) setNonBlocking;
    if (NULL == sock || NULL == receiveAddrInfo)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    /* Code to init XDP UMEM, only 1 socket supported for now */
    assert(NULL == recvSock);
    assert(0 == packet_buffer_size);
    assert(NULL == packet_buffer);
    struct rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
    struct xsk_umem_info* umem;

    /* Allow unlimited locking of memory, so all memory needed for packet
     * buffers can be locked.
     */
    if (setrlimit(RLIMIT_MEMLOCK, &r))
    {
        exit_with_error(errno);
    }

    /* Allocate memory for NUM_FRAMES of the default XDP frame size */
    packet_buffer_size = NUM_FRAMES * opt_xsk_frame_size;
    if (posix_memalign(&packet_buffer, (size_t) getpagesize(), /* PAGE_SIZE aligned */
                       packet_buffer_size))
    {
        exit_with_error(errno);
    }

    /* Create UMEM */
    umem = xsk_configure_umem(packet_buffer, NUM_FRAMES * opt_xsk_frame_size);
    /* Populate FILL ring with UMEM */
    xsk_populate_fill_ring(umem);

    /* Create socket */
    xsk_info = xsk_configure_socket(receiveAddrInfo->ifname, umem, true, false);

    *sock = xsk_socket__fd(xsk_info->xsk);
    recvSock = sock;

    return SOPC_STATUS_OK;
}

#else // WITH_XDP_ETH_SOCKET not defined

static SOPC_ReturnStatus ETH_Socket_CreateToReceive(SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
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
#endif

SOPC_ReturnStatus SOPC_ETH_Socket_CreateToReceive(SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                                  bool setNonBlocking,
                                                  Socket* sock)
{
#ifdef WITH_XDP_ETH_SOCKET
    return XDP_ETH_Socket_CreateToReceive(receiveAddrInfo, setNonBlocking, sock);
#else
    return ETH_Socket_CreateToReceive(receiveAddrInfo, setNonBlocking, sock);
#endif
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

#ifdef WITH_XDP_ETH_SOCKET
static SOPC_ReturnStatus XDP_ETH_Socket_ReceiveFrom(Socket sock,
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

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint32_t rcvd = 0, ret = 0;
    uint32_t idx_rx = 0, idx_fq = 0;
    bool bres = false;

    // Treat available packets until an expected one is received or none available
    while (!bres)
    {
        // Peek if there are any new packets in the RX ring and if so we can read them from the RX ring.
        rcvd = xsk_ring_cons__peek(&xsk_info->rx, 1, &idx_rx);
        if (!rcvd)
        {
            // If the need wakeup flag is set on the FILL ring, the application needs to call poll()/recv
            // to be able to continue to receive packets on the RX ring.
            if (xsk_ring_prod__needs_wakeup(&xsk_info->umem->fq))
            {
                recvfrom(sock, NULL, 0, MSG_DONTWAIT, NULL, NULL);
            }
            return SOPC_STATUS_WOULD_BLOCK;
        }

        // Reserve as many packets as received for the FILL ring
        ret = xsk_ring_prod__reserve(&xsk_info->umem->fq, 1, &idx_fq);
        while (ret != rcvd)
        {
            if (xsk_ring_prod__needs_wakeup(&xsk_info->umem->fq))
            {
                recvfrom(sock, NULL, 0, MSG_DONTWAIT, NULL, NULL);
            }
            ret = xsk_ring_prod__reserve(&xsk_info->umem->fq, 1, &idx_fq);
        }

        // Handle 1 received packet
        // Read received packet descriptors
        uint64_t addr = xsk_ring_cons__rx_desc(&xsk_info->rx, idx_rx)->addr; // index in umem
        uint32_t len = xsk_ring_cons__rx_desc(&xsk_info->rx, idx_rx)->len;

        // Only needed in unaligned mode to retrieve offset:
        // (offset used is carried in the upper 16 bits of the addr)
        uint64_t orig = xsk_umem__extract_addr(addr);
        addr = xsk_umem__add_offset_to_addr(addr);

        //  Get a pointer to the packet data itself inside the umem
        char* pkt = xsk_umem__get_data(xsk_info->umem->buffer, addr);

        assert(buffer->current_size >= len);

        // Reset buffer content since it is reused for reception
        buffer->position = 0;
        buffer->length = 0;
        // Note: it might be possible to attach the data to buffer and then release it after decoding
        status = SOPC_Buffer_Write(buffer, (uint8_t*) pkt, len);
        buffer->position = 0;

        // Push back the umem frame read into the FILL ring
        *xsk_ring_prod__fill_addr(&xsk_info->umem->fq, idx_fq) = orig;

        if (SOPC_STATUS_OK == status)
        {
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

        // Submit reserved packets for FILL ring
        xsk_ring_prod__submit(&xsk_info->umem->fq, 1);
        // Release the RX ring packet descriptor
        xsk_ring_cons__release(&xsk_info->rx, 1);
    }

    if (!bres)
    {
        status = SOPC_STATUS_NOK;
    }

    return status;
}
#else // WITH_XDP_ETH_SOCKET not defined

static SOPC_ReturnStatus ETH_Socket_ReceiveFrom(Socket sock,
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
#endif

SOPC_ReturnStatus SOPC_ETH_Socket_ReceiveFrom(Socket sock,
                                              const SOPC_ETH_Socket_ReceiveAddressInfo* receiveAddrInfo,
                                              bool checkEtherType,
                                              uint16_t etherType,
                                              SOPC_Buffer* buffer)
{
#ifdef WITH_XDP_ETH_SOCKET
    return XDP_ETH_Socket_ReceiveFrom(sock, receiveAddrInfo, checkEtherType, etherType, buffer);
#else
    return ETH_Socket_ReceiveFrom(sock, receiveAddrInfo, checkEtherType, etherType, buffer);
#endif
}

#ifdef WITH_XDP_ETH_SOCKET
static void xdpsock_cleanup(void)
{
    struct xsk_umem* umem = xsk_info->umem->umem;
    xsk_socket__delete(xsk_info->xsk);
    (void) xsk_umem__delete(umem);
}
#endif

void SOPC_ETH_Socket_Close(Socket* sock)
{
    if (NULL != sock)
    {
#ifdef WITH_XDP_ETH_SOCKET
        if (sock == recvSock)
        {
            xdpsock_cleanup();
        }
        else
#endif
        {
            close(*sock);
        }
        *sock = SOPC_INVALID_SOCKET;
    }
}
