/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sopc_chunks_mgr.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "opcua_statuscodes.h"
#include "sopc_crypto_provider.h"

#include "sopc_encoder.h"
#include "sopc_secure_channels_api.h"
#include "sopc_secure_channels_api_internal.h"
#include "sopc_secure_channels_internal_ctx.h"
#include "sopc_singly_linked_list.h"
#include "sopc_sockets_api.h"
#include "sopc_toolkit_config_internal.h"
#include "sopc_toolkit_constants.h"

static const uint8_t SOPC_HEL[3] = {'H', 'E', 'L'};
static const uint8_t SOPC_ACK[3] = {'A', 'C', 'K'};
static const uint8_t SOPC_ERR[3] = {'E', 'R', 'R'};
static const uint8_t SOPC_MSG[3] = {'M', 'S', 'G'};
static const uint8_t SOPC_OPN[3] = {'O', 'P', 'N'};
static const uint8_t SOPC_CLO[3] = {'C', 'L', 'O'};

static bool SC_Chunks_DecodeTcpMsgHeader(SOPC_SecureConnection_ChunkMgrCtx* chunkCtx)
{
    assert(chunkCtx != NULL);
    assert(chunkCtx->chunkInputBuffer != NULL);
    assert(chunkCtx->chunkInputBuffer->length - chunkCtx->chunkInputBuffer->position >= SOPC_TCP_UA_HEADER_LENGTH);
    assert(chunkCtx->currentMsgType == SOPC_MSG_TYPE_INVALID);
    assert(chunkCtx->currentMsgIsFinal == SOPC_MSG_ISFINAL_INVALID);
    assert(chunkCtx->currentMsgSize == 0);

    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    bool result = false;
    uint8_t msgType[3];
    uint8_t isFinal;

    // READ message type
    status = SOPC_Buffer_Read(msgType, chunkCtx->chunkInputBuffer, 3);
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
            result = false;
        }
    }

    // READ IsFinal message chunk
    if (result != false)
    {
        status = SOPC_Buffer_Read(&isFinal, chunkCtx->chunkInputBuffer, 1);
        if (SOPC_STATUS_OK == status)
        {
            switch (isFinal)
            {
            case 'C':
                chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_INTERMEDIATE;
                break;
            case 'F':
                chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_FINAL;
                break;
            case 'A':
                chunkCtx->currentMsgIsFinal = SOPC_MSG_ISFINAL_ABORT;
                break;
            default:
                // unchanged current isFinal value => error case
                result = false;
                break;
            }

            // In TCP UA non secure messages reserved byte shall be set to 'F'
            if (chunkCtx->currentMsgType != SOPC_MSG_TYPE_SC_MSG)
            {
                if (chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
                {
                    result = false;
                }
            }
        }
    }

    // READ message size
    if (result != false)
    {
        status = SOPC_UInt32_Read(&chunkCtx->currentMsgSize, chunkCtx->chunkInputBuffer);
        if (SOPC_STATUS_OK != status || chunkCtx->currentMsgSize <= SOPC_TCP_UA_HEADER_LENGTH)
        {
            // Message size cannot be less or equal to the TCP UA header length
            result = false;
        }
    }

    return result;
}

static bool SC_Chunks_ReadDataFromReceivedBuffer(SOPC_Buffer* inputBuffer,
                                                 SOPC_Buffer* receivedBuffer,
                                                 uint32_t sizeToRead)
{
    // received buffer shall have enough data to be read
    assert(sizeToRead <= (receivedBuffer->length - receivedBuffer->position));
    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool result = false;

    // Retrieve position in which data must be placed in input buffer (end of buffer)
    // Its position will not be updated on purpose (to could be read in the future)
    uint8_t* readDest = &(inputBuffer->data[inputBuffer->length]);

    // Update length of input buffer and check it fits (STATUS_OK returned)
    status = SOPC_Buffer_SetDataLength(inputBuffer, inputBuffer->length + sizeToRead);

    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_Buffer_Read(readDest, receivedBuffer, sizeToRead);
    }

    if (SOPC_STATUS_OK == status)
    {
        result = true;
    }

    return result;
}

static SOPC_SecureChannels_InputEvent SC_Chunks_MsgTypeToRcvEvent(SOPC_Msg_Type msgType)
{
    SOPC_SecureChannels_InputEvent scEvent;
    switch (msgType)
    {
    case SOPC_MSG_TYPE_HEL:
        scEvent = INT_SC_RCV_HEL;
        break;
    case SOPC_MSG_TYPE_ACK:
        scEvent = INT_SC_RCV_ACK;
        break;
    case SOPC_MSG_TYPE_ERR:
        scEvent = INT_SC_RCV_ERR;
        break;
    case SOPC_MSG_TYPE_SC_OPN:
        scEvent = INT_SC_RCV_OPN;
        break;
    case SOPC_MSG_TYPE_SC_CLO:
        scEvent = INT_SC_RCV_CLO;
        break;
    case SOPC_MSG_TYPE_SC_MSG:
        scEvent = INT_SC_RCV_MSG_CHUNKS;
        break;
    default:
        assert(false);
    }
    return scEvent;
}

