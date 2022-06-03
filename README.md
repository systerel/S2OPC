## S2OPC OPC UA Toolkit

This OPC UA Toolkit project provides a Client/Server C source code implementation
including an OPC UA communication stack, B model and C source code
implementation for a minimal set of services and a cryptographic
library adaptation for OPC UA needs (using mbedtls).
This OPC UA Toolkit project also provides a PubSub C source code implementation,
it shares some components with Client/Server part (cryptographic services, OPC UA types, etc.).

The S2OPC OPC UA Client / server Toolkit is the result of the Research and Development project INGOPCS (2016-2018).
Systematic and Minalogic centers of innovation supported the project as part of the French FUI CFP.

INGOPCS goals:
- Develop an open-source and secure implementation of the OPC-UA standard (IEC 62541),
- Verify the implementation with specialized tools (applying formal methods),
- Demonstrate conformance to OPC-UA interoperability requirements,
- Demonstrate compliance of the OPC-UA standard with the security requirements of sensitive Industrial Control Systems.

INGOPCS initial consortium:
- Systerel (leader)
- ARC Informatique
- Atos Worldgrid
- CEA List
- EDF
- Eolane
- Schneider Electric Automation
- Telecom ParisTech
- TrustInSoft

## Certification

The sample server application (toolkit_test_nano_server) created using S2OPC toolkit has been certified by OPC Foundation independent test lab according to the following OPC UA profiles:
- Nano Embedded Device Server
- SecurityPolicy - Basic256
- SecurityPolicy - Basic256Sha256
- User Token-Anonymous Facet
- User Token - User Name
- Password Server Facet

For details, see:
- https://opcfoundation.org/products/view/safe-and-secure-opc
- https://www.s2opc.com/wp-content/uploads/2020/07/comm-systerel-s2opc-certifopc-en-a.pdf

## S2OPC Client/Server Toolkit features

### Common features:

- Asynchronous user application API
- Available security policies (encryption schemes) with any security mode:
    - http://opcfoundation.org/UA/SecurityPolicy#None,
    - http://opcfoundation.org/UA/SecurityPolicy#Basic256,
    - http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256,
    - http://opcfoundation.org/UA/SecurityPolicy#Aes128_Sha256_RsaOaep,
    - http://opcfoundation.org/UA/SecurityPolicy#Aes256_Sha256_RsaPss.
- UserNameIdentity token password encryption for the available security policies

Client side (e.g.: `samples/ClientServer/demo_client`):

- Secure Channel configuration
- Activate a session with an anonymous use or user identified by username/password
- Send a service on session request (read, write, browse, subscribe, etc.)
- Send a discovery service request (getEndpoints, findServer, registerServer, etc.)
- Automated client libraries wrapper and LibSub (simplified interface, automated subscription):
  see `src/ClientServer/frontend/client_wrapper/libs2opc_client_cmds.h` (e.g. `samples/ClientServer/client_wrapper`) for wrapper or `src/ClientServer/frontend/client_wrapper/libs2opc_client.h` for the LibSub
- Python wrapper PyS2OPC for a client: see  `src/ClientServer/frontend/pys2opc/README.md`

Server side (e.g.: `samples/ClientServer/demo_server/toolkit_demo_server.c`):

- Endpoint descriptions configuration on Toolkit initialization
- 1 address space configuration on Toolkit initialization
- Checks and accepts several instances of secure channels
- Checks and accepts activation of several sessions with an anonymous user or user identified by username/password
- Supported services:
  - Read service
  - Write service
  - Browse service
  - BrowseNext service
  - TranslateBrowsePathToNodeIds service
  - GetEndpoints service
  - FindServers service
  - FindServersOnNetwork service
  - RegisterServer2 service
  - RegisterNodes service
  - Only if compiled with WITH_NANO_EXTENDED set to 1:
    - Subscription services (simplified: no monitoredItems filters or deletion, no subscription transfer)

### Current status

- Security policies available: None, Basic256 and Basic256Sha256
- Security modes available: None, Sign and SignAndEncrypt
- Server instantiation: several endpoint descriptions, 1 address space, multiple secure channel instances and session instances
- Server discovery services: getEndpoints, findServers
- Server Nano profile services: read, write, browse, browseNext (1 continuation point per session), translateBrowsePath, registerNodes
- Server extended services (only if compiled with WITH_NANO_EXTENDED set to 1):
  - subscription (no monitoredItems deletion, no subscription transfer),
  - method call (no access to node object nor address space during call, any other event blocked during function execution).
