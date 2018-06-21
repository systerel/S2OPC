#!/bin/bash

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

usage() {
	echo "Usage: $0 CAP_FILE"
	echo ""
	echo "Splits a PCAP file into separate files, one per TCP packet."
	echo "The content of each file is the TCP payload of the packet."
	echo ""
	echo "The generated files are created in the current directory."
	echo ""
	echo "This script is useful to generate a fuzzing corpus out of a"
	echo "network capture of OPC-UA data."
}

check_binary() {
	local bin=$1
	local pkg=$2

	if ! which $bin >/dev/null 2>&1; then
		echo "$bin does not seem to be installed on this machine."
		echo "Install $pkg to resolve this."
		exit 1
	fi
}

CAP_FILE=$1

if [ -z "$CAP_FILE" ]; then
	usage
	exit 1
fi

check_binary tshark wireshark
check_binary xxd vim

set -e

COUNTER=0

for line in $(tshark -r "$CAP_FILE" -T fields -e tcp.payload); do
	echo $line | tr -d ':' | xxd -r -p > $(printf "p%05d.raw" $COUNTER)
	COUNTER=$((COUNTER+1))
done

echo "I littered your directory with $COUNTER files \\o/"
