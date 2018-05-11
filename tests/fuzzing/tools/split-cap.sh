#!/bin/bash

# Copyright (C) 2018 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
