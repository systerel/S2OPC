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

# This script is used in order to prevent merge after usign rebase button in GITLAB.
# It will verify if commits pushed are correctly signed
set -euo pipefail

regex='([0-9a-f]+)[[:blank:]]([A-Z])'

export GNUPGHOME=.
git log --no-merges HEAD ^origin/master --pretty="format: %h %G?" | while read line || [[ -n $line ]];
do
    if [[ $line =~ $regex ]];then
        if [[ "${BASH_REMATCH[2]}" = "N" ]];then
            echo "NOK: no signature of commit: ${BASH_REMATCH[1]}"
            exit 1
        fi
    else
        echo "Unexpected expression found: impossible to check commit signature"
        exit 2
    fi
done
echo "OK: commits are signed"
