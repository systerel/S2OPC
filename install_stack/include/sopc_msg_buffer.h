/**
 *  \file sopc_msg_buffer.h
 *
 *  \brief Binary UA (and TCP UA) message representation with one or several chunks.
 *  It is used for TCP UA messages layer and UA secure messages layer (write/read operations are layer dependent).
 */
/*
 *  Copyright (C) 2016 Systerel and others.
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

#ifndef SOPC_MSG_BUFFER_H_
#define SOPC_MSG_BUFFER_H_

#include "sopc_buffer.h"
#include "sopc_base_types.h"
#include "sopc_namespace_table.h"

/* Length of a TCP UA message Header */
#define TCP_UA_HEADER_LENGTH 8
/* Length of a TCP UA ACK message */
#define TCP_UA_ACK_MSG_LENGTH 28
/* Minimum length of a TCP UA HELLO message (without including URL string content but only its size)*/
#define TCP_UA_HEL_MSG_LENGTH 32
/* Minimum length of a TCP UA ERROR message */
#define TCP_UA_ERR_MIN_MSG_LENGTH 16

/* Position of MessageSize header field in a UA message chunk*/
#define UA_HEADER_LENGTH_POSITION 4
/* Position of IsFinal header field in a UA message chunk*/
#define UA_HEADER_ISFINAL_POSITION 3

/* Length of an UA secure message chunk header */
#define UA_SECURE_MESSAGE_HEADER_LENGTH 12
/* Length of an UA symmetric security header chunk header */
#define UA_SYMMETRIC_SECURITY_HEADER_LENGTH 4
/* Length of an UA secure message chunk sequence header */
#define UA_SECURE_MESSAGE_SEQUENCE_LENGTH 8

extern const SOPC_Byte SOPC_HEL[3]; /**< TCP UA Hello Message type constant */
extern const SOPC_Byte SOPC_ACK[3]; /**< TCP UA Ack Message type constant */
extern const SOPC_Byte SOPC_ERR[3]; /**< TCP UA Error Message type constant */
extern const SOPC_Byte SOPC_MSG[3]; /**< UA Secure Message type constant */
extern const SOPC_Byte SOPC_OPN[3]; /**< UA OpenSecureChannel Message type constant */
extern const SOPC_Byte SOPC_CLO[3]; /**< UA CloseSecureChannel Message type constant */

/**
 *  \brief UA Secure Message types
 */
typedef enum {
    SOPC_SecureMessage,     /**< MSG type */
    SOPC_OpenSecureChannel, /**< OPN type */
    SOPC_CloseSecureChannel /**< CLO type */
} SOPC_SecureMessageType;

/**
 *  \brief TCP UA Message types
 */
typedef enum {
    TCP_UA_Message_Unknown,
    TCP_UA_Message_Invalid,
    TCP_UA_Message_Hello,        /**< HEL type */
    TCP_UA_Message_Acknowledge,  /**< ACK type */
    TCP_UA_Message_Error,        /**< ERR type */
    TCP_UA_Message_SecureMessage /**< MSG, OPN or CLO types */
} TCP_UA_MsgType;

/**
 *  \brief UA Message Chunk IsFinal type
 */
typedef enum {
    SOPC_Msg_Chunk_Unknown,
    SOPC_Msg_Chunk_Invalid,
    SOPC_Msg_Chunk_Intermediate, /**< C type */
    SOPC_Msg_Chunk_Final,        /**< F type */
    SOPC_Msg_Chunk_Abort         /**< A type */
} SOPC_MsgFinalChunk;

/**
 *  \brief UA Message buffer (with one or several chunks).
 *  Note: SOPC_MsgBuffer type must be used to store only one chunk at same time
 *  and SOPC_MsgBuffers type to store several chunks at same time.
 */
typedef struct SOPC_MsgBuffer {
    uint32_t               nbBuffers;              /**< Number of buffers allocated (one per chunk) */
    SOPC_Buffer*           buffers;                /**< Pointers on buffers for the UA Message (nbBuffers buffers) */
    TCP_UA_MsgType         type;                   /**< Type of the TCP UA Message stored */
    SOPC_SecureMessageType secureType;             /**< Type of the UA Secure Message stored (only valid if type = SecureMessage) */
    uint32_t               currentChunkSize;       /**< MessageSize of the current message chunk (current is chunk corresponding to nbChunks) */
    uint32_t               nbChunks;               /**< Current number of message chunks for the current message */
    uint32_t               maxChunks;              /**< Maximum number of message chunks allowed (by UA connection configuration) */
    uint32_t               sequenceNumberPosition; /**< Position of sequence number (data to encrypt start point) */
    SOPC_MsgFinalChunk     isFinal;                /**< IsFinal value of the current message chunk */
    uint32_t               msgRequestId;           /**< Request Id of the current message chunks (ensure all chunks have same id / used for socket transaction Id). Valid when nbChunks > 1 */
    void*                  flushData;              /**< Data stored by structure user and that could be used to flush a message chunk */
    SOPC_NamespaceTable    nsTable;                /**< Namespace table to be used for encoding / decoding UA messages */
    SOPC_EncodeableType**  encTypesTable;          /**< EncodeableType table to be used for encoding / decoding UA messages */
} SOPC_MsgBuffer;

