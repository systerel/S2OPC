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

#include "sopc_chunks_mgr.h"
#include "sopc_chunks_mgr_internal.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "opcua_statuscodes.h"
#include "sopc_crypto_provider.h"

#include "sopc_encoder.h"
#include "sopc_event_timer_manager.h"
#include "sopc_logger.h"
#include "sopc_macros.h"
#include "sopc_mem_alloc.h"
#include "sopc_protocol_constants.h"
#include "sopc_secure_channels_api.h"
#include "sopc_secure_channels_api_internal.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_singly_linked_list.h"
#include "sopc_sockets_api.h"
#include "sopc_toolkit_config_internal.h"

static const uint8_t SOPC_HEL[3] = {'H', 'E', 'L'};
static const uint8_t SOPC_ACK[3] = {'A', 'C', 'K'};
static const uint8_t SOPC_ERR[3] = {'E', 'R', 'R'};
static const uint8_t SOPC_RHE[3] = {'R', 'H', 'E'};
static const uint8_t SOPC_MSG[3] = {'M', 'S', 'G'};
static const uint8_t SOPC_OPN[3] = {'O', 'P', 'N'};
static const uint8_t SOPC_CLO[3] = {'C', 'L', 'O'};

#define SOPC_UA_ABORT_FINAL_CHUNK 'A'
#define SOPC_UA_INTERMEDIATE_CHUNK 'C'
#define SOPC_UA_FINAL_CHUNK 'F'

/**
 * \brief Fills a target buffer from a source buffer up to a given length.
 *
 * Note that this function may copy LESS than \c n bytes to dst, if \c src is
 * less than \c n bytes long. This is NOT considered as an error, the caller
 * should call this function again when there is more data in \c src.
 *
 * In other words, \c dst has been successfully filled if and only if this
 * function returns \c TRUE and \c remaining is set to 0.
 *
 * \param dst        The buffer to fill.
 * \param src        The source buffer.
 * \param n          The number of bytes to accumulate in \c dst.
 * \param remaining  Out parameter, the number of bytes remaining to fill.
 *
 * \return \c TRUE in case of success, \c FALSE in case reading from \c src
 *         fails.
 */
static bool fill_buffer(SOPC_Buffer* dst, SOPC_Buffer* src, uint32_t n, uint32_t* remaining)
{
    if (SOPC_Buffer_Remaining(dst) >= n)
    {
        *remaining = 0;
        return true;
    }

    uint32_t missing = n - SOPC_Buffer_Remaining(dst);
    int64_t read = SOPC_Buffer_ReadFrom(dst, src, missing);

    if (read < 0)
    {
        return false;
    }

    *remaining = n - SOPC_Buffer_Remaining(dst);
    return true;
}

static SOPC_SecureChannel_Config* SOPC_Toolkit_GetSecureChannelConfig(SOPC_SecureConnection* scConnection)
{
    if (scConnection->isServerConnection)
    {
        return SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    }
    else
    {
        return SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
    }
}

static uint32_t SC_Client_StartRequestTimeout(uint32_t connectionIdx, uint32_t requestHandle, uint32_t requestId)
{
    if (SOPC_REQUEST_TIMEOUT_MS <= 0)
    {
        // No request timeout: no timer created
        return 0;
    }

    SOPC_Event event;
    event.event = TIMER_SC_REQUEST_TIMEOUT;
    event.eltId = connectionIdx;
    event.params = requestHandle;
    event.auxParam = requestId;
    uint32_t timerId = SOPC_EventTimer_Create(secureChannelsTimerEventHandler, event, SOPC_REQUEST_TIMEOUT_MS);

    if (0 == timerId)
    {
        SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                 "Services: connection=%" PRIu32 " request timeout timer creation failed",
                                 connectionIdx);
    }

    return timerId;
}

static bool SC_Chunks_DecodeTcpMsgHeader(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx, SOPC_StatusCode* errorStatus)
{
    assert(chunkCtx != NULL);
    assert(chunkCtx->currentChunkInputBuffer != NULL);
    assert(chunkCtx->currentChunkInputBuffer->length - chunkCtx->currentChunkInputBuffer->position >=
           SOPC_TCP_UA_HEADER_LENGTH);
    assert(chunkCtx->currentMsgType == SOPC_MSG_TYPE_INVALID);
    assert(chunkCtx->currentMsgIsFinal == SOPC_MSG_ISFINAL_INVALID);
    assert(chunkCtx->currentMsgSize == 0);
    assert(errorStatus != NULL);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    bool result = false;
    uint8_t msgType[3];
    uint8_t isFinal;

    // READ message type
    status = SOPC_Buffer_Read(msgType, chunkCtx->currentChunkInputBuffer, 3);
    if (SOPC_STATUS_OK == status)
    {
        result = true;
        if (memcmp(msgType, SOPC_HEL, 3) == 0)
        {
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_HEL;
        }
        else if (memcmp(msgType, SOPC_ACK, 3) == 0)
        {
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_ACK;
        }
        else if (memcmp(msgType, SOPC_ERR, 3) == 0)
        {
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_ERR;
        }
        else if (memcmp(msgType, SOPC_MSG, 3) == 0)
        {
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_SC_MSG;
        }
        else if (memcmp(msgType, SOPC_OPN, 3) == 0)
        {
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_SC_OPN;
        }
        else if (memcmp(msgType, SOPC_CLO, 3) == 0)
        {
            chunkCtx->currentMsgType = SOPC_MSG_TYPE_SC_CLO;
        }
        else
        {
            // unchanged current msg type => error case
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: decoding TCP UA header: invalid msg type='%c%c%c'", (char) msgType[0],
                                   (char) msgType[1], (char) msgType[2]);
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;
            result = false;
        }
    }

    // READ IsFinal message chunk
    if (result)
    {
        status = SOPC_Buffer_Read(&isFinal, chunkCtx->currentChunkInputBuffer, 1);
        if (SOPC_STATUS_OK == status)
        {
            switch (isFinal)
            {
            case SOPC_UA_INTERMEDIATE_CHUNK:
                chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_INTERMEDIATE;
                break;
            case SOPC_UA_FINAL_CHUNK:
                chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_FINAL;
                break;
            case SOPC_UA_ABORT_FINAL_CHUNK:
                chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_ABORT;
                break;
            default:
                // unchanged current isFinal value => error case
                result = false;
                *errorStatus = OpcUa_BadTcpMessageTypeInvalid;
                break;
            }

            // In TCP UA non secure messages reserved byte shall be set to 'F'
            if (chunkCtx->currentMsgType != SOPC_MSG_TYPE_SC_MSG)
            {
                if (chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "ChunksMgr: decoding TCP UA header: invalid isFinal='%c' for given msg type='%c%c%c'",
                        (char) isFinal, (char) msgType[0], (char) msgType[1], (char) msgType[2]);
                    *errorStatus = OpcUa_BadTcpMessageTypeInvalid;
                    result = false;
                }
            } // else we could receive 'C', 'A' or 'F'
        }
    }

    // READ message size
    if (result)
    {
        status = SOPC_UInt32_Read(&chunkCtx->currentMsgSize, chunkCtx->currentChunkInputBuffer, 0);
        if (SOPC_STATUS_OK != status || chunkCtx->currentMsgSize <= SOPC_TCP_UA_HEADER_LENGTH)
        {
            // Message size cannot be less or equal to the TCP UA header length
            *errorStatus = OpcUa_BadEncodingError;
            result = false;
        }
        else if (chunkCtx->currentMsgSize > chunkCtx->currentChunkInputBuffer->maximum_size)
        {
            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ChunksMgr: decoding TCP UA header: message size=%u indicated greater than receiveBufferSize=%u",
                chunkCtx->currentMsgSize, chunkCtx->currentChunkInputBuffer->maximum_size);
            *errorStatus = OpcUa_BadTcpMessageTooLarge;
            result = false;
        }
    }

    return result;
}

static SOPC_SecureChannels_InternalEvent SC_Chunks_MsgTypeToRcvEvent(SOPC_Msg_Type msgType,
                                                                     SOPC_Msg_IsFinal currentMsgIsFinal)
{
    switch (msgType)
    {
    case SOPC_MSG_TYPE_HEL:
        return INT_SC_RCV_HEL;
        break;
    case SOPC_MSG_TYPE_ACK:
        return INT_SC_RCV_ACK;
        break;
    case SOPC_MSG_TYPE_ERR:
        return INT_SC_RCV_ERR;
        break;
    case SOPC_MSG_TYPE_SC_OPN:
        return INT_SC_RCV_OPN;
        break;
    case SOPC_MSG_TYPE_SC_CLO:
        return INT_SC_RCV_CLO;
        break;
    case SOPC_MSG_TYPE_SC_MSG:
        if (SOPC_MSG_ISFINAL_ABORT == currentMsgIsFinal)
        {
            return INT_SC_RCV_MSG_CHUNK_ABORT;
        }
        else
        {
            return INT_SC_RCV_MSG_CHUNKS;
        }
        break;
    default:
        assert(false);
        return INT_SC_RCV_ERR;
    }
}

static bool SC_Chunks_IsMsgEncrypted(OpcUa_MessageSecurityMode securityMode, bool isOPN)
{
    assert(securityMode != OpcUa_MessageSecurityMode_Invalid);
    bool toEncrypt = true;
    // Determine if the message must be encrypted
    if (securityMode == OpcUa_MessageSecurityMode_None || (securityMode == OpcUa_MessageSecurityMode_Sign && !isOPN))
    {
        toEncrypt = false;
    }

    return toEncrypt;
}

static bool SC_Chunks_IsMsgSigned(OpcUa_MessageSecurityMode securityMode)
{
    bool toSign = true;
    // Determine if the message must be signed
    if (securityMode == OpcUa_MessageSecurityMode_None)
    {
        toSign = false;
    }
    return toSign;
}

static bool SC_Chunks_DecodeAsymSecurityHeader_Certificates(SOPC_SecureConnection* scConnection,
                                                            SOPC_Endpoint_Config* epConfig,
                                                            SOPC_SecureChannel_Config* scConfig,
                                                            bool* senderCertificatePresence,
                                                            SOPC_CertificateList** clientSenderCertificate,
                                                            bool* receiverCertificatePresence,
                                                            SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConnection->cryptoProvider != NULL);
    assert(scConnection->chunksCtx.currentChunkInputBuffer != NULL);
    assert(senderCertificatePresence != NULL);
    assert(receiverCertificatePresence != NULL);
    assert(clientSenderCertificate != NULL);
    assert(errorStatus != NULL);

    bool result = true;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    /* toEncrypt & toSign are considered possible (true) if we do not enforce the security mode.
     * The security mode can only be enforced when it was already defined by a received OPN message
     * since it is not present in the asymmetric security header (only security policy defined).
     */
    bool enforceSecuMode = false;
    bool toEncrypt = true;
    bool toSign = true;
    SOPC_ByteString otherBsAppCert;
    SOPC_ByteString_Initialize(&otherBsAppCert);
    const SOPC_CertificateList* runningAppCert = NULL;
    const SOPC_PKIProvider* pkiProvider = NULL;
    SOPC_ByteString senderCertificate;
    SOPC_ByteString_Initialize(&senderCertificate);
    SOPC_ByteString receiverCertThumb;
    SOPC_ByteString_Initialize(&receiverCertThumb);
    uint32_t tmpLength = 0;
    uint32_t scConfigIdx = 0;
    uint32_t epConfigIdx = 0;

    if (!scConnection->isServerConnection)
    {
        // CLIENT side: config is mandatory and security mode to be enforced
        assert(scConfig != NULL);
        runningAppCert = scConnection->clientCertificate;
        pkiProvider = scConfig->pki;
        enforceSecuMode = true;
        if (scConfig->crt_srv != NULL)
        {
            // retrieve expected sender certificate as a ByteString
            status =
                SOPC_KeyManager_Certificate_ToDER(scConnection->serverCertificate, &otherBsAppCert.Data, &tmpLength);
            if (SOPC_STATUS_OK == status && tmpLength > 0)
            {
                otherBsAppCert.Length = (int32_t) tmpLength;
            }
        }
        epConfigIdx = scConnection->serverEndpointConfigIdx;
        scConfigIdx = scConnection->endpointConnectionConfigIdx;
    }
    else
    {
        // SERVER side: client config could be defined or not (new secure channel opening)
        assert(epConfig != NULL);
        runningAppCert = scConnection->serverCertificate;
        pkiProvider = epConfig->serverConfigPtr->pki;
        if (scConfig != NULL)
        {
            enforceSecuMode = true;
            if (scConfig->crt_cli != NULL)
            {
                // retrieve expected sender certificate as a ByteString
                status = SOPC_KeyManager_Certificate_ToDER(scConnection->clientCertificate, &otherBsAppCert.Data,
                                                           &tmpLength);
                if (SOPC_STATUS_OK == status && tmpLength > 0)
                {
                    otherBsAppCert.Length = (int32_t) tmpLength;
                }
            }
        }
        scConfigIdx = scConnection->endpointConnectionConfigIdx;
    }

    // Retrieve encryption and signature configuration expected if defined
    if (enforceSecuMode)
    {
        toEncrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, true);
        toSign = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);
    }

    // Sender Certificate:
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ByteString_Read(&senderCertificate, scConnection->chunksCtx.currentChunkInputBuffer, 0);

        if (SOPC_STATUS_OK != status)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr (asym cert): sender certificate decoding error (epCfgIdx=%" PRIu32
                                   " scCfgIdx=%" PRIu32 "): status=%" PRIX32,
                                   epConfigIdx, scConfigIdx, status);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        if (!toSign && senderCertificate.Length > 0)
        {
            status = SOPC_STATUS_NOK;
            // Table 27 part 6: "field shall be null if the Message is not signed"
            *errorStatus = OpcUa_BadCertificateUseNotAllowed;
            *senderCertificatePresence = true;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr (asym cert): sender certificate presence not expected (epCfgIdx=%" PRIu32
                                   " scCfgIdx=%" PRIu32 ")",
                                   epConfigIdx, scConfigIdx);
        }
        else if (toSign && senderCertificate.Length > 0)
        {
            // Sender certificate is present
            *senderCertificatePresence = true;
            if (scConfig != NULL)
            {
                // Check certificate is the same as the one in memory
                // (CLIENT side or SERVER side with already established channel)
                // SERVER side: part 6 v1.03 §6.7.4:
                // Part 6 §6.7.4 aussi il me semble :
                // "The Server shall reject renew requests if the SenderCertificate is not the same as the one used to
                // create the SecureChannel or if there is a problem decrypting or verifying the signature."
                int32_t otherAppCertComparison = 0;
                status = SOPC_ByteString_Compare(&otherBsAppCert, &senderCertificate, &otherAppCertComparison);

                if (status != SOPC_STATUS_OK || otherAppCertComparison != 0)
                {
                    *errorStatus = OpcUa_BadCertificateInvalid;
                    status = SOPC_STATUS_NOK;

                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "ChunksMgr (asym cert): sender certificate is not the one expected (epCfgIdx=%" PRIu32
                        " scCfgIdx=%" PRIu32 ")",
                        epConfigIdx, scConfigIdx);
                }
            }

            if (SOPC_STATUS_OK == status)
            {
                SOPC_CertificateList* cert = NULL;
                status = SOPC_KeyManager_Certificate_CreateOrAddFromDER(senderCertificate.Data,
                                                                        (uint32_t) senderCertificate.Length, &cert);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_CryptoProvider_Certificate_Validate(scConnection->cryptoProvider, pkiProvider, cert,
                                                                      errorStatus);
                }
                if (SOPC_STATUS_OK != status)
                {
                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "ChunksMgr (asym cert): sender certificate validation failed (epCfgIdx=%" PRIu32
                        " scCfgIdx=%" PRIu32 ") with error: %X",
                        epConfigIdx, scConfigIdx, *errorStatus);

                    // TODO:  keep reason in some cases ?
                    *errorStatus = OpcUa_BadTcpInternalError;
                }

                if (!scConnection->isServerConnection || status != SOPC_STATUS_OK)
                {
                    SOPC_KeyManager_Certificate_Free(cert);
                }
                else
                {
                    // Set client application certificate to record
                    *clientSenderCertificate = cert;
                }
            }
        }
        else if (!enforceSecuMode || !toSign)
        {
            // Without security mode to enforce, sender certificate absence could be normal
            *senderCertificatePresence = false;
        }
        else
        {
            status = SOPC_STATUS_NOK;
            // Sender certificate was expected
            *errorStatus = OpcUa_BadCertificateInvalid;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr (asym cert): sender certificate presence expected (epCfgIdx=%" PRIu32
                                   " scCfgIdx=%" PRIu32 ")",
                                   epConfigIdx, scConfigIdx);
        }
    }
    else
    {
        // status == STATUS_NOK
        *errorStatus = OpcUa_BadTcpInternalError;

        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ChunksMgr (asym cert): certificate copy error (epCfgIdx=%" PRIu32 " scCfgIdx=%" PRIu32
                               ")",
                               epConfigIdx, scConfigIdx);
    }

    // Receiver Certificate Thumbprint:
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ByteString_Read(&receiverCertThumb, scConnection->chunksCtx.currentChunkInputBuffer, 0);

        if (SOPC_STATUS_OK == status)
        {
            if (!toEncrypt && receiverCertThumb.Length > 0)
            {
                // Table 27 part 6: "field shall be null if the Message is not encrypted"
                *errorStatus = OpcUa_BadCertificateUseNotAllowed;
                *receiverCertificatePresence = true;

                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "ChunksMgr (asym cert): receiver thumbprint presence not expected (epCfgIdx=%" PRIu32
                    " scCfgIdx=%" PRIu32 ")",
                    epConfigIdx, scConfigIdx);
            }
            else if (toEncrypt && receiverCertThumb.Length > 0)
            {
                // Check thumbprint matches current app certificate thumbprint
                *receiverCertificatePresence = true;
                SOPC_ByteString curAppCertThumbprint;
                SOPC_ByteString_Initialize(&curAppCertThumbprint);
                uint32_t thumbprintLength = 0;
                int32_t runningAppCertComparison = 0;

                status = SOPC_CryptoProvider_CertificateGetLength_Thumbprint(scConnection->cryptoProvider,
                                                                             &thumbprintLength);

                if (SOPC_STATUS_OK == status && thumbprintLength > INT32_MAX)
                {
                    status = SOPC_STATUS_NOK;
                    *errorStatus = OpcUa_BadCertificateInvalid;
                }
                else if (SOPC_STATUS_OK != status)
                {
                    *errorStatus = OpcUa_BadTcpInternalError;
                }

                if (SOPC_STATUS_OK == status)
                {
                    if ((int32_t) thumbprintLength == receiverCertThumb.Length)
                    {
                        status = SOPC_ByteString_InitializeFixedSize(&curAppCertThumbprint, thumbprintLength);
                        if (SOPC_STATUS_OK == status)
                        {
                            status =
                                SOPC_KeyManager_Certificate_GetThumbprint(scConnection->cryptoProvider, runningAppCert,
                                                                          curAppCertThumbprint.Data, thumbprintLength);

                            if (SOPC_STATUS_OK == status)
                            {
                                status = SOPC_ByteString_Compare(&curAppCertThumbprint, &receiverCertThumb,
                                                                 &runningAppCertComparison);

                                if (status != SOPC_STATUS_OK || runningAppCertComparison != 0)
                                {
                                    status = SOPC_STATUS_NOK;
                                    *errorStatus = OpcUa_BadCertificateInvalid;

                                    SOPC_Logger_TraceError(
                                        SOPC_LOG_MODULE_CLIENTSERVER,
                                        "ChunksMgr (asym cert): invalid receiver thumbprint (epCfgIdx=%" PRIu32
                                        " scCfgIdx=%" PRIu32 ")",
                                        epConfigIdx, scConfigIdx);
                                }
                            }
                            else
                            {
                                *errorStatus = OpcUa_BadTcpInternalError;

                                SOPC_Logger_TraceError(
                                    SOPC_LOG_MODULE_CLIENTSERVER,
                                    "ChunksMgr (asym cert): thumbprint computation failed (epCfgIdx=%" PRIu32
                                    " scCfgIdx=%" PRIu32 ")",
                                    epConfigIdx, scConfigIdx);
                            }
                        }
                        else
                        {
                            *errorStatus = OpcUa_BadTcpInternalError;
                        }
                    }
                    else
                    {
                        status = SOPC_STATUS_NOK;
                        *errorStatus = OpcUa_BadCertificateInvalid;

                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "ChunksMgr (asym cert): invalid thumbprint size (epCfgIdx=%" PRIu32
                                               " scCfgIdx=%" PRIu32 ")",
                                               epConfigIdx, scConfigIdx);
                    }
                } // if thumbprint length correctly computed

                SOPC_ByteString_Clear(&curAppCertThumbprint);
            }
            else if (!enforceSecuMode || !toEncrypt)
            { // if toEncrypt
                // Without security mode to enforce, absence could be normal
                *receiverCertificatePresence = false;
            }
            else
            {
                status = SOPC_STATUS_NOK;
                // absence was not expected
                *errorStatus = OpcUa_BadCertificateInvalid;

                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr (asym cert): thumbprint presence expected (epCfgIdx=%" PRIu32
                                       " scCfgIdx=%" PRIu32 ")",
                                       epConfigIdx, scConfigIdx);
            }
        }
        else
        { // if decoded thumbprint
            *errorStatus = OpcUa_BadTcpInternalError;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr (asym cert): receiver thumbprint decoding error (epCfgIdx=%" PRIu32
                                   " scCfgIdx=%" PRIu32 ")",
                                   epConfigIdx, scConfigIdx);
        }
    }

    SOPC_ByteString_Clear(&otherBsAppCert);

    SOPC_ByteString_Clear(&senderCertificate);
    SOPC_ByteString_Clear(&receiverCertThumb);

    if (SOPC_STATUS_OK != status)
    {
        result = false;
    }

    return result;
}

