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

set -e

SRC_DIR="$(cd "$(dirname "$0")" && pwd)"

# See scan-build --help for a list of all checkers
# This list was originally built for Clang 6.0.0

EXTRA_CHECKERS="
alpha.core.BoolAssignment
alpha.core.CallAndMessageUnInitRefArg
alpha.core.CastSize
alpha.core.CastToStruct
alpha.core.Conversion
alpha.core.DynamicTypeChecker
alpha.core.FixedAddr
alpha.core.IdenticalExpr
alpha.core.PointerArithm
alpha.core.SizeofPtr
alpha.core.StackAddressAsyncEscape
alpha.core.TestAfterDivZero
alpha.security.MallocOverflow
alpha.security.ReturnPtrRange
alpha.security.taint.TaintPropagation
alpha.unix.cstring.BufferOverlap
alpha.unix.cstring.NotNullTerminated
alpha.unix.cstring.OutOfBounds
nullability.NullableDereferenced
nullability.NullablePassedToNonnull
nullability.NullableReturnedFromNonnull
security.FloatLoopCounter
valist.CopyToSelf
valist.Uninitialized
valist.Unterminated
"

DISABLED_CHECKERS="
deadcode.DeadStores
alpha.core.PointerSub
"

# Analyzers not enabled by default that cause too many false positives:
# - alpha.security.ArrayBoundV2
# - alpha.deadcode.UnreachableCode

SCAN_BUILD="
scan-build
  -o analyzer-report
  --use-cc=clang
"

for c in $EXTRA_CHECKERS; do SCAN_BUILD="${SCAN_BUILD} -enable-checker $c"; done
for c in $DISABLED_CHECKERS; do SCAN_BUILD="${SCAN_BUILD} -disable-checker $c"; done

mkdir build-analyzer
cd build-analyzer
$SCAN_BUILD cmake -DWITH_NANO_EXTENDED=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo $SRC_DIR
$SCAN_BUILD make -j$(nproc)

# Report folders have names of the form 2018-07-...
REPORT_FOLDER=$(ls -d analyzer-report/20* 2>/dev/null || true)
if [ -n "$REPORT_FOLDER" ] ; then
	echo "Some errors were found by Clang static analyzer."
	echo "Run scan-view $(pwd)/$REPORT_FOLDER to see the report."
	exit 1
fi

echo "No errors found by Clang static analyzer."
exit 0
