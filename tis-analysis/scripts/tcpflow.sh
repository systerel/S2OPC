#!/bin/bash
##########################################################################
#                                                                        #
#  This file is part of deliverable T3.3 of project INGOPCS              #
#                                                                        #
#    Copyright (C) 2017-2018 TrustInSoft                                 #
#                                                                        #
#  All rights reserved.                                                  #
#                                                                        #
##########################################################################

n=0
IFS=' '
while read -r -a line ; do
  for (( i = 1 ; 1 ; i+=1 )) ; do
    # echo "process [$i] = '${line[$i]}'"
    if [ $(( $i % 8 )) -eq 1 ] ; then values+=$'\n' ; fi
    if [[ ${line[$i]} =~ ^[A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9][A-Fa-f0-9]$ ]] ;
    then
      values+="0x${line[$i]:0:2}, "
      values+="0x${line[$i]:2:2}, "
      (( n += 2 ))
    else
#       echo "'${line[$i]}' doesn't match"
      break
    fi
  done
done

echo "/* WARNING: generated file */"
echo
echo "static const unsigned char msg[$n] = {"
echo "$values"
echo "};"
