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

# Regenerates the demo NodeSet XML files from their origin files.
# The PubSub generation also updates the embedded NodeSet XML files used as
# input by .generate-embedded-files.sh.

set -euo pipefail

cd "$(dirname "$0")"

(cd samples/ClientServer/data/address_space && ./generate_demo_nodesets.sh)
(cd samples/PubSub_ClientServer/data/address_space && ./generate_demo_nodesets.sh)

