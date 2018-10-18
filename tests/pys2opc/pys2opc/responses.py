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
from .types import EncodeableType, DataValue, BrowseResult


class Response:
    """
    Base class for Responses.
    Adds a reference to the request and the timestamp of the received time.

    Args:
        payload: An OpcUa_*Response. The payload is optional. If the payload is given, its content
                 is shallow copied, if known.
    """
    def __init__(self, payload):
        self.timestampReceived = None  # The receiver sets the timestamp
        self.request = None
        self.payload = payload

    def get_roundtrip_time(self):
        return self.timestampReceived - self.request.timestampSent


class ReadResponse(Response):
    """
    Parses an OpcUa_ReadResponse.

    Attributes:
        results: List of DataValues corresponding to the read values. You should not modify elements of this list.
    """
    def __init__(self, payload):
        super().__init__(None)
        payload = ffi.cast('OpcUa_ReadResponse*', payload)
        assert payload.encodeableType == EncodeableType.ReadResponse
        self.results = [DataValue.from_sopc_datavalue(payload.Results[i]) for i in range(payload.NoOfResults)]


class WriteResponse(Response):
    """
    Parses an OpcUa_WriteResponse.

    Attributes:
        results: List of StatusCode corresponding to the written values. See is_ok().
    """
    def __init__(self, payload):
        super().__init__(None)
        payload = ffi.cast('OpcUa_WriteResponse*', payload)
        assert payload.encodeableType == EncodeableType.WriteResponse
        self.results = [payload.Results[i] for i in range(payload.NoOfResults)]

    def is_ok(self):
        """
        Returns True if all writes were done successfully.
        """
        return all(res == libsub.SOPC_STATUS_OK for res in self.results)


class BrowseResponse(Response):
    """
    Parses an OpcUa_BrowseResponse.

    Attributes:
        results: A list of BrowseResults.
    """
    def __init__(self, payload):
        super().__init__(None)
        payload = ffi.cast('OpcUa_BrowseResponse*', payload)
        assert payload.encodeableType == EncodeableType.BrowseResponse
        self.results = [BrowseResult(payload.Results[i]) for i in range(payload.NoOfResults)]
