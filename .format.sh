#!/bin/bash
# Script to format code using clang-format

SRCS_DIR=.

find $SRCS_DIR -name "*.c" -or -name "*.h" | xargs clang-format -style=file -i
