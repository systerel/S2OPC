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


from functools import total_ordering
import uuid
from binascii import hexlify, unhexlify

from _pys2opc import ffi, lib as libsub


class Request:
    """
    Base class for Requests. Adds a timestamp to ease the performance measurement.

    Args:
        payload: An OpcUa_*Request.

    Attributes:
        eventResponseReceived: Event that is set when the response is received and the on_generic_response()
                               of the connection has been called.
        requestContext: A (unique) identifier for the request (read-only).
    """
    def __init__(self, payload):
        self.timestampSent = None  # The sender of the request sets the timestamp
        self.response = None
        self.eventResponseReceived = threading.Event()
        self._requestContext = ffi.new_handle(self)
        self.payload = payload

    @property
    def requestContext(self):
        return self._requestContext


class Response:
    """
    Base class for Responses.
    Adds a reference to the request and the timestamp of the received time.

    Args:
        payload: An OpcUa_*Response.
    """
    def __init__(self, payload):
        self.timestampReceived = None  # The receiver sets the timestamp
        self.request = request
        self.payload = payload

    def get_roundtrip_time(self):
        return self.timestampReceived - self.request.timestampSent



def bytestring_to_bytes(bstring):
    """SOPC_ByteString or SOPC_ByteString* to python bytes()"""
    return ffi.string(bstring.Data, bstring.Length)

def string_to_str(string):
    """SOPC_String or SOPC_String* to python str()"""
    return ffi.string(string.Data, string.Length).decode()

def nodeid_to_str(node):
    """SOPC_NodeId or SOPC_NodeId* to its str representation in the OPC-UA XML syntax."""
    snid = 'ns={};'.format(node.Namespace) if node.Namespace != 0 else ''
    if node.IdentifierType == libsub.SOPC_IdentifierType_Numeric:
        snid += 'i={}'.format(node.Data.Numeric)
    elif node.IdentifierType == libsub.SOPC_IdentifierType_String:
        snid += 's=' + string_to_str(node.Data.String)
    elif node.IdentifierType == libsub.SOPC_IdentifierType_Guid:
        # TODO
        raise ValueError('TODO')
    elif node.IdentifierType == libsub.SOPC_IdentifierType_ByteString:
        # This operation may fail, as it may not be possible to represent the ByteString in the string,
        #  but this is how the OPC-UA XML representation of the NodeId works,
        #  so there should not be unrepresentable bytestring NodeId in your AddressSpace anyway.
        snid += 'b=' + bytestring_to_bytes(node.Data.Bstring).decode()
    else:
        raise ValueError('Unknown NodeId type: {}'.format(node.IdentifierType))
    return snid

def guid_to_uuid(guid):
    """SOPC_Guid or SOPC_Guid* to the Python's uuid."""
    # S2OPC internal representation is local-endian, except for Data4,
    #  which is always big endian.
    a = '{:08X}-{:04X}-{:04X}-'.format(guid.Data1, guid.Data2, guid.Data3)
    b = hexlify(bytes(guid.Data4)).decode()
    c = b[:4]+'-'+b[4:]
    return uuid.UUID(a+c)



