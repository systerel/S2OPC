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


from contextlib import contextmanager
import time

from _pys2opc import ffi, lib as libsub

NULL = ffi.NULL
SOPC_STATUS_OK = libsub.SOPC_STATUS_OK

# TODO: make this configurable
LOG_LEVEL = libsub.SOPC_TOOLKIT_LOG_LEVEL_DEBUG

@ffi.def_extern()
def _callback_log(level, text):
    """
    Receives log information from the LibSub (not from the S2OPC toolkit).
    """
    dLevel = {libsub.SOPC_TOOLKIT_LOG_LEVEL_ERROR: '# Error: ',
              libsub.SOPC_TOOLKIT_LOG_LEVEL_WARNING: '# Warning: ',
              libsub.SOPC_TOOLKIT_LOG_LEVEL_INFO: '# Info: ',
              libsub.SOPC_TOOLKIT_LOG_LEVEL_DEBUG: '# Debug: '}
    if level <= LOG_LEVEL:
        print(dLevel[level] + ffi.string(text).decode())

@ffi.def_extern()
def _callback_disconnected(clientId):
    return PyS2OPC._callback_disconnected(clientId)

@ffi.def_extern()
def _callback_datachanged(connectionId, dataId, c_value):
    return PyS2OPC._callback_datachanged(connectionId, dataId, c_value)

@ffi.def_extern()
def _callback_generic_event(connectionId, event, status, responsePayload, responseContext):
    return PyS2OPC._callback_generic_event(connectionId, event, status, responsePayload, responseContext)