/**
 *  \brief Creation of an UA Message buffer containing only 1 buffer
 *
 *  \param buffer            Buffer to attach in the UA Message buffer to store data
 *  \param maxChunks         Maximum number of chunks for an UA Message (determined by connection configuration)
 *  \param flushData         Data to store that could be used to flush a message chunk (optional)
 *  \param nsTable           Namespace table to be used for encoding / decoding UA messages (optional)
 *  \param encTypesTable     EncodeableType table to be used for encoding / decoding UA messages (optional)
 *
 *  \return                  NULL if buffer creation failed (NULL buffer), allocated UA Message Buffer otherwise.
 */
SOPC_MsgBuffer* MsgBuffer_Create(SOPC_Buffer*               buffer,
                                 uint32_t              maxChunks,
                                 void*                 flushData,
                                 SOPC_NamespaceTable*  nsTable,
                                 SOPC_EncodeableType** encTypesTable);


/**
 *  \brief Deallocation of an UA Message buffer and its contained buffer.
 *
 *  \param mBuffer    Address of the UA Message buffer pointer to deallocate. Set to NULL after deallocation.
 */
void MsgBuffer_Delete(SOPC_MsgBuffer** mBuffer);

/**
 *  \brief Reset the UA Message buffer state (buffer content, type, number of chunks, etc.)
 *   in order it could be use to receive / send a new UA message.
 *   Note: properties maxChunks, flushData, namespaces and encodeable types are not modified.
 *
 *  \param mBuffer    Pointer to UA Message buffer to reset
 */
void MsgBuffer_Reset(SOPC_MsgBuffer* mBuffer);

/**
 *  \brief Reset the UA Message buffer state for next chunk reception/sending at the given position
 *   (precedent not kept since only 1 buffer available)
 *
 *  \param mBuffer         Pointer to UA Message buffer to reset for next chunk
 *  \param bodyPosition    Position to which the buffer must be reset. Data before position is kept, data after is erased
 *  (0 or SN position to keep the same UA Secure Message header values for sending next chunk)
 *  \return                GOOD if operation succeeded, BAD otherwise (NULL pointer)
 */
SOPC_StatusCode MsgBuffer_ResetNextChunk(SOPC_MsgBuffer* mBuffer,
                                         uint32_t        bodyPosition);

/**
 *  \brief Set the Secure Message Type of the UA Message (and coherent TCP UA type)
 *
 *  \param mBuffer    Pointer to UA Message buffer
 *  \param sType      Secure message type value to set
 *  \return           GOOD if operation succeeded, BAD otherwise (NULL pointer, TCP UA type not compatible)
 */
SOPC_StatusCode MsgBuffer_SetSecureMsgType(SOPC_MsgBuffer*        mBuffer,
                                           SOPC_SecureMessageType sType);

/**
 *  \brief Copy source UA Message buffer content into destination one.
 *  Note: properties maxChunks, flushData, namespaces and encodeable types are not concerned by the copy.
 *
 *  \param destMsgBuffer    Pointer to destination UA Message buffer
 *  \param srcMsgBuffer     Pointer to source UA Message buffer
 *  \return                 GOOD if operation succeeded, BAD otherwise (NULL pointers, incompatible types)
 */
SOPC_StatusCode MsgBuffer_CopyBuffer(SOPC_MsgBuffer* destMsgBuffer,
                                     SOPC_MsgBuffer* srcMsgBuffer);

/**
 *  \brief UA Message buffer with several buffers (to store several chunks)
 */
typedef struct SOPC_MsgBuffer SOPC_MsgBuffers;

/**
 *  \brief Creation of an UA Message buffer with several buffers (to store several chunks) and allocate the buffers
 *  Note: nbChunks set to 0 after creation, MsgBuffers_NextChunk must be called before storing data for first chunk.
 *
 *  \param maxChunks         Maximum number of chunks for an UA Message (determined by connection configuration).
 *  \param bufferSize        Size of the buffers to allocate
 *  \param flushData         Data to store that could be used to flush a message chunk (optional)
 *  \param nsTable           Namespace table to be used for encoding / decoding UA messages (optional)
 *  \param encTypesTable     EncodeableType table to be used for encoding / decoding UA messages (optional)
 *
 *  \return                  NULL if buffer creation failed (maxChunks == 0, bufferSize == 0, NULL namespaces, NULL encodeable types), allocated UA Message Buffer otherwise.
 */
