#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Licensed to Systerel under one or more contributor license
# agreements. See the NOTICE file distributed with this work
# for additional information regarding copyright ownership.
# Systerel licenses this file to you under the Apache
# License, Version 2.0 (the "License"); you may not use this
# file except in compliance with the License. You may obtain
# a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.


from setuptools import setup
import json
import os

setup(
    setup_requires=['cffi>=1.4.0'],
    install_requires=['cffi>=1.4.0'],
    cffi_modules=['pys2opc_build.py:ffibuilder'],
    packages=['pys2opc'],
    package_dir={'pys2opc': 'pys2opc'},
    package_data={'pys2opc': ['version.json']},

    name='pys2opc',
    version=json.load(open('pys2opc/version.json'))['version'],
    author='Systerel S2OPC',
    author_email='s2opc-support@systerel.fr',
    description='Python Wrapper for the S2OPC Toolkit',
    long_description=open('./README.md').read(),
    url='https://gitlab.com/systerel/S2OPC/',
    keywords='OPC OPC-UA S2OPC',
    classifiers=[
        'Programming Language :: Python :: 3',
        'Programming Language :: C',
        'License :: OSI Approved :: Apache Software License',
        'Operating System :: OS Independent',
        'Development Status :: 4 - Beta',
    ]
)
