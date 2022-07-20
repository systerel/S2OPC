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
BUILD_IMAGE=sha256:12eae257acf165e632233b1903ac84e8236c050be032b7e84b14f1f8344d0119 # build:1.28
MINGW_IMAGE=sha256:4edcdfff072522487730fb268bf834438d47a8fc5e662e5ac7868c4ecb367dcb # mingw_build:1.13
RPI_IMAGE=sha256:1d0eb4c4f99214faf186b4af677f138ef6e01945053e1b7533c99edfdf469f0f # rpi_build:1.4
CHECK_IMAGE=sha256:20d4e72a3c1681a55aeb7b083a28edf4b58d12d113fc9271806d8e70900000ff # check 1.14
TEST_IMAGE=sha256:eeefe721b7a6abc26919fac93a886a72dda7a4e0729730c927f6e84146be2c06 # test:2.13
ZEPHYR_IMAGE=sha256:b0d247f1dc154ce52714bbada4232187820104d69f2d916d265121a51256506e # zephyr_build:v1

# Private images
GEN_IMAGE=sha256:c9763068f1e990534d779f2c327296d33647404b5606cccf1fe63c3b3a18ea17 # docker.aix.systerel.fr/c838/gen:1.3
UACTT_WIN_IMAGE=sha256:6c5d97dd84cf85269c4f89250e6b12601fdd0298eeee14675c8600e0972d953e # com.systerel.fr:5000/c838/uactt-win: 1.8
UACTT_LINUX_IMAGE=sha256:f2a80efc09eb004353cc6dde04f023500113a12f83158086dd79cba639883bc4 # com.systerel.fr:5000/c838/uactt-linux: 1.2