SOPC_MsgBuffers* MsgBuffers_Create(uint32_t              maxChunks,
                                   uint32_t              bufferSize,
                                   void*                 flushData,
                                   SOPC_NamespaceTable*  nsTable,
                                   SOPC_EncodeableType** encTypesTable);

/**
 *  \brief Reset the UA Message buffers state (buffers content, type, number of chunks, etc.)
 *   in order it could be use to receive new UA message.
 *
 *  \param mBuffer    Pointer to UA Message buffers to reset
 */
void MsgBuffers_Reset(SOPC_MsgBuffers* mBuffer);

/**
 *  \brief Deallocation of an UA Message buffers and its contained buffers.
 *
 *  \param mBuffer    Address of the UA Message buffers pointer to deallocate. Set to NULL after deallocation.
 */
void MsgBuffers_Delete(SOPC_MsgBuffers** mBuffer);

/**
 *  \brief Returns the current chunk (last received chunk) buffer of UA Message buffers
 *
 *  \param mBuffer    Pointer to the UA Message buffers
 *  \return           Pointer to the current chunk buffer, NULL if argument was NULL or incoherent
 */
SOPC_Buffer* MsgBuffers_GetCurrentChunk(SOPC_MsgBuffers* mBuffer);

/**
 *  \brief Set the next (empty) chunk buffer of UA Message buffers as current one and returns it
 *
 *  \param mBuffer    Pointer to the UA Message buffers
 *  \param bufferIdx  Index of the new current chunk buffer
 *  \return           Pointer to the next chunk buffer which became current, NULL if argument was NULL or incoherent
 */
SOPC_Buffer* MsgBuffers_NextChunk(SOPC_MsgBuffers* mBuffer,
                             uint32_t*        bufferIdx);

/**
 *  \brief Copy content of current chunk until message body position (headers content) into next chunk, then
 *         set the next chunk buffer of UA Message buffers as current one and returns it.
 *
 *  \param mBuffers      Pointer to the UA Message buffers
 *  \param bodyPosition  Position of the message body first byte, all headers to copy are included before this position
 *  \return              Pointer to the next chunk buffer which became current, NULL if argument was NULL or incoherent
 */
SOPC_Buffer* MsgBuffers_NextChunkWithHeadersCopy(SOPC_MsgBuffers* mBuffers,
                                            uint32_t         bodyPosition);

/**
 *  \brief Set the current chunk buffer as first chunk and reset the next buffers
 *
 *  \param mBuffer    Pointer to the UA Message buffers
 *  \return           GOOD if operation succeeded, BAD otherwise (NULL pointers, nb chunks < 2)
 */
SOPC_StatusCode MsgBuffers_SetCurrentChunkFirst(SOPC_MsgBuffers* mBuffer);

/**
 *  \brief Copy source UA Message buffers content for given indexed buffer into destination UA Message buffer
 *         Note: internal properties of the message buffers are also copied (including nbChunks)
 *
 *  \param destMsgBuffer    Pointer to destination UA Message buffer
 *  \param srcMsgBuffers    Pointer to source UA Message buffer
 *  \param bufferIdx        Index of the buffer to be copied into (< srcMsgBuffers->nbChunks)
 *  \return                 GOOD if operation succeeded, BAD otherwise
 */
SOPC_StatusCode MsgBuffers_CopyBufferIdx(SOPC_MsgBuffer*  destMsgBuffer,
                                         SOPC_MsgBuffers* srcMsgBuffers,
                                         uint32_t         bufferIdx);

/**
 *  \brief Copy source UA Message buffer content into destination UA Message buffers in buffer corresponding to index
 *
 *  \param destMsgBuffer    Pointer to destination UA Message buffers
 *  \param bufferIdx        Index of the buffer to be copied into (< srcMsgBuffer->nbChunks)
 *  \param srcMsgBuffer     Pointer to source UA Message buffer
 *  \param limitedLength    Length to be copied from the source buffer (<= srcMsgBuffer->buffers->length && <= destMsgBuffer->buffers[bufferIdx].max_size)
 *  \return                 GOOD if operation succeeded, BAD otherwise
 */
SOPC_StatusCode MsgBuffers_CopyBuffer(SOPC_MsgBuffers* destMsgBuffer,
                                      uint32_t         bufferIdx,
                                      SOPC_MsgBuffer*  srcMsgBuffer,
                                      uint32_t         limitedLength);


#endif /* SOPC_MSG_BUFFER_H_ */
