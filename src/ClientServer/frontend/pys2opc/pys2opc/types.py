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
import uuid
from binascii import hexlify, unhexlify
import time

from _pys2opc import ffi, lib as libsub


allocator_no_gc = ffi.new_allocator(alloc=libsub.SOPC_Malloc, free=None, should_clear_after_alloc=True)


class NamedMembers:
    """
    This class or its subclass is capable of returning the name of a member of this class from an id.
    `NamedMembers.get_name_from_id` is particularly useful to translate OPC UA constants to readable strings.
    The function `NamedMembers.get_both_from_id` returns a string which contains both the formatted code and its string decode.
    """
    _dCodeNames = None
    @classmethod
    def get_name_from_id(cls, memberId):
        """
        Returns the name of the class member that has the given `memberId`.
        Exclude getters and private members.
        """
        if cls._dCodeNames is None:
            cls._dCodeNames = {getattr(cls, name): name for name in dir(cls) if not name.startswith('get_') and not name.startswith('_')}
        return cls._dCodeNames[memberId]

    @classmethod
    def get_both_from_id(cls, memberId):
        """
        Returns a string "0x01234567 (NAME_OF_ID)" which gives both the hexa-formatted Id ans its name.
        """
        return '0x{:08X} ({})'.format(memberId, cls.get_name_from_id(memberId))


class ReturnStatus(NamedMembers):
    """
    The operation return statuses. These are accessors to the SOPC_ReturnStatus C enum.
    """
    OK                 = libsub.SOPC_STATUS_OK
    NOK                = libsub.SOPC_STATUS_NOK
    INVALID_PARAMETERS = libsub.SOPC_STATUS_INVALID_PARAMETERS
    INVALID_STATE      = libsub.SOPC_STATUS_INVALID_STATE
    ENCODING_ERROR     = libsub.SOPC_STATUS_ENCODING_ERROR
    WOULD_BLOCK        = libsub.SOPC_STATUS_WOULD_BLOCK
    TIMEOUT            = libsub.SOPC_STATUS_TIMEOUT
    OUT_OF_MEMORY      = libsub.SOPC_STATUS_OUT_OF_MEMORY
    CLOSED             = libsub.SOPC_STATUS_CLOSED
    NOT_SUPPORTED      = libsub.SOPC_STATUS_NOT_SUPPORTED


