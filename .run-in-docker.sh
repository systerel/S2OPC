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

set -e

# Set the RUN_INTERACTIVELY environment variable to run the Docker
# container in interactive mode. By default it will run in batch mode.
interactive="${RUN_INTERACTIVELY:+-it}"
network_host="${HOST_NETWORK:+--network host}"

IMAGE=$1
shift

# Keep the same user id as the one running this script through sudo
uid=$(id -u $SUDO_USER)

# Mount point is the path of this script
mount_point="$PWD/$(dirname "$0")"

# Creating fake /etc/passwd file for local user
TMP_FILE=$(mktemp)
echo "docker_user:x:$uid:$uid::/tmp:/sbin/nologin" > $TMP_FILE
docker run --ulimit nofile=1024:1024 $network_host --rm $interactive --user "$uid" \
    --volume="$mount_point":"$mount_point" --volume "$TMP_FILE:/etc/passwd" \
    --workdir "$PWD" --entrypoint /bin/bash "$IMAGE" -c "$*"

rm -f $TMP_FILE