- Server local services: read, write, browse and discovery services
- Server address space modification:
  - mechanisms implemented for remote modification: variables modification with typechecking (data type and value rank), access level and user access level control
  - notification: write notification events are reported through application API
- Client instantiation: multiple secure channel instances and session instances
- Client services requests: any discovery service or service on session request. Requests are only forwarded to server (no functional behavior)
- Address space with all mandatory attributes: AccessLevel, BrowseName, ContainsNoLoop, DataType, DisplayName, EvenNotifier, Executable, Historizing,
  IsAbstract, NodeClass, NodeId, Symmetric, UserAccessLevel, UserExecutable, Value (with single value and array Variants), ValueRank (and References)

### Address space definition and generation

#### Definition of XML address space
The address space should be defined with an XML using the UANodeSet XML format.
All the reciprocal references between nodes shall be present in it, the script `scripts/gen-reciprocal-refs-address-space.xslt`
might be used to generate reciprocal references missing (XSLT 2.0 processor required).

Once the XML address space is defined, the C structure to be used for toolkit configuration shall be generated.
It could be done using the Python C code generator prior to compilation or using the dynamic XML loader using Expat library.

#### Generation of C structure address space
The `scripts/generate-s2opc-address-space.py` tool converts a UANodeSet XML file into a
C file that can be compiled in the binary, and used with the embedded address
space loader (see the `samples/ClientServer/data/address_space/s2opc.xml`
file for example). Not all the features of the schema are supported at the
moment.

S2OPC demo server can also dynamically load a UANodeSet XML at startup.
To do so, set `TEST_SERVER_XML_ADDRESS_SPACE` to the location of the address space and launch the sample server.

### Server configuration

The server configuration can be defined manually using the C structures defined in src/ClientServer/api_toolkit/sopc_user_app_itf.h (SOPC_S2OPC_Config).
It is also possible to use an XML parser for XML complying with schemas/s2opc_clientserver_config.xsd.
The S2OPC demo server uses the custom XML configuration file path set `TEST_SERVER_XML_CONFIG`.

The users authentication and authorization functions should be implemented by applicative code and be compliant with src/ClientServer/configuration/sopc_user_manager.h
It is also possible to use a default implementation defining the list of valid users with global rights on address space based on XML file complying with schemas/s2opc_clientserver_users_config.xsd.
The S2OPC demo server uses the custom XML configuration file path set `TEST_USERS_XML_CONFIG`.

## S2OPC PubSub Toolkit features

S2OPC PubSub implements the OPC UA publish subscribe pattern (OPC UA standard Part 14) which complements the Client/Server pattern defined by the Services (OPC UA standard Part 4).

Note: PubSub is only available on S2OPC Toolkit version > 0.11.0

### PubSub implemented features

The S2OPC PubSub implementation properties are the following:
- the communication layer is Ethernet only, UDP or MQTT (Paho library required),
- the encoding type is UADP,
- security is available (with a local SKS implementation),
- no override management (cf §6.2.10). That means LastUsableValue is used in any cases,
- variable management only, not events,
- no additional verification on sequence numbers,
- the encoding of the dataset is always of type variant (see §6.2.3.2),
- keyframecount is set to 0 (no delta message are sent),
- no calculation of deadband (see §6.2.2.6),
- a single level of priority for WriterGroup (see §6.5.2.4),
- sampling of data at the time of publication instead of sampling (same for subscriptions see §6.3.1),
- no use of SamplingOffset nor PublishingOffset,
- no implementation of discovery requests (see §7.2.2.4).
Note: references point to the standard OPC UA Part 14 (1.04).

## S2OPC Development

