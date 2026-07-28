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

## bench_nodeid_dict

Micro-benchmark for `SOPC_NodeId_Dict_Create` lookup performance. It fills a
dictionary with 500k NodeIds (by default) and measures the time to perform
millions of lookups. It compares a DJB baseline, the current production hash
(`SOPC_NodeId_Hash`) and alternative hash functions (native FNV-1a, numeric
fast-path). The DJB baseline walks the NodeId fields itself rather than calling
`SOPC_NodeId_Hash`, so the reference column keeps its meaning when the
production hash changes.

Lookup timings are repeated 5 times; both the median and the best run are
reported.

Before timing, a validation pass checks:
- all NodeIds in the key set are distinct
- hash stability and equal-key consistency for each variant
- chi-square of the low hash bits over 4096 classes
- probe-chain simulation (with table saturation detection)
- hash-value collision count among distinct keys
- a portable reference hash (canonical FNV-1a on fixed-endian encoding)

Reading the results: the dictionary indexes buckets with
`(hash + f(i)) & (size - 1)`, so only the low bits of the hash matter. The
meaningful quality criteria are therefore `chi2/dof` (1.0 means as uniform as a
random mapping, well above 1 means an unevenly loaded bucket array) and the
probe statistics. The collision count is a sanity check only: for 500k distinct
keys an ideal 64-bit hash is expected to produce about 7e-9 colliding pairs, so
zero collisions is the normal outcome even for a poor hash and cannot be used to
rank candidates.

Example:

```
./bench_nodeid_dict
./bench_nodeid_dict -p numeric
./bench_nodeid_dict -n 500000 -l 10000000 -p string
```

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