class PyS2OPC:
    """
    Wraps the C library. Stores the global status for this purpose.
    """
    _initialized = False
    _configured = False
    _dConfigurations = {}  # Stores known configurations {Id: configurationParameters}
    _dConnections = {}  # Stores known active connections {Id: ConnectionHandler()}

    @staticmethod
    def get_version():
        """Returns version string"""
        return ffi.string(libsub.SOPC_LibSub_GetVersion()).decode()

    @staticmethod
    @contextmanager
    def initialize():
        """
        Toolkit and LibSub initializations.
        Must be called exactly once per process.

        This function supports the context management:
        >>> with PyS2OPC.initialize():
        ...     # Do things here
        ...     pass

        When reaching out of the `with` statement, the Toolkit is automatically cleared.
        See clear().
        """
        assert not PyS2OPC._initialized, 'Library is alread initialized'

        status = libsub.SOPC_LibSub_Initialize([(libsub._callback_log, libsub._callback_disconnected)])
        assert status == SOPC_STATUS_OK, 'Library initialization failed with status code {}.'.format(status)
        PyS2OPC._initialized = True

        try:
            yield
        finally:
            PyS2OPC.clear()

    @staticmethod
    def clear():
        """
        Disconnect current servers and clears the Toolkit.
        Existing Configurations and Connections are then invalid and may be freed.
        """
        # TODO: Disconnect existing clients
        libsub.SOPC_LibSub_Clear()
        PyS2OPC._initialized = False

    @staticmethod
    def add_configuration_unsecured(*,
                                    server_url = 'opc.tcp://localhost:4841',
                                    publish_period = 500,
                                    n_max_keepalive = 3,
                                    n_max_lifetime = 10,
                                    timeout_ms = 10000,
                                    sc_lifetime = 3600000,
                                    token_target = 3):
        """
        Returns a configuration that can be later used in connect() or get_endpoints().

        Args:
            server_url: The endpoint and server url to connect to.
            publish_period: The period of the subscription, in ms.
            n_max_keepalive: The number of times the subscription has no notification to send before
                             sending an empty PublishResponse (the KeepAlive message). It is necessary
                             to keep `n_max_keepalive*timeout_ms*token_target < 5000ms`.
            n_max_lifetime: The maximum number of times a subscription has notifications to send
                            but no available token. In this case, the subscription is destroyed.
            timeout_ms: The connect() timeout, in ms.
            sc_lifetime: The target lifetime of the secure channel, before renewal, in ms.
            token_target: The number of subscription tokens (PublishRequest) that should be
                          made available to the server at anytime.
        """
        assert PyS2OPC._initialized and not PyS2OPC._configured,\
            'Toolkit is not initialized or already configured, cannot add new configurations.'

        pCfgId = ffi.new('SOPC_LibSub_ConfigurationId *')
        dConnectionParameters = {'server_url': ffi.new('char[]', server_url.encode()),
                                 'security_policy': libsub.SOPC_SecurityPolicy_None_URI,
                                 'security_mode': libsub.OpcUa_MessageSecurityMode_None,
                                 'disable_certificate_verification': True,
                                 'path_cert_auth': NULL,
                                 'path_cert_srv': NULL,
                                 'path_cert_cli': NULL,
                                 'path_key_cli': NULL,
                                 'path_crl': NULL,
                                 'policyId': ffi.new('char[]', b"anonymous"),
                                 'username': NULL,
                                 'password': NULL,
                                 'publish_period_ms': publish_period,
                                 'n_max_keepalive': n_max_keepalive,
                                 'n_max_lifetime': n_max_lifetime,
                                 'data_change_callback': libsub._callback_datachanged,
                                 'timeout_ms': timeout_ms,
                                 'sc_lifetime': sc_lifetime,
                                 'token_target': token_target,
                                 'generic_response_callback': libsub._callback_generic_event}
        #import pdb; pdb.set_trace()
        status = libsub.SOPC_LibSub_ConfigureConnection([dConnectionParameters], pCfgId)
        assert status == SOPC_STATUS_OK, 'Configuration failed with status {}.'.format(status)

        cfgId = pCfgId[0]
        config = ClientConfiguration(cfgId, dConnectionParameters)
        PyS2OPC._dConfigurations[cfgId] = config
        return config

    #def add_configuration_secured(parameters, security_parameters):


    @staticmethod
    def configured():
        """
        Must be called after all calls to add_configuration_unsecured() and add_configuration_secured(),
        and before connect() or get_endpoints().

        This tells S2OPC that the configuration phase is over.
        """
        assert PyS2OPC._initialized and not PyS2OPC._configured,\
            'Toolkit is not initialized or already configured, cannot add new configurations.'
        assert libsub.SOPC_LibSub_Configured() == libsub.SOPC_STATUS_OK
        PyS2OPC._configured = True


    @staticmethod
    def get_endpoints(configuration):
        """
        Optional call to fetch the endpoints of the server through the configuration.

        Not implemented yet.
        """
        pass


    @staticmethod
    def connect(configuration, ConnectionHandlerClass):
        """
        Connects to the server with the given `configuration`.
        The configuration is returned by a call to add_configuration_unsecured().
        The ConnectionHandlerClass is a class that must be inherited from BaseConnectionHandler,
        and that at least overrides the callbacks.
        It will be instantiated and the instance is returned.

        It can be optionally used in a `with` statement, which automatically disconnects the connection.
        """
        assert PyS2OPC._initialized and PyS2OPC._configured, 'Toolkit not configured, cannot connect().'
        assert isinstance(configuration, ClientConfiguration)
        cfgId = configuration._id
        assert cfgId in PyS2OPC._dConfigurations, 'Unknown configuration, see add_configuration_unsecured().'
        assert issubclass(ConnectionHandlerClass, BaseConnectionHandler)

        pConId = ffi.new('SOPC_LibSub_DataId *')
        status = libsub.SOPC_LibSub_Connect(cfgId, pConId)
        if status != libsub.SOPC_STATUS_OK:
            raise ConnectionError('Could not connect to the server with the given configuration.')

        connectionId = pConId[0]
        assert connectionId not in PyS2OPC._dConnections,\
            'Subscription library returned a connection id that is already taken by an active connection.'

        connection = ConnectionHandlerClass(connectionId, configuration)
        PyS2OPC._dConnections[connectionId] = connection
        return connection

    @staticmethod
    def _callback_disconnected(clientId):
        if clientId not in PyS2OPC._dConnections:
            print('Disconnected unknown client', clientId)
            return
        connection = PyS2OPC._dConnections.pop(clientId)
        connection.on_disconnect()

    @staticmethod
    def _callback_datachanged(connectionId, dataId, c_value):
        value = DataValue.from_sopc_libsub_value(c_value)
        #print('Data changed (connection {}, data_id {}), new value {}'.format(connection_id, data_id, value.value))
        assert connectionId in PyS2OPC._dConnections, 'Data change notification on unknown connection'
        connection = PyS2OPC._dConnections[connectionId]
        connection._on_datachanged(dataId, value)

    @staticmethod
    def _callback_generic_event(connectionId, event, status, responsePayload, responseContext):
        ts = time.time()
        assert connectionId in PyS2OPC._dConnections, 'Data change notification on unknown connection'
        connection = PyS2OPC._dConnections[connectionId]
        response = Response(responsePayload)
        response.timestampReceived = ts
        connection._on_response(event, status, response, responseContext)



class ClientConfiguration:
    """
    Stores configuration given to the subscription library.
    Such configurations should not be modified, as modifying these will have no impact on the Toolkit.
    """
    def __init__(self, id, parameters):
        self._id = id
        self._parameters = parameters

    @property
    def parameters(self):
        """
        Returns a copy of the parameters given to the configuration.
        Modifying these will have no effect on the configuration whatsoever.
        """
        return self._parameters.copy()


#################################################
# Module Connection
#################################################

