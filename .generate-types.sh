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

# Regenerates the OPC UA types, status codes helpers and test types sources
# from their schemas, then formats the generated C code.

set -euo pipefail

cd "$(dirname "$0")"

# NS0 types
./scripts/gen-sopc-types.py ./schemas/Opc.Ua.Types.bsd
./scripts/generate-embedded-base-type-info.py ./schemas/Opc.Ua.NodeSet2.xml src/Common/opcua_types/sopc_embedded_nodeset2.h
# Status codes helpers
./scripts/generate_statuscodes_helpers.py
# Test types
./scripts/gen-sopc-types.py --types_prefix Custom --ns_index 1 ./tests/ClientServer/data/types/customTypes.bsd --files_path ./tests/ClientServer/unit_tests/helpers/
./scripts/gen-sopc-types.py --types_prefix Custom2 --ns_index 2 ./tests/ClientServer/data/types/level2Types.bsd --files_path ./tests/ClientServer/unit_tests/helpers/ --imported_ns_prefixes Custom

./.format.sh

