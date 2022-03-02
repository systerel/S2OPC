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
BUILD_IMAGE=sha256:01589086151582e8d410aa71cc775890a7521804207af16825c506f406472d02 # build:1.27
MINGW_IMAGE=sha256:3d552edb5eab18138ef4ad6652b6da8fef00dc36dc6b9fd9fe890ffff513c3e5 # mingw_build:1.12
RPI_IMAGE=sha256:1d0eb4c4f99214faf186b4af677f138ef6e01945053e1b7533c99edfdf469f0f # rpi_build:1.4
CHECK_IMAGE=sha256:20d4e72a3c1681a55aeb7b083a28edf4b58d12d113fc9271806d8e70900000ff # check 1.14
TEST_IMAGE=sha256:78189778326b86c5e2926b08dd842ea32235990c003ac64491d557480c0656ff # test:2.12
ZEPHYR_IMAGE=sha256:b0d247f1dc154ce52714bbada4232187820104d69f2d916d265121a51256506e # zephyr_build:v1

# Private images
GEN_IMAGE=sha256:c9763068f1e990534d779f2c327296d33647404b5606cccf1fe63c3b3a18ea17 # gen:1.3
UACTT_WIN_IMAGE=sha256:6c5d97dd84cf85269c4f89250e6b12601fdd0298eeee14675c8600e0972d953e # com2.systerel.fr:5000/c838/uactt-win: 1.8
UACTT_LINUX_IMAGE=sha256:f2a80efc09eb004353cc6dde04f023500113a12f83158086dd79cba639883bc4 # com2.systerel.fr:5000/c838/uactt-linux: 1.2

