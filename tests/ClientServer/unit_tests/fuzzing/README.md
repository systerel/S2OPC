# Fuzzing tests for S2OPC

Each fuzzing test is fuzzing a specific aspect of the library (for example the
decoding of binary messages). A fuzzing test is defined by a C function that
runs on an input buffer, and returns 0 on success.

Fuzzing tests are best run with ASan and UBSan enabled, so that invalid memory
accesses or undefined behaviors that would not normally cause a crash are caught
too.

Each fuzzing test generates one binary with two main functions. If given a
directory, the fuzzer will run the actual fuzzing (starting from a base corpus
inside the directory, and enriching it as the fuzzing progresses). If given one 
or multiple files, it will simply run those cases and stop after. This mode is 
useful to analyse a crash or to make coverage reports. You can also pass the 
flag `-help=1` to a fuzzer to see every option possible.

Compiling fuzzing tests against libFuzzer requires CLang 6.0 or higher (libFuzzer is embedded in CLang starting from 6.0.0).
Other fuzzing engines like AFL have not been investigated yet.

Once a corpus has been made with representative test cases,
the goal is to use OSS-Fuzz, the continuous Fuzzing for Open Source Software.


# libFuzzer (local or in docker check)

## Running fuzzing tests

Let's first compile three versions of S2OPC, one with ASan+UBsan (which we'll 
use to run the fuzzing), one with source code coverage enabled (to see what our
fuzzing corpus actually covers), and one in debug mode (to analyse a crash with
tools like `gdb`). From the cloned directory, run:

```
# Only the check docker has Clang
./.check-in-docker.sh "

# the fuzzing one
mkdir build.san 
cd build.san
CC=clang CFLAGS=-fsanitize=fuzzer-no-link cmake -DENABLE_FUZZING=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_ASAN=1 -DWITH_UBSAN=1 ..
make
cd ..

# the coverage one
mkdir build.cov
cd build.san
CC=clang cmake -DENABLE_FUZZING=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWITH_CLANG_SOURCE_COVERAGE=1 ..
make
cd ..

# the debug one
mkdir build.san
cd build.san
CC=clang cmake -DENABLE_FUZZING=ON -DCMAKE_BUILD_TYPE=Debug ..
make
cd ..
"
```

We also need to get the fuzzing corpuses, by cloning https://gitlab.com/systerel/S2OPC-fuzzing-data
somewhere.

We can now run a fuzzing test by running (assuming a 4-CPU machine)

```
./build.san/bin/server_request_fuzzer -jobs=4 -workers=4 \
  /path/to/S2OPC-fuzzing-data/server_request
```

The workers will report their progress into separate log files, and write any
input vector causing a crash or a timeout in the current working directory.
LibFuzzer will also create new files in the corpus directory for any discovered
input vector that trigger new behaviors.

After a while, we can also check the coverage of our fuzzing corpus by running
the fuzzer against all the input vectors:

```
./build.cov/bin/server_request_fuzzer /path/to/S2OPC-fuzzing-data/server_request/*
```

This will generate a `default.profraw` file holding the coverage data. This file
can be analyzed using the appropriate LLVM tools:

```
./check-in-docker.sh "llvm-profdata merge -sparse *.profraw -o default.profdata"

# Show line by line coverage of the SC_Chunks_TreatTcpPayload function in the
# terminal
./check-in-docker.sh "llvm-cov show ./build.cov/bin/server_request_fuzzer.standalone
  -instr-profile=default.profdata -name=SC_Chunks_TreatTcpPayload"
```

## Writing fuzzing tests

A fuzzing test is a simple C file defining a function `int
LLVMFuzzerTestOneInput(const uint8_t* buf, size_t len)`. This function should
run the test against the input buffer, and return 0 on success. This function is
then linked against LibFuzzer (see the definitions for `server_request_fuzzer` 
in [`CMakeLists.txt`](../../CMakeLists.txt)).

## More information

The following links give valuable information about LibFuzzer etc:

- https://llvm.org/docs/LibFuzzer.html
- https://github.com/google/fuzzer-test-suite/blob/master/tutorial/libFuzzerTutorial.md
- https://llvm.org/docs/CommandGuide/llvm-cov.html


# OSS-Fuzz (distributed)

