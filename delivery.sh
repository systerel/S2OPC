#!/bin/bash
#
#  Script to produce stack delivery archive
#  $1 must be the branch/commit to use for generating the delivery
#  $2 must be the delivery version number (e.g.: 1.0.0)

echo "Check branch name is valid (1st param): $1"
git show-ref refs/heads/$1 &> /dev/null
if [[ $? != 0 ]]; then
    echo "Provide the LOCAL branch name or commit reference to use for generating the delivery"
    exit 1
fi
BRANCH_COMMIT=$1

if [[ -z $BRANCH_COMMIT ]]; then
    exit 1
fi

echo "Check version number is correct: $2"
regexp='^[0-9]+\.[0-9]+.[0-9]+$'
if ! [[ $2 =~ $regexp ]] ; then
   echo "Error: '$2' is not correct version number X.Y.Z";
   exit 1
fi
DELIVERY_NAME=INGOPCS_Toolkit_$2

echo "Check branch and tag does not exist: $DELIVERY_NAME"
git show-ref refs/heads/$DELIVERY_NAME &> /dev/null
if [[ $? == 0 ]]; then
    echo "Error: refs/heads/$DELIVERY_NAME already exists";
    exit 1
fi
git show-ref refs/tags/$DELIVERY_NAME &> /dev/null
if [[ $? == 0 ]]; then
    echo "Error: refs/tags/$DELIVERY_NAME already exists";
    exit 1
fi

echo "Check archive does not exist: $DELIVERY_NAME.tar.gz"
ls $DELIVERY_NAME.tar.gz &> /dev/null
if [[ $? == 0 ]]; then
    echo "Error: $DELIVERY_NAME.tar.gz already exists";
    exit 1
fi

echo "Checking out $BRANCH_COMMIT"
git co $BRANCH_COMMIT &> /dev/null || exit 1
echo "Creation of $DELIVERY_NAME branch"
git co -b $DELIVERY_NAME || exit 1
echo "Generate and commit version file 'VERSION' with '$2' content"
echo "$2" > VERSION
git add VERSION &> /dev/null || exit 1
git commit -S -m "Add VERSION file" &> /dev/null || exit 1
echo "Generate Stack and Toolkit binaries (Stack library and tests, Toolkit tests)"
make cleanall
make all
if [[ $? != 0 ]]; then
    echo "Error: Generation of Stack or Toolkit binaries failed";
    exit 1
fi
echo "Add Stack and Toolkit binaries"
git add -f bin/ &>/dev/null || exit 1
git add -f install_stack/ &>/dev/null || exit 1
make clean || exit 1
git co bin || exit 1
git co install_stack || exit 1
echo "Generate Stack documentation with doxygen"
cd stack
doxygen doxygen/ingopcs-stack.doxyfile &> /dev/null || exit 1
cd ..
echo "Add Stack documentation in delivery branch"
git add -f stack/apidoc &> /dev/null || exit 1
git commit -S -m "Add doxygen documentation for version $DELIVERY_NAME" &> /dev/null || exit 1
echo "Remove delivery script, .gitignore file and commit"
git rm -f delivery.sh .gitignore &> /dev/null || exit 1
git commit -S -m "Remove delivery script and .gitignore file" &> /dev/null || exit 1
echo "Generation of archive of version $DELIVERY_NAME"
git archive --prefix=$DELIVERY_NAME/ -o $DELIVERY_NAME.tar.gz $DELIVERY_NAME || exit 1

if [ $? -eq 0 ]; then
    echo "=============================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' succeeded"
    echo "Please tag the $DELIVERY_NAME branch on bare repository"
else
    echo "==========================================================="
    echo "Creation of delivery archive '$DELIVERY_NAME.tar.gz' FAILED"
    rm -f $DELIVERY_NAME.tar.gz || exit 1
fi
