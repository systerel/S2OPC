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

from setuptools import Distribution, setup

import json
import sys

class BinaryDistribution(Distribution):
    def has_ext_modules(pys2opc):
        return True
    def is_pure(self):
        return False

library_extension = {
    'linux': 'so',
    'win32': 'pyd'
}.get(sys.platform, 'so')

setup(
    packages=['pys2opc'],
    package_data={'pys2opc': ['pys2opc.'+library_extension, 'version.json', 'pys2opc.pyx'] },
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
    ],
    distclass=BinaryDistribution
)
