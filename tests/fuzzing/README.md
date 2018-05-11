# Fuzzing tests for S2OPC

Each fuzzing test is fuzzing a specific aspect of the library (for example the
decoding of binary messages). A fuzzing test is defined by a C function that
runs on an input buffer, and returns 0 on success.

Fuzzing tests are best run with ASan and UBSan enabled, so that invalid memory
accesses or undefined behaviors that would not normally cause a crash are caught
too.

Each fuzzing test generates two binaries, one version linked against LibFuzzer,
and one linked against the standalone test runner in `standalone_fuzzer.c`. The
first one is used to run the actual fuzzing (starting from a base corpus, and
enriching it as fuzzing progresses), while the second one is used to run the
test on one or more specific corpus items, to analyze a crash or to make
coverage reports. The LibFuzzer binaries have a name ending with `.libfuzzer`,
while the standalone ones have a name ending with `.standalone`.

Compiling fuzzing tests against LibFuzzer requires CLang 6.0 or higher. Other
fuzzing engines like AFL have not been investigated yet.

## Running fuzzing tests

Let's first compile two versions of S2OPC, one with ASan+UBsan (which we'll use
to run the fuzzing), and one with source code coverage enabled (to see what our
fuzzing corpus actually covers). From the source directory, run:

```
mkdir build.san build.cov

cd build.san && \
CC=clang cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_ASAN=1 -DWITH_UBSAN=1 .. && \
make && \
cd ..

cd build.cov && \
CC=clang cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_CLANG_SOURCE_COVERAGE=1 .. && \
make && \
cd ..
```

We also need to get the fuzzing corpuses, by cloning https://gitlab.com/systerel/S2OPC-fuzzing-data
somewhere.

We can now run a fuzzing test by running (assuming a 4-CPU machine)

```
./build.san/bin/server_request_fuzzer.libfuzzer -jobs=4 -workers=4 \
  /path/to/S2OPC-fuzzing-data/server_request
```

The workers will report their progress into separate log files, and write any
input vector causing a crash or a timeout in the current working directory.
LibFuzzer will also create new files in the corpus directory for any discovered
input vector that trigger new behaviors.

After a while, we can also check the coverage of our fuzzing corpus by running
the standalone version of the test against all the input vectors:

```
./build.cov/bin/server_request_fuzzer.standalone /path/to/S2OPC-fuzzing-data/server_request/*
```

This will generate a `default.profraw` file holding the coverage data. This file
can be analyzed using the appropriate LLVM tools:

```
llvm-profdata merge -sparse *.profraw -o default.profdata

# Show line by line coverage of the SC_Chunks_TreatTcpPayload function in the
# terminal
llvm-cov show ./build.cov/bin/server_request_fuzzer.standalone \
  -instr-profile=default.profdata -name=SC_Chunks_TreatTcpPayload
```

## Writing fuzzing tests

A fuzzing test is a simple C file defining a function `int
LLVMFuzzerTestOneInput(const uint8_t* buf, size_t len)`. This function should
run the test against the input buffer, and return 0 on success. This function is
then linked either against LibFuzzer or the standalone harness (see the
definitions for `server_request_fuzzer` in `CMakeLists.txt`).

## More information

The following links give valuable information about LibFuzzer etc:

- https://llvm.org/docs/LibFuzzer.html
- https://github.com/google/fuzzer-test-suite/blob/master/tutorial/libFuzzerTutorial.md
- https://llvm.org/docs/CommandGuide/llvm-cov.html
