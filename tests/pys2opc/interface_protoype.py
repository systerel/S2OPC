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



#################################################
# Main module
#################################################

@contextlib.contextmanager
def initialize():
    """
    Toolkit initialization.
    Must be called exactly once per process.
    """
    SOPC_Toolkit_Initialize()
    try:
        yield
    finally:
        clear()

def add_configuration_unsecured(parameters):
    """
    Returns a configuration that can be later used in connect() or get_endpoints().
    """
    if not configured:
        return ClientConfiguration(params, cfg_id)

def add_configuration_secured(parameters, security_parameters):
    if not configured:
        return ClientConfiguration(params, cfg_id)

def configured():
    """
    Must be called after all calls to add_configuration_unsecured() and add_configuration_secured(),
    and before connect() or get_endpoints().

    This tells S2OPC that the configuration phase is over.
    """
    pass

def get_endpoints(configuration):
    """
    Optional call to fetch the endpoints of the server through the configuration.
    """
    pass

# It should be optional to use the with context manager. With high number of connection,
#  it would be very hard to keep track of all connections.
def connect(config, ConnectionHandler):
    """
    Connects to the server with the given configuration.
    The ConnectionHandler is a class that at least overrides the callbacks.
    It will be instantiated and the instance is returned.
    """
    # Keep track of existing connections to be able to close them when clear()
    return ConnectionHandler()

def clear():
    """
    Disconnect current servers and clears the Toolkit.
    Existing Configurations and Connections are then invalid and may be freed.
    """
    pass


#################################################
# Module Configuration
#################################################

class ClientConfiguration(NamedTuple):
    """
    To be defined, mostly static variables.
    """
    pass


#################################################
# Module Connection
#################################################

class BaseConnectionHandler:
    """
    Base class giving the prototypes of the callbacks,
    and implements the LibSub connection wrappers.

    The class will support Python's "with" statements:
    >>> with connect() as connection:
    ...     # Do things here
    ...     pass
    >>> # When getting out of the with statement (error or logic),
    ... # the disconnect() method is automatically called.
    """
    # Internals
    def _on_response(self, response):
        """Associates a Response to a Request"""
        request = self._dRequestContexts[uid]
        response.timestampReceived = time.time()
        request.response = response
        response.request = request
        if unknown_response_type:
            self.on_generic_response(response)
        request.eventResponseReceived.set()
    def _wait_for_response(self, request):
        request.eventResponseReceived.wait()
        return request.response
    def __enter__(self):
        return self
    def __exit__(self, *exc):
        self.disconnect()

    # Disconnection
    def disconnect(self):
        """
        The Toolkit will still call the on_disconnect() callback afterwards.
        """
        pass

    # Callbacks to override
    def on_datachange(self, nodeid, dataValue):
        """
        dataValue is a SOPC_DataValue, it contains the value, the source and server timestamps if available, and the status code.
        """
        raise NotImplementedError
    def on_disconnect(self):
        raise NotImplementedError
    def on_generic_response(self, request, response):
        """
        This callback is called when the class receives a Response it does not known how to handle otherwise.
        """
        raise NotImplementedError

    # Subscription
    def add_nodes_to_subscription(self, nodeids):
        """
        Subscribe to a list of NodeIds in the OPC-UA format:
        - "s=Foobar" for a string NodeId,
        - "i=42" for an integer NodeId,
        - "g=C496578A-0DFE-4b8f-870A-745238C6AEAE" for a GUID-NodeId,
        - "b=Barbar" for a ByteString.
        The string can be prepend by a "ns={};" which specifies the namespace index.
        """
        pass

    # Generic request sender
    def send_generic_request(self, request, bWaitResponse):
        """
        When bWaitResponse, waits for the response and returns it. Otherwise, returns the request.
        """
        request.timestampSent = time.time()
        self._dRequestContexts[new_uid] = request
        if bWaitResponse:
            return self._wait_for_response(request)
        else:
            return request

    # Specialized request sender
    def read_nodes(self, nodeidsAttributes, bWaitResponse):
        """
        When bWaitResponse, waits for the response and returns it. Otherwise, returns the request.
        """
        request = ReadRequest(params)
        obj = self.send_generic_request(request, bWaitResponse)
        if bWaitResponse:
            return zip(*zip(*nodeidsAttributes), response.values)
    def write_nodes(self, nodeidsAttributesValues, bWaitResponse):
        request = WriteRequest(params)
        return self.send_generic_request(request, bWaitResponse)
    def browse_nodes(self, nodeids, bWaitResponse):
        request = BrowseRequest(params)
        return self.send_generic_request(request, bWaitResponse)
    def history_read_nodes(self, nodeids, bWaitResponse):
        request = HistoryReadRequest(params)
        return self.send_generic_request(request, bWaitResponse)


#################################################
# Utils module
#################################################

class Variant:
    @staticmethod
    def from_c_variant(pVariant):
        pass

    def to_c_variant(self):
        return pVariant

class DataValue:
    # The value is stored as Variant().
    def __init__(self, timestampSource, timestampServer, statusCode, value):
        self.timestampSource = timestampSource
        self.timestampServer = timestampServer
        self.statusCode = statusCode
        self.value = value


#################################################
# Module Messages
#################################################

class Request:
    """
    Base class for Requests. Adds a timestamp to ease the performance measurement.
    """
    def __init__(self):
        self.timestampSent
        self.response
        self.eventResponseReceived = threading.Event()

# I am not sure these class will be useful.
class ReadRequest(Request):
    # With wrappers in main?
    pass
class WriteRequest(Request):
    pass
class BrowseRequest(Request):
    pass
class HistoryReadRequest(Request):
    pass


class Response:
    """
    Base class for Responses.
    Adds a reference to the request and the timestamp of the received time.
    """
    def __init__(self):
        self.timestampReceived
        self.request
    def get_roundtrip_time(self):
        return self.timestampReceived - self.request.timestampSent

