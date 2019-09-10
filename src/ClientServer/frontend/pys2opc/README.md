# A Python wrapper for the S2OPC Toolkit

See https://gitlab.com/systerel/S2OPC/ for more information.


## Dependencies

- Python >= 3.3
- Python development tools (package `python3-dev` under Ubuntu)
- setuptools (tested with version >= 38)
- gcc (tested with 4.8.5)
- s2opc libraries and its dependencies, compiled with -fPIC:
  - libs2opc_clientwrapper.a
  - libs2opc.a
  - libmbedcrypto.a
  - libmbedtls.a
  - libmbedx509.a

The following dependencies will be installed while installing PyS2OPC:
- CFFI >= 1.4
- pycparser


## Build and distribute a pre-compiled wheel

The Python module `wheel` can be installed with `pip` to provide a pre-compilation feature to the existing `setup.py` script:

```bash
pip3 install wheel
python3 setup.py bdist_wheel
```

The `.whl` produced in `dist/` can be later installed as `pip3 install pys2opc*.whl` to avoid the recompilation of the module.


## Build the documentation

The Python module `pdoc3` (requires Python3.5+) is used to build an html or pdf version of the documentation for this module.

```bash
pip3 install pdoc3
pdoc3 pys2opc --html
```

The html is portable and produced to `html/`.
The main page is `html/pys2opc/index.html`.
The path can be changed with `-o` or `--out-dir` option given to `pdoc3`.


[modeline]: # ( vim: set syntax=markdown spell spelllang=en: )
