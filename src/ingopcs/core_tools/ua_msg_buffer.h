/*
 *  \file msg_buffer.h
 *
 *  \brief Binary UA (and TCP UA) message representation with one or several chunks.
 *  It is used for TCP UA messages layer and UA secure messages layer (write/read operations are layer dependent).
 *
 *  Created on: Jul 22, 2016
 *      Author: VMO (Systerel)
 */

#ifndef INGOPCS_MSG_BUFFER_H_
#define INGOPCS_MSG_BUFFER_H_

#include <buffer.h>
#include <ua_encodeable.h>
#include <ua_namespace_table.h>
#include "../core_types/sopc_base_types.h"

/** Length of a TCP UA message Header */
#define TCP_UA_HEADER_LENGTH 8
/** Length of a TCP UA ACK message */
#define TCP_UA_ACK_MSG_LENGTH 28
/** Minimum length of a TCP UA ERROR message */
#define TCP_UA_ERR_MIN_MSG_LENGTH 16

/** Position of MessageSize header field in a UA message chunk*/
#define UA_HEADER_LENGTH_POSITION 4
/** Position of IsFinal header field in a UA message chunk*/
#define UA_HEADER_ISFINAL_POSITION 3

/** Length of an UA secure message chunk header */
#define UA_SECURE_MESSAGE_HEADER_LENGTH 12
/** Length of an UA secure message chunk sequence header */
#define UA_SECURE_MESSAGE_SEQUENCE_LENGTH 8

extern const UA_Byte HEL[3]; /**< TCP UA Hello Message type constant */
extern const UA_Byte ACK[3]; /**< TCP UA Ack Message type constant */
extern const UA_Byte ERR[3]; /**< TCP UA Error Message type constant */
extern const UA_Byte MSG[3]; /**< UA Secure Message type constant */
extern const UA_Byte OPN[3]; /**< UA OpenSecureChannel Message type constant */
extern const UA_Byte CLO[3]; /**< UA CloseSecureChannel Message type constant */

/**
 *  \brief UA Secure Message types
 */
typedef enum {
    UA_SecureMessage,     /**< MSG type */
    UA_OpenSecureChannel, /**< OPN type */
    UA_CloseSecureChannel /**< CLO type */
} UA_SecureMessageType;

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
    UA_Msg_Chunk_Unknown,
    UA_Msg_Chunk_Invalid,
    UA_Msg_Chunk_Intermediate, /**< C type */
    UA_Msg_Chunk_Final,        /**< F type */
    UA_Msg_Chunk_Abort         /**< A type */
} UA_MsgFinalChunk;

/**
 *  \brief UA Message buffer (with one or several chunks).
 *  Note: UA_MsgBuffer type must be used to store only one chunk at same time
 *  and UA_MsgBuffers type to store several chunks at same time.
 */
typedef struct UA_MsgBuffer {
    uint32_t             nbBuffers;              /**< Number of buffers allocated (one per chunk) */
    Buffer*              buffers;                /**< Pointers on buffers for the UA Message (nbBuffers buffers) */
    TCP_UA_MsgType       type;                   /**< Type of the TCP UA Message stored */
    UA_SecureMessageType secureType;             /**< Type of the UA Secure Message stored (only valid if type = SecureMessage) */
    uint32_t             currentChunkSize;       /**< MessageSize of the current message chunk (current is chunk corresponding to nbChunks) */
    uint32_t             nbChunks;               /**< Current number of message chunks received or sent for the current message */
    uint32_t             maxChunks;              /**< Maximum number of message chunks allowed (by UA connection configuration) */
    uint32_t             sequenceNumberPosition; /**< Position of sequence number (data to encrypt start point) */
    UA_MsgFinalChunk     isFinal;                /**< IsFinal value of the current message chunk */
    uint32_t             receivedReqId;          /**< Request Id of the received message chunks (ensure all chunks have same id). Valid when nbChunks > 1 */
    void*                flushData;              /**< Data stored by structure user and that could be used to flush a message chunk */
    UA_NamespaceTable    nsTable;                /**< Namespace table to be used for encoding / decoding UA messages */
    UA_EncodeableType**  encTypesTable;          /**< EncodeableType table to be used for encoding / decoding UA messages */
} UA_MsgBuffer;

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
UA_MsgBuffer* MsgBuffer_Create(Buffer*             buffer,
                               uint32_t            maxChunks,
                               void*               flushData,
                               UA_NamespaceTable*  nsTable,
                               UA_EncodeableType** encTypesTable);


/**
 *  \brief Deallocation of an UA Message buffer and its contained buffer.
 *
 *  \param mBuffer    Address of the UA Message buffer pointer to deallocate. Set to NULL after deallocation.
 */
void MsgBuffer_Delete(UA_MsgBuffer** mBuffer);

/**
 *  \brief Reset the UA Message buffer state (buffer content, type, number of chunks, etc.)
 *   in order it could be use to receive / send a new UA message.
 *   Note: properties maxChunks, flushData, namespaces and encodeable types are not modified.
 *
 *  \param mBuffer    Pointer to UA Message buffer to reset
 */
void MsgBuffer_Reset(UA_MsgBuffer* mBuffer);

