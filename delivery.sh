#!/bin/bash
#
#  Script to produce stack delivery archive
#

DELIVERY_NAME=$1
LOCAL_REPO=`pwd`
REPO_NAME=$(basename $LOCAL_REPO)
echo $LOCAL_REPO
echo $REPO_NAME

if [[ -z $DELIVERY_NAME ]]; then
    echo "Provide the delivery name for which a git branch must exist"
    exit 1
fi

git clone $LOCAL_REPO || exit 2
mv $REPO_NAME $DELIVERY_NAME || exit 2
cd $DELIVERY_NAME
git checkout $DELIVERY_NAME || exit 2
rm -rf .git .gitignore .project .cproject delivery.sh
make doc || exit 2
cd ../
tar -zcvf $DELIVERY_NAME.tar.gz $DELIVERY_NAME
if [ $? -eq 0 ]; then
    echo "=============================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' succeeded"
    rm -rf $DELIVERY_NAME
else
    echo "==========================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' failed"
    rm -f $DELIVERY_NAME.tar.gz
fi
