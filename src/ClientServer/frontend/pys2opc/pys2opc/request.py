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
import time

from _pys2opc import ffi, lib as libsub
from .types import ReturnStatus, EncodeableType, allocator_no_gc, AttributeId, str_to_nodeid, VariantType
from .responses import Response, ReadResponse, WriteResponse, BrowseResponse


class Request:
    """
    Base class for Requests. Adds a timestamp to ease the performance measurement.
    Also provides class functions to create new requests more easily.

    Args:
        payload: An `OpcUa_*Request`.

    Attributes:
        eventResponseReceived: Event that is set when the response is received (`AsyncRequestHandler.on_generic_response` called for this `Request`).
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
        """Returns an `uintptr_t`, that is castable to Python int and usable by the libsub API"""
        return self._requestContext

    @staticmethod
    def new_read_request(nodeIds, attributes=None):
        """
        Forges an `OpcUa_ReadRequest` and returns the corresponding `Request`.

        Args:
            nodeIds: A list of NodeIds described as a strings (see `pys2opc` module documentation).
            attributes: Optional: a list of attributes to read. The list has the same length as nodeIds. When omited,
                        reads the `Value` attribute (see `pys2opc.types.AttributeId` for a list of attributes).
        """
        if attributes is None:
            attributes = [AttributeId.Value for _ in nodeIds]
        assert len(nodeIds) == len(attributes),\
            'There should the same number of NodeIds, attributes, and datavalues when reading nodes'
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

        return Request(payload)

    @staticmethod
    def new_write_request(nodeIds, datavalues, attributes=None, types=None):
        """
        Forges an `OpcUa_WriteResponse` and returns the corresponding `Request`.

        Types for `datavalues` must be provided.
        For each `pys2opc.types.DataValue`, the type is either found in `datavalue.variantType`, or in the `types` list.
        If both `datavalue.variantType` and the type in `types` are given, they must be equal.

        Note:
            The `datavalue.variantType` is updated with elements from the `types`.

        Args:
            nodeIds: A list of NodeIds described as a strings (see `pys2opc` module documentation).
            datavalues: A list of `pys2opc.types.DataValue` to write for each NodeId, see `pys2opc.types.DataValue.from_python`
            attributes: Optional: a list of attributes to write. The list has the same length as nodeIds. When omitted,
                        writes the `Value` attribute (see `pys2opc.types.AttributeId` for a list of attributes).
            types: Optional: a list of `pys2opc.types.VariantType` for each value to write.
        """
        if attributes is None:
            attributes = [AttributeId.Value for _ in nodeIds]
        assert len(nodeIds) == len(attributes) == len(datavalues),\
            'There should the same number of NodeIds, attributes, and datavalues when writing nodes'
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
        assert None not in sopc_types, 'Incomplete type information, cannot create write request'

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

        return Request(payload)

    @staticmethod
    def new_browse_request(nodeIds, maxReferencesPerNode=1000):
        """
        Forges an `OpcUa_BrowseResponse` and returns the corresponding `Request`.

        Args:
            nodeIds: A list of NodeIds described as a strings (see `pys2opc` module documentation).
            maxReferencesPerNode: Optional: The maximum number of returned references per node to browse.
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

        return Request(payload)

    def helper_maybe_read_types(nodeIds, datavalues, attributes, types, sendFct):
        """
        Internal helper that makes a `Request` to read the missing types, if any, in the provided `datavalues` and `types` list.
        Return the type list.
        Used by `write_nodes` implementations.
        """
        # Note: this function is here to avoid copy/paste in users of new_write_request that wish to use the "auto-type" functionality.
        #  The sendFct hints that this function may not be in the optimal place.

        if attributes is None:
            attributes = [AttributeId.Value for _ in nodeIds]
        assert len(nodeIds) == len(attributes) == len(datavalues),\
            'There should the same number of NodeIds, attributes, and datavalues when reading nodes'
        if types:
            assert len(nodeIds) == len(types)
        else:
            types = [None] * len(nodeIds)

        # Compute missing types, send the request, and update the missing types.
        sopc_types = [dv.variantType if dv.variantType is not None else ty for dv,ty in zip(datavalues, types)]
        missingTypesInfo = [(i, snid, attr) for i,(snid,attr,ty) in enumerate(zip(nodeIds, attributes, sopc_types)) if ty is None]
        if missingTypesInfo:
            _, readNids, readAttrs = zip(*missingTypesInfo)
            request = Request.new_read_request(readNids, readAttrs)
            readDatavalues = sendFct(request, bWaitResponse=True)
            for (i, _, _), dv in zip(missingTypesInfo, readDatavalues.results):
                assert dv.variantType != VariantType.Null, 'Automatic type detection failed, null type read.'
                sopc_types[i] = dv.variantType

        return sopc_types


