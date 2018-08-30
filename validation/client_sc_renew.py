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
Simple client to launch sc renew degraded tests
"""

from opcua import ua
from opcua.ua import SecurityPolicy
from safety_secure_channels import secure_channels_connect
from sc_renew import secure_channel_renew
from common import sUri, create_client
from tap_logger import TapLogger
from opcua.crypto import security_policies
import re
import sys

if __name__=='__main__':

    # tests with one connexion
    print('Connecting to', sUri)
    client = create_client()
    logger = TapLogger("sc_renew.tap")

    # tests of SC renew with degraded cases
    headerString = "******************* Beginning {0} test of degraded SC renew *********************"
    for sp in [SecurityPolicy, security_policies.SecurityPolicyBasic256]:
        logger.begin_section("security policy {0}".format(re.split("#",sp.URI)[-1]))
        # secure channel connection
        print(headerString.format(re.split("#",sp.URI)[-1]))
        secure_channels_connect(client, sp)

        secure_channel_renew(client, logger)

    logger.finalize_report()

    sys.exit(1 if logger.has_failed_tests else 0)
