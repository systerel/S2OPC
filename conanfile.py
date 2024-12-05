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

from conan import ConanFile

class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("binutils/2.36.1@Systerel+S2OPC/default")
        self.requires("expat/2.6.4@Systerel+S2OPC/default")
        self.requires("gcc/13.3.0@Systerel+S2OPC/default")
        self.requires("make/4.3")
        self.requires("mbedtls/3.6.2")
        self.requires("paho-mqtt-c/1.3.4")
        self.requires("libcheck/0.14.0@Systerel+S2OPC/default")
        self.requires("doxygen/1.12.0@Systerel+S2OPC/default")
        self.requires("gmp/6.2.1",override=True) #Sub-depency of mpfr, gcc and isl
        self.requires("mpfr/4.1.0",override=True) #Sub-depency of gcc
        self.requires("zlib/1.2.13",override=True) #Sub-depency of gcc, binutils, doxygen, mbedtls and openssl
        self.requires("isl/0.24",override=True) #Sub-depency of gcc
        self.requires("openssl/3.2.0",override=True) #Sub-depency of paho-mqtt-c and cmake
        self.requires("cmake/3.23.5@Systerel+S2OPC/default")
