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
BUILD_DIGEST=registry.gitlab.com/systerel/s2opc/build@sha256:047b78d7ccea9c82ed3ee854d11e85afd6efbaf9d283057455d7016a69125695 # build:1.55
MINGW_DIGEST=registry.gitlab.com/systerel/s2opc/mingw_build@sha256:d158e52c51d2f97d4bd13bcc93ebbe5793025c0032dd7e6116bdf9293fc790bf # mingw_build:1.15
RPI_DIGEST=registry.gitlab.com/systerel/s2opc/rpi-build@sha256:070b1ca58667ee5908902ba3eb03e5fa05993d3a86d2c8ddeb6f7f41ed62609c # rpi_build:1.9
CHECK_DIGEST=registry.gitlab.com/systerel/s2opc/check@sha256:8be8c50cd484989c987b6c28ded2e03f30e81ec27bb53e41cecde8cdc3bb86c8 # check 1.22
TEST_DIGEST=registry.gitlab.com/systerel/s2opc/test@sha256:3012dedca1b6fdb11109a1798fbfef03c84244071312254af45aafab41340f73 # test:2.17
ZEPHYR_DIGEST=registry.gitlab.com/systerel/s2opc/zephyr_build@sha256:a3361ba6c51ce52026650d6ed5b9344db7d99343b803fc4eaf4c9c4d39f14536 # zephyr_build:v4.1.0
FREERTOS_DIGEST=registry.gitlab.com/systerel/s2opc/freertos_build:v1.0@sha256:d3da42d15ae8e1973d388cd09e75fc3dfb316763d6fd10115d0fe790e3c81346 # freertos_build:v1.2.2 (FreeRTOS for STM32-H723ZG only for CI purpose)
SONARQUBE_DIGEST=registry.gitlab.com/systerel/s2opc/sonarqube@sha256:4baaa859d74163e40467c5fa630ad205a83a4190b7fe472d32f071619a31ca5e #registry.gitlab.com/systerel/s2opc/sonarqube:1.0

# Private images
GEN_DIGEST=docker.aix.systerel.fr/c838/gen@sha256:e7c7f0427d49d162c66410e05eb04a5859ec5e87a04d2ee82bbf25772acf7adf # docker.aix.systerel.fr/c838/gen:1.4
UACTT_WIN_DIGEST=com.systerel.fr:5000/c838/uactt-win@sha256:98b4488aa85310d1e668af8f42be4973e2e7494fe9f44adbaf81896f794db794 # com.systerel.fr:5000/c838/uactt-win:2.3
UACTT_LINUX_DIGEST=com.systerel.fr:5000/c838/uactt-linux@sha256:8c85eccb42cb856c4795ba367a93f28959295fecc36cd1318aaf89ff27b0db59 # com.systerel.fr:5000/c838/uactt-linux:1.10
NETWORK_FUZZING_DIGEST=docker.aix.systerel.fr/c838/opcua-network-fuzzer@sha256:35e40ba847bdfe5f3e7112f465adb2c70d1f0694521f6ba8d018c55f651ef205 # docker.aix.systerel.fr/c838/opcua-network-fuzzer:3.3
PIKEOS_DIGEST=docker.aix.systerel.fr/c838/pikeos@sha256:3d5dcbab75b0c98a8d27d1cfa239bfa43b5b87bc1bbaca2a7c8a5bfe2adc5aeb # docker.aix.systerel.fr/c838/pikeos:1.2.1
CYBERWATCH_DIGEST=docker.aix.systerel.fr/c838/cyberwatch@sha256:003b648857636b08825c6214c7ae86390e7c958a6814f15ee4c170c737347292 # docker.aix.systerel.fr/c838/cyberwatch:3.0

