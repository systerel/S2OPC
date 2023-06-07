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
BUILD_IMAGE=sha256:12e94f0631ca8a3c9121d142a467351a89d120eaf6d1819e728a9d41d06e218b # build:1.34
MINGW_IMAGE=sha256:51364ca5cfa2d75219e616967f4969112c35758704f24738d0083e05f326e061 # mingw_build:1.13
RPI_IMAGE=sha256:464cfa4e22af6d1933a17b14db182632dfd28febc3495adbd286aa1e5d8265e4 # rpi_build:1.5
CHECK_IMAGE=sha256:55f5a3f10a3e6dd4b1c2d076df98c818eaeb749d623ef57208f6293061a7b8a4 # check 1.16
TEST_IMAGE=sha256:8aa26a1fc2f67f96dff5789e55373bd39e60dabca63acbfd9670707ebc171ee1 # test:2.14
ZEPHYR_IMAGE=sha256:f7aab89279d42f3ff775b237cdf5afc68f80662c7604268a3012ce092523ec8f # zephyr_build:v3.2.0

# Private images
GEN_IMAGE=sha256:0772db3b0f8466def7656b952f1121ca8db70a521bf0b8fd0684e6e2beecc81d # docker.aix.systerel.fr/c838/gen:1.4
UACTT_WIN_IMAGE=sha256:e150d10080d332400b4ee1318c1ff29e38fa57bc3a5348a517440c09e946e447 # com.systerel.fr:5000/c838/uactt-win:2.1
UACTT_LINUX_IMAGE=sha256:39f254e66a175119714a8b1f1d2a2a74617885eb3a000e05c35a8fa5c61f39d3 # com.systerel.fr:5000/c838/uactt-linux:1.3
