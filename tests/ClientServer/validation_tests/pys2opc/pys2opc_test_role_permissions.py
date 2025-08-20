#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

"""
Role permissions tests:
- DefaultRolePermissions(DRP) tests :
    - DRP without RP
    - DRP with RP. RP overrides DRP (addition/restiction)
    - No DRP => RP ignored
- RolePermissions(RP) tests :
    - Read
    - Write
    - Call
    - TODO : AddNodes

Test sequence (with expected results) :
Note : obs = Observer, op = Operator.

# DRP Tests #
Int64_DRP_no_RP : 1. Read with DRP (obs) -> OK
                  2. Write with DRP (obs) -> NOK

Int64_DRP_RP : 3. Read with RP (obs) -> NOK (RP overrides DRP and RP write for obs)
               4. Write with RP (obs) -> OK

Int64_DRP_RP_2 : 5. Read with RP (obs) -> NOK (RP overrides DRP and no RP for obs)

Int64_RP_no_DRP (NS=2) : 6. Write with RP read only (obs) and no DRP -> OK (No DRP => RP ignored)

# RP Tests #
Int64_RP : 7. Read with RP (obs) -> NOK
           8. Read with RP (Op) -> OK
           9. Write with RP (Op) -> NOK
           TODO : Write with RP (ConfigureAdmin) -> OK

MethodNoArg : 10. Call with DRP (obs) -> NOK
              11. Call with RP (Op) -> OK
"""

import os
import getpass
import sys

from pys2opc import PyS2OPC_Client, SOPC_Failure, StatusCode, DataValue, VariantType
from tap_logger import TapLogger

TEST_PASSWORD_PRIV_KEY_ENV_NAME = "TEST_PASSWORD_PRIVATE_KEY"
TEST_USERNAME_ENV_NAME = "TEST_USERNAME"
TEST_PASSWORD_USER_ENV_NAME = "TEST_PASSWORD_USER"
# CONFIG_ADMIN = False

