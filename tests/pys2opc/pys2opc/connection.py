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


import time

from _pys2opc import ffi, lib as libsub
from .types import Request, AttributeId, allocator_no_gc, EncodeableType, str_to_nodeid, ReturnStatus, VariantType
from .responses import Response, ReadResponse, WriteResponse, BrowseResponse


class BaseConnectionHandler:
    """
    Base class giving the prototypes of the callbacks,
    and implements the subscription-library connection wrappers.

    The class supports Python's "with" statements.
    In this case, the connection is automatically closed upon exit of the context.
    """
    def __init__(self, connId, configuration):
        self._id = connId
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

    _dResponseClasses = {EncodeableType.ReadResponse: ReadResponse,
                         EncodeableType.WriteResponse: WriteResponse,
                         EncodeableType.BrowseResponse: BrowseResponse,
                         }

    def _on_response(self, event, status, responsePayload, responseContext, timestamp):
        """
        Receives an OpcUa_*Response, creates a Response, associates it to a Request both-ways.
        It is called for every response received through the LibSub callback_generic_event.
        The dictionary _dResponseClasses contains classes that will be instantiated with the OpcUa_*Response as parameter.
        It is possible to add new elements to this dict to support more response decoders, or override existing decoders.

        Warning: responsePayload is freed by the caller, so the structure or its content must be copied before returning.

        The timestamp parameters is computed on the first line of the event callback,
        hence it is the most accurate instant when the response was received by the Python layer.
        """
        assert responseContext in self._dRequestContexts, 'Unknown requestContext {}.'.format(responseContext)
        request = self._dRequestContexts.pop(responseContext)
        try:
            if event == libsub.SOPC_LibSub_ApplicativeEvent_SendFailed:
                self._connected = False  # Prevent further sends
                self.disconnect()  # Explicitly disconnects to free S²OPC resources
                raise RuntimeError('Request was not sent with status 0x{:08X}'.format(status))
            assert event == libsub.SOPC_LibSub_ApplicativeEvent_Response
            # Build typed response
            encType = ffi.cast('SOPC_EncodeableType**', responsePayload)
            response = self._dResponseClasses.get(encType[0], Response)(responsePayload)
            response.timestampReceived = timestamp  # Passing the timestamp instead of acquiring it here reduces it by ~10µs
            request.response = response
            response.request = request
            if responseContext not in self._sSkipResponse:
                self.on_generic_response(request, response)
            else:
                self._sSkipResponse.remove(responseContext)
        finally:
            # Hopefully the Toolkit always notifies the application, and it is caught here.
            # Also, if the processing of the response fails, it is caught here.
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
        assert request.requestContext not in self._dPendingResponses,\
            'A request with context {} is still waiting for a response'.format(request.requestContext)
        self._dPendingResponses[request.requestContext] = response

    # Disconnection
    def disconnect(self):
        """
        Disconnects the current connexion, and release its resources.
        Returns True if the disconnection was successful.
        """
        # The Toolkit will still call the on_disconnect() callback afterwards.
        status = libsub.SOPC_LibSub_Disconnect(self._id)
        return status == ReturnStatus.OK

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
        This call is always synchroneous, so that the Toolkit waits for the server response to return.
        """
        # TODO: check format?
        if nodeIds:
            n = len(nodeIds)
            lszNodeIds = [ffi.new('char[]', nid.encode()) for nid in nodeIds]
            lAttrIds = ffi.new('SOPC_LibSub_AttributeId[{}]'.format(n), [libsub.SOPC_LibSub_AttributeId_Value for _ in nodeIds])
            lDataIds = ffi.new('SOPC_LibSub_DataId[]', n)
            status = libsub.SOPC_LibSub_AddToSubscription(self._id, lszNodeIds, lAttrIds, n, lDataIds)
            assert status == ReturnStatus.OK, 'Add to subscription failed with status {}'.format(status)
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
        assert status == ReturnStatus.OK, 'AsyncSendRequestOnSession failed with status {}'.format(status)
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
    def read_nodes(self, nodeIds, attributes=None, bWaitResponse=True):
        """
        Forges an OpcUa_ReadRequest and sends it.
        When bWaitResponse, waits for and returns the ReadResponse,
        which contains the attribute results storing the read value for the ith element.
        Otherwise, returns the request.

        Args:
            nodeIds: NodeId described as a string "[ns=x;]t=y" where x is the namespace index, t is the NodeId type
                     (s for a string NodeId, i for integer, b for bytestring, g for GUID), and y is typed content.
            attributes: Optional: a list of attributes to read. The list has the same length as nodeIds. When omited,
                        reads the attribute Value (see :class:`AttributeId` for a list of attributes).
        """
        if attributes is None:
            attributes = [AttributeId.Value for _ in nodeIds]
        assert len(nodeIds) == len(attributes)
        # TODO: protect this from invalid attributes ?
        payload = allocator_no_gc('OpcUa_ReadRequest *')  # The Toolkit takes ownership of this struct
        payload.encodeableType = EncodeableType.ReadRequest
        payload.MaxAge = 0.
        payload.TimestampsToReturn = libsub.OpcUa_TimestampsToReturn_Both
        payload.NoOfNodesToRead = len(nodeIds)
        nodesToRead = allocator_no_gc('OpcUa_ReadValueId[]', len(nodeIds))
        for i, (snid, attr) in enumerate(zip(nodeIds, attributes)):
            nodesToRead[i].encodeableType = EncodeableType.ReadValueId
            nodesToRead[i].NodeId = str_to_nodeid(snid, no_gc=True)[0]
            nodesToRead[i].AttributeId = attr
        payload.NodesToRead = nodesToRead

        request = Request(payload)
        return self.send_generic_request(request, bWaitResponse=bWaitResponse)

    def write_nodes(self, nodeIds, datavalues, attributes=None, types=None, bWaitResponse=True):
        """
        Forges an OpcUa_WriteResponse and sends it.
        When bWaitResponse, waits for  and returns the WriteResponse,
        which has accessors to check whether the writes were successful or not.
        Otherwise, returns the request.

        Types are found in 3 places, for each NodeId and DataValue :
        - in each datavalue.variantType,
        - in the `types` list,
        - in a ReadResponse that is sent and waited upon if both previous sources are set to None.

        The Read request is only sent when at least one datavalue lacks type in both datavalue.variantType and `types`.
        If both datavalue.variantType and the type in `types` are given, they must be equal.

        Args:
            nodeIds: NodeId described as a string "[ns=x;]t=y" where x is the namespace index, t is the NodeId type
                     (s for a string NodeId, i for integer, b for bytestring, g for GUID), and y is typed content.
            datavalues: A list of DataValue to write for each NodeId
            attributes: Optional: a list of attributes to write. The list has the same length as nodeIds. When omitted,
                        reads the attribute Value (see :class:`AttributeId` for a list of attributes).
            types: Optional: a list of VariantType for each value to write.
        """
        if attributes is None:
            attributes = [AttributeId.Value for _ in nodeIds]
        assert len(nodeIds) == len(attributes) == len(datavalues)
        if types:
            assert len(nodeIds) == len(types)

        # Compute types
        sopc_types = []
        types = types or [None] * len(nodeIds)
        for dv, ty in zip(datavalues, types):
            if dv.variantType is not None:
                if ty is not None and ty != dv.variantType:
                    raise ValueError('Inconsistent type, type of datavalue is different from type given in types list')
                sopc_types.append(dv.variantType)
            else:
                sopc_types.append(ty)

        # Where there is still None types, make a read request
        missingTypesInfo = [(i, snid, attr) for i,(snid,attr,ty) in enumerate(zip(nodeIds, attributes, sopc_types)) if ty is None]
        if missingTypesInfo:
            _, readNids, readAttrs = zip(*missingTypesInfo)
            readDatavalues = self.read_nodes(readNids, readAttrs, bWaitResponse=True)
            for (i, _, _), dv in zip(missingTypesInfo, readDatavalues.results):
                assert dv.variantType != VariantType.Null, 'Automatic type detection failed, null type read.'
                sopc_types[i] = dv.variantType

        # Overwrite values' type
        for dv, ty in zip(datavalues, sopc_types):
            dv.variantType = ty

        # Prepare the request, it will be freed by the Toolkit
        payload = allocator_no_gc('OpcUa_WriteRequest *')
        payload.encodeableType = EncodeableType.WriteRequest
        payload.NoOfNodesToWrite = len(nodeIds)
        nodesToWrite = allocator_no_gc('OpcUa_WriteValue[]', len(nodeIds))
        for i, (snid, attr, val) in enumerate(zip(nodeIds, attributes, datavalues)):
            nodesToWrite[i].encodeableType = EncodeableType.WriteValue
            nodesToWrite[i].NodeId = str_to_nodeid(snid, no_gc=True)[0]
            nodesToWrite[i].AttributeId = attr
            nodesToWrite[i].Value = val.to_sopc_datavalue(no_gc=True)[0]
        payload.NodesToWrite = nodesToWrite

        request = Request(payload)
        return self.send_generic_request(request, bWaitResponse=bWaitResponse)

    def browse_nodes(self, nodeIds, maxReferencesPerNode=1000, bWaitResponse=True):
        """
        Forges an OpcUa_BrowseResponse and sends it.
        When bWaitResponse, waits for  and returns the BrowseResponse,
        which has a list BrowseResults in its `results` list.
        Otherwise, returns the request.
        """
        # Prepare the request, it will be freed by the Toolkit
        payload = allocator_no_gc('OpcUa_BrowseRequest *')
        payload.encodeableType = EncodeableType.BrowseRequest
        view = allocator_no_gc('OpcUa_ViewDescription *')
        view.encodeableType = EncodeableType.ViewDescription  # Leave the ViewDescription filled with NULLs
        payload.View = view[0]
        payload.RequestedMaxReferencesPerNode = maxReferencesPerNode
        payload.NoOfNodesToBrowse = len(nodeIds)
        nodesToBrowse = allocator_no_gc('OpcUa_BrowseDescription[]', len(nodeIds))
        for i, snid in enumerate(nodeIds):
            nodesToBrowse[i].encodeableType = EncodeableType.BrowseDescription
            nodesToBrowse[i].NodeId = str_to_nodeid(snid, no_gc=True)[0]
            nodesToBrowse[i].BrowseDirection = libsub.OpcUa_BrowseDirection_Both
            nodesToBrowse[i].IncludeSubtypes = False
            nodesToBrowse[i].NodeClassMask = 0xFF  # See Part4 §5.8.2 Browse, §.2 Parameters
            nodesToBrowse[i].ResultMask = libsub.OpcUa_BrowseResultMask_All
        payload.NodesToBrowse = nodesToBrowse

        request = Request(payload)
        return self.send_generic_request(request, bWaitResponse=bWaitResponse)

    #def history_read_nodes(self, nodeIds, bWaitResponse=True):
    #    return self.send_generic_request(request, bWaitResponse=bWaitResponse)
