#!/bin/bash
# script to generate the C sources files from the XML address spaces
#

make -C address_space_generation all || exit 1
