## S2OPC OPC UA Toolkit

This OPC UA Toolkit project provides a C source code implementation
including an OPC UA communication stack, B model and C source code
implementation for a minimal set of services and a cryptographic
library adaptation for OPC UA needs (using mbedtls).

## S2OPC Toolkit features

Common features:

- Asynchronous user application API
- Available security policies (encryption schemes) with any security mode:
    - http://opcfoundation.org/UA/SecurityPolicy#None,
    - http://opcfoundation.org/UA/SecurityPolicy#Basic256,
    - http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256.

Client side (e.g.: tests/services/toolkit_test_client.c):

- Secure Channel configuration on Toolkit initialization
- Activate a session with an anonymous user
- Send a service on session request (read, write, browse, etc.)
- Send a discovery service request (getEndpoints, etc.)

Server side (e.g.: tests/services/toolkit_test_server.c):

- Endpoint descriptions configuration on Toolkit initialization
- 1 address space configuration on Toolkit initialization
- Checks and accepts several instances of secure channels
- Checks and accepts activation of several sessions with an anonymous user
- Supported services:
  - Read service
  - Write service
  - Browse service (simplified: no continuation point)
  - GetEndpoints service (restriction: locale Ids and profile URIs ignored)

## Current status

- Security policies available: None, Basic256 and Basic256Sha256
- Security modes available: None, Sign and SignAndEncrypt
- Server instantiation: several endpoint descriptions, 1 address space, multiple secure channel instances and session instances
- Server services: getEndpoints, read (no index), write (no index) and simplified browse (no filtering, no continuation point)
- Server local services: server services are locally accessible through application API
- Server address space modification notification: write notification events are reported through application API
- Client instantiation: multiple secure channel instances and session instances
- Client services requests: any discovery service or service on session request. Requests are only forwarded to server (no functional behavior)
- Address space with following attributes: NodeId, NodeClass, BrowseName, Value (with single value Variants),
  References, Access Level (R/W default value only)

## Address space generation

The `generate-ingopcs-address-space` tool converts a UANodeSet XML file into a
C file that can be compiled in the binary, and used with the embedded address
space loader (see the `tests/data/address_space/parts/User_Address_Space.xml`
file for example). Not all the features of the schema are supported at the
moment.

S2OPC server can also dynamically load a UANodeSet XML at startup.
To do so, set `TEST_SERVER_XML_ADDRESS_SPACE` to the location of the address space and launch the server.

## S2OPC Development

