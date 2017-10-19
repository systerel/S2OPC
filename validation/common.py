#!/usr/bin/python3.4
#-*-coding:Utf-8 -*

# Copyright (C) 2017 Systerel and others.
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

import random
import string
from opcua import ua
from opcua.ua import XmlElement

sUri = 'opc.tcp://localhost:4841'

# chaine aléatoire de 30 caractères
random_string = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(random.randint(0,30)))

# liste de tuples sous la forme (Type, valeur initiale, nouvelle valeur)
variantInfoList = [ ('Int64', ua.VariantType.Int64, -1000, random.randint(-9223372036854775808, 9223372036854775807)),
                    ('UInt32', ua.VariantType.UInt32, 1000, random.randint(0, 4294967295)),
                    ('Double', ua.VariantType.Double, 2.0, random.uniform(-2**1022,2**1023)),
                    ('String', ua.VariantType.String, "String:INGOPCS", "String:{}".format(random_string)),
                    ('ByteString', ua.VariantType.ByteString, "ByteString:INGOPCS", "ByteString:{}".format(random_string).encode()),
                    ('XmlElement', ua.VariantType.XmlElement, XmlElement("u:XmlElement:INGOPCS"), XmlElement(u"XmlElement:{}".format(random_string))),
                    ('SByte', ua.VariantType.SByte, -128, random.randint(-128,127)),
                    ('Byte', ua.VariantType.Byte, 255, random.randint(0,255)),
                    ('Int16', ua.VariantType.Int16, -32768, random.randint(-32768, 32767)),
                    ('UInt16', ua.VariantType.UInt16, 65535, random.randint(0, 65535)),
                    ('Int32', ua.VariantType.Int32, -2147483648, random.randint(-2147483648,2147483647)),
                    ('UInt64', ua.VariantType.UInt64, 18446744073709551614, random.randint(0,18446744073709551615)),
                    ('Float', ua.VariantType.Float, 5758787.5876875, random.uniform(-2**126,2**127)),
                  ]
