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

../../../../../scripts/gen-sopc-types.py ./opc.ua.fx.data.nodeset2.bsd --ns_index 3 --types_prefix FX_Data

echo "#ifndef OPCUA_FX_DATA_IDENTIFIERS_H" > opcua_FX_Data_identifiers.h
sed -r 's/([^,]+),([^,]+),[^,]*$/#define OpcUaId_FX_Data_\1 \2/g' opc.ua.fx.data.nodeset2.csv >> opcua_FX_Data_identifiers.h
echo "#endif" >> opcua_FX_Data_identifiers.h

../../../../../scripts/gen-sopc-types.py ./opc.ua.fx.ac.nodeset2.bsd --ns_index 4 --types_prefix FX_AC

echo "#ifndef OPCUA_FX_AC_IDENTIFIERS_H" > opcua_FX_AC_identifiers.h
sed -r 's/([^,]+),([^,]+),[^,]*$/#define OpcUaId_FX_AC_\1 \2/g' opc.ua.fx.ac.nodeset2.csv >> opcua_FX_AC_identifiers.h
echo "#endif" >> opcua_FX_AC_identifiers.h

../../../../../scripts/gen-sopc-types.py ./opc.ua.fx.cm.nodeset2.bsd --ns_index 4 --types_prefix FX_CM --imported_ns_prefixes FX_Data

echo "#ifndef OPCUA_FX_CM_IDENTIFIERS_H" > opcua_FX_CM_identifiers.h
sed -r 's/([^,]+),([^,]+),[^,]*$/#define OpcUaId_FX_CM_\1 \2/g' opc.ua.fx.cm.nodeset2.csv >> opcua_FX_CM_identifiers.h
echo "#endif" >> opcua_FX_CM_identifiers.h
