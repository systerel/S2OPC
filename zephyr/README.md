# HowTo : Develop a  ZEPHYR S2OPC application (PubSub + Server)

## Prerequisite
It is mandatory to have first ZEPHYR and S2OPC installed and configured properly.
See [WIKI](https://trac.aix.systerel.fr/ingopcs/wiki/ZephyrGetStarted)

These samples were tested under Zephyr v3.0.0 (4f8d78ceeb436e82f528511998515f6fc137c6cd) with the following additionnal patches:
- add s2opc in west.yaml (see below)
- change STM32 default network device to `ok`status. Hereunder is the applied patch

```diff
diff --git a/boards/arm/stm32h735g_disco/stm32h735g_disco.dts b/boards/arm/stm32h735g_disco/stm32h735g_disco.dts
index 1867493182..424cc96926 100644
--- a/boards/arm/stm32h735g_disco/stm32h735g_disco.dts
+++ b/boards/arm/stm32h735g_disco/stm32h735g_disco.dts
@@ -105,6 +105,7 @@
                     &eth_txd0_pb12
                     &eth_txd1_pb13>;
        pinctrl-names = "default";
+       status = "okay";
 };

 &sdmmc1 {
```

## Configuration
### ZEPHYR configuration

The configuration is provided in `s2opc/zephyr/Kconfig`. The default configuration is provided for a PubSub + OPC UA server application, and already meets expected constraints (except network configuration : `SOPC_ETH_ADDRESS`,  `SOPC_ENDPOINT_ADDRESS`, ...)
Note: This configuration is based on a Hardware design with a single Ethernet interface. In case several interfaces exist, this may have to be reworked.

### S2OPC configuration
The configuration is provided in `s2opc/zephyr/CMakelist.txt`. The default configuration is provided for a PubSub + OPC UA server application with constant address space (RAM-optimized). Note that when using the constant Address Space, only the DataValue of the nodes can be modified (not the META DATA like status or timestamps)

## Using S2OPC

Adding S2OPC in an external ZEPHYR project requires to:
- include s2opc module in ZEPHYR workspace. If not already included, this can be done by adding the following lines to `west.yml`:

```yaml
    - name: s2opc
      revision: zephyr
      path: modules/lib/s2opc
      url: https://gitlab.com/systerel/S2OPC.git
```

- check that s2opc is up to date (`west update s2opc`)
- In `prj.conf`, ensure to have `CONFIG_S2OPC=y`
- Check the specific configuration in `$ZEPHYR_BASE/modules/lib/s2opc/zephyr/Kconfig` and modify the `prj.conf` accordingly to project needs
- In `CMakeLists.txt`, add the s2opc build options (e.g. `add_definitions (-DWITH_STATIC_SECURITY_DATA=1)`, `add_definitions (-DWITH_USER_ASSERT=1)`, ...). Refer to the [WIKI](https://gitlab.com/systerel/S2OPC/-/wikis/compilation) for all possible options.

## Examples

Several examples are provided. 
Common files to several examples are locarted in subfolder `zephyr_common_src`

- `static_security_data` provides a hard-coded example of Pubsub symmetric keys
- `pubsub_config_static` provides a hard-coded example of PubSub configuration
- `network_init` provides a example of network configuration (tested on STM32H7 and IMX1064RT)
- `cache` provides a basic cache for PubSub communication
- `tls_config` provides the MdebTLS required parameters for its integration into S2OPC
- `server` provides a basic OPC server implementation

Note: a 'cache' can be simply seen as a basic dictionnary containing both variables to be read by "Pub" and writton to by "Sub".

### Building the examples
Using the usual ZEPHYR environment, simply use `west build .` from each folder to compile samples.
A sample `Makefile` is provided as helper, but is not mandatory.


### Testing the examples : Server/client
Note:
- The default IP of demo server is `192.168.42.21/24`
- The default IP of demo client is `192.168.42.22/24`
- It is advised to clean fully the build folders when changing BOARDS. (`west build -t pristine`)
- The native ZEPHYR `NET_SHELL` tools can be used on both cards. (e.g. type `net iface` to show network configuration). Additionnal command `demo` is also available, for the test purpose. Type `demo` in the console to get help.

It is possible to modify the content of the addresss space. Follow these steps before compilation to test different possibilies:
- update the `xml/pubsub_server.xml` file
- start `make regen` command to regenerate the `test_address_space.c` file

Steps:
- Build the server : `cd zephyr/samples/zephyr_server && west build`
- Install the server on first target (`TARG_SRV`). For example, installing an STM32 target on Windows can be done using the command `copy build\zephyr\zephyr.bin E:` (assuming that `E:` is the mount point of `TARG_SERV`)
- Build the client : `cd zephyr/samples/zephyr_clientr && west build`
- Install the client on secong target (`TARG_CLI`).
- Ensure both cards are connected on the same network.
- Open serial monitor (`COM_SRV`) of `TARG_SRV`
- Open serial monitor (`COM_CLI`) of `TARG_CLI`
- (Note: This can be done, for example, using `putty`on windows or `minicom` on linux.)
- Reboot `TARG_SRV` and check output on `COM_SRV`. The output should show something like:

```
# Info: Server initialized.
# Initializing server at EP: opc.tcp://192.168.42.21:4841
# Address space loaded
# Info: Endpoint and Toolkit configured.
# Info: Server started.
# Server started on <opc.tcp://192.168.42.21:4841>
```

- Reboot `TARG_CLI` and check output on `COM_CLI`.
- On `COM_CLI`, create a configuration to the server using the command ` demo conf  opc.tcp://192.168.42.21:4841`
- Launch the browse demo on server using the command `demo conn` on `COM_CLI`
- In case of success, `COM_CLI` will display browse result of `root.Objects` node.

### Testing the examples : ZEPHYR S2OPC PubSub demo

Notes:
- This demo was checked on `mimxrt1064_evk` and `nucleo_h745zi_q_m7` targets
- `zephyr_pubsub` provides an example of cache-based S2OPC PubSub demo over Zephyr (Not based on an OPC address space). The same executable can be deployed over different targets. As the communication is Multicast, there is no issue in the fact that all targets use the same IP address.
- The same MC address is used for both Pub & Sub, so that the same binary can be used for the demo. Feel free to change `CONFIG_SOPC_PUBLISHER_ADDRESS` and `CONFIG_SOPC_SUBSCRIBER_ADDRESS` to test different configurations.
- The native ZEPHYR `NET_SHELL` tools can be used on both cards. (e.g. type `net iface` to show network configuration). Additionnal command `demo` is also available, for the test purpose. Type `demo` in the console to get help.
Steps:
- Install the demo on (at least) 2 targets (`TARG_PUB` and `TARG_SUB`)

- Type the command `demo info` on both targets. This should output the global configuration and status (by default, both Pub and Sub are not started)
- Type `demo print` to show the content of Cache. By default all values are set to 0/empty
- Start the publisher on `TARG_PUB` with the command `demo start pub`
- Start the subscriber on `TARG_SUB` with the command `demo start sub`
- Update any value on `TARG_PUB` (for example with the command `demo set ns=1;s=aString "Hello World!"`)
- Check that the content has been updated on `TARG_SUB` (for example with `demo print ns=1;s=aString`)


### Testing the examples : ZEPHYR S2OPC PtP demo

`zephyr_ptp` provides an example of PtP clock synchronisation on Zephyr.

The demo requires an hardware PtP-capable device to run successfully. It has been tested on STM NUCLEO-144 series.
This demo is interactive and allows the user to start/stop specific clock-related measurement tests.
When connected to a Grand Master clock on a TSN-capable network, the terminal will show every PtP state transition (SLAVE/MASTER/NotConnected).
Use the different commands of the terminal to check PtP precision.

Notes:
- Before compiling, check in `zephyr_ptp_demo`, update the following part to add your device serial number and link a user name and IP:

```c
static const DeviceIdentifier gDevices[]=
{
        // Note: Example to be adapted for each device/configuration!
        {"624248Q", "192.168.42.111", "N.144/A"},
        {"652248Q", "192.168.42.112", "N.144/B"},
};
```
- When the device is not registered, its Id will be displayed on demo start output. Starting the demo once thus permits to get the device ID.
- The device Id will be accepted by matching the beginning of the provided identifier (for example "652248Q" actually matches `652248Q\x13\x00F\x00/`)
- Check the PtP parameters relatively to the remote NTP switch configuration in `prj.conf`

Steps:
- Install on `demo_zephyr_ptp` on a PtP compatible ZEPHYR device. (Tested on NUCLEO 144 with STM32H745ZI_Q-m7)
- Connect the target to a PtP-capable switch.
- Enter `demo info` to show current status.
- Enter `demo wait` to wait for the PtP synchronization
- Type `demo help` to li=st all available tests. For example, the PtP time synchronization can be checked with the command `demo date`

## Attention points
### Address Space
When using the Cache (read/write), it is mandatory to enclose uses by calls to `Cache_Lock` and `Cache_Unlock` to ensure integrity of content. Thus, any user-defined tasks that accesses the cache will prevent the PubSub events to occure in time if the cache locking time is not the shortest possible. Typically, if a value received on the Subscriber must be repeated onto the OPC server, it is important to first read the `DataValue` into a local variable, then release the cache, and then update the OPC server using the local variable.
(Note that the cache could be updated with a double-buffer so as to ensure non blocking accesses)

Example:

```c
SOPC_DataValue* dvCopy = SOPC_Malloc(sizeof(*dvCopy));
SOPC_DataValue_Initialize(dvCopy);
SOPC_ASSERT(NULL != dvCopy);
SOPC_NodeId* pNid = SOPC_NodeId_FromCString(SERVER_STATUS_SUB_VAR_INFO, strlen(SERVER_STATUS_SUB_VAR_INFO));

Cache_Lock();
SOPC_DataValue* dv = Cache_Get(pNid);
if (NULL != dv){
     // Copy to avoid locking Cache for too long
     SOPC_DataValue_Copy(dvCopy, dv);
}
Cache_Unlock();

Server_LocalWriteSingleNode(pNid, dvCopy);

SOPC_DataValue_Clear (dvCopy);
SOPC_Free(dvCopy);
SOPC_NodeId_Clear(pNid);
SOPC_Free(pNid);

```

### Configuration tuning
The ZEPHYR HAL for S2OPC provides an instrumentation of stacks and memory allocation. These instrumentations should be disabled in a project release but can be used during development to ensure optimization of different resources allocations.

`CONFIG_SOPC_HELPER_IMPL_INSTRUM`
- 0 (default): Release build without instrumentation.
- 1 Instrumentation is active. This allows :
    - Dumping current stacks usages (see `SOPC_Thread_GetAllThreadsInfo`)
    -	Checking memory leaks (see `SOPC_MemAlloc_Nb_Allocs`)