/**
 *  \brief Reset the UA Message buffer state for next chunk reception/sending at the given position
 *   (precedent not kept since only 1 buffer available)
 *
 *  \param mBuffer         Pointer to UA Message buffer to reset for next chunk
 *  \param bodyPosition    Position to which the buffer must be reset. Data before position is kept, data after is erased
 *  (0 or SN position to keep the same UA Secure Message header values for sending next chunk)
 *  \return                GOOD if operation succeeded, BAD otherwise (NULL pointer)
 */
SOPC_StatusCode MsgBuffer_ResetNextChunk(UA_MsgBuffer* mBuffer,
                                    uint32_t      bodyPosition);

/**
 *  \brief Set the Secure Message Type of the UA Message (and coherent TCP UA type)
 *
 *  \param mBuffer    Pointer to UA Message buffer
 *  \param sType      Secure message type value to set
 *  \return           GOOD if operation succeeded, BAD otherwise (NULL pointer, TCP UA type not compatible)
 */
SOPC_StatusCode MsgBuffer_SetSecureMsgType(UA_MsgBuffer* mBuffer,
                                      UA_SecureMessageType sType);

/**
 *  \brief Copy source UA Message buffer content into destination one.
 *  Note: properties maxChunks, flushData, namespaces and encodeable types are not concerned by the copy.
 *
 *  \param destMsgBuffer    Pointer to destination UA Message buffer
 *  \param srcMsgBuffer     Pointer to source UA Message buffer
 *  \return                 GOOD if operation succeeded, BAD otherwise (NULL pointers, incompatible types)
 */
SOPC_StatusCode MsgBuffer_CopyBuffer(UA_MsgBuffer* destMsgBuffer,
                                UA_MsgBuffer* srcMsgBuffer);

/**
 *  \brief UA Message buffer with several buffers (to store several chunks)
 */
typedef struct UA_MsgBuffer UA_MsgBuffers;

/**
 *  \brief Creation of an UA Message buffer with several buffers (to store several chunks) and allocate the buffers
 *  Note: nbChunks set to 0 after creation, MsgBuffers_NextChunk must be called before storing data for first chunk.
 *
 *  \param maxChunks         Maximum number of chunks for an UA Message (determined by connection configuration).
 *  \param bufferSize        Size of the buffers to allocate
 *  \param nsTable           Namespace table to be used for encoding / decoding UA messages (optional)
 *  \param encTypesTable     EncodeableType table to be used for encoding / decoding UA messages (optional)
 *
 *  \return                  NULL if buffer creation failed (maxChunks == 0, bufferSize == 0, NULL namespaces, NULL encodeable types), allocated UA Message Buffer otherwise.
 */
UA_MsgBuffers* MsgBuffers_Create(uint32_t            maxChunks,
                                 uint32_t            bufferSize,
                                 UA_NamespaceTable*  nsTable,
                                 UA_EncodeableType** encTypesTable);

/**
 *  \brief Reset the UA Message buffers state (buffers content, type, number of chunks, etc.)
 *   in order it could be use to receive new UA message.
 *
 *  \param mBuffer    Pointer to UA Message buffers to reset
 */
void MsgBuffers_Reset(UA_MsgBuffers* mBuffer);

/**
 *  \brief Deallocation of an UA Message buffers and its contained buffers.
 *
 *  \param mBuffer    Address of the UA Message buffers pointer to deallocate. Set to NULL after deallocation.
 */
void MsgBuffers_Delete(UA_MsgBuffers** mBuffer);

/**
 *  \brief Returns the current chunk (last received chunk) buffer of UA Message buffers
 *
 *  \param mBuffer    Pointer to the UA Message buffers
 *  \return           Pointer to the current chunk buffer, NULL if argument was NULL or incoherent
 */
Buffer* MsgBuffers_GetCurrentChunk(UA_MsgBuffers* mBuffer);

/**
 *  \brief Set the next (empty) chunk buffer of UA Message buffers as current one and returns it
 *
 *  \param mBuffer    Pointer to the UA Message buffers
 *  \param bufferIdx  Index of the new current chunk buffer
 *  \return           Pointer to the next chunk buffer which became current, NULL if argument was NULL or incoherent
 */
Buffer* MsgBuffers_NextChunk(UA_MsgBuffers* mBuffer,
                             uint32_t*      bufferIdx);


/**
 *  \brief Set the current chunk buffer as first chunk and reset the next buffers
 *
 *  \param mBuffer    Pointer to the UA Message buffers
 *  \return           GOOD if operation succeeded, BAD otherwise (NULL pointers, nb chunks < 2)
 */
SOPC_StatusCode MsgBuffers_SetCurrentChunkFirst(UA_MsgBuffers* mBuffer);

/**
 *  \brief Copy source UA Message buffer content into destination UA Message buffers in buffer corresponding to index
 *
 *  \param destMsgBuffer    Pointer to destination UA Message buffers
 *  \param bufferIdx        Index of the buffer to be copied into (< srcMsgBuffer->nbChunks)
 *  \param srcMsgBuffer     Pointer to source UA Message buffer
 *  \param limitedLength    Length to be copied from the source buffer (<= srcMsgBuffer->buffers->length && <= destMsgBuffer->buffers[bufferIdx].max_size)
 *  \return                 GOOD if operation succeeded, BAD otherwise
 */
SOPC_StatusCode MsgBuffers_CopyBuffer(UA_MsgBuffers* destMsgBuffer,
                                 uint32_t       bufferIdx,
                                 UA_MsgBuffer*  srcMsgBuffer,
                                 uint32_t       limitedLength);


#endif /* INGOPCS_MSG_BUFFER_H_ */
