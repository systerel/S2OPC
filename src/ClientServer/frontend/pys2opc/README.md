The PyS2OPC module is a python interface to make OPC UA Clients and Servers.
It is based on the S2OPC library.
See https://gitlab.com/systerel/S2OPC/ for more information.


# Dependencies

All the dependencies are in the S2OPC build docker,
and a python wheel for Py3.6 is produced by the build process.

- Python >= 3.3
- Python development tools (package `python3-dev` under Ubuntu)
- setuptools (tested with version >= 38)
- gcc (tested with 4.8.5)
- s2opc libraries and its dependencies, compiled with `-fPIC`:
  - libs2opc_clientwrapper.a
  - libs2opc.a
  - libmbedcrypto.a
  - libmbedtls.a
  - libmbedx509.a

The following dependencies will be installed while installing PyS2OPC:

- CFFI >= 1.4
- pycparser

To manually produce a wheel package, `pip3 install wheel` then `python3 setup.py bdist_wheel`

To produce the docs, install `pip3 install pdoc3` then `pdoc pys2opc --http :`.
Then documentation can then be browsed at http://localhost:8080/.
Otherwise, `pdoc pys2opc --html` will produce portable html to `html/`.
The main page is `html/pys2opc/index.html`.
The path can be changed with `-o` or `--out-dir` option given to `pdoc`.


# API changes

## From 0.0.x to 0.1.0

- `pys2opc.PyS2OPC` is now split into two halves: `PyS2OPC_Client` and `PyS2OPC_Server`
- `BaseConnectionHandler` is renamed to `BaseClientConnectionHandler`