static bool SC_Chunks_IsMsgEncrypted(OpcUa_MessageSecurityMode securityMode, bool isOPN)
{
    assert(securityMode != OpcUa_MessageSecurityMode_Invalid);
    bool toEncrypt = true;
    // Determine if the message must be encrypted
    if (securityMode == OpcUa_MessageSecurityMode_None ||
        (securityMode == OpcUa_MessageSecurityMode_Sign && false == isOPN))
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
                                                            SOPC_Certificate** clientSenderCertificate,
                                                            bool* receiverCertificatePresence,
                                                            SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConnection->cryptoProvider != NULL);
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);
    assert(senderCertificatePresence != NULL && receiverCertificatePresence != NULL);
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
    const SOPC_Certificate* runningAppCert = NULL;
    const SOPC_PKIProvider* pkiProvider = NULL;
    SOPC_ByteString senderCertificate;
    SOPC_ByteString_Initialize(&senderCertificate);
    SOPC_ByteString receiverCertThumb;
    SOPC_ByteString_Initialize(&receiverCertThumb);
    uint32_t tmpLength = 0;

    if (false == scConnection->isServerConnection)
    {
        // CLIENT side: config is mandatory and security mode to be enforced
        assert(scConfig != NULL);
        runningAppCert = scConfig->crt_cli;
        pkiProvider = scConfig->pki;
        enforceSecuMode = true;
        if (scConfig->crt_srv != NULL)
        {
            // retrieve expected sender certificate as a ByteString
            status = SOPC_KeyManager_Certificate_CopyDER(scConfig->crt_srv, &otherBsAppCert.Data, &tmpLength);
            if (SOPC_STATUS_OK == status && tmpLength > 0)
            {
                otherBsAppCert.Length = (int32_t) tmpLength;
            }
        }
    }
    else
    {
        // SERVER side: client config could be defined or not (new secure channel opening)
        assert(epConfig != NULL);
        runningAppCert = epConfig->serverCertificate;
        pkiProvider = epConfig->pki;
        if (scConfig != NULL)
        {
            enforceSecuMode = true;
            if (scConfig->crt_cli != NULL)
            {
                // retrieve expected sender certificate as a ByteString
                status = SOPC_KeyManager_Certificate_CopyDER(scConfig->crt_cli, &otherBsAppCert.Data, &tmpLength);
                if (SOPC_STATUS_OK == status && tmpLength > 0)
                {
                    otherBsAppCert.Length = (int32_t) tmpLength;
                }
            }
        }
    }

    // Retrieve encryption and signature configuration expected if defined
    if (enforceSecuMode != false)
    {
        toEncrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, true);
        toSign = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);
    }

    // Sender Certificate:
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ByteString_Read(&senderCertificate, scConnection->chunksCtx.chunkInputBuffer);

        if (SOPC_STATUS_OK != status)
        {
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym cert) sender certificate decoding error\n");
            }
        }
    }

    if (SOPC_STATUS_OK == status)
    {
        if (false == toSign && senderCertificate.Length > 0)
        {
            status = SOPC_STATUS_NOK;
            // Table 27 part 6: "field shall be null if the Message is not signed"
            *errorStatus = OpcUa_BadCertificateUseNotAllowed;
            *senderCertificatePresence = true;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym cert) sender certificate presence not expected\n");
            }
        }
        else if (toSign != false && senderCertificate.Length > 0)
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
                    status = OpcUa_BadCertificateInvalid;

                    if (SOPC_DEBUG_PRINTING != false)
                    {
                        printf("ChunksMgr error:(asym cert) sender certificate is not the one expected\n");
                    }
                }
            }

            if (SOPC_STATUS_OK == status)
            {
                SOPC_Certificate* cert = NULL;
                status =
                    SOPC_KeyManager_Certificate_CreateFromDER(senderCertificate.Data, senderCertificate.Length, &cert);
                if (SOPC_STATUS_OK == status)
                {
                    status = SOPC_CryptoProvider_Certificate_Validate(scConnection->cryptoProvider, pkiProvider, cert);
                }
                if (SOPC_STATUS_OK != status)
                {
                    *errorStatus = OpcUa_BadTcpInternalError;

                    if (SOPC_DEBUG_PRINTING != false)
                    {
                        printf("ChunksMgr error:(asym cert) sender certificate validation failed\n");
                    }
                }

                if (false == scConnection->isServerConnection)
                {
                    if (NULL != cert)
                        SOPC_KeyManager_Certificate_Free(cert);
                }
                else
                {
                    // SERVER SIDE ONLY
                    if (SOPC_STATUS_OK == status)
                    {
                        // Set client application certificate to record
                        *clientSenderCertificate = cert;
                    }
                    else
                    {
                        // Error case
                        if (NULL != cert)
                        {
                            SOPC_KeyManager_Certificate_Free(cert);
                        }
                    }
                }
            }
        }
        else if (false == enforceSecuMode || false == toSign)
        {
            // Without security mode to enforce, sender certificate absence could be normal
            *senderCertificatePresence = false;
        }
        else
        {
            status = SOPC_STATUS_NOK;
            // Sender certificate was expected
            *errorStatus = OpcUa_BadCertificateInvalid;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym cert) sender certificate presence expected\n");
            }
        }
    }
    else
    {
        // status == STATUS_NOK
        *errorStatus = OpcUa_BadTcpInternalError;

        if (SOPC_DEBUG_PRINTING != false)
        {
            printf("ChunksMgr error:(asym cert) certificate copy error\n");
        }
    }

    // Receiver Certificate Thumbprint:
    if (SOPC_STATUS_OK == status)
    {
        status = SOPC_ByteString_Read(&receiverCertThumb, scConnection->chunksCtx.chunkInputBuffer);

        if (SOPC_STATUS_OK == status)
        {
            if (false == toEncrypt && receiverCertThumb.Length > 0)
            {
                // Table 27 part 6: "field shall be null if the Message is not encrypted"
                *errorStatus = OpcUa_BadCertificateUseNotAllowed;
                *receiverCertificatePresence = true;

                if (SOPC_DEBUG_PRINTING != false)
                {
                    printf("ChunksMgr error:(asym cert) receiver thumbprint presence not expected\n");
                }
            }
            else if (toEncrypt != false && receiverCertThumb.Length > 0)
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
                        status = SOPC_ByteString_InitializeFixedSize(&curAppCertThumbprint, (int32_t) thumbprintLength);
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

                                    if (SOPC_DEBUG_PRINTING != false)
                                    {
                                        printf("ChunksMgr error:(asym cert) invalid receiver thumbprint\n");
                                    }
                                }
                            }
                            else
                            {
                                *errorStatus = OpcUa_BadTcpInternalError;

                                if (SOPC_DEBUG_PRINTING != false)
                                {
                                    printf("ChunksMgr error:(asym cert) thumbprint computation failed\n");
                                }
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

                        if (SOPC_DEBUG_PRINTING != false)
                        {
                            printf("ChunksMgr error:(asym cert) invalid thumbprint size\n");
                        }
                    }
                } // if thumbprint length correctly computed

                SOPC_ByteString_Clear(&curAppCertThumbprint);
            }
            else if (false == enforceSecuMode || false == toEncrypt)
            { // if toEncrypt
                // Without security mode to enforce, absence could be normal
                *receiverCertificatePresence = false;
            }
            else
            {
                status = SOPC_STATUS_NOK;
                // absence was not expected
                *errorStatus = OpcUa_BadCertificateInvalid;

                if (SOPC_DEBUG_PRINTING != false)
                {
                    printf("ChunksMgr error:(asym cert) thumbprint presence expected\n");
                }
            }
        }
        else
        { // if decoded thumbprint
            *errorStatus = OpcUa_BadTcpInternalError;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym cert) receiver thumbprint decoding error \n");
            }
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
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);

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
    SOPC_Certificate* clientCertificate = NULL;
    int32_t compareRes = -1;
    uint32_t idx = 0;

    if (false == scConnection->isServerConnection)
    {
        // CLIENT side
        clientConfig = SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
        if (NULL == clientConfig)
        {
            result = false;
            *errorStatus = OpcUa_BadInvalidState;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym header) SC configuration not found\n");
            }
        }
    }
    else
    {
        // SERVER side
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

                if (SOPC_DEBUG_PRINTING != false)
                {
                    printf("ChunksMgr error:(asym header) SC configuration not found\n");
                }
            }
        }
    }

    if (result != false)
    {
        // Decode security policy
        status = SOPC_String_Read(&securityPolicy, chunkCtx->chunkInputBuffer);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
            *errorStatus = OpcUa_BadDecodingError;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym header) security policy decoding failed\n");
            }
        }
    }

    if (result != false)
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

                if (SOPC_DEBUG_PRINTING != false)
                {
                    printf("ChunksMgr error:(asym header) security policy value unexpected\n");
                }
            }
            else
            {
                validSecuPolicy = clientConfig->reqSecuPolicyUri;
            }
        }
        else
        {
            assert(scConnection->isServerConnection != false);
            // SERVER side (shall comply with one server security configuration)
            compareRes = -1;
            for (idx = 0; idx < serverConfig->nbSecuConfigs && compareRes != 0; idx++)
            {
                SOPC_SecurityPolicy* secuPolicy = &(serverConfig->secuConfigurations[idx]);
                status = SOPC_String_Compare(&securityPolicy, &secuPolicy->securityPolicy, true, &compareRes);
                if (SOPC_STATUS_OK == status)
                {
                    if (compareRes == 0)
                    {
                        validSecuPolicy = SOPC_String_GetRawCString(&secuPolicy->securityPolicy);
                        validSecuModes = secuPolicy->securityModes;
                    }
                }
            }
        }
        // Rejected if not compatible with security polic-y/ies expected
        if (compareRes != 0)
        {
            result = false;
            *errorStatus = OpcUa_BadSecurityPolicyRejected;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym header) security policy rejected\n");
            }
        }
    }

    if (false != result && NULL == scConnection->cryptoProvider)
    {
        scConnection->cryptoProvider = SOPC_CryptoProvider_Create(validSecuPolicy);
        if (NULL == scConnection->cryptoProvider)
        {
            // Rejected by the cryptographic component
            result = false;
            *errorStatus = OpcUa_BadSecurityPolicyRejected;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym header) security policy invalid\n");
            }
        }
    }

    if (false != result)
    {
        result = SC_Chunks_DecodeAsymSecurityHeader_Certificates(scConnection, serverConfig, clientConfig,
                                                                 &senderCertifPresence, &clientCertificate,
                                                                 &receiverCertifThumbprintPresence, errorStatus);

        if (result == false)
        {
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym header) certificates decoding failed\n");
            }
        }
    }

    if (false != result)
    {
        // Since the security mode could be unknown before decoding (and possibly decrypting) the message
        // we have to deduce it from the certificates presence (None or SignAndEncrypt only possible in OPN)
        if (false == senderCertifPresence && false == receiverCertifThumbprintPresence)
        {
            isSecureModeActive = false;
        }
        else if (senderCertifPresence != false && receiverCertifThumbprintPresence != false)
        {
            isSecureModeActive = true;
        }
        else
        {
            result = false;
            *errorStatus = OpcUa_BadCertificateInvalid;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error:(asym header) certificates presence constraints not verified\n");
            }
        }

        // In case secure channel config is already done (RENEW), check it is correct
        if (false != result && clientConfig != NULL)
        {
            if (false == isSecureModeActive && clientConfig->msgSecurityMode == OpcUa_MessageSecurityMode_None)
            {
                // OK it is compatible
            }
            else if (isSecureModeActive != false &&
                     (clientConfig->msgSecurityMode == OpcUa_MessageSecurityMode_Sign ||
                      clientConfig->msgSecurityMode == OpcUa_MessageSecurityMode_SignAndEncrypt))
            {
                // OK it is compatible
            }
            else
            {
                // Incompatible parameters with already configured security mode
                result = false;
                *errorStatus = OpcUa_BadSecurityChecksFailed;

                if (SOPC_DEBUG_PRINTING != false)
                {
                    printf("ChunksMgr error:(asym header) security mode constraints not verified\n");
                }
            }
        }
    }

    SOPC_String_Clear(&tmpStr);
    SOPC_String_Clear(&securityPolicy);

    if (false != result)
    {
        *isSecurityActive = isSecureModeActive;
        if (NULL == clientConfig && scConnection->isServerConnection != false)
        {
            // SERVER side and only for a new secure channel (<= NULL == clientConfig):
            // - fill temporary secu data necessary to terminate OPN treatment

            scConnection->serverAsymmSecuInfo.clientCertificate = clientCertificate;
            scConnection->serverAsymmSecuInfo.securityPolicyUri = validSecuPolicy;
            scConnection->serverAsymmSecuInfo.validSecurityModes = validSecuModes;
            scConnection->serverAsymmSecuInfo.isSecureModeActive = isSecureModeActive;
        }
    }

    if (false == result && scConnection->state != SECURE_CONNECTION_STATE_SC_CONNECTED &&
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
        if (false == isServerConnection)
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
                                                   bool* isPrecCryptoData,
                                                   SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);

    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    uint32_t tokenId = 0;
    bool result = true;
    bool isTokenValid = false;
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    status = SOPC_UInt32_Read(&tokenId, chunkCtx->chunkInputBuffer);

    if (SOPC_STATUS_OK == status)
    {
        if (false == scConnection->isServerConnection)
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
                *isPrecCryptoData = false;

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
                *isPrecCryptoData = true;

                // Check token expiration
                isTokenValid =
                    SC_Chunks_IsSecuTokenValid(scConnection->isServerConnection, scConnection->precedentSecurityToken);
            }
            else
            {
                result = false;
                *errorStatus = OpcUa_BadSecureChannelTokenUnknown;
            }

            if (true == result && isTokenValid == false)
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
                if (false == scConnection->serverNewSecuTokenActive)
                {
                    // In case of first use of new security token, we shall enforce the old one cannot be used
                    // anymore
                    scConnection->serverNewSecuTokenActive = true;
                }
                *isPrecCryptoData = false;

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
                *isPrecCryptoData = true;

                // Check token expiration
                isTokenValid =
                    SC_Chunks_IsSecuTokenValid(scConnection->isServerConnection, scConnection->precedentSecurityToken);
            }
            else
            {
                result = false;
                *errorStatus = OpcUa_BadSecureChannelTokenUnknown;
            }

            if (true == result && isTokenValid == false)
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

    if (false == isOPN)
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
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool result = true;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    uint32_t seqNumber = 0;

    status = SOPC_UInt32_Read(&seqNumber, chunkCtx->chunkInputBuffer);

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

