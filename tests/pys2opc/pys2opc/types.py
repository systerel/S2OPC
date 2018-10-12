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



class Variant:
    def __init__(self, python_value):
        self.python_value = python_value

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
