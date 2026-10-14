#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

"""
Checks that the SOPC_GCC_DIAGNOSTIC_* macros of sopc_macros.h are balanced in C sources.

Each SOPC_GCC_DIAGNOSTIC_IGNORE_* (and the legacy SOPC_GCC_DIAGNOSTIC_PUSH) saves the diagnostic state
and shall be matched by one SOPC_GCC_DIAGNOSTIC_RESTORE in the same file. Reports, as "file:line: message":
- a RESTORE without a pending IGNORE/PUSH (rejected by clang: "pragma diagnostic pop could not pop"),
- an IGNORE/PUSH never restored (warning silently disabled until the end of the file),
- a conditional compilation block (#if/#ifdef/#ifndef ... #endif) whose branches leave a different number of
  pending IGNORE/PUSH, the implicit empty #else branch included: the macros would be unbalanced depending on the
  configuration. Each branch is checked from the state at the beginning of the block.

Comments, string literals and #define directives (including continuation lines) are ignored.

WARNING: this script has a test suite in an internal tools repository. Any modification of this script
shall be validated by running this test suite again.

Usage: .check-diagnostic-macros.py DIR_OR_FILE [DIR_OR_FILE ...]
Exit code: 0 if all files are balanced, 1 otherwise (including a non existing DIR_OR_FILE).
"""

import os
import re
import sys

# IGNORE_* and PUSH expand to a "#pragma GCC diagnostic push", RESTORE to a "pop"
MACRO_RE = re.compile(r'\bSOPC_GCC_DIAGNOSTIC_(PUSH|RESTORE|IGNORE_\w+)\b')
# Conditional compilation directives opening, splitting or closing a block
COND_RE = re.compile(r'^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b')
# Comments and string/char literals, replaced by spaces while keeping newlines (line numbers preserved)
STRIP_RE = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'', re.DOTALL)


def blank(match):
    """Replaces the matched text by spaces, keeping its newlines."""
    return re.sub(r'[^\n]', ' ', match.group(0))


def check_file(path):
    """Returns the errors of one file.

    Principle: replay the compiler diagnostic stack. Each IGNORE/PUSH pushes an entry on the 'pending' stack,
    each RESTORE pops one, so the stack contains the IGNORE not restored yet at the current line.
    """
    with open(path, encoding='utf-8', errors='replace') as f:
        # Blank comments and literals first: a macro name inside them is not a use
        content = STRIP_RE.sub(blank, f.read())
    errors = []
    pending = []  # stack of (macro, line) not restored yet
    # stack of conditional blocks: {line, start: pending at #if, ends: pending at end of each branch, has_else}
    blocks = []
    in_define = False  # inside a multi-line directive (line ending with a backslash)
    for lineno, line in enumerate(content.split('\n'), 1):
        # Preprocessor directives are not code: macros in a #define body are definitions, not uses
        is_directive = in_define or line.lstrip().startswith('#')
        # A continuation line is never a new directive, even if it starts with '#'
        cond = None if in_define else COND_RE.match(line)
        # The next line continues this directive if this one ends with a backslash
        in_define = is_directive and line.rstrip().endswith('\\')
        if cond is not None:
            # A conditional directive may replace the stack (each branch restarts from the block start)
            pending = check_conditional(path, lineno, cond.group(1), blocks, pending, errors)
        if is_directive:
            continue
        # Several macros may be on the same line: process them in order
        for match in MACRO_RE.finditer(line):
            macro = match.group(1)
            if macro != 'RESTORE':
                # IGNORE_* or PUSH: push, keeping the line to report it if never restored
                pending.append((macro, lineno))
            elif pending:
                # RESTORE: pop the last pending IGNORE
                pending.pop()
            else:
                # RESTORE with an empty stack: clang error "pragma diagnostic pop could not pop"
                errors.append('{}:{}: SOPC_GCC_DIAGNOSTIC_RESTORE without matching SOPC_GCC_DIAGNOSTIC_IGNORE_*'
                              .format(path, lineno))
    # At end of file, nothing shall remain open
    for block in blocks:  # #if without #endif
        errors.append('{}:{}: conditional block not closed by #endif'.format(path, block['line']))
    for macro, lineno in pending:  # IGNORE whose warning stays disabled until the end of the file
        errors.append('{}:{}: SOPC_GCC_DIAGNOSTIC_{} not restored by SOPC_GCC_DIAGNOSTIC_RESTORE'
                      .format(path, lineno, macro))
    return errors


def check_conditional(path, lineno, directive, blocks, pending, errors):
    """Handles a conditional compilation directive and returns the pending stack to use after it.

    Principle: only one branch of a block is compiled, so each branch is replayed from the stack at #if,
    and all branches shall end with the same depth, otherwise the balance depends on the configuration.
    A block without #else has an implicit empty branch, which leaves the stack as at #if.
    """
    if directive in ('if', 'ifdef', 'ifndef'):
        # Opening: save a copy of the stack, the first branch continues with the current one
        blocks.append({'line': lineno, 'start': list(pending), 'ends': [], 'has_else': False})
        return pending
    if not blocks:
        # #elif/#else/#endif outside of any block (malformed file): report and ignore it
        errors.append('{}:{}: #{} without matching #if'.format(path, lineno, directive))
        return pending
    # Innermost open block: nested blocks are closed before their parent
    block = blocks[-1]
    block['ends'].append(pending)  # end of the current branch
    if directive in ('elif', 'else'):
        # New branch: restart from a copy of the stack saved at #if
        block['has_else'] = block['has_else'] or directive == 'else'
        return list(block['start'])
    # #endif: close the block and compare its branches
    blocks.pop()
    if not block['has_else']:
        # No #else: add the implicit empty branch (stack unchanged since #if)
        block['ends'].append(block['start'])
    # Depth of each branch = number of IGNORE still pending at its end. All branches shall have the same depth,
    # i.e. one distinct value. Only depths matter for the push/pop balance, not which IGNORE is pending, e.g.:
    #   balanced:   #if A  IGNORE_CAST_CONST  #else  IGNORE_CAST_ALIGN  #endif  RESTORE  -> depths [1, 1]
    #   unbalanced: #if A  IGNORE_CAST_CONST  #endif  RESTORE  -> depths [1, 0] (implicit #else branch):
    #               when A is not defined, RESTORE has nothing to pop
    depths = [len(end) for end in block['ends']]
    if len(set(depths)) > 1:
        errors.append('{}:{}: branches of this conditional block leave a different number of pending '
                      'SOPC_GCC_DIAGNOSTIC_IGNORE_* ({})'.format(path, block['line'], ', '.join(map(str, depths))))
    # Continue with the first branch (equivalent to the others when balanced)
    return list(block['ends'][0])


def c_files(paths):
    for path in paths:
        if os.path.isfile(path):
            yield path
            continue
        for root, _, files in os.walk(path):
            for name in sorted(files):
                if name.endswith(('.c', '.h')):
                    yield os.path.join(root, name)


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1
    # A missing path is an error: otherwise a renamed directory would silently pass the check
    errors = ['{}: no such file or directory'.format(path) for path in argv[1:] if not os.path.exists(path)]
    for path in c_files(argv[1:]):
        errors.extend(check_file(path))
    for error in errors:
        print(error)
    return 1 if errors else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