- systematic peer review using GitLab Merge Request mechanism,
- formal modeling of OPC-UA services using [B method](https://en.wikipedia.org/wiki/B-Method),
- static analysis using [Frama C](http://frama-c.com/) from CEA and [TIS analyser](https://taas.trust-in-soft.com/) from Trust-In-Soft,
- static analysis using [Coverity](https://scan.coverity.com/),
- check for memory leaks using [Valgrind](http://valgrind.org/),
- use of GCC sanitizers to detect undefined behaviours, race conditions, memory leaks and errors,
- compilation using four compilers (GCC, CLang, mingGw and MSVC) with conservative compilation flags,
- all development and testing environment are bundled into [Docker](https://www.docker.com/) images
- continuous integration with a test bench containing:
    - modular tests using libcheck
    - validation tests using [FreeOPCUA](https://github.com/FreeOpcUa/python-opcua)
    - interoperability tests using [UACTT] (https://opcfoundation.org/developer-tools/certification-test-tools/opc-ua-compliance-test-tool-uactt/),
    - fuzzing tests.

## Demo

See the [demo](https://gitlab.com/systerel/S2OPC/wikis/demo) page from the Wiki.

## Getting started

For a sample server (respectively sample client), you can look at test/services/toolkit_test_server.c (respectively test/services/toolkit_test_client.c).

## S2OPC Linux compilation

Tested under Ubuntu 14.04 and Debian 7.
Prerequisites:
- gcc (tested with GCC version 8.1)
- CMake (tested with CMake version >= 3.5)
- make (tested with GNU Make version >= 4.2)
- Python3 (tested with Python version >= 3.6.3)
- [mbedtls](https://tls.mbed.org/)(>= 2.9.0)
- [check](https://libcheck.github.io/check/)(>= 0.12)

To build the Toolkit library and tests with default configuration on current stable release:
```
  git checkout INGOPCS_Toolkit_0.5.0
  ./build.sh
```
For more information, or to compile the master branch on its latest commit, please refer to the [wiki](https://gitlab.com/systerel/S2OPC/wikis/compilation).

## S2OPC Windows compilation

Tested under Windows 7 and Windows Server 2016.
Prerequisites:
- Visual Studio (tested with Visual Studio 2017)
- CMake (tested with CMake version 3.11.1)
- Python3 (tested with Python version >= 3.6.3)
- [mbedtls](https://tls.mbed.org/) (>= 2.9.0)
- [check](https://libcheck.github.io/check/) (>= 0.12)

To build the Toolkit library and tests with default configuration on current stable release, you can adapt the bat script below:
```
  git checkout INGOPCS_Toolkit_0.5.0

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
  cmake ..
  cmake --build . --target ALL_BUILD

  REM Build MbedTLS
  cd %MBEDTLS_DIR%
  rm -rf build
  mkdir build
  cd build
  cmake ..
  cmake --build . --target ALL_BUILD

  REM Configure S2OPC Project
  cd %CURRENT_DIR%
  rm -rf build
  mkdir build
  cd build

  cmake -DMBEDTLS_INCLUDE_DIR=%MBEDTLS_BUILD_DIR%/../include -DMBEDTLS_LIBRARY=%MBEDTLS_BUILD_DIR%/library/Debug/mbedtls.lib -DMBEDX509_LIBRARY=%MBEDTLS_BUILD_DIR%/library/Debug/mbedx509.lib -DMBEDCRYPTO_LIBRARY=%MBEDTLS_BUILD_DIR%/library/Debug/mbedcrypto.lib -DCHECK_INCLUDE_DIR=%CHECK_BUILD_DIR%\;%CHECK_BUILD_DIR%/src -DCHECK_LIBRARY=%CHECK_BUILD_DIR%/src/debug/check.lib\;%CHECK_BUILD_DIR%/lib/Debug/compat.lib ..

  REM Build S2OPC Project
  cmake --build .
```
The project file INGOPCS.sln can be imported in Visual Studio environment.

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
- Toolkit helpers unit tests: execute ./check_helpers
- Toolkit sockets management layer test: execute ./check_sockets
- Toolkit secure channel (+sockets) management layer: ./test_secure_channels_server and ./test_secure_channels_client in parallel
- Toolkit client/server session and read/write service example:
  execute ./toolkit_test_server and then ./toolkit_test_client in parallel
- Toolkit server and read / write / browse service validation:
  execute ./toolkit_test_server in build/bin/ directory and python3 client.py in validation/ directory
  (depends on FreeOpcUa python client available on github)
- Toolkit server and secure channel security token renewal validation:
  execute ./toolkit_test_server in build/bin/ directory and python3
  client_sc_renew.py in validation/ directory (depends on FreeOpcUa
  python client available on github)

Run OPC UA Compliance Test Tool (UACTT: tool available for OPC foundation corporate members only):
- Run toolkit server example with long timeout parameter in build/bin/ directory: ./toolkit_test_server
- Run the UACTT tests using the UACTT project configuration file acceptances_tests/Acceptation_INGOPCS/Acceptation_INGOPCS.ctt.xml

## Licenses

Unless specifically indicated otherwise in a file, S2OPC files are
licensed under the Apache License Version 2.0, as can be found in `LICENSE`.
The mbedtls library is also distributed under the Apache 2.0 license.
OPC UA Stack code generated with the OPC foundation code generator
tool (UA-ModelCompiler) is distributed under the OPC Foundation MIT License 1.00.

## Commercial support

Commercial support is available on demand for custom development and maintenance support.
Contact: contact@systerel.fr

