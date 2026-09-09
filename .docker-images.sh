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
BUILD_DIGEST=registry.gitlab.com/systerel/s2opc/build@sha256:d48f2685fcb30346acb8f2e64f6f2b0278761290d158052fb9bf31d2b41e9005 # build:2.0
MINGW_DIGEST=registry.gitlab.com/systerel/s2opc/mingw_build@sha256:d158e52c51d2f97d4bd13bcc93ebbe5793025c0032dd7e6116bdf9293fc790bf # mingw_build:1.15
RPI_DIGEST=registry.gitlab.com/systerel/s2opc/rpi-build@sha256:070b1ca58667ee5908902ba3eb03e5fa05993d3a86d2c8ddeb6f7f41ed62609c # rpi_build:1.9
CHECK_DIGEST=registry.gitlab.com/systerel/s2opc/check@sha256:c3a99ad388043fa77a33d93bb61be11969511af6ced23929c16527eb6046e444 # check:2.0
TEST_DIGEST=registry.gitlab.com/systerel/s2opc/test@sha256:d6ca3821754b8f5f74d7fe426e35e7709e1ce772b5f05c9f2e28efd2f4ee3189  # test:2.19
ZEPHYR_DIGEST=registry.gitlab.com/systerel/s2opc/zephyr_build@sha256:a3361ba6c51ce52026650d6ed5b9344db7d99343b803fc4eaf4c9c4d39f14536 # zephyr_build:v4.1.0
FREERTOS_DIGEST=registry.gitlab.com/systerel/s2opc/freertos_build:v1.0@sha256:d3da42d15ae8e1973d388cd09e75fc3dfb316763d6fd10115d0fe790e3c81346 # freertos_build:v1.2.2 (FreeRTOS for STM32-H723ZG only for CI purpose)
SONARQUBE_DIGEST=registry.gitlab.com/systerel/s2opc/sonarqube@sha256:b3ff9482ea22887a6923ddbac82592ff4ad9a4cf44dc7e29df2c9588a5405417 #registry.gitlab.com/systerel/s2opc/sonarqube:2.0
MUSL_DIGEST=registry.gitlab.com/systerel/s2opc/build-musl@sha256:c4aebefcbd437cb1259e947ed99beb61848ee6dcd13052c8485278dbb3f7c431 # build-musl:v3.0

# Private images
GEN_DIGEST=docker.aix.systerel.fr/c838/gen@sha256:e7c7f0427d49d162c66410e05eb04a5859ec5e87a04d2ee82bbf25772acf7adf # docker.aix.systerel.fr/c838/gen:1.4
UACTT_WIN_DIGEST=com.systerel.fr:5000/c838/uactt-win@sha256:98b4488aa85310d1e668af8f42be4973e2e7494fe9f44adbaf81896f794db794 # com.systerel.fr:5000/c838/uactt-win:2.3
UACTT_LINUX_DIGEST=com.systerel.fr:5000/c838/uactt-linux@sha256:3f1eb2312da5cd6b8ff031773c6860bc456ada6a6e93d4addbc8277408c56da6 # com.systerel.fr:5000/c838/uactt-linux:1.11
NETWORK_FUZZING_DIGEST=docker.aix.systerel.fr/c838/opcua-network-fuzzer@sha256:35e40ba847bdfe5f3e7112f465adb2c70d1f0694521f6ba8d018c55f651ef205 # docker.aix.systerel.fr/c838/opcua-network-fuzzer:3.3
PIKEOS_DIGEST=docker.aix.systerel.fr/c838/pikeos@sha256:3d5dcbab75b0c98a8d27d1cfa239bfa43b5b87bc1bbaca2a7c8a5bfe2adc5aeb # docker.aix.systerel.fr/c838/pikeos:1.2.1
CYBERWATCH_DIGEST=docker.aix.systerel.fr/c838/cyberwatch@sha256:003b648857636b08825c6214c7ae86390e7c958a6814f15ee4c170c737347292 # docker.aix.systerel.fr/c838/cyberwatch:3.0
FUZZYSULLY_DIGEST=docker.aix.systerel.fr/c838/opcua-fuzzysully@sha256:6e01e319277f700c635e07ccc697106fbe2789e20af404071a9297d038bba542 # docker.aix.systerel.fr/c838/opcua-fuzzysully:1.1
