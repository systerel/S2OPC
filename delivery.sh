#!/bin/bash
#
#  Script to produce stack delivery archive
#

DELIVERY_NAME=$1

if [[ -z $DELIVERY_NAME ]]; then
    echo "Provide the delivery name for which a git branch must exist"
    exit 1
fi

echo "Checking out master_stack_ingopcs branch"
git co master_stack_ingopcs || exit 1
echo "Creation of $DELIVERY_NAME branch"
git co -b $DELIVERY_NAME || exit 1
echo "Generation of documentation"
doxygen doxygen/ingopcs-stack.doxyfile || exit 1
git add -f apidoc || exit 1
git commit -m "Add doxygen documentation for version $DELIVERY_NAME" || exit 1
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
