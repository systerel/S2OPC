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
#!/usr/bin/python3.4
#-*-coding:Utf-8 -*

# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from opcua import ua
from opcua.crypto import security_policies

def secure_channels_connect(client, security_policy):

    if (security_policy == security_policies.SecurityPolicyBasic256):
        client.set_security(security_policy,
        '../bin/client_public/client.der',
        '../bin/client_private/client.pem',
        server_certificate_path='../bin/server_public/server.der',
        mode=ua.MessageSecurityMode.Sign)

    client.connect()
    print('Connected')

