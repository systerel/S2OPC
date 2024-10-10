The PyS2OPC module is a python interface to make OPC UA Clients and Servers.
It is based on the S2OPC library.
See https://gitlab.com/systerel/S2OPC/ for more information.

# Dependencies

- Python >= 3.9
- gcc (tested with 13.3.0)
- s2opc libraries and its dependencies
- Cython = 3.0.10
- autopxd2 = 2.3.0

# PyS2OPC

PyS2OPC has been created using AutoPXD+Cython to create a binding from S2OPC C library. It is based on the client/server frontend API of S2OPC.