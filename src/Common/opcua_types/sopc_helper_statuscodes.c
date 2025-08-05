/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "sopc_helper_statuscodes.h"
#include "opcua_statuscodes.h"

#include <stdio.h>

#include "sopc_assert.h"
#include "sopc_helper_string.h"
#include "sopc_mem_alloc.h"

static const char* SOPC_Internal_StatusCodeToString(SOPC_StatusCode status, bool* found)
{
    SOPC_ASSERT(found != NULL);
    *found = true;
    switch (status)
    {
    case OpcUa_BadAggregateConfigurationRejected:
        return "OpcUa_BadAggregateConfigurationRejected";
    case OpcUa_BadAggregateInvalidInputs:
        return "OpcUa_BadAggregateInvalidInputs";
    case OpcUa_BadAggregateListMismatch:
        return "OpcUa_BadAggregateListMismatch";
    case OpcUa_BadAggregateNotSupported:
        return "OpcUa_BadAggregateNotSupported";
    case OpcUa_BadAlreadyExists:
        return "OpcUa_BadAlreadyExists";
    case OpcUa_BadApplicationSignatureInvalid:
        return "OpcUa_BadApplicationSignatureInvalid";
    case OpcUa_BadArgumentsMissing:
        return "OpcUa_BadArgumentsMissing";
    case OpcUa_BadAttributeIdInvalid:
        return "OpcUa_BadAttributeIdInvalid";
    case OpcUa_BadBoundNotFound:
        return "OpcUa_BadBoundNotFound";
    case OpcUa_BadBoundNotSupported:
        return "OpcUa_BadBoundNotSupported";
    case OpcUa_BadBrowseDirectionInvalid:
        return "OpcUa_BadBrowseDirectionInvalid";
    case OpcUa_BadBrowseNameDuplicated:
        return "OpcUa_BadBrowseNameDuplicated";
    case OpcUa_BadBrowseNameInvalid:
        return "OpcUa_BadBrowseNameInvalid";
    case OpcUa_BadCertificateChainIncomplete:
        return "OpcUa_BadCertificateChainIncomplete";
    case OpcUa_BadCertificateHostNameInvalid:
        return "OpcUa_BadCertificateHostNameInvalid";
    case OpcUa_BadCertificateInvalid:
        return "OpcUa_BadCertificateInvalid";
    case OpcUa_BadCertificateIssuerRevocationUnknown:
        return "OpcUa_BadCertificateIssuerRevocationUnknown";
    case OpcUa_BadCertificateIssuerRevoked:
        return "OpcUa_BadCertificateIssuerRevoked";
    case OpcUa_BadCertificateIssuerTimeInvalid:
        return "OpcUa_BadCertificateIssuerTimeInvalid";
    case OpcUa_BadCertificateIssuerUseNotAllowed:
        return "OpcUa_BadCertificateIssuerUseNotAllowed";
    case OpcUa_BadCertificatePolicyCheckFailed:
        return "OpcUa_BadCertificatePolicyCheckFailed";
    case OpcUa_BadCertificateRevocationUnknown:
        return "OpcUa_BadCertificateRevocationUnknown";
    case OpcUa_BadCertificateRevoked:
        return "OpcUa_BadCertificateRevoked";
    case OpcUa_BadCertificateTimeInvalid:
        return "OpcUa_BadCertificateTimeInvalid";
    case OpcUa_BadCertificateUntrusted:
        return "OpcUa_BadCertificateUntrusted";
    case OpcUa_BadCertificateUriInvalid:
        return "OpcUa_BadCertificateUriInvalid";
    case OpcUa_BadCertificateUseNotAllowed:
        return "OpcUa_BadCertificateUseNotAllowed";
    case OpcUa_BadCommunicationError:
        return "OpcUa_BadCommunicationError";
    case OpcUa_BadConditionAlreadyDisabled:
        return "OpcUa_BadConditionAlreadyDisabled";
    case OpcUa_BadConditionAlreadyEnabled:
        return "OpcUa_BadConditionAlreadyEnabled";
    case OpcUa_BadConditionAlreadyShelved:
        return "OpcUa_BadConditionAlreadyShelved";
    case OpcUa_BadConditionBranchAlreadyAcked:
        return "OpcUa_BadConditionBranchAlreadyAcked";
    case OpcUa_BadConditionBranchAlreadyConfirmed:
        return "OpcUa_BadConditionBranchAlreadyConfirmed";
    case OpcUa_BadConditionDisabled:
        return "OpcUa_BadConditionDisabled";
    case OpcUa_BadConditionNotShelved:
        return "OpcUa_BadConditionNotShelved";
    case OpcUa_BadConfigurationError:
        return "OpcUa_BadConfigurationError";
    case OpcUa_BadConnectionClosed:
        return "OpcUa_BadConnectionClosed";
    case OpcUa_BadConnectionRejected:
        return "OpcUa_BadConnectionRejected";
    case OpcUa_BadContentFilterInvalid:
        return "OpcUa_BadContentFilterInvalid";
    case OpcUa_BadContinuationPointInvalid:
        return "OpcUa_BadContinuationPointInvalid";
    case OpcUa_BadDataEncodingInvalid:
        return "OpcUa_BadDataEncodingInvalid";
    case OpcUa_BadDataEncodingUnsupported:
        return "OpcUa_BadDataEncodingUnsupported";
    case OpcUa_BadDataLost:
        return "OpcUa_BadDataLost";
    case OpcUa_BadDataSetIdInvalid:
        return "OpcUa_BadDataSetIdInvalid";
    case OpcUa_BadDataTypeIdUnknown:
        return "OpcUa_BadDataTypeIdUnknown";
    case OpcUa_BadDataUnavailable:
        return "OpcUa_BadDataUnavailable";
    case OpcUa_BadDeadbandFilterInvalid:
        return "OpcUa_BadDeadbandFilterInvalid";
    case OpcUa_BadDecodingError:
        return "OpcUa_BadDecodingError";
    case OpcUa_BadDependentValueChanged:
        return "OpcUa_BadDependentValueChanged";
    case OpcUa_BadDeviceFailure:
        return "OpcUa_BadDeviceFailure";
    case OpcUa_BadDialogNotActive:
        return "OpcUa_BadDialogNotActive";
    case OpcUa_BadDialogResponseInvalid:
        return "OpcUa_BadDialogResponseInvalid";
    case OpcUa_BadDisconnect:
        return "OpcUa_BadDisconnect";
    case OpcUa_BadDiscoveryUrlMissing:
        return "OpcUa_BadDiscoveryUrlMissing";
    case OpcUa_BadDominantValueChanged:
        return "OpcUa_BadDominantValueChanged";
    case OpcUa_BadDuplicateReferenceNotAllowed:
        return "OpcUa_BadDuplicateReferenceNotAllowed";
    case OpcUa_BadEdited_OutOfRange:
        return "OpcUa_BadEdited_OutOfRange";
    case OpcUa_BadEdited_OutOfRange_DominantValueChanged:
        return "OpcUa_BadEdited_OutOfRange_DominantValueChanged";
    case OpcUa_BadEdited_OutOfRange_DominantValueChanged_DependentValueChanged:
        return "OpcUa_BadEdited_OutOfRange_DominantValueChanged_DependentValueChanged";
    case OpcUa_BadEncodingError:
        return "OpcUa_BadEncodingError";
    case OpcUa_BadEncodingLimitsExceeded:
        return "OpcUa_BadEncodingLimitsExceeded";
    case OpcUa_BadEndOfStream:
        return "OpcUa_BadEndOfStream";
    case OpcUa_BadEntryExists:
        return "OpcUa_BadEntryExists";
    case OpcUa_BadEventFilterInvalid:
        return "OpcUa_BadEventFilterInvalid";
    case OpcUa_BadEventIdUnknown:
        return "OpcUa_BadEventIdUnknown";
    case OpcUa_BadEventNotAcknowledgeable:
        return "OpcUa_BadEventNotAcknowledgeable";
    case OpcUa_BadExpectedStreamToBlock:
        return "OpcUa_BadExpectedStreamToBlock";
    case OpcUa_BadFilterElementInvalid:
        return "OpcUa_BadFilterElementInvalid";
    case OpcUa_BadFilterLiteralInvalid:
        return "OpcUa_BadFilterLiteralInvalid";
    case OpcUa_BadFilterNotAllowed:
        return "OpcUa_BadFilterNotAllowed";
    case OpcUa_BadFilterOperandCountMismatch:
        return "OpcUa_BadFilterOperandCountMismatch";
    case OpcUa_BadFilterOperandInvalid:
        return "OpcUa_BadFilterOperandInvalid";
    case OpcUa_BadFilterOperatorInvalid:
        return "OpcUa_BadFilterOperatorInvalid";
    case OpcUa_BadFilterOperatorUnsupported:
        return "OpcUa_BadFilterOperatorUnsupported";
    case OpcUa_BadHistoryOperationInvalid:
        return "OpcUa_BadHistoryOperationInvalid";
    case OpcUa_BadHistoryOperationUnsupported:
        return "OpcUa_BadHistoryOperationUnsupported";
    case OpcUa_BadIdentityChangeNotSupported:
        return "OpcUa_BadIdentityChangeNotSupported";
    case OpcUa_BadIdentityTokenInvalid:
        return "OpcUa_BadIdentityTokenInvalid";
    case OpcUa_BadIdentityTokenRejected:
        return "OpcUa_BadIdentityTokenRejected";
    case OpcUa_BadIndexRangeDataMismatch:
        return "OpcUa_BadIndexRangeDataMismatch";
    case OpcUa_BadIndexRangeInvalid:
        return "OpcUa_BadIndexRangeInvalid";
    case OpcUa_BadIndexRangeNoData:
        return "OpcUa_BadIndexRangeNoData";
    case OpcUa_BadInitialValue_OutOfRange:
        return "OpcUa_BadInitialValue_OutOfRange";
    case OpcUa_BadInsufficientClientProfile:
        return "OpcUa_BadInsufficientClientProfile";
    case OpcUa_BadInternalError:
        return "OpcUa_BadInternalError";
    case OpcUa_BadInvalidArgument:
        return "OpcUa_BadInvalidArgument";
    case OpcUa_BadInvalidSelfReference:
        return "OpcUa_BadInvalidSelfReference";
    case OpcUa_BadInvalidState:
        return "OpcUa_BadInvalidState";
    case OpcUa_BadInvalidTimestamp:
        return "OpcUa_BadInvalidTimestamp";
    case OpcUa_BadInvalidTimestampArgument:
        return "OpcUa_BadInvalidTimestampArgument";
    case OpcUa_BadLicenseExpired:
        return "OpcUa_BadLicenseExpired";
    case OpcUa_BadLicenseLimitsExceeded:
        return "OpcUa_BadLicenseLimitsExceeded";
    case OpcUa_BadLicenseNotAvailable:
        return "OpcUa_BadLicenseNotAvailable";
    case OpcUa_BadLocaleNotSupported:
        return "OpcUa_BadLocaleNotSupported";
    case OpcUa_BadLocked:
        return "OpcUa_BadLocked";
    case OpcUa_BadMaxAgeInvalid:
        return "OpcUa_BadMaxAgeInvalid";
    case OpcUa_BadMaxConnectionsReached:
        return "OpcUa_BadMaxConnectionsReached";
    case OpcUa_BadMessageNotAvailable:
        return "OpcUa_BadMessageNotAvailable";
    case OpcUa_BadMethodInvalid:
        return "OpcUa_BadMethodInvalid";
    case OpcUa_BadMonitoredItemFilterInvalid:
        return "OpcUa_BadMonitoredItemFilterInvalid";
    case OpcUa_BadMonitoredItemFilterUnsupported:
        return "OpcUa_BadMonitoredItemFilterUnsupported";
    case OpcUa_BadMonitoredItemIdInvalid:
        return "OpcUa_BadMonitoredItemIdInvalid";
    case OpcUa_BadMonitoringModeInvalid:
        return "OpcUa_BadMonitoringModeInvalid";
    case OpcUa_BadNoCommunication:
        return "OpcUa_BadNoCommunication";
    case OpcUa_BadNoContinuationPoints:
        return "OpcUa_BadNoContinuationPoints";
    case OpcUa_BadNoData:
        return "OpcUa_BadNoData";
    case OpcUa_BadNoDataAvailable:
        return "OpcUa_BadNoDataAvailable";
    case OpcUa_BadNoDeleteRights:
        return "OpcUa_BadNoDeleteRights";
    case OpcUa_BadNoEntryExists:
        return "OpcUa_BadNoEntryExists";
    case OpcUa_BadNoMatch:
        return "OpcUa_BadNoMatch";
    case OpcUa_BadNoSubscription:
        return "OpcUa_BadNoSubscription";
    case OpcUa_BadNoValidCertificates:
        return "OpcUa_BadNoValidCertificates";
    case OpcUa_BadNoValue:
        return "OpcUa_BadNoValue";
    case OpcUa_BadNodeAttributesInvalid:
        return "OpcUa_BadNodeAttributesInvalid";
    case OpcUa_BadNodeClassInvalid:
        return "OpcUa_BadNodeClassInvalid";
    case OpcUa_BadNodeIdExists:
        return "OpcUa_BadNodeIdExists";
    case OpcUa_BadNodeIdInvalid:
        return "OpcUa_BadNodeIdInvalid";
    case OpcUa_BadNodeIdRejected:
        return "OpcUa_BadNodeIdRejected";
    case OpcUa_BadNodeIdUnknown:
        return "OpcUa_BadNodeIdUnknown";
    case OpcUa_BadNodeNotInView:
        return "OpcUa_BadNodeNotInView";
    case OpcUa_BadNonceInvalid:
        return "OpcUa_BadNonceInvalid";
    case OpcUa_BadNotConnected:
        return "OpcUa_BadNotConnected";
    case OpcUa_BadNotExecutable:
        return "OpcUa_BadNotExecutable";
    case OpcUa_BadNotFound:
        return "OpcUa_BadNotFound";
    case OpcUa_BadNotImplemented:
        return "OpcUa_BadNotImplemented";
    case OpcUa_BadNotReadable:
        return "OpcUa_BadNotReadable";
    case OpcUa_BadNotSupported:
        return "OpcUa_BadNotSupported";
    case OpcUa_BadNotTypeDefinition:
        return "OpcUa_BadNotTypeDefinition";
    case OpcUa_BadNotWritable:
        return "OpcUa_BadNotWritable";
    case OpcUa_BadNothingToDo:
        return "OpcUa_BadNothingToDo";
    case OpcUa_BadNumericOverflow:
        return "OpcUa_BadNumericOverflow";
    case OpcUa_BadObjectDeleted:
        return "OpcUa_BadObjectDeleted";
    case OpcUa_BadOperationAbandoned:
        return "OpcUa_BadOperationAbandoned";
    case OpcUa_BadOutOfMemory:
        return "OpcUa_BadOutOfMemory";
    case OpcUa_BadOutOfRange:
        return "OpcUa_BadOutOfRange";
    case OpcUa_BadOutOfRange_DominantValueChanged:
        return "OpcUa_BadOutOfRange_DominantValueChanged";
    case OpcUa_BadOutOfRange_DominantValueChanged_DependentValueChanged:
        return "OpcUa_BadOutOfRange_DominantValueChanged_DependentValueChanged";
    case OpcUa_BadOutOfService:
        return "OpcUa_BadOutOfService";
    case OpcUa_BadParentNodeIdInvalid:
        return "OpcUa_BadParentNodeIdInvalid";
    case OpcUa_BadProtocolVersionUnsupported:
        return "OpcUa_BadProtocolVersionUnsupported";
    case OpcUa_BadQueryTooComplex:
        return "OpcUa_BadQueryTooComplex";
    case OpcUa_BadReferenceLocalOnly:
        return "OpcUa_BadReferenceLocalOnly";
    case OpcUa_BadReferenceNotAllowed:
        return "OpcUa_BadReferenceNotAllowed";
    case OpcUa_BadReferenceTypeIdInvalid:
        return "OpcUa_BadReferenceTypeIdInvalid";
    case OpcUa_BadRefreshInProgress:
        return "OpcUa_BadRefreshInProgress";
    case OpcUa_BadRequestCancelledByClient:
        return "OpcUa_BadRequestCancelledByClient";
    case OpcUa_BadRequestCancelledByRequest:
        return "OpcUa_BadRequestCancelledByRequest";
    case OpcUa_BadRequestHeaderInvalid:
        return "OpcUa_BadRequestHeaderInvalid";
    case OpcUa_BadRequestInterrupted:
        return "OpcUa_BadRequestInterrupted";
    case OpcUa_BadRequestNotAllowed:
        return "OpcUa_BadRequestNotAllowed";
    case OpcUa_BadRequestNotComplete:
        return "OpcUa_BadRequestNotComplete";
    case OpcUa_BadRequestTimeout:
        return "OpcUa_BadRequestTimeout";
    case OpcUa_BadRequestTooLarge:
        return "OpcUa_BadRequestTooLarge";
    case OpcUa_BadRequestTypeInvalid:
        return "OpcUa_BadRequestTypeInvalid";
    case OpcUa_BadRequiresLock:
        return "OpcUa_BadRequiresLock";
    case OpcUa_BadResourceUnavailable:
        return "OpcUa_BadResourceUnavailable";
    case OpcUa_BadResponseTooLarge:
        return "OpcUa_BadResponseTooLarge";
    case OpcUa_BadSecureChannelClosed:
        return "OpcUa_BadSecureChannelClosed";
    case OpcUa_BadSecureChannelIdInvalid:
        return "OpcUa_BadSecureChannelIdInvalid";
    case OpcUa_BadSecureChannelTokenUnknown:
        return "OpcUa_BadSecureChannelTokenUnknown";
    case OpcUa_BadSecurityChecksFailed:
        return "OpcUa_BadSecurityChecksFailed";
    case OpcUa_BadSecurityModeInsufficient:
        return "OpcUa_BadSecurityModeInsufficient";
    case OpcUa_BadSecurityModeRejected:
        return "OpcUa_BadSecurityModeRejected";
    case OpcUa_BadSecurityPolicyRejected:
        return "OpcUa_BadSecurityPolicyRejected";
    case OpcUa_BadSemaphoreFileMissing:
        return "OpcUa_BadSemaphoreFileMissing";
    case OpcUa_BadSensorFailure:
        return "OpcUa_BadSensorFailure";
    case OpcUa_BadSequenceNumberInvalid:
        return "OpcUa_BadSequenceNumberInvalid";
    case OpcUa_BadSequenceNumberUnknown:
        return "OpcUa_BadSequenceNumberUnknown";
    case OpcUa_BadServerHalted:
        return "OpcUa_BadServerHalted";
    case OpcUa_BadServerIndexInvalid:
        return "OpcUa_BadServerIndexInvalid";
    case OpcUa_BadServerNameMissing:
        return "OpcUa_BadServerNameMissing";
    case OpcUa_BadServerNotConnected:
        return "OpcUa_BadServerNotConnected";
    case OpcUa_BadServerTooBusy:
        return "OpcUa_BadServerTooBusy";
    case OpcUa_BadServerUriInvalid:
        return "OpcUa_BadServerUriInvalid";
    case OpcUa_BadServiceUnsupported:
        return "OpcUa_BadServiceUnsupported";
    case OpcUa_BadSessionClosed:
        return "OpcUa_BadSessionClosed";
    case OpcUa_BadSessionIdInvalid:
        return "OpcUa_BadSessionIdInvalid";
    case OpcUa_BadSessionNotActivated:
        return "OpcUa_BadSessionNotActivated";
    case OpcUa_BadShelvingTimeOutOfRange:
        return "OpcUa_BadShelvingTimeOutOfRange";
    case OpcUa_BadShutdown:
        return "OpcUa_BadShutdown";
    case OpcUa_BadSourceNodeIdInvalid:
        return "OpcUa_BadSourceNodeIdInvalid";
    case OpcUa_BadStateNotActive:
        return "OpcUa_BadStateNotActive";
    case OpcUa_BadStructureMissing:
        return "OpcUa_BadStructureMissing";
    case OpcUa_BadSubscriptionIdInvalid:
        return "OpcUa_BadSubscriptionIdInvalid";
    case OpcUa_BadSyntaxError:
        return "OpcUa_BadSyntaxError";
    case OpcUa_BadTargetNodeIdInvalid:
        return "OpcUa_BadTargetNodeIdInvalid";
    case OpcUa_BadTcpEndpointUrlInvalid:
        return "OpcUa_BadTcpEndpointUrlInvalid";
    case OpcUa_BadTcpInternalError:
        return "OpcUa_BadTcpInternalError";
    case OpcUa_BadTcpMessageTooLarge:
        return "OpcUa_BadTcpMessageTooLarge";
    case OpcUa_BadTcpMessageTypeInvalid:
        return "OpcUa_BadTcpMessageTypeInvalid";
    case OpcUa_BadTcpNotEnoughResources:
        return "OpcUa_BadTcpNotEnoughResources";
    case OpcUa_BadTcpSecureChannelUnknown:
        return "OpcUa_BadTcpSecureChannelUnknown";
    case OpcUa_BadTcpServerTooBusy:
        return "OpcUa_BadTcpServerTooBusy";
    case OpcUa_BadTicketInvalid:
        return "OpcUa_BadTicketInvalid";
    case OpcUa_BadTicketRequired:
        return "OpcUa_BadTicketRequired";
    case OpcUa_BadTimeout:
        return "OpcUa_BadTimeout";
    case OpcUa_BadTimestampNotSupported:
        return "OpcUa_BadTimestampNotSupported";
    case OpcUa_BadTimestampsToReturnInvalid:
        return "OpcUa_BadTimestampsToReturnInvalid";
    case OpcUa_BadTooManyArguments:
        return "OpcUa_BadTooManyArguments";
    case OpcUa_BadTooManyMatches:
        return "OpcUa_BadTooManyMatches";
    case OpcUa_BadTooManyMonitoredItems:
        return "OpcUa_BadTooManyMonitoredItems";
    case OpcUa_BadTooManyOperations:
        return "OpcUa_BadTooManyOperations";
    case OpcUa_BadTooManyPublishRequests:
        return "OpcUa_BadTooManyPublishRequests";
    case OpcUa_BadTooManySessions:
        return "OpcUa_BadTooManySessions";
    case OpcUa_BadTooManySubscriptions:
        return "OpcUa_BadTooManySubscriptions";
    case OpcUa_BadTransactionFailed:
        return "OpcUa_BadTransactionFailed";
    case OpcUa_BadTransactionPending:
        return "OpcUa_BadTransactionPending";
    case OpcUa_BadTypeDefinitionInvalid:
        return "OpcUa_BadTypeDefinitionInvalid";
    case OpcUa_BadTypeMismatch:
        return "OpcUa_BadTypeMismatch";
    case OpcUa_BadUnexpectedError:
        return "OpcUa_BadUnexpectedError";
    case OpcUa_BadUnknownResponse:
        return "OpcUa_BadUnknownResponse";
    case OpcUa_BadUserAccessDenied:
        return "OpcUa_BadUserAccessDenied";
    case OpcUa_BadUserSignatureInvalid:
        return "OpcUa_BadUserSignatureInvalid";
    case OpcUa_BadViewIdUnknown:
        return "OpcUa_BadViewIdUnknown";
    case OpcUa_BadViewParameterMismatch:
        return "OpcUa_BadViewParameterMismatch";
    case OpcUa_BadViewTimestampInvalid:
        return "OpcUa_BadViewTimestampInvalid";
    case OpcUa_BadViewVersionInvalid:
        return "OpcUa_BadViewVersionInvalid";
    case OpcUa_BadWaitingForInitialData:
        return "OpcUa_BadWaitingForInitialData";
    case OpcUa_BadWaitingForResponse:
        return "OpcUa_BadWaitingForResponse";
    case OpcUa_BadWouldBlock:
        return "OpcUa_BadWouldBlock";
    case OpcUa_BadWriteNotSupported:
        return "OpcUa_BadWriteNotSupported";
    case OpcUa_GoodCallAgain:
        return "OpcUa_GoodCallAgain";
    case OpcUa_GoodCascade:
        return "OpcUa_GoodCascade";
    case OpcUa_GoodCascadeInitializationAcknowledged:
        return "OpcUa_GoodCascadeInitializationAcknowledged";
    case OpcUa_GoodCascadeInitializationRequest:
        return "OpcUa_GoodCascadeInitializationRequest";
    case OpcUa_GoodCascadeNotInvited:
        return "OpcUa_GoodCascadeNotInvited";
    case OpcUa_GoodCascadeNotSelected:
        return "OpcUa_GoodCascadeNotSelected";
    case OpcUa_GoodClamped:
        return "OpcUa_GoodClamped";
    case OpcUa_GoodCommunicationEvent:
        return "OpcUa_GoodCommunicationEvent";
    case OpcUa_GoodCompletesAsynchronously:
        return "OpcUa_GoodCompletesAsynchronously";
    case OpcUa_GoodDataIgnored:
        return "OpcUa_GoodDataIgnored";
    case OpcUa_GoodDependentValueChanged:
        return "OpcUa_GoodDependentValueChanged";
    case OpcUa_GoodEdited:
        return "OpcUa_GoodEdited";
    case OpcUa_GoodEdited_DependentValueChanged:
        return "OpcUa_GoodEdited_DependentValueChanged";
    case OpcUa_GoodEdited_DominantValueChanged:
        return "OpcUa_GoodEdited_DominantValueChanged";
    case OpcUa_GoodEdited_DominantValueChanged_DependentValueChanged:
        return "OpcUa_GoodEdited_DominantValueChanged_DependentValueChanged";
    case OpcUa_GoodEntryInserted:
        return "OpcUa_GoodEntryInserted";
    case OpcUa_GoodEntryReplaced:
        return "OpcUa_GoodEntryReplaced";
    case OpcUa_GoodFaultStateActive:
        return "OpcUa_GoodFaultStateActive";
    case OpcUa_GoodInitiateFaultState:
        return "OpcUa_GoodInitiateFaultState";
    case OpcUa_GoodLocalOverride:
        return "OpcUa_GoodLocalOverride";
    case OpcUa_GoodMoreData:
        return "OpcUa_GoodMoreData";
    case OpcUa_GoodNoData:
        return "OpcUa_GoodNoData";
    case OpcUa_GoodNonCriticalTimeout:
        return "OpcUa_GoodNonCriticalTimeout";
    case OpcUa_GoodOverload:
        return "OpcUa_GoodOverload";
    case OpcUa_GoodPasswordChangeRequired:
        return "OpcUa_GoodPasswordChangeRequired";
    case OpcUa_GoodPostActionFailed:
        return "OpcUa_GoodPostActionFailed";
    case OpcUa_GoodResultsMayBeIncomplete:
        return "OpcUa_GoodResultsMayBeIncomplete";
    case OpcUa_GoodRetransmissionQueueNotSupported:
        return "OpcUa_GoodRetransmissionQueueNotSupported";
    case OpcUa_GoodShutdownEvent:
        return "OpcUa_GoodShutdownEvent";
    case OpcUa_GoodSubNormal:
        return "OpcUa_GoodSubNormal";
    case OpcUa_GoodSubscriptionTransferred:
        return "OpcUa_GoodSubscriptionTransferred";
    case OpcUa_UncertainConfigurationError:
        return "OpcUa_UncertainConfigurationError";
    case OpcUa_UncertainDataSubNormal:
        return "OpcUa_UncertainDataSubNormal";
    case OpcUa_UncertainDependentValueChanged:
        return "OpcUa_UncertainDependentValueChanged";
    case OpcUa_UncertainDominantValueChanged:
        return "OpcUa_UncertainDominantValueChanged";
    case OpcUa_UncertainEngineeringUnitsExceeded:
        return "OpcUa_UncertainEngineeringUnitsExceeded";
    case OpcUa_UncertainInitialValue:
        return "OpcUa_UncertainInitialValue";
    case OpcUa_UncertainLastUsableValue:
        return "OpcUa_UncertainLastUsableValue";
    case OpcUa_UncertainNoCommunicationLastUsableValue:
        return "OpcUa_UncertainNoCommunicationLastUsableValue";
    case OpcUa_UncertainNotAllNodesAvailable:
        return "OpcUa_UncertainNotAllNodesAvailable";
    case OpcUa_UncertainReferenceNotDeleted:
        return "OpcUa_UncertainReferenceNotDeleted";
    case OpcUa_UncertainReferenceOutOfServer:
        return "OpcUa_UncertainReferenceOutOfServer";
    case OpcUa_UncertainSensorCalibration:
        return "OpcUa_UncertainSensorCalibration";
    case OpcUa_UncertainSensorNotAccurate:
        return "OpcUa_UncertainSensorNotAccurate";
    case OpcUa_UncertainSimulatedValue:
        return "OpcUa_UncertainSimulatedValue";
    case OpcUa_UncertainSubNormal:
        return "OpcUa_UncertainSubNormal";
    case OpcUa_UncertainSubstituteValue:
        return "OpcUa_UncertainSubstituteValue";
    case OpcUa_UncertainTransducerInManual:
        return "OpcUa_UncertainTransducerInManual";
    case 0x00000000:
        return "OpcUa_Good";
    default:
        *found = false;
        if (SOPC_IsBadStatus(status))
        {
            return "(Unknown) Bad Status Code";
        }
        else if (SOPC_IsUncertainStatus(status))
        {
            return "(Unknown) Uncertain Status Code";
        }
        else if (SOPC_IsGoodStatus(status))
        {
            return "(Unknown) Good Status Code";
        }
        else
        {
            return "Unknown Status Code";
        }
    }
}

const char* SOPC_StatusCodeToString(SOPC_StatusCode status)
{
    bool found = false;
    return SOPC_Internal_StatusCodeToString(status, &found);
}

char* SOPC_StatusCodeToStringAlloc(SOPC_StatusCode status)
{
    bool found = false;
    const char* result = SOPC_Internal_StatusCodeToString(status, &found);
    if (found)
    {
        return SOPC_strdup(result);
    }
    else
    {
        char* statusCode = SOPC_Calloc(11, sizeof(char));
        if (NULL == statusCode)
        {
            return NULL;
        }
        snprintf(statusCode, 11, "0x%08X", (unsigned) status);
        return statusCode;
    }
}