class StatusCode(NamedMembers):
    """
    The OpcUa status codes. Directly generated from src/Common/opcua_types/opcua_statuscodes.h.
    Status codes are used in various places among the OPC UA protocol.
    They usually represent the quality of a value (see `pys2opc.types.DataValue`),
    or the status of the execution of a service (see `pys2opc.responses.WriteResponse`).
    """
    # Adds the generic good and bad values, which are only defined as masks in the OPC-UA protocol.
    Good = 0x00000000
    Uncertain = 0x40000000
    Bad = 0x80000000
    # Obtained with the following code snippet from S2OPC sources:
    # for com, k, v in sorted(re.findall(r'/\*=+\n \* (.*)\n \*.+\n#define OpcUa_(\w+) (\w+)', open('src/Common/opcua_types/opcua_statuscodes.h').read(), re.MULTILINE), key=lambda t:int(t[2], 16)): print('    {} = {}  # {}'.format(k, v, com))
    GoodSubscriptionTransferred = 0x002D0000  # The subscription was transferred to another session.
    GoodCompletesAsynchronously = 0x002E0000  # The processing will complete asynchronously.
    GoodOverload = 0x002F0000  # Sampling has slowed down due to resource limitations.
    GoodClamped = 0x00300000  # The value written was accepted but was clamped.
    GoodLocalOverride = 0x00960000  # The value has been overridden.
    GoodEntryInserted = 0x00A20000  # The data or event was successfully inserted into the historical database.
    GoodEntryReplaced = 0x00A30000  # The data or event field was successfully replaced in the historical database.
    GoodNoData = 0x00A50000  # No data exists for the requested time range or event filter.
    GoodMoreData = 0x00A60000  # The data or event field was successfully replaced in the historical database.
    GoodCommunicationEvent = 0x00A70000  # The communication layer has raised an event.
    GoodShutdownEvent = 0x00A80000  # The system is shutting down.
    GoodCallAgain = 0x00A90000  # The operation is not finished and needs to be called again.
    GoodNonCriticalTimeout = 0x00AA0000  # A non-critical timeout occurred.
    GoodDataIgnored = 0x00D90000  # The request pecifies fields which are not valid for the EventType or cannot be saved by the historian.
    GoodEdited = 0x00DC0000  # The value does not come from the real source and has been edited by the server.
    GoodPostActionFailed = 0x00DD0000  # There was an error in execution of these post-actions.
    GoodDependentValueChanged = 0x00E00000  # A dependent value has been changed but the change has not been applied to the device.
    UncertainReferenceOutOfServer = 0x406C0000  # One of the references to follow in the relative path references to a node in the address space in another server.
    UncertainNoCommunicationLastUsableValue = 0x408F0000  # Communication to the data source has failed. The variable value is the last value that had a good quality.
    UncertainLastUsableValue = 0x40900000  # Whatever was updating this value has stopped doing so.
    UncertainSubstituteValue = 0x40910000  # The value is an operational value that was manually overwritten.
    UncertainInitialValue = 0x40920000  # The value is an initial value for a variable that normally receives its value from another variable.
    UncertainSensorNotAccurate = 0x40930000  # The value is at one of the sensor limits.
    UncertainEngineeringUnitsExceeded = 0x40940000  # The value is outside of the range of values defined for this parameter.
    UncertainSubNormal = 0x40950000  # The value is derived from multiple sources and has less than the required number of Good sources.
    UncertainDataSubNormal = 0x40A40000  # The value is derived from multiple values and has less than the required number of Good values.
    UncertainReferenceNotDeleted = 0x40BC0000  # The server was not able to delete all target references.
    UncertainNotAllNodesAvailable = 0x40C00000  # The list of references may not be complete because the underlying system is not available.
    UncertainDominantValueChanged = 0x40DE0000  # The related EngineeringUnit has been changed but the Variable Value is still provided based on the previous unit.
    BadUnexpectedError = 0x80010000  # An unexpected error occurred.
    BadInternalError = 0x80020000  # An internal error occurred as a result of a programming or configuration error.
    BadOutOfMemory = 0x80030000  # Not enough memory to complete the operation.
    BadResourceUnavailable = 0x80040000  # An operating system resource is not available.
    BadCommunicationError = 0x80050000  # A low level communication error occurred.
    BadEncodingError = 0x80060000  # Encoding halted because of invalid data in the objects being serialized.
    BadDecodingError = 0x80070000  # Decoding halted because of invalid data in the stream.
    BadEncodingLimitsExceeded = 0x80080000  # The message encoding/decoding limits imposed by the stack have been exceeded.
    BadUnknownResponse = 0x80090000  # An unrecognized response was received from the server.
    BadTimeout = 0x800A0000  # The operation timed out.
    BadServiceUnsupported = 0x800B0000  # The server does not support the requested service.
    BadShutdown = 0x800C0000  # The operation was cancelled because the application is shutting down.
    BadServerNotConnected = 0x800D0000  # The operation could not complete because the client is not connected to the server.
    BadServerHalted = 0x800E0000  # The server has stopped and cannot process any requests.
    BadNothingToDo = 0x800F0000  # There was nothing to do because the client passed a list of operations with no elements.
    BadTooManyOperations = 0x80100000  # The request could not be processed because it specified too many operations.
    BadDataTypeIdUnknown = 0x80110000  # The extension object cannot be (de)serialized because the data type id is not recognized.
    BadCertificateInvalid = 0x80120000  # The certificate provided as a parameter is not valid.
    BadSecurityChecksFailed = 0x80130000  # An error occurred verifying security.
    BadCertificateTimeInvalid = 0x80140000  # The Certificate has expired or is not yet valid.
    BadCertificateIssuerTimeInvalid = 0x80150000  # An Issuer Certificate has expired or is not yet valid.
    BadCertificateHostNameInvalid = 0x80160000  # The HostName used to connect to a Server does not match a HostName in the Certificate.
    BadCertificateUriInvalid = 0x80170000  # The URI specified in the ApplicationDescription does not match the URI in the Certificate.
    BadCertificateUseNotAllowed = 0x80180000  # The Certificate may not be used for the requested operation.
    BadCertificateIssuerUseNotAllowed = 0x80190000  # The Issuer Certificate may not be used for the requested operation.
    BadCertificateUntrusted = 0x801A0000  # The Certificate is not trusted.
    BadCertificateRevocationUnknown = 0x801B0000  # It was not possible to determine if the Certificate has been revoked.
    BadCertificateIssuerRevocationUnknown = 0x801C0000  # It was not possible to determine if the Issuer Certificate has been revoked.
    BadCertificateRevoked = 0x801D0000  # The certificate has been revoked.
    BadCertificateIssuerRevoked = 0x801E0000  # The issuer certificate has been revoked.
    BadUserAccessDenied = 0x801F0000  # User does not have permission to perform the requested operation.
    BadIdentityTokenInvalid = 0x80200000  # The user identity token is not valid.
    BadIdentityTokenRejected = 0x80210000  # The user identity token is valid but the server has rejected it.
    BadSecureChannelIdInvalid = 0x80220000  # The specified secure channel is no longer valid.
    BadInvalidTimestamp = 0x80230000  # The timestamp is outside the range allowed by the server.
    BadNonceInvalid = 0x80240000  # The nonce does appear to be not a random value or it is not the correct length.
    BadSessionIdInvalid = 0x80250000  # The session id is not valid.
    BadSessionClosed = 0x80260000  # The session was closed by the client.
    BadSessionNotActivated = 0x80270000  # The session cannot be used because ActivateSession has not been called.
    BadSubscriptionIdInvalid = 0x80280000  # The subscription id is not valid.
    BadRequestHeaderInvalid = 0x802A0000  # The header for the request is missing or invalid.
    BadTimestampsToReturnInvalid = 0x802B0000  # The timestamps to return parameter is invalid.
    BadRequestCancelledByClient = 0x802C0000  # The request was cancelled by the client.
    BadNoCommunication = 0x80310000  # Communication with the data source is defined, but not established, and there is no last known value available.
    BadWaitingForInitialData = 0x80320000  # Waiting for the server to obtain values from the underlying data source.
    BadNodeIdInvalid = 0x80330000  # The syntax of the node id is not valid.
    BadNodeIdUnknown = 0x80340000  # The node id refers to a node that does not exist in the server address space.
    BadAttributeIdInvalid = 0x80350000  # The attribute is not supported for the specified Node.
    BadIndexRangeInvalid = 0x80360000  # The syntax of the index range parameter is invalid.
    BadIndexRangeNoData = 0x80370000  # No data exists within the range of indexes specified.
    BadDataEncodingInvalid = 0x80380000  # The data encoding is invalid.
    BadDataEncodingUnsupported = 0x80390000  # The server does not support the requested data encoding for the node.
    BadNotReadable = 0x803A0000  # The access level does not allow reading or subscribing to the Node.
    BadNotWritable = 0x803B0000  # The access level does not allow writing to the Node.
    BadOutOfRange = 0x803C0000  # The value was out of range.
    BadNotSupported = 0x803D0000  # The requested operation is not supported.
    BadNotFound = 0x803E0000  # A requested item was not found or a search operation ended without success.
    BadObjectDeleted = 0x803F0000  # The object cannot be used because it has been deleted.
    BadNotImplemented = 0x80400000  # Requested operation is not implemented.
    BadMonitoringModeInvalid = 0x80410000  # The monitoring mode is invalid.
    BadMonitoredItemIdInvalid = 0x80420000  # The monitoring item id does not refer to a valid monitored item.
    BadMonitoredItemFilterInvalid = 0x80430000  # The monitored item filter parameter is not valid.
    BadMonitoredItemFilterUnsupported = 0x80440000  # The server does not support the requested monitored item filter.
    BadFilterNotAllowed = 0x80450000  # A monitoring filter cannot be used in combination with the attribute specified.
    BadStructureMissing = 0x80460000  # A mandatory structured parameter was missing or null.
    BadEventFilterInvalid = 0x80470000  # The event filter is not valid.
    BadContentFilterInvalid = 0x80480000  # The content filter is not valid.
    BadFilterOperandInvalid = 0x80490000  # The operand used in a content filter is not valid.
    BadContinuationPointInvalid = 0x804A0000  # The continuation point provide is longer valid.
    BadNoContinuationPoints = 0x804B0000  # The operation could not be processed because all continuation points have been allocated.
    BadReferenceTypeIdInvalid = 0x804C0000  # The operation could not be processed because all continuation points have been allocated.
    BadBrowseDirectionInvalid = 0x804D0000  # The browse direction is not valid.
    BadNodeNotInView = 0x804E0000  # The node is not part of the view.
    BadServerUriInvalid = 0x804F0000  # The ServerUri is not a valid URI.
    BadServerNameMissing = 0x80500000  # No ServerName was specified.
    BadDiscoveryUrlMissing = 0x80510000  # No DiscoveryUrl was specified.
    BadSempahoreFileMissing = 0x80520000  # The semaphore file specified by the client is not valid.
    BadRequestTypeInvalid = 0x80530000  # The security token request type is not valid.
    BadSecurityModeRejected = 0x80540000  # The security mode does not meet the requirements set by the Server.
    BadSecurityPolicyRejected = 0x80550000  # The security policy does not meet the requirements set by the Server.
    BadTooManySessions = 0x80560000  # The server has reached its maximum number of sessions.
    BadUserSignatureInvalid = 0x80570000  # The user token signature is missing or invalid.
    BadApplicationSignatureInvalid = 0x80580000  # The signature generated with the client certificate is missing or invalid.
    BadRequestCancelledByRequest = 0x805A0000  # The request was cancelled by the client with the Cancel service.
    BadParentNodeIdInvalid = 0x805B0000  # The parent node id does not to refer to a valid node.
    BadReferenceNotAllowed = 0x805C0000  # The reference could not be created because it violates constraints imposed by the data model.
    BadNodeIdExists = 0x805E0000  # The requested node id is already used by another node.
    BadNodeClassInvalid = 0x805F0000  # The node class is not valid.
    BadBrowseNameInvalid = 0x80600000  # The browse name is invalid.
    BadBrowseNameDuplicated = 0x80610000  # The browse name is not unique among nodes that share the same relationship with the parent.
    BadNodeAttributesInvalid = 0x80620000  # The node attributes are not valid for the node class.
    BadTypeDefinitionInvalid = 0x80630000  # The type definition node id does not reference an appropriate type node.
    BadSourceNodeIdInvalid = 0x80640000  # The source node id does not reference a valid node.
    BadTargetNodeIdInvalid = 0x80650000  # The target node id does not reference a valid node.
    BadDuplicateReferenceNotAllowed = 0x80660000  # The reference type between the nodes is already defined.
    BadInvalidSelfReference = 0x80670000  # The server does not allow this type of self reference on this node.
    BadReferenceLocalOnly = 0x80680000  # The reference type is not valid for a reference to a remote server.
    BadNoDeleteRights = 0x80690000  # The server will not allow the node to be deleted.
    BadServerIndexInvalid = 0x806A0000  # The server index is not valid.
    BadViewIdUnknown = 0x806B0000  # The view id does not refer to a valid view node.
    BadTooManyMatches = 0x806D0000  # The requested operation has too many matches to return.
    BadQueryTooComplex = 0x806E0000  # The requested operation requires too many resources in the server.
    BadNoMatch = 0x806F0000  # The requested operation has no match to return.
    BadMaxAgeInvalid = 0x80700000  # The max age parameter is invalid.
    BadHistoryOperationInvalid = 0x80710000  # The history details parameter is not valid.
    BadHistoryOperationUnsupported = 0x80720000  # The server does not support the requested operation.
    BadWriteNotSupported = 0x80730000  # The server not does support writing the combination of value, status and timestamps provided.
    BadTypeMismatch = 0x80740000  # The value supplied for the attribute is not of the same type as the attribute's value.
    BadMethodInvalid = 0x80750000  # The method id does not refer to a method for the specified object.
    BadArgumentsMissing = 0x80760000  # The client did not specify all of the input arguments for the method.
    BadTooManySubscriptions = 0x80770000  # The server has reached its  maximum number of subscriptions.
    BadTooManyPublishRequests = 0x80780000  # The server has reached the maximum number of queued publish requests.
    BadNoSubscription = 0x80790000  # There is no subscription available for this session.
    BadSequenceNumberUnknown = 0x807A0000  # The sequence number is unknown to the server.
    BadMessageNotAvailable = 0x807B0000  # The requested notification message is no longer available.
    BadInsufficientClientProfile = 0x807C0000  # The Client of the current Session does not support one or more Profiles that are necessary for the Subscription.
    BadTcpServerTooBusy = 0x807D0000  # The server cannot process the request because it is too busy.
    BadTcpMessageTypeInvalid = 0x807E0000  # The type of the message specified in the header invalid.
    BadTcpSecureChannelUnknown = 0x807F0000  # The SecureChannelId and/or TokenId are not currently in use.
    BadTcpMessageTooLarge = 0x80800000  # The size of the message specified in the header is too large.
    BadTcpNotEnoughResources = 0x80810000  # There are not enough resources to process the request.
    BadTcpInternalError = 0x80820000  # An internal error occurred.
    BadTcpEndpointUrlInvalid = 0x80830000  # The Server does not recognize the QueryString specified.
    BadRequestInterrupted = 0x80840000  # The request could not be sent because of a network interruption.
    BadRequestTimeout = 0x80850000  # Timeout occurred while processing the request.
    BadSecureChannelClosed = 0x80860000  # The secure channel has been closed.
    BadSecureChannelTokenUnknown = 0x80870000  # The token has expired or is not recognized.
    BadSequenceNumberInvalid = 0x80880000  # The sequence number is not valid.
    BadConfigurationError = 0x80890000  # There is a problem with the configuration that affects the usefulness of the value.
    BadNotConnected = 0x808A0000  # The variable should receive its value from another variable, but has never been configured to do so.
    BadDeviceFailure = 0x808B0000  # There has been a failure in the device/data source that generates the value that has affected the value.
    BadSensorFailure = 0x808C0000  # There has been a failure in the sensor from which the value is derived by the device/data source.
    BadOutOfService = 0x808D0000  # The source of the data is not operational.
    BadDeadbandFilterInvalid = 0x808E0000  # The deadband filter is not valid.
    BadRefreshInProgress = 0x80970000  # This Condition refresh failed, a Condition refresh operation is already in progress.
    BadConditionAlreadyDisabled = 0x80980000  # This condition has already been disabled.
    BadConditionDisabled = 0x80990000  # Property not available, this condition is disabled.
    BadEventIdUnknown = 0x809A0000  # The specified event id is not recognized.
    BadNoData = 0x809B0000  # No data exists for the requested time range or event filter.
    BadDataLost = 0x809D0000  # Data is missing due to collection started/stopped/lost.
    BadEntryExists = 0x809F0000  # The data or event was not successfully inserted because a matching entry exists.
    BadNoEntryExists = 0x80A00000  # The data or event was not successfully updated because no matching entry exists.
    BadInvalidArgument = 0x80AB0000  # One or more arguments are invalid.
    BadConnectionRejected = 0x80AC0000  # Could not establish a network connection to remote server.
    BadDisconnect = 0x80AD0000  # The server has disconnected from the client.
    BadConnectionClosed = 0x80AE0000  # The network connection has been closed.
    BadInvalidState = 0x80AF0000  # The operation cannot be completed because the object is closed, uninitialized or in some other invalid state.
    BadEndOfStream = 0x80B00000  # Cannot move beyond end of the stream.
    BadNoDataAvailable = 0x80B10000  # No data is currently available for reading from a non-blocking stream.
    BadWaitingForResponse = 0x80B20000  # The asynchronous operation is waiting for a response.
    BadOperationAbandoned = 0x80B30000  # The asynchronous operation was abandoned by the caller.
    BadExpectedStreamToBlock = 0x80B40000  # The stream did not return all data requested (possibly because it is a non-blocking stream).
    BadWouldBlock = 0x80B50000  # Non blocking behaviour is required and the operation would block.
    BadSyntaxError = 0x80B60000  # A value had an invalid syntax.
    BadMaxConnectionsReached = 0x80B70000  # The operation could not be finished because all available connections are in use.
    BadRequestTooLarge = 0x80B80000  # The request message size exceeds limits set by the server.
    BadResponseTooLarge = 0x80B90000  # The response message size exceeds limits set by the client.
    BadEventNotAcknowledgeable = 0x80BB0000  # The event cannot be acknowledged.
    BadInvalidTimestampArgument = 0x80BD0000  # The defined timestamp to return was invalid.
    BadProtocolVersionUnsupported = 0x80BE0000  # The applications do not have compatible protocol versions.
    BadStateNotActive = 0x80BF0000  # The sub-state machine is not currently active.
    BadFilterOperatorInvalid = 0x80C10000  # An unregognized operator was provided in a filter.
    BadFilterOperatorUnsupported = 0x80C20000  # A valid operator was provided, but the server does not provide support for this filter operator.
    BadFilterOperandCountMismatch = 0x80C30000  # The number of operands provided for the filter operator was less then expected for the operand provided.
    BadFilterElementInvalid = 0x80C40000  # The referenced element is not a valid element in the content filter.
    BadFilterLiteralInvalid = 0x80C50000  # The referenced literal is not a valid value.
    BadIdentityChangeNotSupported = 0x80C60000  # The Server does not support changing the user identity assigned to the session.
    BadNotTypeDefinition = 0x80C80000  # The provided Nodeid was not a type definition nodeid.
    BadViewTimestampInvalid = 0x80C90000  # The view timestamp is not available or not supported.
    BadViewParameterMismatch = 0x80CA0000  # The view parameters are not consistent with each other.
    BadViewVersionInvalid = 0x80CB0000  # The view version is not available or not supported.
    BadConditionAlreadyEnabled = 0x80CC0000  # This condition has already been enabled.
    BadDialogNotActive = 0x80CD0000  # The dialog condition is not active.
    BadDialogResponseInvalid = 0x80CE0000  # The response is not valid for the dialog.
    BadConditionBranchAlreadyAcked = 0x80CF0000  # The condition branch has already been acknowledged.
    BadConditionBranchAlreadyConfirmed = 0x80D00000  # The condition branch has already been confirmed.
    BadConditionAlreadyShelved = 0x80D10000  # The condition has already been shelved.
    BadConditionNotShelved = 0x80D20000  # The condition is not currently shelved.
    BadShelvingTimeOutOfRange = 0x80D30000  # The shelving time not within an acceptable range.
    BadAggregateListMismatch = 0x80D40000  # The requested number of Aggregates does not match the requested number of NodeIds.
    BadAggregateNotSupported = 0x80D50000  # The requested Aggregate is not support by the server.
    BadAggregateInvalidInputs = 0x80D60000  # The aggregate value could not be derived due to invalid data inputs.
    BadBoundNotFound = 0x80D70000  # No data found to provide upper or lower bound value.
    BadBoundNotSupported = 0x80D80000  # The server cannot retrieve a bound for the variable.
    BadAggregateConfigurationRejected = 0x80DA0000  # The aggregate configuration is not valid for specified node.
    BadTooManyMonitoredItems = 0x80DB0000  # The request could not be processed because there are too many monitored items in the subscription.
    BadRequestNotAllowed = 0x80E40000  # The request was rejected by the server because it did not meet the criteria set by the server.
    BadTooManyArguments = 0x80E50000  # Too many arguments were provided.
    BadSecurityModeInsufficient = 0x80E60000  # The operation is not permitted over the current secure channel.
    BadCertificateChainIncomplete = 0x810D0000  # The certificate chain is incomplete.
    BadLicenseNotAvailable = 0x81100000  # The UA Server does not have a license which is required to operate in general or to perform a service or operation.


