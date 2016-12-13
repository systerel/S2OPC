#!/bin/bash
#
#  Script to produce stack delivery archive
#  $1 must be the branch/commit to use for generating the delivery
#  $2 must be the delivery name (e.g.: INGOPCS_Stack_1.0)

BRANCH_COMMIT=$1
DELIVERY_NAME=$2

if [[ -z $BRANCH_COMMIT ]]; then
    echo "Provide the branch name or commit reference to use for generating the delivery"
    exit 1
fi

if [[ -z $DELIVERY_NAME ]]; then
    echo "Provide the delivery name for which a git branch must exist"
    exit 1
fi

echo "Checking out $BRANCH_COMMIT"
git co $BRANCH_COMMIT &> /dev/null || exit 1
echo "Creation of $DELIVERY_NAME branch"
git co -b $DELIVERY_NAME || exit 1
echo "Generation of documentation with doxygen"
doxygen doxygen/ingopcs-stack.doxyfile &> /dev/null || exit 1
echo "Add documentation in delivery branch"
git add -f apidoc &> /dev/null || exit 1
git commit -m "Add doxygen documentation for version $DELIVERY_NAME" &> /dev/null || exit 1
echo "Generation of archive of version $DELIVERY_NAME"
git archive --prefix=$DELIVERY_NAME/ -o $DELIVERY_NAME.tar.gz $DELIVERY_NAME || exit 1

if [ $? -eq 0 ]; then
    echo "=============================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' succeeded"
    echo "Please tag the $DELIVERY_NAME branch on bare repository"
else
    echo "==========================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' failed"
    rm -f $DELIVERY_NAME.tar.gz || exit 1
fi
