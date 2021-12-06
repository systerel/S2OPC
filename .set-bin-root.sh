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

set -e

LIST_BIN="sub_scheduler_conf_test pub_scheduler_test"
BIN_DIR="build/bin"

for bin in $LIST_BIN; do
    chown root "$BIN_DIR/$bin" && chmod u+s "$BIN_DIR/$bin" || echo "Warning: failed to set root rights to binary $BIN_DIR/$bin"
done