class AsyncRequestHandler:
    """
    MixIn that implements asynchronous request handling: associates a response to a request.

    This should be derived to implement the `_send_request` method.
    See `request.LibSubAsyncRequestHandler` and `request.LocalAsyncRequestHandler`.
    """
    _dResponseClasses = {EncodeableType.ReadResponse: ReadResponse,
                         EncodeableType.WriteResponse: WriteResponse,
                         EncodeableType.BrowseResponse: BrowseResponse,
                         }

    def __init__(self):
        # TODO: Rework and simplify the distinction between skipped responses / waited response / generic response
        self._dRequestContexts = {}  # Stores requests by their context {requestContext: Request()}
        self._dPendingResponses = {}  # Stores available responses {requestContext: Response()}. See get_response()
        self._sSkipResponse = set()  # Stores the requestContext of Responses that shall not be stored in _dequeResponses.

    def _send_request(self, idx, request):
        """
        Wrapper to the C-function that effectively send the request.
        This must be re-implemented and may change according to the API used to send the request.
        """
        raise NotImplementedError

    def send_generic_request(self, idx, request, bWaitResponse):
        """
        Sends a `request` on link with index `idx` (either a connection id or an endpoint id).
        When `bWaitResponse`, waits for the response and returns it.
        Otherwise, returns the `request`, and the response will be available through `get_response`.
        """
        reqCtx = int(request.requestContext)
        self._dRequestContexts[reqCtx] = request
        request.timestampSent = time.time()
        self._send_request(idx, request)
        if bWaitResponse:
            self._sSkipResponse.add(reqCtx)
            return self._wait_for_response(request)
        else:
            return request

    def _on_response(self, responsePayload, responseContext, timestamp):
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
            if responsePayload is None:
                return
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
        return response

    def on_generic_response(self, request, response):
        """
        This callback is called when the class receives a Response that is not waited upon.
        It is possible to not override it.
        It is possible to not call the on_generic_response of the parent class.

        The default implementation of this method stores the response in a double-end-queue
        which tracks available responses (see pop_response).
        """
        # TODO: Upd doc
        assert request.requestContext not in self._dPendingResponses,\
            'A request with context {} is still waiting for a response'.format(request.requestContext)
        self._dPendingResponses[request.requestContext] = response

    def _wait_for_response(self, request):
        # TODO: Rework and simplify the distinction between skipped responses / waited response / generic response
        request.eventResponseReceived.wait()
        return request.response

    def get_response(self, request):
        """
        Pops the response to the request from the store of available response.
        Returns the response if there is an available response.
        Otherwise returns None.
        """
        # TODO: Rework and simplify the distinction between skipped responses / waited response / generic response
        return self._dPendingResponses.pop(request.requestContext, None)


class LibSubAsyncRequestHandler(AsyncRequestHandler):
    """
    MixIn that implements asynchronous request handling: associates a response to a request.
    Specialized to use the client API.
    """
    def _send_request(self, connId, request):
        status = libsub.SOPC_LibSub_AsyncSendRequestOnSession(connId, request.payload, request.requestContext)
        assert status == ReturnStatus.OK, 'AsyncSendRequestOnSession failed with status {}'.format(ReturnStatus.get_both_from_id(status))


class LocalAsyncRequestHandler(AsyncRequestHandler):
    """
    MixIn that implements asynchronous request handling: associates a response to a request.
    Specialized to use the "local request" API.
    """
    def _send_request(self, epId, request):
        libsub.SOPC_ToolkitServer_AsyncLocalServiceRequest(epId, request.payload, request.requestContext)