static bool SC_Chunks_CheckAsymmetricSecurityHeader(SOPC_SecureConnection* scConnection,
                                                    bool* isSecurityActive,
                                                    SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.currentChunkInputBuffer != NULL);

    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool result = true;
    const char* validSecuPolicy = NULL;
    uint16_t validSecuModes = 0;
    bool isSecureModeActive = true;
    SOPC_String securityPolicy;
    SOPC_String tmpStr;
    SOPC_String_Initialize(&tmpStr);
    SOPC_String_Initialize(&securityPolicy);
    SOPC_SecureChannel_Config* clientConfig = NULL;
    SOPC_Endpoint_Config* serverConfig = NULL;
    bool senderCertifPresence = false;
    bool receiverCertifThumbprintPresence = false;
    SOPC_CertificateList* clientCertificate = NULL;
    int32_t compareRes = -1;
    bool isCompliantPolicy = false;
    uint32_t idx = 0;
    uint32_t scConfigIdx = 0;
    uint32_t epConfigIdx = 0;

    if (!scConnection->isServerConnection)
    {
        scConfigIdx = scConnection->endpointConnectionConfigIdx;
        // CLIENT side
        clientConfig = SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
        if (NULL == clientConfig)
        {
            result = false;
            *errorStatus = OpcUa_BadInvalidState;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr (asym header): SC configuration not found (epCfgIdx=%" PRIu32
                                   " scCfgIdx=%" PRIu32 ")",
                                   epConfigIdx, scConfigIdx);
        }
    }
    else
    {
        // SERVER side
        epConfigIdx = scConnection->serverEndpointConfigIdx;
        scConfigIdx = scConnection->endpointConnectionConfigIdx;
        serverConfig = SOPC_ToolkitServer_GetEndpointConfig(scConnection->serverEndpointConfigIdx);
        if (NULL == serverConfig)
        {
            result = false;
            *errorStatus = OpcUa_BadInvalidState;
        }
        if (scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED ||
            scConnection->state == SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)
        {
            // A client connection config shall already be defined and contains security expected
            clientConfig = SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            if (NULL == clientConfig)
            {
                result = false;
                *errorStatus = OpcUa_BadInvalidState;

                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr (asym header): SC configuration not found (epCfgIdx=%" PRIu32
                                       " scCfgIdx=%" PRIu32 ")",
                                       epConfigIdx, scConfigIdx);
            }
        }
    }

    if (result)
    {
        // Decode security policy (Part6: "this value shall not exceed 255 bytes")
        status = SOPC_String_ReadWithLimitedLength(&securityPolicy, 255, chunkCtx->currentChunkInputBuffer, 0);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
            *errorStatus = OpcUa_BadDecodingError;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr (asym header): security policy decoding failed (epCfgIdx=%" PRIu32
                                   " scCfgIdx=%" PRIu32 ")",
                                   epConfigIdx, scConfigIdx);
        }
    }

    if (result)
    {
        if (clientConfig != NULL)
        {
            // CLIENT side (expected same as requested)
            // SERVER side (clientConfig != NULL => expected same as initial one)
            status = SOPC_String_CopyFromCString(&tmpStr, clientConfig->reqSecuPolicyUri);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_String_Compare(&tmpStr, &securityPolicy, true, &compareRes);
            }
            if (SOPC_STATUS_OK != status)
            {
                result = false;
                *errorStatus = OpcUa_BadDecodingError;

                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr (asym header): security policy value unexpected (epCfgIdx=%" PRIu32
                                       " scCfgIdx=%" PRIu32 ")",
                                       epConfigIdx, scConfigIdx);
            }
            else
            {
                isCompliantPolicy = 0 == compareRes;
                validSecuPolicy = clientConfig->reqSecuPolicyUri;
            }
        }
        else
        {
            assert(scConnection->isServerConnection);
            // SERVER side (shall comply with one or several server security configuration)
            compareRes = -1;
            for (idx = 0; idx < serverConfig->nbSecuConfigs; idx++)
            {
                SOPC_SecurityPolicy* secuPolicy = &(serverConfig->secuConfigurations[idx]);
                status = SOPC_String_Compare(&securityPolicy, &secuPolicy->securityPolicy, true, &compareRes);
                if (SOPC_STATUS_OK == status)
                {
                    if (compareRes == 0)
                    {
                        isCompliantPolicy = true;
                        // It might be true for several endpoint configurations
                        // (separate endpoints with same security policy and different security mode)
                        validSecuPolicy = SOPC_String_GetRawCString(&secuPolicy->securityPolicy);
                        validSecuModes |= secuPolicy->securityModes;
                    }
                }
            }
        }
        // Rejected if not compatible with security polic-y/ies expected
        if (!isCompliantPolicy)
        {
            result = false;
            *errorStatus = OpcUa_BadSecurityPolicyRejected;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr (asym header): security policy rejected (epCfgIdx=%" PRIu32
                                   " scCfgIdx=%" PRIu32 ")",
                                   epConfigIdx, scConfigIdx);
        }
    }

    if (result && NULL == scConnection->cryptoProvider)
    {
        scConnection->cryptoProvider = SOPC_CryptoProvider_Create(validSecuPolicy);
        if (NULL == scConnection->cryptoProvider)
        {
            // Rejected by the cryptographic component
            result = false;
            *errorStatus = OpcUa_BadSecurityPolicyRejected;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr (asym header): security policy invalid (epCfgIdx=%" PRIu32
                                   " scCfgIdx=%" PRIu32 ")",
                                   epConfigIdx, scConfigIdx);
        }
    }

    if (result)
    {
        result = SC_Chunks_DecodeAsymSecurityHeader_Certificates(scConnection, serverConfig, clientConfig,
                                                                 &senderCertifPresence, &clientCertificate,
                                                                 &receiverCertifThumbprintPresence, errorStatus);

        if (!result)
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr (asym header): certificates decoding failed (epCfgIdx=%" PRIu32
                                   " scCfgIdx=%" PRIu32 ")",
                                   epConfigIdx, scConfigIdx);
        }
    }

    if (result)
    {
        // Since the security mode could be unknown before decoding (and possibly decrypting) the message
        // we have to deduce it from the certificates presence (None or SignAndEncrypt only possible in OPN)
        if (!senderCertifPresence && !receiverCertifThumbprintPresence)
        {
            isSecureModeActive = false;
        }
        else if (senderCertifPresence && receiverCertifThumbprintPresence)
        {
            isSecureModeActive = true;
        }
        else
        {
            result = false;
            *errorStatus = OpcUa_BadCertificateInvalid;

            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ChunksMgr (asym header): certificates presence constraints not verified (epCfgIdx=%" PRIu32
                " scCfgIdx=%" PRIu32 ")",
                epConfigIdx, scConfigIdx);
        }

        // In case secure channel config is already done (RENEW), check it is correct
        if (result && clientConfig != NULL)
        {
            if (!isSecureModeActive && clientConfig->msgSecurityMode == OpcUa_MessageSecurityMode_None)
            {
                // OK it is compatible
            }
            else if (isSecureModeActive && (clientConfig->msgSecurityMode == OpcUa_MessageSecurityMode_Sign ||
                                            clientConfig->msgSecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt))
            {
                // OK it is compatible
            }
            else
            {
                // Incompatible parameters with already configured security mode
                result = false;
                *errorStatus = OpcUa_BadSecurityChecksFailed;

                SOPC_Logger_TraceError(
                    SOPC_LOG_MODULE_CLIENTSERVER,
                    "ChunksMgr (asym header): security mode constraints not verified (epCfgIdx=%" PRIu32
                    " scCfgIdx=%" PRIu32 ")",
                    epConfigIdx, scConfigIdx);
            }
        }
    }

    SOPC_String_Clear(&tmpStr);
    SOPC_String_Clear(&securityPolicy);

    if (result)
    {
        *isSecurityActive = isSecureModeActive;
        if (NULL == clientConfig && scConnection->isServerConnection)
        {
            // SERVER side and only for a new secure channel (<= NULL == clientConfig):
            // - fill temporary secu data necessary to terminate OPN treatment

            scConnection->serverAsymmSecuInfo.clientCertificate = clientCertificate;
            scConnection->serverAsymmSecuInfo.securityPolicyUri = validSecuPolicy;
            scConnection->serverAsymmSecuInfo.validSecurityModes = validSecuModes;
            scConnection->serverAsymmSecuInfo.isSecureModeActive = isSecureModeActive;
        }
        else
        {
            SOPC_KeyManager_Certificate_Free(clientCertificate);
        }
    }

    if (!result && scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED &&
        scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED_RENEW)
    {
        // Replace any error with generic error to be used before connection establishment
        *errorStatus = OpcUa_BadSecurityChecksFailed;
    }
    return result;
}

static bool SC_Chunks_IsSecuTokenValid(bool isServerConnection, SOPC_SecureConnection_SecurityToken secuToken)
{
    bool result = false;
    SOPC_TimeReference currentTimeRef = SOPC_TimeReference_GetCurrent();
    uint64_t expiredMs = 0;
    if (currentTimeRef <= secuToken.lifetimeEndTimeRef)
    {
        result = true;
    }
    else
    {
        if (!isServerConnection)
        {
            /* Client side:
             * Specification 1.03 part 4 §5.5.2.1:
             * c) Clients should accept Messages secured by an expired SecurityToken for up to 25 % of the token
             * lifetime.
             */
            // Number of milliseconds since token expiration
            expiredMs = currentTimeRef - secuToken.lifetimeEndTimeRef;
            if (expiredMs <= secuToken.revisedLifetime / 4)
            {
                result = true;
            }
        } // else: nothing indicated on server side => token invalid once expired
    }
    return result;
}

static bool SC_Chunks_CheckSymmetricSecurityHeader(SOPC_SecureConnection* scConnection,
                                                   bool* isPrevCryptoData,
                                                   SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.currentChunkInputBuffer != NULL);

    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    uint32_t tokenId = 0;
    bool result = true;
    bool isTokenValid = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    status = SOPC_UInt32_Read(&tokenId, chunkCtx->currentChunkInputBuffer, 0);

    if (SOPC_STATUS_OK == status)
    {
        if (!scConnection->isServerConnection)
        {
            // CLIENT side: should accept expired security token up to 25% of the token lifetime
            /* Note:
             * From part 6: The Client shall continue to accept the old SecurityToken until it receives the
             * OpenSecureChannel response.
             * => does not imply that old one is refused after that
             * => the server need to send messages with old one until it received a new message from client
             * From part 4: Clients should accept Messages secured by an expired SecurityToken for up to 25 % of the
             * token lifetime.
             * => no indication on old or new security token provided
             */
            if (scConnection->currentSecurityToken.tokenId == tokenId)
            {
                *isPrevCryptoData = false;

                // Check token expiration
                isTokenValid =
                    SC_Chunks_IsSecuTokenValid(scConnection->isServerConnection, scConnection->currentSecurityToken);
            }
            else if (scConnection->precedentSecurityToken.tokenId ==
                         tokenId && // security token is the precedent one
                                    // check precedent security token is defined
                     scConnection->precedentSecurityToken.secureChannelId != 0 &&
                     scConnection->precedentSecurityToken.tokenId != 0)
            {
                // Still valid with old security token => OK
                *isPrevCryptoData = true;

                // Check token expiration
                isTokenValid =
                    SC_Chunks_IsSecuTokenValid(scConnection->isServerConnection, scConnection->precedentSecurityToken);
            }
            else
            {
                result = false;
                *errorStatus = OpcUa_BadSecureChannelTokenUnknown;
            }

            if (result && !isTokenValid)
            {
                result = false;
                *errorStatus = OpcUa_BadSecureChannelTokenUnknown;
            }
        }
        else
        {
            /* SERVER side:
             * - accepts precedent token, even if a new one is defined, until token expiration
             * - accepts the current or new token. If first use of new token set the flag to use the new one to send MSG
             */
            /* Note:
             * From part 6: The Server has to accept requests secured with the old SecurityToken until that
             * SecurityToken expires or until it receives a Message from the Client secured with the new SecurityToken.
             * => does not clearly imply that old one is refused after Client sent a message with new one
             */
            if (scConnection->currentSecurityToken.tokenId == tokenId)
            {
                // It shall be the current security token or first use of new security token
                if (!scConnection->serverNewSecuTokenActive)
                {
                    // In case of first use of new security token, we shall enforce the old one cannot be used
                    // anymore
                    scConnection->serverNewSecuTokenActive = true;
                }
                *isPrevCryptoData = false;

                // Check token expiration
                isTokenValid =
                    SC_Chunks_IsSecuTokenValid(scConnection->isServerConnection, scConnection->currentSecurityToken);
            }
            else if (scConnection->precedentSecurityToken.tokenId ==
                         tokenId && // security token is the precedent one
                                    // check precedent security token is defined
                     scConnection->precedentSecurityToken.secureChannelId != 0 &&
                     scConnection->precedentSecurityToken.tokenId != 0)
            {
                // Still valid with old security token => OK
                *isPrevCryptoData = true;

                // Check token expiration
                isTokenValid =
                    SC_Chunks_IsSecuTokenValid(scConnection->isServerConnection, scConnection->precedentSecurityToken);
            }
            else
            {
                result = false;
                *errorStatus = OpcUa_BadSecureChannelTokenUnknown;
            }

            if (result && !isTokenValid)
            {
                result = false;
                *errorStatus = OpcUa_BadSecureChannelTokenUnknown;
            }
        }
    }
    else
    {
        result = false;
        *errorStatus = OpcUa_BadTcpInternalError;
    }

    return result;
}

static bool SC_Chunks_CheckSeqNumReceived(SOPC_SecureConnection* scConnection,
                                          bool isOPN,
                                          uint32_t seqNumber,
                                          SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    bool result = true;

    if (!isOPN)
    {
        if (scConnection->tcpSeqProperties.lastSNreceived + 1 != seqNumber)
        {
            // Part 6 §6.7.2 v1.03
            if (scConnection->tcpSeqProperties.lastSNreceived > UINT32_MAX - 1024 && seqNumber < 1024)
            {
                scConnection->tcpSeqProperties.lastSNreceived = seqNumber;
            }
            else
            {
                result = false;
                *errorStatus = OpcUa_BadSecurityChecksFailed;
            }
        }
        else
        {
            // Correct sequence number
            scConnection->tcpSeqProperties.lastSNreceived++;
        }
    }
    else
    {
        // reset sequence number since it is an OPN
        scConnection->tcpSeqProperties.lastSNreceived = seqNumber;
    }

    return result;
}

static bool SC_Chunks_CheckSequenceHeaderSN(SOPC_SecureConnection* scConnection,
                                            bool isOPN,
                                            SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.currentChunkInputBuffer != NULL);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool result = true;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    uint32_t seqNumber = 0;

    status = SOPC_UInt32_Read(&seqNumber, chunkCtx->currentChunkInputBuffer, 0);

    if (SOPC_STATUS_OK == status)
    {
        result = SC_Chunks_CheckSeqNumReceived(scConnection, isOPN, seqNumber, errorStatus);
    }
    else
    {
        result = false;
        *errorStatus = OpcUa_BadTcpInternalError;
    }

    return result;
}

