#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
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
    This objective of this class is to generate a TAP report.
    """
    def __init__(self, report_name):
        self.report_name = report_name
        self.nb_tests = 0
        self.fd = open(file=report_name, mode='w', encoding='utf-8')
        self.tests_results = list()
        self.section = ""
        self.has_failed_tests = False

    def begin_section(self, section_name):
        self.section = section_name

    def add_test(self, test_description, test_result):
        self.nb_tests = self.nb_tests + 1
        if test_result:
            self.tests_results.append("ok {0} {1} {2}\n".format(self.nb_tests, self.section, test_description))
        else:
            self.tests_results.append("not ok {0} {1} {2}\n".format(self.nb_tests, self.section, test_description))
            self.has_failed_tests = True

    def finalize_report(self):
        self.fd.write("1..{0}\n".format(self.nb_tests))
        for test_result in self.tests_results:
            self.fd.write(test_result)
        self.fd.close()
