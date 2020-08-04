# Instructions to build a demo OPCUA server with a Publisher and Subscriber module on native posix target of Zephyr (tested on Zephyr 2.3.99 – commit 979e124…)
# Setup building environment for Zephyr

Follow the installation instructions described on the following site to create zephyr home directory (zephyrproject) and install zephyr sdk: https://docs.zephyrproject.org/latest/getting_started/index.html

# Get S2OPC library and OPC-UA demo application
It is assumed for the rest of the document that source code of zephyr has been retrieved from ~/zephyrproject with the following commands:

```
west init ~/zephyrproject
cd ~/zephyrproject
west update
```

This modification is temporary, the time of the final integration of the S2OPC library as an optional module by Zephyr.

Edit the file ~/zephyrproject/zephyr/west.yml and add the following content:

```
  #
  # Please add items below based on alphabetical order
  projects:
    - name: s2opc
      path: modules/lib/s2opc
      url: https://gitlab.com/systerel/S2OPC
      revision: zephyr
```

# Get S2OPC library with the following commands:

```
cd ~/zephyrproject
west update
```

# Setup linux virtual network
To test OPC-UA server with publisher / subscriber module, we need to instantiate a bridge between Linux Host virtual interface and the two virtual interfaces which will be connected to native posix virtual ethernet interface of OPC-UA applications.

The final network we want to instantiate is illustrated by the following schema:

```
Linux Host with an OPC-UA client (UAExpert by example) 
Virtual bridge zeth-br
[192.0.2.2/24 – HW of zeth-1 and HW of zeth-2]


Virtual ethernet port zeth-1 				virtual ethernet of OPCU-UA application 1 
[HW 00:00:5e:00:53:ff]						[192.0.2.10/24 - HW 00:00:5e:00:53:01]


Virtual ethernet port zeth-2				virtual ethernet of OPCU-UA application 2
[HW 00:00:5e:00:54:ff] 						[192.0.2.11/24 - HW 00:00:5e:00:54:01]

This configuration allow:
a) TCP connection between OPC-UA client and OPC-UA server application 1
b) TCP connection between OPC-UA client and OPC-UA server application 2
c) UDP multicast flow from OPC-UA Publisher module 1 to OPC-UA Subscriber module 2
d) UDP multicast flow from OPC-UA Publisher module 2 to OPC-UA Subscriber module 1
```

# Instantiate zeth.x ports

```
cd ~/zephyrproject
sudo ip tuntap add zeth.1 mode tap
sudo ip link set dev zeth.1 up
sudo ip link set dev zeth.1 address 00:00:5e:00:53:ff
sudo ip tuntap add zeth.2 mode tap
sudo ip link set dev zeth.2 up
sudo ip link set dev zeth.2 address 00:00:5e:00:54:ff
```

# Instantiate zeth-br bridge

```
cd ~/zephyrproject
sudo brctl addbr zeth-br
sudo ip link set dev zeth-br up
sudo ip address add 192.0.2.2/24 dev zeth-br
sudo ip route add 192.0.2.0/24 dev zeth-br
sudo brctl addif zeth-br zeth.1
sudo brctl addif zeth-br zeth.2
```

# Remove virtual network

If you want to remove the virtual network, type the following commands:

Remove bridge: 

```
cd ~/zephyrproject
sudo ip link set dev zeth-br down
sudo brctl delbr zeth-br
```

Remove zeth.x ports:

```
sudo ip link set dev zeth.1 down
sudo ip tuntap del zeth.1 mode tap
sudo ip link set dev zeth.2 down
sudo ip tuntap del zeth.2 mode tap
```

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
```
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
```
# Instantiate OPC-UA PubSub server 2

The second instance of the OPC-UA PubSub server listen for OPC-UA Client connection on the following address:
opc.tcp://192.0.2.11:4841 (ipv4 address defined by CONFIG_NET_CONFIG_MY_IPV4_ADDR)

It is connected on the bridge port zeth.2 (CONFIG_ETH_NATIVE_POSIX_DRV_NAME) via its interface with mac address set to 00:00:5e:00:54:01.
It publishes message 14 and message 15 on 232.1.2.101:4840 (CONFIG_PUBLISHER_ADDRESS).
It subscribes message 14 and message 15 published on 232.1.2.100:4840. (CONFIG_SUBSCRIBER_ADDRESS).

```
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
```

# Demo 2 OPC-UA Pubsub on SAM E70 EXPLAINED emulated by Renode

First of all, to install Renode virtual environment, clone renode to your home directory then follow the steps described on the following link: https://github.com/renode/renode/blob/master/README.rst#installation

For the next steps, it is assumed that renode git repository has been cloned into ~/renode, and that source code of zephyr has been retrieved from ~/zephyrproject

# Setting up wired network for SAM E70 EXPLAINED emulated by Renode

