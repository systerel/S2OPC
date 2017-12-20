#!/bin/bash
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
