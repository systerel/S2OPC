#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2018 Systerel and others.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

    def begin_section(self, section_name):
        self.section = section_name

    def add_test(self, test_description, test_result):
        self.nb_tests = self.nb_tests + 1
        if test_result:
            self.tests_results.append("ok {0} {1} {2}\n".format(self.nb_tests, self.section, test_description))
        else:
            self.tests_results.append("not ok {0} {1} {2}\n".format(self.nb_tests, self.section, test_description))

    def finalize_report(self):
        self.fd.write("1..{0}\n".format(self.nb_tests))
        for test_result in self.tests_results:
            self.fd.write(test_result)
        self.fd.close()
