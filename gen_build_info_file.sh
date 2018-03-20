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

echo "DIR IS: $1"

BUILD_INFO_DIR=$1/csrc/services/b2c
BUILD_INFO_FILE=$BUILD_INFO_DIR/toolkit_build_info.h

GITID=$(git log -1 --format='%H')
echo "Commit id : $GITID";
DATE=`date +%Y-%m-%d`

is_dirty() {
   test -n "$(git status --porcelain)"
}

if is_dirty; then
   DIRTY="*";
fi

VERSION="$GITID$DIRTY"
echo "Building SOPC-$VERSION"

echo '#ifndef _toolkit_build_info_h' > $BUILD_INFO_FILE
echo -e '#define _toolkit_build_info_h' >> $BUILD_INFO_FILE
echo '#include "sopc_user_app_itf.h"' >> $BUILD_INFO_FILE
echo 'const SOPC_Build_Info toolkit_build_info = {' >> $BUILD_INFO_FILE
echo '.toolkitVersion = TOOLKIT_VERSION,' >> $BUILD_INFO_FILE
echo '.toolkitSrcSignature ="'$VERSION'",' >> $BUILD_INFO_FILE
echo '.toolkitDockerId = "'$DOCKER_IMAGE'",' >> $BUILD_INFO_FILE
echo '.toolkitBuildDate = "'$DATE'"' >> $BUILD_INFO_FILE
echo '};' >> $BUILD_INFO_FILE
echo '#endif' >> $BUILD_INFO_FILE
