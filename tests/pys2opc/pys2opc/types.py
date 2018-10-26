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


import threading
from functools import total_ordering
import uuid
from binascii import hexlify, unhexlify
import time

from _pys2opc import ffi, lib as libsub


allocator_no_gc = ffi.new_allocator(alloc=libsub.malloc, free=None, should_clear_after_alloc=True)


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
        # Does not use the ffi.new_handle and from_handle capabilities because from_handle is subject to "undefined behavior"
        #  when it is given an unknown pointer...
        self._requestContextVoid = ffi.new_handle(self)  # Keep the void* to avoid garbage collection, ...
        self._requestContext = ffi.cast('uintptr_t', self._requestContextVoid)  # ... but only use the casted value.
        self.payload = payload

    @property
    def requestContext(self):
        """Returns an uintptr_t, that is castable to Python int and usable by the libsub API"""
        return self._requestContext


def bytestring_to_bytes(bstring):
    """SOPC_ByteString or SOPC_ByteString* to python bytes()"""
    if bstring == ffi.NULL or bstring.Data == ffi.NULL or bstring.Length <= 0:
        return b''
    return ffi.string(bstring.Data, bstring.Length)
def bytes_to_bytestring(b, no_gc=False):
    """
    Python bytes to SOPC_ByteString*.

    Args:
        no_gc: When True, the Python garbage collection mechanism is omited and the string must be
               either manually deleted with a call to SOPC_String_Delete or passed to an object
               which will call SOPC_String_Delete for you.
    """
    alloc = allocator_no_gc if no_gc else ffi.new
    bstring = alloc('SOPC_ByteString *')
    bstring.Length = len(b)
    bstring.Data = ffi.new('char[{}]'.format(len(b)), b)
    return bstring

def string_to_str(string):
    """SOPC_String or SOPC_String* to python str()"""
    if string == ffi.NULL or string.Data == ffi.NULL:
        return ''
    return ffi.string(string.Data, string.Length).decode()
def str_to_string(s, no_gc=False):
    """
    Python string to SOPC_String*.

    Args:
        no_gc: When True, the Python garbage collection mechanism is omited and the string must be
               either manually deleted with a call to SOPC_String_Delete or passed to an object
               which will call SOPC_String_Delete for you.
    """
    alloc = allocator_no_gc if no_gc else ffi.new
    string = alloc('SOPC_String *')
    status = libsub.SOPC_String_CopyFromCString(string, ffi.new('char[]', s.encode()))
    assert status == libsub.SOPC_STATUS_OK
    return string

def nodeid_to_str(node):
    """SOPC_NodeId or SOPC_NodeId* to its str representation in the OPC-UA XML syntax."""
    return ffi.string(libsub.SOPC_NodeId_ToCString(node)).decode()
def str_to_nodeid(nid, no_gc=True):
    """
    Python string to SOPC_NodeId*.

    Args:
        no_gc: When True, the Python garbage collection mechanism is omited and the string must be
               either manually deleted with a call to SOPC_String_Delete or passed to an object
               which will call SOPC_String_Delete for you.
    """
    node = libsub.SOPC_NodeId_FromCString(ffi.new('char []', nid.encode()), len(nid))
    if not no_gc:
        # There is no SOPC_NodeId_Delete, so we must make this Deleter.
        # In fact, it is only required for GUID NodeIds...
        def nodeid_cleaner(no):
            libsub.SOPC_NodeId_Clear(no)
            libsub.free(no)
        return ffi.gc(node, nodeid_cleaner)
    return node

def expandednodeid_to_str(exnode):
    """SOPC_ExpandedNodeId or SOPC_ExpandedNodeId* to its str representation in the OPC-UA XML syntax."""
    a = ''
    if exnode.ServerIndex:
        a += 'srv={};'.format(exnode.ServerIndex)
    nsu = string_to_str(ffi.addressof(exnode.NamespaceUri))
    if nsu:
        a += 'nsu={};'.format(nsu)
    b = ffi.string(libsub.SOPC_NodeId_ToCString(ffi.addressof(exnode.NodeId))).decode()
    return a + b

def guid_to_uuid(guid):
    """SOPC_Guid or SOPC_Guid* to the Python's uuid."""
    # S2OPC internal representation is local-endian, except for Data4,
    #  which is always big endian.
    a = '{:08X}-{:04X}-{:04X}-'.format(guid.Data1, guid.Data2, guid.Data3)
    b = hexlify(bytes(guid.Data4)).decode()
    c = b[:4]+'-'+b[4:]
    return uuid.UUID(a+c)
def uuid_to_guid(uid, no_gc=False):
    """
    uuid.UUID object to SOPC_Guid*.

    Args:
        no_gc: When True, the Python garbage collection mechanism is omited and the string must be
               either manually deleted with a call to SOPC_String_Delete or passed to an object
               which will call SOPC_String_Delete for you.
    """
    alloc = allocator_no_gc if no_gc else ffi.new
    guid = alloc('SOPC_Guid*')
    guid.Data1, guid.Data2, guid.Data3 = uid.fields[:3]
    for i,b in enumerate(uid.bytes[-8:]):
        guid.Data4[i] = b
    return guid