Most of this section has been written from the online documentation of [OSS-Fuzz](https://github.com/google/oss-fuzz).
**Overview** of the process to use OSS-Fuzz (detailed in the latter sections):

- Prepare fuzz targets: the functions/scenarios that are tested, their compilation, and the test corpus.
  This work is done in the S2OPC repository.
- Configure OSS-Fuzz to fuzz our fuzz targets.
  OSS-Fuzz builds a docker with a clone of our current sources, its compilers and their options, and its fuzz libraries,
  then distributes the binaries, then runs it (on another docker), then informs us of the results, then repeat.
  There is a `build.sh` script that runs in the docker to build our sources.
  These files are in the OSS-Fuzz repository.
  Don't forget copyright headers.
  Wait before sending a pull request.
- Test locally: we can build and run the same docker, so do it.
  OSS-Fuzz has helpers for that too.
- Push and wait for the fuzzers to run.
- Fetch back the new corpus.


## Configure S²OPC

This work is done in the S²OPC repository.

- Identify functions to be tested.
  The fuzzer calls the function `int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)`.
  The fuzzer gives an input buffer and its size.
  The `LLVMFuzzerTestOneInput` use it as a test input.
  The fuzzer changes something in the input, and checks whether new parts of the code is covered by the input.
  It keeps doing so, until something breaks.
  (The tested code should be fast (runs ~ 1000Hz)).
- Make a binary with a `LLVMFuzzerTestOneInput()` implementation for each of the tested features.
  To be homogeneous, this binary should be built with a rule suffixed by `_fuzzer`.
- Important: only link statically, as the build docker that we create with OSS-Fuzz is not the docker that will run our targets.
- Important: things are running in `/out/` in the runner docker,
  and it is [read-only](https://github.com/google/oss-fuzz/blob/master/docs/fuzzer_environment.md#file-system).
  But `/tmp/` is writable.
- Build a corpus: identify different base inputs that are valid inputs for the fuzzed functions.
  These inputs are just files.
- Important: the corpus may not reside on the S²OPC repository,
  but in the configuration phase of OSS-Fuzz, the `build.sh` script must be able to access them.
  So it may reside in another git.


## Configure OSS-Fuzz

This is a one time configuration, and adding new fuzzers should not change this configuration.
This is done in the OSS-Fuzz repository, in `projects/s2opc`.

- Add a `project.yaml` to oss-fuzz repo
  ([initial pull request](https://github.com/google/oss-fuzz/pull/2133),
  [doc](https://github.com/google/oss-fuzz/blob/master/docs/new_project_guide.md#projectyaml)).

    - Set who we are,
    - Choose sanitizers.

- Add a `Dockerfile`: clone our sources, install our build dependencies.
  This docker is dedicated to build our sources.
  Binaries will not run on this one.
  Add copyright header.
- Add a `build.sh`.
  This one can clone and build our dependencies.
  It calls everything required and put fuzz targets in `$OUT`.
  This includes preparing the corpus as a `.zip`.
  Add copyright header.
- There should not be anything else here, as sources of the fuzz targets are in the S2OPC repository.
- [Test it locally](https://github.com/google/oss-fuzz/blob/master/docs/new_project_guide.md#testing-locally)
  (and see next session) before pushing it.
- Fork and make a pull request and push it to OSS-Fuzz.


## Testing/Running/Coverage locally

The OSS' `infra/helper.py` script can do a lot of interesting things.
First, it builds the required docker image and the fuzzers (see [the already mentioned guide](https://github.com/google/oss-fuzz/blob/master/docs/new_project_guide.md#testing-locally)).
This is done with `build_image`, `build_fuzers` and `shell` commands of `infra/helper.py` which are clearly explained in the guide.
However, two notes:

- Warning, on my machine the commands `build_fuzzer` and `shell` don't have the same `CXXFLAGS`,
  even when using the `--sanitizer` flag, hence there were link bugs.
  Pay attention to `CXXFLAGS`, which must include the `-fsanitize=[...]` part.
- Warning, on my machine, the commands `docker run --cap-add SYS_TRACE` and
  `docker run --priviledged` do not both enable `ptrace`.
  Only the latter.
  Hence `infra/helper.py build_fuzzer`, which uses the `--cap-add` version, is not able to `./configure` libcheck.
  Either replace the `--cap-add` argument in the script, or start the shell as follows:
  ```bash
  $ python infra/helper.py shell --sanitizer [...] s2opc
  # export ENV_SANITIZER=SANITIZER_FLAGS_$SANITIZER; export CFLAGS="$CFLAGS ${!ENV_SANITIZER}";  export CXXFLAGS="$CXXFLAGS ${!ENV_SANITIZER}"
  # compile
  ```

OSS-Fuzz requires us to pass the `check_build` on both `build_fuzzers --sanitize address` and `build_fuzzer--sanitize undefined`.
However, the test may fail indicating "seems to have only partial coverage instrumentation".
It may be because the tested code is too small.
The information is displayed when running the fuzzer, e.g. with 82 counters instead of at least 100:
```log
INFO: Loaded 1 modules   (82 inline 8-bit counters): 82 [0x7c3f90, 0x7c3fe2),
```

Other interesting modes are the `run_fuzzer` and `coverage`.
With supplementary options, it is possible to run locally the fuzzer and save it, or merge existing corpora.
The `build/out/s2opc` folder is mapped to `/out` in the docker container, so it is possible to save the corpus
(note, the zipped corpus is also used in this case):
```bash
mkdir build/out/s2opc/corpus_dir
python infra/helper.py run_fuzzer s2opc parse_tcp_uri_fuzzer /out/corpus_dir
```
Or to merge corpora
(note, the zipped corpus is also used in this case):
```bash
mkdir build/out/s2opc/new_corpus
python infra/helper.py run_fuzzer s2opc parse_tcp_uri_fuzzer /out/new_corpus -merge=1 /out/old_corpus
```

Finally, the `coverage` is used to obtain an interactive coverage web server.
To start it, the fuzzers must be built with `--sanitize coverage`,
and the corpus must be made accessible to the docker container:
```bash
# Existing corpus available in build/out/s2opc/corpus_dir
rm -r build/work/s2opc  # Dependencies must also be re-built
python infra/helper.py build_fuzzers --sanitizer coverage s2opc
python infra/helper.py coverage --fuzz-target parse_tcp_uri_fuzzer --corpus-dir build/out/s2opc/parse_tcp_uri s2opc
```
The latter starts a server on `localhost:8008` by default.


## Fetch the new corpus

As the fuzzer explores new test case, it expands the input corpus.
We can [get it](https://github.com/google/oss-fuzz/blob/master/docs/corpora.md).
It can then be merged to reduce its size, and added to our own corpus.
It requires a Google account that is mentioned in our `project.yaml`.


[modeline]: # ( vim: set syntax=markdown spell spelllang=en: )
