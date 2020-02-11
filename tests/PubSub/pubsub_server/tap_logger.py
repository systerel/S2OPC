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

class TapLogger(object):
    """
    Generates a TAP report from test descriptions and result.
    """
    def __init__(self, report_name):
        self.report_name = report_name
        self.nb_tests = 0
        self.fd = open(file=report_name, mode='w', encoding='utf-8')
        self.tests_results = list()
        self.section = ""
        self.has_failed_tests = False
        self._finalized = False

    def __del__(self):
        if not self._finalized:
            # When the object is destroyed but not finalized,
            #  it means it is our last chance to save it.
            self.finalize_report(last_chance_finalization = True)

    def begin_section(self, section_name):
        self.section = section_name

    def add_test(self, test_description, test_result):
        self.nb_tests = self.nb_tests + 1
        if test_result:
            self.tests_results.append("ok {0} {1} {2}\n".format(self.nb_tests, self.section, test_description))
        else:
            self.tests_results.append("not ok {0} {1} {2}\n".format(self.nb_tests, self.section, test_description))
            self.has_failed_tests = True

    def finalize_report(self, last_chance_finalization = False):
        """
        last_chance_finalization is an internal option which marks the finalization of the TAP as successful or failed.
        """
        self.begin_section('TAP')
        self.add_test('- test finished', not last_chance_finalization)
        self.fd.write("1..{0}\n".format(self.nb_tests))
        self.fd.writelines(self.tests_results)
        self.fd.close()
        self._finalized = True
