#!/usr/bin/env python3

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


# Simple script to templatise and render the pubsub configurations campaigns

from string import Template
import argparse


xml_emitter = '''
<PubSub publisherId="42">
    <connection address="opc.udp://232.1.2.100:4840" mode="publisher">
        <message id="93" version="1" publishingInterval="$pubint" securityMode="$emit_secmode">
            <variable nodeId="ns=1;i=0" dataType="UInt32" displayName="ignored"/>
        </message>
    </connection>
    <!-- The emitter subscribes to the variable that has been looped-back,
         but stores it in another cell of the cache -->
    <connection address="opc.udp://232.1.2.101:4840" mode="subscriber">
        <message id="93" version="1" publishingInterval="$pubint" securityMode="$loop_secmode" publisherId="43">
            <variable nodeId="ns=1;i=1" dataType="UInt32" displayName="ignored"/>
        </message>
    </connection>
</PubSub>
'''

xml_loopback = '''
<PubSub publisherId="43">
    <connection address="opc.udp://232.1.2.100:4840" mode="subscriber">
        <message id="93" version="1" publishingInterval="$pubint" securityMode="$loop_secmode" publisherId="42">
            <variable nodeId="ns=1;i=0" dataType="UInt32" displayName="ignored"/>
        </message>
    </connection>
    <!-- The loopback publishes the variable it receives -->
    <connection address="opc.udp://232.1.2.101:4840" mode="publisher">
        <message id="93" version="1" publishingInterval="$pubint" securityMode="$emit_secmode">
            <variable nodeId="ns=1;i=0" dataType="UInt32" displayName="ignored"/>
        </message>
    </connection>
</PubSub>
'''


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Instantiate XMLs')
    parser.add_argument('publishing_interval', type=float, help='In milliseconds')
    parser.add_argument('emitter_security_mode', choices=('none', 'sign', 'signAndEncrypt'))
    parser.add_argument('--loopback_security_mode', metavar='mode', default='same', choices=('same', 'none', 'sign', 'signAndEncrypt'))
    parser.add_argument('--output-emitter', metavar='path', default='./config_rtt_emitter.xml')
    parser.add_argument('--output-loopback', metavar='path', default='./config_rtt_loopback.xml')
    args = parser.parse_args()

    if args.loopback_security_mode == 'same':
        args.loopback_security_mode = args.emitter_security_mode

    with open(args.output_emitter, 'w') as f:
        f.write(Template(xml_emitter).substitute(pubint=args.publishing_interval,
                                                 emit_secmode=args.emitter_security_mode,
                                                 loop_secmode=args.loopback_security_mode))
    with open(args.output_loopback, 'w') as f:
        f.write(Template(xml_loopback).substitute(pubint=args.publishing_interval,
                                                  emit_secmode=args.emitter_security_mode,
                                                  loop_secmode=args.loopback_security_mode))
