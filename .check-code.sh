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


# Script to check properties on code of the S2OPC project:
# - check absence of use of functions and includes which guarantee compliance with some CERT rules
# - run C sources compilation with Clang compiler using specific options to guarantee some CERT rules
# - run C sources analysis using Clang tidy tool using specific options to guarantee some CERT rules
# - run C sources automatic formatting using Clang format tool to normalize code presentation
#
# OPTIONS:
# - if first argument provided is "advanced", the clang-tidy tool will be used with the default options in addition to CERT specific analyses. It could allow to detect more programming errors but could also contains false positives.

ISADVANCED=$1

BSRC=bsrc
CSRC=src
TST=tests
DEMO=samples

EXITCODE=0
LOGPATH=$(pwd)/pre-build-check.log

# Array to collect all failure reasons
FAILURES=()

# Redirect all output and errors to log file
echo "Pre-build-check log" > $LOGPATH

#### Check contributor list ####
echo "Contributors list verification" | tee -a $LOGPATH
ifs_save=$IFS
IFS=$'\n'
LIST_CONTRIB=(`git log --format="%an <%ae>" | sort -u`)

MISSING_CONTRIBS=()
for contrib in ${LIST_CONTRIB[*]}
do
    grep $contrib Contributors.md &> /dev/null
    if [[ $? != 0 ]]; then
        echo "ERROR: contributor not declared: $contrib" | tee -a $LOGPATH
        MISSING_CONTRIBS+=("$contrib")
        EXITCODE=1
    fi
done