- systematic peer review using GitLab Merge Request mechanism,
- formal modeling of OPC-UA services using [B method](https://en.wikipedia.org/wiki/B-Method),
- static analysis using [Frama C](http://frama-c.com/) from CEA and [TIS analyser](https://taas.trust-in-soft.com/) from Trust-In-Soft,
- static analysis using [Coverity](https://scan.coverity.com/),
- check for memory leaks using [Valgrind](http://valgrind.org/),
- use of GCC sanitizers to detect undefined behaviours, race conditions, memory leaks and errors,
- compilation using several compilers (GCC, CLang, mingGw, MSVC, ...) with conservative compilation flags,
- all development and testing environment are bundled into [Docker](https://www.docker.com/) images
- continuous integration with a test bench containing:
    - modular tests using libcheck
    - validation and interoperability tests using [FreeOPCUA](https://github.com/FreeOpcUa/python-opcua) for Client/Server and [Open62541](https://open62541.org/) for PubSub,
    - certification and interoperability tests of Server using [UACTT] (https://opcfoundation.org/developer-tools/certification-test-tools/opc-ua-compliance-test-tool-uactt/),
    - fuzzing tests.

## Demo

See the [demo](https://gitlab.com/systerel/S2OPC/wikis/demo) page from the Wiki.

## Getting started

### Client/Server
For a sample server, you can look at `samples/ClientServer/demo_server/toolkit_demo_server.c`.
And for the client side, you can look either at the samples without frontend `samples/ClientServer/demo_client/s2opc_*.c` or the samples using the additional client frontend `samples/ClientServer/client_wrapper/`.

At the end of build process, the binaries are available in `build/bin` directory.

### PubSub

For a sample PubSub-Server, you can look at `samples/PubSub_ClientServer/pubsub_server/main.c`.

At the end of build process, the binary is available here `build/bin/pubsub_server`.

## S2OPC Linux compilation

Tested under Debian 9.
Prerequisites:
- Make (tested with GNU Make version 4.2.1)
- CMake (>= 3.5, tested with CMake version 3.9.4)
- GCC (tested with GCC version 11.2.0)
- [Mbedtls](https://tls.mbed.org/)(tested with mbedtls version 2.16.12)
- [Check](https://libcheck.github.io/check/)(tested with libcheck version 0.14 compiled with CMake)
- [expat](https://github.com/libexpat/libexpat)(require libexpat version >= 2.2.10 compiled with CMake)
- Python3 (tested with version 3.6.3)
- [Paho](https://github.com/eclipse/paho.mqtt.c) only needed for PubSub with MQTT (tested with version 1.3.4 compiled with CMake)

To build S2OPC library, samples and tests with default configuration on current stable release:
```
  git clone https://gitlab.com/systerel/S2OPC.git --branch S2OPC_Toolkit_1.2.0
  cd S2OPC
  ./build.sh
```
For more information, or to compile the master branch on its latest commit, please refer to the [wiki](https://gitlab.com/systerel/S2OPC/wikis/compilation).

### Linux compilation with GCC security hardening options

In order to improve binaries security, some compilation options might be activated by setting following variables:
- `SECURITY_HARDENING=1`: activate several program instrumentation options during compilation
- `POSITION_INDEPENDENT_EXECUTABLE=1`: produces position-independent executable to support address space layout randomization (ASLR)
- `USE_STATIC_EXT_LIBS=0`: if active use static version of external libraries, set this variable to 0 avoid embedding external libraries into the binaries.

To build with those options:
```
SECURITY_HARDENING=1 POSITION_INDEPENDENT_EXECUTABLE=1 USE_STATIC_EXT_LIBS=0 ./build.sh
```

Note: some options activated by SECURITY_HARDENING require recent version for GCC (see pre-requisites) and for binutils (tested with 2.36.1).
Note 2: it is possible to use the `hardening-check` tool on binaries to check option activation worked.

## S2OPC Windows compilation

Note: Windows compilation is only possible if S2OPC_CLIENTSERVER_ONLY variable is set during the build (only Client/Server built).

Tested under Windows 10 (32 and 64 bits) and Windows Server 2016 (64 bits).
Prerequisites:
- Visual Studio (tested with Visual Studio 2017)
- CMake (tested with CMake version 3.16.2 and 3.22.2)
- Python3 (tested with Python version >= 3.6.3)
- [mbedtls](https://tls.mbed.org/) (tested with mbedtls version 2.16.12)
- [expat](https://libexpat.github.io/) (tested with expat version 2.4.3)
- [check](https://libcheck.github.io/check/) (tested with libcheck version 0.14 compiled with CMake)

To build S2OPC libraries and tests with default configuration on current stable release, you can use the build_s2opc.bat script by adapting few parameters through environment variables:
- Paths to external libraries source directory: MbedTLS, Expat (optional: needed for XML parsing features only), Check (optional: needed for S2OPC unit tests). Set the  MBEDTLS_DIR, EXPAT_DIR and CHECK_DIR variables.
- Visual Studio version to use and type of build (Release, Debug, etc.). Set the VS_VERSION and CONFIG variables.

By setting environment variables WITH_NANO_EXTENDED, BUILD_SHARED_LIBS, ENABLE_TESTING, ENABLE_SAMPLES and WITH_PYS2OPC it is possible to customize S2OPC build.
- WITH_NANO_EXTENDED (OFF by default): if set to ON, it includes extend features out of the server nano scope, it includes simplified subscription (no filter, no deletion, etc.) and method calls services (no address space access, etc.)
- BUILD_SHARED_LIBS (ON by default): if set to OFF, it builds static S2OPC libraries (necessary for ENABLE_TESTING=ON)
- ENABLE_TESTING (OFF by default): if set to ON, it builds the S2OPC unit tests and validation tests (BUILD_SHARED_LIBS=OFF necessary)
- ENABLE_SAMPLES (OFF by default): if set to ON, it builds the S2OPC demonstration samples (demo server, command line client tools, etc.)
- WITH_PYS2OPC (OFF by default): if set to ON, it builds the Python binding wheel for S2OPC and PYS2OPC_WHEEL_NAME variable shall also be set to define the wheel file name.

The generated project file S2OPC*.sln can then be imported in Visual Studio environment.

For more information, or to compile the master branch on its latest commit, please refer to the [wiki](https://gitlab.com/systerel/S2OPC/wikis/compilation) .

## Hints to build S2OPC on other platforms or without CMake


If CMake is not available or you want to use another tool for compilation you will have to manually configure the sources to compile.
Here are a few hints on how to manage to build without CMake:
- S2OPC libraries sources files (*.c and *.h) are all in src/ sub-directories
- Only one of the dependent platform directory shall be kept for the build : src/Common/helpers_platform_dep/<platform> (linux, windows, etc.)
- src/Common/configuration/sopc_common_build_info.h provides build-specific informations that must be filled in by build procedure:
  - When using CMake, the sopc_common_build_info.c is automatically generated with relevant content.
  - When using any other toolchain, the template provided in file sopc_common_build_info.c_ may be used.
    In that case the template should be copied into a valid .c file to be taken into account (depends on toolchain configuration).
- A s2opc_common_export.h file is expected to be found in the included directories.
  This file shall export several MACRO symbols including S2OPC_COMMON_EXPORT and S2OPC_COMMON_NO_EXPORT.
  - When using CMake, this file is automaatically generated depending on the host target.
  - When using any other toolchain, the template provided in file s2opc_common_export.h_
    for each src/Common/helpers_platform_dep/<platform> directory may be renamed to  s2opc_common_export.h.
    This however may require adaptation for specific toolchains.
    -  S2OPC_COMMON_EXPORT decorates library-exposed symbols (S2OPC_COMMON_NO_EXPORT does not expose)
- The size in bytes of a void pointer on the platform shall be defined as SOPC_PTR_SIZE (should be 4 or 8)

## S2OPC OPC UA Toolkit tests

Prerequisites (only for validation based on FreeOpcUa python client):
- Python 3
- Python cryptography
- FreeOpcUa (tested with version 0.98.13)

Run all tests:
- To run the S2OPC OPC UA Toolkit tests: execute the test-all.sh script: `./test-all.sh`
- Tests results are provided in build/bin/*.tap files and shall indicate "ok" status for each test

Run OPC UA Compliance Test Tool (UACTT: tool available for OPC foundation corporate members only):
- Run toolkit server in build/bin/ directory: ./toolkit_test_nano_server
- Run the UACTT tests using the UACTT project configuration file `tests/ClientServer/acceptance_tools/Acceptation_S2OPC/Acceptation_S2OPC.ctt.xml`

Note: ./toolkit_test_nano_server shall be killed when test is finished
Note 2: if compiled with WITH_NANO_EXTENDED set to 1, ./toolkit_test_nano_server binary name is changed to ./toolkit_test_server

## Licenses

Unless specifically indicated otherwise in a file, S2OPC files are
licensed under the Apache License Version 2.0, as can be found in `LICENSE`.
The mbedtls library is also distributed under the Apache 2.0 license.
OPC UA Stack code generated with the OPC foundation code generator
tool (UA-ModelCompiler) is distributed under the OPC Foundation MIT License 1.00.

## Commercial support

For S2OPC commercial support, see https://www.s2opc.com.