static bool SC_Chunks_CheckSequenceHeaderRequestId(SOPC_SecureConnection* scConnection,
                                                   bool isClient,
                                                   SOPC_Msg_Type receivedMsgType,
                                                   uint32_t* requestId,
                                                   SOPC_StatusCode* errorStatus)
{
    assert(scConnection != NULL);
    assert(scConnection->chunksCtx.chunkInputBuffer != NULL);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool result = true;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;

    SOPC_Msg_Type* recordedMsgType = NULL;

    // Retrieve request id
    status = SOPC_UInt32_Read(requestId, chunkCtx->chunkInputBuffer);
    if (SOPC_STATUS_OK == status)
    {
        if (isClient != false)
        {
            // Check received request Id was expected for the received message type
            recordedMsgType = SOPC_SLinkedList_RemoveFromId(scConnection->tcpSeqProperties.sentRequestIds, *requestId);
            if (recordedMsgType != NULL)
            {
                if (*recordedMsgType != receivedMsgType)
                {
                    result = false;
                    *errorStatus = OpcUa_BadSecurityChecksFailed;
                }
                free(recordedMsgType);
            }
            else
            {
                result = false;
                *errorStatus = OpcUa_BadSecurityChecksFailed;
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

static bool SC_Chunks_DecryptMsg(SOPC_SecureConnection* scConnection, bool isSymmetric, uint32_t isPrecCryptoData)
{
    assert(scConnection != NULL);
    SOPC_Buffer* encryptedBuffer = scConnection->chunksCtx.chunkInputBuffer;
    assert(encryptedBuffer != NULL);
    // Current position is SN position
    uint32_t sequenceNumberPosition = encryptedBuffer->position;

    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    uint32_t decryptedTextLength = 0;
    SOPC_Buffer* plainBuffer = NULL;

    SOPC_Byte* dataToDecrypt = &(encryptedBuffer->data[sequenceNumberPosition]);
    uint32_t lengthToDecrypt = encryptedBuffer->length - sequenceNumberPosition;

    if (false == isSymmetric)
    {
        const SOPC_AsymmetricKey* runningAppPrivateKey = NULL;
        if (false == scConnection->isServerConnection)
        {
            SOPC_SecureChannel_Config* scConfig =
                SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            assert(scConfig != NULL);
            runningAppPrivateKey = scConfig->key_priv_cli;
        }
        else
        {
            SOPC_Endpoint_Config* epConfig =
                SOPC_ToolkitServer_GetEndpointConfig(scConnection->serverEndpointConfigIdx);
            assert(epConfig != NULL);
            runningAppPrivateKey = epConfig->serverKey;
        }

        if (runningAppPrivateKey != NULL)
        {
            status = SOPC_CryptoProvider_AsymmetricGetLength_Decryption(
                scConnection->cryptoProvider, runningAppPrivateKey, lengthToDecrypt, &decryptedTextLength);
            if (SOPC_STATUS_OK == status)
            {
                result = true;
            }
        }

        if (result != false && decryptedTextLength <= scConnection->tcpMsgProperties.receiveBufferSize)
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
            if (result != false)
            {
                status = SOPC_CryptoProvider_AsymmetricDecrypt(
                    scConnection->cryptoProvider, dataToDecrypt, lengthToDecrypt, runningAppPrivateKey,
                    &(plainBuffer->data[sequenceNumberPosition]), decryptedTextLength, &decryptedTextLength);
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
        SOPC_SC_SecurityKeySet* receiverKeySet = NULL;
        if (false == isPrecCryptoData)
        {
            receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
        }
        else
        {
            receiverKeySet = scConnection->precedentSecuKeySets.receiverKeySet;
        }

        status = SOPC_CryptoProvider_SymmetricGetLength_Decryption(scConnection->cryptoProvider, lengthToDecrypt,
                                                                   &decryptedTextLength);

        if (SOPC_STATUS_OK == status)
        {
            result = true;
        }

        if (result != false && decryptedTextLength <= scConnection->tcpMsgProperties.receiveBufferSize)
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
            if (result != false)
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

    if (false == result)
    {
        // Clear all buffers
        SOPC_Buffer_Delete(scConnection->chunksCtx.chunkInputBuffer);
        scConnection->chunksCtx.chunkInputBuffer = NULL;

        if (plainBuffer != NULL)
        {
            SOPC_Buffer_Delete(plainBuffer);
        }
    }
    else
    {
        SOPC_Buffer_Delete(scConnection->chunksCtx.chunkInputBuffer);
        // Replace input buffer with the plain buffer (position == SN position)
        scConnection->chunksCtx.chunkInputBuffer = plainBuffer;
    }

    return result;
}

static bool SC_Chunks_VerifyMsgSignature(SOPC_SecureConnection* scConnection, bool isSymmetric, bool isPrecCryptoData)
{
    assert(scConnection != NULL);
    SOPC_Buffer* buffer = scConnection->chunksCtx.chunkInputBuffer;
    assert(buffer != NULL);

    bool result = false;

    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    uint32_t signatureSize = 0;
    uint32_t signaturePosition = 0;

    if (false == isSymmetric)
    {
        SOPC_AsymmetricKey* publicKey = NULL;
        const SOPC_Certificate* otherAppCertificate = NULL;
        SOPC_SecureChannel_Config* scConfig = NULL;

        if (false == scConnection->isServerConnection)
        {
            scConfig = SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
        }
        else
        {
            scConfig = SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
        }

        if (NULL == scConfig && scConnection->isServerConnection != false)
        {
            // Server side for new OPN: we have to use the temporary stored certificate value
            otherAppCertificate = scConnection->serverAsymmSecuInfo.clientCertificate;
        }
        else if (scConfig != NULL)
        {
            // Client side or Server side in case of OPN renew
            if (false == scConnection->isServerConnection)
            {
                otherAppCertificate = scConfig->crt_srv;
            }
            else
            {
                otherAppCertificate = scConfig->crt_cli;
            }
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
                                                          publicKey, &(buffer->data[signaturePosition]), signatureSize);
        }

        SOPC_KeyManager_AsymmetricKey_Free(publicKey);
    }
    else
    {
        SOPC_SC_SecurityKeySet* receiverKeySet = NULL;
        if (false == isPrecCryptoData)
        {
            receiverKeySet = scConnection->currentSecuKeySets.receiverKeySet;
        }
        else
        {
            receiverKeySet = scConnection->precedentSecuKeySets.receiverKeySet;
        }

        status = SOPC_CryptoProvider_SymmetricGetLength_Signature(scConnection->cryptoProvider, &signatureSize);

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
        result = true;
    }

    return result;
}

static bool SC_Chunks_TreatTcpPayload(SOPC_SecureConnection* scConnection,
                                      uint32_t* requestId,
                                      SOPC_StatusCode* errorStatus)
{
    assert(requestId != NULL);

    bool result = true;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;
    // Note: we do not treat multiple chunks => guaranteed by HEL/ACK exchanged (chunk config)
    assert(chunkCtx->currentMsgIsFinal == SOPC_MSG_ISFINAL_FINAL);

    SOPC_ReturnStatus status = SOPC_STATUS_OK;
    bool asymmSecuHeader = false;
    bool symmSecuHeader = false;
    bool sequenceHeader = false;
    bool hasSecureChannelId = false;
    bool isOPN = false;

    uint32_t secureChannelId = 0;

    bool toDecrypt = false;
    bool toCheckSignature = false;
    bool isPrecCryptoData = false;

    SOPC_SecureChannel_Config* scConfig = NULL;

    // Note: for non secure message we already check those messages are expected
    //       regarding the connection type (client/server)
    switch (chunkCtx->currentMsgType)
    {
    case SOPC_MSG_TYPE_HEL:
        if (false == scConnection->isServerConnection || chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            // A client shall not receive a HELLO message
            // or HELLO message chunk shall be final
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error : invalid message HEL received \n");
            }
        }
        // Nothing to do: whole payload to transmit to the secure connection state manager
        break;
    case SOPC_MSG_TYPE_ACK:
        if (scConnection->isServerConnection != false || chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            // A server shall not receive an ACK message
            // or ACK message chunk shall be final
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error : invalid message ACK received \n");
            }
        }
        // Nothing to do: whole payload to transmit to the secure connection state manager
        break;
    case SOPC_MSG_TYPE_ERR:
        if (scConnection->isServerConnection != false || chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            // A server shall not receive an ERROR message
            // or ERR message chunk shall be final
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error : invalid message ERR received \n");
            }
        }
        // Nothing to do: whole payload to transmit to the secure connection state manager
        break;
    case SOPC_MSG_TYPE_SC_OPN:
        if (chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            // OPN message chunk shall be final
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error : invalid message OPN received \n");
            }
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
        if (false == scConnection->isServerConnection || chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_FINAL)
        {
            result = false;
            *errorStatus = OpcUa_BadTcpMessageTypeInvalid;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error : invalid message CLO received \n");
            }
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

    if (false != result && hasSecureChannelId != false)
    {
        // Decode secure channel id
        status = SOPC_UInt32_Read(&secureChannelId, chunkCtx->chunkInputBuffer);
        assert(SOPC_STATUS_OK == status);
        if (isOPN != false)
        {
            if (scConnection->currentSecurityToken.secureChannelId == 0)
            {
                /* It is a new secure channel (security token not defined => new OPN), value shall be:
                 * - 0 from client (server case)
                 * - not 0 from server (client case)
                 */
                if (scConnection->isServerConnection == false)
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

                        if (SOPC_DEBUG_PRINTING != false)
                        {
                            printf("ChunksMgr error: server provided invalid initial secure channel Id\n");
                        }
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

                        if (SOPC_DEBUG_PRINTING != false)
                        {
                            printf("ChunksMgr error: client provided invalid initial secure channel Id\n");
                        }
                    } // else: secure channel Id == 0 from client expected, then new Id set by server in OPN response
                }
            }
            else if (scConnection->currentSecurityToken.secureChannelId != secureChannelId)
            {
                // Error: it shall be the expected secure channel Id
                result = false;
                *errorStatus = OpcUa_BadTcpSecureChannelUnknown;

                if (SOPC_DEBUG_PRINTING != false)
                {
                    printf("ChunksMgr error: invalid secure channel Id\n");
                }
            }
        }
        else
        {
            if (scConnection->currentSecurityToken.secureChannelId != secureChannelId)
            {
                // Error: it shall be the expected secure channel Id when not an OPN
                result = false;
                *errorStatus = OpcUa_BadTcpSecureChannelUnknown;

                if (SOPC_DEBUG_PRINTING != false)
                {
                    printf("ChunksMgr error: invalid secure channel Id\n");
                }
            }
        }
    }

    if (result != false && asymmSecuHeader != false)
    {
        // OPN case: asymmetric secu header
        bool isSecurityActive = false;
        result = SC_Chunks_CheckAsymmetricSecurityHeader(scConnection, &isSecurityActive, errorStatus);
        if (result != false)
        {
            toDecrypt = isSecurityActive;
            toCheckSignature = isSecurityActive;
            isPrecCryptoData = false; // asymmetric => unused parameter in decrypt / check sign
        }
        else
        {
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error: asymmetric security header verification failed\n");
            }
        }
    }

    if (result != false && symmSecuHeader != false)
    {
        // CLO or MSG case: symmetric security header
        if (false == scConnection->isServerConnection)
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

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error : SC configuration not found \n");
            }
        }
        else
        {
            result = SC_Chunks_CheckSymmetricSecurityHeader(scConnection, &isPrecCryptoData, errorStatus);
            if (result != false)
            {
                toDecrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, isOPN);
                toCheckSignature = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);
            }
            else
            {
                if (SOPC_DEBUG_PRINTING != false)
                {
                    printf("ChunksMgr error: symmetric security header verification failed\n");
                }
            }
        }
    }

    if (result != false && toDecrypt != false)
    {
        // Decrypt the message
        result = SC_Chunks_DecryptMsg(scConnection,
                                      false == isOPN, // isSymmetric
                                      isPrecCryptoData);
        if (false == result)
        {
            *errorStatus = OpcUa_BadSecurityChecksFailed;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error: decryption failed\n");
            }
        }
    }

    if (result != false && toCheckSignature != false)
    {
        // Check decrypted message signature
        result = SC_Chunks_VerifyMsgSignature(scConnection,
                                              false == isOPN, // isSymmetric
                                              isPrecCryptoData);
        if (false == result)
        {
            *errorStatus = OpcUa_BadApplicationSignatureInvalid;

            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error: signature verification failed\n");
            }
        }
    }

    if (result != false && sequenceHeader != false)
    {
        result = SC_Chunks_CheckSequenceHeaderSN(scConnection, isOPN, errorStatus);

        if (result != false)
        {
            result = SC_Chunks_CheckSequenceHeaderRequestId(scConnection,
                                                            false == scConnection->isServerConnection, // isClient
                                                            chunkCtx->currentMsgType, requestId, errorStatus);
            if (result == false)
            {
                if (SOPC_DEBUG_PRINTING != false)
                {
                    printf("ChunksMgr error: request Id verification failed\n");
                }
            }
        }
        else
        {
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error: SN verification failed\n");
            }
        }
    }

    return result;
}

