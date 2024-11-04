#!/usr/bin/env python3
# cython: language_level=3str
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
from functools import total_ordering
import time
import os
import json
import getpass
import threading
import time
from binascii import hexlify
import uuid
from libc.stdlib cimport calloc, free
from libc.string cimport strncpy
from libc.stdint cimport UINT8_MAX, UINT16_MAX, UINT32_MAX

VERSION = json.load(open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'version.json')))['version']

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

class AttributeId(NamedMembers):
    Invalid = SOPC_AttributeId.SOPC_AttributeId_Invalid
    NodeId = SOPC_AttributeId.SOPC_AttributeId_NodeId
    NodeClass = SOPC_AttributeId.SOPC_AttributeId_NodeClass
    BrowseName = SOPC_AttributeId.SOPC_AttributeId_BrowseName
    DisplayName = SOPC_AttributeId.SOPC_AttributeId_DisplayName
    Description = SOPC_AttributeId.SOPC_AttributeId_Description
    WriteMask = SOPC_AttributeId.SOPC_AttributeId_WriteMask
    UserWriteMask = SOPC_AttributeId.SOPC_AttributeId_UserWriteMask
    IsAbstract = SOPC_AttributeId.SOPC_AttributeId_IsAbstract
    Symmetric = SOPC_AttributeId.SOPC_AttributeId_Symmetric
    InverseName = SOPC_AttributeId.SOPC_AttributeId_InverseName
    ContainsNoLoops = SOPC_AttributeId.SOPC_AttributeId_ContainsNoLoops
    EventNotifier = SOPC_AttributeId.SOPC_AttributeId_EventNotifier
    Value = SOPC_AttributeId.SOPC_AttributeId_Value
    DataType = SOPC_AttributeId.SOPC_AttributeId_DataType
    ValueRank = SOPC_AttributeId.SOPC_AttributeId_ValueRank
    ArrayDimensions = SOPC_AttributeId.SOPC_AttributeId_ArrayDimensions
    AccessLevel = SOPC_AttributeId.SOPC_AttributeId_AccessLevel
    UserAccessLevel = SOPC_AttributeId.SOPC_AttributeId_UserAccessLevel
    MinimumSamplingInterval = SOPC_AttributeId.SOPC_AttributeId_MinimumSamplingInterval
    Historizing = SOPC_AttributeId.SOPC_AttributeId_Historizing
    Executable = SOPC_AttributeId.SOPC_AttributeId_Executable
    UserExecutable = SOPC_AttributeId.SOPC_AttributeId_UserExecutable
    DataTypeDefinition = SOPC_AttributeId.SOPC_AttributeId_DataTypeDefinition
    RolePermissions = SOPC_AttributeId.SOPC_AttributeId_RolePermissions
    UserRolePermissions = SOPC_AttributeId.SOPC_AttributeId_UserRolePermissions
    AccessRestrictions = SOPC_AttributeId.SOPC_AttributeId_AccessRestrictions
    AccessLevelEx = SOPC_AttributeId.SOPC_AttributeId_AccessLevelEx

class NodeClass(NamedMembers):
    """
    The available node classes as an enum.
    """
    Unspecified   = OpcUa_NodeClass.OpcUa_NodeClass_Unspecified
    Object        = OpcUa_NodeClass.OpcUa_NodeClass_Object
    Variable      = OpcUa_NodeClass.OpcUa_NodeClass_Variable
    Method        = OpcUa_NodeClass.OpcUa_NodeClass_Method
    ObjectType    = OpcUa_NodeClass.OpcUa_NodeClass_ObjectType
    VariableType  = OpcUa_NodeClass.OpcUa_NodeClass_VariableType
    ReferenceType = OpcUa_NodeClass.OpcUa_NodeClass_ReferenceType
    DataType      = OpcUa_NodeClass.OpcUa_NodeClass_DataType
    View          = OpcUa_NodeClass.OpcUa_NodeClass_View

class ReturnStatus(NamedMembers):
    STATUS_OK = SOPC_ReturnStatus.SOPC_STATUS_OK
    STATUS_NOK = SOPC_ReturnStatus.SOPC_STATUS_NOK
    STATUS_INVALID_PARAMETERS = SOPC_ReturnStatus.SOPC_STATUS_INVALID_PARAMETERS
    STATUS_INVALID_STATE = SOPC_ReturnStatus.SOPC_STATUS_INVALID_STATE
    STATUS_ENCODING_ERROR = SOPC_ReturnStatus.SOPC_STATUS_ENCODING_ERROR
    STATUS_WOULD_BLOCK = SOPC_ReturnStatus.SOPC_STATUS_WOULD_BLOCK
    STATUS_TIMEOUT = SOPC_ReturnStatus.SOPC_STATUS_TIMEOUT
    STATUS_OUT_OF_MEMORY = SOPC_ReturnStatus.SOPC_STATUS_OUT_OF_MEMORY
    STATUS_CLOSED = SOPC_ReturnStatus.SOPC_STATUS_CLOSED
    STATUS_NOT_SUPPORTED = SOPC_ReturnStatus.SOPC_STATUS_NOT_SUPPORTED

class VariantType(NamedMembers):
    Null = SOPC_BuiltinId.SOPC_Null_Id
    Boolean = SOPC_BuiltinId.SOPC_Boolean_Id
    SByte = SOPC_BuiltinId.SOPC_SByte_Id
    Byte = SOPC_BuiltinId.SOPC_Byte_Id
    Int16 = SOPC_BuiltinId.SOPC_Int16_Id
    UInt16 = SOPC_BuiltinId.SOPC_UInt16_Id
    Int32 = SOPC_BuiltinId.SOPC_Int32_Id
    UInt32 = SOPC_BuiltinId.SOPC_UInt32_Id
    Int64 = SOPC_BuiltinId.SOPC_Int64_Id
    UInt64 = SOPC_BuiltinId.SOPC_UInt64_Id
    Float = SOPC_BuiltinId.SOPC_Float_Id
    Double = SOPC_BuiltinId.SOPC_Double_Id
    String = SOPC_BuiltinId.SOPC_String_Id
    DateTime = SOPC_BuiltinId.SOPC_DateTime_Id
    Guid = SOPC_BuiltinId.SOPC_Guid_Id
    ByteString = SOPC_BuiltinId.SOPC_ByteString_Id
    XmlElement = SOPC_BuiltinId.SOPC_XmlElement_Id
    NodeId = SOPC_BuiltinId.SOPC_NodeId_Id
    ExpandedNodeId = SOPC_BuiltinId.SOPC_ExpandedNodeId_Id
    StatusCode = SOPC_BuiltinId.SOPC_StatusCode_Id
    QualifiedName = SOPC_BuiltinId.SOPC_QualifiedName_Id
    LocalizedText = SOPC_BuiltinId.SOPC_LocalizedText_Id
    ExtensionObject = SOPC_BuiltinId.SOPC_ExtensionObject_Id
    DataValue = SOPC_BuiltinId.SOPC_DataValue_Id
    Variant = SOPC_BuiltinId.SOPC_Variant_Id
    DiagnosticInfo = SOPC_BuiltinId.SOPC_DiagnosticInfo_Id


class StatusCode(NamedMembers):
    """
    The OpcUa status codes. Directly generated from src/Common/opcua_types/opcua_statuscodes.h.
    Status codes are used in various places among the OPC UA protocol.
    They usually represent the quality of a value (see `pys2opc.DataValue`),
    or the status of the execution of a service (see `pys2opc.WriteResponse`, `pys2opc.BrowseResponse`).
    """
    # Adds the generic good and bad values, which are only defined as masks in the OPC UA protocol.
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

    @staticmethod
    def isGoodStatus(status : StatusCode) -> bool:
        return SOPC_IsGoodStatus(status)

class SOPC_Failure(Exception):
    """
    Exception sub class to manage failure from S2OPC code
    """

    def __init__(self, message: str,  status: ReturnStatus|None = None):
        super().__init__(message if status is None else '{} (Code {})'.format(message, ReturnStatus.get_name_from_id(status)))
        self.status = status

class ClientConnectionFailure(Exception):
    """
    Unexpected connection failure exception meaning the connection has been closed.
    It is raised when an attempt to use the closed connection it is made.
    """
    pass

class ServiceFailure(Exception):
    """
    Exception sub class to manage service failure
    """
    def __init__(self, message: str,  status: StatusCode|None = None):
        super().__init__(message if status is None else '{} (Code {})'.format(message, StatusCode.get_name_from_id(status)))
        self.status = status

class ServiceFault(ServiceFailure):
    """
    ServiceFailure sub class to manage service fault
    """
    def __init__(self, message: str,  status: StatusCode|None = None):
        super().__init__(message, status)

class PyS2OPC:
    """
    Base class for components that are common to both Clients and Servers.
    """

    _initialized_srv: bool = False
    """Server initialization status"""
    _initialized_cli: bool = False
    """Client initialization status"""

    # @staticmethod : Python 3.9 does not support both and report error on staticmethod is not callable
    @contextmanager
    def initialize(logLevel: SOPC_Log_Level=SOPC_Log_Level.SOPC_LOG_LEVEL_DEBUG, logPath: str='logs/', logFileMaxBytes: int=1048576, logMaxFileNumber: int=50) -> None:
        """
        Common initialization (Client/Server): configure log, start toolkit threads, etc.
        Automatically called by `PyS2OPC_Client.initialize` or `PyS2OPC_Server.initialize` if not done.

        Performs toolkit common initialization if this has not already been done
        """
        if logFileMaxBytes > UINT32_MAX:
            raise ValueError("logFileMaxBytes is too large (must be less than UINT32_MAX)")
        if logMaxFileNumber > UINT16_MAX:
            raise ValueError("logMaxFileNumber is too large (must be less than UINT16_MAX)")
        b_logPath = logPath.encode()
        cdef SOPC_LogSystem_File_Configuration log_file_conf = SOPC_LogSystem_File_Configuration(
                                                               logDirPath = b_logPath,
                                                               logMaxBytes = logFileMaxBytes,
                                                               logMaxFiles = logMaxFileNumber)
        cdef SOPC_Log_SystemConfiguration log_file_conf_u = SOPC_Log_SystemConfiguration(fileSystemLogConfig = log_file_conf)
        cdef SOPC_Log_Configuration log_config = SOPC_Log_Configuration(
                                                logLevel = logLevel,
                                                logSystem = SOPC_Log_System.SOPC_LOG_SYSTEM_FILE,
                                                logSysConfig = log_file_conf_u)

        status = SOPC_ReturnStatus.SOPC_STATUS_OK
        if not SOPC_CommonHelper_GetInitialized():
            status = SOPC_CommonHelper_Initialize(&log_config)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Library common initialization failed ', status)

    @staticmethod
    def _clear() -> None:
        """
        Common clear (Client/Server).
        Automatically called by `PyS2OPC_Client.clear` or `PyS2OPC_Server.clear` if applicable.

        Performs toolkit common clear.
        """
        SOPC_CommonHelper_Clear()

    @staticmethod
    def get_version() -> str:
        """Returns complete version string (PyS2OPC, S2OPC_Common, S2OPC_ClientServer)"""
        cdef SOPC_Toolkit_Build_Info build_info = SOPC_CommonHelper_GetBuildInfo()
        return 'PyS2OPC v' + VERSION + '\n' + \
               'S2OPC_Common       - Version: {}, SrcCommit: {}, DockerId: {}, BuildDate: {}\n'.format(
                build_info.commonBuildInfo.buildVersion.decode('UTF-8'),build_info.commonBuildInfo.buildSrcCommit.decode('UTF-8'),
                build_info.commonBuildInfo.buildDockerId.decode('UTF-8'), build_info.commonBuildInfo.buildBuildDate.decode('UTF-8')) + \
                'S2OPC_ClientServer - Version: {}, SrcCommit: {}, DockerId: {}, BuildDate: {}\n'.format(
                build_info.clientServerBuildInfo.buildVersion.decode('UTF-8'), build_info.clientServerBuildInfo.buildSrcCommit.decode('UTF-8'),
                build_info.clientServerBuildInfo.buildDockerId.decode('UTF-8'), build_info.clientServerBuildInfo.buildBuildDate.decode('UTF-8'))

    @staticmethod
    def _get_password(prompt :str) -> str:
        """
        Return a zero-terminated byte string which contain the password.
        """
        pwd = getpass.getpass(prompt='{}'.format(prompt))
        return pwd