if [[ ${#MISSING_CONTRIBS[@]} -gt 0 ]]; then
    FAILURES+=("Contributors not declared in Contributors.md: ${MISSING_CONTRIBS[*]}")
fi

IFS=$ifs_save

# Prepare a C source file for grep-based CERT checks:
# - mask string literals (so "continue" in a string is not matched)
# - remove comments (so continue in a comment is not matched)
# Output has the same number of lines as the input (required for grep -n line numbers).
strip_c_comments_for_grep()
{
	# Step 1 (sed): mask string literals before touching comments.
	#   s/\\"//g              remove escaped quotes (simplifies the next substitution)
	#   s/"[^"]*"/"..."/g     replace "...." contents with a placeholder
	# Without this step, a string like "/* not a comment */" would be mangled by the
	# comment stripper, and words inside strings would false-trigger grep.
	sed 's/\\"//g; s/"[^"]*"/"..."/g' "$1" |
	# Step 2 (perl): strip // and /* */ comments line by line.
	perl -ne '
		# -n  : read stdin line by line into $_, do not auto-print
		# -e  : inline program (as opposed to a .pl file)
		# chomp: remove the trailing newline from $_ (we add it back explicitly below)
		chomp;

		# $in: true while we are inside an unclosed /* ... */ block spanning several lines.
		if ($in) {
			# Look for the closing */ on this line (.*? = non-greedy; /s = . matches newlines).
			if (s/.*?\*\///s) { $in = 0; }
			# Still inside the block: blank the line (comment text must not reach grep).
			else { $_ = ""; }
		}

		# Remove a // comment and everything after it on this line.
		s/\/\/.*$//;

		# Remove all complete /* ... */ blocks on this line (handles several on one line).
		while (s/\/\*.*?\*\///s) {}

		# If /* remains without a closing */ on this line, start a multiline block.
		if (s/\/\*.*//s) { $in = 1; }

		# Always print one output line per input line, even when $_ is empty.
		# Do not use perl -p here: -p skips printing when $_ is "", which drops lines and
		# shifts grep -n line numbers.
		print "$_\n";
	'
}

# Prepare stdin (already comment-stripped) for the break-outside-switch grep check:
# erase every switch (expr) { ... } block so that break inside switch/case is ignored.
# Loop break; and other break outside switch remain visible to grep.
# Line count is preserved (newlines in blanked regions are kept) for grep -n.
strip_c_switch_blocks_for_grep()
{
	perl -0777 -e '
		# -0777: slurp the entire stdin into a single string (switch bodies span many lines).
		# $_   : the whole file content; <> reads from stdin when no file argument is given.
		$_ = do { local $/; <> };

		# Process every switch statement in the file, one after another.
		while (1) {
			# Stop when no more "switch" word remains.
			# \b = word boundary (matches switch but not myswitch); /s = . matches newlines.
			last unless $_ =~ /\bswitch\b/s;

			# Byte offset in $_ where this switch starts ($-[0] = start of regex match).
			my $start = $-[0];
			# Text from the switch keyword to the end of the file.
			my $rest = substr($_, $start);

			# Match "switch" then any characters except { up to the opening brace of the body.
			# Example: "switch (x)\n    {" — the { that opens the switch body, not earlier ones.
			unless ($rest =~ /\bswitch\b[^{]*\{/s) { last; }

			# $+[0] = position in $rest just after that opening { (length relative to $rest).
			# Must be $+[0] alone, not $+[0]-$start ($start is an offset in $_, not in $rest).
			my $pos = $+[0];

			# Brace counting from the switch body opening { (depth 1 = inside the switch block).
			my $depth = 1;
			while ($pos < length($rest) && $depth > 0) {
				my $c = substr($rest, $pos, 1);
				$depth++ if $c eq "{";   # nested block (e.g. if { } inside a case)
				$depth-- if $c eq "}";   # closing brace
				$pos++;
			}
			# When $depth returns to 0, $pos points just after the switch closing }.

			# Replace the whole switch block with spaces, but keep newline characters.
			# Replacing newlines with spaces would merge lines and break grep -n.
			my $blank = substr($_, $start, $pos);
			$blank =~ s/[^\n]/ /g;
			substr($_, $start, $pos) = $blank;
		}

		print;
	'
}

#### Check absence of functions / includes ####
echo "Checking specific functions or headers not used in code" | tee -a $LOGPATH
EXLUDE_CERT_VERIFIED_MANUALLY="*\/linux\/p_sopc_askpass.c"
CHECK_CERT_RULE_ABSENCE_FAILED=false

CHECK_CERT_RULE_ABSENCE="(goto|continue|restrict|fgets|fgetws|getc|putc|getwc|putwc|fsetpos|rand|readlink|vfork|putenv|lstat|setuid|setgid|getuid|getgid|seteuid|geteuid|fork|pthread_kill|pthread_cancel|pthread_exit)"
for FILE in $(find $CSRC -not -path $EXLUDE_CERT_VERIFIED_MANUALLY -not -path "*/pikeos/time/*" -name "*.c" -or -not -path "*/pikeos/time/*" -not -path "*/pikeos/p_time_c99.h" -name "*.h") ;
do
	RESULT=$(strip_c_comments_for_grep "${FILE}" | grep -nwiE $CHECK_CERT_RULE_ABSENCE)
	if ! [[ -z ${RESULT} ]] ; then
		echo "${FILE}:${RESULT}" | tee -a $LOGPATH
		CHECK_CERT_RULE_ABSENCE_FAILED=true
	fi
done
if $CHECK_CERT_RULE_ABSENCE_FAILED; then
    EXITCODE=1
    FAILURES+=("Forbidden CERT functions/headers found in sources: $CHECK_CERT_RULE_ABSENCE")
    echo "ERROR: checking absence of functions or headers: $CHECK_CERT_RULE_ABSENCE" | tee -a $LOGPATH
fi

#### Check absence of break outside switch ####
echo "Checking break keyword used only inside switch" | tee -a $LOGPATH
CHECK_BREAK_OUTSIDE_SWITCH_FAILED=false
CHECK_BREAK_ABSENCE='\bbreak\b'
for FILE in $(find $CSRC -name "*.c" -or -name "*.h") ;
do
	RESULT=$(strip_c_comments_for_grep "${FILE}" | strip_c_switch_blocks_for_grep | grep -nwiE $CHECK_BREAK_ABSENCE)
	if ! [[ -z ${RESULT} ]] ; then
		echo "${FILE}:${RESULT}" | tee -a $LOGPATH
		CHECK_BREAK_OUTSIDE_SWITCH_FAILED=true
	fi
done
if $CHECK_BREAK_OUTSIDE_SWITCH_FAILED; then
    EXITCODE=1
    FAILURES+=("Forbidden break outside switch found in sources")
    echo "ERROR: checking absence of break outside switch" | tee -a $LOGPATH
fi

CHECK_DIRECT_ABSENCE_FAILED=false
CHECK_DIRECT_ABSENCE="(printf)"
for FILE in $(find $CSRC -name "*.c" -or -name "*.h") ;
do
	# Preprocessor lines are kept to allow explicit overrides (e.g. #define printf ...)
	RESULT=$(strip_c_comments_for_grep "${FILE}" | sed 's/^ *#.*$//g' | grep -nwiE $CHECK_DIRECT_ABSENCE )
	if ! [[ -z "${RESULT}" ]] ; then
		echo "${FILE}:${RESULT}" | tee -a $LOGPATH
		CHECK_DIRECT_ABSENCE_FAILED=true
	fi
done

if $CHECK_DIRECT_ABSENCE_FAILED; then
    EXITCODE=1
    FAILURES+=("Forbidden direct functions found in sources: $CHECK_DIRECT_ABSENCE")
    echo "ERROR: checking absence of functions or headers: $CHECK_DIRECT_ABSENCE" | tee -a $LOGPATH
fi

#### Additionnal check for function, where we want to ensure that there is no false detection due to common wording.
CHECK_ABSENCE='(\bassert *[\(])'

echo "Checking specific functions not used in code" | tee -a $LOGPATH
find $CSRC -name "*.c" -or -name "*.h" | xargs grep -Ec "$CHECK_ABSENCE" | grep -Ec ":[^0]+" | xargs test 0 -eq
if [[ $? != 0 ]]; then
    echo "ERROR: checking absence of functions: $CHECK_ABSENCE" | tee -a $LOGPATH
    find $CSRC -name "*.c" -or -name "*.h" | xargs grep -nE "$CHECK_ABSENCE" | tee -a $LOGPATH
    EXITCODE=1
    FAILURES+=("Forbidden 'assert()' calls found in sources")
fi

CHECK_STD_MEM_ALLOC_ABSENCE="(\bfree\b\(|\bmalloc\b\(|\bcalloc\b\(|\brealloc\b\(|=.*\bfree\b|=.*\bmalloc\b|=.*\bcalloc\b|=.*\brealloc\b)"
EXCLUDE_STD_MEM_IMPLEM="*\/p_sopc_mem_alloc.c"

find $CSRC -not -path $EXCLUDE_STD_MEM_IMPLEM -name "*.c" | xargs grep -E $CHECK_STD_MEM_ALLOC_ABSENCE | grep -Ec ":[^0]+" | xargs test 0 -eq
if [[ $? != 0 ]]; then
    echo "ERROR: checking absence of std library use for memory allocation in toolkit" | tee -a $LOGPATH
    find $CSRC -not -path $EXCLUDE_STD_MEM_IMPLEM -name "*.c" | xargs grep -nE $CHECK_STD_MEM_ALLOC_ABSENCE | tee -a $LOGPATH
    EXITCODE=1
    FAILURES+=("Standard memory allocation functions (malloc/free/...) used in toolkit sources")
fi
find $TST -name "*.c" | xargs grep -E $CHECK_STD_MEM_ALLOC_ABSENCE | grep -Ec ":[^0]+" | xargs test 0 -eq
if [[ $? != 0 ]]; then
    echo "ERROR: checking absence of std library use for memory allocation in tests" | tee -a $LOGPATH
    find $TST -name "*.c" | xargs grep -nE $CHECK_STD_MEM_ALLOC_ABSENCE | tee -a $LOGPATH
    EXITCODE=1
    FAILURES+=("Standard memory allocation functions (malloc/free/...) used in test sources")
fi
find $DEMO -name "*.c" | xargs grep -E $CHECK_STD_MEM_ALLOC_ABSENCE | grep -Ec ":[^0]+" | xargs test 0 -eq
if [[ $? != 0 ]]; then
    echo "ERROR: checking absence of std library use for memory allocation in samples" | tee -a $LOGPATH
    find $DEMO -name "*.c" | xargs grep -nE $CHECK_STD_MEM_ALLOC_ABSENCE | tee -a $LOGPATH
    EXITCODE=1
    FAILURES+=("Standard memory allocation functions (malloc/free/...) used in sample sources")
fi

#### Check SOPC_GCC_DIAGNOSTIC_* macros are balanced ####
echo "Checking each SOPC_GCC_DIAGNOSTIC_IGNORE_* is matched by a SOPC_GCC_DIAGNOSTIC_RESTORE" | tee -a $LOGPATH
./.check-diagnostic-macros.py $CSRC $TST $DEMO 2>&1 | tee -a $LOGPATH
if [[ ${PIPESTATUS[0]} != 0 ]]; then
    EXITCODE=1
    FAILURES+=("Unbalanced SOPC_GCC_DIAGNOSTIC_IGNORE_*/SOPC_GCC_DIAGNOSTIC_RESTORE macros found in sources")
    echo "ERROR: checking SOPC_GCC_DIAGNOSTIC_* macros balance" | tee -a $LOGPATH
fi

#### Clang static analyzer ####
echo "Compilation with Clang static analyzer" | tee -a $LOGPATH
rm -fr build-analyzer && ./.run-clang-static-analyzer.sh 2>&1 | tee -a $LOGPATH

# Keep result
STATIC_ANALYSIS_STATUS=${PIPESTATUS[0]}

## Analyze C sources with clang-tidy ####

echo "Checking specific CERT rules using clang-tidy tool" | tee -a $LOGPATH
# CERT rules to verify
if [[ -z $ISADVANCED || $ISADVANCED != "advanced" ]]; then
    # remove default rules
    REMOVE_DEFAULT_RULES="-*,"
else
    # do not remove default rules
    echo "clang-tidy tool: keep default rules for advanced analysis of code" | tee -a $LOGPATH
    REMOVE_DEFAULT_RULES=""
fi

CERT_RULES=cert-flp30-c,cert-fio38-c,cert-env33-c,cert-err34-c,cert-msc30-c
# Define include directories
SRC_DIRS=(`find $CSRC -not -path "*windows*" -not -path "*freertos*" -not -path "*zephyr*" -not -path "*pikeos*" -type d`)
SRC_INCL=${SRC_DIRS[@]/#/-I}
# includes the generated export file
SRC_INCL="$SRC_INCL -Ibuild-analyzer/src/Common"
# mandatory include for CycloneCRYPTO
SRC_INCL="$SRC_INCL -I/usr/local/include/cyclone_crypto"
CLANG_TIDY_LOG=clang_tidy.log
# Run clang-tidy removing default checks (-*) and adding CERT rules verification
find $CSRC -not -path "*windows*" -not -path "*freertos*" -not -path "*zephyr*" -not -path "*uanodeset_expat*" -not -path "*pikeos*" -name "*.c" -exec clang-tidy {} -warnings-as-errors -header-filter=.* -checks=$REMOVE_DEFAULT_RULES$CERT_RULES -- $SRC_INCL -D_GNU_SOURCE -D__error_t_defined \; &> $CLANG_TIDY_LOG
# Check if resulting log contains error or warnings
grep -wiEc "(error|warning)" $CLANG_TIDY_LOG | xargs test 0 -eq
if [[ $? != 0 ]]; then
    echo "ERROR: checking CERT rules $CERT_RULES with clang-tidy: see log $CLANG_TIDY_LOG" | tee -a $LOGPATH
    # Note: for default checks the scan-build tool can be use to build the project (scan-build ./build.sh).
    #       It generates an HTML report providing diagnostics of the warning
    EXITCODE=1
    FAILURES+=("clang-tidy reported violations for CERT rules: $CERT_RULES (see $CLANG_TIDY_LOG)")
else
    \rm $CLANG_TIDY_LOG
fi

# Remove static analysis build since it is not necessary anymore
if [[ $STATIC_ANALYSIS_STATUS != 0 ]]; then
    EXITCODE=1
    FAILURES+=("Clang static analyzer reported errors (build-analyzer directory preserved for inspection)")
else
    rm -fr build-analyzer
fi

#### Format C sources with clang-format ####

echo "Clang automatic formatting check" | tee -a $LOGPATH
./.format.sh >> $LOGPATH
ALREADY_FORMAT=`git ls-files -m $BSRC $CSRC $TST $DEMO | grep -v bgenc`

if [[ -z $ALREADY_FORMAT ]]; then
    echo "C source code formatting already done" | tee -a $LOGPATH
else
    echo "ERROR: C source code code formatting not done or not committed" | tee -a $LOGPATH
    git diff | tee -a $LOGPATH
    EXITCODE=1
    FAILURES+=("clang-format found unformatted or uncommitted files: $ALREADY_FORMAT")
fi

#### Check license in files ####

echo "License in files verification" | tee -a $LOGPATH
./.license-check.sh >> $LOGPATH

if [[ $? != 0 ]]; then
    echo "ERROR: license in files verification failed, see $LOGPATH" | tee -a $LOGPATH
    EXITCODE=1
    FAILURES+=("License header missing or invalid in one or more files (see $LOGPATH)")
fi

#### Final summary ####

echo "" | tee -a $LOGPATH
echo "======================================" | tee -a $LOGPATH
if [[ $EXITCODE -eq 0 ]]; then
    echo "  Pre-build check: COMPLETED WITH SUCCESS" | tee -a $LOGPATH
else
    echo "  Pre-build check: COMPLETED WITH ERRORS" | tee -a $LOGPATH
    echo "  ${#FAILURES[@]} check(s) failed:" | tee -a $LOGPATH
    echo "" | tee -a $LOGPATH
    for i in "${!FAILURES[@]}"; do
        echo "  [$(( i + 1 ))] ${FAILURES[$i]}" | tee -a $LOGPATH
    done
    echo "" | tee -a $LOGPATH
    echo "  Full details available in: $LOGPATH" | tee -a $LOGPATH
fi
echo "======================================" | tee -a $LOGPATH

exit $EXITCODE