static bool SC_Chunks_CheckSequenceHeaderRequestId(
    SOPC_SecureConnection* scConnection,
    bool isClient,
    SOPC_Msg_IsFinal receivedMsgIsFinal,
    SOPC_Msg_Type receivedMsgType,
    uint32_t* requestIdOrHandle, // for server or requestHandle for client
    bool* messageTimeoutExpired,
    SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.currentChunkInputBuffer != NULL);
    assert(requestIdOrHandle != NULL);
    assert(messageTimeoutExpired != NULL);

    bool result = true;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;

    SOPC_SentRequestMsg_Context* recordedMsgCtx = NULL;
    // Except if it is a response to an expired request, the message is not expired
    *messageTimeoutExpired = false;

    // Retrieve request id
    result = SOPC_STATUS_OK == SOPC_UInt32_Read(requestIdOrHandle, chunkCtx->currentChunkInputBuffer, 0);
    if (!result)
    {
        result = false;
        *errorStatus = OpcUa_BadTcpInternalError;
    }

    // (In case of multi-chunk message) Check it is the same requestId than previous chunks
    if (result)
    {
        if (chunkCtx->hasCurrentMsgRequestId && *requestIdOrHandle != chunkCtx->currentMsgRequestId)
        {
            // Different requestId found
            result = false;
            *errorStatus = OpcUa_BadSecurityChecksFailed;
        }
    }

    if (result)
    {
        if (isClient)
        {
            // Check received request Id was expected for the received message type
            recordedMsgCtx =
                SOPC_SLinkedList_RemoveFromId(scConnection->tcpSeqProperties.sentRequestIds, *requestIdOrHandle);
            if (recordedMsgCtx != NULL)
            {
                *messageTimeoutExpired = recordedMsgCtx->timeoutExpired;
                if (recordedMsgCtx->msgType != receivedMsgType)
                {
                    SOPC_EventTimer_Cancel(recordedMsgCtx->timerId); // Deactivate timer for this request

                    // Re-enqueue the request id in order application receive the request timeout on SC closure
                    SOPC_SLinkedList_Append(scConnection->tcpSeqProperties.sentRequestIds, *requestIdOrHandle,
                                            (void*) recordedMsgCtx);
                    result = false;
                    *errorStatus = OpcUa_BadSecurityChecksFailed;
                }
                else if (SOPC_MSG_ISFINAL_INTERMEDIATE == receivedMsgIsFinal)
                {
                    // We receive a part of the response message:
                    // do not deactivate timer and re-enqueue the request id as expected id
                    // (Note: prepend since other chunks for this message shall be the next ones)
                    void* prependedCtx = SOPC_SLinkedList_Prepend(scConnection->tcpSeqProperties.sentRequestIds,
                                                                  *requestIdOrHandle, (void*) recordedMsgCtx);
                    result = prependedCtx == recordedMsgCtx;
                }
                else
                {
                    // Complete response received

                    // Deactivate timer for this request
                    SOPC_EventTimer_Cancel(recordedMsgCtx->timerId);
                    // Set the requestHandle (only used in case of abort message)
                    *requestIdOrHandle = recordedMsgCtx->requestHandle;
                    // We received the complete response message
                    SOPC_Free(recordedMsgCtx);
                }
            }
            else
            {
                result = false;
                *errorStatus = OpcUa_BadSecurityChecksFailed;
            }
        }
    }

    if (result)
    {
        // We shall keep data to check same requestId is used for all chunks
        chunkCtx->hasCurrentMsgRequestId = true;
        chunkCtx->currentMsgRequestId = *requestIdOrHandle;
    }

    return result;
}

static bool SC_Chunks_GetSecurityKeySets(SOPC_SecureConnection* scConnection,
                                         bool isPrevCryptoData,
                                         SOPC_SC_SecurityKeySet** senderKeySet,
                                         SOPC_SC_SecurityKeySet** receiverKeySet)
{
    assert(NULL != senderKeySet);
    assert(NULL != receiverKeySet);

    if (isPrevCryptoData)
    {
        *senderKeySet = scConnection->precedentSecuKeySets.senderKeySet;
        *receiverKeySet = scConnection->precedentSecuKeySets.receiverKeySet;
    }
    else
    {
        *senderKeySet = scConnection->currentSecuKeySets.senderKeySet;
        *receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
    }

    if (NULL == *senderKeySet || NULL == *receiverKeySet)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "ChunksMgr: sign or encrypt message (symm): security key sets missing (scConfigIdx=%" PRIu32 ")",
            scConnection->endpointConnectionConfigIdx);
        return false;
    }

    return true;
}

static bool SC_Chunks_DecryptMsg(SOPC_SecureConnection* scConnection,
                                 bool isSymmetric,
                                 bool isPrevCryptoData,
                                 const char** errorReason)
{
    assert(scConnection != NULL);
    SOPC_Buffer* encryptedBuffer = scConnection->chunksCtx.currentChunkInputBuffer;
    assert(encryptedBuffer != NULL);
    // Current position is SN position
    uint32_t sequenceNumberPosition = encryptedBuffer->position;

    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    uint32_t decryptedTextLength = 0;
    SOPC_Buffer* plainBuffer = NULL;

    SOPC_Byte* dataToDecrypt = &(encryptedBuffer->data[sequenceNumberPosition]);
    uint32_t lengthToDecrypt = encryptedBuffer->length - sequenceNumberPosition;

    if (!isSymmetric)
    {
        if (scConnection->privateKey != NULL)
        {
            status = SOPC_CryptoProvider_AsymmetricGetLength_Decryption(
                scConnection->cryptoProvider, scConnection->privateKey, lengthToDecrypt, &decryptedTextLength);
            if (SOPC_STATUS_OK == status)
            {
                result = true;
            }
        }

        if (result && decryptedTextLength <= scConnection->tcpMsgProperties.receiveBufferSize)
        {
            // Allocate a new plain buffer of the size of the non encrypted length + decryptedTextLength
            plainBuffer = SOPC_Buffer_Create(sequenceNumberPosition + decryptedTextLength);
            if (NULL == plainBuffer)
            {
                result = false;
            }
            else
            {
                // Copy non encrypted data from original buffer to plain text buffer
                status = SOPC_Buffer_CopyWithLength(plainBuffer, encryptedBuffer, sequenceNumberPosition);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }
            if (result)
            {
                status = SOPC_CryptoProvider_AsymmetricDecrypt(scConnection->cryptoProvider, dataToDecrypt,
                                                               lengthToDecrypt, scConnection->privateKey,
                                                               &(plainBuffer->data[sequenceNumberPosition]),
                                                               decryptedTextLength, &decryptedTextLength, errorReason);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_Buffer_SetDataLength(plainBuffer, sequenceNumberPosition + decryptedTextLength);
                    assert(SOPC_STATUS_OK == status);
                    // Set position to sequence header
                    status = SOPC_Buffer_SetPosition(plainBuffer, sequenceNumberPosition);
                    assert(SOPC_STATUS_OK == status);
                }
                else
                {
                    result = false;
                }
            }
        }
    }
    else
    {
        SOPC_SC_SecurityKeySet* senderKeySet = NULL;
        SOPC_SC_SecurityKeySet* receiverKeySet = NULL;

        if (SC_Chunks_GetSecurityKeySets(scConnection, isPrevCryptoData, &senderKeySet, &receiverKeySet))
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CryptoProvider_SymmetricGetLength_Decryption(scConnection->cryptoProvider, lengthToDecrypt,
                                                                       &decryptedTextLength);
        }

        if (SOPC_STATUS_OK == status)
        {
            result = true;
        }

        if (result && decryptedTextLength <= scConnection->tcpMsgProperties.receiveBufferSize)
        {
            // Allocate a new plain buffer of the size of the non encrypted length + decryptedTextLength
            plainBuffer = SOPC_Buffer_Create(sequenceNumberPosition + decryptedTextLength);
            if (NULL == plainBuffer)
            {
                result = false;
            }
            else
            {
                // Copy non encrypted data from original buffer to plain text buffer
                status = SOPC_Buffer_CopyWithLength(plainBuffer, encryptedBuffer, sequenceNumberPosition);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }
            if (result)
            {
                status = SOPC_CryptoProvider_SymmetricDecrypt(
                    scConnection->cryptoProvider, dataToDecrypt, lengthToDecrypt, receiverKeySet->encryptKey,
                    receiverKeySet->initVector, &(plainBuffer->data[sequenceNumberPosition]), decryptedTextLength);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_Buffer_SetDataLength(plainBuffer, sequenceNumberPosition + decryptedTextLength);
                    assert(SOPC_STATUS_OK == status);
                    // Set position to sequence header
                    status = SOPC_Buffer_SetPosition(plainBuffer, sequenceNumberPosition);
                    assert(SOPC_STATUS_OK == status);
                }
                else
                {
                    result = false;
                }
            }
        }
    } // Symmetric algo branch

    if (!result)
    {
        // Clear all buffers
        SOPC_Buffer_Delete(scConnection->chunksCtx.currentChunkInputBuffer);
        scConnection->chunksCtx.currentChunkInputBuffer = NULL;

        if (plainBuffer != NULL)
        {
            SOPC_Buffer_Delete(plainBuffer);
        }
    }
    else
    {
        SOPC_Buffer_Delete(scConnection->chunksCtx.currentChunkInputBuffer);
        // Replace input buffer with the plain buffer (position == SN position)
        scConnection->chunksCtx.currentChunkInputBuffer = plainBuffer;
    }

    return result;
}

static bool SC_Chunks_VerifyMsgSignature(SOPC_SecureConnection* scConnection,
                                         bool isSymmetric,
                                         bool isPrevCryptoData,
                                         uint32_t* sigPosition,
                                         const char** errorReason)
{
    assert(scConnection != NULL);
    SOPC_Buffer* buffer = scConnection->chunksCtx.currentChunkInputBuffer;
    assert(buffer != NULL);

    bool result = false;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    uint32_t signatureSize = 0;
    uint32_t signaturePosition = 0;

    if (!isSymmetric)
    {
        SOPC_AsymmetricKey* publicKey = NULL;
        const SOPC_CertificateList* otherAppCertificate = NULL;
        SOPC_SecureChannel_Config* scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection);

        if (NULL == scConfig && scConnection->isServerConnection)
        {
            // Server side for new OPN: we have to use the temporary stored certificate value
            otherAppCertificate = scConnection->serverAsymmSecuInfo.clientCertificate;
        }
        else if (scConfig != NULL)
        {
            // Client side or Server side in case of OPN renew
            otherAppCertificate = SC_PeerCertificate(scConnection);
        }
        else
        {
            status = SOPC_STATUS_NOK;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(otherAppCertificate, &publicKey);
        }

        if (status == SOPC_STATUS_OK)
        {
            status = SOPC_CryptoProvider_AsymmetricGetLength_Signature(scConnection->cryptoProvider, publicKey,
                                                                       &signatureSize);
        }

        if (status == SOPC_STATUS_OK)
        {
            signaturePosition = buffer->length - signatureSize;

            status = SOPC_CryptoProvider_AsymmetricVerify(scConnection->cryptoProvider, buffer->data, signaturePosition,
                                                          publicKey, &(buffer->data[signaturePosition]), signatureSize,
                                                          errorReason);
        }

        SOPC_KeyManager_AsymmetricKey_Free(publicKey);
    }
    else
    {
        SOPC_SC_SecurityKeySet* senderKeySet = NULL;
        SOPC_SC_SecurityKeySet* receiverKeySet = NULL;

        if (SC_Chunks_GetSecurityKeySets(scConnection, isPrevCryptoData, &senderKeySet, &receiverKeySet))
        {
            status = SOPC_STATUS_OK;
        }
        else
        {
            status = SOPC_STATUS_INVALID_STATE;
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CryptoProvider_SymmetricGetLength_Signature(scConnection->cryptoProvider, &signatureSize);
        }

        if (status == SOPC_STATUS_OK)
        {
            signaturePosition = buffer->length - signatureSize;
            status = SOPC_CryptoProvider_SymmetricVerify(scConnection->cryptoProvider, buffer->data, signaturePosition,
                                                         receiverKeySet->signKey, &(buffer->data[signaturePosition]),
                                                         signatureSize);
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        *sigPosition = signaturePosition;
        result = true;
    }

    return result;
}

static bool SOPC_Remove_Padding(SOPC_SecureConnection* scConnection)
{
    uint32_t nbPaddingBytes = 0;
    SOPC_Buffer* buffer = scConnection->chunksCtx.currentChunkInputBuffer;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    if (scConnection->hasExtraPaddingSize)
    {
        // Last byte is the extraPaddingSize byte
        // (most significant byte of 2 bytes integer used to specify the padding size)
        nbPaddingBytes = (uint32_t) buffer->data[buffer->length - 1] << 8;
        // Remove the ExtraPaddinSize byte
        status = SOPC_Buffer_SetDataLength(buffer, buffer->length - 1);
    }

    if (SOPC_STATUS_OK == status)
    {
        nbPaddingBytes += buffer->data[buffer->length - 1];
        nbPaddingBytes++; // +1 for the PaddingSize byte which is not included

        // Remove all byte related to padding
        status = SOPC_Buffer_SetDataLength(buffer, buffer->length - nbPaddingBytes);
    }

    return SOPC_STATUS_OK == status;
}

static bool SC_Chunks_TreatMsgMultiChunks(SOPC_SecureConnection* scConnection, SOPC_StatusCode* errorStatus)
{
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    SOPC_SecureConnection_TcpProperties* tcpProperties = &scConnection->tcpMsgProperties;
    assert(SOPC_MSG_TYPE_SC_MSG == chunkCtx->currentMsgType);

    bool checkBodyMessageSize = false;
    bool addIntermediateChunk = false;
    bool mergeFinalChunk = false;
    bool clearIntermediateChunks = false;

    switch (chunkCtx->currentMsgIsFinal)
    {
    case SOPC_MSG_ISFINAL_INTERMEDIATE:
        // Note: number of intermediate chunks already checked by SC_Chunks_CheckMultiChunkContext
        checkBodyMessageSize = true;
        addIntermediateChunk = true;
        break;
    case SOPC_MSG_ISFINAL_FINAL:
        checkBodyMessageSize = true;
        mergeFinalChunk = true;
        break;
    case SOPC_MSG_ISFINAL_ABORT:
        clearIntermediateChunks = true;
        mergeFinalChunk = true; // Abort is a final chunk but intermediate chunks shall be cleared before merge !
        break;
    default:
        assert(false);
    }

    uint32_t totalSize = 0;
    if (checkBodyMessageSize)
    {
        // Check message size (total of chunks size)

        if (SOPC_ScInternalContext_GetNbIntermediateInputChunks(chunkCtx) > 0)
        {
            SOPC_SLinkedListIterator it = SOPC_SLinkedList_GetIterator(chunkCtx->intermediateChunksInputBuffers);
            while (SOPC_SLinkedList_HasNext(&it))
            {
                totalSize += SOPC_Buffer_Remaining((SOPC_Buffer*) SOPC_SLinkedList_Next(&it));
            }
        }
        totalSize += SOPC_Buffer_Remaining(chunkCtx->currentChunkInputBuffer);
        if (totalSize > tcpProperties->receiveMaxMessageSize)
        {
            *errorStatus = OpcUa_BadTcpMessageTooLarge;
            return false;
        }
    }

    if (addIntermediateChunk)
    {
        // Add an intermediate chunk and wait for next chunk

        if (!SOPC_ScInternalContext_AddIntermediateInputChunk(tcpProperties, chunkCtx,
                                                              chunkCtx->currentChunkInputBuffer))
        {
            *errorStatus = OpcUa_BadOutOfMemory;
            return false;
        }
        /* currentChunkInputBuffer is lent to the linked list, which will free it */
        chunkCtx->currentChunkInputBuffer = NULL;
        // No current chunk or message context kept
        SOPC_ScInternalContext_ClearCurrentInputChunkContext(chunkCtx);
    }

    if (clearIntermediateChunks)
    {
        // Clear intermediate chunks in case of final abort chunk (containing associated error in current chunk)
        SOPC_ScInternalContext_ClearIntermediateInputChunks(chunkCtx);
    }

    if (mergeFinalChunk)
    {
        SOPC_Buffer* mergedBuffer = NULL;
        if (SOPC_ScInternalContext_GetNbIntermediateInputChunks(chunkCtx) > 0)
        {
            // Merge several unencrypted chunks into one buffer containing complete message.

            assert(totalSize > 0); // Ensure size was computed
            uint32_t remaining = 0;
            bool result = false;
            mergedBuffer = SOPC_Buffer_Create(totalSize);
            if (NULL == mergedBuffer)
            {
                *errorStatus = OpcUa_BadOutOfMemory;
                return false;
            }

            SOPC_Buffer* bufferToMerge = SOPC_SLinkedList_PopHead(chunkCtx->intermediateChunksInputBuffers);
            while (NULL != bufferToMerge)
            {
                result = fill_buffer(mergedBuffer, bufferToMerge, totalSize, &remaining);
                assert(result);
                assert(0 == SOPC_Buffer_Remaining(bufferToMerge));
                SOPC_Buffer_Delete(bufferToMerge);
                bufferToMerge = SOPC_SLinkedList_PopHead(chunkCtx->intermediateChunksInputBuffers);
            }
            result = fill_buffer(mergedBuffer, chunkCtx->currentChunkInputBuffer, totalSize, &remaining);
            assert(result);
            assert(0 == remaining);
            assert(0 == SOPC_Buffer_Remaining(chunkCtx->currentChunkInputBuffer));
            SOPC_Buffer_Delete(chunkCtx->currentChunkInputBuffer);
            chunkCtx->currentChunkInputBuffer = NULL;
        }
        else
        {
            // No merge to do if only one final chunk received.

            mergedBuffer = chunkCtx->currentChunkInputBuffer;
            chunkCtx->currentChunkInputBuffer = NULL;
        }

        // Set merged buffer as the current message buffer
        chunkCtx->currentMessageInputBuffer = mergedBuffer;
    }

    assert(NULL == chunkCtx->currentChunkInputBuffer);
    return true;
}

