#!/bin/bash

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

set -euo pipefail

# Generate demo NodeSet for Nano extended profile
../../../../scripts/nodeset-address-space-utils.py --output s2opc.xml.tmp s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml
cat licence.xml > s2opc.xml
sed '1d' s2opc.xml.tmp >> s2opc.xml
echo '' >> s2opc.xml
rm s2opc.xml.tmp

# Generate demo NodeSet for Nano extended profile
../../../../scripts/nodeset-address-space-utils.py --output s2opc_nano.xml.tmp --remove-max-monitored-items --remove-methods --remove-max-node-management s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml
cat licence.xml > s2opc_nano.xml
sed '1d' s2opc_nano.xml.tmp >> s2opc_nano.xml
echo '' >> s2opc_nano.xml
rm s2opc_nano.xml.tmp
