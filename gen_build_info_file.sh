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

BUILD_INFO_DIR=$1/csrc/configuration
BUILD_INFO_FILE=$BUILD_INFO_DIR/sopc_toolkit_build_info.h

DATE=`date +%Y-%m-%d`

# check if git command is available and
# current directory is a git repository
`git status>/dev/null 2>&1`
IS_GIT_REPO=$?

is_dirty() {
    test -n "$(git status --porcelain)"
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
    sed -i 's/.toolkitSrcCommit =.*/.toolkitSrcCommit = "'"$VERSION"'",/' $BUILD_INFO_FILE
    sed -i 's/.toolkitDockerId =.*/.toolkitDockerId = "'"$DOCKER_IMAGE"'",/' $BUILD_INFO_FILE
    sed -i 's/.toolkitBuildDate =.*/.toolkitBuildDate = "'"$DATE"'",/' $BUILD_INFO_FILE
    sed -i 's/.toolkitSrcCommit =.*/.toolkitSrcCommit = "'"$VERSION"'",/' $BUILD_INFO_FILE
else
    echo "ERROR: file $BUILD_INFO_FILE not found"
    exit 1
fi
