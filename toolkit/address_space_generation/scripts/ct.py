#!/usr/bin/env python3
# -*- coding: utf8 -*-
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