static void SC_Chunks_TreatReceivedBuffer(SOPC_SecureConnection* scConnection,
                                          uint32_t scConnectionIdx,
                                          SOPC_Buffer* receivedBuffer)
{
    assert(scConnection != NULL);
    assert(receivedBuffer != NULL);
    assert(receivedBuffer->position == 0);

    uint32_t sizeToRead = 0;
    uint32_t sizeAlreadyRead = 0;
    uint32_t sizeAvailable = 0;
    SOPC_StatusCode errorStatus = SOPC_GoodGenericStatus; // Good
    uint32_t requestId = 0;
    bool result = true;
    SOPC_SecureConnection_ChunkMgrCtx* chunkCtx = &scConnection->chunksCtx;

    sizeAvailable = receivedBuffer->length - receivedBuffer->position;
    // Continue until an error occurred OR received buffer is empty (could contain 1 or several messages)
    while (result != false && sizeAvailable > 0)
    {
        if (NULL == chunkCtx->chunkInputBuffer)
        {
            // No incomplete message data: create a new buffer
            chunkCtx->chunkInputBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.receiveBufferSize);
            if (NULL == chunkCtx->chunkInputBuffer)
            {
                errorStatus = OpcUa_BadOutOfMemory;
                result = false;
            }
        }

        if (result != false)
        {
            // OPC UA TCP MESSAGE HEADER TREATMENT
            bool decodeHeader = false;

            if (chunkCtx->chunkInputBuffer->length < SOPC_TCP_UA_HEADER_LENGTH)
            {
                // Message data was already received but not enough to know the message size
                //  => new attempt to retrieve message header containing size

                // Compute size to read to obtain the complete message header
                sizeToRead = SOPC_TCP_UA_HEADER_LENGTH - chunkCtx->chunkInputBuffer->length;
                sizeAvailable = receivedBuffer->length - receivedBuffer->position;

                if (sizeAvailable >= sizeToRead)
                {
                    // Complete header available: retrieve header data from received buffer
                    result =
                        SC_Chunks_ReadDataFromReceivedBuffer(chunkCtx->chunkInputBuffer, receivedBuffer, sizeToRead);

                    if (false == result)
                    {
                        errorStatus = OpcUa_BadTcpMessageTooLarge;

                        if (SOPC_DEBUG_PRINTING != false)
                        {
                            printf("ChunksMgr error : message (header) too large for buffer \n");
                        }
                    }
                    else
                    {
                        // Enough data to decode header
                        decodeHeader = true;
                    }
                }
                else
                {
                    // Complete header not available: retrieve available data from received buffer
                    result =
                        SC_Chunks_ReadDataFromReceivedBuffer(chunkCtx->chunkInputBuffer, receivedBuffer, sizeAvailable);

                    if (false == result)
                    {
                        errorStatus = OpcUa_BadTcpMessageTooLarge;

                        if (SOPC_DEBUG_PRINTING != false)
                        {
                            printf("ChunksMgr error: message (header) too large for buffer \n");
                        }
                    }
                }
            }

            if (result != false && decodeHeader != false)
            {
                // Decode the received OPC UA TCP message header
                result = SC_Chunks_DecodeTcpMsgHeader(&scConnection->chunksCtx);
                if (false == result)
                {
                    errorStatus = OpcUa_BadTcpMessageTypeInvalid;
                    if (SOPC_DEBUG_PRINTING != false)
                    {
                        printf("ChunksMgr error: message type invalid \n");
                    }
                }
                else
                {
                    if (SOPC_DEBUG_PRINTING != false)
                    {
                        printf("ChunksMgr: received TCP UA message type: %d\n", scConnection->chunksCtx.currentMsgType);
                    }
                }
            }
        } /* END OF OPC UA TCP MESSAGE HEADER TREATMENT */

        if (result != false)
        {
            /* OPC UA TCP MESSAGE PAYLOAD TREATMENT */

            bool completePayload = false;
            if (chunkCtx->chunkInputBuffer->length >= SOPC_TCP_UA_HEADER_LENGTH)
            {
                // Message header is decoded but message payload not (completly) retrieved
                //  => attempt to retrieve complete message payload
                assert(chunkCtx->currentMsgSize > 0); // message size was decoded
                assert(chunkCtx->currentMsgType != SOPC_MSG_TYPE_INVALID);
                assert(chunkCtx->currentMsgIsFinal != SOPC_MSG_ISFINAL_INVALID);

                sizeAvailable = receivedBuffer->length - receivedBuffer->position;
                // Incomplete message payload data size already retrieved in input buffer
                sizeAlreadyRead = chunkCtx->chunkInputBuffer->length - chunkCtx->chunkInputBuffer->position;

                if (chunkCtx->currentMsgSize > SOPC_TCP_UA_HEADER_LENGTH + sizeAlreadyRead)
                {
                    sizeToRead = chunkCtx->currentMsgSize - SOPC_TCP_UA_HEADER_LENGTH - sizeAlreadyRead;
                }
                else
                {
                    // Size provided by message seems invalid
                    result = false;
                    errorStatus = OpcUa_BadTcpInternalError; // not really internal no error corresponding

                    if (SOPC_DEBUG_PRINTING != false)
                    {
                        printf("ChunksMgr error : message size invalid \n");
                    }
                }

                if (result != false)
                {
                    if (sizeAvailable >= sizeToRead)
                    {
                        // Complete payload available: retrieve payload data from received buffer
                        result = SC_Chunks_ReadDataFromReceivedBuffer(chunkCtx->chunkInputBuffer, receivedBuffer,
                                                                      sizeToRead);
                        if (false == result)
                        {
                            errorStatus = OpcUa_BadTcpMessageTooLarge;

                            if (SOPC_DEBUG_PRINTING != false)
                            {
                                printf("ChunksMgr error : message too large for buffer \n");
                            }
                        }

                        // Enough data to read complete message
                        completePayload = true;
                    }
                    else
                    {
                        result = SC_Chunks_ReadDataFromReceivedBuffer(chunkCtx->chunkInputBuffer, receivedBuffer,
                                                                      sizeAvailable);

                        if (false == result)
                        {
                            errorStatus = OpcUa_BadTcpMessageTooLarge;

                            if (SOPC_DEBUG_PRINTING != false)
                            {
                                printf("ChunksMgr error: message too large for buffer \n");
                            }
                        }
                    }
                }
            }

            if (result != false && completePayload != false)
            {
                // Decode OPC UA Secure Conversation MessageChunk specific headers if necessary (not HEL/ACK/ERR)
                result = SC_Chunks_TreatTcpPayload(scConnection, &requestId, &errorStatus);
                if (result != false)
                {
                    // Transmit OPC UA message to secure connection state manager
                    SOPC_SecureChannels_InputEvent scEvent = SC_Chunks_MsgTypeToRcvEvent(chunkCtx->currentMsgType);
                    if (scEvent == INT_SC_RCV_ERR || scEvent == INT_SC_RCV_CLO)
                    {
                        // Treat as prio events
                        SOPC_SecureChannels_EnqueueInternalEventAsNext(scEvent, scConnectionIdx,
                                                                       (void*) chunkCtx->chunkInputBuffer, requestId);
                    }
                    else
                    {
                        SOPC_SecureChannels_EnqueueInternalEvent(scEvent, scConnectionIdx,
                                                                 (void*) chunkCtx->chunkInputBuffer, requestId);
                    }
                    chunkCtx->chunkInputBuffer = NULL;
                    // reset chunk context (buffer not deallocated since provided to secure connection state
                    // manager)
                    memset(&scConnection->chunksCtx, 0, sizeof(SOPC_SecureConnection_ChunkMgrCtx));
                }
            }
        } /* END OF OPC UA TCP MESSAGE PAYLOAD TREATMENT */

        if (false == result)
        {
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ChunksMgr error: INT_SC_RCV_FAILURE: %x\n", errorStatus);
            }
            // Treat as prio events
            SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_RCV_FAILURE, scConnectionIdx, NULL, errorStatus);
            SOPC_Buffer_Delete(chunkCtx->chunkInputBuffer);
            // reset chunk context
            memset(&scConnection->chunksCtx, 0, sizeof(SOPC_SecureConnection_ChunkMgrCtx));
            receivedBuffer->length = 0;
            receivedBuffer->position = 0;
        }

        // Update available data remaining in received buffer
        sizeAvailable = receivedBuffer->length - receivedBuffer->position;
    }

    SOPC_Buffer_Delete(receivedBuffer);
}

