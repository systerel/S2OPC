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
BUILD_DIGEST=registry.gitlab.com/systerel/s2opc/build@sha256:0254605b9ee11bf28e1a75dcf19bd99126db2479b397fdd7d6423d0179d24744 # build:1.42
MINGW_DIGEST=registry.gitlab.com/systerel/s2opc/mingw_build@sha256:5cbfd3181a228b9510b402cea6ac83175ac482d348dd2673f6a4c7aeac40f773 # mingw_build:1.14
RPI_DIGEST=registry.gitlab.com/systerel/s2opc/rpi-build@sha256:ef0e91dae560b77396e142d81fc1a9740bd7f51355a2055c3b6510faa33ea8c4 # rpi_build:1.7
CHECK_DIGEST=registry.gitlab.com/systerel/s2opc/check@sha256:1458207c7241df9955cd3ec65fc397847fa8ac3122110c4a4b9387843ec1e5a8 # check 1.20
TEST_DIGEST=registry.gitlab.com/systerel/s2opc/test@sha256:8fbec254f6b8c8467b780d407c6f3c2e87d87899e141b8b2c9013ba30b4f0c8d # test:2.16
ZEPHYR_DIGEST=registry.gitlab.com/systerel/s2opc/zephyr_build:v3.6.0-b@sha256:13069418dd7b0317f93f02986b01f285f0f66956670f2fecded9850ace85d734 # zephyr_build:v3.6.0-b
FREERTOS_DIGEST=registry.gitlab.com/systerel/s2opc/freertos_build:v1.0@sha256:09addc3a3bfcd46ff5581ab21229330fb4cc506c331cf7a31b0a58a0d886de0d # freertos_build:v1.1 (FreeRTOS for STM32-H723ZG only for CI purpose)

# Private images
GEN_DIGEST=docker.aix.systerel.fr/c838/gen@sha256:e7c7f0427d49d162c66410e05eb04a5859ec5e87a04d2ee82bbf25772acf7adf # docker.aix.systerel.fr/c838/gen:1.4
UACTT_WIN_DIGEST=com.systerel.fr:5000/c838/uactt-win@sha256:98b4488aa85310d1e668af8f42be4973e2e7494fe9f44adbaf81896f794db794 # com.systerel.fr:5000/c838/uactt-win:2.3
UACTT_LINUX_DIGEST=com.systerel.fr:5000/c838/uactt-linux@sha256:e63916a03bf824d9d9f1f5b16479605c663507ab829b2fbc131f2b6b4666935f # com.systerel.fr:5000/c838/uactt-linux:1.4
NETWORK_FUZZING_DIGEST=com.systerel.fr:5000/c838/opcua-network-fuzzer@sha256:ed423e0733e2a5131dd57f9e8ef2160230e6838ca94308ea2289bb50ce419e28 #com.systerel.fr:5000/c838/opcua-network-fuzzer:3.0
PIKEOS_DIGEST=docker.aix.systerel.fr/c838/pikeos:1.1@sha256:52814500eeabfcb6596097a218634dce5dbbe19cc416cdeed473ac0fcd59fafb # docker.aix.systerel.fr/c838/pikeos:1.1