def bytestring_to_bytes(bstring):
    """SOPC_ByteString or SOPC_ByteString* to python bytes()"""
    if bstring == ffi.NULL or bstring.Data == ffi.NULL or bstring.Length <= 0:
        return b''
    return bytes(ffi.unpack(bstring.Data, bstring.Length))
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
    bstring.Data = allocator_no_gc('char[{}]'.format(len(b)), b)
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
    assert status == ReturnStatus.OK
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
    assert node != ffi.NULL, 'SOPC_NodeId_FromCString failed on string "{}"'.format(nid)
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
    SOPC_DateTime (the number of 100 nanosecond intervals since January 1, 1601)
    to Python time (the floating point number of seconds since 01/01/1970, see help(time)).

    Warning, as opposed to other sopc_type_to_python_type functions, this one does not accept pointers.
    """
    # (datetime.date(1970,1,1) - datetime.date(1601,1,1)).total_seconds() * 1000 * 1000 * 10
    # When datetime is NULL, this results in some minutes that are due to the int->double conversion.
    return (datetime - 116444736000000000)/1e7
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
    A `Variant` is the Pythonic representation of a SOPC_Variant.
    The SOPC_Variant is a C-structure that can contain multiple built-in types,
    such as integers, floats, strings.

    A Variant instance supports the arithmetic, comparison operations, and increment operators of the Python types.
    It can be used as any Python value.
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

    A SOPC_Variant can be converted to a Variant with the static method `pys2opc.types.Variant.from_sopc_variant`.
    A `Variant` can be converted to a SOPC_Variant with the method `pys2opc.types.Variant.to_sopc_variant`.

    Please note that, for now, this object lacks the support for some of the types that can normally be encapsulated in OPC UA Variants:
    ExpandedNodeId, ExtensionObject, DiagnosticInfo, Variant (you cannot have a Variant encapsulated in a Variant), or DataValue
    (you cannot have a DataValue encapsulated in a Variant, but `DataValue` are available).
    It fails for both parsing received OPC UA Variants and encoding `Variant`s to be sent.

    Attributes:
        variantType: Optional: The type of the `Variant` (see `pys2opc.types.VariantType`) when the value is produced from a SOPC_Variant*.
    """
    def __init__(self, python_value, variantType=None):
        self._value = python_value
        self.variantType = variantType

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
                return Variant((string_to_str(LocalizedText.defaultLocale), string_to_str(LocalizedText.defaultText)), sopc_type)
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
                # S2OPC should not be able to be in this case
                return Variant([], sopc_type)
            elif sopc_type == libsub.SOPC_Boolean_Id:
                return Variant([content.BooleanArr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_SByte_Id:
                return Variant([content.SbyteArr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_Byte_Id:
                return Variant([content.ByteArr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_Int16_Id:
                return Variant([content.Int16Arr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_UInt16_Id:
                return Variant([content.Uint16Arr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_Int32_Id:
                return Variant([content.Int32Arr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_UInt32_Id:
                return Variant([content.Uint32Arr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_Int64_Id:
                return Variant([content.Int64Arr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_UInt64_Id:
                return Variant([content.Uint64Arr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_Float_Id:
                return Variant([content.FloatvArr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_Double_Id:
                return Variant([content.DoublevArr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_String_Id:
                return Variant([string_to_str(ffi.addressof(content.StringArr[i])) for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_DateTime_Id:
                return Variant([datetime_to_float(content.DateArr[i]) for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_Guid_Id:
                return Variant([guid_to_uuid(ffi.addressof(content.GuidArr[i])) for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_ByteString_Id:
                return Variant([bytestring_to_bytes(ffi.addressof(content.BstringArr[i])) for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_XmlElement_Id:
                return Variant([bytestring_to_bytes(ffi.addressof(content.XmlEltArr[i])) for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_NodeId_Id:
                return Variant([nodeid_to_str(ffi.addressof(content.NodeIdArr[i])) for i in range(length)], sopc_type)
            #elif sopc_type == libsub.SOPC_ExpandedNodeId_Id:
            #    return Variant([content. for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_StatusCode_Id:
                return Variant([content.StatusArr[i] for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_QualifiedName_Id:
                Qname = content.QnameArr[i]
                return Variant([(Qname.NamespaceIndex, string_to_str(Qname.Name.Data)) for i in range(length)], sopc_type)
            elif sopc_type == libsub.SOPC_LocalizedText_Id:
                LocalizedText = content.LocalizedTextArr[i]
                return Variant([(string_to_str(LocalizedText.defaultLocale), string_to_str(LocalizedText.defaultText)) for i in range(length)], sopc_type)
            #elif sopc_type == libsub.SOPC_ExtensionObject_Id:
            #    return Variant([content. for i in range(length)], sopc_type)
            #elif sopc_type == libsub.SOPC_DataValue_Id:
            #    return Variant([content. for i in range(length)], sopc_type)
            #elif sopc_type == libsub.SOPC_Variant_Id:
            #    return Variant([content. for i in range(length)], sopc_type)
            #elif sopc_type == libsub.SOPC_DiagnosticInfo_Id:
            #    return Variant([content. for i in range(length)], sopc_type)
            raise ValueError('SOPC_Variant to Python conversion not supported for built-in type {}.'.format(sopc_type))
        elif variant.ArrayType == libsub.SOPC_VariantArrayType_Matrix:
            raise ValueError('SOPC_Variant matrices are not supported.')
        else:
            raise ValueError('Invalid SOPC_Variant.ArrayType')

    allocator = ffi.new_allocator(alloc=libsub.SOPC_Malloc, free=libsub.SOPC_Variant_Delete, should_clear_after_alloc=True)

    def to_sopc_variant(self, *, copy_type_from_variant=None, sopc_variant_type=None, no_gc=False):
        """
        Converts the current Variant to a SOPC_Variant*.
        Handles both single values and array values.
        The type may be specified by either self.variantType, copy_type_from_variant, or sopc_variant_type.
        If types is specified by multiple means, the type must be the same in all cases.

        Args:
            copy_type_from_variant: A C SOPC_Variant* or a Python Variant from which the type is copied.
            sopc_variant_type: A VariantType constant.
            no_gc: When True, the Python garbage collection mechanism is omited and the string must be
                   either manually deleted with a call to SOPC_String_Delete or passed to an object
                   which will call SOPC_String_Delete for you.
        """
        # Detect type
        sPossibleTypes = {self.variantType, sopc_variant_type}
        if copy_type_from_variant is not None:
            if isinstance(copy_type_from_variant, Variant):
                sPossibleTypes.add(copy_type_from_variant.variantType)
            else:
                sPossibleTypes.add(copy_type_from_variant.BuiltInTypeId)
        sPossibleTypes.remove(None)
        if not sPossibleTypes:
            raise ValueError('No type detected, please supply self.variantType or sopc_variant_type or sopy_type_from_variant.')
        if len(sPossibleTypes) > 1:
            raise ValueError('More than one type detected, please supply self.variantType or sopc_variant_type or sopy_type_from_variant.')
        sopc_type, = sPossibleTypes

        # Create and fill variant
        if no_gc:
            variant = allocator_no_gc('SOPC_Variant*')
        else:
            variant = Variant.allocator('SOPC_Variant*')
        variant.BuiltInTypeId = sopc_type
        # Testing whether this is an array or not is not straightforward because of qualified names and localized text that are couples
        if sopc_type in (libsub.SOPC_QualifiedName_Id, libsub.SOPC_LocalizedText_Id):
            is_array = len(self._value) > 0 and isinstance(self._value[0], (list, tuple))  # If self._value has no length -> malformed value
        else:
            is_array = isinstance(self._value, (list, tuple))
        if not is_array:
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
                loc.defaultLocale, loc.defaultText = map(lambda s:str_to_string(s, no_gc=True)[0], self._value)
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
                    locs[i].defaultLocale, locs[i].defaultText = map(lambda s:str_to_string(s, no_gc=True), v)
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

    def get_python(self):
        """
        Returns the python object wrapped by this Variant.
        Does not copy the object before returning it, hence modifying it will modify the `Variant`'s value.
        """
        return self._value


class VariantType(NamedMembers):
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
        timestampSource: The last time the value changed, specified by the writer, as a Python timestamp.
        timestampServer: The last time the value changed, according to the server, as a Python timestamp.
        statusCode: The quality associated to the value OR the reason why there is no available value.
                    It is a value from the `pys2opc.types.StatusCode` enum (e.g. `pys2opc.types.StatusCode.BadAttributeIdInvalid`).
        variant: The `pys2opc.types.Variant` storing the value.
        variantType: An accessor to the `variantType` attribute of `pys2opc.types.Variant`.
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
    def variantType(self):
        return self.variant.variantType
    @variantType.setter
    def variantType(self, ty):
        self.variant.variantType = ty

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
        Converts a SOPC_DataValue* or SOPC_DataValue to a Python `DataValue`.
        """
        return DataValue(datetime_to_float(datavalue.SourceTimestamp),
                         datetime_to_float(datavalue.ServerTimestamp),
                         datavalue.Status, Variant.from_sopc_variant(ffi.addressof(datavalue.Value)))

    @staticmethod
    def from_python(val):
        """
        Creates a DataValue from a Python value or a `pys2opc.types.Variant`.
        Creates the Variant, sets the status code to OK, and set source timestamp to now.
        """
        if not isinstance(val, Variant):
            val = Variant(val)
        return DataValue(time.time(), 0, StatusCode.Good, val)

    allocator = ffi.new_allocator(alloc=libsub.SOPC_Malloc, free=libsub.SOPC_DataValue_Delete, should_clear_after_alloc=True)

    def to_sopc_datavalue(self, *, copy_type_from_variant=None, sopc_variant_type=None, no_gc=False):
        """
        Converts a new SOPC_DataValue from the Python `DataValue`.
        See `pys2opc.types.Variant.to_sopc_variant` for a documentation of the arguments.

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
        source = float_to_datetime(self.timestampSource) if self.timestampSource else (0,)
        server = float_to_datetime(self.timestampServer) if self.timestampServer else (0,)
        datavalue.SourceTimestamp = source[0]
        datavalue.ServerTimestamp = server[0]
        datavalue.SourcePicoSeconds = 0
        datavalue.ServerPicoSeconds = 0
        return datavalue

    def get_python(self):
        """
        Returns the python object wrapped by this `DataValue`.
        Accessor to `pys2opc.types.Variant.get_python`.
        Use this when it is known that the Variant object will not be reused (e.g. by a future call to write_nodes).

        Does not copy the object before returning it.
        """
        return self.variant.get_python()


class LogLevel(NamedMembers):
    """
    Accessors to SOPC_Log_Level enum.
    """
    Error = libsub.SOPC_LOG_LEVEL_ERROR
    Warning = libsub.SOPC_LOG_LEVEL_WARNING
    Info = libsub.SOPC_LOG_LEVEL_INFO
    Debug = libsub.SOPC_LOG_LEVEL_DEBUG


class AttributeId(NamedMembers):
    """
    The constants to use for the attributes to read or write.
    These are accessors to the SOPC_LibSub_AttributeId C enum.
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


class EncodeableType(NamedMembers):
    """
    The known SOPC_EncodeableTypes*, used in the request and response OpcUa_* types.
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
    The `BrowseResult` is a low-level structures that contains the results of browsing a single node.

    Attributes:
        status: the status code of the browse operation.
        continuationPoint: whether the browse is incomplete (continuationPoint not empty) or not.
        references: list of outgoing `pys2opc.types.Reference`s.
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
            dispName = (string_to_str(sopc_ref.DisplayName.defaultLocale),
                        string_to_str(sopc_ref.DisplayName.defaultText))
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

    def __repr__(self):
        return "Reference({!r}, {!r}, {!r}, {!r}, {!r},  {!r}, {!r})".format(self.referenceTypeId, self.isForward, self.nodeId, self.browseName, self.displayName, self.nodeClass, self.typeDefinition)

    def __str__(self):
        return "Reference(type='{}', forward={}) to node(id='{}', name='{}', class={}, type='{}')".format(self.referenceTypeId, bool(self.isForward), self.nodeId, self.browseName[1], NodeClass.get_name_from_id(self.nodeClass), self.typeDefinition)

class SecurityPolicy(NamedMembers):
    """
    The available security policies. These are accessors to the SOPC_SecurityPolicy_* strings.
    """
    # None is a reserved keyword
    PolicyNone     = libsub.SOPC_SecurityPolicy_None_URI
    Basic256       = libsub.SOPC_SecurityPolicy_Basic256_URI
    Basic256Sha256 = libsub.SOPC_SecurityPolicy_Basic256Sha256_URI
    Aes128Sha256RsaOaep = libsub.SOPC_SecurityPolicy_Aes128Sha256RsaOaep_URI
    Aes256Sha256RsaPss = libsub.SOPC_SecurityPolicy_Aes256Sha256RsaPss_URI

class SecurityMode(NamedMembers):
    """
    The available security modes. These are accessors to the OpcUa_MessageSecurityMode enum.
    """
    # None is a reserved keyword
    ModeNone       = libsub.OpcUa_MessageSecurityMode_None
    Sign           = libsub.OpcUa_MessageSecurityMode_Sign
    SignAndEncrypt = libsub.OpcUa_MessageSecurityMode_SignAndEncrypt

class NodeClass(NamedMembers):
    """
    The available node classes. These are accessors to the OpcUa_NodeClass enum.
    """
    Unspecified   = libsub.OpcUa_NodeClass_Unspecified
    Object        = libsub.OpcUa_NodeClass_Object
    Variable      = libsub.OpcUa_NodeClass_Variable
    Method        = libsub.OpcUa_NodeClass_Method
    ObjectType    = libsub.OpcUa_NodeClass_ObjectType
    VariableType  = libsub.OpcUa_NodeClass_VariableType
    ReferenceType = libsub.OpcUa_NodeClass_ReferenceType
    DataType      = libsub.OpcUa_NodeClass_DataType
    View          = libsub.OpcUa_NodeClass_View

class UserAuthorization(NamedMembers):
    """
    The possible requested user authorization Operation types.
    These are accessors to the SOPC_UserAuthorization_OperationType enum.
    They are used for the user authorization callback (see `pys2opc.server_callbacks.BaseUserHandler`).
    """
    Read       = libsub.SOPC_USER_AUTHORIZATION_OPERATION_READ
    Write      = libsub.SOPC_USER_AUTHORIZATION_OPERATION_WRITE
    Executable = libsub.SOPC_USER_AUTHORIZATION_OPERATION_EXECUTABLE

class UserAuthentication(NamedMembers):
    """
    The possible requested user authentication Status types.
    These are accessors to the SOPC_UserAuthentication_Status enum.
    They are used as answer to the user authentication callback (see `pys2opc.server_callbacks.BaseUserHandler`).
    """
    InvalidToken  = libsub.SOPC_USER_AUTHENTICATION_INVALID_TOKEN
    RejectedToken = libsub.SOPC_USER_AUTHENTICATION_REJECTED_TOKEN
    AccessDenied  = libsub.SOPC_USER_AUTHENTICATION_ACCESS_DENIED
    """It is strongly discouraged to use ACCESS_DENIED, prefer REJECTED_TOKEN"""
    Ok            = libsub.SOPC_USER_AUTHENTICATION_OK


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
    datetime = ffi.new('SOPC_DateTime*')
    datetime[0] = libsub.SOPC_Time_GetCurrentTimeUTC()
    assert abs(datetime_to_float(datetime[0]) - t) < .1
    assert datetime_to_float(float_to_datetime(t)[0]) == t

    # Thu Nov 30 04:57:25.694287 2034 UTC, unix timestamp is 2048471845.694287
    assert ntp_to_python(18285654237264005879) == 2048471845.694287  # Hopefully with this one there is no float-rounding errors.

    print(Reference("i=1000", True, "i=10001000", [0, "bla"], None, 1, "i=2"))
    