bool SC_Chunks_TreatTcpPayload(SOPC_SecureConnection* scConnection,
                               uint32_t* requestIdOrHandle,
                               bool* ignoreExpiredMessage,
                               SOPC_StatusCode* errorStatus)
{
    assert(requestIdOrHandle != NULL);
    assert(ignoreExpiredMessage != NULL);
    *ignoreExpiredMessage = false; // default value

    bool result = true;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;

    bool asymmSecuHeader = false;
    bool symmSecuHeader = false;
    bool sequenceHeader = false;
    bool hasSecureChannelId = false;
    bool isOPN = false;

    uint32_t secureChannelId = 0;

    bool toDecrypt = false;
    bool toCheckSignature = false;
    bool isPrevCryptoData = false;

    SOPC_SecureChannel_Config* scConfig = NULL;

    uint32_t signaturePosition = 0;
    const char* errorReason = "";

    // Note: for non secure message we already check those messages are expected
    //       regarding the connection type (client/server)
    switch (chunkCtx->currentMsgType)
    {
    case SOPC_MSG_TYPE_HEL:
        if (!scConnection->isServerConnection || chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            // A client shall not receive a HELLO message
            // or HELLO message chunk shall be final
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: invalid or unexpected HEL message received (epCfgIdx=%" PRIu32
                                   ", scCfgIdx=%" PRIu32 ")",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
        }
        // Nothing to do: whole payload to transmit to the secure connection state manager
        break;
    case SOPC_MSG_TYPE_ACK:
        if (scConnection->isServerConnection || chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            // A server shall not receive an ACK message
            // or ACK message chunk shall be final
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: invalid or unexpected ACK message received (epCfgIdx=%" PRIu32
                                   ", scCfgIdx=%" PRIu32 ")",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
        }
        // Nothing to do: whole payload to transmit to the secure connection state manager
        break;
    case SOPC_MSG_TYPE_ERR:
        if (scConnection->isServerConnection || chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            // A server shall not receive an ERROR message
            // or ERR message chunk shall be final
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: invalid or unexpected ERR message received (epCfgIdx=%" PRIu32
                                   ", scCfgIdx=%" PRIu32 ")",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
        }
        // Nothing to do: whole payload to transmit to the secure connection state manager
        break;
    case SOPC_MSG_TYPE_SC_OPN:
        if (chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            // OPN message chunk shall be final
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: invalid OPN message received (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32
                                   ")",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
        }
        else
        {
            isOPN = true;
            hasSecureChannelId = true;
            asymmSecuHeader = true;
            sequenceHeader = true;
        }
        break;
    case SOPC_MSG_TYPE_SC_CLO:
        if (!scConnection->isServerConnection || chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: invalid or unexpected CLO message received (epCfgIdx=%" PRIu32
                                   ", scCfgIdx=%" PRIu32 ")",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
        }
        else
        {
            hasSecureChannelId = true;
            symmSecuHeader = true;
            sequenceHeader = true;
        }
        break;
    case SOPC_MSG_TYPE_SC_MSG:
        hasSecureChannelId = true;
        symmSecuHeader = true;
        sequenceHeader = true;
        break;
    default:
        assert(false);
    }

    if (result && hasSecureChannelId)
    {
        // Decode secure channel id
        result = (SOPC_UInt32_Read(&secureChannelId, chunkCtx->currentChunkInputBuffer, 0) == SOPC_STATUS_OK);
    }

    if (result && hasSecureChannelId)
    {
        if (isOPN)
        {
            if (0 == scConnection->currentSecurityToken.secureChannelId)
            {
                /* It is a new secure channel (security token not defined => new OPN), value shall be:
                 * - 0 from client (server case)
                 * - not 0 from server (client case)
                 */
                if (!scConnection->isServerConnection)
                {
                    // Client side
                    if (secureChannelId != 0)
                    {
                        /* Store the secure channel Id provided as temporary context to be checked in case of new OPN
                         * (ISSUE) response. It shall be checked when decoding the OPN response in SC state manager
                         * transition to ScConnected state.
                         */
                        scConnection->clientSecureChannelId = secureChannelId;
                    }
                    else
                    {
                        // The server shall always provide a secure channel Id != 0
                        result = false;
                        *errorStatus = OpcUa_BadSecurityChecksFailed;

                        SOPC_Logger_TraceError(
                            SOPC_LOG_MODULE_CLIENTSERVER,
                            "ChunksMgr: server provided invalid initial secure channel Id 0 (epCfgIdx=%" PRIu32
                            ", scCfgIdx=%" PRIu32 ")",
                            scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
                    }
                }
                else
                {
                    // Server side
                    if (secureChannelId != 0)
                    {
                        // The client shall not provide a secure channel Id != 0 on new OPN
                        /*
                         * Note: the specification 1.03 part 6 §6.7.6 does not indicate clearly that client shall send
                         * 0 but it is due to the fact it does not differentiate client/server case. It seems reasonable
                         * to guarantee it anyway: "This value may be 0 if the Message is an OpenSecureChannel request."
                         */
                        result = false;
                        *errorStatus = OpcUa_BadSecurityChecksFailed;

                        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                               "ChunksMgr: client provided invalid initial secure channel Id=%" PRIu32
                                               " (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                               secureChannelId, scConnection->serverEndpointConfigIdx,
                                               scConnection->endpointConnectionConfigIdx);
                    } // else: secure channel Id == 0 from client expected, then new Id set by server in OPN response
                }
            }
            else if (scConnection->currentSecurityToken.secureChannelId != secureChannelId)
            {
                // Error: it shall be the expected secure channel Id
                result = false;
                *errorStatus = OpcUa_BadTcpSecureChannelUnknown;

                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr: invalid secure channel Id=%" PRIu32 " expected Id=%" PRIu32
                                       " (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                       secureChannelId, scConnection->currentSecurityToken.secureChannelId,
                                       scConnection->serverEndpointConfigIdx,
                                       scConnection->endpointConnectionConfigIdx);
            }
        }
        else
        {
            if (scConnection->currentSecurityToken.secureChannelId != secureChannelId)
            {
                // Error: it shall be the expected secure channel Id when not an OPN
                result = false;
                *errorStatus = OpcUa_BadTcpSecureChannelUnknown;

                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr: invalid secure channel Id=%" PRIu32 " expected Id=%" PRIu32
                                       " (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                       secureChannelId, scConnection->currentSecurityToken.secureChannelId,
                                       scConnection->serverEndpointConfigIdx,
                                       scConnection->endpointConnectionConfigIdx);
            }
        }
    }

    if (result && asymmSecuHeader)
    {
        // OPN case: asymmetric secu header
        bool isSecurityActive = false;
        result = SC_Chunks_CheckAsymmetricSecurityHeader(scConnection, &isSecurityActive, errorStatus);
        if (result)
        {
            toDecrypt = isSecurityActive;
            toCheckSignature = isSecurityActive;
            isPrevCryptoData = false; // asymmetric => unused parameter in decrypt / check sign
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: asymmetric security header verification failed (epCfgIdx=%" PRIu32
                                   ", scCfgIdx=%" PRIu32 ")",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
        }
    }

    if (result && symmSecuHeader)
    {
        // CLO or MSG case: symmetric security header
        if (!scConnection->isServerConnection)
        {
            scConfig = SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
        }
        else
        {
            scConfig = SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
        }
        // If a symmetric message is received, secure channel should be configured
        if (NULL == scConfig)
        {
            result = false;
            *errorStatus = OpcUa_BadSecurityChecksFailed;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: SC configuration not found (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
        }
        else
        {
            result = SC_Chunks_CheckSymmetricSecurityHeader(scConnection, &isPrevCryptoData, errorStatus);
            if (result)
            {
                toDecrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, isOPN);
                toCheckSignature = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr: symmetric security header verification failed (epCfgIdx=%" PRIu32
                                       ", scCfgIdx=%" PRIu32 ")",
                                       scConnection->serverEndpointConfigIdx,
                                       scConnection->endpointConnectionConfigIdx);
            }
        }
    }

    if (result && toDecrypt)
    {
        // Decrypt the message
        result = SC_Chunks_DecryptMsg(scConnection,
                                      false == isOPN, // isSymmetric
                                      isPrevCryptoData, &errorReason);
        if (!result)
        {
            *errorStatus = OpcUa_BadSecurityChecksFailed;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: decryption failed (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 "): %s",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx,
                                   errorReason);
        }
    }

    if (result && toCheckSignature)
    {
        // Check decrypted message signature
        result = SC_Chunks_VerifyMsgSignature(scConnection,
                                              false == isOPN, // isSymmetric
                                              isPrevCryptoData, &signaturePosition, &errorReason);
        if (result)
        {
            // Set signature bytes as unreadable in the buffer (signature uses last bytes)
            SOPC_ReturnStatus status =
                SOPC_Buffer_SetDataLength(scConnection->chunksCtx.currentChunkInputBuffer, signaturePosition);
            assert(SOPC_STATUS_OK == status);
        }
        else
        {
            *errorStatus = OpcUa_BadSecurityChecksFailed;

            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ChunksMgr: signature verification failed (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 "): %s",
                scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx, errorReason);
        }
    }

    if (result && sequenceHeader)
    {
        result = SC_Chunks_CheckSequenceHeaderSN(scConnection, isOPN, errorStatus);

        if (result)
        {
            result = SC_Chunks_CheckSequenceHeaderRequestId(scConnection,
                                                            false == scConnection->isServerConnection, // isClient
                                                            chunkCtx->currentMsgIsFinal, chunkCtx->currentMsgType,
                                                            requestIdOrHandle, ignoreExpiredMessage, errorStatus);
            if (!result)
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr: request Id/Handle=%" PRIu32
                                       " (or associated type) verification failed (epCfgIdx=%" PRIu32
                                       ", scCfgIdx=%" PRIu32 ")",
                                       *requestIdOrHandle, scConnection->serverEndpointConfigIdx,
                                       scConnection->endpointConnectionConfigIdx);
            }
        }
        else
        {
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: SN verification failed (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
        }
    }

    if (result && toDecrypt)
    {
        // Set the padding bytes as unreadable bytes in the buffer
        result = SOPC_Remove_Padding(scConnection);
        if (!result)
        {
            *errorStatus = OpcUa_BadDecodingError;
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: padding removal failed (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                   scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);
        }
    }

    // Once security header, encryption and signature is treated we have to deal with multi-chunks aspect
    if (result)
    {
        if (SOPC_MSG_TYPE_SC_MSG == chunkCtx->currentMsgType)
        {
            result = SC_Chunks_TreatMsgMultiChunks(scConnection, errorStatus);
        }
        else
        {
            // Single chunk, move it as complete message buffer
            chunkCtx->currentMessageInputBuffer = chunkCtx->currentChunkInputBuffer;
            chunkCtx->currentChunkInputBuffer = NULL;
        }
    }

    return result;
}

// Returns true if a message was decoded (false if there was not enough data).
// Any potential error is stored in *error.
bool SC_Chunks_DecodeReceivedBuffer(SOPC_SecureConnection_ChunkMgrCtx* ctx,
                                    SOPC_Buffer* receivedBuffer,
                                    SOPC_StatusCode* error)
{
    assert(ctx != NULL);
    assert(receivedBuffer != NULL);
    assert(error != NULL);

    uint32_t remaining = 0;
    *error = SOPC_GoodGenericStatus;

    // Header decoding
    if (ctx->currentMsgSize == 0)
    {
        if (!fill_buffer(ctx->currentChunkInputBuffer, receivedBuffer, SOPC_TCP_UA_HEADER_LENGTH, &remaining))
        {
            *error = OpcUa_BadTcpInternalError;
            return false;
        }

        if ((remaining > 0) || !SC_Chunks_DecodeTcpMsgHeader(ctx, error))
        {
            return false;
        }
    }

    // Payload decoding
    assert(ctx->currentMsgSize > 0); // message size was decoded
    assert(ctx->currentMsgType != SOPC_MSG_TYPE_INVALID);
    assert(ctx->currentMsgIsFinal != SOPC_MSG_ISFINAL_INVALID);

    if (!fill_buffer(ctx->currentChunkInputBuffer, receivedBuffer, ctx->currentMsgSize - SOPC_TCP_UA_HEADER_LENGTH,
                     &remaining))
    {
        *error = OpcUa_BadTcpInternalError;
        return false;
    }

    return (remaining == 0);
}

static bool SC_Chunks_CheckMultiChunkContext(SOPC_SecureConnection_ChunkMgrCtx* ctx,
                                             SOPC_SecureConnection_TcpProperties* tcpProperties,
                                             SOPC_StatusCode* error)
{
    /* Check if number of chunks received is compatible with configured number of chunks accepted.
     * Valid cases are:
     * - Chunk is final ('F' or 'A')
     * - Chunk is intermediate and no limit number of chunks is defined
     * - Chunk is intermediate and number of chunks already received (previously + current) is strictly less than the
     *   maximum number of chunks.
     *   Note: a final chunk is still expected and will increase by 1 the final number of chunks.
     */
    if (SOPC_MSG_ISFINAL_INTERMEDIATE == ctx->currentMsgIsFinal && tcpProperties->receiveMaxChunkCount != 0 &&
        SOPC_ScInternalContext_GetNbIntermediateInputChunks(ctx) + 1 >= tcpProperties->receiveMaxChunkCount)
    {
        // Too many intermediate chunks received
        *error = OpcUa_BadTcpMessageTooLarge;
        return false;
    }
    return true;
}

static void SC_Chunks_TreatReceivedBuffer(SOPC_SecureConnection* scConnection,
                                          uint32_t scConnectionIdx,
                                          SOPC_Buffer* receivedBuffer)
{
    bool result = true;
    assert(scConnection != NULL);
    assert(receivedBuffer != NULL);
    assert(receivedBuffer->position == 0);

    SOPC_StatusCode errorStatus = SOPC_GoodGenericStatus; // Good
    uint32_t requestIdOrHandle = 0;
    bool ignoreExpiredMessage = false; // Set to true if message is response to expired request
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;

    // Continue until an error occurred OR received buffer is empty (could contain 1 or several messages)
    while (result && SOPC_Buffer_Remaining(receivedBuffer) > 0)
    {
        if (NULL == chunkCtx->currentChunkInputBuffer)
        {
            // No incomplete message data: create a new buffer
            chunkCtx->currentChunkInputBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.receiveBufferSize);
            if (NULL == chunkCtx->currentChunkInputBuffer)
            {
                errorStatus = OpcUa_BadOutOfMemory;
                // TREATMENT STOPPED HERE
                break;
            }
        }

        if (!SC_Chunks_DecodeReceivedBuffer(chunkCtx, receivedBuffer, &errorStatus))
        {
            // Note: if false is returned but no error status is set it only means there is not enough data
            if (errorStatus != SOPC_GoodGenericStatus)
            {
                result = false;
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr: TCP UA header decoding failed with statusCode=%" PRIX32
                                       " (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                       errorStatus, scConnection->serverEndpointConfigIdx,
                                       scConnection->endpointConnectionConfigIdx);
            }
            // TREATMENT STOPPED HERE
            break;
        }

        SOPC_Logger_TraceDebug(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "ChunksMgr: received TCP UA message type SOPC_Msg_Type=%u SOPC_Msg_IsFinal=%u (epCfgIdx=%" PRIu32
            ", scCfgIdx=%" PRIu32 ")",
            chunkCtx->currentMsgType, chunkCtx->currentMsgIsFinal, scConnection->serverEndpointConfigIdx,
            scConnection->endpointConnectionConfigIdx);

        // Decode OPC UA Secure Conversation MessageChunk specific headers if necessary (not HEL/ACK/ERR)
        if (SC_Chunks_CheckMultiChunkContext(chunkCtx, &scConnection->tcpMsgProperties, &errorStatus) &&
            SC_Chunks_TreatTcpPayload(scConnection, &requestIdOrHandle, &ignoreExpiredMessage, &errorStatus))
        {
            // Current chunk shall have been moved into intermediate chunk buffers or into complete message buffer
            assert(NULL == chunkCtx->currentChunkInputBuffer);
            if (NULL != chunkCtx->currentMessageInputBuffer)
            {
                if (!ignoreExpiredMessage)
                {
                    // Transmit OPC UA message to secure connection state manager
                    SOPC_SecureChannels_InternalEvent scEvent =
                        SC_Chunks_MsgTypeToRcvEvent(chunkCtx->currentMsgType, chunkCtx->currentMsgIsFinal);
                    if (scEvent == INT_SC_RCV_ERR || scEvent == INT_SC_RCV_CLO)
                    {
                        // Treat as prio events
                        SOPC_SecureChannels_EnqueueInternalEventAsNext(scEvent, scConnectionIdx,
                                                                       (uintptr_t) chunkCtx->currentMessageInputBuffer,
                                                                       requestIdOrHandle);
                    }
                    else
                    {
                        SOPC_SecureChannels_EnqueueInternalEvent(scEvent, scConnectionIdx,
                                                                 (uintptr_t) chunkCtx->currentMessageInputBuffer,
                                                                 requestIdOrHandle);
                    }
                    /* currentMessageInputBuffer is lent to the secure channel, which will free it */
                    chunkCtx->currentMessageInputBuffer = NULL;
                    SOPC_ScInternalContext_ClearInputChunksContext(chunkCtx);
                }
                else
                {
                    SOPC_Logger_TraceInfo(SOPC_LOG_MODULE_CLIENTSERVER,
                                          "ChunksMgr: ignored response of expired request with requestHandle=%" PRIu32
                                          " (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                          requestIdOrHandle, scConnection->serverEndpointConfigIdx,
                                          scConnection->endpointConnectionConfigIdx);

                    // Message shall be ignored since it is response to an expired request
                    SOPC_Buffer_Delete(chunkCtx->currentMessageInputBuffer);
                    chunkCtx->currentMessageInputBuffer = NULL;
                    SOPC_ScInternalContext_ClearInputChunksContext(chunkCtx);
                }
            }
        }
        else
        {
            result = false;
        }
    }

    if (!result)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "ChunksMgr: raised INT_SC_RCV_FAILURE: %" PRIX32 ": (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
            errorStatus, scConnection->serverEndpointConfigIdx, scConnection->endpointConnectionConfigIdx);

        // Treat as prio events
        SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_RCV_FAILURE, scConnectionIdx, (uintptr_t) NULL,
                                                       errorStatus);
        SOPC_ScInternalContext_ClearInputChunksContext(chunkCtx);
    }

    SOPC_Buffer_Delete(receivedBuffer);
}

static bool SC_Chunks_EncodeTcpMsgHeader(uint32_t scConnectionIdx,
                                         SOPC_SecureConnection* scConnection,
                                         SOPC_Msg_Type sendMsgType,
                                         uint8_t isFinalChar,
                                         SOPC_Buffer* buffer,
                                         SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(buffer != NULL);
    bool result = false;
    const uint8_t* msgTypeBytes = NULL;
    uint32_t messageSize = 0; // Could be temporary depending on message type / secu parameters
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    switch (sendMsgType)
    {
    case SOPC_MSG_TYPE_HEL:
        msgTypeBytes = SOPC_HEL;
        result = SOPC_UA_FINAL_CHUNK == isFinalChar;
        break;
    case SOPC_MSG_TYPE_ACK:
        msgTypeBytes = SOPC_ACK;
        result = SOPC_UA_FINAL_CHUNK == isFinalChar;
        break;
    case SOPC_MSG_TYPE_ERR:
        msgTypeBytes = SOPC_ERR;
        result = SOPC_UA_FINAL_CHUNK == isFinalChar;
        break;
    case SOPC_MSG_TYPE_RHE:
        msgTypeBytes = SOPC_RHE;
        result = SOPC_UA_FINAL_CHUNK == isFinalChar;
        break;
    case SOPC_MSG_TYPE_SC_OPN:
        msgTypeBytes = SOPC_OPN;
        result = SOPC_UA_FINAL_CHUNK == isFinalChar;
        break;
    case SOPC_MSG_TYPE_SC_CLO:
        msgTypeBytes = SOPC_CLO;
        result = SOPC_UA_FINAL_CHUNK == isFinalChar;
        break;
    case SOPC_MSG_TYPE_SC_MSG:
        msgTypeBytes = SOPC_MSG;
        result = true;
        break;
    default:
        assert(false);
    }

    if (result)
    {
        status = SOPC_Buffer_Write(buffer, msgTypeBytes, 3);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }
    if (result)
    {
        status = SOPC_Buffer_Write(buffer, &isFinalChar, 1);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }
    if (result)
    {
        if (buffer->length >= SOPC_TCP_UA_HEADER_LENGTH)
        {
            messageSize = buffer->length;
        }
        else
        {
            messageSize = SOPC_TCP_UA_HEADER_LENGTH;
        }
        status = SOPC_UInt32_Write(&messageSize, buffer, 0);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }

    if (!result)
    {
        *errorStatus = OpcUa_BadEncodingError;

        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "ChunksMgr: treat send buffer: failed to encode message header (msgType=%u, scIdx=%" PRIu32
            ", scCfgIdx=%" PRIu32 ")",
            sendMsgType, scConnectionIdx, scConnection->endpointConnectionConfigIdx);
    }

    return result;
}

