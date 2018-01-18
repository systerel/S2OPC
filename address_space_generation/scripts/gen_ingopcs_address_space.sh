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

set -e
COMMON_AS=./common_part/Common_Address_Space.xml

usage() {
    echo "Usage: `basename $0` [nom du fichier XML contenant l'address space utilisateur][nom du fichier XML resultat]"
}

main() {
    readonly user_as_file=$1
    readonly result_file=$2
    if [[ ! -f "$2" ]] ; then
    usage
    exit 1
    fi

    rm -f $result_file

    sed '/<!-- End: Root folders -->/,/<\/UANodeSet>/d' $COMMON_AS > $result_file
    # suppression des deux premières lignes et de la partie namespaceris/alias
    sed "1,2d" $user_as_file | sed  "/Alias/d" | sed "/NamespaceUris/d" >> $result_file

    printf "Generation terminée \n"
}

main $@
