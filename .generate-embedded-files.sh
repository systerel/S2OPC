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

# Regenerates the embedded samples and tests C sources (static address spaces
# and PubSub configuration), then formats the generated C code.
# Shall be run after .generate-demo-nodesets.sh which updates its XML inputs.

set -euo pipefail

cd "$(dirname "$0")"

./scripts/generate-s2opc-address-space.py ./samples/embedded/cli_pubsub_server/xml/s2opc_pubsub_embedded_nodeset.xml ./samples/embedded/cli_pubsub_server/src/test_address_space.c --const_addspace
./scripts/generate-s2opc_pubsub-static-config.py ./tests/embedded/validation_test/config_pubsub.xml ./tests/embedded/validation_test/pubsub_config_static.c
./scripts/generate-s2opc-address-space.py ./tests/embedded/validation_test/xml/s2opc_pubsub_embedded_nodeset.xml ./tests/embedded/validation_test/embedded_address_space.c --const_addspace

./.format.sh

