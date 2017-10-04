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

Type_list = ['Int64','UInt32','Double','String','ByteString','XmlElement']

Variant_List = [ua.VariantType.Int64, ua.VariantType.UInt32, ua.VariantType.Double, ua.VariantType.String, ua.VariantType.ByteString, ua.VariantType.XmlElement]

Initial_values_list = [-1000,1000,2.0,"String:INGOPCS","ByteString:INGOPCS".encode(),XmlElement(u"XmlElement:INGOPCS")]

# chaine aléatoire de 30 caractères
random_string = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(random.randint(0,30)))

New_values_list = [random.randint(-2**63+1,2**63+1),random.randint(0,2**32-1), random.uniform(-2**1022,2**1023),"String:{}".format(random_string),"ByteString:{}".format(random_string).encode(),XmlElement(u"XmlElement:{}".format(random_string))]

