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


# Checks license is present as expected in the header of the source code files
# of the project.

# Create temporary files
# Helper script holds the head | diff, so that its compatible with find | xargs
HELPER_SCRIPT=$(mktemp)
HEADER_C=$(mktemp)
HEADER_PY=$(mktemp)
HEADER_SH=$(mktemp)
HEADER_XSL=$(mktemp)

# Helper script takes the reference header and the file to test
echo '#!/bin/bash
if [[ -z $1 || -z "$2" ]] ; then
    exit 2
fi
n_lines=$(wc -l < $1)
if [[ $n_lines -eq 0 ]] ; then
    echo Empty header file $1
    exit 3
fi
head -n $n_lines "$2" | diff - $1 > /dev/null
last=$?
if [[ ! $last -eq 0 ]] ; then
    echo Invalid copyright header in file $2
    exit 1
fi' > $HELPER_SCRIPT
chmod +x $HELPER_SCRIPT

# C copyright
echo '/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
' > $HEADER_C

# Py header
echo '#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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
' > $HEADER_PY

# Script header
echo '#!/bin/bash

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
' > $HEADER_SH
echo '<!--
Copyright (C) 2018 Systerel and others.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
-->
' > $HEADER_XSL


# Find and check
err=0
find csrc tests address_space_generation -name "*.[hc]" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_C || { echo 'Expected header:' ; cat $HEADER_C ; err=1 ; }
find bsrc -maxdepth 1 -name "*.[im]??" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_C || { echo 'Expected header:' ; cat $HEADER_C ; err=1 ; }
find validation address_space_generation -name "*.py" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_PY || { echo 'Expected header:' ; cat $HEADER_PY ; err=1 ; }
find . -name "*.sh" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_SH || { echo 'Expected header:' ; cat $HEADER_SH ; err=1 ; }
find address_space_generation acceptances_tests -name "*.xsl" -print0 | xargs -0 -n 1 $HELPER_SCRIPT $HEADER_XSL || { echo 'Expected header:' ; cat $HEADER_XSL ; err=1 ; }

if [[ 0 -eq $err ]] ; then
    echo "All copyrights are OK"
fi
exit $err
