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

from _pys2opc import ffi, lib as libsub


class Request:
    """
    Base class for Requests. Adds a timestamp to ease the performance measurement.

    Args:
        paylaod: An OpcUa_*Request.

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
    """
    def __init__(self, payload):
        self.timestampReceived = None  # The receiver sets the timestamp
        self.request = request
        self.payload = payload

    def get_roundtrip_time(self):
        return self.timestampReceived - self.request.timestampSent



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
                return Variant(ffi.string(variant.Value.String.Data, variant.Value.String.Length).decode())
            elif sopc_type == libsub.SOPC_DateTime_Id:
                return Variant(variant.Value.Date)  # int64_t
            #elif sopc_type == libsub.SOPC_Guid_Id:
            #    return Variant(variant.Value.)
            elif sopc_type == libsub.SOPC_ByteString_Id:
                return Variant(ffi.string(variant.Value.Bstring.Data, variant.Value.Bstring.Length))
            #elif sopc_type == libsub.SOPC_XmlElement_Id:
            #    return Variant(variant.Value.)
            #elif sopc_type == libsub.SOPC_NodeId_Id:
            #    # TODO: NodeId
            #    return Variant(variant.Value.)
            #elif sopc_type == libsub.SOPC_ExpandedNodeId_Id:
            #    return Variant(variant.Value.)
            ##elif sopc_type == libsub.SOPC_StatusCode_Id:
            ##    return Variant(variant.Value.Status)
            ##elif sopc_type == libsub.SOPC_QualifiedName_Id:
            ##    Qname = variant.Value.Qname
            ##    return Variant((Qname.NamespaceIndex, ffi.string(Qname.Name.Data, Qname.Name.Length)))
            ##elif sopc_type == libsub.SOPC_LocalizedText_Id:
            ##    return Variant(variant.Value.)
            ##elif sopc_type == libsub.SOPC_ExtensionObject_Id:
            ##    return Variant(variant.Value.)
            ##elif sopc_type == libsub.SOPC_DataValue_Id:
            ##    return Variant(variant.Value.)
            ##elif sopc_type == libsub.SOPC_Variant_Id:
            ##    return Variant(variant.Value.)
            ##elif sopc_type == libsub.SOPC_DiagnosticInfo_Id:
            ##    return Variant(variant.Value.)
        elif variant.ArrayType == libsub.SOPC_VariantArrayType_Array:
            raise NotImplementedError
        elif variant.ArrayType == libsub.SOPC_VariantArrayType_Matrix:
            raise NotImplementedError
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