static bool SC_Chunks_EncodeAsymSecurityHeader(uint32_t scConnectionIdx,
                                               SOPC_SecureConnection* scConnection,
                                               SOPC_SecureChannel_Config* scConfig,
                                               SOPC_Buffer* buffer,
                                               int32_t* securityPolicyLength,
                                               int32_t* senderCertificateSize,
                                               SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConnection->cryptoProvider != NULL);
    assert(scConfig != NULL);
    assert(scConfig->reqSecuPolicyUri != NULL);
    assert(buffer != NULL);
    assert(senderCertificateSize != NULL);
    bool result = true;
    bool toEncrypt = true;
    bool toSign = true;
    SOPC_String strSecuPolicy;
    SOPC_String_Initialize(&strSecuPolicy);
    SOPC_ByteString bsSenderCert;
    SOPC_ByteString_Initialize(&bsSenderCert);
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    toEncrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, true);
    toSign = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);

    // Security Policy:
    status = SOPC_String_CopyFromCString(&strSecuPolicy, scConfig->reqSecuPolicyUri);
    if (SOPC_STATUS_OK != status || strSecuPolicy.Length <= 0)
    {
        result = false;
        *errorStatus = OpcUa_BadTcpInternalError;
    }
    else
    {
        *securityPolicyLength = strSecuPolicy.Length;
    }

    if (result)
    {
        status = SOPC_String_Write(&strSecuPolicy, buffer, 0);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }
    }

    // Sender Certificate:
    if (result)
    {
        uint32_t length = 0;
        const SOPC_CertificateList* senderCert = SC_OwnCertificate(scConnection);

        if (senderCert != NULL)
        {
            status = SOPC_KeyManager_Certificate_ToDER(senderCert, &bsSenderCert.Data, &length);
            if (SOPC_STATUS_OK == status && length <= INT32_MAX)
            {
                bsSenderCert.Length = (int32_t) length;
                *senderCertificateSize = (int32_t) length;
            }
            else
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
        }
    }
    if (result)
    {
        // Note: part 6 v1.03 table 27: This field shall be null if the Message is not signed
        if (toSign)
        {
            if (bsSenderCert.Length > 0)
            {
                status = SOPC_STATUS_OK;
            }
            else
            {
                status = SOPC_STATUS_INVALID_PARAMETERS;
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr: SC_Chunks_EncodeAsymSecurityHeader: sender certificate "
                                       "missing in Sign mode (scIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                       scConnectionIdx, scConnection->endpointConnectionConfigIdx);
            }
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_ByteString_Write(&bsSenderCert, buffer, 0);
            }
            if (SOPC_STATUS_OK != status)
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
        }
        else
        {
            // Note: foundation stack expects -1 value whereas 0 is also valid:
            const int32_t minusOne = -1;
            status = SOPC_Int32_Write(&minusOne, buffer, 0);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
            // NULL string: nothing to write
        }
    }

    // Receiver Certificate Thumbprint:
    if (result)
    {
        const SOPC_CertificateList* receiverCertCrypto = SC_PeerCertificate(scConnection);

        // Note: part 6 v1.03 table 27: This field shall be null if the Message is not encrypted
        if (toEncrypt && receiverCertCrypto != NULL)
        {
            SOPC_ByteString recCertThumbprint;
            SOPC_ByteString_Initialize(&recCertThumbprint);
            uint32_t thumbprintLength = 0;
            status =
                SOPC_CryptoProvider_CertificateGetLength_Thumbprint(scConnection->cryptoProvider, &thumbprintLength);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }

            if (result)
            {
                status = SOPC_ByteString_InitializeFixedSize(&recCertThumbprint, thumbprintLength);
                if (thumbprintLength <= INT32_MAX && SOPC_STATUS_OK == status)
                {
                    // OK
                }
                else
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
            if (result)
            {
                status = SOPC_KeyManager_Certificate_GetThumbprint(scConnection->cryptoProvider, receiverCertCrypto,
                                                                   recCertThumbprint.Data, thumbprintLength);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
            if (result)
            {
                status = SOPC_ByteString_Write(&recCertThumbprint, buffer, 0);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
            SOPC_ByteString_Clear(&recCertThumbprint);
        }
        else if (!toEncrypt)
        {
            // Note: foundation stack expects -1 value whereas 0 is also valid:
            const int32_t minusOne = -1;
            status = SOPC_Int32_Write(&minusOne, buffer, 0);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
            // NULL string: nothing to write
        }
        else
        {
            // Certificate shall be defined in configuration if necessary (configuration constraint)
            assert(false);
        }
    }

    SOPC_String_Clear(&strSecuPolicy);
    SOPC_ByteString_Clear(&bsSenderCert);

    if (!result)
    {
        SOPC_Logger_TraceError(
            SOPC_LOG_MODULE_CLIENTSERVER,
            "ChunksMgr: treat send buffer: failed to encode asymmetric security header (scIdx=%" PRIu32
            ", scCfgIdx=%" PRIu32 ")",
            scConnectionIdx, scConnection->endpointConnectionConfigIdx);
    }

    return result;
}

static bool SC_Chunks_Is_ExtraPaddingSizePresent(uint32_t plainTextBlockSize)
{
    // Extra-padding necessary if padding could be greater 256 bytes (2048 bits)
    // (1 byte for padding size field + 255 bytes of padding).
    // => padding max value is plainTextBlockSize regarding the formula of padding size
    //    (whereas spec part 6 indicates it depends on the key size which is incorrect)
    if (plainTextBlockSize > 256)
    {
        return true;
    }
    return false;
}

static uint32_t SC_Chunks_ComputeMaxBodySize(uint32_t nonEncryptedHeadersSize,
                                             uint32_t chunkSize,
                                             bool toEncrypt,
                                             uint32_t cipherBlockSize,
                                             uint32_t plainBlockSize,
                                             bool toSign,
                                             uint32_t signatureSize,
                                             bool* hasExtraPaddingSize)
{
    uint32_t result = 0;
    uint32_t paddingSizeFields = 0;
    *hasExtraPaddingSize = false;
    if (!toEncrypt)
    {
        // No encryption => consider same size blocks and no padding size fields
        cipherBlockSize = 1;
        plainBlockSize = 1;
        paddingSizeFields = 0;
    }
    else
    {
        // By default only 1 byte for padding size field. +1 if extra padding
        paddingSizeFields = 1;
        if (SC_Chunks_Is_ExtraPaddingSizePresent(plainBlockSize))
        {
            *hasExtraPaddingSize = true;
            paddingSizeFields += 1;
        }
    }

    if (!toSign)
    {
        signatureSize = 0;
    }

    // Ensure cipher block size is greater or equal to plain block size:
    //  otherwise the plain size could be greater than the  buffer size regarding computation
    assert(cipherBlockSize >= plainBlockSize);

    /*
     * Use formulae of spec 1.03.6 errata (see mantis ticket #2897):
     * MaxBodySize = PlainTextBlockSize * Floor((MessageChunkSize - HeaderSize) / CipherTextBlockSize)
     *                - SequenceHeaderSize - SignatureSize - PaddingByteSize;
     *
     */
    result = plainBlockSize * ((chunkSize - nonEncryptedHeadersSize) / cipherBlockSize) -
             SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH - signatureSize - paddingSizeFields;

    // Maximum body size (+headers+signature+padding size fields) cannot be greater than maximum buffer size
    assert(chunkSize >= (nonEncryptedHeadersSize + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH + result + signatureSize +
                         paddingSizeFields));

    return result;
}

static bool SC_Chunks_GetSendingCryptoSizes(SOPC_SecureConnection* scConnection,
                                            SOPC_SecureChannel_Config* scConfig,
                                            bool isSymmetricAlgo,
                                            bool* toEncrypt,
                                            bool* toSign,
                                            uint32_t* signatureSize,
                                            uint32_t* cipherTextBlockSize,
                                            uint32_t* plainTextBlockSize)
{
    assert(scConnection != NULL);
    assert(scConfig != NULL);
    assert(toEncrypt != NULL);
    assert(toSign != NULL);
    assert(signatureSize != NULL);
    assert(cipherTextBlockSize != NULL);
    assert(plainTextBlockSize != NULL);
    bool result = true;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!isSymmetricAlgo)
    {
        // ASYMMETRIC CASE

        if (scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None)
        {
            SOPC_AsymmetricKey* receiverPublicKey = NULL;
            SOPC_AsymmetricKey* senderPublicKey = NULL;
            const SOPC_CertificateList* receiverAppCertificate = SC_PeerCertificate(scConnection);
            const SOPC_CertificateList* senderAppCertificate = SC_OwnCertificate(scConnection);

            // Asymmetric case: used only for opening channel, signature AND encryption mandatory in this case
            *toEncrypt = true;
            *toSign = true;

            status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(senderAppCertificate, &senderPublicKey);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
            }

            if (result)
            {
                status =
                    SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(receiverAppCertificate, &receiverPublicKey);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }

            if (result)
            {
                // Compute block sizes (using receiver key => encryption with other application public key)
                status = SOPC_CryptoProvider_AsymmetricGetLength_Msgs(scConnection->cryptoProvider, receiverPublicKey,
                                                                      cipherTextBlockSize, plainTextBlockSize);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }
            if (result)
            {
                // Compute signature size (using sender key => signature with sender application private key)
                status = SOPC_CryptoProvider_AsymmetricGetLength_Signature(scConnection->cryptoProvider,
                                                                           senderPublicKey, signatureSize);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }

            SOPC_KeyManager_AsymmetricKey_Free(senderPublicKey);
            SOPC_KeyManager_AsymmetricKey_Free(receiverPublicKey);
        }
        else
        {
            *toEncrypt = false; // No data encryption
            *toSign = false;    // No signature
        }
    }
    else
    {
        // SYMMETRIC CASE

        if (scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None)
        {
            if (scConfig->msgSecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt)
            {
                // Encryption necessary: compute block sizes
                *toEncrypt = true;

                status = SOPC_CryptoProvider_SymmetricGetLength_Blocks(scConnection->cryptoProvider,
                                                                       cipherTextBlockSize, plainTextBlockSize);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }
            else
            {
                *toEncrypt = false;
            }

            if (result)
            {
                // Signature necessary in both Sign and SignAndEncrypt cases: compute signature size
                *toSign = true;
                status = SOPC_CryptoProvider_SymmetricGetLength_Signature(scConnection->cryptoProvider, signatureSize);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }
        }
        else
        {
            // No signature or encryption
            *toEncrypt = false;
            *toSign = false;
        }
    }

    return result;
}

static uint32_t SC_Chunks_GetSendingMaxBodySize(SOPC_SecureConnection* scConnection,
                                                SOPC_SecureChannel_Config* scConfig,
                                                uint32_t chunkSize,
                                                uint32_t nonEncryptedHeadersSize,
                                                bool isSymmetric,
                                                bool* hasExtraPaddingSize)
{
    assert(scConnection != NULL);
    assert(scConfig != NULL);
    uint32_t maxBodySize = 0;
    bool result = true;

    bool toEncrypt = false;
    uint32_t cipherBlockSize = 0;
    uint32_t plainBlockSize = 0;
    bool toSign = false;
    uint32_t signatureSize = 0;

    result = SC_Chunks_GetSendingCryptoSizes(scConnection, scConfig, isSymmetric, &toEncrypt, &toSign, &signatureSize,
                                             &cipherBlockSize, &plainBlockSize);

    // Compute the max body size regarding encryption and signature use
    if (result)
    {
        maxBodySize = SC_Chunks_ComputeMaxBodySize(nonEncryptedHeadersSize, chunkSize, toEncrypt, cipherBlockSize,
                                                   plainBlockSize, toSign, signatureSize, hasExtraPaddingSize);
    }

    return maxBodySize;
}

static uint16_t SC_Chunks_GetPaddingSize(
    uint32_t bytesToEncrypt, // called bytesToWrite in spec part 6 but it should not since it includes SN header !
    uint32_t plainBlockSize,
    uint32_t signatureSize)
{
    uint32_t lresult = 0;
    uint16_t result = 0;
    // By default only 1 padding size field + 1 if extra padding
    uint8_t paddingSizeFields = 1;
    if (SC_Chunks_Is_ExtraPaddingSizePresent(plainBlockSize))
    {
        paddingSizeFields++;
    }
    uint32_t missingBytesForBlockMultiple = (bytesToEncrypt + signatureSize + paddingSizeFields) % plainBlockSize;
    if (0 != missingBytesForBlockMultiple)
    {
        lresult = plainBlockSize - missingBytesForBlockMultiple;
    } // else no padding necessary

    if (lresult <= UINT16_MAX)
    {
        result = (uint16_t) lresult;
    }
    else
    {
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ScChunksMgr: Unexpected padding size '%" PRIu32 "' > UINT16_MAX", lresult);
    }
    return result;
}

static bool SOPC_Chunks_EncodePadding(uint32_t scConnectionIdx,
                                      SOPC_SecureConnection* scConnection,
                                      SOPC_SecureChannel_Config* scConfig,
                                      SOPC_Buffer* buffer,
                                      bool isSymmetricAlgo,
                                      uint32_t sequenceNumberPosition,
                                      uint32_t* signatureSize,
                                      uint16_t* realPaddingLength, // >= paddingSizeField
                                      bool* hasExtraPadding,
                                      SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConfig != NULL);
    assert(buffer != NULL);
    assert(signatureSize != NULL);
    assert(realPaddingLength != NULL);
    assert(hasExtraPadding != NULL);

    bool result = true;

    bool toEncrypt = false;
    uint32_t cipherBlockSize = 0;
    uint32_t plainTextBlockSize = 0;
    bool toSign = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    result = SC_Chunks_GetSendingCryptoSizes(scConnection, scConfig, isSymmetricAlgo, &toEncrypt, &toSign,
                                             signatureSize, &cipherBlockSize, &plainTextBlockSize);

    if (result && toEncrypt)
    {
        *realPaddingLength =
            SC_Chunks_GetPaddingSize(buffer->length - sequenceNumberPosition, plainTextBlockSize, *signatureSize);
        // Get the least significant byte value of the padding size
        const uint8_t lsbPaddingSize = (uint8_t)(0x00FF & *realPaddingLength);
        // Get the most significant byte value of the padding size
        const uint8_t msbPaddingSize = (uint8_t)((0xFF00 & *realPaddingLength) >> 8);

        // Use least significant byte value to define PaddingSize field
        uint8_t paddingSizeField = lsbPaddingSize;
        status = SOPC_Buffer_Write(buffer, &paddingSizeField, 1);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }

        if (result)
        {
            // The value of each byte of the padding is equal to paddingSize:
            SOPC_Byte* paddingBytes = SOPC_Malloc(sizeof(SOPC_Byte) * (*realPaddingLength));
            if (paddingBytes != NULL)
            {
                memset(paddingBytes, paddingSizeField, *realPaddingLength);
                status = SOPC_Buffer_Write(buffer, paddingBytes, *realPaddingLength);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
                SOPC_Free(paddingBytes);
            }
            else
            {
                result = false;
            }
        }

        // Extra-padding necessary if padding could be greater 256 bytes
        if (result && SC_Chunks_Is_ExtraPaddingSizePresent(plainTextBlockSize))
        {
            *hasExtraPadding = 1; // True
            // extra padding = most significant byte of 2 bytes padding size
            uint8_t extraPadding = msbPaddingSize;
            status = SOPC_Buffer_Write(buffer, &extraPadding, 1);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
            }
        }
    }
    else
    {
        result = false;
    }

    if (!result)
    {
        *errorStatus = OpcUa_BadTcpInternalError;

        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ChunksMgr: treat send buffer: padding encoding failed (scIdx=%" PRIu32
                               ", scCfgIdx=%" PRIu32 ")",
                               scConnectionIdx, scConnection->endpointConnectionConfigIdx);
    }

    return result;
}

static bool SOPC_Chunks_GetSignatureSize_EncodePadding(uint32_t scConnectionIdx,
                                                       SOPC_SecureConnection* scConnection,
                                                       SOPC_SecureChannel_Config* scConfig,
                                                       SOPC_Buffer* buffer,
                                                       bool isSymmetricAlgo,
                                                       bool toEncrypt,
                                                       bool toSign,
                                                       uint32_t sequenceNumberPosition,
                                                       uint32_t* signatureSize,
                                                       bool* hasPadding,
                                                       uint16_t* realPaddingLength, // >= paddingSizeField
                                                       bool* hasExtraPadding,
                                                       SOPC_StatusCode* errorStatus)
{
    bool result = false;
    /* ENCODE PADDING */
    if (toEncrypt)
    {
        *hasPadding = true;
        result = SOPC_Chunks_EncodePadding(scConnectionIdx, scConnection, scConfig, buffer, isSymmetricAlgo,
                                           sequenceNumberPosition, signatureSize, realPaddingLength, hasExtraPadding,
                                           errorStatus);
    }
    else if (toSign)
    {
        /* SIGN ONLY: ONLY DEFINE SIGNATURE SIZE */
        *hasPadding = false;
        bool tmpBool;
        uint32_t tmpInt;
        // TODO: avoid to compute non necessary crypto values
        result = SC_Chunks_GetSendingCryptoSizes(scConnection, scConfig, isSymmetricAlgo, &tmpBool, &tmpBool,
                                                 signatureSize, &tmpInt, &tmpInt);

        if (!result)
        {
            *errorStatus = OpcUa_BadTcpInternalError;
        }
    }
    else
    {
        result = true;
        *signatureSize = 0;
        *hasPadding = false;
        *realPaddingLength = 0;
        *hasExtraPadding = false;
    }
    return result;
}