cdef class _Request:
    """
    A C class storing all request related context:
        - Request C struct
        - User request context
        - timestamp
        - Event to indicate when the response is received
        - Request function builder
    """

    # Object variable #
    cdef void* _request
    """ The C request """
    cdef uintptr_t _requestContext
    """ User request context """
    cdef int _timestampSent
    cdef object _eventResponseReceived
    """ Response event handler"""

    def __cinit__(self):
        self._timestampSent = 0  # The sender of the request sets the timestamp
        self._eventResponseReceived = threading.Event()

    @staticmethod
    cdef _Request c_new_request(void* request):
        cdef _Request Req = _Request.__new__(_Request)
        Req._request = request
        Req._requestContext = <uintptr_t>(<void*> Req) # Unique ID
        return Req

    @staticmethod
    def new_read_request(nodeIds: list[str], attributes: list[AttributeId]=[]) -> _Request:
        """
        Create a Read request from a list of nodeIds and its corresponding list of attributes
        (set by default to `AttributeId.Value`).
        View the possible attributes in the `AttributeId` enumeration
        """
        if attributes == []:
            attributes = [AttributeId.Value for _ in nodeIds]
        assert len(nodeIds) == len(attributes),\
            'There should the same number of NodeIds, attributes, and datavalues when reading nodes'

        cdef OpcUa_ReadRequest* readRequest = NULL
        readRequest = SOPC_ReadRequest_Create(len(nodeIds), OpcUa_TimestampsToReturn.OpcUa_TimestampsToReturn_Both)
        if NULL != readRequest:
            for  i, (snid, attr) in enumerate(zip(nodeIds, attributes)):
                b_snid = snid.encode()
                status = SOPC_ReadRequest_SetReadValueFromStrings(readRequest, i, b_snid, attr, NULL)
                if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                    raise SOPC_Failure('SOPC_ReadRequest_SetReadValueFromStrings failed for {}'.format(snid), status)
        else:
            raise MemoryError
        req = _Request.c_new_request(request=readRequest)
        return req

    @staticmethod
    def new_write_request(nodeIds: list[str], datavalues: list[DataValue], attributes: list[AttributeId]=[], types: list[VariantType]=[]) -> _Request:
        """
        Create a Write request from a list of nodeIds, its corresponding list of datavalues,
        and list of attributes (set by default to `AttributeId.Value`).
        View the possible attributes in the AttributeId enumeration
        """
        if attributes == []:
            attributes = [AttributeId.Value for _ in nodeIds]
        assert len(nodeIds) == len(attributes) == len(datavalues),\
            'There should the same number of NodeIds, attributes, and datavalues when writing nodes'
        if types != []:
            assert len(nodeIds) == len(types), 'There should the same number of NodeIds and Types when writing nodes'
        else:
            types = [None] * len(nodeIds)

        # Compute types
        sopc_types = []
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

        cdef OpcUa_WriteRequest* writeRequest = NULL
        writeRequest = SOPC_WriteRequest_Create(len(nodeIds))
        cdef SOPC_DataValue* writeValue = <SOPC_DataValue*> calloc(len(nodeIds), sizeof(SOPC_DataValue))

        if NULL != writeRequest:
            for  i, (snid, attr, dv) in enumerate(zip(nodeIds, attributes, datavalues)):
                b_snid = snid.encode()
                _C_DataValue.to_sopc_datavalue(dv, &(writeValue[i]))
                status = SOPC_WriteRequest_SetWriteValueFromStrings(writeRequest, i, b_snid, attr, NULL, &(writeValue[i]))
                if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                    raise SOPC_Failure('SOPC_WriteRequest_SetWriteValueFromStrings failed for {}'.format(snid), status)
            free(writeValue)
            writeValue = NULL
        else:
            raise MemoryError
        req = _Request.c_new_request(request=writeRequest)
        return req

    @staticmethod
    def new_browse_request(nodeIds: list[str], size_t maxReferencesPerNode) -> _Request:
        """
        Create a Browse request from a list of nodeIds and a maxReferencesPerNode (Indicates the maximum number of references
        to return for each starting node specified in the request (0 means no limitation)).
        """
        cdef OpcUa_BrowseRequest* browseRequest = NULL
        browseRequest = SOPC_BrowseRequest_Create(len(nodeIds), maxReferencesPerNode, NULL)
        if NULL != browseRequest:
            for  i, snid in enumerate(nodeIds):
                b_snid = snid.encode()
                status = SOPC_BrowseRequest_SetBrowseDescriptionFromStrings(browseRequest, i, b_snid,
                                                                            OpcUa_BrowseDirection.OpcUa_BrowseDirection_Both, NULL, True, OpcUa_NodeClass.OpcUa_NodeClass_Unspecified,
                                                                            OpcUa_BrowseResultMask.OpcUa_BrowseResultMask_All)
                if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                    raise SOPC_Failure('SOPC_BrowseRequest_SetBrowseDescriptionFromStrings failed for {}'.format(snid), status)
        else:
            raise MemoryError
        req = _Request.c_new_request(request=browseRequest)
        return req

    @staticmethod
    def new_callmethod_request(objectNodeId: str, methodNodeId: str, inputArgList: list[Variant]) -> _Request:
        """
        Create a Call request with `methodNodeId` as node to execute, `objectNodeId` as target and inputArgList as input arguments
        """
        b_objectNodeId: bytes = objectNodeId.encode()
        b_methodNodeId: bytes = methodNodeId.encode()

        cdef SOPC_ReturnStatus status = SOPC_ReturnStatus.SOPC_STATUS_OK
        cdef SOPC_Variant* pVariantInputArguments = NULL

        cdef OpcUa_CallRequest* pCallRequest = SOPC_CallRequest_Create(1)
        status = SOPC_ReturnStatus.SOPC_STATUS_OK if NULL != pCallRequest else SOPC_ReturnStatus.SOPC_STATUS_OUT_OF_MEMORY
        errorMessage: str = 'Memory issue : SOPC_CallRequest_Create failed'

        if status == SOPC_ReturnStatus.SOPC_STATUS_OK and len(inputArgList) > 0: # If status is OK and there are input argument(s)
            pVariantInputArguments = <SOPC_Variant*> calloc(len(inputArgList), sizeof(SOPC_Variant))
            status = SOPC_ReturnStatus.SOPC_STATUS_OK if NULL != pVariantInputArguments else SOPC_ReturnStatus.SOPC_STATUS_OUT_OF_MEMORY
            errorMessage: str = 'Memory issue : Variant(s) allocation failed'

        # Translate python variants into C variants (Call Method Input Arguments)
        if status == SOPC_ReturnStatus.SOPC_STATUS_OK:
            for i, variant in enumerate(inputArgList):
                _C_Variant.to_sopc_variant(variant, &pVariantInputArguments[i])
            status = SOPC_CallRequest_SetMethodToCallFromStrings(pCallRequest, 0, b_objectNodeId, b_methodNodeId, len(inputArgList), pVariantInputArguments)
            errorMessage: str = 'SOPC_CallRequest_SetMethodToCallFromStrings failed for method {}'.format(methodNodeId)

        if NULL != pVariantInputArguments:
            for i in range(len(inputArgList)):
                SOPC_Variant_Clear(&pVariantInputArguments[i])
            free(pVariantInputArguments)
            pVariantInputArguments = NULL

        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure(errorMessage, status)

        req = _Request.c_new_request(request=pCallRequest)
        return req

cdef class Response:
    """
    Parent class of all response types `ReadResponse`, `WriteResponse`, `BrowseResponse`, `AsyncResponse`.
    """

    # Object variable #
    cdef void* _response
    cdef uintptr_t _responseContext

    def __cinit__(self):
        pass

    def __dealloc__(self):
        cdef SOPC_EncodeableType** encType = <SOPC_EncodeableType**> self._response
        if self._response is not NULL:
            SOPC_Encodeable_Delete(encType[0], <void**> &self._response)

    @staticmethod
    cdef Response c_new_response(void* response, uintptr_t responseContext):
        cdef Response Resp = Response.__new__(Response)
        Resp._response = response
        Resp._responseContext = responseContext
        return Resp

    def _parse_generic_response(self) -> (Response | None):
        """
        Generic function to call the appropriate parsing function depending on the type of response
        """
        cdef SOPC_EncodeableType** encType = <SOPC_EncodeableType**> self._response
        if encType[0] == &OpcUa_ReadResponse_EncodeableType:
            return self._parse_read_response()
        elif encType[0] == &OpcUa_WriteResponse_EncodeableType:
            return self._parse_write_response()
        elif encType[0] == &OpcUa_BrowseResponse_EncodeableType:
            return self._parse_browse_response()
        elif encType[0] == &OpcUa_CallResponse_EncodeableType:
            return self._parse_call_response()
        else:
            return None

    def _parse_read_response(self) -> (ReadResponse | None):
        """
        Parses an OpcUa_ReadResponse.
        Return : ReadResponse (class) which stores a list of DataValue,
                 None in case of service failure
        """
        cdef OpcUa_ReadResponse* readResponse = <OpcUa_ReadResponse*> self._response
        if SOPC_IsGoodStatus(readResponse.ResponseHeader.ServiceResult):
            results = [_C_DataValue.from_sopc_datavalue(&(readResponse.Results[i])) for i in range(readResponse.NoOfResults)]
            return ReadResponse(results=results)
        else:
            return None

    def _parse_write_response(self) -> (WriteResponse | None):
        """
        Parses an OpcUa_WriteResponse.
        Return : WriteResponse (class) which stores a list of StatusCode,
                 None in case of service failure
        """
        cdef OpcUa_WriteResponse* writeResponse = <OpcUa_WriteResponse*> self._response
        if writeResponse != NULL:
            if SOPC_IsGoodStatus(writeResponse.ResponseHeader.ServiceResult):
                results = [writeResponse.Results[i] for i in range(writeResponse.NoOfResults)]
                return WriteResponse(results=results)
        else:
            return None

    def _parse_browse_response(self) -> (BrowseResponse | None):
        """
        Parses an OpcUa_BrowseResponse.
        Return : BrowseResponse (class) which stores a list of BrowseResult,
                 None in case of service failure
        """
        cdef OpcUa_BrowseResponse* browseResponse = <OpcUa_BrowseResponse*> self._response
        cdef OpcUa_BrowseResult* result = NULL
        if SOPC_IsGoodStatus(browseResponse.ResponseHeader.ServiceResult):
            results = [_C_BrowseResult.from_sopc_browseResult(&(browseResponse.Results[i])) for i in range(browseResponse.NoOfResults)]
            return BrowseResponse(results=results)
        else:
            return None

    def _parse_call_response(self) -> (CallResponse | None):
        """
        Parses an OpcUa_CallResponse.
        Return : CallResponse (class) which stores a list of StatusCode and a list of Variant,
                 None in case of service failure
        """
        cdef OpcUa_CallResponse* pCallResponse = <OpcUa_CallResponse*> self._response
        inputArgResults: list[StatusCode] = []
        outputResults: list[Variant] = []
        if SOPC_IsGoodStatus(pCallResponse.ResponseHeader.ServiceResult):
            callResult = pCallResponse.Results[0].StatusCode
            for i in range(pCallResponse.Results[0].NoOfInputArgumentResults):
                inputArgResult: StatusCode = pCallResponse.Results[0].InputArgumentResults[i]
                inputArgResults.append(inputArgResult)
            for i in range(pCallResponse.Results[0].NoOfOutputArguments):
                outputArg: Variant = _C_Variant.from_sopc_variant(&(pCallResponse.Results[0].OutputArguments[i]))
                outputResults.append(outputArg)
            return CallResponse(callResult, inputArgResults, outputResults)
        else:
            return None

class ReadResponse(Response):
    """
    A ReadResponse provides the data values content resulting of OPC UA Read service requested.
    """

    def __init__(self, results: list[DataValue]):
        self._results = results

    @property
    def results(self) -> list[DataValue]:
        """
        Result data values have the index corresponding to the NodeId index
        provided in `BaseClientConnectionHandler.read_nodes`
        """
        return self._results

class WriteResponse(Response):
    """
    A WriteResponse provides the status codes resulting of OPC UA Write service requested
    """

    def __init__(self, results: list[StatusCode]):
        self._results = results

    def is_ok(self) -> bool:
        """
        Returns True if all writes were done successfully.
        """
        return all(SOPC_IsGoodStatus(res) for res in self._results)

    @property
    def results(self) -> list[StatusCode]:
        """
        Result status codes have the index corresponding to the NodeId index
        provided in `BaseClientConnectionHandler.write_nodes`.
        Each status code indicate the success `StatusCode.isGoodStatus` or failure of the write operation.
        """
        return self._results

class BrowseResponse(Response):
    """
    A Browse response provides the status codes resulting of OPC UA Browse service requested treatment.
    """

    def __init__(self, results: list[BrowseResult]):
        self._results = results

    def is_ok(self) -> bool:
        """
        Returns True if all browses were done successfully.
        """
        return all(SOPC_IsGoodStatus(res.status) for res in self._results)

    @property
    def results(self) -> list[BrowseResult]:
        """
        Result browse results codes have the index corresponding to the NodeId index
        provided in `BaseClientConnectionHandler.browse_nodes`.
        Each status code indicate the success `StatusCode.isGoodStatus` or failure of the write operation.
        """
        return self._results

class CallResponse(Response):
    """
    A Call response provides the status codes and list of variant (output arguments of method called)
    resulting of OPC UA CallMethod service requested treatment.
    """

    def __init__(self, callResult: StatusCode, inputArgResults: list[StatusCode], outputResults: list[Variant]):
        self._callResult = callResult
        self._inputArgResults = inputArgResults
        self._outputResults = outputResults

    def is_ok(self) -> bool:
        """
        Returns True if the callmethod were done successfully.
        """
        return SOPC_IsGoodStatus(self._callResult)

    @property
    def callResult(self) -> StatusCode:
        """
        Returns the status code of the callmethod.
        """
        return self._callResult

    @property
    def inputArgResults(self) -> list[StatusCode]:
        """
        inputArgResults status codes have the index corresponding to the inputArgList index
        provided in `BaseClientConnectionHandler.call_method`.
        Each status code indicates the correctness of each inputs argument provided to the callmethod.
        """
        return self._inputArgResults

    @property
    def outputResults(self) -> list[Variant]:
        """
        Returns the result (if any) of the method called.
        This result is given in the form of Variant list
        and has the index corresponding to the method's output index
        provided in `BaseClientConnectionHandler.call_method`.
        """
        return self._outputResults


class AsyncResponse(Response):
    """
    A special Response subtype to provide a handler for asynchronous OPC UA service call and response access.
    """
#   To do this, it must keep AsyncRequestHandler as context.
#   (Needed to wrap it into a subclass of Response to stay consistent with prototype of cdef function : `_AsyncRequestHandler_send_generic_request`)

    def __init__(self, asyncReqHdlr: _AsyncRequestHandler):
        self._asyncReqHdlr = asyncReqHdlr

    def get_response(self) -> Response|None:
        """
        Returns the response to the request if there is an available response (received).
        Otherwise returns None.
        The response type depends on the service used: `ReadResponse`, `WriteResponse`, `BrowseResponse`.
        Note: if a service failure occurs, raise an `Exception` sub class : `ServiceFailure`
        """
        asyncReqHdlr: _AsyncRequestHandler = self._asyncReqHdlr
        cdef _Request request = asyncReqHdlr._dRequestContext
        if request._eventResponseReceived.is_set():
            if asyncReqHdlr._dServiceException is not None: # Service failed
                raise asyncReqHdlr._dServiceException
            if asyncReqHdlr._dResponseContext is not None:
                return asyncReqHdlr._dResponseContext._parse_generic_response()
        return None

