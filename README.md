## S2OPC OPC UA Toolkit

This OPC UA Toolkit project provides a C source code implementation
including an OPC UA communication stack, B model and C source code
implementation for a minimal set of services and a cryptographic
library adaptation for OPC UA needs (using mbedtls).

The S2OPC UA Toolkit is the result of the Research and Development project INGOPCS (2016-2018).
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

## S2OPC Toolkit features

### Common features:

- Asynchronous user application API
- Available security policies (encryption schemes) with any security mode:
    - http://opcfoundation.org/UA/SecurityPolicy#None,
    - http://opcfoundation.org/UA/SecurityPolicy#Basic256,
    - http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256.

Client side (e.g.: tests/services/toolkit_test_client.c):

- Secure Channel configuration
- Activate a session with an anonymous use or user identified by username/password
- Send a service on session request (read, write, browse, subscribe, etc.)
- Send a discovery service request (getEndpoints, findServer, registerServer, etc.)
- Automated client library LibSub (simplified interface, automated subscription):
  see tests/client_subscription/libs2opc_client.h (e.g. tests/client_subscription/client.c)
- Python wrapper PyS2OPC for a client: see tests/pys2opc/README.md

Server side (e.g.: tests/services/toolkit_test_server.c):

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
  - RegisterNodes service
  - Only if compiled with WITH_NANO_EXTENDED set to 1:
    - Subscription services (simplified: no monitoredItems filters or deletion, no subscription transfer)

### Cryptography services use constraints
- Only one authority certificate can be provided by channel (/endpoint)
  for server (/client) certificates validation,
- Authority certificate must be present in provided
  "TrustListLocation" path in DER format and must be named
  "cacert.der"

## Current status

- Security policies available: None, Basic256 and Basic256Sha256
- Security modes available: None, Sign and SignAndEncrypt
- Server instantiation: several endpoint descriptions, 1 address space, multiple secure channel instances and session instances
- Server discovery services: getEndpoints, findServers
- Server Nano profile services: read, write, browse, browseNext (1 continuation point per session), translateBrowsePath, registerNodes
- Server extended services (only if compiled with WITH_NANO_EXTENDED set to 1): subscription (no monitoredItems deletion, no subscription transfer)
- Server local services: read, write, browse and discovery services
- Server address space modification:
  - mechanisms implemented for remote modification: variables modification with typechecking (data type and value rank), access level and user access level control
  - notification: write notification events are reported through application API
- Client instantiation: multiple secure channel instances and session instances
- Client services requests: any discovery service or service on session request. Requests are only forwarded to server (no functional behavior)
- Address space with all mandatory attributes: AccessLevel, BrowseName, ContainsNoLoop, DataType, DisplayName, EvenNotifier, Executable, Historizing,
  IsAbstract, NodeClass, NodeId, Symmetric, UserAccessLevel, UserExecutable, Value (with single value and array Variants), ValueRank (and References)

## Address space generation

The `scripts/generate-s2opc-address-space.py` tool converts a UANodeSet XML file into a
C file that can be compiled in the binary, and used with the embedded address
space loader (see the `tests/data/address_space/parts/s2opc.xml`
file for example). Not all the features of the schema are supported at the
moment.