static bool SC_Chunks_EncodeTcpMsgHeader(SOPC_SecureConnection* scConnection,
                                         SOPC_Msg_Type sendMsgType,
                                         bool isFinalChunk,
                                         SOPC_Buffer* buffer)
{
    assert(scConnection != NULL);
    assert(buffer != NULL);
    bool result = false;
    const uint8_t* msgTypeBytes = NULL;
    uint8_t isFinalChunkByte = 'F';
    uint32_t messageSize = 0; // Could be temporary depending on message type / secu parameters
    SOPC_ReturnStatus status = SOPC_STATUS_OK;

    switch (sendMsgType)
    {
    case SOPC_MSG_TYPE_HEL:
        msgTypeBytes = SOPC_HEL;
        result = (isFinalChunk != false);
        break;
    case SOPC_MSG_TYPE_ACK:
        msgTypeBytes = SOPC_ACK;
        result = (isFinalChunk != false);
        break;
    case SOPC_MSG_TYPE_ERR:
        msgTypeBytes = SOPC_ERR;
        result = (isFinalChunk != false);
        break;
    case SOPC_MSG_TYPE_SC_OPN:
        msgTypeBytes = SOPC_OPN;
        result = (isFinalChunk != false);
        break;
    case SOPC_MSG_TYPE_SC_CLO:
        msgTypeBytes = SOPC_CLO;
        result = (isFinalChunk != false);
        break;
    case SOPC_MSG_TYPE_SC_MSG:
        msgTypeBytes = SOPC_MSG;
        result = true;
        break;
    default:
        assert(false);
    }

    if (result != false)
    {
        status = SOPC_Buffer_Write(buffer, msgTypeBytes, 3);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }
    if (result != false)
    {
        if (false == isFinalChunk)
        {
            // Set intermediate chunk value
            isFinalChunkByte = 'C';
        }
        status = SOPC_Buffer_Write(buffer, &isFinalChunkByte, 1);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }
    if (result != false)
    {
        if (buffer->length >= SOPC_TCP_UA_HEADER_LENGTH)
        {
            messageSize = buffer->length;
        }
        else
        {
            messageSize = SOPC_TCP_UA_HEADER_LENGTH;
        }
        status = SOPC_UInt32_Write(&messageSize, buffer);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }
    }
    return result;
}

