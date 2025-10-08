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
SRC_CFG_FILE=Acceptation_S2OPC.ctt.xml
NANO_CFG_FILE=Acceptation_S2OPC.nano.ctt.xml
SRC_SELECTION_FILE=Acceptation_S2OPC.selection.xml
NODE_MGT_SELECTION_FILE_PART=Acceptation_S2OPC.selection.node_mgt_group.xml.part
NODE_MGT_SELECTION_FILE=Acceptation_S2OPC.node_mgt.selection.xml
HISTORY_SELECTION_FILE_PART=Acceptation_S2OPC.selection.history.xml.part
AUDITING_SELECTION_FILE_PART=Acceptation_S2OPC.selection.auditing_group.xml.part

# remove methods from configuration
sed -r 's/\"ns=1;s=Method[^\"]+\"/\"\"/g' $SRC_CFG_FILE > $NANO_CFG_FILE

# add node management group selection to the dedicated selection file
head -n -2 $SRC_SELECTION_FILE > $NODE_MGT_SELECTION_FILE
cat $NODE_MGT_SELECTION_FILE_PART >> $NODE_MGT_SELECTION_FILE
cat $HISTORY_SELECTION_FILE_PART >> $NODE_MGT_SELECTION_FILE
cat $AUDITING_SELECTION_FILE_PART >> $NODE_MGT_SELECTION_FILE
tail -n -2 $SRC_SELECTION_FILE >> $NODE_MGT_SELECTION_FILE