S2OPC server can also dynamically load a UANodeSet XML at startup.
To do so, set `TEST_SERVER_XML_ADDRESS_SPACE` to the location of the address space and launch the sample server.

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
    - validation tests using [FreeOPCUA](https://github.com/FreeOpcUa/python-opcua)
    - interoperability tests using [UACTT] (https://opcfoundation.org/developer-tools/certification-test-tools/opc-ua-compliance-test-tool-uactt/),
    - fuzzing tests.

## Demo

See the [demo](https://gitlab.com/systerel/S2OPC/wikis/demo) page from the Wiki.

## Getting started

For a sample server (respectively sample client), you can look at test/services/toolkit_test_server.c
 (respectively test/services/toolkit_test_client.c and tests/client_subscription/client.c).

## S2OPC Linux compilation

Tested under Ubuntu 16.04 and Debian 9.
Prerequisites:
- Make (tested with GNU Make version 4.2.1)
- CMake (tested with CMake version 3.9.4)
- GCC (tested with GCC version 8.1.0)
- [Mbedtls](https://tls.mbed.org/)(tested with mbedtls version 2.16.0)
- [Check](https://libcheck.github.io/check/)(tested with libcheck version 0.12)
- Python3 (tested with version 3.6.3)

To build the Toolkit library and tests with default configuration on current 
stable release:
```
  git clone https://gitlab.com/systerel/S2OPC.git --branch S2OPC_Toolkit_0.7.1
  cd S2OPC
  ./build.sh
```
For more information, or to compile the master branch on its latest commit, please refer to the [wiki](https://gitlab.com/systerel/S2OPC/wikis/compilation).

## S2OPC Windows compilation

Tested under Windows 7 and Windows Server 2016.
Prerequisites:
- Visual Studio (tested with Visual Studio 2017)
- CMake (tested with CMake version 3.11.1)
- Python3 (tested with Python version >= 3.6.3)
- [mbedtls](https://tls.mbed.org/) (tested with mbedtls version 2.12.0)
- [check](https://libcheck.github.io/check/) (tested with libcheck version 0.12)

To build the Toolkit library and tests with default configuration on current stable release, you can adapt the bat script below:
```
  REM Set env variables
  set CURRENT_DIR="%~dp0"
  set CHECK_DIR=[path to check sources directory]
  set CHECK_BUILD_DIR="%CHECK_DIR%\build"
  set MBEDTLS_DIR=[path to mbedtls source directory]
  set MBEDTLS_BUILD_DIR="%MBEDTLS_DIR%\build"

  REM Build Check
  cd %CHECK_DIR%
  rm -rf build
  mkdir build
  cd build
  cmake .. -G "Visual Studio 15 2017 Win64"
  cmake --build . --target ALL_BUILD --config RelWithDebInfo

  REM Build MbedTLS
  cd %MBEDTLS_DIR%
  rm -rf build
  mkdir build
  cd build
  cmake .. -G "Visual Studio 15 2017 Win64"
  cmake --build . --target ALL_BUILD --config RelWithDebInfo

  REM Configure S2OPC Project
  cd %CURRENT_DIR%
  rm -rf build
  mkdir build
  cd build

  cmake -DMBEDTLS_INCLUDE_DIR=%MBEDTLS_BUILD_DIR%/../include -DMBEDTLS_LIBRARY=%MBEDTLS_BUILD_DIR%/library/Debug/mbedtls.lib -DMBEDX509_LIBRARY=%MBEDTLS_BUILD_DIR%/library/Debug/mbedx509.lib -DMBEDCRYPTO_LIBRARY=%MBEDTLS_BUILD_DIR%/library/Debug/mbedcrypto.lib -DCHECK_INCLUDE_DIR=%CHECK_BUILD_DIR%\;%CHECK_BUILD_DIR%/src -DCHECK_LIBRARY=%CHECK_BUILD_DIR%/src/debug/check.lib\;%CHECK_BUILD_DIR%/lib/Debug/compat.lib .. -G "Visual Studio 15 2017 Win64"

  REM Build S2OPC Project
  cmake --build . --config RelWithDebInfo
```
The project file S2OPC.sln can be imported in Visual Studio environment.

For more information, or to compile the master branch on its latest commit, please refer to the [wiki](https://gitlab.com/systerel/S2OPC/wikis/compilation) .

## S2OPC OPC UA Toolkit tests

Prerequisites (only for validation based on FreeOpcUa python client):
- Python 3
- Python cryptography
- FreeOpcUa (tested with version 0.90.6)

Run all tests:
- To run the S2OPC OPC UA Toolkit tests: execute the test-all.sh script: `./test-all.sh`
- Tests results are provided in build/bin/*.tap files and shall indicate "ok" status for each test

Run a particular test (build/bin/ directory):
- Toolkit benchmark utility: execute ./bench_tool to see help
- Toolkit helpers unit tests: execute ./check_helpers
- Toolkit client library (LibSub) test: execute ./check_libsub
- Toolkit sockets management layer test: execute ./check_sockets
- Toolkit secure channel (+sockets) management layer: ./check_sc_rcv_buffer, ./check_sc_rcv_encrypted_buffer, ./test_secure_channels_server and ./test_secure_channels_client in parallel
- Toolkit client library based command line samples: to see help execute: ./s2opc_browse, ./s2opc_discovery, ./s2opc_findserver, ./s2opc_read, ./s2opc_register, ./s2opc_subscription_client, ./s2opc_write
- Toolkit address space parser test: execute ./s2opc_parse_uanodeset to see help
- Toolkit client/server session and read/write service example:
  execute ./toolkit_test_nano_server and then ./toolkit_test_client in parallel
  (or toolkit_test_client_service_faults for service fault specialized test)
- Toolkit server and read / write / browse service validation:
  execute ./toolkit_test_nano_server in build/bin/ directory and python3 client.py in validation/ directory
  (depends on FreeOpcUa python client available on github)
- Toolkit server and secure channel security token renewal validation:
  execute ./toolkit_test_nano_server in build/bin/ directory and python3
  client_sc_renew.py in validation/ directory (depends on FreeOpcUa
  python client available on github)
- Toolkit server local services validation: execute ./toolkit_test_nano_server_local_service

Run OPC UA Compliance Test Tool (UACTT: tool available for OPC foundation corporate members only):
- Run toolkit server in build/bin/ directory: ./toolkit_test_nano_server
- Run the UACTT tests using the UACTT project configuration file acceptances_tests/Acceptation_S2OPC/Acceptation_S2OPC.ctt.xml

Note: ./toolkit_test_nano_server shall be killed when test is finished
Note 2: if compiled with WITH_NANO_EXTENDED set to 1, ./toolkit_test_nano_server binary name is changed to ./toolkit_test_server

## Licenses

Unless specifically indicated otherwise in a file, S2OPC files are
licensed under the Apache License Version 2.0, as can be found in `LICENSE`.
The mbedtls library is also distributed under the Apache 2.0 license.
OPC UA Stack code generated with the OPC foundation code generator
tool (UA-ModelCompiler) is distributed under the OPC Foundation MIT License 1.00.

## Commercial support

Commercial support is available on demand for custom development and maintenance support.
Contact: s2opc-support@systerel.fr