@total_ordering
class Variant:
    """
    A Variant is the Pythonic representation of a SOPC_Variant.
    The SOPC_Variant is a C-structure that can contain multiple built-in types,
    such as integers, floats, strings.

    A Variant instance supports the arithmetic, comparison operations, and increment operators of the Python types.
    For instance:
    >>> Variant(2) + Variant(6)  # Produces a new Variant
    Variant(8)
    >>> Variant('foo') + Variant('bar')
    Variant('foobar')
    >>> Variant(b'to') * 2  # ByteString
    Variant(b'toto')
    >>> v = Variant(2); v *= 2; v
    Variant(4)
    >>> Variant(2) == Variant(2) and Variant(2.) == Variant(2) and Variant(2.) == 2
    True

    A SOPC_Variant can be converted to a Variant with the static method Variant.from_sopc_variant().
    A Variant can be converted to a SOPC_Variant with the method to_sopc_variant().
    """
    def __init__(self, python_value):
        self._value = python_value

    def __repr__(self):
        return 'Variant(' + repr(self._value) + ')'
    def __str__(self):
        return str(self._value)

    def __eq__(s, o):
        if isinstance(o, Variant):
            return s._value == o._value
        return s._value == o
    def __lt__(s, o):
        if isinstance(o, Variant):
            return s._value < o._value
        return s._value < o

    # Arithmetics
    def __add__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value + o._value)
        return Variant(s._value + o)
    def __sub__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value - o._value)
        return Variant(s._value - o)
    def __mul__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value * o._value)
        return Variant(s._value * o)
    #def __matmul__(s, o):
    #    if isinstance(o, Variant):
    #        return Variant(s._value @ o._value)
    #    return Variant(s._value @ o)
    def __truediv__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value / o._value)
        return Variant(s._value / o)
    def __floordiv__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value // o._value)
        return Variant(s._value // o)
    def __mod__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value % o._value)
        return Variant(s._value % o)
    def __divmod__(s, o):
        if isinstance(o, Variant):
            return Variant(divmod(s._value, o._value))
        return Variant(divmod(s._value, o))
    def __pow__(s, o, *args):
        if isinstance(o, Variant):
            return Variant(pow(s._value, o._value, *args))
        return Variant(pow(s._value, o, *args))
    def __lshift__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value << o._value)
        return Variant(s._value << o)
    def __rshift__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value >> o._value)
        return Variant(s._value >> o)
    def __and__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value & o._value)
        return Variant(s._value & o)
    def __xor__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value ^ o._value)
        return Variant(s._value ^ o)
    def __or__(s, o):
        if isinstance(o, Variant):
            return Variant(s._value | o._value)
        return Variant(s._value | o)
    def __radd__(s, o):
        return Variant(o + s._value)
    def __rsub__(s, o):
        return Variant(o - s._value)
    def __rmul__(s, o):
        return Variant(o * s._value)
    #def __rmatmul__(s, o):
    #    return Variant(o @ s._value)
    def __rtruediv__(s, o):
        return Variant(o / s._value)
    def __rfloordiv__(s, o):
        return Variant(o // s._value)
    def __rmod__(s, o):
        return Variant(o % s._value)
    def __rdivmod__(s, o):
        return Variant(divmod(o, s._value))
    def __rpow__(s, o):
        return Variant(pow(o, s._value))
    def __rlshift__(s, o):
        return Variant(o << s._value)
    def __rrshift__(s, o):
        return Variant(o >> s._value)
    def __rand__(s, o):
        return Variant(o & s._value)
    def __rxor__(s, o):
        return Variant(o ^ s._value)
    def __ror__(s, o):
        return Variant(o | s._value)
    def __iadd__(s, o):
        if isinstance(o, Variant):
            s._value += o._value
        else:
            s._value += o
        return s
    def __isub__(s, o):
        if isinstance(o, Variant):
            s._value -= o._value
        else:
            s._value -= o
        return s
    def __imul__(s, o):
        if isinstance(o, Variant):
            s._value *= o._value
        else:
            s._value *= o
        return s
    #def __imatmul__(s, o):
    #    if isinstance(o, Variant):
    #        s._value @= o._value
    #    else:
    #        s._value @= o
    #    return s
    def __itruediv__(s, o):
        if isinstance(o, Variant):
            s._value /= o._value
        else:
            s._value /= o
        return s
    def __ifloordiv__(s, o):
        if isinstance(o, Variant):
            s._value //= o._value
        else:
            s._value //= o
        return s
    def __imod__(s, o):
        if isinstance(o, Variant):
            s._value %= o._value
        else:
            s._value %= o
        return s
    def __ipow__(s, o):
        if isinstance(o, Variant):
            s._value **= o._value
        else:
            s._value **= o
        return s
    def __ilshift__(s, o):
        if isinstance(o, Variant):
            s._value <<= o._value
        else:
            s._value <<= o
        return s
    def __irshift__(s, o):
        if isinstance(o, Variant):
            s._value >>= o._value
        else:
            s._value >>= o
        return s
    def __iand__(s, o):
        if isinstance(o, Variant):
            s._value &= o._value
        else:
            s._value &= o
        return s
    def __ixor__(s, o):
        if isinstance(o, Variant):
            s._value ^= o._value
        else:
            s._value ^= o
        return s
    def __ior__(s, o):
        if isinstance(o, Variant):
            s._value |= o._value
        else:
            s._value |= o
        return s
    def __neg__(s):
        return Variant(-s._value)
    def __pos__(s):
        return Variant(+s._value)
    def __abs__(s):
        return Variant(abs(s._value))
    def __invert__(s):
        return Variant(~s._value)
    def __complex__(s):
        return complex(s._value)
    def __int__(s):
        return int(s._value)
    def __float__(s):
        return float(s._value)
    def __index__(s):
        return int(s._value)
    def __round__(s, *args):
        return round(s._value)
    def __trunc__(s):
        return trunc(s._value)
    def __floor__(s):
        return floor(s._value)
    def __ceil__(s):
        return ceil(s._value)

    @staticmethod
    def from_sopc_variant(variant):
        """
        Returns a Variant initialized from a SOPC_Variant or a SOPC_Variant* (or a void*).
        """
        variant = ffi.cast('SOPC_Variant *', variant)
        sopc_type = variant.BuiltInTypeId

        if variant.ArrayType == libsub.SOPC_VariantArrayType_SingleValue:
            if sopc_type == libsub.SOPC_Null_Id:
                return Variant(None)
            elif sopc_type == libsub.SOPC_Boolean_Id:
                return Variant(variant.Value.Boolean)
            elif sopc_type == libsub.SOPC_SByte_Id:
                return Variant(variant.Value.Sbyte)
            elif sopc_type == libsub.SOPC_Byte_Id:
                return Variant(variant.Value.Byte)
            elif sopc_type == libsub.SOPC_Int16_Id:
                return Variant(variant.Value.Int16)
            elif sopc_type == libsub.SOPC_UInt16_Id:
                return Variant(variant.Value.Uint16)
            elif sopc_type == libsub.SOPC_Int32_Id:
                return Variant(variant.Value.Int32)
            elif sopc_type == libsub.SOPC_UInt32_Id:
                return Variant(variant.Value.Uint32)
            elif sopc_type == libsub.SOPC_Int64_Id:
                return Variant(variant.Value.Int64)
            elif sopc_type == libsub.SOPC_UInt64_Id:
                return Variant(variant.Value.Uint64)
            elif sopc_type == libsub.SOPC_Float_Id:
                return Variant(variant.Value.Floatv)
            elif sopc_type == libsub.SOPC_Double_Id:
                return Variant(variant.Value.Doublev)
            elif sopc_type == libsub.SOPC_String_Id:
                return Variant(string_to_str(variant.Value.String))
            elif sopc_type == libsub.SOPC_DateTime_Id:
                return Variant(variant.Value.Date)  # int64_t
            elif sopc_type == libsub.SOPC_Guid_Id:
                return Variant(guid_to_uuid(variant.Value.Guid))
            elif sopc_type == libsub.SOPC_ByteString_Id:
                return Variant(bytestring_to_bytes(variant.Value.Bstring))
            elif sopc_type == libsub.SOPC_XmlElement_Id:
                return Variant(bytestring_to_bytes(variant.Value.XmlElt))
            elif sopc_type == libsub.SOPC_NodeId_Id:
                return Variant(nodeid_to_str(variant.Value.NodeId))
            #elif sopc_type == libsub.SOPC_ExpandedNodeId_Id:
            #    return Variant(variant.Value.)
            elif sopc_type == libsub.SOPC_StatusCode_Id:
                return Variant(variant.Value.Status)
            elif sopc_type == libsub.SOPC_QualifiedName_Id:
                Qname = variant.Value.Qname
                return Variant((Qname.NamespaceIndex, string_to_str(Qname.Name.Data)))
            elif sopc_type == libsub.SOPC_LocalizedText_Id:
                LocalizedText = variant.Value.LocalizedText
                return Variant((string_to_str(LocalizedText.Locale), string_to_str(LocalizedText.Text)))
            #elif sopc_type == libsub.SOPC_ExtensionObject_Id:
            #    return Variant(variant.Value.)
            #elif sopc_type == libsub.SOPC_DataValue_Id:
            #    return Variant(variant.Value.)
            #elif sopc_type == libsub.SOPC_Variant_Id:
            #    return Variant(variant.Value.)
            #elif sopc_type == libsub.SOPC_DiagnosticInfo_Id:
            #    return Variant(variant.Value.)
            raise ValueError('SOPC_Variant to Python conversion not supported for built-in type {}.'.format(sopc_type))
        elif variant.ArrayType == libsub.SOPC_VariantArrayType_Array:
            sopc_array = variant.Value.Array
            length = sopc_array.Length
            content = sopc_array.Content
            if sopc_type == libsub.SOPC_Null_Id:
                return [Variant(None) for i in range(length)]
            elif sopc_type == libsub.SOPC_Boolean_Id:
                return [Variant(content.BooleanArr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_SByte_Id:
                return [Variant(content.SbyteArr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_Byte_Id:
                return [Variant(content.ByteArr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_Int16_Id:
                return [Variant(content.Int16Arr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_UInt16_Id:
                return [Variant(content.Uint16Arr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_Int32_Id:
                return [Variant(content.Int32Arr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_UInt32_Id:
                return [Variant(content.Uint32Arr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_Int64_Id:
                return [Variant(content.Int64Arr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_UInt64_Id:
                return [Variant(content.Uint64Arr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_Float_Id:
                return [Variant(content.FloatvArr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_Double_Id:
                return [Variant(content.DoublevArr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_String_Id:
                return [Variant(string_to_str(content.StringArr[i])) for i in range(length)]
            elif sopc_type == libsub.SOPC_DateTime_Id:
                return [Variant(content.DateArr[i]) for i in range(length)]  # int64_t
            elif sopc_type == libsub.SOPC_Guid_Id:
                return [Variant(guid_to_uuid(content.GuidArr[i])) for i in range(length)]
            elif sopc_type == libsub.SOPC_ByteString_Id:
                return [Variant(bytestring_to_bytes(content.BstringArr[i])) for i in range(length)]
            elif sopc_type == libsub.SOPC_XmlElement_Id:
                return [Variant(bytestring_to_bytes(content.XmlEltArr[i])) for i in range(length)]
            elif sopc_type == libsub.SOPC_NodeId_Id:
                return [Variant(nodeid_to_str(content.NodeIdArr[i])) for i in range(length)]
            #elif sopc_type == libsub.SOPC_ExpandedNodeId_Id:
            #    return [Variant(content.) for i in range(length)]
            elif sopc_type == libsub.SOPC_StatusCode_Id:
                return [Variant(content.StatusArr[i]) for i in range(length)]
            elif sopc_type == libsub.SOPC_QualifiedName_Id:
                Qname = content.QnameArr[i]
                return [Variant((Qname.NamespaceIndex, string_to_str(Qname.Name.Data))) for i in range(length)]
            elif sopc_type == libsub.SOPC_LocalizedText_Id:
                LocalizedText = content.LocalizedTextArr[i]
                return [Variant((string_to_str(LocalizedText.Locale), string_to_str(LocalizedText.Text))) for i in range(length)]
            #elif sopc_type == libsub.SOPC_ExtensionObject_Id:
            #    return [Variant(content.) for i in range(length)]
            #elif sopc_type == libsub.SOPC_DataValue_Id:
            #    return [Variant(content.) for i in range(length)]
            #elif sopc_type == libsub.SOPC_Variant_Id:
            #    return [Variant(content.) for i in range(length)]
            #elif sopc_type == libsub.SOPC_DiagnosticInfo_Id:
            #    return [Variant(content.) for i in range(length)]
            raise ValueError('SOPC_Variant to Python conversion not supported for built-in type {}.'.format(sopc_type))
        elif variant.ArrayType == libsub.SOPC_VariantArrayType_Matrix:
            raise ValueError('SOPC_Variant matrices are not supported.')
        else:
            raise ValueError('Invalid SOPC_Variant.ArrayType')

    def to_sopc_variant(self):
        # Keep track of allocated values
        # Use DoNotClear
        return pVariant

class DataValue:
    # The value is stored as Variant().
    def __init__(self, timestampSource, timestampServer, statusCode, python_value):
        self.timestampSource = timestampSource
        self.timestampServer = timestampServer
        self.statusCode = statusCode
        self.value = python_value

    @staticmethod
    def from_sopc_libsub_value(libsub_value):
        return DataValue(libsub_value.source_timestamp, libsub_value.server_timestamp, libsub_value.quality, Variant.from_sopc_variant(libsub_value.raw_value))


if __name__ == '__main__':
    # Auto-test
    import struct
    guid = ffi.new('SOPC_Guid *')
    guid.Data1, guid.Data2, guid.Data3 = struct.unpack('>IHH', unhexlify(b'72962B91FA754AE6'))
    guid.Data4 = [0x8D, 0x28, 0xB4, 0x04, 0xDC, 0x7D, 0xAF, 0x63]
    assert uuid.UUID('72962b91-fa75-4ae6-8d28-b404dc7daf63') == guid_to_uuid(guid)
