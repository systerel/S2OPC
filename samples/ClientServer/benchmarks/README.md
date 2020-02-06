# Benchmark utilities for S2OPC

This directory holds a few utilities useful when benchmarking the performance of
the S2OPC server (and, to some extent, client):

## generate-nodeset

This script generates an arbitrary sized address space in an XML file. An
address space of size `N` is populated with boolean variables having a NodeId of
the form `ns=1;s=Objects.I` where `I` is a number from 0 to `N`-1.

## bench_tool

This program gets compiled as part of normal builds and ends up in the `bin/`
directory along all the other binaries. It connects to a server on localhost on
port 4841 (the endpoint is hardcoded so far at the top of the file) with no
security and benchmarks the performance of various kind of requests. The size of
each request is settable via the command line. The program will keep doing
measurements until the average time stabilizes enough that it is representative.

## Putting it all together

### Generating the address space

Let's assume we want to benchmark the performance of ReadValue requests on an
address space of size 10 000.

First, we generate the XML file of the address space:

```
./generate-nodeset.py 10000 address_space.xml
```

### Running the benchmark

We're now ready to actually run the benchmark. Make sure that the XML address
space loader was compiled with S2OPC: there should be a file named
`libs2opc-loader-uanodeset-expat.a` in your build directory. If that file is
not present, make sure the Expat library and its development headers are found
by CMake at configuration time.

Start the server from `bin/` in the build directory:

```
TEST_SERVER_XML_ADDRESS_SPACE=../tests/benchmarks/address_space.xml ./toolkit_test_server
```

And run the benchmark against it (reading 10 values in each ReadValue request):

```
./bench_tool read 10000 10
```