class PyS2OPC_Client_Test():
    @staticmethod
    def get_client_key_password() -> str:
        pwd = os.getenv(TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        if pwd is None:
            print("{} not set: set it or enter it interactively".format(TEST_PASSWORD_PRIV_KEY_ENV_NAME))
            pwd = getpass.getpass(prompt='Client private key password:')
        return pwd

    @staticmethod
    def get_username_password(connConfigUserId: str) -> tuple[str, str]:
        # if CONFIG_ADMIN:
        #     return "me", "1234"
        username = os.getenv(TEST_USERNAME_ENV_NAME)
        if username is None:
            print("{} not set: set it or enter it interactively".format(TEST_USERNAME_ENV_NAME))
            username = getpass.getpass(prompt="UserName of user (e.g.: 'user1'): ")
        pwd = os.getenv(TEST_PASSWORD_USER_ENV_NAME)
        if pwd is None:
            print("{} not set: set it or enter it interactively".format(TEST_PASSWORD_USER_ENV_NAME))
            pwd = getpass.getpass(prompt='Password for user: ')
        return username, pwd

# overload the default method to get key password and username with environment variable
PyS2OPC_Client.get_client_key_password = PyS2OPC_Client_Test.get_client_key_password
PyS2OPC_Client.get_username_password = PyS2OPC_Client_Test.get_username_password

# global variable corresponding to DataValues to write into NODES #
DV_Int64 = DataValue.from_python(12, False)
DV_Int64.variantType= VariantType.Int64

# Error code attends when role permissions isn't give to a service #
BadUserAccessDenied: int = 0x801F0000

if __name__ == '__main__':

    print("Start Tests Role Permissions")
    # --- Test Role Permissions --- #
    logger = TapLogger('validation_pys2opc_role_permissions.tap')

    with PyS2OPC_Client.initialize():
        try:
            # Connect clients #
            configs = PyS2OPC_Client.load_client_configuration_from_file(os.path.join('S2OPC_Client_Wrapper_Config.xml'))
            connect_obs = PyS2OPC_Client.connect(configs["read"]) # Obs : anonymous
            connect_op = PyS2OPC_Client.connect(configs["write"]) # Op : username : user1, pwd : password
            # Clients connected #

            # - 1 : Tests DRP - #
            logger.begin_section('DRP tests -')

            # DRP no RP
            readResponse = connect_obs.read_nodes(['ns=1;s=Int64_DRP_no_RP'])
            logger.add_test('Observer reads successfully (DRP:observer = Read). Result = 0x{:08X}.'
                            .format(readResponse.results[0].statusCode), StatusCode.isGoodStatus(readResponse.results[0].statusCode))
            writeResponse = connect_obs.write_nodes(['ns=1;s=Int64_DRP_no_RP'], [DV_Int64])
            logger.add_test('Observer fails to write (DRP:observer = Read). Result = 0x{:08X}. Expected = 0x{:08X}.'
                            .format(writeResponse.results[0], BadUserAccessDenied), writeResponse.results[0] == BadUserAccessDenied)

            # DRP RP
            readResponse = connect_obs.read_nodes(['ns=1;s=Int64_DRP_RP'])
            logger.add_test('Observer fails to read (RP:observer = Write overrides DRP:observer = Read). Result = 0x{:08X}. Expected = 0x{:08X}.'
                            .format(readResponse.results[0].statusCode, BadUserAccessDenied),readResponse.results[0].statusCode == BadUserAccessDenied)
            writeResponse = connect_obs.write_nodes(['ns=1;s=Int64_DRP_RP'], [DV_Int64])
            logger.add_test('Observer writes successfully (RP:observer = Write overrides DRP:observer = Read). Result = 0x{:08X}.'
                            .format(writeResponse.results[0]), writeResponse.is_ok())

            # DRP RP 2
            readResponse = connect_obs.read_nodes(['ns=1;s=Int64_DRP_RP_2'])
            logger.add_test('Observer fails to read (RP:observer = None overrides DRP:observer = Read). Result = 0x{:08X}. Expected = 0x{:08X}.'
                            .format(readResponse.results[0].statusCode, BadUserAccessDenied), readResponse.results[0].statusCode == BadUserAccessDenied)

            # no DRP (NS=2)
            writeResponse = connect_obs.write_nodes(['ns=2;s=Int64_RP_no_DRP'], [DV_Int64])
            logger.add_test('Observer write successfully (whereas RP:observer = Read) because no DPR => No RolePersmissions restriction. Result = 0x{:08X}.'
                            .format(writeResponse.results[0]), writeResponse.is_ok())

            # - 2 : Tests RP - #
            logger.begin_section('RP tests -')

            # RP Read/Write
            readResponse = connect_obs.read_nodes(['ns=1;s=Int64_RP'])
            logger.add_test('Observer fails to read (RP:observer = None). Result = 0x{:08X}. Expected = 0x{:08X}.'
                            .format(readResponse.results[0].statusCode, BadUserAccessDenied), readResponse.results[0].statusCode == BadUserAccessDenied)
            readResponse = connect_op.read_nodes(['ns=1;s=Int64_RP'])
            logger.add_test('Operator reads successfully (RP:operator = Read). Result = 0x{:08X}.'
                            .format(readResponse.results[0].statusCode), StatusCode.isGoodStatus(readResponse.results[0].statusCode))
            writeResponse = connect_op.write_nodes(['ns=1;s=Int64_RP'], [DV_Int64])
            logger.add_test('Operator fails to write (RP:operator = Read). Result = 0x{:08X}. Expected = 0x{:08X}.'
                            .format(writeResponse.results[0], BadUserAccessDenied), writeResponse.results[0] == BadUserAccessDenied)
            # # TODO : Add this test when changing users on the same connection is possible
            # # As we're using the same “write” connection configuration for Operator and confAdmin,
            # # we need to switch between the two.
            # connect_op.disconnect()
            # CONFIG_ADMIN = True
            # connect_confAdmin = PyS2OPC_Client.connect(configs["write"]) # confAdmin : username : me, pwd : 1234
            # writeResponse = connect_confAdmin.write_nodes(['ns=1;s=Int64_RP'], [DV_Int64])
            # logger.add_test('Observer writes successfully (RP:ConfigureAdmin = Read). Result = 0x{:08X}.'
            #                 .format(writeResponse.results[0]), writeResponse.is_ok())

            # RP Call
            callMethodResponse = connect_obs.call_method("ns=1;s=TestObject", "ns=1;s=MethodNoArg")
            logger.add_test('Observer fails to call (RP:observer = Read). Result = 0x{:08X}. Expected = 0x{:08X}.'
                            .format(callMethodResponse.callResult, BadUserAccessDenied), callMethodResponse.callResult == BadUserAccessDenied)
            callMethodResponse = connect_op.call_method("ns=1;s=TestObject", "ns=1;s=MethodNoArg")
            logger.add_test('Operator calls successfully (RP:observer = Read + Write + Call). Result = 0x{:08X}.'
                            .format(callMethodResponse.callResult), callMethodResponse.is_ok())
        except SOPC_Failure as f:
            print(f, flush=True)
            sys.exit(1)

    logger.finalize_report()
    sys.exit(1 if logger.has_failed_tests else 0)
