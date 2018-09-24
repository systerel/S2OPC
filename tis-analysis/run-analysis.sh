#!/usr/bin/env bash

set -Eeuo pipefail

function check_file_exists {
    test -e "$1" || { echo "ERROR: missing file '$1'"; exit 1; }
}

function run_test {
    local _stdout="results/$1.log"
    local _log="results/$1_check.log"
    local _oracle="results/$1_check.oracle"
    echo -e "\n\nRUNNING ANALYSIS: $1"
    if make CHECK_DIR=/builds/check MBEDTLS_DIR=/builds/mbedtls "$_log"; then
        check_file_exists "$_log"
        check_file_exists "$_oracle"
        local _diff=$(diff "$_log" "$_oracle" || true)
        if test -z "$_diff"
        then echo "OK."
        else
	    echo "DIFFERENCES DETECTED:"
            echo "    - ANALYZER OUTPUT:"
            cat "$_stdout"
            echo "    - DIFFERENCES:"
	    echo "$_diff"
	    echo "ANALYSIS $1 FAILED. ABORT."
	    exit 1
        fi
    else
        echo "FAILURE:"
        cat "$_stdout"
        echo "ANALYSIS $1 FAILED. ABORT."
        exit 1
    fi
}

analysis=(
    ti_addspace
    ti_test_read
    ti_test_write
    addspace
    test_read
    test_write
#   helpers  (TODO)
#   tk_client (TODO)
#   sch_client (TODO)
#   sockets (TODO)
)

for a in "${analysis[@]}"; do run_test "$a"; done