class BaseConnectionHandler:
    """
    Base class giving the prototypes of the callbacks,
    and implements the subscription-library connection wrappers.

    The class supports Python's "with" statements.
    In this case, the connection is automatically closed upon exit of the context.
    """
    def __init__(self, id, configuration):
        self._id = id
        self.configuration = configuration
        self._dRequestContexts = {}  # Stores requests by their context {requestContext: Request()}
        self._dPendingResponses = {}  # Stores available responses {requestContext: Response()}. See get_response()
        self._sSkipResponse = set()  # Stores the requestContext of Responses that shall not be stored in _dequeResponses.
        self._connected = True
        self._dSubscription = {}  # Associates data_id to string NodeId

    # Internals
    def _on_datachanged(self, dataId, value):
        """
        Internal wrapper, calls on_datachanged() with a string NodeId.
        """
        assert dataId in self._dSubscription, 'Data change notification on unknown NodeId'
        self.on_datachanged(self._dSubscription[dataId], value)

    def _on_response(self, event, status, response, responseContext):
        """
        Receive a Response, associates it to a Request both-ways.
        Shall be called for every response received through callback_generic_event.
        """
        assert responseContext in self._dRequestContexts, 'Unknown requestContext {}.'.format(responseContext)
        request = self._dRequestContexts[responseContext]
        response.timestampReceived = time.time()
        request.response = response
        response.request = request
        if responseContext not in self._sSkipResponse:
            self.on_generic_response(response)
        else:
            del self._sSkipResponse[responseContext]
        request.eventResponseReceived.set()
    def _wait_for_response(self, request):
        request.eventResponseReceived.wait()
        return request.response
    def __enter__(self):
        return self
    def __exit__(self, *exc):
        self.disconnect()

    # Callbacks
    def on_datachanged(self, nodeId, dataValue):
        """
        dataValue is copied from the SOPC_DataValue: it contains the value,
        the source and server timestamps if available, and the status code.
        """
        raise NotImplementedError
    def on_disconnect(self):
        """
        Called when the disconnection of this connection is effective.
        """
        self._connected = False

    def on_generic_response(self, request, response):
        """
        This callback is called when the class receives a Response that is not waited upon.
        It is possible to not override it.

        The default implementation of this method stores the response in a double-end-queue
        which tracks available responses (see pop_response).
        It is possible to not call the on_generic_response of the parent class.
        """
        self._dequeResponses.append(response)

    # Disconnection
    def disconnect(self):
        """
        Disconnects the current connexion, and release its resources.
        Returns True if the disconnection was successful.
        """
        # The Toolkit will still call the on_disconnect() callback afterwards.
        status = libsub.SOPC_LibSub_Disconnect(self._id)
        return status == libsub.SOPC_STATUS_OK

    @property
    def connected(self):
        return self._connected

    # Subscription
    def add_nodes_to_subscription(self, nodeIds):
        """
        Subscribe to a list of string of NodeIds in the OPC-UA format:
        - "i=42" for an integer NodeId,
        - "s=Foobar" for a string NodeId,
        - "g=C496578A-0DFE-4b8f-870A-745238C6AEAE" for a GUID-NodeId,
        - "b=Barbar" for a ByteString.
        The string can be prepend by a "ns={};" which specifies the namespace index.
        """
        # TODO: check format?
        if nodeIds:
            n = len(nodeIds)
            lszNodeIds = [ffi.new('char[]', nid.encode()) for nid in nodeIds]
            lAttrIds = ffi.new('SOPC_LibSub_AttributeId[{}]'.format(n), [libsub.SOPC_LibSub_AttributeId_Value for _ in nodeIds])
            lDataIds = ffi.new('SOPC_LibSub_DataId[{}]'.format(n))
            status = libsub.SOPC_LibSub_AddToSubscription(self._id, lszNodeIds, lAttrIds, n, lDataIds)
            assert status == libsub.SOPC_STATUS_OK, 'Add to subscription failed with status {}'.format(status)
            for i, nid in zip(lDataIds, nodeIds):
                assert i not in self._dSubscription, 'data_id returned by Toolkit is already associated to a NodeId.'
                self._dSubscription[i] = nid

    # Generic request sender
    def send_generic_request(self, request, bWaitResponse):
        """
        Sends a request. When bWaitResponse:
        - waits for the response and returns it,
        - otherwise, returns the request, and the response will be available through get_response().
        """
        reqCtx = request.requestContext
        self._dRequestContexts[reqCtx] = request
        request.timestampSent = time.time()
        status = libsub.SOPC_LibSub_AsyncSendRequestOnSession(self._id, request.payload, request.requestContext)
        if bWaitResponse:
            self._sSkipResponse.add(reqCtx)
            return self._wait_for_response(request)
        else:
            return request

    def get_response(self, request):
        """
        Pops the response to the request from the store of available response.
        Returns the response if there is an available response.
        Otherwise returns None.
        """
        return self._dPendingResponses.pop(request.requestContext, None)

    # Specialized request sender
    def read_nodes(self, nodeidsAttributes, bWaitResponse=False):
        """
        When bWaitResponse, waits for the response and returns it. Otherwise, returns the request.
        """
        request = ReadRequest(params)
        obj = self.send_generic_request(request, bWaitResponse=False)
        #if bWaitResponse:
        #    return zip(*zip(*nodeidsAttributes), response.values)
    def write_nodes(self, nodeidsAttributesValues, bWaitResponse=False):
        # TODO:
        request = WriteRequest(params)
        return self.send_generic_request(request, bWaitResponse=False)
    def browse_nodes(self, nodeids, bWaitResponse):
        request = BrowseRequest(params)
        return self.send_generic_request(request, bWaitResponse=False)
    def history_read_nodes(self, nodeids, bWaitResponse):
        request = HistoryReadRequest(params)
        return self.send_generic_request(request, bWaitResponse=False)


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
