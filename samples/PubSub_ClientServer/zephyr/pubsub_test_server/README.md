# Instructions to build a demo OPCUA server with a Publisher and Subscriber module on native posix target of Zephyr (tested on Zephyr 2.3.99 – commit 979e124…)
# Setup building environment for Zephyr

Follow the installation instructions described on the following site to create zephyr home directory (zephyrproject) and install zephyr sdk: https://docs.zephyrproject.org/latest/getting_started/index.html

# Get S2OPC library and OPC-UA demo application
It is assumed for the rest of the document that source code of zephyr has been retrieved from ~/zephyrproject with the following commands:

west init ~/zephyrproject
cd ~/zephyrproject
west update

This modification is temporary, the time of the final integration of the S2OPC library as an optional module by Zephyr.

Edit the file ~/zephyrproject/zephyr/west.yml and add the following content:

…
  #
  # Please add items below based on alphabetical order
  projects:
    - name: s2opc
      path: modules/lib/s2opc
      url: https://gitlab.com/systerel/S2OPC
      revision: zephyr
…

# Get S2OPC library with the following commands:

cd ~/zephyrproject
west update

# Setup linux virtual network
To test OPC-UA server with publisher / subscriber module, we need to instantiate a bridge between Linux Host virtual interface and the two virtual interfaces which will be connected to native posix virtual ethernet interface of OPC-UA applications.

The final network we want to instantiate is illustrated by the following schema:

Linux Host with an OPC-UA client (UAExpert by example) 

Virtual bridge zeth-br
[192.0.2.2/24 – HW of zeth-1 and HW of zeth-2]


Virtual ethernet port zeth-1 				virtual ethernet of OPCU-UA application 1 
[HW 00:00:5e:00:53:ff]						[192.0.2.10/24 - HW 00:00:5e:00:53:01]


Virtual ethernet port zeth-2				virtual ethernet of OPCU-UA application 2
[HW 00:00:5e:00:54:ff] 						[192.0.2.11/24 - HW 00:00:5e:00:54:01]

This configuration allow:
	TCP connection between OPC-UA client and OPC-UA server application 1
	TCP connection between OPC-UA client and OPC-UA server application 2
	UDP multicast flow from OPC-UA Publisher module 1 to OPC-UA Subscriber module 2
	UDP multicast flow from OPC-UA Publisher module 2 to OPC-UA Subscriber module 1


# Instantiate zeth.x ports

cd ~/zephyrproject
sudo ip tuntap add zeth.1 mode tap
sudo ip link set dev zeth.1 up
sudo ip link set dev zeth.1 address 00:00:5e:00:53:ff
sudo ip tuntap add zeth.2 mode tap
sudo ip link set dev zeth.2 up
sudo ip link set dev zeth.2 address 00:00:5e:00:54:ff

# Instantiate zeth-br bridge

cd ~/zephyrproject
sudo brctl addbr zeth-br
sudo ip link set dev zeth-br up
sudo ip address add 192.0.2.2/24 dev zeth-br
sudo ip route add 192.0.2.0/24 dev zeth-br
sudo brctl addif zeth-br zeth.1
sudo brctl addif zeth-br zeth.2

# Remove virtual network

If you want to remove the virtual network, type the following commands:

Remove bridge: 

cd ~/zephyrproject
sudo ip link set dev zeth-br down
sudo brctl delbr zeth-br

Remove zeth.x ports:

sudo ip link set dev zeth.1 down
sudo ip tuntap del zeth.1 mode tap
sudo ip link set dev zeth.2 down
sudo ip tuntap del zeth.2 mode tap

# Patch of zephyr kernel for v2.3.99 and before (all board)

Until version 2.3.99 of the kernel, multicast transmission is only possible for ipv4 starting with 224. The
following modification shall be applied so that the L2 layer replaces the MAC address of the interface
by a multicast MAC address for ALL multicast ipv4 addresses.

Official correction progression can be followed by this currently opened issue:
https://github.com/zephyrproject-rtos/zephyr/issues/26584
The following patch can be applied:

1) Open the file ~zephyrproject/zephyr/subsys/net/l2/ethernet/ethernet.c
2) Redefine the content of the following static functions:

```
static inline bool ethernet_ipv4_dst_is_broadcast_or_mcast(struct net_pkt *pkt)
{
    if (net_ipv4_is_addr_bcast(net_pkt_iface(pkt), &NET_IPV4_HDR(pkt)->dst) ||
        net_ipv4_is_addr_mcast(&(NET_IPV4_HDR(pkt)->dst)))
    {
        return true;
    }
    return false;
}

static bool ethernet_fill_in_dst_on_ipv4_mcast(struct net_pkt *pkt, struct net_eth_addr *dst)
{
    if (net_ipv4_is_addr_mcast(&(NET_IPV4_HDR(pkt)->dst))) 
    {
        dst->addr[0] = 0x01;
        dst->addr[1] = 0x00;
        dst->addr[2] = 0x5e;
        dst->addr[3] = NET_IPV4_HDR(pkt)->dst.s4_addr[1];
        dst->addr[4] = NET_IPV4_HDR(pkt)->dst.s4_addr[2];
        dst->addr[5] = NET_IPV4_HDR(pkt)->dst.s4_addr[3];
        dst->addr[3] &= 0x7f;
        return true;
    }
    return false;
}
```
# Patch of zephyr kernel for v2.3.99 and before (mimxrt1064 board)

Until version 2.3.99 of the kernel, IPv4 multicast datagrams can't be received for mimxrt1046_evk board
(missing ethernet API).

In order to support IPv4 multicast datagrams, it is necessary to modify the mimxrt1064_evk ethernet driver
API to join a multicast group.

For mimxrt1064, it is the file eth_mcux.c, which could provide the configuration function set_config to add
or remove a multicast source adress filter to the interface.

Official correction progression can be followed by this currently opened issue:
https://github.com/zephyrproject-rtos/zephyr/issues/26585
The following patch can be applied:

1) Open the file ~zephyrproject/zephyr/drivers/ethernet/eth_mcux.c
2) Define the following static function:
```
static int set_config(     struct device *dev, 
                           enum ethernet_config_type type,
                           const struct ethernet_config *config)
{
    int result = -1;
    switch (type)
    {
    case ETHERNET_CONFIG_TYPE_FILTER:
    {
        switch (config->filter.type)
        {
        case ETHERNET_FILTER_TYPE_SRC_MAC_ADDRESS:
        {
            /* Adds or leaves the ENET device to a multicast group.*/
            uint8_t multicastMacAddr[6];
            for (uint32_t i = 0; i < 6; i++)
            {
                multicastMacAddr[i] = config->filter.mac_address.addr[i];
                if (config->filter.set)
                {
                    printk("\r\nENET_AddMulticastGroup\r\n");
                    ENET_AddMulticastGroup(ENET, multicastMacAddr);
                }
                else
                {
                    printk("\r\nENET_LeaveMulticastGroup\r\n");
                    ENET_LeaveMulticastGroup(ENET, multicastMacAddr);
                }
                result = 0;
            }
        }
        break;
        default:
        {
            //Nothing
        }
        break;
        }
        break;
        default:
        {
            //Nothing
        }
        break;
    }
    }
    return result;
}
```
3) Add the function set_config to the driver api:

```
static const struct ethernet_api api_funcs = {
    .iface_api.init = eth_iface_init,
    .get_capabilities = eth_mcux_get_capabilities,
    .send = eth_tx,
    .set_config = set_config, /*Ajout de la fonction set config a l'API*/
};
```
# Build OPC-UA Pub Sub Server
For each application, 7 parameters will be set by compilation process: 
-	port of the bridge on which is connected the instance (CONFIG_ETH_NATIVE_POSIX_DRV_NAME)
-	mac address of the interface of the instance (CONFIG_ETH_NATIVE_POSIX_MAC_ADDR)
-	ipv4 of the interface of the instance (CONFIG_NET_CONFIG_MY_IPV4_ADDR)
-	net mask of the interface of the instance (CONFIG_NET_CONFIG_MY_IPV4_NETMASK)
-	default gateway ipv4 address (CONFIG_NET_CONFIG_MY_IPV4_GW)
-	SOPC Publisher multicast address and port for emission (CONFIG_PUBLISHER_ADDRESS)
-	SOPC Subscriber multicast address and port for reception (CONFIG_SUBSCRIBER_ADDRESS)

