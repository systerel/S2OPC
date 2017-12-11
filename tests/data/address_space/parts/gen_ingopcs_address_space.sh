#!/bin/bash
set -e
COMMON_AS=Common_Address_Space.xml
GEN_AS=../ingopcs.xml

usage() {
    echo "Usage: `basename $0` [nom du fichier XML contenant l'address space utilisateur]"
}

main() {
    readonly user_as_file=$1
    if [[ ! -f "$1" ]] ; then
    usage
    exit 1
    fi

    rm -f $GEN_AS

    sed '/<!-- End: Root folders -->/,/<\/UANodeSet>/d' $COMMON_AS > $GEN_AS
    # suppression des deux premières lignes et de la partie namespaceris/alias
    sed "1,2d" $user_as_file | sed  "/Alias/d" | sed "/NamespaceUris/d" >> $GEN_AS

    printf "Generation terminée \n"
}

main $@
