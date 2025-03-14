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

import re
import sys
from autopxd import cli
if __name__ == '__main__':
    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])
    exit_cli_code = cli(standalone_mode=False)
    if exit_cli_code is None:
        # Insert custom lines
        filename = sys.argv[-1]
        print("Add custom lines at the beginning of the file : ", filename)

        fileR = open(filename,"r")
        text = fileR.read()
        # Change declaration to be able to execute C code without the GIL 
        # e.g. SOPC_ServerHelper_Serve should not take it in order C callbacks are able to take it in parallel
        text_with_nogil = re.sub(r'cdef extern from "(\w+\.h)":', r'cdef extern from "\1" nogil:', text)
        fileR.close()

        # Insert the cython language level at the beginning of the PXD file 
        # Add the declarations not generated by autopxd (see the `s2opc_non_auto.pxd` file).
        textInsert = "# cython: language_level=3str\nfrom s2opc_non_auto cimport *\n"

        fileW = open(filename, "w")
        fileW.write(textInsert + text_with_nogil)

    sys.exit(exit_cli_code)
