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

import os
import getpass

from pys2opc import PyS2OPC_Client

TEST_PASSWORD_PRIV_KEY_ENV_NAME = "TEST_PASSWORD_PRIVATE_KEY"
TEST_PASSWORD_USER_ENV_NAME = "TEST_PASSWORD_PRIVATE_KEY"

class ReconnectingContext:
    """
    ConnectionHandler which reconnects when there was an error.
    Does not try to reconnect if the connect() fails.
    Handles connection.disconnect() when freed.
    """
    def __init__(self, confId, connCls):
        self.confId = confId
        self.connCls = connCls
        self.connection = None

    def __del__(self):
        # Disconnects cleanly if there is still a pending connection
        if self.connection is not None and self.connection.connected:
            self.connection.disconnect()

    def get_connection(self):
        """
        Either returns an existing connection, or creates a new one if the previous one crashed.
        """
        if self.connection is None or not self.connection.connected:
            print('+', end='', flush=True)
            self.connection = PyS2OPC_Client.connect(self.confId, self.connCls)
        return self.connection

class PyS2OPC_Client_Test():
    @staticmethod
    def get_client_key_password():
        pwd = os.getenv(TEST_PASSWORD_PRIV_KEY_ENV_NAME)
        if pwd is None:
            pwd = getpass.getpass(prompt='Client private key password:')
        return pwd

    @staticmethod
    def get_user_password(username):
        pwd = os.getenv(TEST_PASSWORD_USER_ENV_NAME)
        if pwd is None:
            pwd = getpass.getpass(prompt='({}) username password:'.format(username))
        return pwd

# overload the default method to get key password
PyS2OPC_Client.get_client_key_password = PyS2OPC_Client_Test.get_client_key_password
