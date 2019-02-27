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
from setuptools.command.build_ext import build_ext
from distutils.file_util import copy_file
import json
import os


class BuildExtWithCopy(build_ext):
    """Build the extension library as _pys2opc.xxx.so and copy it to _pys2opc.so so that cmake can use it more easily"""
    def run(self):
        super().run()
        for ext in self.extensions:
            basename = self.get_ext_fullname(ext.name)
            filename = self.get_ext_filename(basename)
            src_fname = os.path.join(self.build_lib, filename)
            dst_fname = os.path.join(self.build_lib, '.'.join((basename, filename.split('.')[-1])))
            copy_file(src_fname, dst_fname, verbose=self.verbose, dry_run=self.dry_run)


setup(
    setup_requires=['cffi>=1.4.0'],
    install_requires=['cffi>=1.4.0'],
    cffi_modules=['pys2opc/pys2opc_build.py:ffibuilder'],
    packages=['pys2opc'],
    package_dir={'pys2opc': 'pys2opc'},
    package_data={'pys2opc': ['version.json']},
    cmdclass={'build_ext': BuildExtWithCopy},

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