static bool SC_Chunks_CheckMaxSenderCertificateSize(uint32_t scConnectionIdx,
                                                    SOPC_SecureConnection* scConnection,
                                                    int32_t senderCertificateSize,
                                                    uint32_t messageChunkSize,
                                                    int32_t securityPolicyUriLength,
                                                    bool hasPadding,
                                                    uint16_t realPaddingLength,
                                                    bool hasExtraPadding,
                                                    uint32_t asymmetricSignatureSize,
                                                    SOPC_StatusCode* errorStatus)
{
    bool result = false;
    int32_t maxSize = 0;

    if (messageChunkSize <= INT32_MAX)
    {
        result = true;
        maxSize = // Fit in a single message chunk with at least 1 byte of body
            (int32_t) messageChunkSize - SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH - 4 - // URI length field size
            securityPolicyUriLength - 4 -                                           // Sender certificate length field
            4 -  // Receiver certificate thumbprint length field
            20 - // Receiver certificate thumbprint length
            8;
    }

    if (result && hasPadding)
    {
        maxSize = maxSize - 1 - // padding length field size
                  (int32_t) realPaddingLength;
        if (hasExtraPadding)
        {
            // ExtraPaddingSize field size to remove
            maxSize = maxSize - 1;
        }
    }
    if (result)
    {
        if (asymmetricSignatureSize <= INT32_MAX)
        {
            maxSize = maxSize - (int32_t) asymmetricSignatureSize;
        }
        else
        {
            result = false;
        }
    }

    if (senderCertificateSize > maxSize)
    {
        result = false;
    }

    if (!result)
    {
        *errorStatus = scConnection->isServerConnection ? OpcUa_BadResponseTooLarge : OpcUa_BadRequestTooLarge;
        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ChunksMgr: treat send buffer: asymmetric max sender certificate size check failed "
                               "(scIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                               scConnectionIdx, scConnection->endpointConnectionConfigIdx);
    }

    return result;
}

static bool SC_Chunks_GetEncryptedDataLength(SOPC_SecureConnection* scConnection,
                                             SOPC_SecureChannel_Config* scConfig,
                                             uint32_t plainDataLength,
                                             bool isSymmetricAlgo,
                                             uint32_t* cipherDataLength)
{
    assert(scConnection != NULL);
    assert(scConfig != NULL);
    assert(cipherDataLength != NULL);
    bool result = true;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    if (!isSymmetricAlgo)
    {
        const SOPC_CertificateList* otherAppCertificate = SC_PeerCertificate(scConnection);

        if (NULL == otherAppCertificate)
        {
            result = false;
        }
        else
        {
            SOPC_AsymmetricKey* otherAppPublicKey = NULL;
            status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(otherAppCertificate, &otherAppPublicKey);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
            }

            // Retrieve cipher length
            if (result)
            {
                status = SOPC_CryptoProvider_AsymmetricGetLength_Encryption(
                    scConnection->cryptoProvider, otherAppPublicKey, plainDataLength, cipherDataLength);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }

            SOPC_KeyManager_AsymmetricKey_Free(otherAppPublicKey);
        }
    }
    else
    {
        if (NULL == scConnection->currentSecuKeySets.senderKeySet ||
            NULL == scConnection->currentSecuKeySets.receiverKeySet)
        {
            result = false;
        }
        else
        {
            // Retrieve cipher length
            status = SOPC_CryptoProvider_SymmetricGetLength_Encryption(scConnection->cryptoProvider, plainDataLength,
                                                                       cipherDataLength);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
            }
        }
    }

    return result;
}

static bool SC_Chunks_EncodeSignatureNoError(SOPC_SecureConnection* scConnection,
                                             SOPC_Buffer* buffer,
                                             bool symmetricAlgo,
                                             bool isPrevCryptoData,
                                             uint32_t signatureSize,
                                             const char** errorReason)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_ByteString signedData;

    if (!symmetricAlgo)
    {
        if (scConnection->privateKey == NULL)
        {
            return false;
        }

        status = SOPC_ByteString_InitializeFixedSize(&signedData, signatureSize);

        if (SOPC_STATUS_OK == status)
        {
            if (signedData.Length > 0)
            {
                status = SOPC_CryptoProvider_AsymmetricSign(scConnection->cryptoProvider, buffer->data, buffer->length,
                                                            scConnection->privateKey, signedData.Data,
                                                            (uint32_t) signedData.Length, errorReason);
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_Write(buffer, signedData.Data, (uint32_t) signedData.Length);
        }

        SOPC_ByteString_Clear(&signedData);
        return (SOPC_STATUS_OK == status);
    }
    else
    {
        SOPC_SC_SecurityKeySet* senderKeySet = NULL;
        SOPC_SC_SecurityKeySet* receiverKeySet = NULL;

        if (!SC_Chunks_GetSecurityKeySets(scConnection, isPrevCryptoData, &senderKeySet, &receiverKeySet))
        {
            return false;
        }

        status = SOPC_ByteString_InitializeFixedSize(&signedData, signatureSize);
        if (SOPC_STATUS_OK == status)
        {
            if (signedData.Length > 0)
            {
                status = SOPC_CryptoProvider_SymmetricSign(scConnection->cryptoProvider, buffer->data, buffer->length,
                                                           senderKeySet->signKey, signedData.Data,
                                                           (uint32_t) signedData.Length);
            }
            else
            {
                status = SOPC_STATUS_NOK;
            }
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_Write(buffer, signedData.Data, (uint32_t) signedData.Length);
        }

        SOPC_ByteString_Clear(&signedData);
        return (SOPC_STATUS_OK == status);
    }
}

static bool SC_Chunks_EncodeSignature(uint32_t scConnectionIdx,
                                      SOPC_SecureConnection* scConnection,
                                      SOPC_Buffer* buffer,
                                      bool symmetricAlgo,
                                      bool isPrevCryptoData,
                                      uint32_t signatureSize,
                                      SOPC_StatusCode* errorStatus)
{
    const char* errorReason = "";
    bool result = SC_Chunks_EncodeSignatureNoError(scConnection, buffer, symmetricAlgo, isPrevCryptoData, signatureSize,
                                                   &errorReason);
    if (!result)
    {
        *errorStatus = OpcUa_BadEncodingError;

        SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                               "ChunksMgr: treat send buffer: encoding signature failed : (scIdx=%" PRIu32
                               ", scCfgIdx=%" PRIu32 "): %s",
                               scConnectionIdx, scConnection->endpointConnectionConfigIdx, errorReason);
    }
    return result;
}

static bool SC_Chunks_EncryptMsg(SOPC_SecureConnection* scConnection,
                                 SOPC_Buffer* nonEncryptedBuffer,
                                 bool symmetricAlgo,
                                 bool isPrevCryptoData,
                                 uint32_t sequenceNumberPosition,
                                 uint32_t encryptedDataLength,
                                 SOPC_Buffer* encryptedBuffer,
                                 SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(nonEncryptedBuffer != NULL);
    assert(encryptedBuffer != NULL);
    assert(errorStatus != NULL);
    bool result = false;

    const char* errorReason = "";
    SOPC_SecureChannel_Config* scConfig = NULL;

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Byte* dataToEncrypt = &nonEncryptedBuffer->data[sequenceNumberPosition];
    const uint32_t dataToEncryptLength = nonEncryptedBuffer->length - sequenceNumberPosition;

    if (!symmetricAlgo)
    {
        /* ASYMMETRIC CASE */

        const SOPC_CertificateList* otherAppCertificate = NULL;
        if (!scConnection->isServerConnection)
        {
            // Client side
            scConfig = SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            assert(scConfig != NULL);
            otherAppCertificate = scConnection->serverCertificate;
        }
        else
        {
            // Server side
            scConfig = SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            assert(scConfig != NULL); // Even on server side it is guaranteed by secure connection state manager (no
                                      // sending in wrong state)
            otherAppCertificate = scConnection->clientCertificate;
        }

        SOPC_AsymmetricKey* otherAppPublicKey = NULL;
        SOPC_Byte* encryptedData = NULL;

        // Retrieve other app public key from certificate
        status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(otherAppCertificate, &otherAppPublicKey);

        // Check size of encrypted data array
        if (SOPC_STATUS_OK != status)
        {
            *errorStatus = OpcUa_BadTcpInternalError;

            SOPC_Logger_TraceError(
                SOPC_LOG_MODULE_CLIENTSERVER,
                "ChunksMgr: encrypt message (asymm): failed to create public key from certificate (scConfigIdx=%" PRIu32
                ")",
                scConnection->endpointConnectionConfigIdx);
        }
        else
        {
            result = true;
            if (encryptedBuffer->maximum_size < sequenceNumberPosition + encryptedDataLength)
            {
                result = false;
            }
            encryptedData = encryptedBuffer->data;
            if (NULL == encryptedData)
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
            else
            {
                // Copy non encrypted headers part
                memcpy(encryptedData, nonEncryptedBuffer->data, sequenceNumberPosition);
                // Set correct message size and encrypted buffer length
                status = SOPC_Buffer_SetDataLength(encryptedBuffer, sequenceNumberPosition + encryptedDataLength);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
        }

        // Encrypt
        if (result)
        {
            status = SOPC_CryptoProvider_AsymmetricEncrypt(
                scConnection->cryptoProvider, dataToEncrypt, dataToEncryptLength, otherAppPublicKey,
                &encryptedData[sequenceNumberPosition], encryptedDataLength, &errorReason);

            if (SOPC_STATUS_OK != status)
            {
                result = false;
                *errorStatus = OpcUa_BadEncodingError;

                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr: encrypt message (asymm): failed to encrypt message "
                                       "(scConfigIdx=%" PRIu32 "): %s",
                                       scConnection->endpointConnectionConfigIdx, errorReason);
            }
        }

        SOPC_KeyManager_AsymmetricKey_Free(otherAppPublicKey);
    }
    else
    {
        /* SYMMETRIC CASE */

        SOPC_SC_SecurityKeySet* senderKeySet = NULL;
        SOPC_SC_SecurityKeySet* receiverKeySet = NULL;

        if (!SC_Chunks_GetSecurityKeySets(scConnection, isPrevCryptoData, &senderKeySet, &receiverKeySet))
        {
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }
        else
        {
            result = true;
            SOPC_Byte* encryptedData = NULL;

            // Check size of encrypted data array
            if (encryptedBuffer->maximum_size < sequenceNumberPosition + encryptedDataLength)
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
            encryptedData = encryptedBuffer->data;
            if (NULL == encryptedData)
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
            else
            {
                // Copy non encrypted headers part
                memcpy(encryptedData, nonEncryptedBuffer->data, sequenceNumberPosition);
                // Set correct message size and encrypted buffer length
                status = SOPC_Buffer_SetDataLength(encryptedBuffer, sequenceNumberPosition + encryptedDataLength);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }

            // Encrypt
            if (result)
            {
                status = SOPC_CryptoProvider_SymmetricEncrypt(
                    scConnection->cryptoProvider, dataToEncrypt, dataToEncryptLength, senderKeySet->encryptKey,
                    senderKeySet->initVector, &encryptedData[sequenceNumberPosition], encryptedDataLength);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadEncodingError;

                    SOPC_Logger_TraceError(
                        SOPC_LOG_MODULE_CLIENTSERVER,
                        "ChunksMgr: encrypt message (symm): failed to encrypt message (scConfigIdx=%" PRIu32 ")",
                        scConnection->endpointConnectionConfigIdx);
                }
            }

        } // End valid key set
    }
    return result;
}

static bool SC_Chunks_TreatSendBufferTCPonly(uint32_t scConnectionIdx,
                                             SOPC_SecureConnection* scConnection,
                                             SOPC_Msg_Type sendMsgType,
                                             SOPC_Buffer* inputBuffer,
                                             SOPC_Buffer** outputBuffer,
                                             SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(inputBuffer != NULL);
    assert(outputBuffer != NULL);
    assert(errorStatus != NULL);
    assert(SOPC_MSG_TYPE_HEL == sendMsgType || SOPC_MSG_TYPE_ACK == sendMsgType || SOPC_MSG_TYPE_ERR == sendMsgType ||
           SOPC_MSG_TYPE_RHE == sendMsgType);
    *outputBuffer = NULL;

    // Set the position at the beginning of the buffer
    // (to be read or to encode header for which space was left)
    SOPC_StatusCode status = SOPC_Buffer_SetPosition(inputBuffer, 0);
    assert(SOPC_STATUS_OK == status);

    /* ENCODE OPC UA TCP HEADER PHASE */
    bool result = SC_Chunks_EncodeTcpMsgHeader(scConnectionIdx, scConnection, sendMsgType, SOPC_UA_FINAL_CHUNK,
                                               inputBuffer, errorStatus);

    if (result)
    {
        *outputBuffer = inputBuffer;
    }

    return result;
}

static bool SC_Chunks_EncodeScMessageHeaderAdditionalField(SOPC_SecureConnection* scConnection,
                                                           SOPC_Buffer* nonEncryptedBuffer,
                                                           SOPC_StatusCode* errorStatus)
{
    SOPC_StatusCode status =
        SOPC_UInt32_Write(&scConnection->currentSecurityToken.secureChannelId, nonEncryptedBuffer, 0);
    if (SOPC_STATUS_OK != status)
    {
        *errorStatus = OpcUa_BadEncodingError;
        return false;
    }
    return true;
}

// Encode (encrypted) message size
static bool SC_Chunks_EncodeMessageSize(SOPC_SecureChannel_Config* scConfig,
                                        SOPC_SecureConnection* scConnection,
                                        SOPC_Buffer* nonEncryptedBuffer,
                                        bool symmetricAlgo,
                                        bool toEncrypt,
                                        uint32_t sequenceNumberPosition,
                                        uint32_t signatureSize,
                                        uint32_t* encryptedDataLength,
                                        SOPC_StatusCode* errorStatus)
{
    bool result = false;
    SOPC_StatusCode status;
    uint32_t messageSize = 0;
    if (toEncrypt)
    {
        // Compute final encrypted message length:
        // Data to encrypt = already encoded message from encryption start + signature size
        const uint32_t plainDataToEncryptLength = nonEncryptedBuffer->length - sequenceNumberPosition + signatureSize;

        result = SC_Chunks_GetEncryptedDataLength(scConnection, scConfig, plainDataToEncryptLength, symmetricAlgo,
                                                  encryptedDataLength);

        if (result)
        {
            messageSize = sequenceNumberPosition + *encryptedDataLength; // non encrypted length + encrypted length
            status = SOPC_UInt32_Write(&messageSize, nonEncryptedBuffer, 0);
            result = (SOPC_STATUS_OK == status);
        }
    }
    else
    {
        // Size = current buffer length + signature if signed
        messageSize = nonEncryptedBuffer->length + signatureSize;
        status = SOPC_UInt32_Write(&messageSize, nonEncryptedBuffer, 0);
        result = (SOPC_STATUS_OK == status);
    }

    if (!result)
    {
        *errorStatus = OpcUa_BadTcpInternalError;
    }

    return result;
}

static bool SC_Chunks_EncodeSequenceNumber(SOPC_SecureConnection* scConnection,
                                           SOPC_Buffer* nonEncryptedBuffer,
                                           uint32_t sequenceNumberPosition,
                                           SOPC_StatusCode* errorStatus)
{
    bool result = false;
    // Set position to sequence number
    SOPC_StatusCode status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, sequenceNumberPosition);
    if (SOPC_STATUS_OK == status)
    {
        if (scConnection->tcpSeqProperties.lastSNsent > UINT32_MAX - 1024)
        { // Part 6 §6.7.2 v1.03
            scConnection->tcpSeqProperties.lastSNsent = 1;
        }
        else
        {
            scConnection->tcpSeqProperties.lastSNsent = scConnection->tcpSeqProperties.lastSNsent + 1;
        }
        status = SOPC_UInt32_Write(&scConnection->tcpSeqProperties.lastSNsent, nonEncryptedBuffer, 0);

        result = (SOPC_STATUS_OK == status);
    }

    if (!result)
    {
        *errorStatus = OpcUa_BadTcpInternalError;
    }

    return result;
}

static bool SC_Chunks_EncodeRequestId(SOPC_SecureConnection* scConnection,
                                      SOPC_Buffer* nonEncryptedBuffer,
                                      bool isFinalChunk,
                                      uint32_t requestIdOrHandle,
                                      uint32_t* outRequestId,
                                      SOPC_StatusCode* errorStatus)
{
    uint32_t requestId = 0;
    if (!scConnection->isServerConnection)
    {
        // Client case: retrieve next requestId and generate next one if necessary
        if (0 == scConnection->clientNextReqId)
        {
            // As client we do not use the value 0 for requestId, we reserve it to represent invalid value internally
            // (not specified by OPC UA specification)
            scConnection->clientNextReqId = 1;
        }
        // Retrieve the requestId to encode
        requestId = scConnection->clientNextReqId;

        // Generate new requestId only if current chunk is final (keep same until final is sent)
        if (isFinalChunk)
        {
            scConnection->clientNextReqId = (scConnection->clientNextReqId + 1) % UINT32_MAX;
        }
    }
    else
    {
        // Server case: return requestId provided by client
        requestId = requestIdOrHandle;
    }

    SOPC_StatusCode status = SOPC_UInt32_Write(&requestId, nonEncryptedBuffer, 0);
    bool result = (SOPC_STATUS_OK == status);
    if (result)
    {
        *outRequestId = requestId;
    }
    else
    {
        *errorStatus = OpcUa_BadTcpInternalError;
    }

    return result;
}

static bool SC_Chunks_Encrypt(SOPC_SecureConnection* scConnection,
                              SOPC_Buffer* nonEncryptedBuffer,
                              bool symmetricAlgo,
                              bool isPrevCryptoData,
                              uint32_t sequenceNumberPosition,
                              uint32_t encryptedDataLength,
                              SOPC_Buffer** outputBuffer,
                              SOPC_StatusCode* errorStatus)
{
    bool result = false;
    SOPC_Buffer* encryptedBuffer = NULL;

    if (scConnection->tcpMsgProperties.sendBufferSize >= sequenceNumberPosition + encryptedDataLength)
    {
        encryptedBuffer = SOPC_Buffer_Create(sequenceNumberPosition + encryptedDataLength);
    }

    if (encryptedBuffer != NULL)
    {
        result = SC_Chunks_EncryptMsg(scConnection, nonEncryptedBuffer, symmetricAlgo, isPrevCryptoData,
                                      sequenceNumberPosition, encryptedDataLength, encryptedBuffer, errorStatus);

        if (!result)
        {
            // errorStatus set by SC_Chunks_EncryptMsg

            // Deallocate internal buffer (not set as outputBuffer since result == false)
            SOPC_Buffer_Delete(encryptedBuffer);
            encryptedBuffer = NULL;
        }
    }
    else
    {
        *errorStatus = OpcUa_BadOutOfMemory;
    }

    if (result)
    {
        *outputBuffer = encryptedBuffer;
    }

    return result;
}

