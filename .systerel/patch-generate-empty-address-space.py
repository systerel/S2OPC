#!/usr/bin/env python3
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

import re, sys

src = open(sys.argv[1], encoding='utf8').read()

# Replace the main() execution block
old = re.search(
    r"    print\('Generating C address space.*?    print\('Done\.'\)",
    src, re.DOTALL
).group(0)

new = """\
    with open(args.c_file, 'w', encoding='utf8') as out_fd:
        out_fd.write(c_header)
        out_fd.write("const bool sopc_embedded_is_const_addspace = false;\\n\\n")
        out_fd.write("SOPC_AddressSpace_Node SOPC_Embedded_AddressSpace_Nodes[1] = {{0}};\\n")
        out_fd.write("const uint32_t SOPC_Embedded_AddressSpace_nNodes = 0;\\n\\n")
        out_fd.write("// Unused variable but it is still necessary to link the loader of embedded address space\\n")
        out_fd.write("SOPC_Variant* SOPC_Embedded_VariableVariant = NULL;\\n")
        out_fd.write("const uint32_t SOPC_Embedded_VariableVariant_nb = 0;\\n")
    print('Done.')"""

open(sys.argv[1], 'w', encoding='utf8').write(src.replace(old, new))
