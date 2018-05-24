#!/bin/bash

# Copyright (C) 2018 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


#  Run the script given parameters in the docker
#
set -e

DOCKER_IMAGE=sha256:8fbae29855a58b1aa89dfcfae6f6e24b432bedfecfd0c57a32e5063504609c10 # mingw_build:1.4

if [[ -z $SOPC_DOCKER_NEEDS_SUDO ]]; then
    /etc/scripts/run-in-docker $DOCKER_IMAGE CMAKE_TOOLCHAIN_FILE=toolchain-mingw32-w64.cmake BUILD_SHARED_LIBS=true DOCKER_IMAGE=$DOCKER_IMAGE "$@"
else
    sudo /etc/scripts/run-in-docker $DOCKER_IMAGE CMAKE_TOOLCHAIN_FILE=toolchain-mingw32-w64.cmake BUILD_SHARED_LIBS=true DOCKER_IMAGE=$DOCKER_IMAGE "$@"
fi
