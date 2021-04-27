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

"""
Server side callbacks base classes.
"""

from . import ffi, libsub
from .types import EncodeableType, nodeid_to_str, string_to_str, DataValue#, UserAuthentication, UserAuthorization


class BaseAddressSpaceHandler:
    """
    Base class for the Address Space notification callback.
    You should derive this class and re-implement its `BaseAddressSpaceHandler.on_datachanged`.
    """
    def _on_datachanged(self, ctx, event, write_value, status):
        """
        Internal, translates the input from the C call to something easier to use.
        Please see `BaseAddressSpaceHandler.on_datachanged`.
        """
        # Note: ctx might be used to acces session user / security mode and policy of connection used to write
        assert event == libsub.AS_WRITE_EVENT, 'Only address space write events are supported for now, received 0x{:X}'.format(event)
        wvi = ffi.cast('OpcUa_WriteValue *', write_value)
        assert wvi.encodeableType == EncodeableType.WriteValue
        self.on_datachanged(nodeid_to_str(ffi.addressof(wvi.NodeId)), wvi.AttributeId,
                            DataValue.from_sopc_datavalue(wvi.Value),
                            string_to_str(wvi.IndexRange), status)

    def on_datachanged(self, nodeId, attrId, dataValue, indexRange, status):
        """
        This is the main callback for the address space notifications (write events).
        The notification is called each time a `WriteRequest` is treated by the server.
        The corresponding `OpcUa_WriteValue` is split accross the arguments for convenience.

        There are no notifications from local writes,
        as they are suppressed by the C library.

        You must re-implement this callback.

        Args:
            nodeId: The written NodeId as a string (see [NodeId Concept](/pys2opc#nodeid-concept))
            attrId: The written `pys2opc.types.AttributeId`
            value: The new `pys2opc.types.DataValue`
            indexRange: An index range (string) for the DataValue (if any, the DataValue should only have this length)
            status: The `pys2opc.types.StatusCode` of this operation that will be returned to the client.
                    This differs from the status code of the value, which is contained in the DataValue.
        """
        raise NotImplementedError()


class BaseUserHandler:
    """
    Base class for the authentications and authorizations of users.
    You should derive this class and re-implement its `BaseUserHandler.on_user_authentication` and `BaseUserHandler.on_user_authorization`.
    """
    pass
    #def _on_user_authentication(self, authenticationManager, pUser, pUserAuthenticated):
    #    # TODO: This call must be as fast as possible, to avoid stalling the toolkit
    #    return ReturnStatus.NOT_SUPPORTED

    #def _on_user_authorisation(self, authorizationManager, operationType, nodeId, attributeId, pUser, pbOperationAuthorized):
    #    # TODO: This call must be as fast as possible, to avoid stalling the toolkit
    #    return ReturnStatus.NOT_SUPPORTED


# TODO: class BaseMethodHandler:
