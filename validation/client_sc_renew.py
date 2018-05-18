#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2018 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""
Simple client to launch sc renew degraded tests
"""

from opcua import ua, Client
from opcua.ua import SecurityPolicy
from safety_secure_channels import secure_channels_connect
from sc_renew import secure_channel_renew
from common import sUri
from tap_logger import TapLogger
from opcua.crypto import security_policies
import re
import sys

if __name__=='__main__':

    # tests with one connexion
    print('Connecting to', sUri)
    client = Client(sUri)
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
