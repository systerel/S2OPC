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


# Checks license is present as expected in the header of the source code files
# of the project.

# Create temporary files
# Helper script holds the head | diff, so that its compatible with find | xargs
HELPER_SCRIPT=$(mktemp)
HEADER_C=$(mktemp)
HEADER_PY=$(mktemp)
HEADER_SH=$(mktemp)
HEADER_XSL=$(mktemp)

# Helper script takes the reference header and the file to test
echo '#!/bin/bash
if [[ -z $1 || -z "$2" ]] ; then
    echo Warning: empty argument, there may be no file found with this extension
    exit 0
fi
n_lines=$(wc -l < $1)
if [[ $n_lines -eq 0 ]] ; then
    echo Empty header file $1
    exit 3
fi
head -n $n_lines "$2" | diff - $1 > /dev/null
last=$?
if [[ ! $last -eq 0 ]] ; then
    echo Invalid copyright header in file $2
    exit 1
fi' > $HELPER_SCRIPT
chmod +x $HELPER_SCRIPT

# C copyright
echo '/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
' > $HEADER_C

# Py header
echo '#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
' > $HEADER_PY

# Script header
echo '#!/bin/bash

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
' > $HEADER_SH

# XSL header
echo '<?xml version="1.0" encoding="UTF-8"?>
<!--
Licensed to Systerel under one or more contributor license
agreements. See the NOTICE file distributed with this work
for additional information regarding copyright ownership.
Systerel licenses this file to you under the Apache
License, Version 2.0 (the "License"); you may not use this
file except in compliance with the License. You may obtain
a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
-->
' > $HEADER_XSL


# Do the finds
err=0
# Exclude some files
pushd csrc/opcua_types/ > /dev/null
for f in opcua_identifiers.h opcua_statuscodes.h sopc_types.h sopc_types.c; do
    mv $f $f"_"
done
popd > /dev/null
find csrc tests -name "*.[hc]" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_C || { echo 'Expected header:' ; cat $HEADER_C ; err=1 ; }
pushd csrc/opcua_types/ > /dev/null
for f in opcua_identifiers.h opcua_statuscodes.h sopc_types.h sopc_types.c; do
    mv $f"_" $f
done
popd > /dev/null

find bsrc -maxdepth 1 -name "*.[im]??" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_C || { echo 'Expected header:' ; cat $HEADER_C ; err=1 ; }
find bsrc -maxdepth 1 -name "*.pmm" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_C || { echo 'Expected header:' ; cat $HEADER_C ; err=1 ; }
find bsrc -maxdepth 1 -name "*.ref" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_C || { echo 'Expected header:' ; cat $HEADER_C ; err=1 ; }
find . -name "*.py" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_PY || { echo 'Expected header:' ; cat $HEADER_PY ; err=1 ; }
find . -name "*.sh" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_SH || { echo 'Expected header:' ; cat $HEADER_SH ; err=1 ; }
find . -type f -name "make*" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_SH || { echo 'Expected header:' ; cat $HEADER_SH ; err=1 ; }
find . -name "*.xsl" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_XSL || { echo 'Expected header:' ; cat $HEADER_XSL ; err=1 ; }

if [[ 0 -eq $err ]] ; then
    echo "All copyrights are OK"
fi
exit $err
