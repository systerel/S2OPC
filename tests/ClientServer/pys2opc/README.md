# A Python wrapper for the S2OPC Toolkit

See https://gitlab.com/systerel/S2OPC/ for more information.


## Dependencies

- Python >= 3.3
- Python development tools (package `python3-dev` under Ubuntu)
- setuptools (tested with version >= 38)
- gcc (tested with 4.8.5)
- s2opc libraries and its dependencies, compiled with -fPIC:
  - libclient_subscription.a
  - libs2opc.a
  - libmbedcrypto.a
  - libmbedtls.a
  - libmbedx509.a

The following dependencies will be installed while installing PyS2OPC:
- CFFI >= 1.4
- pycparser


