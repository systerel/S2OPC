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

set -euox pipefail

# Generate demo NodeSet for Micro profile + node management
../../../../scripts/nodeset-address-space-utils.py --output s2opc_node_mgt.xml.tmp s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml s2opc_demo_data_perfs.xml
cat licence.xml > s2opc_node_mgt.xml
sed '1d' s2opc_node_mgt.xml.tmp >> s2opc_node_mgt.xml
echo '' >> s2opc_node_mgt.xml
rm s2opc_node_mgt.xml.tmp

# Generate demo NodeSet for Micro profile
../../../../scripts/nodeset-address-space-utils.py --output s2opc.xml.tmp --remove-max-node-management s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml s2opc_demo_data_perfs.xml
cat licence.xml > s2opc.xml
sed '1d' s2opc.xml.tmp >> s2opc.xml
echo '' >> s2opc.xml
rm s2opc.xml.tmp

# Generate PubSub demo NodeSet for Micro profile
../../../../scripts/nodeset-address-space-utils.py --output s2opc_pubsub_server.xml.tmp --remove-max-node-management s2opc_base_nodeset_origin.xml ../../../PubSub_ClientServer/data/address_space/s2opc_demo_pubsub.xml
cat licence.xml > s2opc_pubsub_server.xml
sed '1d' s2opc_pubsub_server.xml.tmp >> s2opc_pubsub_server.xml
echo '' >> s2opc_pubsub_server.xml
mv s2opc_pubsub_server.xml ../../../PubSub_ClientServer/data/address_space/
rm s2opc_pubsub_server.xml.tmp

# Generate demo NodeSet for Micro profile + SKS
../../../../scripts/nodeset-address-space-utils.py --output s2opc_sks.xml.tmp --remove-max-node-management s2opc_base_nodeset_origin.xml s2opc_base_sks_origin.xml s2opc_demo_data_origin.xml s2opc_demo_data_perfs.xml
cat licence.xml > s2opc_sks.xml
sed '1d' s2opc_sks.xml.tmp >> s2opc_sks.xml
echo '' >> s2opc_sks.xml
rm s2opc_sks.xml.tmp

# Generate demo NodeSet for Nano profile
../../../../scripts/nodeset-address-space-utils.py --output s2opc_nano.xml.tmp --remove-max-monitored-items --remove-methods --remove-max-node-management s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml s2opc_demo_data_perfs.xml
cat licence.xml > s2opc_nano.xml
sed '1d' s2opc_nano.xml.tmp >> s2opc_nano.xml
echo '' >> s2opc_nano.xml
rm s2opc_nano.xml.tmp

# Generate PubSub demo NodeSet for Nano profile
../../../../scripts/nodeset-address-space-utils.py --output s2opc_pubsub_nano_server.xml.tmp --remove-max-node-management --remove-methods --remove-max-node-management s2opc_base_nodeset_origin.xml ../../../PubSub_ClientServer/data/address_space/s2opc_demo_pubsub.xml
cat licence.xml > s2opc_pubsub_nano_server.xml
sed '1d' s2opc_pubsub_nano_server.xml.tmp >> s2opc_pubsub_nano_server.xml
echo '' >> s2opc_pubsub_nano_server.xml
mv s2opc_pubsub_nano_server.xml ../../../PubSub_ClientServer/data/address_space/
rm s2opc_pubsub_nano_server.xml.tmp

# Generate demo NodeSet with limited base info data
../../../../scripts/nodeset-address-space-utils.py --output s2opc_no_base_info.xml.tmp --remove-node-ids-greater-than 3000 --remove-methods s2opc_base_nodeset_origin.xml s2opc_demo_data_origin.xml
cat licence.xml > s2opc_no_base_info.xml
sed '1d' s2opc_no_base_info.xml.tmp >> s2opc_no_base_info.xml
echo '' >> s2opc_no_base_info.xml
rm s2opc_no_base_info.xml.tmp
