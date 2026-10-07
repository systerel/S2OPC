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
    settings = ("os", "compiler", "build_type", "arch")
    generators = ("CMakeToolchain", "CMakeDeps")

    def requirements(self):
        self.requires("expat/2.9.0")
        self.requires("mbedtls/3.6.7")
        self.requires("cyclone-common/2.6.0@systerel+s2opc/default")
        self.requires("cyclone-crypto/2.6.0@systerel+s2opc/default")
        self.requires("paho-mqtt-c/1.3.4")
        self.requires("libcheck/0.14.0@systerel+s2opc/default")
        self.requires("doxygen/1.12.0")

    def build_requirements(self):
        self.tool_requires("cmake/3.31.12")
        self.tool_requires("make/4.3")
        self.tool_requires("gcc/15.3.0")
        self.tool_requires("binutils/2.47@systerel+s2opc/default")