cdef class _AsyncRequestHandler:
    """
    MixIn that implements asynchronous request handling: associates a response to a request.
    """

    # Object variable #
    cdef _Request _dRequestContext
    cdef Response _dResponseContext
    cdef Exception _dServiceException
    """Handle service failure exception"""

    def __cinit__(self):
        self._dRequestContext = None
        self._dResponseContext = None
        self._dServiceException = None

    cdef Response _send_generic_request(self, isLocalService: bool, SOPC_ClientConnection* connection, request: _Request, bWaitResponse: bool):
        """
        Sends a `request` on this `connection`.
        If `isLocalService` is set, this function will use the server's local services
        When `bWaitResponse`, waits for the response and returns it.
        Otherwise, returns the `AsyncResponse`, and the response will be available through `get_response` (on AsyncResponse returned).
        """
        request._timestampSent = time.time()
        self._dRequestContext = request
        if isLocalService:
            status = SOPC_ServerHelper_LocalServiceAsync(request._request, request._requestContext)
        elif connection != NULL:
            status = SOPC_ClientHelper_ServiceAsync(connection, request._request, request._requestContext)
        else:
            assert False, '_send_generic_request: !isLocalService => connection != NULL'
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('ServiceAsync failed to send request.', status)
        if bWaitResponse:
            return self._wait_for_response(request)
        else:
            return AsyncResponse(self)

    @staticmethod
    cdef list[VariantType] _helper_maybe_read_types(isLocalService: bool, SOPC_ClientConnection* connection, reqHandlerCtx : dict[uintptr_t, _AsyncRequestHandler], nodeIds: list[str], datavalues: list[DataValue], attributes: list[AttributeId], types: list[VariantType]):
        """
        Internal helper that makes a `_Request` to read the missing types, if any, in the provided `datavalues` and `types` list.
        Return the type list.
        Used by `write_nodes` implementations.
        """
        assert len(nodeIds) == len(datavalues),\
            'There should the same number of NodeIds and datavalues when reading nodes type'
        if types != []:
            assert len(nodeIds) == len(types),\
                'There should the same number of NodeIds and types when reading nodes type'
        else:
            types = [None] * len(nodeIds)

        if attributes != []:
            assert len(nodeIds) == len(attributes),\
                'There should the same number of NodeIds and attributes when reading nodes type'
        else:
            attributes = [AttributeId.Value for _ in nodeIds]

        # Compute missing types, send the request, and update the missing types.
        sopc_types = [dv.variantType if dv.variantType is not None else ty for dv,ty in zip(datavalues, types)]
        missingTypesInfo = [(i, snid, attr) for i,(snid,attr,ty) in enumerate(zip(nodeIds, attributes, sopc_types)) if ty is None]
        if missingTypesInfo:
            _, readNids, readAttrs = zip(*missingTypesInfo)
            request: _Request = _Request.new_read_request(list(readNids), attributes=list(readAttrs))
            reqHandler = _AsyncRequestHandler()
            reqHandlerCtx[request._requestContext] = reqHandler
            readDatavalues = reqHandler._send_generic_request(isLocalService, connection, request, True)
            for (i, _, _), dv in zip(missingTypesInfo, readDatavalues.results):
                assert dv.variantType != SOPC_BuiltinId.SOPC_Null_Id, 'Automatic type detection failed, null type read.'
                sopc_types[i] = dv.variantType
        return sopc_types

    def _wait_for_response(self, request: _Request) -> (Response | None):
        request._eventResponseReceived.wait()
        if self._dServiceException is not None: # Service failed
            raise self._dServiceException
        if self._dResponseContext is not None:
            return self._dResponseContext._parse_generic_response()
        return None

    cdef void _on_response(self, SOPC_EncodeableType* type, const void* response, uintptr_t userContext, float timestamp):
        """
        Receives an OpcUa_*Response (of this `type`), creates a *Response* with `response` and `userContext`,
        and associates it to a Request both-ways in this class (`_AsyncRequestHandler`).
        It is called for every `response` received through the S2OPC callback.

        Warning: `response` is freed by the caller (S2OPC), so the structure or its content must be copied before returning.

        The `timestamp` parameters is computed on the first line of the event callback,
        hence it is the most accurate instant when the response was received by the Python layer.

        Finish by setting the eventResponseReceived handler.

        Note: if a service failure occurs, store an `Exception` sub class : `ServiceFailure` or `ServiceFault`.
        This exception will be raised in the `_AsyncRequestHandler._wait_for_response` function
        or in the `AsyncResponse.get_response` function in case of asynchronous response.
        """
        cdef void* response_cpy = NULL
        cdef _Request request = self._dRequestContext
        cdef OpcUa_ServiceFault* serviceFault = NULL

        if type == NULL or response == NULL:
            self._dServiceException = ServiceFailure('ServiceFailure : No response from server (maybe disconnected ?)', StatusCode.BadCommunicationError)
        elif type == &OpcUa_ServiceFault_EncodeableType:
            serviceFault = <OpcUa_ServiceFault*> response
            self._dServiceException = ServiceFault('ServiceFault received from server', serviceFault.ResponseHeader.ServiceResult)
        elif (type == &OpcUa_ReadResponse_EncodeableType or type == &OpcUa_WriteResponse_EncodeableType
             or type == &OpcUa_BrowseResponse_EncodeableType or type == &OpcUa_CallResponse_EncodeableType):
            status = SOPC_EncodeableObject_Create(type, &response_cpy)
            if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                raise SOPC_Failure('_on_response: SOPC_EncodeableObject_Create failed', status)
            status = SOPC_EncodeableObject_Copy(type, response_cpy, response)
            if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                raise SOPC_Failure('_on_response: SOPC_EncodeableObject_Copy failed', status)
            # push response and userContext in its class (Response)
            self._dResponseContext = Response.c_new_response(response_cpy, userContext)
        else:
            self._dServiceException = ServiceFailure('ServiceFailure : Response type does not match the type of the sent request', StatusCode.BadUnexpectedError)

        request._eventResponseReceived.set()


# --- Server ---

cdef bool _callback_get_server_key_password(char** outPassword) noexcept with gil:
    """
    Retrieve server key password
    """
    password = PyS2OPC_Server.get_server_key_password()
    b_password = password.encode() + b'\0'
    outPassword[0] = <char*> calloc(1, sizeof(char) * len(b_password))
    assert outPassword[0] != NULL, 'Out of memory (in _callback_get_key_password function)'
    strncpy(outPassword[0], b_password, len(b_password))
    return True

cdef void _callback_stop_server (SOPC_ReturnStatus status) noexcept with gil: # with gil, to avoid GIL release when server closing
    PyS2OPC_Server._serving = False

cdef void _callback_write_notification(const SOPC_CallContext* callCtxPtr, OpcUa_WriteValue* writeValue, SOPC_StatusCode writeStatus) noexcept with gil:
    """
    Callback from S2OPC Server, called when a write notification has arrived.
    This function retrieves the BaseAddressSpaceHandler class from the associated server and calls the _on_datachanged method.
    """
    assert callCtxPtr != NULL and writeValue != NULL
    if PyS2OPC_Server._adds_handler is not None:
        addSHandler: BaseAddressSpaceHandler = PyS2OPC_Server._adds_handler
        addSHandler._on_datachanged(callCtxPtr, writeValue, writeStatus)

cdef void _callback_LocalServiceAsyncResp(SOPC_EncodeableType* type, void* response, uintptr_t userContext) noexcept with gil:
    """
    Callback from S2OPC Server, called in response to a local service.
    This function retrieves the _AsyncRequestHandler class with `userContext` from the associated server and calls the _on_response method.
    """
    asyncReqHandler: _AsyncRequestHandler = PyS2OPC_Server._req_handler.pop(userContext)
    timestamp = time.time()
    asyncReqHandler._on_response(type, response, userContext, timestamp)

class PyS2OPC_Server(PyS2OPC):
    """
    The Server side of the PyS2OPC library.
    """
    _serving: bool = False
    """ Indicates server activity """
    _req_handler: dict[uintptr_t, AsyncReqHandler] = {}
    """ Store request with userContext as index """
    _adds_handler = None
    """ Store class/subclass of BaseAddressSpaceHandler """

    # @staticmethod : Python 3.9 does not support both and report error on staticmethod is not callable
    @contextmanager
    def initialize(logLevel: SOPC_Log_Level=SOPC_Log_Level.SOPC_LOG_LEVEL_DEBUG, logPath: str='logs/', logFileMaxBytes: int=1048576, logMaxFileNumber: int=50) -> None:
        """
        Toolkit initialization for Server.
        Call common initialization.

        This function supports the context management:
        >>> with PyS2OPC_Server.initialize():
        ...     # Do things here, namely configure then wait
        ...     pass

        When reaching out of the `with` statement, the Toolkit is automatically cleared.
        See `PyS2OPC_Server.clear`.

        Args:
            logLevel: log level (`SOPC_Log_Level`):
                ERROR: only errors are traced
                WARNING: only warnings and errors are traced
                INFO: information level, warnings and errors are traced
                DEBUG: all available information is traced
            logPath: the path for log files creation (might be relative to current working directory).
                     logPath is created if it does not exist.
            logFileMaxBytes: The maximum size of the log files, before changing the log index.
            logMaxFileNumber: The maximum number of log indexes before cycling logs and reusing the first log.
        """
        if logFileMaxBytes > UINT32_MAX:
            raise ValueError("logFileMaxBytes is too large (must be less than UINT32_MAX)")
        if logMaxFileNumber > UINT16_MAX:
            raise ValueError("logMaxFileNumber is too large (must be less than UINT16_MAX)")
        PyS2OPC.initialize(logLevel, logPath, logFileMaxBytes, logMaxFileNumber)
        status = SOPC_ServerConfigHelper_Initialize()
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Library server initialization failed ', status)
        PyS2OPC._initialized_srv = True
        try:
            yield
        finally:
            PyS2OPC_Server.clear()

    @staticmethod
    def clear() -> None:
        """
        Clear the server configuration and clear/stop the toolkit if applicable.
        """
        SOPC_ServerConfigHelper_Clear()
        PyS2OPC._initialized_srv = False
        if PyS2OPC._initialized_cli == False:
            PyS2OPC._clear()

    @staticmethod
    def load_server_configuration_from_files(xml_server_config_path: str, xml_address_space_config_path: str,
                                             xml_users_config_path: str, address_space_handler: BaseAddressSpaceHandler=None):
        """
        Configure server from XML configuration files for: server endpoints, address space and users credential and rights.
        This function must be called after `PyS2OPC_Server.initialize`.

        Args:
            xml_server_config_path: path to server configuration XML file ([s2opc_clientserver_config.xsd](https://gitlab.com/systerel/S2OPC/-/blob/master/schemas/s2opc_clientserver_config.xsd?ref_type=heads) schema)
            xml_address_space_config_path: path to address space configuration XML file ([UANodeSet.xsd](https://github.com/OPCFoundation/UA-Nodeset/blob/v1.04/Schema/UANodeSet.xsd) schema)
            xml_users_config_path: path to users credential and rights configuration XML file ([s2opc_clientserver_users_config.xsd](https://gitlab.com/systerel/S2OPC/-/blob/master/schemas/s2opc_clientserver_users_config.xsd?ref_type=heads) schema)
            address_space_handler: None (no write notification) or an instance of a subclass of `BaseAddressSpaceHandler`
        """
        # Set Password Callback
        SOPC_ServerConfigHelper_SetKeyPasswordCallback(_callback_get_server_key_password)
        # Set LocalServiceAsync Callback
        SOPC_ServerConfigHelper_SetLocalServiceAsyncResponse(_callback_LocalServiceAsyncResp)

        b_xml_server_config_path = xml_server_config_path.encode()
        b_xml_address_space_config_path = xml_address_space_config_path.encode()
        b_xml_users_config_path = xml_users_config_path.encode()
        status = SOPC_ServerConfigHelper_ConfigureFromXML(b_xml_server_config_path, b_xml_address_space_config_path,
                                                          b_xml_users_config_path, NULL)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Server configuration (from XML) failed ', status)
        # Set address space handler
        if address_space_handler is not None:
            assert isinstance(address_space_handler, BaseAddressSpaceHandler)
            PyS2OPC_Server._adds_handler = address_space_handler
            SOPC_ServerConfigHelper_SetWriteNotifCallback(_callback_write_notification)

    # @staticmethod : Python 3.9 does not support both and report error on staticmethod is not callable
    @contextmanager
    def serve() -> None:
        """
        Starts the server asynchronously.
        Server information node is updated and endpoints are asynchronously requested to be opened.

        Supports the context management facilities to close the server:
        >>> with PyS2OPC_Server.serve():
        ...     # Do things here
        ...     pass
        When reaching out of the `with` statement, the server is automatically closed.
        See `PyS2OPC_Server.stop_serve`.

        If you don't have applicative application, and callbacks are enough,
        see instead `PyS2OPC_Server.serve_forever`.
        """
        status = SOPC_ServerHelper_StartServer(_callback_stop_server)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('StartServer failed.', status)
        PyS2OPC_Server._serving = True
        try:
            yield
        finally:
            PyS2OPC_Server.stop_serve()

    @staticmethod
    def serving() -> bool:
        """
        Returns true if the server is in service, otherwise false.
        """
        return PyS2OPC_Server._serving

    @staticmethod
    def stop_serve() -> SOPC_ReturnStatus:
        """
        Stop the server (in a blocking way during shutdown phase).
        Then wait for `_callback_stop_server` before return, to avoid clearing the server too early.

        Note: Server stops after 5 seconds for shutdown phase to indicate shutdown in ServerState node
        """
        status = SOPC_ServerHelper_StopServer()
        while PyS2OPC_Server._serving: # wait for _callback_stop_server
            time.sleep(0.1)
        return status

    @staticmethod
    def serve_forever() -> None:
        """
        Start server forever.
        Can be interrupted with a `KeyboardInterrupt` (`SIGINT`).
        """
        with nogil:
            SOPC_ServerHelper_Serve(True)

    @staticmethod
    def get_server_key_password() -> str:
        """
        Default method that is called during configuration phase if an encrypted private key is used,
        it shall return the password to decrypt the server private key.

        It uses [get_pass](https://docs.python.org/3/library/getpass.html#module-getpass)
        library and homonymous function to prompt the user for password.

        It is possible to overwrite this function by assiging a new implementation to `PyS2OPC_Server.get_server_key_password`
        to obtain a different behavior.
        """
        return PyS2OPC._get_password("Server private key password:")

    # --- Local services implementation --- #

    @staticmethod
    def read_nodes(nodeIds: list[str], attributes: list[AttributeId]=[], bWaitResponse: bool=True) -> (Response):
        """
        Requests to execute the OPC UA Read service locally and returns the Read response content.
        See `ReadResponse` for result content.

        Args:
            nodeIds: The list of nodeIds to read (see ["NodeId concept"](#nodeid-concept)).
            attributes: The list of attributes to be read in the previous given  nodes (1 attribute per node).
                        See `AttributeId` enum values.
                        Automatically set to `AttributeId.Value` if empty.
            bWaitResponse: True, wait the response and returns a `ReadResponse`.
                           Otherwise return an `AsyncResponse` to retrieve it later
                           (see `AsyncResponse.get_response`).
        """
        request: _Request = _Request.new_read_request(nodeIds, attributes=attributes)
        ReqHandler = _AsyncRequestHandler()
        PyS2OPC_Server._req_handler[request._requestContext] = ReqHandler
        return ReqHandler._send_generic_request(True, NULL, request, bWaitResponse)

    @staticmethod
    def write_nodes(nodeIds: list[str], datavalues: list[DataValue], attributes: list[AttributeId]=[], bWaitResponse :bool=True) -> (Response):
        """
        Requests to execute the OPC UA Write service locally and returns the Write response content.
        See `WriteResponse` for result content.

        Args:
            nodeIds: The list of nodeIds to write (see ["NodeId concept"](#nodeid-concept)).
            datavalues: The list of DataValues to write (1 per node). see `DataValue`
            attributes: The list of attributes to be written for the given nodeIds (1 attribute per node).
                        Automatically set to `AttributeId.Value` if empty.
                        (only Value is supported for the moment)
            bWaitResponse: True, wait the response and returns a `WriteResponse`.
                           Otherwise return an `AsyncResponse` to retrieve it later
                           (see `AsyncResponse.get_response`).
        """
        for dv in datavalues:
            # Unset timestamp server since it is not writable in servers (it will lead to write failure)
            dv.timestampServer = None
        request: _Request = _Request.new_write_request(nodeIds, datavalues, attributes=attributes)
        ReqHandler = _AsyncRequestHandler()
        PyS2OPC_Server._req_handler[request._requestContext] = ReqHandler
        return ReqHandler._send_generic_request(True, NULL, request, bWaitResponse)

    @staticmethod
    def browse_nodes(nodeIds: list[str], maxReferencesPerNode: int = 1000, bWaitResponse: bool=True) -> (Response):
        """
        Requests to execute the OPC UA Browse service locally and returns the Browse response content.
        See `BrowseResponse` for result content.

        Args:
            nodeIds: The list of nodeIds to browse (see ["NodeId concept"](#nodeid-concept)).
            maxReferencesPerNode: Indicates the maximum number of references to return for each starting node
                                  specified in the request (0 means no limitation).
            bWaitResponse: True, wait the response and returns a `BrowseResponse`.
                           Otherwise return an `AsyncResponse` to retrieve it later
                           (see `AsyncResponse.get_response`).
        """
        request: _Request = _Request.new_browse_request(nodeIds, maxReferencesPerNode)
        ReqHandler = _AsyncRequestHandler()
        PyS2OPC_Server._req_handler[request._requestContext] = ReqHandler
        return ReqHandler._send_generic_request(True, NULL, request, bWaitResponse)

