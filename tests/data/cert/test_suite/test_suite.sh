#!/bin/bash

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


# -keyform der does not work...
openssl req -x509 -newkey rsa:1024 -nodes -keyout test_suite.key.pem -out test_suite.der -outform der -days 100
openssl pkey -in test_suite.key.pem -out test_suite.key -outform der
echo "Signed public key:"
hexdump -ve '/1 "%02x"' test_suite.der
echo -e "\nPrivate key"
hexdump -ve '/1 "%02x"' test_suite.key