static bool SC_Chunks_CreateClientSentRequestContext(uint32_t scConnectionIdx,
                                                     SOPC_SecureConnection* scConnection,
                                                     uint32_t requestIdOrHandle,
                                                     SOPC_Msg_Type sendMsgType,
                                                     uint32_t requestId,
                                                     SOPC_StatusCode* errorStatus)
{
    bool result = false;
    SOPC_SentRequestMsg_Context* msgCtx = NULL;
    SOPC_SentRequestMsg_Context* msgCtxRes = NULL;
    switch (sendMsgType)
    {
    case SOPC_MSG_TYPE_SC_OPN:
    case SOPC_MSG_TYPE_SC_MSG:
        /* CLIENT SIDE: RECORD REQUEST SENT (response expected)*/
        msgCtx = SOPC_Calloc(1, sizeof(SOPC_SentRequestMsg_Context));
        if (msgCtx != NULL)
        {
            msgCtx->timeoutExpired = false;
            msgCtx->scConnectionIdx = scConnectionIdx;
            msgCtx->requestHandle = requestIdOrHandle; // Client side: it contains request handle
            msgCtx->msgType = sendMsgType;
            msgCtx->timerId = SC_Client_StartRequestTimeout(scConnectionIdx, requestIdOrHandle, requestId);
            msgCtxRes =
                SOPC_SLinkedList_Append(scConnection->tcpSeqProperties.sentRequestIds, requestId, (void*) msgCtx);

            result = (msgCtx == msgCtxRes);
        }
        break;
    case SOPC_MSG_TYPE_SC_CLO:
        // No response expected
        result = true;
        break;
    case SOPC_MSG_TYPE_HEL:
    case SOPC_MSG_TYPE_ACK:
    case SOPC_MSG_TYPE_ERR:
    case SOPC_MSG_TYPE_INVALID:
    default:
        assert(false);
    }

    if (!result)
    {
        *errorStatus = OpcUa_BadTcpInternalError;
    }
    return result;
}

static bool SC_Chunks_TreatSendBufferOPN(
    uint32_t scConnectionIdx,
    SOPC_SecureConnection* scConnection,
    uint32_t requestIdOrHandle, // requestId when sender is server / requestHandle when client
    SOPC_Msg_Type sendMsgType,
    SOPC_Buffer* inputBuffer,
    SOPC_Buffer** outputBuffer,
    SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(inputBuffer != NULL);
    assert(outputBuffer != NULL);
    assert(errorStatus != NULL);
    SOPC_SecureChannel_Config* scConfig = NULL;
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_Buffer* nonEncryptedBuffer;
    uint32_t requestId = 0;
    uint32_t sequenceNumberPosition = 0; // Position from which encryption start
    int32_t senderCertificateSize = 0;
    int32_t securityPolicyLength = 0;
    uint32_t signatureSize = 0;
    bool hasPadding = false;
    uint16_t realPaddingLength = 0; // padding + extra total size
    bool hasExtraPadding = false;
    uint32_t encryptedDataLength = 0;

    /* PRE-CONFIG PHASE */

    // Note: when sending a secure conversation message, the secure connection configuration shall be
    // defined
    scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection);
    assert(scConfig != NULL); // Even on server side guaranteed by the secure connection state manager

    bool toEncrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, true);
    bool toSign = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);

    // In specific case of OPN the input buffer contains only message body
    // (without bytes reserved for headers since it is not static size)
    assert(scConnection->tcpMsgProperties.sendBufferSize > 0);
    nonEncryptedBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);

    /* ENCODE OPC UA TCP HEADER PHASE */
    result = SC_Chunks_EncodeTcpMsgHeader(scConnectionIdx, scConnection, sendMsgType, SOPC_UA_FINAL_CHUNK,
                                          nonEncryptedBuffer, errorStatus);

    /* ENCODE OPC UA SECURE CONVERSATION MESSAGE PHASE*/
    if (result)
    {
        /* ENCODE OPC UA SECURE CONVERSATION MESSAGE EXTRA FIELD (secure channel Id) */
        result = SC_Chunks_EncodeScMessageHeaderAdditionalField(scConnection, nonEncryptedBuffer, errorStatus);
    }

    /* ENCODE ASYMMETRIC SECURITY HEADER */
    if (result)
    {
        result = SC_Chunks_EncodeAsymSecurityHeader(scConnectionIdx, scConnection, scConfig, nonEncryptedBuffer,
                                                    &securityPolicyLength, &senderCertificateSize, errorStatus);
    }

    /* COMPUTE ASYM. / SYM. MAX BODY SIZES */
    /* CHECK ASYM. MAX BODY SIZE */
    if (result)
    {
        // Compute max body sizes (asymm. and symm.) and check asymmetric max body size

        // Next position is the sequence number position
        sequenceNumberPosition = nonEncryptedBuffer->position;

        if (scConnection->asymmSecuMaxBodySize == 0 && scConnection->symmSecuMaxBodySize == 0)
        {
            scConnection->asymmSecuMaxBodySize =
                SC_Chunks_GetSendingMaxBodySize(scConnection, scConfig, scConnection->tcpMsgProperties.sendBufferSize,
                                                sequenceNumberPosition, false, &scConnection->hasExtraPaddingSize);
            scConnection->symmSecuMaxBodySize = SC_Chunks_GetSendingMaxBodySize(
                scConnection, scConfig, scConnection->tcpMsgProperties.sendBufferSize,
                // sequenceNumber position for symmetric case:
                (SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH + SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH), true,
                &scConnection->hasExtraPaddingSize);
        }
        if (scConnection->asymmSecuMaxBodySize == 0 || scConnection->symmSecuMaxBodySize == 0)
        {
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }
        else
        {
            // Check asymmetric max body size compliant
            if (inputBuffer->length > scConnection->asymmSecuMaxBodySize)
            {
                result = false;
                *errorStatus = scConnection->isServerConnection ? OpcUa_BadResponseTooLarge : OpcUa_BadRequestTooLarge;
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ChunksMgr: treat send buffer: asymmetric max body size check failed : %" PRIu32
                                       " > max = %" PRIu32 " (scIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                       inputBuffer->length, scConnection->asymmSecuMaxBodySize, scConnectionIdx,
                                       scConnection->endpointConnectionConfigIdx);
            }
        }
    }

    // RESERVE BYTES FOR SEQUENCE HEADER
    if (result)
    {
        assert(nonEncryptedBuffer->length < nonEncryptedBuffer->position + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
        status = SOPC_Buffer_SetDataLength(nonEncryptedBuffer,
                                           nonEncryptedBuffer->position + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_SetPosition(nonEncryptedBuffer,
                                             nonEncryptedBuffer->position + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
        }
        if (SOPC_STATUS_OK != status)
        {
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }
    }

    // COPY BODY BYTES FROM INPUT BUFFER
    if (result)
    {
        status = SOPC_Buffer_Write(nonEncryptedBuffer, inputBuffer->data, inputBuffer->length);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }
    }

    // GET SIGNATURE SIZE + ((when toEncrypt = true only)  ENCODE PADDING + GET PADDING LENGTH)
    if (result)
    {
        result = SOPC_Chunks_GetSignatureSize_EncodePadding(
            scConnectionIdx, scConnection, scConfig, nonEncryptedBuffer, false, toEncrypt, toSign,
            sequenceNumberPosition, &signatureSize, &hasPadding, &realPaddingLength, &hasExtraPadding, errorStatus);
    }

    // CHECK MaxSenderCertificate
    if (result)
    {
        // ASYMMETRIC SECURITY SPECIFIC CASE: check MaxSenderCertificate side (padding necessary)
        // TODO: since we already encoded everything except signature, is it really necessary ?
        result = SC_Chunks_CheckMaxSenderCertificateSize(
            scConnectionIdx, scConnection, senderCertificateSize, nonEncryptedBuffer->maximum_size,
            securityPolicyLength, hasPadding, realPaddingLength, hasExtraPadding, signatureSize, errorStatus);
    }

    if (result)
    {
        // Set position to message size field
        status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, SOPC_UA_HEADER_MESSAGE_SIZE_POSITION);
        assert(SOPC_STATUS_OK == status);
    }

    if (result)
    {
        /* ENCODE (ENCRYPTED) MESSAGE SIZE */
        result = SC_Chunks_EncodeMessageSize(scConfig, scConnection, nonEncryptedBuffer, false, toEncrypt,
                                             sequenceNumberPosition, signatureSize, &encryptedDataLength, errorStatus);
    }

    if (result)
    {
        /* ENCODE SEQUENCE NUMBER */
        result = SC_Chunks_EncodeSequenceNumber(scConnection, nonEncryptedBuffer, sequenceNumberPosition, errorStatus);
    }

    if (result)
    {
        /* ENCODE REQUEST ID */
        result = SC_Chunks_EncodeRequestId(scConnection, nonEncryptedBuffer, true, requestIdOrHandle, &requestId,
                                           errorStatus);
    }

    if (result)
    {
        // Set the buffer at the end for next write
        status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, nonEncryptedBuffer->length);
        assert(SOPC_STATUS_OK == status);
    }

    /* SIGN MESSAGE */
    if (result && toSign)
    {
        result = SC_Chunks_EncodeSignature(scConnectionIdx, scConnection, nonEncryptedBuffer, false, false,
                                           signatureSize, errorStatus);
    }

    if (result)
    {
        /* ENCRYPT MESSAGE */
        if (toEncrypt)
        {
            result = SC_Chunks_Encrypt(scConnection, nonEncryptedBuffer, false, false, sequenceNumberPosition,
                                       encryptedDataLength, outputBuffer, errorStatus);

            // It is only an internal buffer, it shall be freed
            SOPC_Buffer_Delete(nonEncryptedBuffer);
            nonEncryptedBuffer = NULL;
        }
        else
        {
            // No encryption output buffer is non encrypted buffer
            *outputBuffer = nonEncryptedBuffer;
        }
    }

    /* RECORD REQUEST CONTEXT (CLIENT ONLY) */
    if (result && !scConnection->isServerConnection)
    {
        result = SC_Chunks_CreateClientSentRequestContext(scConnectionIdx, scConnection, requestIdOrHandle, sendMsgType,
                                                          requestId, errorStatus);
    }

    if (!result)
    {
        *outputBuffer = NULL;
        // It is only an internal buffer, it shall be freed here
        // otherwise it is the input buffer freed by caller
        SOPC_Buffer_Delete(nonEncryptedBuffer);
        nonEncryptedBuffer = NULL;
    }

    return result;
}

static bool SC_Chunks_TreatSendBufferMSGCLO(
    uint32_t scConnectionIdx,
    SOPC_SecureConnection* scConnection,
    uint32_t requestIdOrHandle, // requestId when sender is server / requestHandle when client
    SOPC_Msg_Type sendMsgType,
    uint8_t isFinalChar,
    SOPC_Buffer** inputChunkBuffer,
    SOPC_Buffer** outputBuffer,
    SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(inputChunkBuffer != NULL);
    assert(*inputChunkBuffer != NULL);
    assert(outputBuffer != NULL);
    assert(errorStatus != NULL);
    SOPC_Buffer* nonEncryptedBuffer = *inputChunkBuffer;
    SOPC_SecureChannel_Config* scConfig = NULL;
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    uint32_t requestId = 0;
    uint32_t bodySize = 0;
    uint32_t tokenId = 0;
    bool isPrevCryptoData = false;
    uint32_t signatureSize = 0;
    bool hasPadding = false;
    uint16_t realPaddingLength = 0; // padding + extra total size
    bool hasExtraPadding = false;
    uint32_t encryptedDataLength = 0;

    /* PRE-CONFIG PHASE */

    // Note: when sending a secure conversation message, the secure connection configuration shall be
    // defined
    scConfig = SOPC_Toolkit_GetSecureChannelConfig(scConnection);
    assert(scConfig != NULL); // Even on server side guaranteed by the secure connection state manager

    bool toEncrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, false);
    bool toSign = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);

    // Set the position at the beginning of the buffer (to be read or to encode header for which space was
    // left)
    status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, 0);
    assert(SOPC_STATUS_OK == status);

    /* ENCODE OPC UA TCP HEADER PHASE */
    result = SC_Chunks_EncodeTcpMsgHeader(scConnectionIdx, scConnection, sendMsgType, isFinalChar, nonEncryptedBuffer,
                                          errorStatus);

    /* ENCODE OPC UA SECURE CONVERSATION MESSAGE PHASE*/
    if (result)
    {
        /* ENCODE OPC UA SECURE CONVERSATION MESSAGE EXTRA FIELD (secure channel Id) */
        result = SC_Chunks_EncodeScMessageHeaderAdditionalField(scConnection, nonEncryptedBuffer, errorStatus);
    }

    if (result)
    {
        /* CHECK MAX BODY SIZE */
        assert(scConnection->symmSecuMaxBodySize != 0);
        // Note: buffer already contains the message body (buffer length == end of body)
        bodySize = nonEncryptedBuffer->length - SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH;
        if (bodySize > scConnection->symmSecuMaxBodySize)
        {
            // Note: we do not manage several chunks for now (expected place to manage it)
            result = false;
            *errorStatus = scConnection->isServerConnection ? OpcUa_BadResponseTooLarge : OpcUa_BadRequestTooLarge;
            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: treat send buffer: symmetric max body size check failed : %" PRIu32
                                   " > max = %" PRIu32 " (scIdx=%" PRIu32 ", scCfgIdx=%" PRIu32 ")",
                                   bodySize, scConnection->symmSecuMaxBodySize, scConnectionIdx,
                                   scConnection->endpointConnectionConfigIdx);
        }
    }

    if (result)
    {
        /* ENCODE SYMMETRIC SECURITY HEADER */
        //  retrieve tokenId
        if (scConnection->isServerConnection && !scConnection->serverNewSecuTokenActive)
        {
            // Server side only (SC renew): new token is not active yet, use the precedent token
            // Note: no timeout on precedent token validity implemented, but it is checked on each client request
            assert(scConnection->precedentSecurityToken.tokenId != 0);
            assert(scConnection->precedentSecurityToken.secureChannelId != 0);
            tokenId = scConnection->precedentSecurityToken.tokenId;
            isPrevCryptoData = true;
        }
        else
        {
            // Use current token
            tokenId = scConnection->currentSecurityToken.tokenId;
            isPrevCryptoData = false;
        }
        //  encode tokenId
        status = SOPC_UInt32_Write(&tokenId, nonEncryptedBuffer, 0);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }
    }

    if (result)
    {
        // Set position to end of body
        status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, nonEncryptedBuffer->length);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }
    }

    if (result)
    {
        // GET SIGNATURE SIZE + ((when toEncrypt = true only)  ENCODE PADDING + GET PADDING LENGTH)
        result = SOPC_Chunks_GetSignatureSize_EncodePadding(
            scConnectionIdx, scConnection, scConfig, nonEncryptedBuffer, true, toEncrypt, toSign,
            SOPC_UA_SYMMETRIC_SEQUENCE_HEADER_POSITION, &signatureSize, &hasPadding, &realPaddingLength,
            &hasExtraPadding, errorStatus);
    }

    if (result)
    {
        // Set position to message size field
        status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, SOPC_UA_HEADER_MESSAGE_SIZE_POSITION);
        assert(SOPC_STATUS_OK == status);
    }

    if (result)
    {
        /* ENCODE (ENCRYPTED) MESSAGE SIZE */
        result = SC_Chunks_EncodeMessageSize(scConfig, scConnection, nonEncryptedBuffer, true, toEncrypt,
                                             SOPC_UA_SYMMETRIC_SEQUENCE_HEADER_POSITION, signatureSize,
                                             &encryptedDataLength, errorStatus);
    }

    if (result)
    {
        /* ENCODE SEQUENCE NUMBER */
        result = SC_Chunks_EncodeSequenceNumber(scConnection, nonEncryptedBuffer,
                                                SOPC_UA_SYMMETRIC_SEQUENCE_HEADER_POSITION, errorStatus);
    }

    if (result)
    {
        /* ENCODE REQUEST ID */

        result = SC_Chunks_EncodeRequestId(scConnection, nonEncryptedBuffer, isFinalChar != 'C', requestIdOrHandle,
                                           &requestId, errorStatus);
    }

    if (result)
    {
        // Set the buffer at the end for next write
        status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, nonEncryptedBuffer->length);
        assert(SOPC_STATUS_OK == status);
    }

    if (result && toSign)
    {
        /* SIGN MESSAGE */
        result = SC_Chunks_EncodeSignature(scConnectionIdx, scConnection, nonEncryptedBuffer, true, isPrevCryptoData,
                                           signatureSize, errorStatus);

        if (!result)
        {
            *errorStatus = OpcUa_BadEncodingError;

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: treat send buffer: encoding signature failed : (scIdx=%" PRIu32
                                   ", scCfgIdx=%" PRIu32 ")",
                                   scConnectionIdx, scConnection->endpointConnectionConfigIdx);
        }
    }

    if (result)
    {
        /* ENCRYPT MESSAGE */
        if (toEncrypt)
        {
            result = SC_Chunks_Encrypt(scConnection, nonEncryptedBuffer, true, isPrevCryptoData,
                                       SOPC_UA_SYMMETRIC_SEQUENCE_HEADER_POSITION, encryptedDataLength, outputBuffer,
                                       errorStatus);
            // Note: do not deallocate inputChunkBuffer for reuse
        }
        else
        {
            // No encryption output buffer is non encrypted buffer
            *outputBuffer = nonEncryptedBuffer;
            // Input chunk buffer shall not be reused
            *inputChunkBuffer = NULL;
        }
    }

    /* RECORD REQUEST CONTEXT (CLIENT ONLY) */
    if (result && !scConnection->isServerConnection && isFinalChar == 'F')
    {
        result = SC_Chunks_CreateClientSentRequestContext(scConnectionIdx, scConnection, requestIdOrHandle, sendMsgType,
                                                          requestId, errorStatus);
    }

    if (!result)
    {
        *outputBuffer = NULL;
    }
    return result;
}