Those parameters are defined by kernel configuration variables. Default values are defined by the sample project configuration file, which can be retrieved from the following path:

~\zephyrproject\modules\lib\s2opc\samples\PubSub_ClientServer\zephyr\pubsub_test_server\prj.conf

# Instantiate OPC-UA PubSub server 1

The first instance of the OPC-UA PubSub server listens for OPC-UA Client connection on the following address:
opc.tcp://192.0.2.10:4841 (ipv4 address defined by CONFIG_NET_CONFIG_MY_IPV4_ADDR)

It is connected on the bridge port zeth.1 (CONFIG_ETH_NATIVE_POSIX_DRV_NAME) via its interface with mac address set to 00:00:5e:00:53:01 (CONFIG_ETH_NATIVE_POSIX_MAC_ADDR).

It publishes message 14 and message 15 on 232.1.2.100:4840 (CONFIG_PUBLISHER_ADDRESS).
It subscribes message 14 and message 15 published on 232.1.2.101:4840. (CONFIG_SUBSCRIBER_ADDRESS).

cd ~/zephyrproject
west build -p -t run -d ~/zephyrproject/build/pubsub_1/ -b native_posix ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server \
   -DCONFIG_NET_CONFIG_MY_IPV4_ADDR=\"192.0.2.10\" \
   -DCONFIG_NET_CONFIG_MY_IPV4_NETMASK=\"255.255.255.0\" \
   -DCONFIG_NET_CONFIG_MY_IPV4_GW=\"192.0.2.2\" \
   -DCONFIG_ETH_NATIVE_POSIX_DRV_NAME=\"zeth.1\" \
   -DCONFIG_ETH_NATIVE_POSIX_MAC_ADDR=\"00:00:5e:00:53:01\" \
   -DCONFIG_ETH_NATIVE_POSIX_RANDOM_MAC=n \
   -DCONFIG_PUBLISHER_ADDRESS=\"232.1.2.100:4840\" \
   -DCONFIG_SUBSCRIBER_ADDRESS=\"232.1.2.101:4840\"

# Instantiate OPC-UA PubSub server 2

The second instance of the OPC-UA PubSub server listen for OPC-UA Client connection on the following address:
opc.tcp://192.0.2.11:4841 (ipv4 address defined by CONFIG_NET_CONFIG_MY_IPV4_ADDR)

It is connected on the bridge port zeth.2 (CONFIG_ETH_NATIVE_POSIX_DRV_NAME) via its interface with mac address set to 00:00:5e:00:54:01.
It publishes message 14 and message 15 on 232.1.2.101:4840 (CONFIG_PUBLISHER_ADDRESS).
It subscribes message 14 and message 15 published on 232.1.2.100:4840. (CONFIG_SUBSCRIBER_ADDRESS).

cd ~/zephyrproject
west build -p -t run -d ~/zephyrproject/build/pubsub_2/ -b native_posix ~/zephyrproject/modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server \
   -DCONFIG_NET_CONFIG_MY_IPV4_ADDR=\"192.0.2.11\" \
   -DCONFIG_NET_CONFIG_MY_IPV4_NETMASK=\"255.255.255.0\" \
   -DCONFIG_NET_CONFIG_MY_IPV4_GW=\"192.0.2.2\" \
   -DCONFIG_ETH_NATIVE_POSIX_DRV_NAME=\"zeth.2\" \
   -DCONFIG_ETH_NATIVE_POSIX_MAC_ADDR=\"00:00:5e:00:54:01\" \
   -DCONFIG_ETH_NATIVE_POSIX_RANDOM_MAC=n \
   -DCONFIG_PUBLISHER_ADDRESS=\"232.1.2.101:4840\" \
   -DCONFIG_SUBSCRIBER_ADDRESS=\"232.1.2.100:4840\"

