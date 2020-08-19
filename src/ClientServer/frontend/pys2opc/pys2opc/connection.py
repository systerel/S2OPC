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
from .types import ReturnStatus
from .request import Request, LibSubAsyncRequestHandler


class BaseClientConnectionHandler(LibSubAsyncRequestHandler):
    """
    Base class giving the prototypes of the callbacks,
    and implements the subscription-library connection wrappers.

    The class supports Python's "with" statements.
    In this case, the connection is automatically closed upon exit of the context.
    """
    def __init__(self, connId, configuration):
        super().__init__()
        self._id = connId
        self.configuration = configuration
        self._connected = True
        self._dSubscription = {}  # Associates data_id to string NodeId

    def __enter__(self):
        return self
    def __exit__(self, *exc):
        self.disconnect()

    # Internals
    def _on_datachanged(self, dataId, value):
        """
        Internal wrapper, calls on_datachanged() with a string NodeId.
        """
        assert dataId in self._dSubscription, 'Data change notification on unknown NodeId'
        self.on_datachanged(self._dSubscription[dataId], value)

    def _on_disconnect(self):
        """
        Internal wrapper, calls on_disconnect()
        """
        self._connected = False
        self.on_disconnect()

    def _on_response(self, event, status, responsePayload, responseContext, timestamp):
        if event == libsub.SOPC_LibSub_ApplicativeEvent_SendFailed:
            self._connected = False  # Prevent further sends
            self.disconnect()  # Explicitly disconnects to free S²OPC resources
            super._on_response(None, responseContext, timestamp)  # Still signal that the response is received
            raise RuntimeError('Request was not sent with status 0x{:08X}'.format(status))
        assert event == libsub.SOPC_LibSub_ApplicativeEvent_Response
        super()._on_response(responsePayload, responseContext, timestamp)

    # Callbacks
    def on_datachanged(self, nodeId, dataValue):
        """
        This callback is called upon reception of a value change for a node.
        See `pys2opc.connection.BaseClientConnectionHandler.add_nodes_to_subscription`.

        `nodeId` is the string containing the NodeId of the changed node and
        `dataValue` is the new value (see `pys2opc.types.DataValue`).
        """
        raise NotImplementedError
    def on_disconnect(self):
        """
        Called when the disconnection of this connection is effective.
        """
        pass

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
        """
        Returns whether this connection is still active and usable.
        """
        return self._connected

    # Subscription
    def add_nodes_to_subscription(self, nodeIds):
        """
        Subscribe to a list of string of NodeIds in the OPC-UA format (see `pys2opc` module documentation).
        This call is always synchroneous, so that the Toolkit waits for the server response to return.

        The callback `pys2opc.connection.BaseClientConnectionHandler.on_datachanged` will be called once for each new value of the nodes.
        In particular, the callback is at least called once for the initial value.
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

    # Specialized request sender
    def read_nodes(self, nodeIds, attributes=None, bWaitResponse=True):
        """
        Forges an `OpcUa_ReadRequest` and sends it.
        When `bWaitResponse`, waits for and returns the `pys2opc.responses.ReadResponse`,
        which contains the attribute results storing the read value for the ith element.
        Otherwise, returns the `pys2opc.request.Request`.

        See `pys2opc.request.Request.new_read_request` for details on the other arguments.
        """
        request = Request.new_read_request(nodeIds, attributes=attributes)
        return self.send_generic_request(self._id, request, bWaitResponse=bWaitResponse)

    def write_nodes(self, nodeIds, datavalues, attributes=None, types=None, bWaitResponse=True, bAutoTypeWithRead=True):
        """
        Forges an `OpcUa_WriteResponse` and sends it.
        When `bWaitResponse`, waits for  and returns the `pys2opc.responses.WriteResponse`,
        which has accessors to check whether the writes were successful or not.
        Otherwise, returns the `pys2opc.request.Request`.

        When `bAutoTypeWithRead`, this method tries to determine missing `datavalues` types (see `pys2opc.types.DataValues`).
        It sends a blocking `ReadRequest` first on the `nodeIds` and `attributes`, and deduce the types from the response.
        The request is only sent when at least one datavalue lacks type in both `datavalue.variantType` and `types`.
        The type deduction may fail if the node does not exist or if the value is a `null`.

        See `pys2opc.request.Request.new_write_request` for details on the other arguments.

        Note:
            As `bAutoTypeWithRead` sends a blocking request, it should not be used with `bWaitResponse = False`.

        Note:
            The `datavalue.variantType` is updated with elements from the `types` or from the automatic read request.
        """
        # Where there are unknown types, makes a read request first
        if bAutoTypeWithRead:
            sendFct = lambda request,**kwargs: self.send_generic_request(self._id, request, **kwargs)
            types = Request.helper_maybe_read_types(nodeIds, datavalues, attributes, types, sendFct)

        # Make the actual write request
        request = Request.new_write_request(nodeIds, datavalues, attributes=attributes, types=types)
        return self.send_generic_request(self._id, request, bWaitResponse=bWaitResponse)

    def browse_nodes(self, nodeIds, maxReferencesPerNode=1000, bWaitResponse=True):
        """
        Forges an `OpcUa_BrowseResponse` and sends it.
        When `bWaitResponse`, waits for  and returns the `pys2opc.responses.BrowseResponse`,
        which has a list `pys2opc.types.BrowseResult`s in its `results` attribute.
        Otherwise, returns the `pys2opc.request.Request`.

        See `pys2opc.request.Request.new_browse_request` for details on the other arguments.
        """
        request = Request.new_browse_request(nodeIds, maxReferencesPerNode=maxReferencesPerNode)
        return self.send_generic_request(self._id, request, bWaitResponse=bWaitResponse)

    #def history_read_nodes(self, nodeIds, bWaitResponse=True):
    #    return self.send_generic_request(self._id, request, bWaitResponse=bWaitResponse)