# --- Types ---

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
        nodeClass: NodeClass of the nodeId (see `NodeClass`).
        typeDefinition: NodeId of the type of the target node, when the target NodeId is a Variable or an Object.
                        It defines which VariableType or ObjectType node was used to instantiate the target node.
    """
    def __init__(self, referenceTypeId: str, isForward: bool, nodeId: str, browseName: tuple[uint16_t, str], displayName: tuple[str, str], nodeClass: NodeClass, typeDefinition: str):
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
        return "Reference(type='{}', forward={}) to node(id='{}', name='{}', class={}, type='{}')".format(self.referenceTypeId, self.isForward, self.nodeId, self.browseName, self.nodeClass, self.typeDefinition)

class BrowseResult:
    """
    The `BrowseResult` is a low-level structures that contains the results of browsing a single node.

    Attributes:
        status: the status code of the browse operation.
        continuationPoint: whether the browse is incomplete (continuationPoint not empty) or not.
        references: list of outgoing `pys2opc.Reference`s.
    """
    def __init__(self, statusCode: SOPC_ReturnStatus, continuationPoint: bytes, references: list[Reference]):
        self.status = statusCode
        self.continuationPoint = continuationPoint
        self.references = references

cdef class _C_BrowseResult:

    # Object variable #
    cdef OpcUa_BrowseResult* browse_result

    @staticmethod
    cdef _C_BrowseResult c_new_browse_result(OpcUa_BrowseResult* p_br):
        cdef _C_BrowseResult c_BR = _C_BrowseResult.__new__(_C_BrowseResult)
        c_BR.browse_result = p_br
        return c_BR

    @staticmethod
    cdef from_sopc_browseResult(OpcUa_BrowseResult* browseResult):
        """
        Converts a SOPC_DataValue* or SOPC_DataValue to a Python `DataValue`.
        """
        python_bytes: bytes = bytestring_to_bytes(&(browseResult.ContinuationPoint))
        references_list: list[Reference] = _C_BrowseResult.sopc_references_to_list(browseResult.References, browseResult.NoOfReferences)
        return BrowseResult(statusCode=browseResult.StatusCode, continuationPoint=python_bytes, references=references_list)

    @staticmethod
    cdef list sopc_references_to_list(OpcUa_ReferenceDescription* references, int32_t noOfReferences):

        reference_list: list[Reference] = []
        for i in range(noOfReferences):
            assert references[i].encodeableType == &OpcUa_ReferenceDescription_EncodeableType
            refType = SOPC_NodeId_ToCString(&(references[i].ReferenceTypeId))
            fwd: bool = references[i].IsForward
            expNid = expandednodeid_to_str(&(references[i].NodeId))
            bwsName = (references[i].BrowseName.NamespaceIndex, string_to_str(&(references[i].BrowseName.Name)))
            dispName = (string_to_str(&(references[i].DisplayName.defaultLocale)),
                        string_to_str(&(references[i].DisplayName.defaultText)))
            nodCls = references[i].NodeClass
            typeDef = expandednodeid_to_str(&(references[i].TypeDefinition))
            reference_list.append(Reference(refType.decode(), fwd, expNid, bwsName, dispName, nodCls, typeDef))
            free(refType)
        return reference_list

# --- Conversion functions : Python - C struct (S2OPC) --- #

cdef str string_to_str(SOPC_String* string):
    """ SOPC_String* to python str """
    if string == NULL or string.Data == NULL or string.Length <= 0:
        return ''
    return (string.Data).decode()

cdef bytes bytestring_to_bytes(SOPC_ByteString* bstring):
    """ SOPC_ByteString* to python bytes """
    if bstring == NULL or bstring.Data == NULL or bstring.Length <= 0:
        return b''
    return <bytes> bstring.Data[:bstring.Length]

cdef str nodeid_to_str(const SOPC_NodeId* node):
    """ SOPC_NodeId* to its str representation in the OPC UA XML syntax """
    return SOPC_NodeId_ToCString(node).decode()

cdef SOPC_NodeId* str_to_nodeid(str nid):
    """ Python str to SOPC_NodeId* """
    cdef SOPC_NodeId* node = NULL
    node = SOPC_NodeId_FromCString(nid, len(nid))
    assert node != NULL, 'SOPC_NodeId_FromCString failed on string "{}"'.format(nid)
    return node

cdef str expandednodeid_to_str(SOPC_ExpandedNodeId* exnode):
    """ SOPC_ExpandedNodeId* to its str representation in the OPC UA XML syntax """
    a = ''
    if exnode.ServerIndex:
        a += 'srv={};'.format(exnode.ServerIndex)
    nsu = string_to_str(&exnode.NamespaceUri)
    if nsu:
        a += 'nsu={};'.format(nsu)
    b = SOPC_NodeId_ToCString(&(exnode.NodeId))
    b_python: str = b.decode()
    free(b)
    return a + b_python

cdef datetime_to_float(SOPC_DateTime datetime):
    """
    SOPC_DateTime (the number of 100 nanosecond intervals since January 1, 1601)
    to Python time (the floating point number of seconds since 01/01/1970, see help(time)).

    Warning, as opposed to other sopc_type_to_python_type functions, this one does not accept pointers.
    """
    # (datetime.date(1970,1,1) - datetime.date(1601,1,1)).total_seconds() * 1000 * 1000 * 10
    # When datetime is NULL, this results in some minutes that are due to the int->double conversion.
    return (datetime - 116444736000000000)/1e7

cdef SOPC_DateTime float_to_datetime(t):
    """ Python timestamp to SOPC_DateTime """
    cdef SOPC_DateTime datetime = int(t*1e7) + 116444736000000000
    return datetime

cdef guid_to_uuid(SOPC_Guid* guid):
    """ SOPC_Guid* to the Python's uuid """
    # S2OPC internal representation is local-endian, except for Data4,
    #  which is always big endian.
    a = '{:08X}-{:04X}-{:04X}-'.format(guid.Data1, guid.Data2, guid.Data3)
    b = hexlify(bytes(guid.Data4)).decode()
    c = b[:4]+'-'+b[4:]
    return uuid.UUID(a+c)

cdef void uuid_to_guid(uid, SOPC_Guid* guid_dest):
    """ uuid.UUID object to SOPC_Guid* """
    guid_dest.Data1, guid_dest.Data2, guid_dest.Data3 = uid.fields[:3]
    for i,b in enumerate(uid.bytes[-8:]):
        guid_dest.Data4[i] = b
    return

# --- Client --- #

cdef bool _callback_get_client_username_password(const SOPC_SecureConnection_Config* secConnConfig, char** outUserName, char** outPassword) noexcept with gil:
    userName, password = PyS2OPC_Client.get_username_password()
    if None == userName or None == password:
        return False

    b_userName = userName.encode() + b'\0'
    outUserName[0] = <char*> calloc(1, sizeof(char) * len(b_userName))
    assert outUserName[0] != NULL, 'Out of memory (in _callback_get_client_username_password function)'
    strncpy(outUserName[0], b_userName, len(b_userName))

    b_password = password.encode() + b'\0'
    outPassword[0] = <char*> calloc(1, sizeof(char) * len(b_password))
    assert outPassword[0] != NULL, 'Out of memory (in _callback_get_client_username_password function)'
    strncpy(outPassword[0], b_password, len(b_password))
    return True

cdef bool _callback_get_client_user_cert_key_password(const SOPC_SecureConnection_Config* secConnConfig, const char* userCertThumb, char** outPassword) noexcept with gil:
    certThumb: str = userCertThumb.decode()
    password: str = PyS2OPC_Client.get_user_X509_key_password(certThumb)
    if None == password:
        return False

    b_password = password.encode() + b'\0'
    outPassword[0] = <char*> calloc(1, sizeof(char) * len(b_password))
    assert outPassword[0] != NULL, 'Out of memory (in _callback_get_client_user_cert_key_password)'
    strncpy(outPassword[0], b_password, len(b_password))
    return True

cdef bool _callback_get_client_key_password(char** outPassword) noexcept with gil:
    password = PyS2OPC_Client.get_client_key_password()
    if None == password:
        return False

    b_password = password.encode() + b'\0'
    outPassword[0] = <char*> calloc(1, sizeof(char) * len(b_password))
    assert outPassword[0] != NULL, 'Out of memory (in _callback_get_key_password function)'
    strncpy(outPassword[0], b_password, len(b_password))
    return True

cdef void _callback_client_connection_event(SOPC_ClientConnection* conn, SOPC_ClientConnectionEvent event, SOPC_StatusCode status) noexcept with gil:
    cdef uintptr_t id_sc = <uintptr_t> (<void*> conn)
    connectHandler: BaseClientConnectionHandler = PyS2OPC_Client._dConnectHandler[id_sc]
    if connectHandler is not None and event == SOPC_ClientConnectionEvent.SOPC_ClientConnectionEvent_Disconnected:
        connectHandler: BaseClientConnectionHandler = PyS2OPC_Client._dConnectHandler[id_sc]
        connectHandler._connected = False

cdef void _callback_ClientServiceAsyncResp(SOPC_EncodeableType* type, const void* response, uintptr_t userContext) noexcept with gil:
    asyncReqHandler: _AsyncRequestHandler = BaseClientConnectionHandler._req_handler.pop(userContext)
    timestamp = time.time()
    asyncReqHandler._on_response(type, response, userContext, timestamp)

cdef class _C_Configurations:
    """
    Stores Client C connection configurations.
    """

    # Object variable #
    cdef size_t _nbConfigs
    cdef SOPC_SecureConnection_Config** _scConfigArray

    def __cinit__(self):
        pass

    @staticmethod
    cdef _C_Configurations c_new_conf(size_t nbConfigs, SOPC_SecureConnection_Config** scConfigArray):
        cdef _C_Configurations conf = _C_Configurations.__new__(_C_Configurations)
        conf._nbConfigs = nbConfigs
        conf._scConfigArray = scConfigArray
        return conf

    @staticmethod
    cdef SOPC_SecureConnection_Config* getConfig(_C_Configurations configs, size_t configIdx):
        assert configs._nbConfigs > configIdx
        return configs._scConfigArray[configIdx]

