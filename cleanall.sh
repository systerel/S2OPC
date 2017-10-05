#!/bin/bash
# Script to build the INGOPCS project:
# Clean build and bin dirs

./clean.sh
\rm -fr ./src/services/B_genC
make -C address_space_generation cleanall
