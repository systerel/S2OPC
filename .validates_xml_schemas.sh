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

SAMPLES=samples
TESTS=tests
XML_FILE_PATHS=(`find $SAMPLES $TESTS -name "*.xml"`)

EXCLUDED_XML_FILE_PATHS=("tests/ClientServer/data/config/S2OPC_Server_UACTT_Config.xml")

XSD_DIR="schemas"
XSD_FILES=("UANodeSet.xsd" "s2opc_clientserver_config.xsd" "s2opc_clientserver_users_config.xsd" "s2opc_pubsub_config.xsd" )

ERROR_COUNTER=0
for xml in ${XML_FILE_PATHS[@]}; do
    for excXML in ${EXCLUDED_XML_FILE_PATHS[@]}; do
        if [ "$xml" == "$excXML" ]; then
            # ignore XML file, continue to next one
            continue 2
        fi
    done
    # Check if it uses one XSD file
    for xsd in ${XSD_FILES[@]}; do
        if xmllint --xpath "/*" "$xml" | head -1 | grep -q "$xsd"; then
            xmllint --schema "$XSD_DIR/$xsd" --noout "$xml"
            if [ $? -ne 0 ]; then
                ERROR_COUNTER=$((ERROR_COUNTER + 1))
            fi
            # XML file checked, continue to next one
            continue 2
        fi
    done
done

exit $ERROR_COUNTER