To evaluate an OPC-UA server with publisher / subscriber module, we need to instantiate a virtual Ethernet interface on the Linux Host. This interface will be connected to a renode virtual switch port.
2 virtual renode machines will be instanciated then connected to this virtual renode switch.
The final network we want to instantiate is illustrated by the following schema:

```
** Linux world **
Host with an OPC-UA client (UAExpert by example) 
Virtual ethernet tap0
[192.0.2.2/24 – HW 00:00:00:03:03:03]
                       | 
** Renode world **     |
		       |
		       |		Emulated ethernet interface of OPCU-UA application 1 
		       |    -------------[192.0.2.10/24 - HW 00:00:00:01:01:01]
		       |    |
   		Emulated switch 
		            |		Emulated ethernet interface of OPCU-UA application 2
			    -------------[192.0.2.11/24 - HW 00:00:00:02:02:02]
This configuration allow:
a) TCP connection between OPC-UA client and OPC-UA server application 1
b) TCP connection between OPC-UA client and OPC-UA server application 2
c) UDP multicast flow from OPC-UA Publisher module 1 to OPC-UA Subscriber module 2
d) UDP multicast flow from OPC-UA Publisher module 2 to OPC-UA Subscriber module 1
```

Each UDP multicast flow transports 2 messages transmitted each 1000ms, a message with its identifier value set to 14, which transports 2 datasets (Boolean and String data), and a message with its identifier set to 15, which transports 2 others datasets (Int64 and UInt64 data).

1) Open a new terminal then create virtual Ethernet tap0

```
cd ~/zephyrproject
sudo ip tuntap add tap0 mode tap
sudo ip link set dev tap0 up
sudo ip link set dev tap0 address 00:00:00:03:03:03
sudo ip address add 192.0.2.2/24 dev tap0
sudo ip route add 192.0.2.0/24 dev tap0
```

2) Open a new terminal then start Renode monitor 

```
cd ~/zephyrproject
sudo renode
```

3) From the renode monitor, create a virtual switch then connect it to tap0

```
(monitor) using sysbus
(monitor) emulation CreateSwitch "switch"
(monitor) emulation CreateTap "tap0" "tap"
(monitor) connector Connect host.tap switch
```

# Patch of zephyr kernel for v2.3.99 and before (sam e70 explained emulated board)

In order to configure MAC address from build command line, we modify SAM E70 ethernet driver with the following patch:

1) Open the following file:

```
~/zephyrproject/zephyr/drivers/ethernet/KConfig.sam_gmac
```

2) Add the following lines:

```
config ETH_SAM_GMAC_MAC_STATIC
	string "MAC address for the interface"
	default ""
	depends on !ETH_SAM_GMAC_MAC_I2C_EEPROM
	help
	  Specify a MAC address for the ethernet
```

3) Open the following file:

```
~/zephyrproject/zephyr/drivers/ethernet/eth_sam_gmac.c
```

4) Modify the function used to generate MAC address following lines: 

```
static void generate_mac(uint8_t mac_addr[6])
{
 #if defined(CONFIG_ETH_SAM_GMAC_MAC_I2C_EEPROM)
 	get_mac_addr_from_i2c_eeprom(mac_addr);
 #elif DT_INST_PROP(0, zephyr_random_mac_address)
 	gen_random_mac(mac_addr, ATMEL_OUI_B0, ATMEL_OUI_B1, ATMEL_OUI_B2);
 #else
	if (CONFIG_ETH_SAM_GMAC_MAC_STATIC[0] != 0)
	{
	    if (net_bytes_from_str(mac_addr, 6*sizeof(uint8_t), CONFIG_ETH_SAM_GMAC_MAC_STATIC) < 0)
	    {
	        LOG_ERR("Invalid MAC address %s", CONFIG_ETH_SAM_GMAC_MAC_STATIC);
	    }
	}
 #endif
}
```

# Build the 2 OPC-UA Pub Sub Server binaries for SAM E70 EXPLAINED emulated by Renode

For each application, 7 parameters will be set by compilation process: 
- ignore EEPROM MAC configuration (CONFIG_NET_CONFIG_MY_IPV4_ADDR set to no)
- mac address of the interface of the instance (CONFIG_ETH_SAM_GMAC_MAC_STATIC)
- ipv4 of the interface of the instance (CONFIG_NET_CONFIG_MY_IPV4_ADDR)
- net mask of the interface of the instance (CONFIG_NET_CONFIG_MY_IPV4_NETMASK)
- default gateway ipv4 address (CONFIG_NET_CONFIG_MY_IPV4_GW)
- SOPC Publisher multicast address and port for emission (CONFIG_PUBLISHER_ADDRESS)
- SOPC Subscriber multicast address and port for reception (CONFIG_SUBSCRIBER_ADDRESS)