static bool SC_Chunks_EncodeAsymSecurityHeader(SOPC_SecureConnection* scConnection,
                                               SOPC_SecureChannel_Config* scConfig,
                                               SOPC_Buffer* buffer,
                                               uint32_t* securityPolicyLength,
                                               uint32_t* senderCertificateSize,
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
    const SOPC_Certificate* receiverCertCrypto = NULL;
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
        *securityPolicyLength = (uint32_t) strSecuPolicy.Length;
    }

    if (result != false)
    {
        status = SOPC_String_Write(&strSecuPolicy, buffer);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
            *errorStatus = OpcUa_BadTcpInternalError;
        }
    }

    // Sender Certificate:
    if (result != false)
    {
        const SOPC_Certificate* senderCert = NULL;
        uint32_t length = 0;
        if (false == scConnection->isServerConnection)
        {
            // Client side
            senderCert = scConfig->crt_cli;
        }
        else
        {
            // Server side
            senderCert = scConfig->crt_srv;
        }
        if (senderCert != NULL)
        {
            status = SOPC_KeyManager_Certificate_CopyDER(senderCert, &bsSenderCert.Data, &length);
            if (SOPC_STATUS_OK == status && length <= INT32_MAX)
            {
                bsSenderCert.Length = (int32_t) length;
                *senderCertificateSize = length;
            }
            else
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
        }
    }
    if (result != false)
    {
        // Note: part 6 v1.03 table 27: This field shall be null if the Message is not signed
        if (toSign != false && bsSenderCert.Length > 0)
        {
            status = SOPC_ByteString_Write(&bsSenderCert, buffer);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
                *errorStatus = OpcUa_BadTcpInternalError;
            }
        }
        else if (false == toSign)
        {
            // Note: foundation stack expects -1 value whereas 0 is also valid:
            const int32_t minusOne = -1;
            status = SOPC_Int32_Write(&minusOne, buffer);
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

    // Receiver Certificate Thumbprint:
    if (result != false)
    {
        // Retrieve correct certificate
        if (false == scConnection->isServerConnection)
        {
            // Client side
            receiverCertCrypto = scConfig->crt_srv;
        }
        else
        {
            // Server side
            receiverCertCrypto = scConfig->crt_cli;
        }

        // Note: part 6 v1.03 table 27: This field shall be null if the Message is not encrypted
        if (toEncrypt != false && receiverCertCrypto != NULL)
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

            if (result != false)
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
            if (result != false)
            {
                status = SOPC_KeyManager_Certificate_GetThumbprint(scConnection->cryptoProvider, receiverCertCrypto,
                                                                   recCertThumbprint.Data, thumbprintLength);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
            if (result != false)
            {
                status = SOPC_ByteString_Write(&recCertThumbprint, buffer);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
            SOPC_ByteString_Clear(&recCertThumbprint);
        }
        else if (false == toEncrypt)
        {
            // Note: foundation stack expects -1 value whereas 0 is also valid:
            const int32_t minusOne = -1;
            status = SOPC_Int32_Write(&minusOne, buffer);
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
                                             uint32_t signatureSize)
{
    uint32_t result = 0;
    uint32_t paddingSizeFields = 0;
    if (false == toEncrypt)
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
        if (SC_Chunks_Is_ExtraPaddingSizePresent(plainBlockSize) != false)
        {
            paddingSizeFields += 1;
        }
    }

    if (false == toSign)
    {
        signatureSize = 0;
    }

    // Ensure cipher block size is greater or equal to plain block size:
    //  otherwise the plain size could be greater than the  buffer size regarding computation
    assert(cipherBlockSize >= plainBlockSize);

    // Use formulae of spec 1.03 part 6 §6.7.2 even if controversial (see mantis ticket #2897)
    result =
        plainBlockSize * ((chunkSize - nonEncryptedHeadersSize - signatureSize - paddingSizeFields) / cipherBlockSize) -
        SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH;

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

    if (false == isSymmetricAlgo)
    {
        // ASYMMETRIC CASE

        if (scConfig->msgSecurityMode != OpcUa_MessageSecurityMode_None)
        {
            SOPC_AsymmetricKey* receiverPublicKey = NULL;
            SOPC_AsymmetricKey* senderPublicKey = NULL;
            const SOPC_Certificate* receiverAppCertificate = NULL;
            const SOPC_Certificate* senderAppCertificate = NULL;

            // Asymmetric case: used only for opening channel, signature AND encryption mandatory in this case
            *toEncrypt = true;
            *toSign = true;

            if (false == scConnection->isServerConnection)
            {
                // Client side
                senderAppCertificate = scConfig->crt_cli;
                receiverAppCertificate = scConfig->crt_srv;
            }
            else
            {
                // Server side
                senderAppCertificate = scConfig->crt_srv;
                receiverAppCertificate = scConfig->crt_cli;
            }

            status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(senderAppCertificate, &senderPublicKey);
            if (SOPC_STATUS_OK != status)
            {
                result = false;
            }

            if (result != false)
            {
                status =
                    SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(receiverAppCertificate, &receiverPublicKey);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }

            if (result != false)
            {
                // Compute block sizes (using receiver key => encryption with other application public key)
                status = SOPC_CryptoProvider_AsymmetricGetLength_Msgs(scConnection->cryptoProvider, receiverPublicKey,
                                                                      cipherTextBlockSize, plainTextBlockSize);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }
            if (result != false)
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

            if (result != false)
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
                                                bool isSymmetric)
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
    if (result != false)
    {
        maxBodySize = SC_Chunks_ComputeMaxBodySize(nonEncryptedHeadersSize, chunkSize, toEncrypt, cipherBlockSize,
                                                   plainBlockSize, toSign, signatureSize);
    }

    return maxBodySize;
}

static uint16_t SC_Chunks_GetPaddingSize(
    uint32_t bytesToEncrypt, // called bytesToWrite in spec part 6 but it should not since it includes SN header !
    uint32_t plainBlockSize,
    uint32_t signatureSize)
{
    // By default only 1 padding size field + 1 if extra padding
    uint8_t paddingSizeFields = 1;
    if (SC_Chunks_Is_ExtraPaddingSizePresent(plainBlockSize))
    {
        paddingSizeFields += 1;
    }
    return plainBlockSize - ((bytesToEncrypt + signatureSize + paddingSizeFields) % plainBlockSize);
}

static bool SOPC_Chunks_EncodePadding(SOPC_SecureConnection* scConnection,
                                      SOPC_SecureChannel_Config* scConfig,
                                      SOPC_Buffer* buffer,
                                      bool isSymmetricAlgo,
                                      uint32_t sequenceNumberPosition,
                                      uint32_t* signatureSize,
                                      uint16_t* realPaddingLength, // >= paddingSizeField
                                      bool* hasExtraPadding)
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

    if (result != false && toEncrypt != false)
    {
        *realPaddingLength =
            SC_Chunks_GetPaddingSize(buffer->length - sequenceNumberPosition, plainTextBlockSize, *signatureSize);
        // Little endian conversion of padding:
        SOPC_EncodeDecode_UInt16(realPaddingLength);
        status = SOPC_Buffer_Write(buffer, (SOPC_Byte*) realPaddingLength, 1);
        if (SOPC_STATUS_OK != status)
        {
            result = false;
        }

        if (result != false)
        {
            uint8_t paddingSizeField = 0;
            paddingSizeField = 0xFF & *realPaddingLength;
            // The value of each byte of the padding is equal to paddingSize:
            SOPC_Byte* paddingBytes = malloc(sizeof(SOPC_Byte) * (*realPaddingLength));
            if (paddingBytes != NULL)
            {
                memset(paddingBytes, paddingSizeField, *realPaddingLength);
                status = SOPC_Buffer_Write(buffer, paddingBytes, *realPaddingLength);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
                free(paddingBytes);
            }
            else
            {
                result = false;
            }
        }

        // Extra-padding necessary if padding could be greater 256 bytes
        if (result != false && SC_Chunks_Is_ExtraPaddingSizePresent(plainTextBlockSize) != false)
        {
            *hasExtraPadding = 1; // True
            // extra padding = most significant byte of 2 bytes padding size
            SOPC_Byte extraPadding = 0x00FF & *realPaddingLength;
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

    return result;
}

static bool SC_Chunks_CheckMaxSenderCertificateSize(uint32_t senderCertificateSize,
                                                    uint32_t messageChunkSize,
                                                    uint32_t securityPolicyUriLength,
                                                    bool hasPadding,
                                                    uint32_t realPaddingLength,
                                                    bool hasExtraPadding,
                                                    uint32_t asymmetricSignatureSize)
{
    bool result = false;
    int32_t maxSize = // Fit in a single message chunk with at least 1 byte of body
        messageChunkSize - SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH - 4 - // URI length field size
        securityPolicyUriLength - 4 -                                 // Sender certificate length field
        4 -                                                           // Receiver certificate thumbprint length field
        20 -                                                          // Receiver certificate thumbprint length
        8;                                                            // Sequence header size
    if (hasPadding != false)
    {
        maxSize = maxSize - 1 - // padding length field size
                  realPaddingLength;
        if (hasExtraPadding)
        {
            // ExtraPaddingSize field size to remove
            maxSize = maxSize - 1;
        }
    }
    maxSize = maxSize - asymmetricSignatureSize;

    if (senderCertificateSize <= (uint32_t) maxSize)
    {
        result = true;
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

    if (false == isSymmetricAlgo)
    {
        const SOPC_Certificate* otherAppCertificate = NULL;
        if (false == scConnection->isServerConnection)
        {
            // Client side
            otherAppCertificate = scConfig->crt_srv;
        }
        else
        {
            // Server side
            otherAppCertificate = scConfig->crt_cli;
        }
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
            if (result != false)
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

static bool SC_Chunks_EncodeSignature(SOPC_SecureConnection* scConnection,
                                      SOPC_Buffer* buffer,
                                      bool symmetricAlgo,
                                      uint32_t signatureSize)
{
    bool result = false;
    SOPC_ReturnStatus status = SOPC_STATUS_NOK;
    SOPC_ByteString signedData;

    if (false == symmetricAlgo)
    {
        const SOPC_AsymmetricKey* runningAppPrivateKey = NULL;
        if (false == scConnection->isServerConnection)
        {
            SOPC_SecureChannel_Config* scConfig =
                SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            assert(scConfig != NULL);
            runningAppPrivateKey = scConfig->key_priv_cli;
        }
        else
        {
            SOPC_Endpoint_Config* epConfig =
                SOPC_ToolkitServer_GetEndpointConfig(scConnection->serverEndpointConfigIdx);
            assert(epConfig != NULL);
            runningAppPrivateKey = epConfig->serverKey;
        }

        if (runningAppPrivateKey != NULL)
        {
            status = SOPC_ByteString_InitializeFixedSize(&signedData, signatureSize);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_CryptoProvider_AsymmetricSign(scConnection->cryptoProvider, buffer->data, buffer->length,
                                                        runningAppPrivateKey, signedData.Data, signedData.Length);
        }

        if (SOPC_STATUS_OK == status)
        {
            status = SOPC_Buffer_Write(buffer, signedData.Data, signedData.Length);
        }
        if (SOPC_STATUS_OK == status)
        {
            result = true;
        }
        SOPC_ByteString_Clear(&signedData);
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
            status = SOPC_ByteString_InitializeFixedSize(&signedData, signatureSize);
            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_CryptoProvider_SymmetricSign(scConnection->cryptoProvider, buffer->data, buffer->length,
                                                           scConnection->currentSecuKeySets.senderKeySet->signKey,
                                                           signedData.Data, signedData.Length);
            }

            if (SOPC_STATUS_OK == status)
            {
                status = SOPC_Buffer_Write(buffer, signedData.Data, signedData.Length);
            }
            if (SOPC_STATUS_OK == status)
            {
                result = true;
            }

            SOPC_ByteString_Clear(&signedData);
        }
    }
    return result;
}

static bool SC_Chunks_EncryptMsg(SOPC_SecureConnection* scConnection,
                                 SOPC_Buffer* nonEncryptedBuffer,
                                 bool symmetricAlgo,
                                 uint32_t sequenceNumberPosition,
                                 uint32_t encryptedDataLength,
                                 SOPC_Buffer* encryptedBuffer)
{
    assert(scConnection != NULL);
    assert(nonEncryptedBuffer != NULL);
    assert(encryptedBuffer != NULL);
    bool result = false;

    SOPC_SecureChannel_Config* scConfig = NULL;

    SOPC_ReturnStatus status = SOPC_STATUS_INVALID_PARAMETERS;
    SOPC_Byte* dataToEncrypt = &nonEncryptedBuffer->data[sequenceNumberPosition];
    const uint32_t dataToEncryptLength = nonEncryptedBuffer->length - sequenceNumberPosition;

    if (false == symmetricAlgo)
    {
        /* ASYMMETRIC CASE */

        const SOPC_Certificate* otherAppCertificate = NULL;
        if (false == scConnection->isServerConnection)
        {
            // Client side
            scConfig = SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            assert(scConfig != NULL);
            otherAppCertificate = scConfig->crt_srv;
        }
        else
        {
            // Server side
            scConfig = SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            assert(scConfig != NULL); // Even on server side it is guaranteed by secure connection state manager (no
                                      // sending in wrong state)
            otherAppCertificate = scConfig->crt_cli;
        }

        SOPC_AsymmetricKey* otherAppPublicKey = NULL;
        SOPC_Byte* encryptedData = NULL;

        // Retrieve other app public key from certificate
        status = SOPC_KeyManager_AsymmetricKey_CreateFromCertificate(otherAppCertificate, &otherAppPublicKey);

        // Check size of encrypted data array
        if (SOPC_STATUS_OK == status)
        {
            result = true;
            if (encryptedBuffer->max_size < sequenceNumberPosition + encryptedDataLength)
            {
                result = false;
            }
            encryptedData = encryptedBuffer->data;
            if (NULL == encryptedData)
            {
                result = false;
            }
            else
            {
                // Copy non encrypted headers part
                memcpy(encryptedData, nonEncryptedBuffer->data, sequenceNumberPosition);
                // Set correct message size and encrypted buffer length
                status = SOPC_Buffer_SetDataLength(encryptedBuffer, sequenceNumberPosition + encryptedDataLength);
                assert(SOPC_STATUS_OK == status);
            }
        }

        // Encrypt
        if (result != false)
        {
            status = SOPC_CryptoProvider_AsymmetricEncrypt(scConnection->cryptoProvider, dataToEncrypt,
                                                           dataToEncryptLength, otherAppPublicKey,
                                                           &encryptedData[sequenceNumberPosition], encryptedDataLength);

            if (SOPC_STATUS_OK != status)
            {
                result = false;
            }
        }

        SOPC_KeyManager_AsymmetricKey_Free(otherAppPublicKey);
    }
    else
    {
        /* SYMMETRIC CASE */

        if (NULL == scConnection->currentSecuKeySets.senderKeySet ||
            NULL == scConnection->currentSecuKeySets.receiverKeySet)
        {
            result = false;
        }
        else
        {
            result = true;
            SOPC_Byte* encryptedData = NULL;

            // Check size of encrypted data array
            if (encryptedBuffer->max_size < sequenceNumberPosition + encryptedDataLength)
            {
                result = false;
            }
            encryptedData = encryptedBuffer->data;
            if (NULL == encryptedData)
            {
                result = false;
            }
            else
            {
                // Copy non encrypted headers part
                memcpy(encryptedData, nonEncryptedBuffer->data, sequenceNumberPosition);
                // Set correct message size and encrypted buffer length
                status = SOPC_Buffer_SetDataLength(encryptedBuffer, sequenceNumberPosition + encryptedDataLength);
                assert(SOPC_STATUS_OK == status);
            }

            // Encrypt
            if (result != false)
            {
                status = SOPC_CryptoProvider_SymmetricEncrypt(
                    scConnection->cryptoProvider, dataToEncrypt, dataToEncryptLength,
                    scConnection->currentSecuKeySets.senderKeySet->encryptKey,
                    scConnection->currentSecuKeySets.senderKeySet->initVector, &encryptedData[sequenceNumberPosition],
                    encryptedDataLength);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                }
            }

        } // End valid key set
    }
    return result;
}

static bool SC_Chunks_TreatSendBuffer(SOPC_SecureConnection* scConnection,
                                      uint32_t optRequestId,
                                      SOPC_Msg_Type sendMsgType,
                                      bool isSendTcpOnly,
                                      bool isOPN,
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
    uint32_t bodySize = 0;
    uint32_t sequenceNumberPosition = 0; // Position from which encryption start
    uint32_t tokenId = 0;
    uint32_t senderCertificateSize = 0;
    uint32_t securityPolicyLength = 0;
    uint32_t signatureSize = 0;
    bool hasPadding = false;
    uint16_t realPaddingLength = 0; // padding + extra total size
    bool hasExtraPadding = false;
    uint32_t encryptedDataLength = 0;

    /* PRE-CONFIG PHASE */

    // Set the position at the beginning of the buffer (to be read or to could encode header for which space was
    // left)
    status = SOPC_Buffer_SetPosition(inputBuffer, 0);
    assert(SOPC_STATUS_OK == status);

    if (isOPN != false)
    {
        // In specific case of OPN the input buffer contains only message body
        // (without bytes reserved for headers since it is not static size)
        assert(scConnection->tcpMsgProperties.sendBufferSize > 0);
        nonEncryptedBuffer = SOPC_Buffer_Create(scConnection->tcpMsgProperties.sendBufferSize);
    }
    else
    {
        // In other cases the input buffer contains the message body
        //  but also the reserved bytes for headers before body bytes
        nonEncryptedBuffer = inputBuffer;
    }

    /* ENCODE OPC UA TCP HEADER PHASE */
    result = SC_Chunks_EncodeTcpMsgHeader(scConnection, sendMsgType,
                                          true, // isFinal
                                          nonEncryptedBuffer);
    if (result != false)
    {
        if (false == isSendTcpOnly)
        {
            /* ENCODE OPC UA SECURE CONVERSATION MESSAGE PHASE*/

            // Note: when sending a secure conversation message, the secure connection configuration shall be
            // defined
            if (false == scConnection->isServerConnection)
            {
                scConfig = SOPC_ToolkitClient_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            }
            else
            {
                scConfig = SOPC_ToolkitServer_GetSecureChannelConfig(scConnection->endpointConnectionConfigIdx);
            }
            assert(scConfig != NULL); // Even on server side guaranteed by the secure connection state manager

            bool toEncrypt = SC_Chunks_IsMsgEncrypted(scConfig->msgSecurityMode, isOPN);
            bool toSign = SC_Chunks_IsMsgSigned(scConfig->msgSecurityMode);

            /* ENCODE OPC UA SECURE CONVERSATION MESSAGE EXTRA FIELD (secure channel Id) */
            status = SOPC_UInt32_Write(&scConnection->currentSecurityToken.secureChannelId, nonEncryptedBuffer);
            if (SOPC_STATUS_OK != status)
            {
                *errorStatus = OpcUa_BadEncodingError;
                result = false;
            }

            if (false == isOPN)
            {
                // SYMMETRIC SECURITY CASE
                sequenceNumberPosition =
                    SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH + SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH;

                /* CHECK MAX BODY SIZE */
                assert(scConnection->symmSecuMaxBodySize != 0);
                // Note: buffer already contains the message body (buffer length == end of body)
                bodySize = nonEncryptedBuffer->length - // symm headers
                           (SOPC_UA_SECURE_MESSAGE_HEADER_LENGTH + SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH +
                            SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
                if (bodySize > scConnection->symmSecuMaxBodySize)
                {
                    // Note: we do not manage several chunks for now (expected place to manage it)
                    result = false;
                    if (false == scConnection->isServerConnection)
                    {
                        *errorStatus = OpcUa_BadRequestTooLarge;
                    }
                    else
                    {
                        *errorStatus = OpcUa_BadResponseTooLarge;
                    }
                }

                if (result != false)
                {
                    /* ENCODE SYMMETRIC SECURITY HEADER */
                    //  retrieve tokenId
                    if (scConnection->isServerConnection != false && false == scConnection->serverNewSecuTokenActive)
                    {
                        // Server side only (SC renew): new token is not active yet, use the precedent token
                        // TODO: timeout on precedent token validity to be implemented
                        assert(scConnection->precedentSecurityToken.tokenId != 0 &&
                               scConnection->precedentSecurityToken.secureChannelId != 0);
                        tokenId = scConnection->precedentSecurityToken.tokenId;
                    }
                    else
                    {
                        // Use current token
                        tokenId = scConnection->currentSecurityToken.tokenId;
                    }
                    //  encode tokenId
                    status = SOPC_UInt32_Write(&tokenId, nonEncryptedBuffer);
                    if (SOPC_STATUS_OK != status)
                    {
                        result = false;
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }

                // Set position to end of body
                status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, nonEncryptedBuffer->length);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }
            else
            {
                // ASYMMETRIC SECURITY CASE

                /* ENCODE ASYMMETRIC SECURITY HEADER */
                if (result != false)
                {
                    result =
                        SC_Chunks_EncodeAsymSecurityHeader(scConnection, scConfig, nonEncryptedBuffer,
                                                           &senderCertificateSize, &securityPolicyLength, errorStatus);
                    if (false == result)
                    {
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }

                if (result != false)
                {
                    // Compute max body sizes (asymm. and symm.) and check asymmetric max body size

                    // Next position is the sequence number position
                    sequenceNumberPosition = nonEncryptedBuffer->position;

                    if (scConnection->asymmSecuMaxBodySize == 0 && scConnection->symmSecuMaxBodySize == 0)
                    {
                        scConnection->asymmSecuMaxBodySize = SC_Chunks_GetSendingMaxBodySize(
                            scConnection, scConfig, nonEncryptedBuffer->max_size, sequenceNumberPosition, false);
                        scConnection->symmSecuMaxBodySize = SC_Chunks_GetSendingMaxBodySize(
                            scConnection, scConfig, nonEncryptedBuffer->max_size,
                            // sequenceNumber position for symmetric case:
                            (SOPC_UA_HEADER_LENGTH_POSITION + SOPC_UA_SYMMETRIC_SECURITY_HEADER_LENGTH), true);
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
                            if (false == scConnection->isServerConnection)
                            {
                                // Client side
                                *errorStatus = OpcUa_BadRequestTooLarge;
                            }
                            else
                            {
                                // Server side
                                *errorStatus = OpcUa_BadResponseTooLarge;
                            }
                        }
                    }
                }

                // Add necessary bytes for encoding sequence header later
                if (result != false)
                {
                    assert(nonEncryptedBuffer->length <
                           nonEncryptedBuffer->position + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
                    status = SOPC_Buffer_SetDataLength(
                        nonEncryptedBuffer, nonEncryptedBuffer->position + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
                    if (SOPC_STATUS_OK == status)
                    {
                        status = SOPC_Buffer_SetPosition(
                            nonEncryptedBuffer, nonEncryptedBuffer->position + SOPC_UA_SECURE_MESSAGE_SEQUENCE_LENGTH);
                    }
                    if (SOPC_STATUS_OK != status)
                    {
                        result = false;
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }

                // Copy body bytes from input buffer
                status = SOPC_Buffer_Write(nonEncryptedBuffer, inputBuffer->data, inputBuffer->length);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            } // End of Symmetric/Asymmetric security header encoding (+body content bytes)

            if (result != false)
            {
                /* ENCODE PADDING */
                if (toEncrypt != false)
                {
                    hasPadding = true;
                    result = SOPC_Chunks_EncodePadding(scConnection, scConfig, nonEncryptedBuffer,
                                                       false == isOPN, // isSymmetricAlgo
                                                       sequenceNumberPosition, &signatureSize, &realPaddingLength,
                                                       &hasExtraPadding);
                    if (false == result)
                    {
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }
                else if (toSign != false)
                {
                    /* SIGN ONLY: ONLY DEFINE SIGNATURE SIZE */
                    bool tmpBool;
                    uint32_t tmpInt;
                    // TODO: avoid to compute non necessary crypto values
                    result = SC_Chunks_GetSendingCryptoSizes(scConnection, scConfig, false == isOPN, &tmpBool, &tmpBool,
                                                             &signatureSize, &tmpInt, &tmpInt);
                }
                else
                {
                    signatureSize = 0;
                    hasPadding = false;
                    realPaddingLength = 0;
                    hasExtraPadding = false;
                }
            }

            if (result != false && isOPN != false)
            {
                // ASYMMETRIC SECURITY SPECIFIC CASE: check MaxSenderCertificate side (padding necessary)
                // TODO: since we already encoded everything except signature, is it really necessary ?
                result = SC_Chunks_CheckMaxSenderCertificateSize(senderCertificateSize, nonEncryptedBuffer->max_size,
                                                                 securityPolicyLength, hasPadding, realPaddingLength,
                                                                 hasExtraPadding, signatureSize);
                if (false == result)
                {
                    if (false == scConnection->isServerConnection)
                    {
                        // Client side
                        *errorStatus = OpcUa_BadRequestTooLarge;
                    }
                    else
                    {
                        // Server side
                        *errorStatus = OpcUa_BadResponseTooLarge;
                    }
                }
            }

            if (result != false)
            {
                /* ENCODE (ENCRYPTED) MESSAGE SIZE */

                // Set position to message size field
                status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, SOPC_UA_HEADER_LENGTH_POSITION);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
                else
                {
                    uint32_t messageSize = 0;
                    if (false == toEncrypt)
                    {
                        // Size = current buffer length + signature if signed
                        messageSize = nonEncryptedBuffer->length + signatureSize;
                        status = SOPC_UInt32_Write(&messageSize, nonEncryptedBuffer);
                        if (SOPC_STATUS_OK != status)
                        {
                            result = false;
                            *errorStatus = OpcUa_BadTcpInternalError;
                        }
                    }
                    else
                    {
                        // Compute final encrypted message length:
                        // Data to encrypt = already encoded message from encryption start + signature size
                        const uint32_t plainDataToEncryptLength =
                            nonEncryptedBuffer->length - sequenceNumberPosition + signatureSize;

                        result = SC_Chunks_GetEncryptedDataLength(scConnection, scConfig, plainDataToEncryptLength,
                                                                  false == isOPN, // isSymmetricAlgo
                                                                  &encryptedDataLength);

                        if (result != false)
                        {
                            messageSize =
                                sequenceNumberPosition + encryptedDataLength; // non encrypted length + encrypted length
                            status = SOPC_UInt32_Write(&messageSize, nonEncryptedBuffer);
                            if (SOPC_STATUS_OK != status)
                            {
                                result = false;
                                *errorStatus = OpcUa_BadTcpInternalError;
                            }
                        }
                    }
                }
            }

            if (result != false)
            {
                /* ENCODE SEQUENCE NUMBER */

                // Set position to sequence number
                status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, sequenceNumberPosition);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
                else
                {
                    if (scConnection->tcpSeqProperties.lastSNsent > UINT32_MAX - 1024)
                    { // Part 6 §6.7.2 v1.03
                        scConnection->tcpSeqProperties.lastSNsent = 1;
                    }
                    else
                    {
                        scConnection->tcpSeqProperties.lastSNsent = scConnection->tcpSeqProperties.lastSNsent + 1;
                    }
                    status = SOPC_UInt32_Write(&scConnection->tcpSeqProperties.lastSNsent, nonEncryptedBuffer);
                    if (SOPC_STATUS_OK != status)
                    {
                        result = false;
                        *errorStatus = OpcUa_BadTcpInternalError;
                    }
                }
            }

            if (result != false)
            {
                /* ENCODE REQUEST ID */

                if (false == scConnection->isServerConnection)
                {
                    requestId = (scConnection->clientLastReqId + 1) % UINT32_MAX;
                    if (requestId == 0)
                    {
                        requestId = 1;
                    }
                    scConnection->clientLastReqId = requestId;
                }
                else
                {
                    requestId = optRequestId;
                }

                status = SOPC_UInt32_Write(&requestId, nonEncryptedBuffer);
                if (SOPC_STATUS_OK != status)
                {
                    result = false;
                    *errorStatus = OpcUa_BadTcpInternalError;
                }
            }

            if (result != false)
            {
                // Set the buffer at the end for next write
                status = SOPC_Buffer_SetPosition(nonEncryptedBuffer, nonEncryptedBuffer->length);
                assert(SOPC_STATUS_OK == status);
            }

            if (result != false && toSign != false)
            {
                /* SIGN MESSAGE */
                result = SC_Chunks_EncodeSignature(scConnection, nonEncryptedBuffer,
                                                   false == isOPN, // = isSymmetric
                                                   signatureSize);
            }

            if (result != false)
            {
                /* ENCRYPT MESSAGE */
                if (toEncrypt != false)
                {
                    SOPC_Buffer* encryptedBuffer = NULL;

                    if (scConnection->tcpMsgProperties.sendBufferSize >= sequenceNumberPosition + encryptedDataLength)
                    {
                        encryptedBuffer = SOPC_Buffer_Create(sequenceNumberPosition + encryptedDataLength);
                    }
                    else
                    {
                        // TODO: return status message too large ? => is there any guarantee due to plain buffer
                        // size ?
                    }

                    if (encryptedBuffer != NULL)
                    {
                        result = SC_Chunks_EncryptMsg(scConnection, nonEncryptedBuffer,
                                                      false == isOPN, // = isSymmetric
                                                      sequenceNumberPosition, encryptedDataLength, encryptedBuffer);
                    }
                    else
                    {
                        result = false;
                    }

                    if (result != false)
                    {
                        *outputBuffer = encryptedBuffer;
                    }

                    if (inputBuffer != nonEncryptedBuffer)
                    {
                        // If it is only an internal buffer, it shall be freed here
                        // otherwise it is the input buffer freed by caller
                        SOPC_Buffer_Delete(nonEncryptedBuffer);
                        nonEncryptedBuffer = NULL;
                    }
                }
                else
                {
                    // No encryption output buffer is non encrypted buffer
                    *outputBuffer = nonEncryptedBuffer;
                }
            }

            if (result != false && false == scConnection->isServerConnection)
            {
                SOPC_Msg_Type* msgType = NULL;
                switch (sendMsgType)
                {
                case SOPC_MSG_TYPE_SC_OPN:
                case SOPC_MSG_TYPE_SC_MSG:
                    /* CLIENT SIDE: RECORD REQUEST SENT (response expected)*/
                    msgType = calloc(1, sizeof(SOPC_Msg_Type));
                    if (msgType != NULL)
                    {
                        *msgType = sendMsgType;
                        if (msgType != SOPC_SLinkedList_Append(scConnection->tcpSeqProperties.sentRequestIds, requestId,
                                                               (void*) msgType))
                        {
                            result = false;
                        }
                    }
                    else
                    {
                        result = false;
                    }
                    break;
                case SOPC_MSG_TYPE_SC_CLO:
                    // No response expected
                    break;
                case SOPC_MSG_TYPE_HEL:
                case SOPC_MSG_TYPE_ACK:
                case SOPC_MSG_TYPE_ERR:
                case SOPC_MSG_TYPE_INVALID:
                default:
                    assert(false);
                }
            }
        }
        else
        {
            // simple OPC UA TCP message: just set the output buffer
            *outputBuffer = nonEncryptedBuffer;
        }
    }
    else
    {
        *errorStatus = OpcUa_BadEncodingError;
    }

    return result;
}

void SOPC_ChunksMgr_Dispatcher(SOPC_SecureChannels_InputEvent event, uint32_t eltId, void* params, uintptr_t auxParam)
{
    SOPC_Msg_Type sendMsgType = SOPC_MSG_TYPE_INVALID;
    SOPC_Buffer* buffer = (SOPC_Buffer*) params;
    SOPC_Buffer* outputBuffer = NULL;
    SOPC_StatusCode errorStatus = SOPC_GoodGenericStatus; // Good
    bool isSendCase = false;
    bool isSendTcpOnly = false;
    bool isOPN = false;
    bool result = false;
    // True if socket will be closed after sending this message (ERR, CLO)
    bool socketWillClose = false;
    SOPC_SecureConnection* scConnection = SC_GetConnection(eltId);
    uint32_t* requestIdForSendFailure = NULL;

    assert(buffer != NULL);

    if (scConnection != NULL && scConnection->state != SECURE_CONNECTION_STATE_SC_CLOSED)
    {
        switch (event)
        {
        /* Sockets events: */
        case SOCKET_RCV_BYTES:
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ScChunksMgr: SOCKET_RCV_BYTES\n");
            }
            /* id = secure channel connection index,
           params = (SOPC_Buffer*) received buffer */
            if (scConnection != NULL)
            {
                SC_Chunks_TreatReceivedBuffer(scConnection, eltId, buffer);
            } // else: socket should already receive close request
            break;
            /* SC connection manager -> OPC UA chunks message manager */
            // id = secure channel connection index,
            // params = (SOPC_Buffer*) buffer positioned to message payload
            // auxParam = request Id context if response
        case INT_SC_SND_HEL:
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ScChunksMgr: INT_SC_SND_HEL\n");
            }
            isSendCase = true;
            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_HEL;
            break;
        case INT_SC_SND_ACK:
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ScChunksMgr: INT_SC_SND_ACK\n");
            }
            isSendCase = true;
            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_ACK;
            break;
        case INT_SC_SND_ERR:
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ScChunksMgr: INT_SC_SND_ERR\n");
            }
            socketWillClose = true;
            isSendCase = true;
            isSendTcpOnly = true;
            sendMsgType = SOPC_MSG_TYPE_ERR;
            break;
        case INT_SC_SND_OPN:
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ScChunksMgr: INT_SC_SND_OPN\n");
            }
            isSendCase = true;
            isOPN = true;
            sendMsgType = SOPC_MSG_TYPE_SC_OPN;
            // Note: only message to be provided without size of header reserved (variable size for asymmetric secu
            // header)
            break;
        case INT_SC_SND_CLO:
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ScChunksMgr: INT_SC_SND_CLO\n");
            }
            socketWillClose = true;
            isSendCase = true;
            sendMsgType = SOPC_MSG_TYPE_SC_CLO;
            break;
        case INT_SC_SND_MSG_CHUNKS:
            if (SOPC_DEBUG_PRINTING != false)
            {
                printf("ScChunksMgr: INT_SC_SND_MSG_CHUNKS\n");
            }
            isSendCase = true;
            sendMsgType = SOPC_MSG_TYPE_SC_MSG;
            break;
        default:
            // Already filtered by secure channels API module
            assert(false);
        }

        if (isSendCase != false)
        {
            assert(auxParam <= UINT32_MAX);
            result = SC_Chunks_TreatSendBuffer(scConnection, auxParam, sendMsgType, isSendTcpOnly, isOPN, buffer,
                                               &outputBuffer, &errorStatus);
            if (false == result)
            {
                if (false == socketWillClose)
                {
                    // Treat as prio events
                    requestIdForSendFailure = malloc(sizeof(uint32_t));
                    if (requestIdForSendFailure != NULL)
                    {
                        *requestIdForSendFailure = (uint32_t) auxParam;
                    }
                    SOPC_SecureChannels_EnqueueInternalEventAsNext(INT_SC_SND_FAILURE, eltId, requestIdForSendFailure,
                                                                   errorStatus);
                }
                else
                {
                    if (SOPC_DEBUG_PRINTING)
                    {
                        printf("Failed sending message type '%d' before socket closed\n", sendMsgType);
                    }
                }
                // If buffer not reused for sending on socket: delete it
                SOPC_Buffer_Delete(buffer);
            }
            else
            {
                // Require write of output buffer on socket
                SOPC_Sockets_EnqueueEvent(SOCKET_WRITE, scConnection->socketIndex, (void*) outputBuffer, 0);

                if (buffer != outputBuffer)
                {
                    // If buffer not reused for sending on socket: delete it
                    SOPC_Buffer_Delete(buffer);
                }
            }
        }
    }
    else
    { // SC not connected: ignore event and delete buffer data
        SOPC_Buffer_Delete(buffer);
    }
}