void SOPC_ChunksMgr_OnSocketEvent(SOPC_Sockets_OutputEvent event, uint32_t eltId, uintptr_t params, uintptr_t auxParam)
{
    SOPC_UNUSED_ARG(auxParam);
    SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);
    SOPC_Buffer* buffer = (void*) params;

    if (scConnection == NULL || buffer == NULL || scConnection->state == SECURE_CONNECTION_STATE_SC_CLOSED)
    {
        SOPC_Buffer_Delete(buffer);
        return;
    }

    switch (event)
    {
    /* Sockets events: */
    case SOCKET_RCV_BYTES:
        /* id = secure channel connection index,
           params = (SOPC_Buffer*) received buffer */

        SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ScChunksMgr: SOCKET_RCV_BYTES scIdx=%" PRIu32, eltId);

        // Ensure the buffer position is 0 to treat it
        if (SOPC_Buffer_SetPosition(buffer, 0) != SOPC_STATUS_OK)
        {
            SOPC_Buffer_Delete(buffer);

            SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ChunksMgr: raised INT_SC_RCV_FAILURE: %X: (epCfgIdx=%" PRIu32 ", scCfgIdx=%" PRIu32
                                   ")",
                                   OpcUa_BadInvalidArgument, scConnection->serverEndpointConfigIdx,
                                   scConnection->endpointConnectionConfigIdx);
            SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_RCV_FAILURE, eltId, (uintptr_t) NULL,
                                                           OpcUa_BadInvalidArgument);
            return;
        }

        SC_Chunks_TreatReceivedBuffer(scConnection, eltId, buffer);
        break;
    default:
        assert(false);
    }
}

/**
 * \brief Compute the number of chunks to send given the message size and TCP UA settings.
 *
 * In other words, \c dst has been successfully filled if and only if this
 * function returns \c TRUE and \c remaining is set to 0.
 *
 * \param scConnection  The connection properties (TCP UA settings, server or client)
 * \param msgType       The message type (multi-chunk only allowed for MSG type)
 * \param msgBodyLength Length of the unencrypted message body to send
 * \param nbChunks      Output parameter valid when \c TRUE returned, the number of chunks to use
 * \param errorStatus   Output parameter valid when \c FALSE returned, the error status to use
 *
 * \return \c TRUE in case is possible to send the message with the returned number of chunks,
 *         \c FALSE when it is impossible to send the message
 */
static bool SC_Chunks_ComputeNbChunksToSend(SOPC_SecureConnection* scConnection,
                                            SOPC_Msg_Type msgType,
                                            uint32_t msgBodyLength,
                                            uint32_t* nbChunks,
                                            SOPC_StatusCode* errorStatus)
{
    assert(NULL != nbChunks);
    assert(NULL != errorStatus);
    assert(SOPC_MSG_TYPE_SC_MSG == msgType || SOPC_MSG_TYPE_SC_CLO == msgType);

    bool result = false;
    SOPC_SecureConnection_TcpProperties* tcpProperties = &scConnection->tcpMsgProperties;
    if (0 == tcpProperties->sendMaxMessageSize || msgBodyLength <= tcpProperties->sendMaxMessageSize)
    {
        if (msgBodyLength <= scConnection->symmSecuMaxBodySize)
        {
            *nbChunks = 1;
            result = true;
        }
        else if (SOPC_MSG_TYPE_SC_MSG == msgType)
        {
            uint32_t requestedNbChunks = msgBodyLength / scConnection->symmSecuMaxBodySize;
            if (msgBodyLength % scConnection->symmSecuMaxBodySize)
            {
                // Previous computed value is truncated, an additional chunk is necessary
                requestedNbChunks++;
            }

            if (0 == tcpProperties->sendMaxChunkCount || requestedNbChunks <= tcpProperties->sendMaxChunkCount)
            {
                *nbChunks = requestedNbChunks;
                result = true;
            }

        } // else: only MSG type is allowed to use multi-chunks
    }

    if (!result)
    {
        *errorStatus = scConnection->isServerConnection ? OpcUa_BadResponseTooLarge : OpcUa_BadRequestTooLarge;
    }

    return result;
}

static bool SC_Chunks_NextOutputChunkBuffer(SOPC_SecureConnection* scConnection,
                                            SOPC_Buffer* msgBuffer,
                                            SOPC_Buffer** nextChunkBuffer,
                                            SOPC_StatusCode* errorStatus,
                                            char** errorReason)
{
    // Note: msgBuffer position is set to first byte of body to be sent
    uint32_t remainingBytes = SOPC_Buffer_Remaining(msgBuffer);
    uint32_t nextChunkBodySize =
        remainingBytes < scConnection->symmSecuMaxBodySize ? remainingBytes : scConnection->symmSecuMaxBodySize;
    if (NULL == *nextChunkBuffer)
    {
        // Note: allocate of max buffer size since we need to encode headers in addition to body
        //       (symmSecuMaxBodySize is computed based on this size)
        *nextChunkBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
    }
    else
    {
        assert(SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH + nextChunkBodySize <= (*nextChunkBuffer)->maximum_size);
        SOPC_Buffer_Reset(*nextChunkBuffer);
    }
    bool result = *nextChunkBuffer != NULL;
    if (result)
    {
        // Set buffer position and length after the message headers to only manage message body part
        SOPC_ReturnStatus status =
            SOPC_Buffer_SetDataLength(*nextChunkBuffer, SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH);
        assert(SOPC_STATUS_OK == status);
        status = SOPC_Buffer_SetPosition(*nextChunkBuffer, SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH);
        assert(SOPC_STATUS_OK == status);

        // Copy message body bytes to be sent in the next chunk
        uint32_t remaining = 0;
        result = fill_buffer(*nextChunkBuffer, msgBuffer, nextChunkBodySize, &remaining);
        assert(result);
        assert(0 == remaining);

        if (result)
        {
            // Restore position prior to message headers to be encoded first
            status = SOPC_Buffer_SetPosition(*nextChunkBuffer, 0);
            assert(SOPC_STATUS_OK == status);
        }
        else
        {
            *errorStatus = OpcUa_BadTcpInternalError;
            *errorReason = "Internal error when copying next chunk buffer";
        }
    }
    else
    {
        *errorStatus = OpcUa_BadOutOfMemory;
        *errorReason = "Internal error when allocating next chunk buffer";
    }
    return result;
}

static bool SC_Chunks_EncodeAbortMsg(SOPC_Buffer* inputMsgBuffer, SOPC_StatusCode errorStatus, char* reason)
{
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_String tempString;
    SOPC_String_Initialize(&tempString);

    SOPC_Buffer_Reset(inputMsgBuffer);

    // Let size of the headers for the chunk manager
    status = SOPC_Buffer_SetDataLength(inputMsgBuffer, SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH);
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_SetPosition(inputMsgBuffer, SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH);
    }

    if (SOPC_STATUS_OK == status)
    {
        // This shall be one of the values listed in OPC UA TCP error codes table
        SOPC_StatusCode normalizedStatus = SOPC_StatusCode_ToTcpErrorCode(errorStatus);
        status = SOPC_UInt32_Write(&normalizedStatus, inputMsgBuffer, 0);
    }
    if (SOPC_STATUS_OK == status)
    {
        SOPC_String_AttachFromCstring(&tempString, reason);
    }
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_String_Write(&tempString, inputMsgBuffer, 0);
    }

    SOPC_String_Clear(&tempString);

    return SOPC_STATUS_OK == status;
}

static uint8_t SC_Chunks_IsNextChunkIntermediateOrFinal(uint32_t nb_chunks_required, uint32_t nb_chunks_sent)
{
    if (nb_chunks_required > nb_chunks_sent + 1)
    {
        return SOPC_UA_INTERMEDIATE_CHUNK;
    }
    else
    {
        return SOPC_UA_FINAL_CHUNK;
    }
}

static bool SC_Chunks_TreatSendMessageBuffer(
    uint32_t scConnectionIdx,
    SOPC_SecureConnection* scConnection,
    uint32_t requestIdOrHandle, // requestId when sender is server / requestHandle when client
    SOPC_Msg_Type sendMsgType,
    bool isTcpUaOnly,
    bool isOPN,
    SOPC_Buffer* inputMsgBuffer,
    bool* failedWithAbortMessage, /* output parameter to indicate abort message was sent */
    SOPC_StatusCode* errorStatus) // input / output parameter: only an input when true == *failedWithAbortMessage
{
    bool result = false;
    uint32_t nb_chunks = 0;
    uint32_t nb_chunks_sent = 0;
    SOPC_Buffer* inputChunkBuffer = NULL;
    SOPC_Buffer* outputChunkBuffer = NULL;

    assert(NULL != failedWithAbortMessage);

    // Set the position at the beginning of the message buffer where starts message headers
    // (corresponding empty bytes were left for headers)
    SOPC_StatusCode status = SOPC_Buffer_SetPosition(inputMsgBuffer, 0);
    assert(SOPC_STATUS_OK == status);

    if (isTcpUaOnly)
    {
        // HEL / ACK / ERR / RHE case
        assert(SOPC_MSG_TYPE_HEL == sendMsgType || SOPC_MSG_TYPE_ACK == sendMsgType ||
               SOPC_MSG_TYPE_ERR == sendMsgType || SOPC_MSG_TYPE_RHE == sendMsgType);
        result = SC_Chunks_TreatSendBufferTCPonly(scConnectionIdx, scConnection, sendMsgType, inputMsgBuffer,
                                                  &outputChunkBuffer, errorStatus);

        if (result)
        {
            // Require write of output buffer on socket
            SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, scConnection->socketIndex, (uintptr_t) outputChunkBuffer, 0);
        }
    }
    else if (isOPN)
    {
        // OPN case (asymmetric case)
        assert(SOPC_MSG_TYPE_SC_OPN == sendMsgType);
        result = SC_Chunks_TreatSendBufferOPN(scConnectionIdx, scConnection, requestIdOrHandle, sendMsgType,
                                              inputMsgBuffer, &outputChunkBuffer, errorStatus);

        if (result)
        {
            // Require write of output buffer on socket
            SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, scConnection->socketIndex, (uintptr_t) outputChunkBuffer, 0);
        }
    }
    else
    {
        assert(SOPC_MSG_TYPE_SC_MSG == sendMsgType || SOPC_MSG_TYPE_SC_CLO == sendMsgType);
        // MSG (/CLO) case (symmetric case)

        char* errorReason = NULL;

        /* Part 6 (1.03): §6.7.3 MessageChunks and error handling:
         * If an error occurs creating a MessageChunk then the sender shall [...]
         * tells the receiver that an error occurred and that it should discard
         * the previous chunks. The sender indicates that the MessageChunk contains an error
         * by setting the IsFinal flag to ‘A’ (for Abort).
         * [...] the receiver shall ignore the Message but shall not close the SecureChannel.
         *
         * Note: since it is possible to fail building first chunk it seems reasonable to send a final chunk 'A'
         * even when no intermediate chunks were sent before. It allows to indicate failure to peer but to keep the
         * SC opened as indicated.
         */
        *failedWithAbortMessage = true;
        errorReason = "Not enough bytes available in chunk(s) to send the message.";

        // Move forward to message body only in order to compute the number of chunks only based on body size
        status = SOPC_Buffer_SetPosition(inputMsgBuffer, SOPC_UA_SYMMETRIC_SECURE_MESSAGE_HEADERS_LENGTH);
        assert(SOPC_STATUS_OK == status);

        result = SC_Chunks_ComputeNbChunksToSend(scConnection, sendMsgType, SOPC_Buffer_Remaining(inputMsgBuffer),
                                                 &nb_chunks, errorStatus);

        if (result)
        {
            assert(nb_chunks > 0);
            if (nb_chunks == 1)
            {
                // We use the buffer directly, restore position prior to message headers and use it directly as
                // final chunk
                status = SOPC_Buffer_SetPosition(inputMsgBuffer, 0);
                assert(SOPC_STATUS_OK == status);
                inputChunkBuffer = inputMsgBuffer;
                // Deallocation of input buffer transfered to chunks buffer
                inputMsgBuffer = NULL;
            }
            else
            {
                assert(!isOPN);
                result = SC_Chunks_NextOutputChunkBuffer(scConnection, inputMsgBuffer, &inputChunkBuffer, errorStatus,
                                                         &errorReason);
            }
        }

        while (result && nb_chunks_sent < nb_chunks)
        {
            result =
                SC_Chunks_TreatSendBufferMSGCLO(scConnectionIdx, scConnection, requestIdOrHandle, sendMsgType,
                                                SC_Chunks_IsNextChunkIntermediateOrFinal(nb_chunks, nb_chunks_sent),
                                                &inputChunkBuffer, &outputChunkBuffer, errorStatus);

            if (result)
            {
                // Require write of output buffer on socket
                SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, scConnection->socketIndex, (uintptr_t) outputChunkBuffer, 0);
            }
            else
            {
                // Deallocate output buffer since not transmitted to socket layer
                SOPC_Buffer_Delete(outputChunkBuffer);
                // Input chunk buffer is always deallocated after the loop
                // (if it was not forwarded as the output buffer)
                outputChunkBuffer = NULL;
            }

            nb_chunks_sent++;

            if (result && nb_chunks_sent < nb_chunks)
            {
                assert(outputChunkBuffer != inputMsgBuffer); // otherwise only one chunk to send
                result = SC_Chunks_NextOutputChunkBuffer(scConnection, inputMsgBuffer, &inputChunkBuffer, errorStatus,
                                                         &errorReason);
            }
        }

        // Deallocate chunk since it will not be used it anymore
        SOPC_Buffer_Delete(inputChunkBuffer);
        inputChunkBuffer = NULL;

        // In case of failure (or requested abort chunk) we send an abort chunk
        if (!result)
        {
            // Note: Reuse inputMsgBuffer for abort message
            result = SC_Chunks_EncodeAbortMsg(inputMsgBuffer, *errorStatus, errorReason);

            if (result)
            {
                SOPC_Logger_TraceWarning(SOPC_LOG_MODULE_CLIENTSERVER,
                                         "ScChunksMgr: encoded an abort chunk for: scIdx=%" PRIu32
                                         " reqId/Handle=%" PRIu32 " sc=%X reason='%s'",
                                         scConnectionIdx, requestIdOrHandle,
                                         SOPC_StatusCode_ToTcpErrorCode(*errorStatus), errorReason);
                result = SC_Chunks_TreatSendBufferMSGCLO(scConnectionIdx, scConnection, requestIdOrHandle, sendMsgType,
                                                         SOPC_UA_ABORT_FINAL_CHUNK, &inputMsgBuffer, &outputChunkBuffer,
                                                         errorStatus);
            }

            if (result)
            {
                // Require write of output buffer on socket
                SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, scConnection->socketIndex, (uintptr_t) outputChunkBuffer, 0);
                // Final result is complete message cannot be sent even if abort message was
                result = false;
            }
            else
            {
                // Abort message sending failed, keep error as a fatal failure to send a msg on SC
                *failedWithAbortMessage = false;
                // Deallocate output buffer since not transmitted to socket layer
                SOPC_Buffer_Delete(outputChunkBuffer);
                outputChunkBuffer = NULL;
                // InputMsgBuffer deallocated before return of function if it was not forwarded as the outputChunkBuffer
            }
        }
    }

    if (inputMsgBuffer != outputChunkBuffer)
    {
        // If buffer not reused for sending on socket: delete it (function caller shall consider it NULL)
        SOPC_Buffer_Delete(inputMsgBuffer);
    } // else: buffer reused for sending on socket (function caller shall consider it NULL)

    return result;
}

void SOPC_ChunksMgr_Dispatcher(SOPC_SecureChannels_InternalEvent event,
                               uint32_t eltId,
                               uintptr_t params,
                               uintptr_t auxParam)
{
    SOPC_Msg_Type sendMsgType = SOPC_MSG_TYPE_INVALID;
    SOPC_Buffer* buffer = (SOPC_Buffer*) params;
    SOPC_StatusCode errorStatus = SOPC_GoodGenericStatus; // Good
    bool isSendTcpOnly = false;
    bool isOPN = false;
    bool result = true;
    // True if socket will be closed after sending this message (ERR, CLO)
    bool socketWillClose = false;
    SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);
    bool failedWithAbortMessage = false;

    if (scConnection != NULL && scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED)
    {
        switch (event)
        {
            /* SC connection manager -> OPC UA chunks message manager */
            // id = secure channel connection index,
            // params = (SOPC_Buffer*) buffer positioned to message payload
            // auxParam = request Id context if response
        case INT_SC_SND_HEL:
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ScChunksMgr: INT_SC_SND_HEL scIdx=%" PRIu32, eltId);

            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_HEL;
            break;
        case INT_SC_SND_ACK:
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ScChunksMgr: INT_SC_SND_ACK scIdx=%" PRIu32, eltId);

            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_ACK;
            break;
        case INT_SC_SND_ERR:
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ScChunksMgr: INT_SC_SND_ERR scIdx=%" PRIu32, eltId);

            socketWillClose = true;
            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_ERR;
            break;
        case INT_SC_SND_RHE:
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ScChunksMgr: INT_SC_SND_RHE scIdx=%" PRIu32, eltId);

            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_RHE;
            break;
        case INT_SC_SND_OPN:
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ScChunksMgr: INT_SC_SND_OPN scIdx=%" PRIu32, eltId);

            isOPN = true;
            sendMsgType = SOPC_MSG_TYPE_SC_OPN;
            // Note: only message to be provided without size of header reserved (variable size for asymmetric secu
            // header)
            break;
        case INT_SC_SND_CLO:
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER, "ScChunksMgr: INT_SC_SND_CLO scIdx=%" PRIu32, eltId);

            socketWillClose = true;
            sendMsgType = SOPC_MSG_TYPE_SC_CLO;
            break;
        case INT_SC_SND_MSG_CHUNKS:
            SOPC_Logger_TraceDebug(SOPC_LOG_MODULE_CLIENTSERVER,
                                   "ScChunksMgr: INT_SC_SND_MSG_CHUNKS scIdx=%" PRIu32 " reqId/Handle=%" PRIuPTR, eltId,
                                   auxParam);

            sendMsgType = SOPC_MSG_TYPE_SC_MSG;
            break;
        default:
            // Already filtered by secure channels API module
            assert(false);
        }

        if (NULL == buffer || auxParam > UINT32_MAX)
        {
            result = false;
            errorStatus = OpcUa_BadInvalidArgument;
        }

        if (result)
        {
            result =
                SC_Chunks_TreatSendMessageBuffer(eltId, scConnection, (uint32_t) auxParam, sendMsgType, isSendTcpOnly,
                                                 isOPN, buffer, &failedWithAbortMessage, &errorStatus);
            buffer = NULL; // Consumed by previous call
        }

        if (!result)
        {
            if (!socketWillClose)
            {
                if (failedWithAbortMessage)
                {
                    SOPC_SecureChannels_EnqueueInternalEvent(INT_SC_SENT_ABORT_FAILURE, eltId, auxParam, errorStatus);
                }
                else
                {
                    // Treat as prio events
                    SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_SND_FATAL_FAILURE, eltId, auxParam,
                                                                   errorStatus);
                }
            }
            else
            {
                SOPC_Logger_TraceError(SOPC_LOG_MODULE_CLIENTSERVER,
                                       "ScChunksMgr: Failed sending message type SOPC_Msg_Type=%u before socket closed "
                                       "scIdx=%" PRIu32,
                                       sendMsgType, eltId);
            }
        }
    }

    // Always free buffer if not NULL
    SOPC_Buffer_Delete(buffer);
}