Those parameters are defined by kernel configuration variables. Default values are defined by the sample project configuration file, which can be retrieved from the following path:

```
~\zephyrproject\modules\lib\s2opc\samples\PubSub_ClientServer\zephyr\pubsub_test_server\prj.conf
```

1) Build OPC-UA Pub Sub Server 1 

```
cd ~/zephyrproject
west build -fp -d ./build/pubsub_sam_1/ -b sam_e70_xplained ./modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server/  -G"Eclipse CDT4 - Ninja" \
   -DCONFIG_NET_CONFIG_MY_IPV4_ADDR=\"192.0.2.10\" \
   -DCONFIG_NET_CONFIG_MY_IPV4_NETMASK=\"255.255.255.0\" \
   -DCONFIG_NET_CONFIG_MY_IPV4_GW=\"192.0.2.2\" \
   -DCONFIG_ETH_SAM_GMAC_MAC_I2C_EEPROM=n \
   -DCONFIG_ETH_SAM_GMAC_MAC_STATIC=\"00:00:00:01:01:01\" \
   -DCONFIG_PUBLISHER_ADDRESS=\"232.1.2.100:4840\" \
   -DCONFIG_SUBSCRIBER_ADDRESS=\"232.1.2.101:4840\"
```

2) Build OPCUA Pub Sub Server 2

```
cd ~/zephyrproject
west build -fp -d ./build/pubsub_sam_2/ -b sam_e70_xplained ./modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server/  -G"Eclipse CDT4 - Ninja" \
   -DCONFIG_NET_CONFIG_MY_IPV4_ADDR=\"192.0.2.11\" \
   -DCONFIG_NET_CONFIG_MY_IPV4_NETMASK=\"255.255.255.0\" \
   -DCONFIG_NET_CONFIG_MY_IPV4_GW=\"192.0.2.2\" \
   -DCONFIG_ETH_SAM_GMAC_MAC_I2C_EEPROM=n \
   -DCONFIG_ETH_SAM_GMAC_MAC_STATIC=\"00:00:00:02:02:02\" \
   -DCONFIG_PUBLISHER_ADDRESS=\"232.1.2.101:4840\" \
   -DCONFIG_SUBSCRIBER_ADDRESS=\"232.1.2.100:4840\"
```

# Run the 2 OPC-UA Pub Sub Server for SAM E70 EXPLAINED emulated by Renode

It is assumed that renode has been started from ~/zephyrproject and its repository has been cloned into ~/renode.

1) From renode terminal launched from ~/zephyrproject directory, set configuration variables:

```
(monitor) set bin_pubsub1 @./build/pubsub_sam_1/zephyr/zephyr.elf
(monitor) set bin_pubsub2 @./build/pubsub_sam_2/zephyr/zephyr.elf
(monitor) set name_pubsub1 “PUBSUB1”
(monitor) set name_pubsub2 “PUBSUB2”
```

2) Create machine PUBSUB1

```
(monitor) mach create $name_pubsub1
(PUBSUB1) machine LoadPlatformDescription @../renode/platforms/cpus/sam_e70.repl
(PUBSUB1) cpu PerformanceInMips 125
(PUBSUB1) showAnalyzer sysbus.usart1
(PUBSUB1) logLevel 3 nvic
(PUBSUB1) sysbus LoadELF $bin_pubsub1 	#Chargement binaire
(PUBSUB1) connector Connect gem switch 	#Connexion switch
(PUBSUB1) start		  		#Start machine
(PUBSUB1) mach clear			#Switch from machine PUBSUB1 to monitor cmd line
(monitor) #mach set $name_pubsub2	#Switch from monitor cmd line to PUBSUB1 cmd line
```

3) Create machine PUBSUB2

```
(monitor) mach create $name_pubsub2
(PUBSUB2) machine LoadPlatformDescription @../renode/platforms/cpus/sam_e70.repl
(PUBSUB2) cpu PerformanceInMips 125
(PUBSUB2) showAnalyzer sysbus.usart1
(PUBSUB2) logLevel 3 nvic
(PUBSUB2) sysbus LoadELF $bin_pubsub2 	#Chargement binaire
(PUBSUB2) connector Connect gem switch 	#Connexion switch
(PUBSUB2) start		  		#Start machine
(PUBSUB2) mach clear			#Switch from machine PUBSUB2 to monitor cmd line
(monitor) #mach set $name_pubsub2	#Switch from monitor cmd line to PUBSUB2 cmd line
```

4) To stop and remove machines, type the following command line

```
(monitor) mach rem $name_pubsub1
(monitor) mach rem $name_pubsub2
```

5) Running and stopping machine can use the following scripts 
```
(monitor) s @./modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server/start_2_pubsub.resc
(monitor) s @./modules/lib/s2opc/samples/PubSub_ClientServer/zephyr/pubsub_test_server/stop_2_pubsub.resc
```

