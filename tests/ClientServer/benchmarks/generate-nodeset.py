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

import argparse
import shutil

HEADER_FILENAME = 'address_space_header.xml.part'
FOOTER_FILENAME = 'address_space_footer.xml.part'

variable_template = '''
  <UAVariable DataType="Boolean" NodeId="{node_id}" BrowseName="{browse_name}" UserAccessLevel="3" AccessLevel="3">
    <DisplayName>{display_name}</DisplayName>
    <References>
      <Reference ReferenceType="HasTypeDefinition">i=63</Reference>
      <Reference ReferenceType="Organizes" IsForward="false">i=85</Reference>
    </References>
    <Value>
      <uax:Boolean>false</uax:Boolean>
    </Value>
  </UAVariable>
'''


def generate_address_space_xml(out, size):
    with open(HEADER_FILENAME) as header_fd:
        shutil.copyfileobj(header_fd, out)

    for i in range(0, size):
        object_data = {
            'browse_name': '1:OBJ_%d' % i,
            'node_id': 'ns=1;s=Objects.%d' % i,
            'display_name': 'Object_%d' % i
        }

        out.write(variable_template.format(**object_data))

    with open(FOOTER_FILENAME) as footer_fd:
        shutil.copyfileobj(footer_fd, out)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Generate an address space of a given size')
    parser.add_argument('size', metavar='SIZE', type=int,
                        help='Number of objects to put in the address space')
    parser.add_argument('out', metavar='OUT_XML', type=str,
                        help='Name of the output XML file')

    args = parser.parse_args()

    with open(args.out, 'w') as fd:
        generate_address_space_xml(fd, args.size)
