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


ERROR = 'ERROR: '


class MergeTests(unittest.TestCase):

    def setUp(self)->None:
        self.tmp_dir = make_temp_dir()

    def tearDown(self):
        if CLEANUP_TEMP:
            shutil.rmtree(self.tmp_dir)

    def assert_content_equal(self, expected, actual):
        with open(expected, encoding='utf-8') as exp_file, open(actual, encoding='utf-8') as act_file:
            exp = exp_file.readlines()
            act = act_file.readlines()
            delta = difflib.unified_diff(exp, act, 'expected', 'actual')
            d = ''.join(delta)
            if len(d) > 0:
                self.fail(d)

    def _launch_merge(self, result_name, options, *src_paths):
        parser = make_argparser()
        actual = pj(self.tmp_dir, result_name)
        res_src_paths = [pj(RESOURCE_DIR, src) for src in src_paths]
        args = parser.parse_args([*res_src_paths, '-o', actual, *options])
        run_merge(args)
        return actual

    def run_test(self, result_name, options, *src_paths):
        expected = pj(RESOURCE_DIR, 'expected', result_name)
        actual = self._launch_merge(result_name, options, *src_paths)
        self.assert_content_equal(expected, actual)

    def run_error(self, expected_message, options, *src_paths):
        try:
            self._launch_merge(ERROR, options, *src_paths)
        except Exception as e:
            self.assertEqual(expected_message, str(e))
            return
        self.fail("An error was expected, but the merge succeeded")

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

    def test_merge_same_namespace_in_different_files(self):
        # file1: temperature as NS1
        # file2: temperature as NS1
        self.run_test('test_merge_ns0_temperature.xml', [],
                      'ns0.xml', 'TestTemperatureNS_split1.NodeSet2.xml', 'TestTemperatureNS_split2.NodeSet2.xml')

    def test_merge_same_namespace_in_different_files_with_other_ns(self):
        # file1: temperature as NS1, pressure as NS2
        # file2: pressure as NS1
        self.run_test('test_merge_ns0_temperature_pressure.xml', [],
                      'ns0.xml', 'TestTemperatureNS_PressureNS_split1.NodeSet2.xml', 'TestPressureNS_split2.NodeSet2.xml')

    def test_merge_error_same_node_id_in_one_file(self):
        # file1: temperature as NS1 with duplicate node ID
        self.run_error("There are duplicate Node IDs: ['ns=1;i=15001', 'ns=1;i=15006']", [],
                      'ns0.xml', 'TestTemperatureNS_duplicate_nodeID.NodeSet2.xml')

    def test_merge_error_same_node_id_in_two_files(self):
        # file1: temperature as NS1, pressure as NS2
        # file2: pressure as NS2 with duplicate node ID for pressure NS
        self.run_error("There are duplicate Node IDs: ['ns=2;i=15014']", [],
                      'ns0.xml', 'TestTemperatureNS_PressureNS_split1.NodeSet2.xml', 'TestPressureNS_split2_duplicate_nodeID.NodeSet2.xml')

    def test_remove_subtree(self):
        self.run_test('test_remove_subtree.xml', ['--remove-subtree', 'ns=1;i=15009'],
                      'ns0.xml', 'TestTemperatureNS.NodeSet2.xml')

    def test_remove_unused(self):
        self.run_test('test_remove_unused.xml', ['--remove-unused'],
                      'ns0.xml', 'TestUnusedTypes.NodeSet2.xml')

    def test_remove_unused_retainNS0(self):
        self.run_test('test_remove_unused_retainNS0.xml', ['--remove-unused', '--retain-ns0'],
                      'ns0.xml', 'TestUnusedTypes.NodeSet2.xml')

    def test_remove_unused_retain_types(self):
        self.run_test('test_remove_unused_retain_types.xml', ['--remove-unused', '--retain-types', 'i=6', 'i=27'],
                      'ns0.xml', 'TestUnusedTypes.NodeSet2.xml')


if __name__ == '__main__':
    unittest.main()
