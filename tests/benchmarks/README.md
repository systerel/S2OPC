# Benchmark utilities for S2OPC

This directory holds a few utilities useful when benchmarking the performance of
the S2OPC server (and, to some extent, client):

## generate-nodeset

This script generates an arbitrary sized address space in an XML file. An
address space of size `N` is populated with boolean variables having a NodeId of
the form `ns=42;s=Objects.I` where `I` is a number from 0 to `N`-1.

## generate-ingopcs-address-space

This script generates a C representation of an XML address space suitable for
S2OPC. It essentially does the same as the normal address space generation
tools, but without mixing Makefiles, Bash, Python Mako templates and XSLT. More
importantly, it operates in linear time, which allows generating larger address
spaces in a reasonable time.

## bench_reads

This program gets compiled as part of normal builds and ends up in the `bin/`
directory along all the other binaries. It connects to a server on localhost on
port 4841 (the endpoint is hardcoded so far at the top of the file) with no
security and benchmarks the performance of ReadValue requests. The size of each
request is settable via the command line. The program will keep doing
measurements until the average time stabilizes enough that it is representative.

## Putting it all together

### Generating the address space and compiling the server

Let's assume we want to benchmark the performance of ReadValue requests on an
address space of size 10 000.

First, we generate the XML file of the address space:

```
./generate-nodeset 10000 address_space.xml
```

We then generate the corresponding C file (the build system expects this file to
be called `ingopcs_addspace.c`):

```
./generate-ingopcs-address-space address_space.xml ingopcs_addspace.c
```

Finally, we tell CMake where to find our address space, and compile it into the
server (from the build directory):

```
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DADDSPACEGEN_DIR=$(pwd)/../tests/benchmarks -DDISABLE_CHECK_ADDSPACE=1 ..
```

The command above disables the compilation of the address space check binary, as
we didn't generate one.

Finally, we compile our server:

```
make -j$(nproc)
```

### Running the benchmark

We're now ready to actually run the benchmark. Start the server from the `bin/`
directory:

```
./toolkit_test_server
```

And run the benchmark against it (reading 10 values in each ReadValue request):

```
./bench_reads 10000 10
```
