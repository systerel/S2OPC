#!/bin/bash
##########################################################################
#                                                                        #
#  This file is part of deliverable T3.3 of project INGOPCS              #
#                                                                        #
#    Copyright (C) 2017 TrustInSoft                                      #
#                                                                        #
#  All rights reserved.                                                  #
#                                                                        #
##########################################################################

set -x

sudo apt-get update

PKGS=(
  # Needed by INGOPCS (but I hope this is already install on the system)
  gcc make

  # Needed by the Check library
  automake autoconf libtool pkg-config texinfo
)

sudo apt-get install --yes --no-install-recommends ${PKGS[@]}
