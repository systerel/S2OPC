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
BUILD_IMAGE=sha256:346ec7469f7dfedc8d5078c51c0d485915667647a2816b8ac36e1e35d04190 # build:1.36
MINGW_IMAGE=sha256:51364ca5cfa2d75219e616967f4969112c35758704f24738d0083e05f326e061 # mingw_build:1.13
RPI_IMAGE=sha256:464cfa4e22af6d1933a17b14db182632dfd28febc3495adbd286aa1e5d8265e4 # rpi_build:1.5
CHECK_IMAGE=sha256:d66ce82fe01794b918387be9f331f82cc768ef8acf58f16ff6f8840c3f6b67 # check 1.17
TEST_IMAGE=sha256:6727e7354990d8cae3700093066e218b4360aeed84cbbd9f350d576c3c6033 # test:2.15
ZEPHYR_IMAGE=sha256:f7c2b2759954d9b876adfc933978b2974cba027f591878a876b27ebee1c0d54d # zephyr_build:v3.2.0-b
FREERTOS_IMAGE=sha256:c028e915256b915c8f32ccf1d9c601113f78972b54650d40257bcfcd723725e2 # freertos_build:v1.0 (FreeRTOS for STM32-H723ZG only for CI purpose)

# Private images
GEN_IMAGE=sha256:0772db3b0f8466def7656b952f1121ca8db70a521bf0b8fd0684e6e2beecc81d # docker.aix.systerel.fr/c838/gen:1.4
UACTT_WIN_IMAGE=sha256:e150d10080d332400b4ee1318c1ff29e38fa57bc3a5348a517440c09e946e447 # com.systerel.fr:5000/c838/uactt-win:2.1
UACTT_LINUX_IMAGE=sha256:39f254e66a175119714a8b1f1d2a2a74617885eb3a000e05c35a8fa5c61f39d3 # com.systerel.fr:5000/c838/uactt-linux:1.3
