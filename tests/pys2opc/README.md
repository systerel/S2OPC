# A Python wrapper for the S2OPC Toolkit

See https://gitlab.com/systerel/S2OPC/ for more information.


## Dependencies

- Python >= 3.3
- Python development tools (package `python3-dev` under Ubuntu)
- setuptools (tested with version >= 38)
- gcc (tested with 4.8.5)
- s2opc libraries and its dependencies, compiled with -fPIC:
  - libclient_subscription.a
  - libingopcs.a
  - libmbedcrypto.a
  - libmbedtls.a
  - libmbedx509.a

The following dependencies will be installed while installing PyS2OPC:
- CFFI >= 1.4
- pycparser


## Using the docker image

### Installing the docker image

The following command pulls the `pys2opc` docker image on your machine from the file:
```bash
docker load -i pys2opc-0.0.1.docker.tar
```

The docker comes pre-installed with Python 3.4 and the current PyS2OPC version.


### Running the server

Find the IP of the docker:
```bash
ifconfig  # Or
ifconfig docker0
```

This gives the address of the docker subnet.
The server will probably listen on `2` on this subnetwork (e.g. `172.17.0.2`)

Start the server in docker:
```bash
docker run -it --rm --volume $PWD:$PWD --workdir /bin-s2opc --entrypoint /bin/bash pys2opc -c "TEST_SERVER_XML_ADDRESS_SPACE=/etc/pys2opc/s2opc_cnes.xml ./toolkit_test_server"
```


### Configuring and running the clients

Change the default server address and the defaults paths for the examples:
```bash
cd examples
sed 's/localhost/172.17.0.2/' -i _connection_configuration.py  # Change the IP as needed
sed 's,../../build/bin,/bin-s2opc,' -i _connection_configuration.py
```

Start any of the clients in the docker:
```bash
docker run -it --rm --volume $PWD:$PWD --workdir $PWD --entrypoint /bin/bash pys2opc -c ./0-read-write.py
docker run -it --rm --volume $PWD:$PWD --workdir $PWD --entrypoint /bin/bash pys2opc -c ./1-browse.py
docker run -it --rm --volume $PWD:$PWD --workdir $PWD --entrypoint /bin/bash pys2opc -c ./2-subscribe.py
docker run -it --rm --volume $PWD:$PWD --workdir $PWD --entrypoint /bin/bash pys2opc -c ./3-multi-connection-multi-request.py
```

## Installation in a virtual environment

### Creating the virtual environment and installing PyS2OPC

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

```bash
pip install .
```


### Running the server

From pys2opc base folder:

```bash
cd bin-s2opc
TEST_SERVER_XML_ADDRESS_SPACE=../s2opc_cnes.xml ./toolkit_test_server
```

### Configuring and running the examples

Configuring the secure connections:
```bash
cd examples
sed 's,../../build/bin,../bin-s2opc,' -i _connection_configuration.py
```

Clients:
```bash
./0-read-write.py
./1-browse.py
./2-subscribe.py
./3-multi-connection-multi-request.py
```
