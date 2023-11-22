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
BUILD_IMAGE=sha256:8e0b9d304625e87b124620cf0c961e7b2a045134d6631affac0793fd7f3d4fcb # build:1.39
MINGW_IMAGE=sha256:10b620d966863253a69ce402e6e11c0932321c77569de3ea4363c31d81ab849a # mingw_build:1.14
RPI_IMAGE=sha256:6833a87456545ac3dcfa8a0490a11e20ada97383b3a6768b27f3406940c1b23b # rpi_build:1.6
CHECK_IMAGE=sha256:bf7e8accd7bf22c7092fa706bb7662e089e4a0b178908bfc1d13f7094e880b01 # check 1.20
TEST_IMAGE=sha256:6727e7354990d8cae3700093066e218b4360aeed84cbbd9f350d576c3c6033 # test:2.15
ZEPHYR_IMAGE=sha256:f7c2b2759954d9b876adfc933978b2974cba027f591878a876b27ebee1c0d54d # zephyr_build:v3.2.0-b
FREERTOS_IMAGE=sha256:93bc81ad9e4d8d6eda5566ea763987d87dfbfb3f202497bab9fecbb0b1e5c8b5 # freertos_build:v1.1 (FreeRTOS for STM32-H723ZG only for CI purpose)

# Private images
GEN_IMAGE=sha256:0772db3b0f8466def7656b952f1121ca8db70a521bf0b8fd0684e6e2beecc81d # docker.aix.systerel.fr/c838/gen:1.4
UACTT_WIN_IMAGE=sha256:e150d10080d332400b4ee1318c1ff29e38fa57bc3a5348a517440c09e946e447 # com.systerel.fr:5000/c838/uactt-win:2.1
UACTT_LINUX_IMAGE=sha256:39f254e66a175119714a8b1f1d2a2a74617885eb3a000e05c35a8fa5c61f39d3 # com.systerel.fr:5000/c838/uactt-linux:1.3
