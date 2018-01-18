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


# Compile the given mako template

from mako.template import Template
from mako.lookup import TemplateLookup
import argparse

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("f_in", help="the template file to compile")
    parser.add_argument("f_out", help="the output file")
    parser.add_argument("param", nargs='?', default="{}", help="a dictionary that gives values to template variables")
    args = parser.parse_args()

    mylookup = TemplateLookup(directories=['.'])
    mytemplate = Template(filename=args.f_in, output_encoding='utf-8', input_encoding='utf-8', lookup=mylookup, strict_undefined=True)
    open(args.f_out, 'wb').write(mytemplate.render(**eval(args.param)))

if __name__ == "__main__":
    main()
