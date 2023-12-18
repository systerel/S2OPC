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

function gen_nodeset() {
	OUTPUT="$1"
	shift
	../../../../scripts/nodeset_address_space_utils.py --output "$OUTPUT".tmp $@
	cat ../../../ClientServer/data/address_space/licence.xml > "$OUTPUT"
	sed '1d' "$OUTPUT".tmp >> "$OUTPUT"
	echo '' >> "$OUTPUT"
	rm "$OUTPUT".tmp
}

# Generate demo NodeSet for Micro profile
gen_nodeset s2opc_pubsub_nodeset.xml --remove-max-node-management ../../../ClientServer/data/address_space/s2opc_base_nodeset_origin.xml s2opc_pubsub_demo_data_origin.xml

# Generate demo NodeSet with limited base info data
gen_nodeset s2opc_pubsub_embedded_nodeset.xml --remove-subtree "i=92" "i=93" --remove-unused --remove-backward-refs --remove-methods ../../../ClientServer/data/address_space/s2opc_base_nodeset_origin.xml s2opc_pubsub_embedded_data_origin.xml
mv s2opc_pubsub_embedded_nodeset.xml ../../../embedded/cli_pubsub_server/xml/
