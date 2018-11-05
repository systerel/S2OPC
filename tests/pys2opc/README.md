# A Python wrapper for the S2OPC Toolkit

See https://gitlab.com/systerel/S2OPC/ for more information.


## Dependencies

- Python >= 3.3 (and its development tools)
- setuptools
- gcc
- s2opc libraries and its dependencies, compiled with -fPIC:
  - libclient_subscription.a
  - libingopcs.a
  - libmbedcrypto.a
  - libmbedtls.a
  - libmbedx509.a

The following dependencies will be installed while installing PyS2OPC:
- CFFI >= 1.4
- pycparser

## Installation

### Using a virtualenv

A virtual environment is used to install dependencies in a container isolated from the OS' python installation.
[See virtual environment quickguide.](https://docs.python.org/3/tutorial/venv.html)

In the working directory of your choice:

```bash
python3 -m venv path-to-env-files
source path-to-env-files/bin/activate
```

The first command creates the Python virtual environment in a subfolder called `path-to-env-files`.
The second command must be executed to activate the environment and use it instead of the OS' python installation.

Once the virtual environment is activated, the PyS2OPC library can be installed.


### Installation

It is recommended to install this development version in the virtual environment:

```bash
pip install .
```


### Examples

```bash
cd examples
./0-read-write.py
./1-browse.py
./2-subscribe.py
./3-multi-connection-multi-request.py
```