class ConnectionConfiguration:
    """
    The connection configuration to be used with `PyS2OPC_Client.connect` function.
    """
    def __init__(self, configs : _C_Configurations, cfgIdx : int):
        assert cfgIdx < configs._nbConfigs
        self._configs = configs
        self._cfgIdx = cfgIdx

    @property
    def connectionId(self) -> str:
        return SOPC_ClientConfigHelper_GetUserIdFromConfig(_C_Configurations.getConfig(self._configs, self._cfgIdx)).decode()

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

    Attributes:
        value: The python value of the `Variant`.
        variantType: Optional: The type of the `Variant`.
    """

    def __init__(self, python_value, variantType: VariantType|None = None):
        self.value = python_value
        self.variantType = variantType

    def __repr__(self):
        return 'Variant(' + repr(self.value) + ')'
    def __str__(self):
        return str(self.value)

    def __eq__(s, o):
        if isinstance(o, Variant):
            return s.value == o.value
        return s.value == o
    def __lt__(s, o):
        if isinstance(o, Variant):
            return s.value < o.value
        return s.value < o

    # Arithmetics
    def __add__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value + o.value)
        return Variant(s.value + o)
    def __sub__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value - o.value)
        return Variant(s.value - o)
    def __mul__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value * o.value)
        return Variant(s.value * o)
    #def __matmul__(s, o):
    #    if isinstance(o, Variant):
    #        return Variant(s.value @ o.value)
    #    return Variant(s.value @ o)
    def __truediv__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value / o.value)
        return Variant(s.value / o)
    def __floordiv__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value // o.value)
        return Variant(s.value // o)
    def __mod__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value % o.value)
        return Variant(s.value % o)
    def __divmod__(s, o):
        if isinstance(o, Variant):
            return Variant(divmod(s.value, o.value))
        return Variant(divmod(s.value, o))
    # def __pow__(s, o, *args):
    #     if isinstance(o, Variant):
    #         return Variant(pow(s.value, o.value, *args))
    #     return Variant(pow(s.value, o, *args))
    def __lshift__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value << o.value)
        return Variant(s.value << o)
    def __rshift__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value >> o.value)
        return Variant(s.value >> o)
    def __and__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value & o.value)
        return Variant(s.value & o)
    def __xor__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value ^ o.value)
        return Variant(s.value ^ o)
    def __or__(s, o):
        if isinstance(o, Variant):
            return Variant(s.value | o.value)
        return Variant(s.value | o)
    def __radd__(s, o):
        return Variant(o + s.value)
    def __rsub__(s, o):
        return Variant(o - s.value)
    def __rmul__(s, o):
        return Variant(o * s.value)
    #def __rmatmul__(s, o):
    #    return Variant(o @ s.value)
    def __rtruediv__(s, o):
        return Variant(o / s.value)
    def __rfloordiv__(s, o):
        return Variant(o // s.value)
    def __rmod__(s, o):
        return Variant(o % s.value)
    def __rdivmod__(s, o):
        return Variant(divmod(o, s.value))
    def __rpow__(s, o):
        return Variant(pow(o, s.value))
    def __rlshift__(s, o):
        return Variant(o << s.value)
    def __rrshift__(s, o):
        return Variant(o >> s.value)
    def __rand__(s, o):
        return Variant(o & s.value)
    def __rxor__(s, o):
        return Variant(o ^ s.value)
    def __ror__(s, o):
        return Variant(o | s.value)
    def __iadd__(s, o):
        if isinstance(o, Variant):
            s.value += o.value
        else:
            s.value += o
        return s
    def __isub__(s, o):
        if isinstance(o, Variant):
            s.value -= o.value
        else:
            s.value -= o
        return s
    def __imul__(s, o):
        if isinstance(o, Variant):
            s.value *= o.value
        else:
            s.value *= o
        return s
    #def __imatmul__(s, o):
    #    if isinstance(o, Variant):
    #        s.value @= o.value
    #    else:
    #        s.value @= o
    #    return s
    def __itruediv__(s, o):
        if isinstance(o, Variant):
            s.value /= o.value
        else:
            s.value /= o
        return s
    def __ifloordiv__(s, o):
        if isinstance(o, Variant):
            s.value //= o.value
        else:
            s.value //= o
        return s
    def __imod__(s, o):
        if isinstance(o, Variant):
            s.value %= o.value
        else:
            s.value %= o
        return s
    def __ipow__(s, o):
        if isinstance(o, Variant):
            s.value **= o.value
        else:
            s.value **= o
        return s
    def __ilshift__(s, o):
        if isinstance(o, Variant):
            s.value <<= o.value
        else:
            s.value <<= o
        return s
    def __irshift__(s, o):
        if isinstance(o, Variant):
            s.value >>= o.value
        else:
            s.value >>= o
        return s
    def __iand__(s, o):
        if isinstance(o, Variant):
            s.value &= o.value
        else:
            s.value &= o
        return s
    def __ixor__(s, o):
        if isinstance(o, Variant):
            s.value ^= o.value
        else:
            s.value ^= o
        return s
    def __ior__(s, o):
        if isinstance(o, Variant):
            s.value |= o.value
        else:
            s.value |= o
        return s
    def __neg__(s):
        return Variant(-s.value)
    def __pos__(s):
        return Variant(+s.value)
    def __abs__(s):
        return Variant(abs(s.value))
    def __invert__(s):
        return Variant(~s.value)
    def __complex__(s):
        return complex(s.value)
    def __int__(s):
        return int(s.value)
    def __float__(s):
        return float(s.value)
    def __index__(s):
        return int(s.value)
    def __round__(s, *args):
        return round(s.value)
    # def __trunc__(s):
    #     return trunc(s.value)
    # def __floor__(s):
    #     return floor(s.value)
    # def __ceil__(s):
    #     return ceil(s.value)

    # Containers
    def __len__(s):
        return len(s.value)
    def __getitem__(s, k):
        return s.value[k]
    def __setitem__(s, k, v):
        s.value[k] = v
    def __delitem__(s, k):
        del s.value[k]
    def __iter__(s):
        yield from iter(s.value)  # Requires Python3 >= 3.3
    def __reversed__(s):
        yield from reversed(s.value)
    def __contains__(s, k):
        return k in s.value

    def get_python(self):
        """
        Returns The python value of the `Variant`
        """
        return self.value

cdef class _C_Variant:

    # Object variable #
    cdef SOPC_Variant* p_sopc_var

    @staticmethod
    cdef _C_Variant c_new_variant(SOPC_Variant* p_var):
        cdef _C_Variant C_VAR = _C_Variant.__new__(_C_Variant)
        C_VAR.p_sopc_var = p_var
        return C_VAR

    @staticmethod
    cdef from_sopc_variant(SOPC_Variant* pVariant):
        """
        Returns a Variant initialized from a SOPC_Variant* (or a void*).
        """
        sopc_type = pVariant.BuiltInTypeId

        if pVariant.ArrayType == SOPC_VariantArrayType.SOPC_VariantArrayType_SingleValue:
            if sopc_type == SOPC_BuiltinId.SOPC_Null_Id:
                return Variant(None, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Boolean_Id:
                return Variant(pVariant.Value.Boolean, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_SByte_Id:
                return Variant(pVariant.Value.Sbyte, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Byte_Id:
                return Variant(pVariant.Value.Byte, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Int16_Id:
                return Variant(pVariant.Value.Int16, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt16_Id:
                return Variant(pVariant.Value.Uint16, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Int32_Id:
                return Variant(pVariant.Value.Int32, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt32_Id:
                return Variant(pVariant.Value.Uint32, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Int64_Id:
                return Variant(pVariant.Value.Int64, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt64_Id:
                return Variant(pVariant.Value.Uint64, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Float_Id:
                return Variant(pVariant.Value.Floatv, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Double_Id:
                return Variant(pVariant.Value.Doublev, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_String_Id:
                return Variant(string_to_str(&(pVariant.Value.String)), sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_DateTime_Id:
                return Variant(datetime_to_float(pVariant.Value.Date), sopc_type)  # int64_t
            elif sopc_type == SOPC_BuiltinId.SOPC_Guid_Id:
                return Variant(guid_to_uuid(pVariant.Value.Guid), sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_ByteString_Id:
                return Variant(bytestring_to_bytes(&(pVariant.Value.Bstring)), sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_XmlElement_Id:
                return Variant(bytestring_to_bytes(&(pVariant.Value.XmlElt)), sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_NodeId_Id:
                return Variant(nodeid_to_str(pVariant.Value.NodeId), sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_ExpandedNodeId_Id:
            #    return Variant(pVariant.Value., sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_StatusCode_Id:
                return Variant(pVariant.Value.Status, sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_QualifiedName_Id:
                return Variant((pVariant.Value.Qname.NamespaceIndex, string_to_str(&(pVariant.Value.Qname.Name))), sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_LocalizedText_Id:
                return Variant((string_to_str(&(pVariant.Value.LocalizedText.defaultLocale)), string_to_str(&(pVariant.Value.LocalizedText.defaultText))), sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_ExtensionObject_Id:
            #    return Variant(pVariant.Value., sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_DataValue_Id:
            #    return Variant(pVariant.Value., sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_Variant_Id:
            #    return Variant(pVariant.Value., sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_DiagnosticInfo_Id:
            #    return Variant(pVariant.Value., sopc_type)
            raise ValueError('SOPC_Variant to Python conversion not supported for built-in type {}.'.format(sopc_type))
        elif pVariant.ArrayType == SOPC_VariantArrayType.SOPC_VariantArrayType_Array:
            if sopc_type == SOPC_BuiltinId.SOPC_Null_Id:
                # S2OPC should not be able to be in this case
                return Variant([], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Boolean_Id:
                return Variant([pVariant.Value.Array.Content.BooleanArr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_SByte_Id:
                return Variant([pVariant.Value.Array.Content.SbyteArr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Byte_Id:
                return Variant([pVariant.Value.Array.Content.ByteArr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Int16_Id:
                return Variant([pVariant.Value.Array.Content.Int16Arr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt16_Id:
                return Variant([pVariant.Value.Array.Content.Uint16Arr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Int32_Id:
                return Variant([pVariant.Value.Array.Content.Int32Arr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt32_Id:
                return Variant([pVariant.Value.Array.Content.Uint32Arr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Int64_Id:
                return Variant([pVariant.Value.Array.Content.Int64Arr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt64_Id:
                return Variant([pVariant.Value.Array.Content.Uint64Arr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Float_Id:
                return Variant([pVariant.Value.Array.Content.FloatvArr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Double_Id:
                return Variant([pVariant.Value.Array.Content.DoublevArr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_String_Id:
                return Variant([string_to_str(&(pVariant.Value.Array.Content.StringArr[i])) for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_DateTime_Id:
                return Variant([datetime_to_float(pVariant.Value.Array.Content.DateArr[i]) for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_Guid_Id:
                return Variant([guid_to_uuid(&(pVariant.Value.Array.Content.GuidArr[i])) for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_ByteString_Id:
                return Variant([bytestring_to_bytes(&(pVariant.Value.Array.Content.BstringArr[i])) for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_XmlElement_Id:
                return Variant([bytestring_to_bytes(&(pVariant.Value.Array.Content.XmlEltArr[i])) for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_NodeId_Id:
                return Variant([nodeid_to_str(&(pVariant.Value.Array.Content.NodeIdArr[i])) for i in range(pVariant.Value.Array.Length)], sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_ExpandedNodeId_Id:
            #    return Variant([pVariant.Value.Array.Content. for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_StatusCode_Id:
                return Variant([pVariant.Value.Array.Content.StatusArr[i] for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_QualifiedName_Id:
                return Variant([(pVariant.Value.Array.Content.QnameArr[i].NamespaceIndex, string_to_str(&(pVariant.Value.Array.Content.QnameArr[i].Name))) for i in range(pVariant.Value.Array.Length)], sopc_type)
            elif sopc_type == SOPC_BuiltinId.SOPC_LocalizedText_Id:
                return Variant([(string_to_str(&(pVariant.Value.Array.Content.LocalizedTextArr[i].defaultLocale)), string_to_str(&(pVariant.Value.Array.Content.LocalizedTextArr[i].defaultText))) for i in range(pVariant.Value.Array.Length)], sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_ExtensionObject_Id:
            #    return Variant([content. for i in range(length)], sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_DataValue_Id:
            #    return Variant([content. for i in range(length)], sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_Variant_Id:
            #    return Variant([content. for i in range(length)], sopc_type)
            #elif sopc_type == SOPC_BuiltinId.SOPC_DiagnosticInfo_Id:
            #    return Variant([content. for i in range(length)], sopc_type)
            raise ValueError('SOPC_Variant to Python conversion not supported for built-in type {}.'.format(sopc_type))
        elif pVariant.ArrayType == SOPC_VariantArrayType.SOPC_VariantArrayType_Matrix:
            raise ValueError('SOPC_Variant matrices are not supported.')
        else:
            raise ValueError('Invalid SOPC_Variant.ArrayType')

    @staticmethod
    cdef SOPC_Variant* to_sopc_variant(var: Variant, SOPC_Variant* variant):
        """
        Converts the current Variant to a SOPC_Variant*.
        Handles both single values and array values.

        Args:
            var: [IN] A Python Variant.
            variant: [OUT] The C SOPC_Variant*.
        """
        # Need to declare cdef variable before condition
        cdef SOPC_QualifiedName* qname = NULL
        cdef SOPC_LocalizedText* loc = NULL
        cdef SOPC_ReturnStatus status = SOPC_ReturnStatus.SOPC_STATUS_OK

        # Create and fill variant
        if var.variantType == None:
            raise ValueError('Python to SOPC_Variant conversion not supported for unknown variant type with content: {}.'.format(var))
        cdef SOPC_BuiltinId sopc_type = var.variantType
        variant_value = var.value
        variant.BuiltInTypeId = sopc_type
        # Testing whether this is an array or not is not straightforward because of qualified names and localized text that are couples
        if sopc_type in (SOPC_BuiltinId.SOPC_QualifiedName_Id, SOPC_BuiltinId.SOPC_LocalizedText_Id):
            is_array = len(variant_value) > 0 and isinstance(variant_value[0], (list, tuple))  # If self.value has no length -> malformed value
        else:
            is_array = isinstance(variant_value, (list, tuple))
        if not is_array:
            # Single values
            variant.ArrayType = SOPC_VariantArrayType.SOPC_VariantArrayType_SingleValue
            if sopc_type == SOPC_BuiltinId.SOPC_Null_Id:
                pass
            elif sopc_type == SOPC_BuiltinId.SOPC_Boolean_Id:
                variant.Value.Boolean = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_SByte_Id:
                variant.Value.Sbyte = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_Byte_Id:
                variant.Value.Byte = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_Int16_Id:
                variant.Value.Int16 = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt16_Id:
                variant.Value.Uint16 = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_Int32_Id:
                variant.Value.Int32 = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt32_Id:
                variant.Value.Uint32 = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_Int64_Id:
                variant.Value.Int64 = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt64_Id:
                variant.Value.Uint64 = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_Float_Id:
                variant.Value.Floatv = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_Double_Id:
                variant.Value.Doublev = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_String_Id:
                status = SOPC_String_CopyFromCString(&(variant.Value.String), variant_value.encode())
                if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                    raise SOPC_Failure('Failed to convert String python variant {} to C variant'.format(variant_value), status)
            elif sopc_type == SOPC_BuiltinId.SOPC_DateTime_Id:
                variant.Value.Date = float_to_datetime(variant_value)
            elif sopc_type == SOPC_BuiltinId.SOPC_Guid_Id:
                variant.Value.Guid = <SOPC_Guid*> calloc(1, sizeof(SOPC_Guid))
                uuid_to_guid(variant_value, variant.Value.Guid)
            elif sopc_type == SOPC_BuiltinId.SOPC_ByteString_Id:
                status = SOPC_ByteString_CopyFromBytes(&(variant.Value.Bstring), variant_value, len(variant_value))
                if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                    raise SOPC_Failure('Failed to convert the ByteString python variant {} to C variant'.format(variant_value), status)
            elif sopc_type == SOPC_BuiltinId.SOPC_XmlElement_Id:
                status = SOPC_ByteString_CopyFromBytes(&(variant.Value.XmlElt), variant_value, len(variant_value))
                if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                    raise SOPC_Failure('Failed to convert the XmlElement python variant {} to C variant'.format(variant_value), status)
            elif sopc_type == SOPC_BuiltinId.SOPC_NodeId_Id:
                variant.Value.NodeId = str_to_nodeid(variant_value)
            #elif sopc_type == SOPC_BuiltinId.SOPC_ExpandedNodeId_Id:
            #    variant.Value. = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_StatusCode_Id:
                variant.Value.Status = variant_value
            elif sopc_type == SOPC_BuiltinId.SOPC_QualifiedName_Id:
                qname = <SOPC_QualifiedName*> calloc(1, sizeof(SOPC_QualifiedName))
                qname.NamespaceIndex = variant_value[0]
                # qname.Name = str_to_string(variant_value[1])[0]
                SOPC_String_CopyFromCString(&(qname.Name), variant_value[1])
                variant.Value.Qname = qname
            elif sopc_type == SOPC_BuiltinId.SOPC_LocalizedText_Id:
                loc = <SOPC_LocalizedText*> calloc(1, sizeof(SOPC_LocalizedText))
                SOPC_String_CopyFromCString(&(loc.defaultLocale), variant_value[0])
                SOPC_String_CopyFromCString(&(loc.defaultText), variant_value[1])
                # loc.defaultLocale, loc.defaultText = map(lambda s:str_to_string(s)[0], variant_value)
                variant.Value.LocalizedText = loc
            #elif sopc_type == SOPC_BuiltinId.SOPC_ExtensionObject_Id:
            #    variant.Value. = variant_value
            #elif sopc_type == SOPC_BuiltinId.SOPC_DataValue_Id:
            #    variant.Value. = variant_value
            #elif sopc_type == SOPC_BuiltinId.SOPC_Variant_Id:
            #    variant.Value. = variant_value
            #elif sopc_type == SOPC_BuiltinId.SOPC_DiagnosticInfo_Id:
            #    variant.Value. = variant_value
            else:
                raise ValueError('Python to SOPC_Variant conversion not supported for the given type {}.'.format(sopc_type))
        else:
            # Arrays or Matrices values (but not Matrices)
            # assert not any(map(lambda n:isinstance(n, (list, tuple)), self.value)), 'Multi dimensional arrays are not supported.'
            variant.ArrayType = SOPC_VariantArrayType.SOPC_VariantArrayType_Array
            variant.Value.Array.Length = len(variant_value)
            ## content = variant.Value.Array.Content
            if sopc_type == SOPC_BuiltinId.SOPC_Null_Id:
                pass
            elif sopc_type == SOPC_BuiltinId.SOPC_Boolean_Id:
                variant.Value.Array.Content.BooleanArr = <SOPC_Boolean*> calloc(len(variant_value), sizeof(SOPC_Boolean))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.BooleanArr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_SByte_Id:
                variant.Value.Array.Content.SbyteArr = <SOPC_SByte*> calloc(len(variant_value), sizeof(SOPC_SByte))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.SbyteArr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_Byte_Id:
                variant.Value.Array.Content.ByteArr = <SOPC_Byte*> calloc(len(variant_value), sizeof(SOPC_Byte))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.ByteArr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_Int16_Id:
                variant.Value.Array.Content.Int16Arr = <int16_t*> calloc(len(variant_value), sizeof(int16_t))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.Int16Arr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt16_Id:
                variant.Value.Array.Content.Uint16Arr = <uint16_t*> calloc(len(variant_value), sizeof(uint16_t))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.Uint16Arr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_Int32_Id:
                variant.Value.Array.Content.Int32Arr = <int32_t*> calloc(len(variant_value), sizeof(int32_t))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.Int32Arr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt32_Id:
                variant.Value.Array.Content.Uint32Arr = <uint32_t*> calloc(len(variant_value), sizeof(uint32_t))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.Uint32Arr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_Int64_Id:
                variant.Value.Array.Content.Int64Arr = <int64_t*> calloc(len(variant_value), sizeof(int64_t))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.Int64Arr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_UInt64_Id:
                variant.Value.Array.Content.Uint64Arr = <uint64_t*> calloc(len(variant_value), sizeof(uint64_t))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.Uint64Arr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_Float_Id:
                variant.Value.Array.Content.FloatvArr = <float*> calloc(len(variant_value), sizeof(float))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.FloatvArr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_Double_Id:
                variant.Value.Array.Content.DoublevArr = <double*> calloc(len(variant_value), sizeof(double))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.DoublevArr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_String_Id:
                variant.Value.Array.Content.StringArr = <SOPC_String*> calloc(len(variant_value), sizeof(SOPC_String))
                for i, val in enumerate(variant_value):
                    status = SOPC_String_CopyFromCString(&(variant.Value.Array.Content.StringArr[i]), val)
                    if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                        raise SOPC_Failure('Failed to convert the String python variant {} to C variant'.format(val), status)
            elif sopc_type == SOPC_BuiltinId.SOPC_DateTime_Id:
                variant.Value.Array.Content.DateArr = <SOPC_DateTime*> calloc(len(variant_value), sizeof(SOPC_DateTime))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.DateArr[i] = float_to_datetime(val)
            elif sopc_type == SOPC_BuiltinId.SOPC_Guid_Id:
                variant.Value.Array.Content.GuidArr = <SOPC_Guid*> calloc(len(variant_value), sizeof(SOPC_Guid))
                for i, val in enumerate(variant_value):
                    uuid_to_guid(variant_value, &(variant.Value.Array.Content.GuidArr[i]))
                    # variant.Value.Array.Content.GuidArr[i] = uuid_to_guid(val)[0] # Double allocation
            elif sopc_type == SOPC_BuiltinId.SOPC_ByteString_Id:
                variant.Value.Array.Content.BstringArr = <SOPC_ByteString*> calloc(len(variant_value), sizeof(SOPC_ByteString))
                for i, val in enumerate(variant_value):
                    status = SOPC_ByteString_CopyFromBytes(&(variant.Value.Array.Content.BstringArr[i]), val, len(val))
                    if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                        raise SOPC_Failure('Failed to convert the ByteString python variant {} to C variant'.format(val), status)
            elif sopc_type == SOPC_BuiltinId.SOPC_XmlElement_Id:
                variant.Value.Array.Content.XmlEltArr = <SOPC_XmlElement*> calloc(len(variant_value), sizeof(SOPC_XmlElement))
                for i, val in enumerate(variant_value):
                    status = SOPC_ByteString_CopyFromBytes(&(variant.Value.Array.Content.XmlEltArr[i]), val, len(val))
                    if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                        raise SOPC_Failure('Failed to convert the XmlElement python variant {} to C variant'.format(val), status)
            elif sopc_type == SOPC_BuiltinId.SOPC_NodeId_Id:
                variant.Value.Array.Content.NodeIdArr = <SOPC_NodeId*> calloc(len(variant_value), sizeof(SOPC_NodeId))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.NodeIdArr[i] = str_to_nodeid(val)[0] # Double allocation
            #elif sopc_type == SOPC_BuiltinId.SOPC_ExpandedNodeId_Id:
            #    content.Arr = allocator_no_gc('[]', variant_value)
            elif sopc_type == SOPC_BuiltinId.SOPC_StatusCode_Id:
                variant.Value.Array.Content.StatusArr = <SOPC_StatusCode*> calloc(len(variant_value), sizeof(SOPC_StatusCode))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.StatusArr[i] = val
            elif sopc_type == SOPC_BuiltinId.SOPC_QualifiedName_Id:
                variant.Value.Array.Content.QnameArr = <SOPC_QualifiedName*> calloc(len(variant_value), sizeof(SOPC_QualifiedName))
                for i, val in enumerate(variant_value):
                    variant.Value.Array.Content.QnameArr[i].NamespaceIndex = val[0]
                    SOPC_String_CopyFromCString(&(variant.Value.Array.Content.QnameArr[i].Name), val[1])
            elif sopc_type == SOPC_BuiltinId.SOPC_LocalizedText_Id:
                variant.Value.Array.Content.LocalizedTextArr = <SOPC_LocalizedText*> calloc(len(variant_value), sizeof(SOPC_LocalizedText))
                for i, val in enumerate(variant_value):
                    SOPC_String_CopyFromCString(&(variant.Value.Array.Content.LocalizedTextArr[i].defaultLocale), val[0])
                    SOPC_String_CopyFromCString(&(variant.Value.Array.Content.LocalizedTextArr[i].defaultText), val[1])
            #elif sopc_type == SOPC_BuiltinId.SOPC_ExtensionObject_Id:
            #    content.Arr = allocator_no_gc('[]', variant_value)
            #elif sopc_type == SOPC_BuiltinId.SOPC_DataValue_Id:
            #    content.Arr = allocator_no_gc('[]', variant_value)
            #elif sopc_type == SOPC_BuiltinId.SOPC_Variant_Id:
            #    content.Arr = allocator_no_gc('[]', variant_value)
            #elif sopc_type == SOPC_BuiltinId.SOPC_DiagnosticInfo_Id:
            #    content.Arr = allocator_no_gc('[]', variant_value)
            else:
                raise ValueError('Python to SOPC_Variant conversion not supported for the given type {}.'.format(sopc_type))
        return variant

class DataValue:
    """
    Representation of the OPC UA DataValue.

    Attributes:
        variant: The `pys2opc.Variant` storing the value.
        variantType: An accessor to the `variantType` attribute of `pys2opc.Variant`.
        timestampSource: The last time the value changed, specified by the writer, as a Python timestamp.
        timestampServer: The last time the value changed, according to the server, as a Python timestamp.
        statusCode: The quality associated to the value OR the reason why there is no available value.
                    It is a value from the `pys2opc.StatusCode` enum (e.g. `pys2opc.StatusCode.BadAttributeIdInvalid`).
    """
    def __init__(self, variant: Variant, timestampSource: float|None = None, timestampServer: float|None = None, statusCode: StatusCode = StatusCode.Good):
        self.timestampSource = timestampSource
        self.timestampServer = timestampServer
        self.statusCode = statusCode
        self.variant = variant

    def __repr__(self):
        return 'DataValue('+repr(self.variant)+')'
    def __str__(self):
        return repr(self)

    @property
    def variantType(self) -> VariantType:
        """
        variantType: An accessor to the `variantType` attribute of the `pys2opc.Variant`.
        """
        return self.variant.variantType

    @variantType.setter
    def variantType(self, ty : VariantType):
        self.variant.variantType = ty

    @staticmethod
    def from_python(val, setTs : bool = True) -> DataValue:
        """
        Creates a DataValue from a Python value or a `pys2opc.Variant`.
        Creates the Variant, sets the status code to OK, and set source timestamp to current time
        (when setTs is `True`).

        Args:
           val: a builtin Python value or a `pys2opc.Variant`.
           setTs: set the source timestamp to the current timestamp
        """
        if not isinstance(val, Variant):
            val = Variant(val)

        sourceTs = None
        if setTs:
            sourceTs = time.time()
        return DataValue(val, sourceTs, None, StatusCode.Good)

    def get_python(self):
        """
        Returns the python object wrapped by this `DataValue`.
        Accessor to `pys2opc.Variant.get_python`.
        Use this when it is known that the Variant object will not be reused (e.g. by a future call to write_nodes).

        Does not copy the object before returning it.
        """
        return self.variant.get_python()

cdef class _C_DataValue:
    """
    Keep all `cdef` methods of DataValue class, and the C SOPC_DataValue*
    """

    # Object variable #
    cdef SOPC_DataValue* p_sopc_dv

    @staticmethod
    cdef _C_DataValue c_new_datavalue(SOPC_DataValue* p_dv):
        cdef _C_DataValue c_DV = _C_DataValue.__new__(_C_DataValue)
        c_DV.p_sopc_dv = p_dv
        return c_DV

    @staticmethod
    cdef from_sopc_datavalue(SOPC_DataValue* datavalue):
        """
        Converts a SOPC_DataValue* to a Python `DataValue`.
        """
        return DataValue(_C_Variant.from_sopc_variant(&(datavalue.Value)),
                         datetime_to_float(datavalue.SourceTimestamp),
                         datetime_to_float(datavalue.ServerTimestamp),
                         datavalue.Status)

    @staticmethod
    cdef SOPC_DataValue* to_sopc_datavalue(DV : DataValue, SOPC_DataValue* datavalue):
        """
        Converts a python DataValue to a C SOPC_DataValue*.

        Args:
            var: [IN] A Python DataValue.
            variant: [OUT] The C SOPC_DataValue*.
        """
        _C_Variant.to_sopc_variant(DV.variant, &(datavalue.Value))
        datavalue.Status = DV.statusCode
        source = float_to_datetime(DV.timestampSource) if DV.timestampSource else (0)
        server = float_to_datetime(DV.timestampServer) if DV.timestampServer else (0)
        datavalue.SourceTimestamp = source
        datavalue.ServerTimestamp = server
        datavalue.SourcePicoSeconds = 0
        datavalue.ServerPicoSeconds = 0
        return datavalue

cdef class BaseAddressSpaceHandler:
    """
    Base class for the Address Space notification callback.
    You should derive this class and re-implement its `BaseAddressSpaceHandler.on_datachanged` method.
    """
    cdef _on_datachanged(self, const SOPC_CallContext* ctx, OpcUa_WriteValue* value, SOPC_StatusCode status):
        assert value.encodeableType == &OpcUa_WriteValue_EncodeableType
        self.on_datachanged(nodeid_to_str(&(value.NodeId)), value.AttributeId,
                            _C_DataValue.from_sopc_datavalue(&(value.Value)),
                            string_to_str(&(value.IndexRange)), status)

    def on_datachanged(self, nodeId: str, attrId: AttributeId, dataValue: DataValue, indexRange: str, status: SOPC_StatusCode):
        """
        This is the main callback for the address space notifications (write events).
        The notification is called each time a write attribute operation succeeded on the server.

        There are no notifications for local writes using `PyS2OPC_Server.write_nodes`.

        This callback shall be re-implemented.

        Args:
            nodeId: The written NodeId as a string (see ["NodeId concept"](#nodeid-concept)).
            attrId: The written `AttributeId`
            dataValue: The new `pys2opc.DataValue`
            indexRange: An index range (string) selection on the value
            status: The `pys2opc.StatusCode` of this operation that will be returned to the client.
                    This differs from the status code of the value, which is contained in the DataValue.
        """
        raise NotImplementedError()

cdef class _C_BaseClientConnectionHandler:

    # Object variable #
    cdef SOPC_ClientConnection *_secureConnection
    cdef SOPC_ClientHelper_Subscription* _subscription
    """
    Counter used to link the index of Monitored items given to S2OPC and its nodeIds stored into `_list_nodeIds_handler`
    It keep the number of Monitored items created
    """

    def __cinit__(self):
        pass

    @staticmethod
    cdef _C_BaseClientConnectionHandler c_new_ClientConHandler(SOPC_ClientConnection *secureConnection):
        cdef _C_BaseClientConnectionHandler connection = _C_BaseClientConnectionHandler.__new__(_C_BaseClientConnectionHandler)
        connection._secureConnection = secureConnection
        return connection

    @staticmethod
    cdef void _callback_subscriptionNotification(const SOPC_ClientHelper_Subscription* subscription, SOPC_StatusCode status,
                                                    SOPC_EncodeableType* notificationType, uint32_t nbNotifElts, const void* notification,
                                                    uintptr_t* monitoredItemCtxArray) noexcept with gil:
        cdef const OpcUa_DataChangeNotification* notifs = NULL
        cdef uintptr_t index_ctx
        cdef SOPC_ClientConnection* secureConnection = SOPC_ClientHelper_GetSecureConnection(subscription)
        cdef uintptr_t id_sc = <uintptr_t> (<void*> secureConnection)
        cdef uintptr_t id_sub = <uintptr_t> (<void*> subscription)
        nodeIds_list: list[str] = BaseClientConnectionHandler._list_nodeIds_handler[id_sub]
        if SOPC_IsGoodStatus(status) and &OpcUa_DataChangeNotification_EncodeableType == notificationType:
            notifs = <const OpcUa_DataChangeNotification*> notification
            for i in range(nbNotifElts):
                index_ctx = monitoredItemCtxArray[i]
                if PyS2OPC_Client._dConnectHandler[id_sc] is not None:
                    connectHandler: BaseClientConnectionHandler = PyS2OPC_Client._dConnectHandler[id_sc]
                    connectHandler.on_datachanged(nodeId=nodeIds_list[index_ctx],
                                                  dataValue=_C_DataValue.from_sopc_datavalue(&(notifs.MonitoredItems[i].Value)))

class BaseClientConnectionHandler:
    """
    Base class giving the prototypes of the callbacks,
    implements Read/Write/Browse client methods,
    and implements the subscription-library connection wrappers.

    The class supports Python's "with" statements.
    In this case, the connection is automatically closed upon exit of the context.

    An exception `ClientConnectionFailure` might be raised when calling OPC UA services
    if the connection with the server has been lost which might be verified using `BaseClientConnectionHandler.connected`.
    """

    # Global class variable #
    _req_handler: dict[uintptr_t, AsyncReqHandler] = {}
    """ Store request with userContext as index """
    _list_nodeIds_handler: dict = {} # ID_subscription : list[str]
    """ Store Monitored items nodeIds """
    _monitoredItems_ID: dict = {} # ID_subscription : list[int]
    """ Store Monitored items ID """

    def __init__(self, c_cliConHandler: _C_BaseClientConnectionHandler):
        self._c_cliConHandler: _C_BaseClientConnectionHandler = c_cliConHandler
        self._connected: bool = True
        self._counter_MI_ctx: int = 0

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.disconnect()

    @property
    def connected(self):
        """
        Returns whether this connection is still active and usable.
        """
        return self._connected

    def _checkConnection(self):
        if not self.connected:
            raise ClientConnectionFailure('Connection lost: server may have closed the connection or disconnect() may have been called')

    # Callbacks
    def on_datachanged(self, nodeId: str, dataValue: DataValue):
        """
        This callback is called upon reception of a value change for a subscribed node.
        See `pys2opc.BaseClientConnectionHandler.add_nodes_to_subscription`.

        Args:
            nodeId: the string containing the NodeId of the changed node.
            dataValue: the new value (see `pys2opc.DataValue`).
        """
        raise NotImplementedError

    def disconnect(self) -> bool:
        """
        Disconnect the current connection.
        Return True if the connection was established and disconnection succeeded, False otherwise.
        """
        c_cliConHandler: _C_BaseClientConnectionHandler = self._c_cliConHandler
        if NULL != c_cliConHandler._secureConnection:
            self._connected = False
            status = SOPC_ClientHelper_Disconnect(&(c_cliConHandler._secureConnection))
            return status == SOPC_ReturnStatus.SOPC_STATUS_OK
        return False

    def read_nodes(self, nodeIds: list[str], attributes: list[AttributeId] = [], bWaitResponse: bool=True) -> (Response):
        """
        Requests to execute the OPC UA Read service on server and returns the Read response content.
        See `ReadResponse` for result content.

        Args:
            nodeIds: The list of nodeIds to read (see ["NodeId concept"](#nodeid-concept)).
            attributes: The list of attributes to be read in the previous given  nodes (1 attribute per node).
                        See `AttributeId` enum values.
                        Automatically set to `AttributeId.Value` if empty.
            bWaitResponse: True, wait the response and returns a `ReadResponse`.
                           Otherwise return an `AsyncResponse` to retrieve it later
                           (see `AsyncResponse.get_response`).
        """
        self._checkConnection()
        request: _Request = _Request.new_read_request(nodeIds, attributes=attributes)
        ReqHandler = _AsyncRequestHandler()
        BaseClientConnectionHandler._req_handler[request._requestContext] = ReqHandler
        c_cliConHandler: _C_BaseClientConnectionHandler = self._c_cliConHandler
        return ReqHandler._send_generic_request(False, c_cliConHandler._secureConnection, request, bWaitResponse)

    def write_nodes(self, nodeIds: list[str], datavalues: list[DataValue], attributes: list[AttributeId]=[], types=[], bWaitResponse: bool = True, autoTypeWithRead : bool =True) -> (Response):
        """
        Requests to execute the OPC UA Write service on server and returns the Write response content.
        See `WriteResponse` for result content.

        Args:
            nodeIds: The list of nodeIds to write (see ["NodeId concept"](#nodeid-concept)).
            datavalues: The list of DataValues to write (1 per node). see `DataValue`
            attributes: The list of attributes to be written for the given nodeIds (1 attribute per node).
                        See `AttributeId` enum.
                        Automatically set to `AttributeId.Value` if empty.
            bWaitResponse: True, wait the response and returns a `WriteResponse`.
                           Otherwise return an `AsyncResponse` to retrieve it later
                           (see `AsyncResponse.get_response`).

        Note 1: if the data values are not typed, an attempt to read the values first will be done.
                If some types are still missing afterward an assertion will be raised.
        Note 2: if data value server timestamp is set, it will be reset for the write operation as it is not writable in servers
        """
        self._checkConnection()
        c_cliConHandler: _C_BaseClientConnectionHandler = self._c_cliConHandler
        if autoTypeWithRead:
            types = _AsyncRequestHandler._helper_maybe_read_types(False, c_cliConHandler._secureConnection, BaseClientConnectionHandler._req_handler, nodeIds, datavalues, attributes, types)

        for dv in datavalues:
            # Unset timestamp server since it is not writable in servers (it will lead to write failure)
            dv.timestampServer = None
        request: _Request = _Request.new_write_request(nodeIds, datavalues, attributes=attributes, types=types)
        reqHandler = _AsyncRequestHandler()
        BaseClientConnectionHandler._req_handler[request._requestContext] = reqHandler
        return reqHandler._send_generic_request(False, c_cliConHandler._secureConnection, request, bWaitResponse)

    def browse_nodes(self, nodeIds: list[str], maxReferencesPerNode: int = 1000, bWaitResponse: bool=True) -> (Response):
        """
        Requests to execute the OPC UA Browse service on server and returns the Browse response content.
        See `BrowseResponse` for result content.

        Args:
            nodeIds: The list of nodeIds to browse (see ["NodeId concept"](#nodeid-concept)).
            maxReferencesPerNode: Indicates the maximum number of references to return for each starting node
                                  specified in the request (0 means no limitation).
            bWaitResponse: True, wait the response and returns a `BrowseResponse`.
                           Otherwise return an `AsyncResponse` to retrieve it later
                           (see `AsyncResponse.get_response`).
        """
        self._checkConnection()
        request: _Request = _Request.new_browse_request(nodeIds, maxReferencesPerNode)
        ReqHandler = _AsyncRequestHandler()
        BaseClientConnectionHandler._req_handler[request._requestContext] = ReqHandler
        c_cliConHandler: _C_BaseClientConnectionHandler = self._c_cliConHandler
        return ReqHandler._send_generic_request(False, c_cliConHandler._secureConnection, request, bWaitResponse)

    def call_method(self, objectNodeId: str, methodNodeId: str, inputArgList: list[Variant] = [], bWaitResponse: bool=True) -> (Response):
        """
        Requests to execute the OPC UA CallMethod service on server and returns the Call response content.
        See `CallResponse` for result content.

        Args:
            objectNodeId: The nodeId of the object node as string on which method call will be executed.
                           Shall not be NULL
            methodNodeId: The nodeId of the method node to execute as string
                           (it might be object's method node instance or method node of its ObjectType).
                           Shall not be NULL
            inputArgList:  The Variant list containing the input arguments for the method call.
            bWaitResponse: True, wait the response and returns a `BrowseResponse`.
                           Otherwise return an `AsyncResponse` to retrieve it later
                           (see `AsyncResponse.get_response`).

        Note : For more information on ["NodeId concept"](#nodeid-concept).
        """
        self._checkConnection()
        request: _Request = _Request.new_callmethod_request(objectNodeId, methodNodeId, inputArgList)
        ReqHandler = _AsyncRequestHandler()
        BaseClientConnectionHandler._req_handler[request._requestContext] = ReqHandler
        c_cliConHandler: _C_BaseClientConnectionHandler = self._c_cliConHandler
        return ReqHandler._send_generic_request(False, c_cliConHandler._secureConnection, request, bWaitResponse)

    def create_subscription(self,
                            requestedPublishingInterval: float = 500,
                            requestedLifetimeCount: int = 10,
                            requestedMaxKeepAliveCount: int = 3,
                            maxNotificationsPerPublish: int = 1000,
                            priority: int = 0,
                            nbPublishTokens: int = 3) -> None:
        """
        Creates a new subscription with customizable parameters. Only one subscription per connection is available for the moment.

        If default values of parameters are the one expected, there is no need to call this function as `BaseClientConnectionHandler.add_nodes_to_subscription` will automatically call it.

        Args:
            requestedPublishingInterval: Requested publishing interval in milliseconds.
            requestedLifetimeCount: Requested lifetime count for the subscription (number of publishing cycles).
                                    When the subscription publishing interval expired this count of times
                                    without Publish token available the subscription is deleted by the server.
                                    It shall be at least 3 times `requestedMaxKeepAliveCount`.
            requestedMaxKeepAliveCount: Requested max keep alive count for the subscription (number of publishing cycles).
                                        When the subscription publishing interval expired this count of times
                                        without notification to send, the server sends a keep alive Publish response.
                                        `requestedLifetimeCount` shall be at least 3 times this value.
            maxNotificationsPerPublish: Maximum number of notifications sent in a Publish response.
            priority: Priority of the subscription (0 means not special priority).
            nbPublishTokens: The number of publish tokens to be used by the client.
        """
        self._checkConnection()
        c_cliConHandler: _C_BaseClientConnectionHandler = self._c_cliConHandler
        # Check and convert(if necessary) python inputs
        if (requestedLifetimeCount > UINT32_MAX or requestedMaxKeepAliveCount > UINT32_MAX or maxNotificationsPerPublish > UINT32_MAX or nbPublishTokens > UINT32_MAX) :
            raise ValueError("requestedLifetimeCount or requestedMaxKeepAliveCount or maxNotificationsPerPublish or nbPublishTokens is too large (must be less than UINT32_MAX)")
        if priority > UINT8_MAX:
            raise ValueError("priority is too large (must be less than UINT8_MAX)")
        cdef SOPC_Boolean bPublishingEnabled = 1
        # configure subscription
        cdef OpcUa_CreateSubscriptionRequest* subscription_config = SOPC_CreateSubscriptionRequest_Create(
                                                                        requestedPublishingInterval,
                                                                        requestedLifetimeCount,
                                                                        requestedMaxKeepAliveCount,
                                                                        maxNotificationsPerPublish,
                                                                        bPublishingEnabled,
                                                                        priority)
        if subscription_config == NULL:
            raise MemoryError
        # create a subscription
        cdef SOPC_ClientHelper_Subscription* subscription = SOPC_ClientHelper_CreateSubscription(c_cliConHandler._secureConnection,
                                                            subscription_config,
                                                            c_cliConHandler._callback_subscriptionNotification, <uintptr_t> NULL)
        if subscription == NULL:
            raise MemoryError
        status = SOPC_ClientHelper_Subscription_SetAvailableTokens(subscription, nbPublishTokens)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Failed to create the subscription', status)
        c_cliConHandler._subscription = subscription

    def add_nodes_to_subscription(self, nodeIds: list[str]) -> list[bool]:
        """
        Add a list of NodeIds in the OPC UA format to the subscription (see ["NodeId concept"](#nodeid-concept)).
        If there is no subscription created, this function creates one beforehand using default parameters (see `BaseClientConnectionHandler.create_subscription`).
        Stores context for each node, which is reused by the callback function and to close the subscription correctly.

        The callback `pys2opc.BaseClientConnectionHandler.on_datachanged` will be called once for each new value of a subscribed node.
        In particular, the callback is at least called once for the initial value.

        The returned boolean list indicates if creation of the monitored item for the nodeId at same index succeeded
        """
        self._checkConnection()
        cdef SOPC_ReturnStatus status = SOPC_ReturnStatus.SOPC_STATUS_OK
        c_cliConHandler: _C_BaseClientConnectionHandler = self._c_cliConHandler

        # if no subscrition has been created, create it
        if c_cliConHandler._subscription == NULL:
            self.create_subscription()
        cdef uintptr_t id_sub = <uintptr_t> (<void*> c_cliConHandler._subscription)

        # Prepare the creation request for monitored items
        cdef OpcUa_CreateMonitoredItemsRequest* createMIreq = NULL
        # nodeIdsCtxArray keeps the index to retrieve the node ID (:str store into `BaseClientConnectionHandler._list_nodeIds_handler`)
        # in the callback func : `self._callback_subscriptionNotification`
        cdef uintptr_t* nodeIdsCtxArray = <uintptr_t*> calloc(len(nodeIds), sizeof(uintptr_t))
        cdef char** c_list_nodeIds = <char**> calloc(len(nodeIds), sizeof(char*))
        list_nodeIds: list[str] = BaseClientConnectionHandler._list_nodeIds_handler.get(id_sub, [])
        for i, nid in enumerate(nodeIds):
            nodeIdsCtxArray[i] = self._counter_MI_ctx
            list_nodeIds.append(nid)
            self._counter_MI_ctx = self._counter_MI_ctx + 1
            b_nodeIds: bytes = nid.encode() + b'\0'
            c_list_nodeIds[i] = <char*> calloc(len(b_nodeIds), sizeof(char))
            assert c_list_nodeIds[i] != NULL, 'Out of memory (in add_nodes_to_subscription function)'
            strncpy(c_list_nodeIds[i], b_nodeIds, len(b_nodeIds))
        BaseClientConnectionHandler._list_nodeIds_handler[id_sub] = list_nodeIds

        createMIreq = SOPC_CreateMonitoredItemsRequest_CreateDefaultFromStrings(0, <size_t> len(nodeIds), c_list_nodeIds,
                                                                                OpcUa_TimestampsToReturn.OpcUa_TimestampsToReturn_Both)
        # Free c_list_nodeIds
        for i in range(len(nodeIds)):
            free(c_list_nodeIds[i])
        free(c_list_nodeIds)
        c_list_nodeIds = NULL

        if NULL == createMIreq:
            status = SOPC_ReturnStatus.SOPC_STATUS_OUT_OF_MEMORY

        # Create the monitored items
        # Response is necessary to know if creation succeeded or not
        cdef OpcUa_CreateMonitoredItemsResponse* createMIresp = <OpcUa_CreateMonitoredItemsResponse*> calloc(1, sizeof(OpcUa_CreateMonitoredItemsResponse))
        OpcUa_CreateMonitoredItemsResponse_Initialize(createMIresp)
        cdef const uintptr_t* monitoredItemCtxArray = <const uintptr_t*> nodeIdsCtxArray
        status = SOPC_ClientHelper_Subscription_CreateMonitoredItems(c_cliConHandler._subscription, createMIreq,
                                                                        monitoredItemCtxArray, createMIresp)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Failed to create monitored items', status)
        free(nodeIdsCtxArray)
        nodeIdsCtxArray = NULL

        result : list[bool] = []
        list_MI: list[int] = BaseClientConnectionHandler._monitoredItems_ID.get(id_sub, [])
        for i in range(createMIresp.NoOfResults):
            list_MI.append(createMIresp.Results[i].MonitoredItemId)
            result.append(SOPC_IsGoodStatus(createMIresp.Results[i].StatusCode))
        BaseClientConnectionHandler._monitoredItems_ID[id_sub] = list_MI

        # Clear the create monitored items response
        OpcUa_CreateMonitoredItemsResponse_Clear(createMIresp)
        free(createMIresp)
        createMIresp = NULL

        return result

    def close_subscription(self) -> None:
        """
        Close the subscription (no more call to `BaseClientConnectionHandler.on_datachanged` will be done)
        """
        self._checkConnection()
        c_cliConHandler: _C_BaseClientConnectionHandler = self._c_cliConHandler
        if c_cliConHandler._subscription == NULL:
            return

        cdef uintptr_t id_sub = <uintptr_t> (<void*> c_cliConHandler._subscription)
        list_MI: list[int] = BaseClientConnectionHandler._monitoredItems_ID.get(id_sub, [])
        # Prepare the delete request for monitored items
        cdef OpcUa_DeleteMonitoredItemsRequest* deleteMIreq = NULL
        deleteMIreq = SOPC_DeleteMonitoredItemsRequest_Create(0, <size_t> (self._counter_MI_ctx), NULL)
        for i in range(self._counter_MI_ctx):
            status = SOPC_DeleteMonitoredItemsRequest_SetMonitoredItemId(deleteMIreq, i, list_MI[i])
            if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
                raise SOPC_Failure('Failed to set the monitored item identifier to delete', status)

        # Delete the monitored items
        cdef OpcUa_DeleteMonitoredItemsResponse deleteMIresp
        OpcUa_DeleteMonitoredItemsResponse_Initialize(&deleteMIresp)

        status = SOPC_ClientHelper_Subscription_DeleteMonitoredItems(c_cliConHandler._subscription, deleteMIreq, &deleteMIresp)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            OpcUa_DeleteMonitoredItemsRequest_Clear(deleteMIreq)
            free(deleteMIreq)
            raise SOPC_Failure('Failed to delete monitored items', status)
        # else: we should log something when it fails

        OpcUa_DeleteMonitoredItemsResponse_Clear(&deleteMIresp)

        # Close the subscription
        if NULL != c_cliConHandler._subscription:
            localStatus = SOPC_ClientHelper_DeleteSubscription(&(c_cliConHandler._subscription))
            free(c_cliConHandler._subscription)
            c_cliConHandler._subscription = NULL
            if (SOPC_ReturnStatus.SOPC_STATUS_OK != localStatus):
                raise SOPC_Failure('Failed to delete subscription', status)

class PyS2OPC_Client(PyS2OPC):
    """
    The Client side of the PyS2OPC library.
    Used to create clients only.
    """

    _dConnectHandler = {}
    """  Stores known active connections {id: ConnectionHandler()} """

    # @staticmethod : Python 3.9 does not support both and report error on staticmethod is not callable
    @contextmanager
    def initialize(logLevel:SOPC_Log_Level = SOPC_Log_Level.SOPC_LOG_LEVEL_DEBUG, logPath:str = 'logs/', logFileMaxBytes:int = 1048576, logMaxFileNumber:int = 50):
        """
        Toolkit initialization for Client.
        Call common initialization.

        This function supports the context management:
        >>> with PyS2OPC_Client.initialize():
        ...     # Do things here, namely configure then wait
        ...     pass

        When reaching out of the `with` statement, the Toolkit is automatically cleared.
        See `PyS2OPC_Client.clear`.

        Args:
            logLevel: log level (`SOPC_Log_Level`):
                ERROR: only errors are traced
                WARNING: only warnings and errors are traced
                INFO: information level, warnings and errors are traced
                DEBUG: all available information is traced
            logPath: the path for log files creation (might be relative to current working directory).
                     logPath is created if it does not exist.
            logFileMaxBytes: The maximum size of the log files, before changing the log index.
            logMaxFileNumber: The maximum number of log indexes before cycling logs and reusing the first log.
        """
        if logFileMaxBytes > UINT32_MAX:
            raise ValueError("logFileMaxBytes is too large (must be less than UINT32_MAX)")
        if logMaxFileNumber > UINT16_MAX:
            raise ValueError("logMaxFileNumber is too large (must be less than UINT16_MAX)")
        PyS2OPC.initialize(logLevel, logPath, logFileMaxBytes, logMaxFileNumber)
        status = SOPC_ClientConfigHelper_Initialize()
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Library client initialization failed', status)
        PyS2OPC._initialized_cli = True

        try:
            yield
        finally:
            PyS2OPC_Client.clear()

    @staticmethod
    def clear() -> None:
        """
        Clear the client configuration and clear/stop the toolkit if applicable.
        """
        SOPC_ClientConfigHelper_Clear()
        PyS2OPC._initialized_cli = False
        if PyS2OPC._initialized_srv == False:
            PyS2OPC._clear()

    @staticmethod
    def load_client_configuration_from_file(xml_client_config_path: str) -> dict[str, ConnectionConfiguration]:
        """
        Configure client from XML configuration file.
        This function must be called after `PyS2OPC_Client.initialize`.

        Args:
            xml_client_config_path: Path to client configuration XML file ([s2opc_clientserver_config.xsd](https://gitlab.com/systerel/S2OPC/-/blob/master/schemas/s2opc_clientserver_config.xsd?ref_type=heads) schema)
        """
        b_xml_client_config_path = xml_client_config_path.encode()
        cdef size_t nbConfigs = 0
        cdef SOPC_SecureConnection_Config** scConfigArray = NULL
        status = SOPC_ClientConfigHelper_ConfigureFromXML(b_xml_client_config_path, NULL, &nbConfigs, &scConfigArray)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Client configuration (from XML {}) failed'.format(xml_client_config_path), status)
        status = SOPC_ClientConfigHelper_SetServiceAsyncResponse(_callback_ClientServiceAsyncResp)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Failed to configure client service async callback', status)
        result : dict = {}
        configs = _C_Configurations.c_new_conf(nbConfigs, scConfigArray)
        for i in range(configs._nbConfigs):
            config = _C_Configurations.getConfig(configs, i)
            configId = SOPC_ClientConfigHelper_GetUserIdFromConfig(config).decode()
            if configId in result:
                raise SOPC_Failure('Configuration ID {} is not unique in XML configuration {}'
                                   .format(configId, xml_client_config_path))
            result[configId] = ConnectionConfiguration(configs, i)

        return result

    @staticmethod
    def connect(connCfg: ConnectionConfiguration, ConnectionHandlerClass=None, sc_lifetime: int = 3600000) -> BaseClientConnectionHandler:
        """
        This function must be called after `PyS2OPC_Client.load_client_configuration_from_file` and use one of the parsed configurations.
        Use a configuration instance and provided parameters to connect to the server.

        Args:
            connCfg: The configuration to establish the connection to the server
            ConnectionHandlerClass: (optional) class that inherits from `BaseClientConnectionHandler`,
                                    and overrides at least overrides the `BaseClientConnectionHandler.on_datachanged` callback for subscription.
            sc_lifetime: Requested lifetime in millisecond for the secure channel.

        It can be optionally used in a `with` statement, which automatically disconnects the connection. See `BaseClientConnectionHandler`
        """
        if sc_lifetime > UINT32_MAX:
            raise ValueError("sc_lifetime is too large (must be less than UINT32_MAX)")
        cdef SOPC_SecureConnection_Config * sConnCfg = _C_Configurations.getConfig(connCfg._configs, connCfg._cfgIdx)
        assert NULL != sConnCfg, "Unexpected failure to load configuration"
        status = SOPC_ClientConfigHelper_SetClientKeyPasswordCallback(_callback_get_client_key_password)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Failed to configure the client key password callback', status)
        status = SOPC_ClientConfigHelper_SetUserNamePasswordCallback(_callback_get_client_username_password)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Failed to configure the client username password callback', status)
        status = SOPC_ClientConfigHelper_SetUserKeyPasswordCallback(_callback_get_client_user_cert_key_password)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Failed to configure the client user X509 key password callback', status)
        cdef SOPC_ClientConnection* secureConnection = NULL
        status = SOPC_SecureConnectionConfig_SetReqLifetime(sConnCfg, sc_lifetime)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK and status != SOPC_ReturnStatus.SOPC_STATUS_INVALID_STATE:
            raise SOPC_Failure('Failed to configure the SC lifetime', status)
        status = SOPC_ClientHelper_Connect(sConnCfg, _callback_client_connection_event, &secureConnection)
        if status != SOPC_ReturnStatus.SOPC_STATUS_OK:
            raise SOPC_Failure('Could not connect to the server with the given configuration', status)

        cdef uintptr_t id_sc = <uintptr_t> (<void*> secureConnection)
        c_connection = _C_BaseClientConnectionHandler.c_new_ClientConHandler(secureConnection)
        if ConnectionHandlerClass is not None:
            assert issubclass(ConnectionHandlerClass, BaseClientConnectionHandler)
            connection = ConnectionHandlerClass(c_connection)
        else:
            connection = BaseClientConnectionHandler(c_connection)
        # Set connection handler
        PyS2OPC_Client._dConnectHandler[id_sc] = connection
        return connection

    @staticmethod
    def get_client_key_password() -> str:
        """
        Default method that is called during configuration phase if an encrypted private key is used,
        it shall return the password to decrypt the client private key.

        It uses [get_pass](https://docs.python.org/3/library/getpass.html#module-getpass)
        library and homonymous function to prompt the user for password.

        It is possible to overwrite this function by assiging a new implementation to `PyS2OPC_Client.get_client_key_password`
        to obtain a different behavior.
        """
        return PyS2OPC._get_password('Client private key password: ')

    @staticmethod
    def get_username_password() -> tuple[str, str]:
        """
        Default method that is called during configuration phase if the UserPolicy requires a user,
        it shall return the username and password associated as a 2-tuples of string.

        It uses [get_pass](https://docs.python.org/3/library/getpass.html#module-getpass)
        library and homonymous function to prompt the user for username and password.

        It is possible to overwrite this function by assiging a new implementation to `PyS2OPC_Client.get_username_password`
        to obtain a different behavior.
        """
        username = PyS2OPC._get_password('UserName of user: ')
        pwd = PyS2OPC._get_password('Password for user: ')
        return username, pwd

    @staticmethod
    def get_user_X509_key_password(userCertThumb : str) -> str:
        """
        Default method that is called during configuration phase if the UserPolicy requires a user X509 certificate,
        it shall return the password to be used to decrypt the user X509 private key.

        It uses [get_pass](https://docs.python.org/3/library/getpass.html#module-getpass)
        library and homonymous function to prompt the user for password.

        It is possible to overwrite this function by assiging a new implementation to `PyS2OPC_Client.get_user_X509_key_password`
        to obtain a different behavior.

        Args:
            userCertThumb: the user X509 certificate thumbprint for which the private key password is requested for decryption
        """
        return PyS2OPC._get_password('Password for user X509 certificate thumbprint {}: '.format(userCertThumb))
