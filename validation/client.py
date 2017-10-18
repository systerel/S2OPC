#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2017 Systerel and others.
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
Simple client to launch validation tests
"""

from opcua import ua, Client
from opcua.ua import SecurityPolicy
from attribute_read import attribute_read_tests
from attribute_write_values import attribute_write_values_tests
from safety_secure_channels import secure_channels_connect
from view_basic import browse_tests
from common import sUri
from tap_logger import TapLogger
from opcua.crypto import security_policies
import re

if __name__=='__main__':
    print('Connecting to', sUri)
    client = Client(sUri)
    logger = TapLogger("validation.tap")

    for sp in [SecurityPolicy, security_policies.SecurityPolicyBasic256]:
        logger.begin_section("security policy {0}".format(re.split("#",sp.URI)[-1]))
        try:
            secure_channels_connect(client, sp)

            endPoints = client.get_endpoints()
            #print('endPoints:', endPoints)

            # Read tests
            attribute_read_tests(client, logger)

            # write tests
            attribute_write_values_tests(client, logger)

            # browse tests
            browse_tests(client, logger)

        finally:
            client.disconnect()
            print('Disconnected')

    logger.finalize_report()

