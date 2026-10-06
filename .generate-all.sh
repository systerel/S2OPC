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

# Regenerates all the generated files checked by the CI test-check job,
# in dependency order. Use ./.verify-no-git-changes.sh afterwards to check
# that the repository files were up to date.

set -euo pipefail

cd "$(dirname "$0")"

./.generate-types.sh
./.generate-demo-nodesets.sh
./.generate-embedded-files.sh

