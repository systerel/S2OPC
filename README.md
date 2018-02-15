# INGOPCS OPC UA Toolkit

This OPC UA Toolkit project provides a C source code implementation
including an OPC UA communication stack, B model and C source code
implementation for a minimal set of services and a cryptographic
library adaptation for OPC UA needs (using mbedtls).

This project contains the following elements:
- acceptances_tests: OPC UA Compliance Test Tool configuration and launch script
- address_space_generation: address space XML description to C source file generator to configure server Toolkit
- address_space_generation/genc: already generated C source files for the Toolkit tests
- apidoc: generated HTML documentation of the C source code with doxygen
- bin: built binaries tests of the toolkit for Linux platform (64 bits)
- bin_windows: built binaries tests and DLL of the toolkit for Windows platform (32 bits)
- bsrc: B model of the Toolkit used to generate C source files for services management layer
- csrc: root directory for C source files of the INGOPCS project OPC UA Toolkit
- csrc/api_toolkit: user application API to request server/client treatments once Toolkit configured
- csrc/configuration: user application API and shared configuration of the Toolkit server/client
- csrc/crypto: cryptographic API and generic layer for managing certificates and keys
- csrc/crypto/mbedtls: adaptation layer between crypotgraphic generic layer and mbedtls library
- csrc/helpers: independent helpers used by Toolkit
- csrc/helpers_platform_dep: independent helpers dependent on platform (Linux and Windows) used by Toolkit
- csrc/opcua_types: opcua types implementation, encoder and helpers
- csrc/secure_channels: secure channels services management layer based on I/O event dispatcher
- csrc/services: services (except secure channels) management layer based on I/O event dispatcher
- csrc/services/b2c: adaptation layer between generated C source files from B model and C source code
- csrc/services/bgenc: generated C source files from B model
- csrc/sockets: sockets management layer base on I/O event dispatcher
- doxygen: doxygen configuration file for HTML documentation generation
- install_linux: library (static and shared) and headers to install the toolkit as a library
- install_windows: shared library and headers to install the toolkit as a library
- tests: tests source code and data for the Toolkit library
- validation: client validation tool using FreeOpcUa python library


Compilation (Linux, tested under Ubuntu 14.04 and Debian 7):
- Prerequisites:
  * gcc (tested with GCC version >= 4.8.4)
  * CMake (tested with CMake version >= 2.8.12.2)
  * make (tested with GNU Make version >= 3.81)
  * mbedtls (>= 2.6.0): https://tls.mbed.org/
  * check (>= 0.10): https://libcheck.github.io/check/ (without sub-unit: use ./configure --enable-subunit=no)
- To build the Toolkit library and tests:
```
  ./build.sh
```
  OR see README.cmake to do it manually

Address space generation:
- see address_space_generation/README file for generator dependencies
- XML file provided complying with schema https://opcfoundation.org/UA/schemas/1.03/UANodeSet.xsd with limitations
  (see tests/data/address_space/parts/User_Address_Space.xml files for example)

Licenses:
- Unless specifically indicated otherwise in a file, INGOPCS files are
licensed under the GNU AFFERO GPL v3 license, as can be found in `agpl-3.0.txt`
- OPC UA Stack code generated with the OPC foundation code generator
  tool (UA-ModelCompiler) is distributed under the OPC Foundation MIT
  License 1.00
- The mbedtls library is also distributed under the Apache 2.0 license

Current status:
- Security policies available: None, Basic256 and Basic256Sha256
- Security mode available: None, Sign and SignAndEncrypt
- Server instantiation: several endpoint descriptions, 1 address space, multiple secure channel instances and session instances
- Server services: getEndpoints, read (no index), write (no index) and simplified browse (no continuation point)
- Client instantiation: multiple secure channel instances and session instances
- Client services requests: read, write and browse
- Address space with following attributes: NodeId, NodeClass, BrowseName, Value (with single value Variants),
  References, Access Level (R/W default value only)

# INGOPCS OPC UA Toolkit features

 Common features:
 * Asynchronous user application API
 * Available security policy (encryption schemes) with any security mode:
   + http://opcfoundation.org/UA/SecurityPolicy#None,
   + http://opcfoundation.org/UA/SecurityPolicy#Basic256,
   + http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256.

 Client side (e.g.: tests/services/toolkit_test_client.c):
 * Secure Channel configurations on Toolkit initialization
 * Activate a session with an anonymous user
 * Send a read request
 * Send a write request
 * Send a browse request

 Server side (e.g.: tests/services/toolkit_test_server.c):
* Endpoint descriptions configuration on Toolkit initialization
* 1 address space configuration on Toolkit initialization
* Checks and accepts several instances of secure channel
* Checks and accepts activation of several sessions with an anonymous user
* Supported services:
  + Read service
  + Write service
  + Browse service (simplified: no continuation point)
  + GetEndpoints service (restriction: locale Ids and profile URIs ignored)

# INGOPCS OPC UA Toolkit tests

Prerequisites (only for validation based on FreeOpcUa python client):
- Python 3
- Python cryptography
- FreeOpcUa (tested with version 0.90.6)

Run all tests:
- To run the INGOPCS OPC UA Toolkit tests: execute the test-all.sh script: `./test-all.sh`
- Tests results are provided in bin/*.tap files and shall indicate "ok" status for each test

Run a particular test (bin/ directory):
- Toolkit helpers unit tests: execute ./check_helpers
- Toolkit sockets management layer test: execute ./check_sockets
- Toolkit secure channel (+sockets) management layer: ./test_secure_channels_server and ./test_secure_channels_client in parallel
- Toolkit read service test: execute ./toolkit_test_read
- Toolkit write service test: execute ./toolkit_test_write
- Toolkit client/server session and read/write service example:
  execute ./toolkit_test_server and then ./toolkit_test_client in parallel
- Toolkit server and read / write / browse service validation:
  execute ./toolkit_test_server in bin/ directory and python3 client.py in validation/ directory
  (depends on FreeOpcUa python client available on github)
- Toolkit server and secure channel security token renewal validation:
  execute ./toolkit_test_server in bin/ directory and python3
  client_sc_renew.py in validation/ directory (depends on FreeOpcUa
  python client available on github)

Run OPC UA Compliance Test Tool (UACTT: tool available for OPC foundation corporate members only):
- Run toolkit server example with long timeout parameter in bin/ directory: ./toolkit_test_server 100000
- Run the UACTT tests using the UACTT project configuration file acceptances_tests/Acceptation_INGOPCS/Acceptation_INGOPCS.ctt.xml
