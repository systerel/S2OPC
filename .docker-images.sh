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
BUILD_IMAGE=sha256:7d593f99cec17d248dc42c31b9be947aa0f17344f513be1a66008ce7547097c0 # build:1.30
MINGW_IMAGE=sha256:3d552edb5eab18138ef4ad6652b6da8fef00dc36dc6b9fd9fe890ffff513c3e5 # mingw_build:1.12
RPI_IMAGE=sha256:1d0eb4c4f99214faf186b4af677f138ef6e01945053e1b7533c99edfdf469f0f # rpi_build:1.4
CHECK_IMAGE=sha256:20d4e72a3c1681a55aeb7b083a28edf4b58d12d113fc9271806d8e70900000ff # check 1.14
TEST_IMAGE=sha256:eeefe721b7a6abc26919fac93a886a72dda7a4e0729730c927f6e84146be2c06 # test:2.13
ZEPHYR_IMAGE=sha256:b0d247f1dc154ce52714bbada4232187820104d69f2d916d265121a51256506e # zephyr_build:v1

# Private images
GEN_IMAGE=sha256:0772db3b0f8466def7656b952f1121ca8db70a521bf0b8fd0684e6e2beecc81d # docker.aix.systerel.fr/c838/gen:1.4
UACTT_WIN_IMAGE=sha256:1ea36bfabfd56f4567eda8e40c3ac774a4342fdeb18ac642ab31a03bd78f2862 # com.systerel.fr:5000/c838/uactt-win:1.9
UACTT_LINUX_IMAGE=sha256:f2a80efc09eb004353cc6dde04f023500113a12f83158086dd79cba639883bc4 # com.systerel.fr:5000/c838/uactt-linux:1.2

