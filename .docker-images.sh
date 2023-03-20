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

# Define the docker images used in S2OPC

# Public images (registry.gitlab.com/systerel/s2opc)
BUILD_IMAGE=sha256:024014c02579da07da2c2eb45ad97ec84f35e6ae135ca4e7c09bd8e0ebca5255 # build:1.33
MINGW_IMAGE=sha256:3d552edb5eab18138ef4ad6652b6da8fef00dc36dc6b9fd9fe890ffff513c3e5 # mingw_build:1.12
RPI_IMAGE=sha256:464cfa4e22af6d1933a17b14db182632dfd28febc3495adbd286aa1e5d8265e4 # rpi_build:1.5
CHECK_IMAGE=sha256:068bd9b338a6fa1848d31fee375d779c2351dde8737dc50cb904333dec13d071 # check 1.15
TEST_IMAGE=sha256:8aa26a1fc2f67f96dff5789e55373bd39e60dabca63acbfd9670707ebc171ee1 # test:2.14
ZEPHYR_IMAGE=sha256:b0d247f1dc154ce52714bbada4232187820104d69f2d916d265121a51256506e # zephyr_build:v1

# Private images
GEN_IMAGE=sha256:0772db3b0f8466def7656b952f1121ca8db70a521bf0b8fd0684e6e2beecc81d # docker.aix.systerel.fr/c838/gen:1.4
UACTT_WIN_IMAGE=sha256:376167d519bfb1353e577892999efe5fd51a438c0477baac4ee0dd5a5eb923c8 # com.systerel.fr:5000/c838/uactt-win:1.12
UACTT_LINUX_IMAGE=sha256:f2a80efc09eb004353cc6dde04f023500113a12f83158086dd79cba639883bc4 # com.systerel.fr:5000/c838/uactt-linux:1.2

