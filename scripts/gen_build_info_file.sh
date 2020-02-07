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

BUILD_INFO_FILE=$1

DATE=`date +%Y-%m-%d`

# check if git command is available and
# current directory is a git repository
`git status>/dev/null 2>&1`
IS_GIT_REPO=$?

is_dirty() {
    git diff-index --quiet HEAD &> /dev/null
    if [[ $? == 0 ]]; then
        return 1
    else
        return 0
    fi
}

if [ $IS_GIT_REPO -eq 0 ]
then
    GITID=$(git log -1 --format='%H')
    echo "Commit id : $GITID";
    if is_dirty; then
        DIRTY="*";
    fi
    VERSION="$GITID$DIRTY"
else
    VERSION="Unknown_Revision"
fi

echo "Building SOPC-$VERSION"

if [ -f "$BUILD_INFO_FILE" ]
then
    sed -i 's/.buildSrcCommit =.*/.buildSrcCommit = "'"$VERSION"'",/' $BUILD_INFO_FILE
    sed -i 's/.buildDockerId =.*/.buildDockerId = "'"$DOCKER_IMAGE"'",/' $BUILD_INFO_FILE
    sed -i 's/.buildBuildDate =.*/.buildBuildDate = "'"$DATE"'",/' $BUILD_INFO_FILE
else
    echo "ERROR: file $BUILD_INFO_FILE not found"
    exit 1
fi
