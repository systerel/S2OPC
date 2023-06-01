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

import difflib
import os
from os.path import join as pj
import shutil
import unittest

from nodeset_address_space_utils import run_merge, make_argparser


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
RESOURCE_DIR = pj(SCRIPT_DIR, 'resources')

# remove the temporary files produced during the tests
# set to False to keep the actual files and analyse them
CLEANUP_TEMP = True


def make_temp_dir():
    tmp_dir = pj(RESOURCE_DIR, 'actual')
    if not os.path.isdir(tmp_dir):
        os.mkdir(tmp_dir)
    return tmp_dir


class MergeTests(unittest.TestCase):

    def assert_content_equal(self, expected, actual):
        with open(expected, encoding='utf-8') as exp_file, open(actual, encoding='utf-8') as act_file:
            exp = exp_file.readlines()
            act = act_file.readlines()
            if exp != act:
                d = difflib.Differ()
                self.fail(''.join(d.compare(exp, act)))

    def run_test(self, result_name, options, *src_paths):
        parser = make_argparser()
        # TODO: with tempfile.TemporaryDirectory() as tmp_dir:
        tmp_dir = make_temp_dir()
        try:
            expected = pj(RESOURCE_DIR, 'expected', result_name)
            actual = pj(tmp_dir, result_name)
            res_src_paths = [pj(RESOURCE_DIR, src) for src in src_paths]
            args = parser.parse_args([*options, *res_src_paths, '-o', actual])
            run_merge(args)
            self.assert_content_equal(expected, actual)
        finally:
            if CLEANUP_TEMP:
                shutil.rmtree(tmp_dir)

    def test_ns0_alone(self):
        self.run_test('test_ns0_alone.xml', [],
                      'ns0.xml')

    def test_merge_ns0_temperature(self):
        self.run_test('test_merge_ns0_temperature.xml', [],
                      'ns0.xml', 'TestTemperatureNS.NodeSet2.xml')

    def test_merge_ns0_temperature_pressure(self):
        self.run_test('test_merge_ns0_temperature_pressure.xml', [],
                      'ns0.xml', 'TestTemperatureNS.NodeSet2.xml', 'TestPressureNS.NodeSet2.xml')

    def test_merge_pressure_with_ref_to_temperature(self):
        self.run_test('test_merge_pressure_with_ref_to_temperature.xml', [],
                      'ns0.xml', 'TestTemperatureNS.NodeSet2.xml', 'TestPressureNS_with_TemperatureNS.NodeSet2.xml')
    
    def test_merge_with_ns_srv_arrays(self):
        self.run_test('test_merge_with_ns_srv_arrays.xml', [],
                      'ns0.xml', 'TestTemperatureNS_with_ns_srv_arrays.NodeSet2.xml', 'TestPressureNS_with_ns_srv_arrays.NodeSet2.xml')

    def test_merge_with_ns_srv_arrays2(self):
        # pressure is given before temperature
        self.run_test('test_merge_with_ns_srv_arrays2.xml', [],
                      'ns0.xml', 'TestPressureNS_with_ns_srv_arrays.NodeSet2.xml', 'TestTemperatureNS_with_ns_srv_arrays.NodeSet2.xml')

    def test_remove_subtree(self):
        self.run_test('test_remove_subtree.xml', ['--remove-subtree', 'ns=1;i=15009'],
                      'ns0.xml', 'TestTemperatureNS.NodeSet2.xml')

    def test_remove_unused(self):
        self.run_test('test_remove_unused.xml', ['--remove-unused'],
                      'ns0.xml', 'TestUnusedTypes.NodeSet2.xml')


if __name__ == '__main__':
    unittest.main()
