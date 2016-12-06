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
git co master_stack_ingopcs
echo "Creation of $DELIVERY_NAME branch"
git co -b $DELIVERY_NAME
echo "Generation of documentation"
doxygen doxygen/ingopcs-stack.doxyfile
git add -f apidoc
git commit -m "Add doxygen documentation for version $DELIVERY_NAME"
echo "Generation of archive of version $DELIVERY_NAME"
git archive --prefix=$DELIVERY_NAME/ -o $DELIVERY_NAME.tar.gz $DELIVERY_NAME

if [ $? -eq 0 ]; then
    echo "=============================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' succeeded"
    echo "Please tag the $DELIVERY_NAME branch on bare repository"
else
    echo "==========================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' failed"
    rm -f $DELIVERY_NAME.tar.gz
fi