def datetime_to_float(datetime):
    """
    SOPC_DateTime* (the number of 100 nanosecond intervals since January 1, 1601)
    to Python time (the floating point number of seconds since 01/01/1970, see help(time)).
    """
    nsec = datetime[0]
    # (datetime.date(1970,1,1) - datetime.date(1601,1,1)).total_seconds() * 1000 * 1000 * 10
    return (nsec - 116444736000000000)/1e7
def float_to_datetime(t, no_gc=True):
    """
    Python timestamp to SOPC_DateTime*.

    Args:
        no_gc: When True, the Python garbage collection mechanism is omited and the string must be
               either manually deleted with a call to SOPC_String_Delete or passed to an object
               which will call SOPC_String_Delete for you.
    """
    alloc = allocator_no_gc if no_gc else ffi.new
    datetime = alloc('SOPC_DateTime*')
    # (datetime.date(1970,1,1) - datetime.date(1601,1,1)).total_seconds() * 1000 * 1000 * 10
    datetime[0] = int(t*1e7) + 116444736000000000
    return datetime
def ntp_to_python(i):
    """uint64_t NTP to Python time."""
    # Epoch is 01/01/1900 here.
    secs = i>>32
    fsecs = (i&0xFFFFFFFF)/(2**32)
    return secs - 2208988800. + fsecs


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

    Attributes:
        variant_type: Optional: The type of the Variant (see VariantType) when the value is produced from a SOPC_Variant*.
    """
    def __init__(self, python_value, variant_type=None):
        self._value = python_value
        self.variant_type = variant_type

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

    # Containers
    def __len__(s):
        return len(s._value)
    def __getitem__(s, k):
        return s._value[k]
    def __setitem__(s, k, v):
        s._value[k] = v
    def __delitem__(s, k):
        del s._value[k]
    def __iter__(s):
        yield from iter(s._value)  # Requires Python3 >= 3.3
    def __reversed__(s):
        yield from reversed(s._value)
    def __contains__(s, k):
        return k in s._value

    @staticmethod
    def from_sopc_variant(variant):
        """
        Returns a Variant initialized from a SOPC_Variant or a SOPC_Variant* (or a void*).
        """
        variant = ffi.cast('SOPC_Variant *', variant)
        sopc_type = variant.BuiltInTypeId

        if variant.ArrayType == libsub.SOPC_VariantArrayType_SingleValue:
            if sopc_type == libsub.SOPC_Null_Id:
                return Variant(None, sopc_type)
            elif sopc_type == libsub.SOPC_Boolean_Id:
                return Variant(variant.Value.Boolean, sopc_type)
            elif sopc_type == libsub.SOPC_SByte_Id:
                return Variant(variant.Value.Sbyte, sopc_type)
            elif sopc_type == libsub.SOPC_Byte_Id:
                return Variant(variant.Value.Byte, sopc_type)
            elif sopc_type == libsub.SOPC_Int16_Id:
                return Variant(variant.Value.Int16, sopc_type)
            elif sopc_type == libsub.SOPC_UInt16_Id:
                return Variant(variant.Value.Uint16, sopc_type)
            elif sopc_type == libsub.SOPC_Int32_Id:
                return Variant(variant.Value.Int32, sopc_type)
            elif sopc_type == libsub.SOPC_UInt32_Id:
                return Variant(variant.Value.Uint32, sopc_type)
            elif sopc_type == libsub.SOPC_Int64_Id:
                return Variant(variant.Value.Int64, sopc_type)
            elif sopc_type == libsub.SOPC_UInt64_Id:
                return Variant(variant.Value.Uint64, sopc_type)
            elif sopc_type == libsub.SOPC_Float_Id:
                return Variant(variant.Value.Floatv, sopc_type)
            elif sopc_type == libsub.SOPC_Double_Id:
                return Variant(variant.Value.Doublev, sopc_type)
            elif sopc_type == libsub.SOPC_String_Id:
                return Variant(string_to_str(variant.Value.String), sopc_type)
            elif sopc_type == libsub.SOPC_DateTime_Id:
                return Variant(datetime_to_float(variant.Value.Date), sopc_type)  # int64_t
            elif sopc_type == libsub.SOPC_Guid_Id:
                return Variant(guid_to_uuid(variant.Value.Guid), sopc_type)
            elif sopc_type == libsub.SOPC_ByteString_Id:
                return Variant(bytestring_to_bytes(variant.Value.Bstring), sopc_type)
            elif sopc_type == libsub.SOPC_XmlElement_Id:
                return Variant(bytestring_to_bytes(variant.Value.XmlElt), sopc_type)
            elif sopc_type == libsub.SOPC_NodeId_Id:
                return Variant(nodeid_to_str(variant.Value.NodeId), sopc_type)
            #elif sopc_type == libsub.SOPC_ExpandedNodeId_Id:
            #    return Variant(variant.Value., sopc_type)
            elif sopc_type == libsub.SOPC_StatusCode_Id:
                return Variant(variant.Value.Status, sopc_type)
            elif sopc_type == libsub.SOPC_QualifiedName_Id:
                Qname = variant.Value.Qname
                return Variant((Qname.NamespaceIndex, string_to_str(Qname.Name)), sopc_type)
            elif sopc_type == libsub.SOPC_LocalizedText_Id:
                LocalizedText = variant.Value.LocalizedText
                return Variant((string_to_str(LocalizedText.Locale), string_to_str(LocalizedText.Text)), sopc_type)
            #elif sopc_type == libsub.SOPC_ExtensionObject_Id:
            #    return Variant(variant.Value., sopc_type)
            #elif sopc_type == libsub.SOPC_DataValue_Id:
            #    return Variant(variant.Value., sopc_type)
            #elif sopc_type == libsub.SOPC_Variant_Id:
            #    return Variant(variant.Value., sopc_type)
            #elif sopc_type == libsub.SOPC_DiagnosticInfo_Id:
            #    return Variant(variant.Value., sopc_type)
            raise ValueError('SOPC_Variant to Python conversion not supported for built-in type {}.'.format(sopc_type))
        elif variant.ArrayType == libsub.SOPC_VariantArrayType_Array:
            sopc_array = variant.Value.Array
            length = sopc_array.Length
            content = sopc_array.Content
            if sopc_type == libsub.SOPC_Null_Id:
                return [Variant(None, sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_Boolean_Id:
                return [Variant(content.BooleanArr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_SByte_Id:
                return [Variant(content.SbyteArr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_Byte_Id:
                return [Variant(content.ByteArr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_Int16_Id:
                return [Variant(content.Int16Arr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_UInt16_Id:
                return [Variant(content.Uint16Arr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_Int32_Id:
                return [Variant(content.Int32Arr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_UInt32_Id:
                return [Variant(content.Uint32Arr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_Int64_Id:
                return [Variant(content.Int64Arr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_UInt64_Id:
                return [Variant(content.Uint64Arr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_Float_Id:
                return [Variant(content.FloatvArr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_Double_Id:
                return [Variant(content.DoublevArr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_String_Id:
                return [Variant(string_to_str(ffi.addressof(content.StringArr[i])), sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_DateTime_Id:
                return [Variant(datetime_to_float(content.DateArr[i]), sopc_type) for i in range(length)]  # int64_t
            elif sopc_type == libsub.SOPC_Guid_Id:
                return [Variant(guid_to_uuid(ffi.addressof(content.GuidArr[i])), sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_ByteString_Id:
                return [Variant(bytestring_to_bytes(ffi.addressof(content.BstringArr[i])), sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_XmlElement_Id:
                return [Variant(bytestring_to_bytes(ffi.addressof(content.XmlEltArr[i])), sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_NodeId_Id:
                return [Variant(nodeid_to_str(ffi.addressof(content.NodeIdArr[i])), sopc_type) for i in range(length)]
            #elif sopc_type == libsub.SOPC_ExpandedNodeId_Id:
            #    return [Variant(content., sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_StatusCode_Id:
                return [Variant(content.StatusArr[i], sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_QualifiedName_Id:
                Qname = content.QnameArr[i]
                return [Variant((Qname.NamespaceIndex, string_to_str(Qname.Name.Data)), sopc_type) for i in range(length)]
            elif sopc_type == libsub.SOPC_LocalizedText_Id:
                LocalizedText = content.LocalizedTextArr[i]
                return [Variant((string_to_str(LocalizedText.Locale), string_to_str(LocalizedText.Text)), sopc_type) for i in range(length)]
            #elif sopc_type == libsub.SOPC_ExtensionObject_Id:
            #    return [Variant(content., sopc_type) for i in range(length)]
            #elif sopc_type == libsub.SOPC_DataValue_Id:
            #    return [Variant(content., sopc_type) for i in range(length)]
            #elif sopc_type == libsub.SOPC_Variant_Id:
            #    return [Variant(content., sopc_type) for i in range(length)]
            #elif sopc_type == libsub.SOPC_DiagnosticInfo_Id:
            #    return [Variant(content., sopc_type) for i in range(length)]
            raise ValueError('SOPC_Variant to Python conversion not supported for built-in type {}.'.format(sopc_type))
        elif variant.ArrayType == libsub.SOPC_VariantArrayType_Matrix:
            raise ValueError('SOPC_Variant matrices are not supported.')
        else:
            raise ValueError('Invalid SOPC_Variant.ArrayType')

    allocator = ffi.new_allocator(alloc=libsub.malloc, free=libsub.SOPC_Variant_Delete, should_clear_after_alloc=True)

    def to_sopc_variant(self, *, copy_type_from_variant=None, sopc_variant_type=None, no_gc=False):
        """
        Converts the current Variant to a SOPC_Variant*.
        Handles both single values and array values.
        The type may be specified by either self.variant_type, copy_type_from_variant, or sopc_variant_type.
        If types is specified by multiple means, the type must be the same in all cases.

        Args:
            copy_type_from_variant: A C SOPC_Variant* or a Python Variant from which the type is copied.
            sopc_variant_type: A VariantType constant.
            no_gc: When True, the Python garbage collection mechanism is omited and the string must be
                   either manually deleted with a call to SOPC_String_Delete or passed to an object
                   which will call SOPC_String_Delete for you.
        """
        # Detect type
        sPossibleTypes = {self.variant_type, sopc_variant_type}
        if copy_type_from_variant is not None:
            if isinstance(copy_type_from_variant, Variant):
                sPossibleTypes.add(copy_type_from_variant.variant_type)
            else:
                sPossibleTypes.add(copy_type_from_variant.BuiltInTypeId)
        sPossibleTypes.remove(None)
        if not sPossibleTypes:
            raise ValueError('No type detected, please supply self.variant_type or sopc_variant_type or sopy_type_from_variant.')
        if len(sPossibleTypes) > 1:
            raise ValueError('More than one type detected, please supply self.variant_type or sopc_variant_type or sopy_type_from_variant.')
        sopc_type, = sPossibleTypes

        # Create and fill variant
        if no_gc:
            variant = allocator_no_gc('SOPC_Variant*')
        else:
            variant = Variant.allocator('SOPC_Variant*')
        variant.BuiltInTypeId = sopc_type
        if not isinstance(self._value, (list, tuple)):
            # Single values
            variant.ArrayType = libsub.SOPC_VariantArrayType_SingleValue
            if sopc_type == libsub.SOPC_Null_Id:
                pass
            elif sopc_type == libsub.SOPC_Boolean_Id:
                variant.Value.Boolean = self._value
            elif sopc_type == libsub.SOPC_SByte_Id:
                variant.Value.Sbyte = self._value
            elif sopc_type == libsub.SOPC_Byte_Id:
                variant.Value.Byte = self._value
            elif sopc_type == libsub.SOPC_Int16_Id:
                variant.Value.Int16 = self._value
            elif sopc_type == libsub.SOPC_UInt16_Id:
                variant.Value.Uint16 = self._value
            elif sopc_type == libsub.SOPC_Int32_Id:
                variant.Value.Int32 = self._value
            elif sopc_type == libsub.SOPC_UInt32_Id:
                variant.Value.Uint32 = self._value
            elif sopc_type == libsub.SOPC_Int64_Id:
                variant.Value.Int64 = self._value
            elif sopc_type == libsub.SOPC_UInt64_Id:
                variant.Value.Uint64 = self._value
            elif sopc_type == libsub.SOPC_Float_Id:
                variant.Value.Floatv = self._value
            elif sopc_type == libsub.SOPC_Double_Id:
                variant.Value.Doublev = self._value
            elif sopc_type == libsub.SOPC_String_Id:
                variant.Value.String = str_to_string(self._value, no_gc=True)[0]
            elif sopc_type == libsub.SOPC_DateTime_Id:
                variant.Value.Date = float_to_datetime(self._value, no_gc=True)[0]
            elif sopc_type == libsub.SOPC_Guid_Id:
                variant.Value.Guid = uuid_to_guid(self._value, no_gc=True)[0]
            elif sopc_type == libsub.SOPC_ByteString_Id:
                variant.Value.Bstring = bytes_to_bytestring(self._value, no_gc=True)[0]
            elif sopc_type == libsub.SOPC_XmlElement_Id:
                variant.Value.XmlElt = bytes_to_bytestring(self._value, no_gc=True)[0]
            elif sopc_type == libsub.SOPC_NodeId_Id:
                variant.Value.NodeId = str_to_nodeid(self._value, no_gc=True)
            #elif sopc_type == libsub.SOPC_ExpandedNodeId_Id:
            #    variant.Value. = self._value
            elif sopc_type == libsub.SOPC_StatusCode_Id:
                variant.Value.Status = self._value
            elif sopc_type == libsub.SOPC_QualifiedName_Id:
                qname = allocator_no_gc('SOPC_QualifiedName *')
                qname.NamespaceIndex = self._value[0]
                qname.Name = str_to_string(self._value[1], no_gc=True)[0]
                variant.Value.Qname = qname
            elif sopc_type == libsub.SOPC_LocalizedText_Id:
                loc = allocator_no_gc('SOPC_LocalizedText *')
                loc.Locale, loc.Text = map(lambda s:str_to_string(s, no_gc=True)[0], self._value)
                variant.Value.LocalizedText = loc
            #elif sopc_type == libsub.SOPC_ExtensionObject_Id:
            #    variant.Value. = self._value
            #elif sopc_type == libsub.SOPC_DataValue_Id:
            #    variant.Value. = self._value
            #elif sopc_type == libsub.SOPC_Variant_Id:
            #    variant.Value. = self._value
            #elif sopc_type == libsub.SOPC_DiagnosticInfo_Id:
            #    variant.Value. = self._value
            else:
                raise ValueError('Python to SOPC_Variant conversion not supported for the given type {}.'.format(sopc_type))
        else:
            # Arrays or Matrices values (but not Matrices)
            assert not any(map(lambda n:isinstance(n, (list, tuple)), self._value)), 'Multi dimensional arrays are not supported.'
            variant.ArrayType = libsub.SOPC_VariantArrayType_Array
            variant.Value.Array.Length = len(self._value)
            content = variant.Value.Array.Content
            if sopc_type == libsub.SOPC_Null_Id:
                pass
            elif sopc_type == libsub.SOPC_Boolean_Id:
                content.BooleanArr = allocator_no_gc('SOPC_Boolean[]', self._value)
            elif sopc_type == libsub.SOPC_SByte_Id:
                content.SbyteArr = allocator_no_gc('SOPC_SByte[]', self._value)
            elif sopc_type == libsub.SOPC_Byte_Id:
                content.ByteArr = allocator_no_gc('SOPC_Byte[]', self._value)
            elif sopc_type == libsub.SOPC_Int16_Id:
                content.Int16Arr = allocator_no_gc('int16_t[]', self._value)
            elif sopc_type == libsub.SOPC_UInt16_Id:
                content.Uint16Arr = allocator_no_gc('uint16_t[]', self._value)
            elif sopc_type == libsub.SOPC_Int32_Id:
                content.Int32Arr = allocator_no_gc('int32_t[]', self._value)
            elif sopc_type == libsub.SOPC_UInt32_Id:
                content.Uint32Arr = allocator_no_gc('uint32_t[]', self._value)
            elif sopc_type == libsub.SOPC_Int64_Id:
                content.Int64Arr = allocator_no_gc('int64_t[]', self._value)
            elif sopc_type == libsub.SOPC_UInt64_Id:
                content.Uint64Arr = allocator_no_gc('uint64_t[]', self._value)
            elif sopc_type == libsub.SOPC_Float_Id:
                content.FloatvArr = allocator_no_gc('float[]', self._value)
            elif sopc_type == libsub.SOPC_Double_Id:
                content.DoublevArr = allocator_no_gc('double[]', self._value)
            elif sopc_type == libsub.SOPC_String_Id:
                content.StringArr = allocator_no_gc('SOPC_String[]', [str_to_string(s, no_gc=True)[0] for s in self._value])
            elif sopc_type == libsub.SOPC_DateTime_Id:
                content.DateArr = allocator_no_gc('SOPC_DateTime[]', [float_to_datetime(s, no_gc=True)[0] for s in self._value])
            elif sopc_type == libsub.SOPC_Guid_Id:
                content.GuidArr = allocator_no_gc('SOPC_Guid[]', [uuid_to_guid(s, no_gc=True)[0] for s in self._value])
            elif sopc_type == libsub.SOPC_ByteString_Id:
                content.BstringArr = allocator_no_gc('SOPC_ByteString[]', [bytes_to_bytestring(v, no_gc=True)[0] for v in self._value])
            elif sopc_type == libsub.SOPC_XmlElement_Id:
                content.XmlEltArr = allocator_no_gc('SOPC_XmlElement[]', [bytes_to_bytestring(v, no_gc=True)[0] for v in self._value])
            elif sopc_type == libsub.SOPC_NodeId_Id:
                content.NodeIdArr = allocator_no_gc('SOPC_NodeId[]', [str_to_nodeid(v, no_gc=True)[0] for v in self._value])
            #elif sopc_type == libsub.SOPC_ExpandedNodeId_Id:
            #    content.Arr = allocator_no_gc('[]', self._value)
            elif sopc_type == libsub.SOPC_StatusCode_Id:
                content.StatusArr = allocator_no_gc('SOPC_StatusCode[]', self._value)
            elif sopc_type == libsub.SOPC_QualifiedName_Id:
                qnames = allocator_no_gc('SOPC_QualifiedName[]', len(self._value))
                for i,v in enumerate(self._value):
                    qnames[i].NamespaceIndex = v[0]
                    qnames[i].Name = str_to_string(v[1], no_gc=True)
                content.QnameArr = qnames
            elif sopc_type == libsub.SOPC_LocalizedText_Id:
                locs = allocator_no_gc('SOPC_LocalizedText[]', len(self._value))
                for i,v in enumerate(self._value):
                    locs[i].Locale, locs[i].Text = map(lambda s:str_to_string(s, no_gc=True), v)
                content.LocalizedTextArr = locs
            #elif sopc_type == libsub.SOPC_ExtensionObject_Id:
            #    content.Arr = allocator_no_gc('[]', self._value)
            #elif sopc_type == libsub.SOPC_DataValue_Id:
            #    content.Arr = allocator_no_gc('[]', self._value)
            #elif sopc_type == libsub.SOPC_Variant_Id:
            #    content.Arr = allocator_no_gc('[]', self._value)
            #elif sopc_type == libsub.SOPC_DiagnosticInfo_Id:
            #    content.Arr = allocator_no_gc('[]', self._value)
            else:
                raise ValueError('Python to SOPC_Variant conversion not supported for the given type {}.'.format(sopc_type))
        return variant


class VariantType:
    """
    A copy of the SOPC_BuiltinId type.
    """
    Null            = libsub.SOPC_Null_Id
    Boolean         = libsub.SOPC_Boolean_Id
    SByte           = libsub.SOPC_SByte_Id
    Byte            = libsub.SOPC_Byte_Id
    Int16           = libsub.SOPC_Int16_Id
    UInt16          = libsub.SOPC_UInt16_Id
    Int32           = libsub.SOPC_Int32_Id
    UInt32          = libsub.SOPC_UInt32_Id
    Int64           = libsub.SOPC_Int64_Id
    UInt64          = libsub.SOPC_UInt64_Id
    Float           = libsub.SOPC_Float_Id
    Double          = libsub.SOPC_Double_Id
    String          = libsub.SOPC_String_Id
    DateTime        = libsub.SOPC_DateTime_Id
    Guid            = libsub.SOPC_Guid_Id
    ByteString      = libsub.SOPC_ByteString_Id
    XmlElement      = libsub.SOPC_XmlElement_Id
    NodeId          = libsub.SOPC_NodeId_Id
    #ExpandedNodeId  = libsub.SOPC_ExpandedNodeId_Id
    StatusCode      = libsub.SOPC_StatusCode_Id
    QualifiedName   = libsub.SOPC_QualifiedName_Id
    LocalizedText   = libsub.SOPC_LocalizedText_Id
    #ExtensionObject = libsub.SOPC_ExtensionObject_Id
    #DataValue       = libsub.SOPC_DataValue_Id
    #Variant         = libsub.SOPC_Variant_Id
    #DiagnosticInfo  = libsub.SOPC_DiagnosticInfo_Id


class DataValue:
    """
    A Python representation of the DataValue.

    Attributes:
        timestampSource: The last time the value changed, specified by the writer.
        timestampServer: The last time the value changed, according to the server.
        statusCode: The quality associated to the value OR the reason why there is no available value (e.g. BadAttributeInvalid).
        variant: The Variant storing the value.
        variant_type: An accessor to the :attr:`Variant.variant_type`, see Variant.
    """
    def __init__(self, timestampSource, timestampServer, statusCode, variant):
        self.timestampSource = timestampSource
        self.timestampServer = timestampServer
        self.statusCode = statusCode
        self.variant = variant

    def __repr__(self):
        return 'DataValue('+repr(self.variant)+')'
    def __str__(self):
        return repr(self)

    @property
    def variant_type(self):
        return self.variant.variant_type
    @variant_type.setter
    def variant_type(self, ty):
        self.variant.variant_type = ty

    @staticmethod
    def from_sopc_libsub_value(libsub_value):
        """
        Converts a SOPC_LibSub_Value* or a SOPC_LibSub_Value to a Python DataValue.
        Its main usage is in the data change callback used with subscriptions.
        See from_sopc_datavalue() for a more generic SOPC_DataValue conversion.
        """
        return DataValue(ntp_to_python(libsub_value.source_timestamp),
                         ntp_to_python(libsub_value.server_timestamp),
                         libsub_value.quality, Variant.from_sopc_variant(libsub_value.raw_value))

    @staticmethod
    def from_sopc_datavalue(datavalue):
        """
        Converts a SOPC_DataValue* or SOPC_DataValue to a Python DataValue.
        """
        return DataValue(datetime_to_float(ffi.new('SOPC_DateTime*', datavalue.SourceTimestamp)),
                         datetime_to_float(ffi.new('SOPC_DateTime*', datavalue.ServerTimestamp)),
                         datavalue.Status, Variant.from_sopc_variant(ffi.addressof(datavalue.Value)))

    @staticmethod
    def from_python(val):
        """
        Creates a DataValue from a Python value or a Variant.
        Creates the Variant, sets the status code to OK, and set source timestamp to now.
        """
        if not isinstance(val, Variant):
            val = Variant(val)
        return DataValue(time.time(), 0, libsub.SOPC_STATUS_OK, val)

    allocator = ffi.new_allocator(alloc=libsub.malloc, free=libsub.SOPC_DataValue_Delete, should_clear_after_alloc=True)

    def to_sopc_datavalue(self, *, copy_type_from_variant=None, sopc_variant_type=None, no_gc=False):
        """
        Converts a new SOPC_DataValue from the Python DataValue.
        See :func:`Variant.to_sopc_variant` for a documentation of the arguments.

        The returned value is garbage collected when the returned value is not referenced anymore.
        """
        if no_gc:
            datavalue = allocator_no_gc('SOPC_DataValue *')
        else:
            datavalue = DataValue.allocator('SOPC_DataValue *')
        sopc_variant = self.variant.to_sopc_variant(copy_type_from_variant=copy_type_from_variant, sopc_variant_type=sopc_variant_type, no_gc=True)
        datavalue.Value = sopc_variant[0]
        datavalue.Status = self.statusCode
        # This allocs an int64_t which must be retained until it is copied in datavalue
        source = float_to_datetime(self.timestampSource)
        server = float_to_datetime(self.timestampServer)
        datavalue.SourceTimestamp = source[0]
        datavalue.ServerTimestamp = server[0]
        datavalue.SourcePicoSeconds = 0
        datavalue.ServerPicoSeconds = 0
        return datavalue


class AttributeId:
    """
    A copy of the SOPC_LibSub_AttributeId enum.
    """
    NodeId                  = libsub.SOPC_LibSub_AttributeId_NodeId
    NodeClass               = libsub.SOPC_LibSub_AttributeId_NodeClass
    BrowseName              = libsub.SOPC_LibSub_AttributeId_BrowseName
    DisplayName             = libsub.SOPC_LibSub_AttributeId_DisplayName
    Description             = libsub.SOPC_LibSub_AttributeId_Description
    WriteMask               = libsub.SOPC_LibSub_AttributeId_WriteMask
    UserWriteMask           = libsub.SOPC_LibSub_AttributeId_UserWriteMask
    IsAbstract              = libsub.SOPC_LibSub_AttributeId_IsAbstract
    Symmetric               = libsub.SOPC_LibSub_AttributeId_Symmetric
    InverseName             = libsub.SOPC_LibSub_AttributeId_InverseName
    ContainsNoLoops         = libsub.SOPC_LibSub_AttributeId_ContainsNoLoops
    EventNotifier           = libsub.SOPC_LibSub_AttributeId_EventNotifier
    Value                   = libsub.SOPC_LibSub_AttributeId_Value
    DataType                = libsub.SOPC_LibSub_AttributeId_DataType
    ValueRank               = libsub.SOPC_LibSub_AttributeId_ValueRank
    ArrayDimensions         = libsub.SOPC_LibSub_AttributeId_ArrayDimensions
    AccessLevel             = libsub.SOPC_LibSub_AttributeId_AccessLevel
    UserAccessLevel         = libsub.SOPC_LibSub_AttributeId_UserAccessLevel
    MinimumSamplingInterval = libsub.SOPC_LibSub_AttributeId_MinimumSamplingInterval
    Historizing             = libsub.SOPC_LibSub_AttributeId_Historizing
    Executable              = libsub.SOPC_LibSub_AttributeId_Executable
    UserExecutable          = libsub.SOPC_LibSub_AttributeId_UserExecutable


class EncodeableType:
    """
    A copy of the known SOPC_EncodeableTypes, give these address a readable name.
    """
    ReadValueId          = ffi.addressof(libsub.OpcUa_ReadValueId_EncodeableType)
    ReadRequest          = ffi.addressof(libsub.OpcUa_ReadRequest_EncodeableType)
    ReadResponse         = ffi.addressof(libsub.OpcUa_ReadResponse_EncodeableType)
    WriteValue           = ffi.addressof(libsub.OpcUa_WriteValue_EncodeableType)
    WriteRequest         = ffi.addressof(libsub.OpcUa_WriteRequest_EncodeableType)
    WriteResponse        = ffi.addressof(libsub.OpcUa_WriteResponse_EncodeableType)
    BrowseRequest        = ffi.addressof(libsub.OpcUa_BrowseRequest_EncodeableType)
    BrowseResponse       = ffi.addressof(libsub.OpcUa_BrowseResponse_EncodeableType)
    ViewDescription      = ffi.addressof(libsub.OpcUa_ViewDescription_EncodeableType)
    BrowseDescription    = ffi.addressof(libsub.OpcUa_BrowseDescription_EncodeableType)
    ReferenceDescription = ffi.addressof(libsub.OpcUa_ReferenceDescription_EncodeableType)
    BrowseResult         = ffi.addressof(libsub.OpcUa_BrowseResult_EncodeableType)


class BrowseResult:
    """
    The BrowseResult is a low-level structures that contains the list of References for a node,
    but also the status code of the Browse operation, and, if needed, a continuation point.
    """
    def __init__(self, sopc_browseresult):
        assert sopc_browseresult.encodeableType == EncodeableType.BrowseResult
        self.status = sopc_browseresult.StatusCode
        self.continuationPoint = bytestring_to_bytes(ffi.addressof(sopc_browseresult.ContinuationPoint))
        self.references = []
        for sopc_ref in [sopc_browseresult.References[i] for i in range(sopc_browseresult.NoOfReferences)]:
            assert sopc_ref.encodeableType == EncodeableType.ReferenceDescription
            refType = nodeid_to_str(ffi.addressof(sopc_ref.ReferenceTypeId))
            fwd = sopc_ref.IsForward
            expNid = expandednodeid_to_str(ffi.addressof(sopc_ref.NodeId))
            bwsName = (sopc_ref.BrowseName.NamespaceIndex, string_to_str(sopc_ref.BrowseName.Name))
            dispName = (string_to_str(sopc_ref.DisplayName.Locale), string_to_str(sopc_ref.DisplayName.Text))
            nodCls = sopc_ref.NodeClass
            typeDef = expandednodeid_to_str(ffi.addressof(sopc_ref.TypeDefinition))
            self.references.append(Reference(refType, fwd, expNid, bwsName, dispName, nodCls, typeDef))

class Reference:
    """
    A Python version for the OpcUa_ReferenceDescription, a low level representation of the references.
    Does not contain the source of the Reference.

    Attributes:
        referenceTypeId: The string NodeId that defines the type of the Reference.
        isForward: True when the reference is forward (going from the browsed node to nodeId),
                   False when the reference is in the inverse direction (from the nodeId to the browsed node).
        nodeId: Target Expanded nodeId of the Reference. When the node is in the address space,
                Expanded NodeId and NodeId have the same string representation.
        browseName: Browse name of the nodeId, as a qualified name, i.e. a couple (namespaceIndex, str).
        displayName: Display name of the nodeId, as a localized text, i.e. a couple (str of the chosen locale, str in this locale).
        nodeClass: NodeClass of the nodeId.
        typeDefinition: NodeId of the type of the target node, when the target NodeId is a Variable or an Object.
                        It defines which VariableType or ObjectType node was used to instantiate the target node.
    """
    def __init__(self, referenceTypeId, isForward, nodeId, browseName, displayName, nodeClass, typeDefinition):
        self.referenceTypeId = referenceTypeId
        self.isForward       = isForward
        self.nodeId          = nodeId
        self.browseName      = browseName
        self.displayName     = displayName
        self.nodeClass       = nodeClass
        self.typeDefinition  = typeDefinition


if __name__ == '__main__':
    # Auto-test
    import struct
    guid = ffi.new('SOPC_Guid *')
    guid.Data1, guid.Data2, guid.Data3 = struct.unpack('>IHH', unhexlify(b'72962B91FA754AE6'))
    guid.Data4 = [0x8D, 0x28, 0xB4, 0x04, 0xDC, 0x7D, 0xAF, 0x63]
    uid = uuid.UUID('72962b91-fa75-4ae6-8d28-b404dc7daf63')
    assert uid == guid_to_uuid(guid)
    assert uid == guid_to_uuid(uuid_to_guid(uid))

    snids = ['s=Counter', 'ns=40001;s=Counter', 'i=2255', 'g=72962b91-fa75-4ae6-8d28-b404dc7daf63', 'b=foobar']
    assert all([snid == nodeid_to_str(str_to_nodeid(snid)) for snid in snids])

    s = 'S2OPC foobar test string'
    assert s == string_to_str(str_to_string(s))
    assert s.encode() == bytestring_to_bytes(bytes_to_bytestring(s.encode()))

    # TODO: test without sopc_variant_type then with copy_type_from_variant
    vals = Variant([2*i - 4 for i in range(64)])
    assert vals == Variant.from_sopc_variant(vals.to_sopc_variant(sopc_variant_type=VariantType.Int16))
    val = Variant('i=81')
    assert val == Variant.from_sopc_variant(val.to_sopc_variant(sopc_variant_type=VariantType.NodeId))
    vals = Variant(['i={}'.format(i) for i in range(64)])
    assert vals == Variant.from_sopc_variant(vals.to_sopc_variant(sopc_variant_type=VariantType.NodeId))
    val = Variant('Foo')
    assert val == Variant.from_sopc_variant(val.to_sopc_variant(sopc_variant_type=VariantType.String))
    vals = Variant(['Foo', 'Bar'])
    assert vals == Variant.from_sopc_variant(vals.to_sopc_variant(sopc_variant_type=VariantType.String))

    t = time.time()
    datetime = ffi.new('SOPC_DateTime*');
    datetime[0] = libsub.SOPC_Time_GetCurrentTimeUTC()
    assert abs(datetime_to_float(datetime) - t) < .1
    assert datetime_to_float(float_to_datetime(t)) == t

    # Thu Nov 30 04:57:25.694287 2034 UTC, unix timestamp is 2048471845.694287
    assert ntp_to_python(18285654237264005879) == 2048471845.694287  # Hopefully with this one there is no float-rounding errors.
