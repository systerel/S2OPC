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

import random
import string
import sys
from opcua import ua
from opcua.ua import XmlElement

running_in_linux=sys.platform.startswith('linux')

sUri = 'opc.tcp://localhost:4841'

securityPolicyNoneURI = "http://opcfoundation.org/UA/SecurityPolicy#None"
securityPolicyBasic256URI = "http://opcfoundation.org/UA/SecurityPolicy#Basic256"
securityPolicyBasic256Sha256URI = "http://opcfoundation.org/UA/SecurityPolicy#Basic256Sha256"

# chaine aléatoire de 30 caractères
random_string = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(random.randint(0,30)))

# liste de tuples sous la forme (Type, valeur initiale, nouvelle valeur)
variantInfoList = [ ('Int64', ua.VariantType.Int64, -1000, random.randint(-9223372036854775808, 9223372036854775807)),
                    ('UInt32', ua.VariantType.UInt32, 1000, random.randint(0, 4294967295)),
                    ('Double', ua.VariantType.Double, 2.0, random.uniform(0,2**1023) * random.choice([-1,1])),
                    ('String', ua.VariantType.String, "String:INGOPCS", "String:{}".format(random_string)),
                    ('ByteString', ua.VariantType.ByteString, str.encode("ByteString:INGOPCS"), "ByteString:{}".format(random_string).encode()),
                    ('XmlElement', ua.VariantType.XmlElement, XmlElement("XmlElement:INGOPCS"), XmlElement("XmlElement:{}".format(random_string))),
                    ('SByte', ua.VariantType.SByte, -127, random.randint(-128,127)),
                    ('Byte', ua.VariantType.Byte, 255, random.randint(0,255)),
                    ('Int16', ua.VariantType.Int16, -32767, random.randint(-32768, 32767)),
                    ('UInt16', ua.VariantType.UInt16, 65535, random.randint(0, 65535)),
                    ('Int32', ua.VariantType.Int32, -2147483647, random.randint(-2147483648,2147483647)),
                    ('UInt64', ua.VariantType.UInt64, 1844674407370955, random.randint(0,18446744073709551615)),
                    ('Float', ua.VariantType.Float, 109517.875, random.uniform(-2**127,2**127)),
                  ]
