/*
 * Entry point for tools tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 *
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


#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include "sopc_encoder.h"
#include "sopc_msg_buffer.h"
#include "check_stack.h"


START_TEST(test_ua_msg_buffer_create_set_type)
{
    SOPC_StatusCode status = 0;
    int flushData = 3;
    SOPC_NamespaceTable table;
    Namespace_Initialize(&table);
    SOPC_EncodeableType* encTypes[1];


    // Test creation / set type
    //// Test nominal case
    Buffer* buf = Buffer_Create(10);
    SOPC_MsgBuffer* msgBuf = MsgBuffer_Create(buf, 1, &flushData, &table, encTypes);
    ck_assert(msgBuf != NULL);
    ck_assert(msgBuf->nbBuffers == 1);
    ck_assert(msgBuf->buffers == buf);
    ck_assert(msgBuf->type == TCP_UA_Message_Unknown);
    ck_assert(msgBuf->secureType == SOPC_SecureMessage);
    ck_assert(msgBuf->currentChunkSize == 0);
    ck_assert(msgBuf->nbChunks == 1);
    ck_assert(msgBuf->maxChunks == 1);
    ck_assert(msgBuf->sequenceNumberPosition == 0);
    ck_assert(msgBuf->isFinal == SOPC_Msg_Chunk_Unknown);
    ck_assert(msgBuf->receivedReqId == 0);
    ck_assert(msgBuf->flushData == &flushData);
    ck_assert(msgBuf->nsTable.namespaceArray == table.namespaceArray);
    ck_assert(msgBuf->encTypesTable == encTypes);

    status = MsgBuffer_SetSecureMsgType(msgBuf, SOPC_OpenSecureChannel);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuf->secureType == SOPC_OpenSecureChannel);

    msgBuf->type = TCP_UA_Message_SecureMessage;
    status = MsgBuffer_SetSecureMsgType(msgBuf, SOPC_CloseSecureChannel);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuf->secureType == SOPC_CloseSecureChannel);

    //// Test set type degraded cases
    status = MsgBuffer_SetSecureMsgType(NULL, SOPC_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    msgBuf->type = TCP_UA_Message_Hello; // Not a secure type
    status = MsgBuffer_SetSecureMsgType(msgBuf, SOPC_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    msgBuf->type = TCP_UA_Message_Acknowledge; // Not a secure type
    status = MsgBuffer_SetSecureMsgType(msgBuf, SOPC_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    msgBuf->type = TCP_UA_Message_Error; // Not a secure type
    status = MsgBuffer_SetSecureMsgType(msgBuf, SOPC_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    msgBuf->type = TCP_UA_Message_Invalid; // Not a secure type
    status = MsgBuffer_SetSecureMsgType(msgBuf, SOPC_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    MsgBuffer_Delete(&msgBuf);

    //// Test buffer creation degraded cases
    msgBuf = MsgBuffer_Create(NULL, 1, &flushData, &table, encTypes);
    ck_assert(msgBuf == NULL);


}
END_TEST

START_TEST(test_ua_msg_buffer_reset)
{
    SOPC_StatusCode status = 0;
    uint8_t flushData = 3;
    SOPC_NamespaceTable table;
    Namespace_Initialize(&table);
    SOPC_EncodeableType* encTypes[1];

    // Test reset
    //// Test nominal case
    Buffer* buf = Buffer_Create(10);
    SOPC_MsgBuffer* msgBuf = MsgBuffer_Create(buf, 1, &flushData, &table, encTypes);
    ////// Modify properties
    Buffer_Write(buf, &flushData, 1);
    msgBuf->type = TCP_UA_Message_SecureMessage;
    MsgBuffer_SetSecureMsgType(msgBuf, SOPC_CloseSecureChannel);
    msgBuf->currentChunkSize = 9;
    msgBuf->nbChunks = 2;
    msgBuf->maxChunks = 3;
    msgBuf->sequenceNumberPosition = 2;
    msgBuf->isFinal = SOPC_Msg_Chunk_Final;
    msgBuf->receivedReqId = 1;
    msgBuf->flushData = NULL;
    msgBuf->nsTable.namespaceArray = NULL;
    msgBuf->encTypesTable = NULL;
    ////// Check modified properties
    ck_assert(msgBuf->buffers->data[0] == flushData);
    ck_assert(msgBuf->buffers->length == 1);
    ck_assert(msgBuf->secureType == SOPC_CloseSecureChannel);

    ////// Reset msg buffer
    MsgBuffer_Reset(msgBuf);
    ////// Check reset properties
    ck_assert(msgBuf->buffers->data[0] == 0);
    ck_assert(msgBuf->buffers->length == 0);
    ck_assert(msgBuf->type == TCP_UA_Message_Unknown);
    ck_assert(msgBuf->secureType == SOPC_SecureMessage);
    ck_assert(msgBuf->currentChunkSize == 0);
    ck_assert(msgBuf->nbChunks == 1);
    ck_assert(msgBuf->sequenceNumberPosition == 0);
    ck_assert(msgBuf->isFinal == SOPC_Msg_Chunk_Unknown);
    ck_assert(msgBuf->receivedReqId == 0);

    /////// Check properties not reset
    ck_assert(msgBuf->maxChunks == 3);
    ck_assert(msgBuf->flushData == NULL);
    ck_assert(msgBuf->nsTable.namespaceArray == NULL);
    ck_assert(msgBuf->encTypesTable == NULL);
    MsgBuffer_Delete(&msgBuf);

    ////// Modify properties
    buf = Buffer_Create(10);
    msgBuf = MsgBuffer_Create(buf, 2, &flushData, &table, encTypes);
    Buffer_Write(buf, &flushData, 1);
    Buffer_Write(buf, &flushData, 1);
    msgBuf->type = TCP_UA_Message_SecureMessage;
    MsgBuffer_SetSecureMsgType(msgBuf, SOPC_CloseSecureChannel);
    msgBuf->currentChunkSize = 9;
    msgBuf->sequenceNumberPosition = 2;
    msgBuf->isFinal = SOPC_Msg_Chunk_Final;
    msgBuf->receivedReqId = 1;

    ck_assert(msgBuf->buffers->data[0] == flushData);
    ck_assert(msgBuf->buffers->data[1] == flushData);
    ck_assert(msgBuf->buffers->length == 2);

    ////// Reset for next chunk keeping data before position 1
    status = MsgBuffer_ResetNextChunk(msgBuf, 1);
    ck_assert(status == STATUS_OK);
    ////// Increased number of chunks
    ck_assert(msgBuf->nbChunks == 2);
    ////// Buffer reset using given position
    ck_assert(msgBuf->buffers->data[0] == flushData);
    ck_assert(msgBuf->buffers->data[1] == 0);
    ck_assert(msgBuf->buffers->length == 1);
    ck_assert(msgBuf->currentChunkSize == 1);
    ////// Internal properties unchanged (sample)
    ck_assert(msgBuf->secureType == SOPC_CloseSecureChannel);
    ck_assert(msgBuf->sequenceNumberPosition == 2);
    ck_assert(msgBuf->isFinal == SOPC_Msg_Chunk_Final);
    ck_assert(msgBuf->receivedReqId == 1);


    //// Test degraded case
    ////// NULL msg buffer
    MsgBuffer_Reset(NULL); // Nothing verifiable
    status = MsgBuffer_ResetNextChunk(NULL, 1);
    ck_assert(status != STATUS_OK);
    ////// Invalid position for reset
    status = MsgBuffer_ResetNextChunk(msgBuf, 2);
    ck_assert(status != STATUS_OK);

    MsgBuffer_Delete(&msgBuf);
}
END_TEST

START_TEST(test_ua_msg_buffer_copy)
{
    SOPC_StatusCode status = 0;
    uint8_t flushData = 3;
    SOPC_NamespaceTable table;
    Namespace_Initialize(&table);
    SOPC_EncodeableType* encTypes[1];

    // Test reset
    //// Test nominal case
    Buffer* buf = Buffer_Create(10);
    SOPC_MsgBuffer* msgBuf = MsgBuffer_Create(buf, 1, &flushData, &table, encTypes);
    ////// Modify properties
    Buffer_Write(buf, &flushData, 1);
    msgBuf->type = TCP_UA_Message_SecureMessage;
    MsgBuffer_SetSecureMsgType(msgBuf, SOPC_CloseSecureChannel);
    msgBuf->currentChunkSize = 9;
    msgBuf->nbChunks = 2;
    msgBuf->maxChunks = 3;
    msgBuf->sequenceNumberPosition = 2;
    msgBuf->isFinal = SOPC_Msg_Chunk_Final;
    msgBuf->receivedReqId = 1;
    ////// Check modified properties
    ck_assert(msgBuf->buffers->data[0] == flushData);
    ck_assert(msgBuf->buffers->length == 1);
    ck_assert(msgBuf->secureType == SOPC_CloseSecureChannel);

    Buffer* bufDest = Buffer_Create(2);
    SOPC_MsgBuffer* msgBufDest = MsgBuffer_Create(bufDest, 2, NULL, NULL, NULL);
    ////// Reset msg buffer
    status = MsgBuffer_CopyBuffer(msgBufDest, msgBuf);
    ck_assert(status == STATUS_OK);
    ////// Check copied properties
    ck_assert(msgBufDest->buffers != msgBuf->buffers);
    ck_assert(msgBufDest->buffers->data[0] == flushData);
    ck_assert(msgBufDest->buffers->length == 1);
    ck_assert(msgBufDest->type == TCP_UA_Message_SecureMessage);
    ck_assert(msgBufDest->secureType == SOPC_CloseSecureChannel);
    ck_assert(msgBufDest->currentChunkSize == 9);
    ck_assert(msgBufDest->nbChunks == 2);
    ck_assert(msgBufDest->sequenceNumberPosition == 2);
    ck_assert(msgBufDest->isFinal == SOPC_Msg_Chunk_Final);
    ck_assert(msgBufDest->receivedReqId == 1);
    /////// Check not concerned fields were not copied
    ck_assert(msgBufDest->maxChunks == 2);
    ck_assert(msgBufDest->flushData == NULL);
    ck_assert(msgBufDest->nsTable.namespaceArray == NULL);
    ck_assert(msgBufDest->encTypesTable == NULL);


    //// Test degraded case
    ////// NULL msg buffer
    status = MsgBuffer_CopyBuffer(msgBufDest, NULL);
    ck_assert(status != STATUS_OK);
    status = MsgBuffer_CopyBuffer(NULL,msgBuf);

    ////// Fulfill 3 bytes in buffer data of src
    ////// and try to copy in a msg buffer with buffer data max size = 2
    Buffer_Write(buf, &flushData, 1);
    Buffer_Write(buf, &flushData, 1);
    ck_assert(msgBuf->buffers->length == 3);
    status = MsgBuffer_CopyBuffer(msgBufDest, msgBuf);
    ck_assert(status != STATUS_OK);

    MsgBuffer_Delete(&msgBuf);
    MsgBuffer_Delete(&msgBufDest);
}
END_TEST

START_TEST(test_ua_msg_buffers_create)
{
    SOPC_NamespaceTable table;
    Namespace_Initialize(&table);
    SOPC_EncodeableType* encTypes[1];

    // Test creation
    //// Test nominal case
    SOPC_MsgBuffers* msgBuf = MsgBuffers_Create(3, 10, &table, encTypes);
    ck_assert(msgBuf != NULL);
    ck_assert(msgBuf->nbBuffers == 3);
    ck_assert(msgBuf->buffers[0].data != NULL);
    ck_assert(msgBuf->buffers[1].data != NULL);
    ck_assert(msgBuf->buffers[2].data != NULL);
    ck_assert(msgBuf->type == TCP_UA_Message_Unknown);
    ck_assert(msgBuf->secureType == SOPC_SecureMessage);
    ck_assert(msgBuf->currentChunkSize == 0);
    ck_assert(msgBuf->nbChunks == 0);
    ck_assert(msgBuf->maxChunks == 3);
    ck_assert(msgBuf->sequenceNumberPosition == 0);
    ck_assert(msgBuf->isFinal == SOPC_Msg_Chunk_Unknown);
    ck_assert(msgBuf->receivedReqId == 0);
    ck_assert(msgBuf->nsTable.namespaceArray == table.namespaceArray);
    ck_assert(msgBuf->encTypesTable == encTypes);
    MsgBuffers_Delete(&msgBuf);

    //// Test buffer creation degraded cases
    msgBuf = MsgBuffers_Create(0, 10, &table, encTypes);
    ck_assert(msgBuf == NULL);
    msgBuf = MsgBuffers_Create(3, 0, &table, encTypes);
    ck_assert(msgBuf == NULL);
    msgBuf = MsgBuffers_Create(3, 10, NULL, encTypes);
    ck_assert(msgBuf == NULL);
    msgBuf = MsgBuffers_Create(3, 10, &table, NULL);
    ck_assert(msgBuf == NULL);

}
END_TEST

START_TEST(test_ua_msg_buffers_chunk_mgr)
{
    SOPC_NamespaceTable table;
    Namespace_Initialize(&table);
    SOPC_EncodeableType* encTypes[1];
    Buffer* buf = NULL;
    Buffer* buf2 = NULL;
    uint32_t bufIdx = 0;
    SOPC_StatusCode status = STATUS_NOK;

    // Test chunks management
    //// Test nominal case
    SOPC_MsgBuffers* msgBuf = MsgBuffers_Create(3, 10, &table, encTypes);

    ////// No current chunk before NextChunk called
    buf2 = MsgBuffers_GetCurrentChunk(msgBuf);
    ck_assert(buf2 == NULL);

    buf = MsgBuffers_NextChunk(msgBuf, &bufIdx);
    ck_assert(bufIdx == 0);
    ck_assert(msgBuf->nbChunks == 1);
    buf2 = MsgBuffers_GetCurrentChunk(msgBuf);
    ck_assert(buf == buf2);
    ck_assert(buf == &(msgBuf->buffers[bufIdx]));
    msgBuf->buffers[bufIdx].data[0] = 0xFF;
    msgBuf->buffers[bufIdx].length = 1;
    msgBuf->buffers[bufIdx].position = 1;

    buf = MsgBuffers_NextChunk(msgBuf, &bufIdx);
    ck_assert(bufIdx == 1);
    ck_assert(msgBuf->nbChunks == 2);
    buf2 = MsgBuffers_GetCurrentChunk(msgBuf);
    ck_assert(buf == buf2);
    ck_assert(buf == &(msgBuf->buffers[1]));
    msgBuf->buffers[bufIdx].data[0] = 0xEE;
    msgBuf->buffers[bufIdx].length = 2;
    msgBuf->buffers[bufIdx].position = 2;

    buf = MsgBuffers_NextChunk(msgBuf, &bufIdx);
    ck_assert(bufIdx == 2);
    ck_assert(msgBuf->nbChunks == 3);
    buf2 = MsgBuffers_GetCurrentChunk(msgBuf);
    ck_assert(buf == buf2);
    ck_assert(buf == &(msgBuf->buffers[2]));
    msgBuf->buffers[bufIdx].data[0] = 0xDD;
    msgBuf->buffers[bufIdx].length = 3;
    msgBuf->buffers[bufIdx].position = 3;

    status = MsgBuffers_SetCurrentChunkFirst(msgBuf);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuf->nbChunks == 1);
    buf2 = MsgBuffers_GetCurrentChunk(msgBuf);
    ck_assert(buf2 == &(msgBuf->buffers[0]));
    ck_assert(msgBuf->buffers[0].data[0] == 0xDD);
    ck_assert(msgBuf->buffers[0].length == 3);
    ck_assert(msgBuf->buffers[0].position == 3);
    ck_assert(msgBuf->buffers[1].data[0] == 0x00);
    ck_assert(msgBuf->buffers[1].length == 0);
    ck_assert(msgBuf->buffers[1].position == 0);
    ck_assert(msgBuf->buffers[2].data[0] == 0x00);
    ck_assert(msgBuf->buffers[2].length == 0);
    ck_assert(msgBuf->buffers[2].position == 0);

    //// Test degraded cases
    ////// NULL pointers
    buf = MsgBuffers_GetCurrentChunk(NULL);
    ck_assert(buf == NULL);
    status = MsgBuffers_SetCurrentChunkFirst(NULL);
    ck_assert(status != STATUS_OK);
    buf = MsgBuffers_NextChunk(NULL, &bufIdx);
    ck_assert(buf == NULL);
    buf = MsgBuffers_NextChunk(msgBuf, NULL);
    ck_assert(buf == NULL);
    ////// Set current chunk first with only 1 chunk
    status = MsgBuffers_SetCurrentChunkFirst(msgBuf);
    ck_assert(status != STATUS_OK);
    MsgBuffers_Delete(&msgBuf);
    msgBuf = MsgBuffers_Create(3, 10, &table, encTypes);
    ////// Set current chunk first with only 0 chunk
    status = MsgBuffers_SetCurrentChunkFirst(msgBuf);
    ck_assert(status != STATUS_OK);
    MsgBuffers_Delete(&msgBuf);
}
END_TEST

START_TEST(test_ua_msg_buffers_copy)
{
    SOPC_StatusCode status = 0;
    uint8_t flushData = 3;
    SOPC_NamespaceTable table;
    Namespace_Initialize(&table);
    SOPC_EncodeableType* encTypes[1];

    // Test reset
    //// Test nominal case
    Buffer* buf = Buffer_Create(10);
    SOPC_MsgBuffer* msgBuf = MsgBuffer_Create(buf, 1, &flushData, &table, encTypes);
    ////// Modify properties
    Buffer_Write(buf, &flushData, 1);
    msgBuf->type = TCP_UA_Message_SecureMessage;
    MsgBuffer_SetSecureMsgType(msgBuf, SOPC_CloseSecureChannel);
    msgBuf->currentChunkSize = 9;
    msgBuf->nbChunks = 2;
    msgBuf->maxChunks = 3;
    msgBuf->sequenceNumberPosition = 2;
    msgBuf->isFinal = SOPC_Msg_Chunk_Final;
    msgBuf->receivedReqId = 1;
    ////// Check modified properties
    ck_assert(msgBuf->buffers->data[0] == flushData);
    ck_assert(msgBuf->buffers->length == 1);
    ck_assert(msgBuf->secureType == SOPC_CloseSecureChannel);

    SOPC_MsgBuffer* msgBufsDest = MsgBuffers_Create(3, 5, &table, encTypes);
    ////// Copy msg buffer
    status = MsgBuffers_CopyBuffer(msgBufsDest, 0,
                                   msgBuf, 1);
    ck_assert(status == STATUS_OK);
    ////// Check copied properties
    ck_assert(msgBufsDest->buffers[0].data != msgBuf->buffers->data);
    ck_assert(msgBufsDest->buffers[0].data[0] == flushData);
    ck_assert(msgBufsDest->buffers[0].length == 1);
    ck_assert(msgBufsDest->type == TCP_UA_Message_SecureMessage);
    ck_assert(msgBufsDest->secureType == SOPC_CloseSecureChannel);
    ck_assert(msgBufsDest->currentChunkSize == 9);
    ck_assert(msgBufsDest->nbChunks == 2);
    ck_assert(msgBufsDest->sequenceNumberPosition == 2);
    ck_assert(msgBufsDest->isFinal == SOPC_Msg_Chunk_Final);
    ck_assert(msgBufsDest->receivedReqId == 1);
    /////// Check not concerned fields were not copied
    ck_assert(msgBufsDest->maxChunks == 3);
    ck_assert(msgBufsDest->flushData == NULL);
    ck_assert(msgBufsDest->nsTable.namespaceArray == table.namespaceArray);
    ck_assert(msgBufsDest->encTypesTable == encTypes);

    status = MsgBuffers_CopyBuffer(msgBufsDest, 1,
                                   msgBuf, 1);
    ck_assert(status == STATUS_OK);
    ////// Check copied properties
    ck_assert(msgBufsDest->buffers[1].data != msgBuf->buffers->data);
    ck_assert(msgBufsDest->buffers[1].data[0] == flushData);
    ck_assert(msgBufsDest->buffers[1].length == 1);
    ck_assert(msgBufsDest->type == TCP_UA_Message_SecureMessage);
    ck_assert(msgBufsDest->secureType == SOPC_CloseSecureChannel);
    ck_assert(msgBufsDest->currentChunkSize == 9);
    ck_assert(msgBufsDest->nbChunks == 2);
    ck_assert(msgBufsDest->sequenceNumberPosition == 2);
    ck_assert(msgBufsDest->isFinal == SOPC_Msg_Chunk_Final);
    ck_assert(msgBufsDest->receivedReqId == 1);
    /////// Check not concerned fields were not copied
    ck_assert(msgBufsDest->maxChunks == 3);
    ck_assert(msgBufsDest->flushData == NULL);
    ck_assert(msgBufsDest->nsTable.namespaceArray == table.namespaceArray);
    ck_assert(msgBufsDest->encTypesTable == encTypes);


    //// Test degraded case
    ////// NULL msg buffer
    status = MsgBuffers_CopyBuffer(msgBufsDest, 0, NULL, 1);
    ck_assert(status != STATUS_OK);
    status = MsgBuffers_CopyBuffer(NULL, 0, msgBuf, 1);
    ck_assert(status != STATUS_OK);
    ////// size of 0
    status = MsgBuffers_CopyBuffer(msgBufsDest, 0, msgBuf, 0);
    ck_assert(status != STATUS_OK);
    ////// invalid buffer idx
    status = MsgBuffers_CopyBuffer(msgBufsDest, 3, msgBuf, 1);
    ck_assert(status != STATUS_OK);
    ////// invalid size
    status = MsgBuffers_CopyBuffer(msgBufsDest, 0, msgBuf, 3);
    ck_assert(status != STATUS_OK);

    MsgBuffer_Delete(&msgBuf);
    MsgBuffers_Delete(&msgBufsDest);
}
END_TEST


START_TEST(test_ua_encoder_endianess_mgt)
{
    int16_t v16 = 0;
    uint16_t vu16 = 0;
    int32_t v32 = 0;
    uint32_t vu32 = 0;
    int64_t v64 = 0;
    uint64_t vu64 = 0;
    float vfloat = 0.0;
    double vdouble = 0.0;

    uint8_t* bytes = NULL;

    // Test encoding with same endianess in machine and UA binary
    endianess = P_Endianess_LittleEndian;
    bytes = (uint8_t*) &v16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    SOPC_EncodeDecode_Int16(&v16);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC);

    bytes = (uint8_t*) &vu16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    SOPC_EncodeDecode_UInt16(&vu16);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC);

    bytes = (uint8_t*) &v32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    SOPC_EncodeDecode_Int32(&v32);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC &&
              bytes[2] == 0xCD && bytes[3] == 0xDE);

    bytes = (uint8_t*) &vu32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    SOPC_EncodeDecode_UInt32(&vu32);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC &&
              bytes[2] == 0xCD && bytes[3] == 0xDE);

    bytes = (uint8_t*) &v64;
    bytes[0] = 0x00;
    bytes[1] = 0x11;
    bytes[2] = 0x22;
    bytes[3] = 0x33;
    bytes[4] = 0xAB;
    bytes[5] = 0xBC;
    bytes[6] = 0xCD;
    bytes[7] = 0xDE;
    SOPC_EncodeDecode_Int64(&v64);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x11 &&
              bytes[2] == 0x22 && bytes[3] == 0x33 &&
              bytes[4] == 0xAB && bytes[5] == 0xBC &&
              bytes[6] == 0xCD && bytes[7] == 0xDE);

    bytes = (uint8_t*) &vu64;
    bytes[0] = 0x00;
    bytes[1] = 0x11;
    bytes[2] = 0x22;
    bytes[3] = 0x33;
    bytes[4] = 0xAB;
    bytes[5] = 0xBC;
    bytes[6] = 0xCD;
    bytes[7] = 0xDE;
    SOPC_EncodeDecode_UInt64(&vu64);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x11 &&
              bytes[2] == 0x22 && bytes[3] == 0x33 &&
              bytes[4] == 0xAB && bytes[5] == 0xBC &&
              bytes[6] == 0xCD && bytes[7] == 0xDE);

    floatEndianess = P_Endianess_LittleEndian;
    bytes = (uint8_t*) &vfloat;
    bytes[0] = 0x00;
    bytes[1] = 0x00;
    bytes[2] = 0xD0;
    bytes[3] = 0xC0;
    SOPC_EncodeDecode_Float(&vfloat);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 &&
              bytes[2] == 0xD0 && bytes[3] == 0xC0);

    bytes = (uint8_t*) &vdouble;
    bytes[0] = 0x00;
    bytes[1] = 0x00;
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    bytes[4] = 0x00;
    bytes[5] = 0x00;
    bytes[6] = 0x1A;
    bytes[7] = 0xC0;
    SOPC_EncodeDecode_Double(&vdouble);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 &&
              bytes[2] == 0x00 && bytes[3] == 0x00 &&
              bytes[4] == 0x00 && bytes[5] == 0x00 &&
              bytes[6] == 0x1A && bytes[7] == 0xC0);


    // Test encoding with different endianess in machine and UA binary
    endianess = P_Endianess_BigEndian;
    bytes = (uint8_t*) &v16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    SOPC_EncodeDecode_Int16(&v16);
    ck_assert(bytes[1] == 0xAB && bytes[0] == 0xBC);

    bytes = (uint8_t*) &vu16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    SOPC_EncodeDecode_UInt16(&vu16);
    ck_assert(bytes[1] == 0xAB && bytes[0] == 0xBC);

    bytes = (uint8_t*) &v32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    SOPC_EncodeDecode_Int32(&v32);
    ck_assert(bytes[3] == 0xAB && bytes[2] == 0xBC &&
              bytes[1] == 0xCD && bytes[0] == 0xDE);

    bytes = (uint8_t*) &vu32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    SOPC_EncodeDecode_UInt32(&vu32);
    ck_assert(bytes[3] == 0xAB && bytes[2] == 0xBC &&
              bytes[1] == 0xCD && bytes[0] == 0xDE);

    bytes = (uint8_t*) &v64;
    bytes[0] = 0x00;
    bytes[1] = 0x11;
    bytes[2] = 0x22;
    bytes[3] = 0x33;
    bytes[4] = 0xAB;
    bytes[5] = 0xBC;
    bytes[6] = 0xCD;
    bytes[7] = 0xDE;
    SOPC_EncodeDecode_Int64(&v64);
    ck_assert(bytes[7] == 0x00 && bytes[6] == 0x11 &&
              bytes[5] == 0x22 && bytes[4] == 0x33 &&
              bytes[3] == 0xAB && bytes[2] == 0xBC &&
              bytes[1] == 0xCD && bytes[0] == 0xDE);

    bytes = (uint8_t*) &vu64;
    bytes[0] = 0x00;
    bytes[1] = 0x11;
    bytes[2] = 0x22;
    bytes[3] = 0x33;
    bytes[4] = 0xAB;
    bytes[5] = 0xBC;
    bytes[6] = 0xCD;
    bytes[7] = 0xDE;
    SOPC_EncodeDecode_UInt64(&vu64);
    ck_assert(bytes[7] == 0x00 && bytes[6] == 0x11 &&
              bytes[5] == 0x22 && bytes[4] == 0x33 &&
              bytes[3] == 0xAB && bytes[2] == 0xBC &&
              bytes[1] == 0xCD && bytes[0] == 0xDE);

    floatEndianess = P_Endianess_BigEndian;
    bytes = (uint8_t*) &vfloat;
    bytes[0] = 0xC0;
    bytes[1] = 0xD0;
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    SOPC_EncodeDecode_Float(&vfloat);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 &&
              bytes[2] == 0xD0 && bytes[3] == 0xC0);

    bytes = (uint8_t*) &vdouble;
    bytes[0] = 0xC0;
    bytes[1] = 0x1A;
    bytes[2] = 0x00;
    bytes[3] = 0x00;
    bytes[4] = 0x00;
    bytes[5] = 0x00;
    bytes[6] = 0x00;
    bytes[7] = 0x00;
    SOPC_EncodeDecode_Double(&vdouble);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 &&
              bytes[2] == 0x00 && bytes[3] == 0x00 &&
              bytes[4] == 0x00 && bytes[5] == 0x00 &&
              bytes[6] == 0x1A && bytes[7] == 0xC0);

}
END_TEST

START_TEST(test_ua_encoder_basic_types)
{
    InitPlatformDependencies(); // Necessary to initialize endianess configuration
    SOPC_StatusCode status = STATUS_OK;
    Buffer* buffer = Buffer_Create(100);
    SOPC_MsgBuffer* msgBuffer = MsgBuffer_Create(buffer, 1, NULL, NULL, NULL);

    Buffer* buffer2 = Buffer_Create(8);
    SOPC_MsgBuffer* msgBufferFull = MsgBuffer_Create(buffer2, 1, NULL, NULL, NULL);

    // Test Byte nominal and degraded cases
    //// Nominal write
    SOPC_Byte byte = 0xAE;
    status = SOPC_Byte_Write(&byte, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[0] == 0xAE);
    //// Degraded write
    status = SOPC_Byte_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Byte_Write(&byte, NULL);
    ck_assert(status != STATUS_OK);
    msgBufferFull->buffers->position = 8; // Set buffer full
    status = SOPC_Byte_Write(&byte, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    byte = 0x00;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_Byte_Read(&byte, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(byte == 0xAE);
    //// Degraded read
    status = SOPC_Byte_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Byte_Read(&byte, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_Byte_Read(&byte, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test Boolean nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    SOPC_Boolean bool = FALSE;
    status = SOPC_Boolean_Write(&bool, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[0] == FALSE);
    bool = 1; // not FALSE
    status = SOPC_Boolean_Write(&bool, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[1] == 1);
    bool = 2; // not FALSE
    status = SOPC_Boolean_Write(&bool, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[2] == 1); // True value always encoded as 1

    //// Degraded write
    status = SOPC_Boolean_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Boolean_Write(&bool, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_Boolean_Write(&bool, msgBufferFull); // Test with full buffer
    ck_assert(status != STATUS_OK);

    //// Nominal read
    bool = 4;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    msgBuffer->buffers->data[2] = 2; // Simulates a true value received as 2
    status = SOPC_Boolean_Read(&bool, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(bool == FALSE);
    status = SOPC_Boolean_Read(&bool, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(bool == 1);
    status = SOPC_Boolean_Read(&bool, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(bool == 1); // True value always decoded as 1
    //// Degraded read
    status = SOPC_Boolean_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Boolean_Read(&bool, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_Boolean_Read(&bool, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test SByteuv16nal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    SOPC_SByte sbyte = -1;
    status = SOPC_SByte_Write(&sbyte, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    //// Degraded write
    status = SOPC_SByte_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_SByte_Write(&sbyte, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_SByte_Write(&sbyte, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    sbyte = 0x00;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_SByte_Read(&sbyte, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(sbyte == -1);
    //// Degraded read
    status = SOPC_SByte_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_SByte_Read(&sbyte, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_SByte_Read(&sbyte, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test Int16 nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    int16_t v16 = -2;

    status = SOPC_Int16_Write(&v16, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// Little endian encoded
    ck_assert(msgBuffer->buffers->data[0] == 0xFE &&
              msgBuffer->buffers->data[1] == 0xFF);
    //// Degraded write
    status = SOPC_Int16_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Int16_Write(&v16, NULL);
    ck_assert(status != STATUS_OK);
    status = msgBufferFull->buffers->position = 7; // Set buffer almost full (1 byte left)
    status = SOPC_Int16_Write(&v16, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    v16 = 0;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_Int16_Read(&v16, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(v16 == -2);
    //// Degraded read
    status = SOPC_Int16_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Int16_Read(&v16, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_Int16_Read(&v16, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test UInt16 nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    uint16_t vu16 = 2;

    status = SOPC_UInt16_Write(&vu16, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// Little endian encoded
    ck_assert(msgBuffer->buffers->data[0] == 0x02 &&
              msgBuffer->buffers->data[1] == 0x00);
    //// Degraded write
    status = SOPC_UInt16_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_UInt16_Write(&vu16, NULL);
    ck_assert(status != STATUS_OK);
    status = msgBufferFull->buffers->position = 7; // Set buffer almost full (1 byte left)
    status = SOPC_UInt16_Write(&vu16, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    vu16 = 0;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_UInt16_Read(&vu16, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(vu16 == 2);
    //// Degraded read
    status = SOPC_UInt16_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_UInt16_Read(&vu16, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_UInt16_Read(&vu16, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test Int32 nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    int32_t v32 = -2;

    status = SOPC_Int32_Write(&v32, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// Little endian encoded
    ck_assert(msgBuffer->buffers->data[0] == 0xFE &&
              msgBuffer->buffers->data[1] == 0xFF &&
              msgBuffer->buffers->data[2] == 0xFF &&
              msgBuffer->buffers->data[3] == 0xFF);
    //// Degraded write
    status = SOPC_Int32_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Int32_Write(&v32, NULL);
    ck_assert(status != STATUS_OK);
    status = msgBufferFull->buffers->position = 5; // Set buffer almost full (3 byte left)
    status = SOPC_Int32_Write(&v32, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    v32 = 0;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_Int32_Read(&v32, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(v32 == -2);
    //// Degraded read
    status = SOPC_Int32_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Int32_Read(&v32, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_Int32_Read(&v32, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test UInt32 nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    uint32_t vu32 = 1048578;

    status = SOPC_UInt32_Write(&vu32, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// Little endian encoded
    ck_assert(msgBuffer->buffers->data[0] == 0x02 &&
              msgBuffer->buffers->data[1] == 0x00 &&
              msgBuffer->buffers->data[2] == 0x10 &&
              msgBuffer->buffers->data[3] == 0x00);
    //// Degraded write
    status = SOPC_UInt32_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_UInt32_Write(&vu32, NULL);
    ck_assert(status != STATUS_OK);
    status = msgBufferFull->buffers->position = 5; // Set buffer almost full (3 byte left)
    status = SOPC_UInt32_Write(&vu32, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    vu32 = 0;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_UInt32_Read(&vu32, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(vu32 == 1048578);
    //// Degraded read
    status = SOPC_UInt32_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_UInt32_Read(&vu32, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_UInt32_Read(&vu32, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test Int64 nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    int64_t v64 = -2;

    status = SOPC_Int64_Write(&v64, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// Little endian encoded
    ck_assert(msgBuffer->buffers->data[0] == 0xFE &&
              msgBuffer->buffers->data[1] == 0xFF &&
              msgBuffer->buffers->data[2] == 0xFF &&
              msgBuffer->buffers->data[3] == 0xFF &&
              msgBuffer->buffers->data[4] == 0xFF &&
              msgBuffer->buffers->data[5] == 0xFF &&
              msgBuffer->buffers->data[6] == 0xFF &&
              msgBuffer->buffers->data[7] == 0xFF);
    //// Degraded write
    status = SOPC_Int64_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Int64_Write(&v64, NULL);
    ck_assert(status != STATUS_OK);
    status = msgBufferFull->buffers->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_Int64_Write(&v64, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    v64 = 0;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_Int64_Read(&v64, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(v64 == -2);
    //// Degraded read
    status = SOPC_Int64_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Int64_Read(&v64, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_Int64_Read(&v64, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test UInt64 nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    uint64_t vu64 = 0x100000000000002;

    status = SOPC_UInt64_Write(&vu64, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// Little endian encoded
    ck_assert(msgBuffer->buffers->data[0] == 0x02 &&
              msgBuffer->buffers->data[1] == 0x00 &&
              msgBuffer->buffers->data[2] == 0x00 &&
              msgBuffer->buffers->data[3] == 0x00 &&
              msgBuffer->buffers->data[4] == 0x00 &&
              msgBuffer->buffers->data[5] == 0x00 &&
              msgBuffer->buffers->data[6] == 0x00 &&
              msgBuffer->buffers->data[7] == 0x01);
    //// Degraded write
    status = SOPC_UInt64_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_UInt64_Write(&vu64, NULL);
    ck_assert(status != STATUS_OK);
    status = msgBufferFull->buffers->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_UInt64_Write(&vu64, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    vu64 = 0;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_UInt64_Read(&vu64, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(vu64 == 0x100000000000002);
    //// Degraded read
    status = SOPC_UInt64_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_UInt64_Read(&vu64, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_UInt64_Read(&vu64, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test Float nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    float vfloat = -6.5;

    status = SOPC_Float_Write(&vfloat, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// Little endian encoded
    ck_assert(msgBuffer->buffers->data[0] == 0x00 &&
              msgBuffer->buffers->data[1] == 0x00 &&
              msgBuffer->buffers->data[2] == 0xD0 &&
              msgBuffer->buffers->data[3] == 0xC0);
    //// Degraded write
    status = SOPC_Float_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Float_Write(&vfloat, NULL);
    ck_assert(status != STATUS_OK);
    status = msgBufferFull->buffers->position = 5; // Set buffer almost full (3 byte left)
    status = SOPC_Float_Write(&vfloat, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    vfloat = 0;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_Float_Read(&vfloat, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(vfloat == -6.5);
    //// Degraded read
    status = SOPC_Float_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Float_Read(&vfloat, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_Float_Read(&vfloat, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test Double nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    double vdouble = -6.5;

    status = SOPC_Double_Write(&vdouble, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// Little endian encoded
    ck_assert(msgBuffer->buffers->data[0] == 0x00 &&
              msgBuffer->buffers->data[1] == 0x00 &&
              msgBuffer->buffers->data[2] == 0x00 &&
              msgBuffer->buffers->data[3] == 0x00 &&
              msgBuffer->buffers->data[4] == 0x00 &&
              msgBuffer->buffers->data[5] == 0x00 &&
              msgBuffer->buffers->data[6] == 0x1A &&
              msgBuffer->buffers->data[7] == 0xC0);
    //// Degraded write
    status = SOPC_Double_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Double_Write(&vdouble, NULL);
    ck_assert(status != STATUS_OK);
    status = msgBufferFull->buffers->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_Double_Write(&vdouble, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    vdouble = 0;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_Double_Read(&vdouble, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(vdouble == -6.5);
    //// Degraded read
    status = SOPC_Double_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Double_Read(&vdouble, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_Double_Read(&vdouble, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    // Test DateTime nominal and degraded cases
    MsgBuffer_Reset(msgBuffer);
    //// Nominal write
    SOPC_DateTime vDate = -2;

    status = SOPC_DateTime_Write(&vDate, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// Little endian encoded
    ck_assert(msgBuffer->buffers->data[0] == 0xFE &&
              msgBuffer->buffers->data[1] == 0xFF &&
              msgBuffer->buffers->data[2] == 0xFF &&
              msgBuffer->buffers->data[3] == 0xFF &&
              msgBuffer->buffers->data[4] == 0xFF &&
              msgBuffer->buffers->data[5] == 0xFF &&
              msgBuffer->buffers->data[6] == 0xFF &&
              msgBuffer->buffers->data[7] == 0xFF);
    //// Degraded write
    status = SOPC_DateTime_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_DateTime_Write(&vDate, NULL);
    ck_assert(status != STATUS_OK);
    status = msgBufferFull->buffers->position = 1; // Set buffer almost full (7 byte left)
    status = SOPC_DateTime_Write(&vDate, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    vDate = 0;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_DateTime_Read(&vDate, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(vDate == -2);
    //// Degraded read
    status = SOPC_DateTime_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_DateTime_Read(&vDate, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_DateTime_Read(&vDate, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    MsgBuffer_Delete(&msgBuffer);
    MsgBuffer_Delete(&msgBufferFull);
}
END_TEST

START_TEST(test_ua_encoder_other_types)
{
    InitPlatformDependencies(); // Necessary to initialize endianess configuration
    SOPC_StatusCode status = STATUS_OK;
    Buffer* buffer = Buffer_Create(100);
    SOPC_MsgBuffer* msgBuffer = MsgBuffer_Create(buffer, 1, NULL, NULL, NULL);

    Buffer* buffer2 = Buffer_Create(8);
    SOPC_MsgBuffer* msgBufferFull = MsgBuffer_Create(buffer2, 1, NULL, NULL, NULL);

    //////////////////////////////////////////////
    // Test ByteString nominal and degraded cases
    //// Nominal write
    SOPC_ByteString* bs = SOPC_ByteString_Create();
    SOPC_ByteString* bs2 = SOPC_ByteString_Create();
    uint8_t boyString[3] = {0x42, 0x6F, 0x79}; // Boy

    ////// Empty string
    bs->Length = 0;
    status = SOPC_ByteString_Write(bs, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    ck_assert(msgBuffer->buffers->data[1] == 0xFF);
    ck_assert(msgBuffer->buffers->data[2] == 0xFF);
    ck_assert(msgBuffer->buffers->data[3] == 0xFF);

    MsgBuffer_Reset(msgBuffer);
    bs->Length = -1;
    status = SOPC_ByteString_Write(bs, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    ck_assert(msgBuffer->buffers->data[1] == 0xFF);
    ck_assert(msgBuffer->buffers->data[2] == 0xFF);
    ck_assert(msgBuffer->buffers->data[3] == 0xFF);

    MsgBuffer_Reset(msgBuffer);
    bs->Length = -10;
    status = SOPC_ByteString_Write(bs, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    ck_assert(msgBuffer->buffers->data[1] == 0xFF);
    ck_assert(msgBuffer->buffers->data[2] == 0xFF);
    ck_assert(msgBuffer->buffers->data[3] == 0xFF);

    /////// Non empty bytestring
    MsgBuffer_Reset(msgBuffer);
    status = SOPC_ByteString_AttachFromBytes(bs, boyString, 3);
    ck_assert(status == STATUS_OK);
    status = SOPC_ByteString_Write(bs, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[0] == 0x03);
    ck_assert(msgBuffer->buffers->data[1] == 0x00);
    ck_assert(msgBuffer->buffers->data[2] == 0x00);
    ck_assert(msgBuffer->buffers->data[3] == 0x00);
    ck_assert(msgBuffer->buffers->data[4] == 0x42);
    ck_assert(msgBuffer->buffers->data[5] == 0x6F);
    ck_assert(msgBuffer->buffers->data[6] == 0x79);


    //// Degraded write
    status = SOPC_ByteString_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_ByteString_Write(bs, NULL);
    ck_assert(status != STATUS_OK);
    msgBufferFull->buffers->position = 2; // Set buffer almost full (6 byte left)
    status = SOPC_ByteString_Write(bs, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(SOPC_ByteString_Equal(bs, bs2) != FALSE);
    ck_assert(bs2->Length == 3);
    ck_assert(bs2->Data[0] == 0x42);
    ck_assert(bs2->Data[1] == 0x6F);
    ck_assert(bs2->Data[2] == 0x79);

    ////// Read 0 length bytestring
    msgBuffer->buffers->data[0] = 0x00;
    msgBuffer->buffers->data[1] = 0x00;
    msgBuffer->buffers->data[2] = 0x00;
    msgBuffer->buffers->data[3] = 0x00;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);status = SOPC_ByteString_Read(bs2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(bs2->Length == -1); // Null bytestring always decoded -1

    ////// Read negative length bytestring
    msgBuffer->buffers->data[0] = 0xFF;
    msgBuffer->buffers->data[1] = 0x00;
    msgBuffer->buffers->data[2] = 0x00;
    msgBuffer->buffers->data[3] = 0xFF;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(bs2->Length == -1); // Null bytestring always decoded -1

    ////// Read -1 length bytestring
    msgBuffer->buffers->data[0] = 0xFF;
    msgBuffer->buffers->data[1] = 0xFF;
    msgBuffer->buffers->data[2] = 0xFF;
    msgBuffer->buffers->data[3] = 0xFF;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_ByteString_Clear(bs2);
    SOPC_ByteString_Initialize(bs2);
    status = SOPC_ByteString_Read(bs2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(bs2->Length == -1); // Null bytestring always decoded -1
    //// Degraded read
    status = SOPC_ByteString_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_ByteString_Read(bs, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_ByteString_Read(bs, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    SOPC_ByteString_Delete(bs);
    SOPC_ByteString_Delete(bs2);
    bs = NULL;
    bs2 = NULL;


    /////////////////////////////////////////
    // Test String nominal and degraded cases
    //// Nominal write
    MsgBuffer_Reset(msgBuffer);
    SOPC_String str;
    SOPC_String_Initialize(&str);
    SOPC_String str2;
    SOPC_String_Initialize(&str2);

    ////// Empty string
    str.Length = 0;
    status = SOPC_String_Write(&str, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    ck_assert(msgBuffer->buffers->data[1] == 0xFF);
    ck_assert(msgBuffer->buffers->data[2] == 0xFF);
    ck_assert(msgBuffer->buffers->data[3] == 0xFF);

    MsgBuffer_Reset(msgBuffer);
    str.Length = -1;
    status = SOPC_String_Write(&str, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    ck_assert(msgBuffer->buffers->data[1] == 0xFF);
    ck_assert(msgBuffer->buffers->data[2] == 0xFF);
    ck_assert(msgBuffer->buffers->data[3] == 0xFF);

    MsgBuffer_Reset(msgBuffer);
    str.Length = -10;
    status = SOPC_String_Write(&str, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    ck_assert(msgBuffer->buffers->data[1] == 0xFF);
    ck_assert(msgBuffer->buffers->data[2] == 0xFF);
    ck_assert(msgBuffer->buffers->data[3] == 0xFF);

    /////// Non empty bytestring
    MsgBuffer_Reset(msgBuffer);
    status = SOPC_String_AttachFromCstring(&str, "Boy");
    ck_assert(status == STATUS_OK);
    status = SOPC_String_Write(&str, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[0] == 0x03);
    ck_assert(msgBuffer->buffers->data[1] == 0x00);
    ck_assert(msgBuffer->buffers->data[2] == 0x00);
    ck_assert(msgBuffer->buffers->data[3] == 0x00);
    ck_assert(msgBuffer->buffers->data[4] == 0x42);
    ck_assert(msgBuffer->buffers->data[5] == 0x6F);
    ck_assert(msgBuffer->buffers->data[6] == 0x79);


    //// Degraded write
    status = SOPC_String_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_String_Write(&str, NULL);
    ck_assert(status != STATUS_OK);
    msgBufferFull->buffers->position = 2; // Set buffer almost full (6 byte left)
    status = SOPC_String_Write(&str, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(SOPC_String_Equal(&str, &str2) != FALSE);
    ck_assert(memcmp(SOPC_String_GetRawCString(&str2), "Boy", 3) == 0);

    ////// Read 0 length bytestring
    msgBuffer->buffers->data[0] = 0x00;
    msgBuffer->buffers->data[1] = 0x00;
    msgBuffer->buffers->data[2] = 0x00;
    msgBuffer->buffers->data[3] = 0x00;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);status = SOPC_String_Read(&str2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(str2.Length == -1); // Null bytestring always decoded -1

    ////// Read negative length bytestring
    msgBuffer->buffers->data[0] = 0xFF;
    msgBuffer->buffers->data[1] = 0x00;
    msgBuffer->buffers->data[2] = 0x00;
    msgBuffer->buffers->data[3] = 0xFF;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(str2.Length == -1); // Null bytestring always decoded -1

    ////// Read -1 length bytestring
    msgBuffer->buffers->data[0] = 0xFF;
    msgBuffer->buffers->data[1] = 0xFF;
    msgBuffer->buffers->data[2] = 0xFF;
    msgBuffer->buffers->data[3] = 0xFF;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_String_Clear(&str2);
    SOPC_String_Initialize(&str2);
    status = SOPC_String_Read(&str2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(str2.Length == -1); // Null bytestring always decoded -1
    //// Degraded read
    status = SOPC_String_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_String_Read(&str, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_String_Read(&str, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    SOPC_String_Clear(&str);
    SOPC_String_Clear(&str2);

    //////////////////////////////////////////
    // Test XmlElement nominal and degraded cases
    //// Nominal write
    MsgBuffer_Reset(msgBuffer);
    SOPC_XmlElement xmlElt;
    SOPC_XmlElement_Initialize(&xmlElt);
    SOPC_XmlElement xmlElt2;
    SOPC_XmlElement_Initialize(&xmlElt2);
    uint8_t balA[3] = {0x3C, 0x41, 0x3E}; // <A>

    ////// Empty string
    xmlElt.Length = 0;
    status = SOPC_XmlElement_Write(&xmlElt, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    ck_assert(msgBuffer->buffers->data[1] == 0xFF);
    ck_assert(msgBuffer->buffers->data[2] == 0xFF);
    ck_assert(msgBuffer->buffers->data[3] == 0xFF);

    MsgBuffer_Reset(msgBuffer);
    xmlElt.Length = -1;
    status = SOPC_XmlElement_Write(&xmlElt, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    ck_assert(msgBuffer->buffers->data[1] == 0xFF);
    ck_assert(msgBuffer->buffers->data[2] == 0xFF);
    ck_assert(msgBuffer->buffers->data[3] == 0xFF);

    MsgBuffer_Reset(msgBuffer);
    xmlElt.Length = -10;
    status = SOPC_XmlElement_Write(&xmlElt, msgBuffer);
    ck_assert(status == STATUS_OK);
    ////// -1 length must be encoded for null string
    ck_assert(msgBuffer->buffers->data[0] == 0xFF);
    ck_assert(msgBuffer->buffers->data[1] == 0xFF);
    ck_assert(msgBuffer->buffers->data[2] == 0xFF);
    ck_assert(msgBuffer->buffers->data[3] == 0xFF);

    /////// Non empty bytestring
    MsgBuffer_Reset(msgBuffer);
    xmlElt.Data = malloc(sizeof(SOPC_Byte) * 3);
    ck_assert(xmlElt.Data != NULL);
    ck_assert(memcpy(xmlElt.Data, balA, 3) == xmlElt.Data);
    xmlElt.Length = 3;
    status = SOPC_XmlElement_Write(&xmlElt, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[0] == 0x03);
    ck_assert(msgBuffer->buffers->data[1] == 0x00);
    ck_assert(msgBuffer->buffers->data[2] == 0x00);
    ck_assert(msgBuffer->buffers->data[3] == 0x00);
    ck_assert(msgBuffer->buffers->data[4] == 0x3C);
    ck_assert(msgBuffer->buffers->data[5] == 0x41);
    ck_assert(msgBuffer->buffers->data[6] == 0x3E);


    //// Degraded write
    status = SOPC_XmlElement_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_XmlElement_Write(&xmlElt, NULL);
    ck_assert(status != STATUS_OK);
    msgBufferFull->buffers->position = 2; // Set buffer almost full (6 byte left)
    status = SOPC_XmlElement_Write(&xmlElt, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(memcmp(xmlElt.Data, xmlElt2.Data, 3) == 00);
    ck_assert(xmlElt2.Length == 3);
    ck_assert(xmlElt2.Data[0] == 0x3C);
    ck_assert(xmlElt2.Data[1] == 0x41);
    ck_assert(xmlElt2.Data[2] == 0x3E);

    ////// Read 0 length bytestring
    msgBuffer->buffers->data[0] = 0x00;
    msgBuffer->buffers->data[1] = 0x00;
    msgBuffer->buffers->data[2] = 0x00;
    msgBuffer->buffers->data[3] = 0x00;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(xmlElt2.Length == -1); // Null bytestring always decoded -1

    ////// Read negative length bytestring
    msgBuffer->buffers->data[0] = 0xFF;
    msgBuffer->buffers->data[1] = 0x00;
    msgBuffer->buffers->data[2] = 0x00;
    msgBuffer->buffers->data[3] = 0xFF;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(xmlElt2.Length == -1); // Null bytestring always decoded -1

    ////// Read -1 length bytestring
    msgBuffer->buffers->data[0] = 0xFF;
    msgBuffer->buffers->data[1] = 0xFF;
    msgBuffer->buffers->data[2] = 0xFF;
    msgBuffer->buffers->data[3] = 0xFF;
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    SOPC_XmlElement_Clear(&xmlElt2);
    SOPC_XmlElement_Initialize(&xmlElt2);
    status = SOPC_XmlElement_Read(&xmlElt2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(xmlElt2.Length == -1); // Null bytestring always decoded -1
    //// Degraded read
    status = SOPC_XmlElement_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_XmlElement_Read(&xmlElt, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_XmlElement_Read(&xmlElt, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    SOPC_XmlElement_Clear(&xmlElt);
    SOPC_XmlElement_Clear(&xmlElt2);

    // Resize "full" msg buffer
    /////////////////////////////////////////
    MsgBuffer_Delete(&msgBufferFull);
    buffer2 = Buffer_Create(32);
    msgBufferFull = MsgBuffer_Create(buffer2, 1, NULL, NULL, NULL);
    /////////////////////////////////////////
    // Test GUID nominal and degraded cases
    //// Nominal write
    MsgBuffer_Reset(msgBuffer);
    SOPC_Guid guid;
    SOPC_Guid_Initialize(&guid);
    SOPC_Guid guid2;
    SOPC_Guid_Initialize(&guid2);
    guid.Data1 = 0x72962B91;
    guid.Data2 = 0xFA75;
    guid.Data3 = 0x4ae6;
    guid.Data4[0] = 0x8D;
    guid.Data4[1] = 0x28;
    guid.Data4[2] = 0xB4;
    guid.Data4[3] = 0x04;
    guid.Data4[4] = 0xDC;
    guid.Data4[5] = 0x7D;
    guid.Data4[6] = 0xAF;
    guid.Data4[7] = 0x63;

    status = SOPC_Guid_Write(&guid, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[0] == 0x91);
    ck_assert(msgBuffer->buffers->data[1] == 0x2B);
    ck_assert(msgBuffer->buffers->data[2] == 0x96);
    ck_assert(msgBuffer->buffers->data[3] == 0x72);
    ck_assert(msgBuffer->buffers->data[4] == 0x75);
    ck_assert(msgBuffer->buffers->data[5] == 0xFA);
    ck_assert(msgBuffer->buffers->data[6] == 0xE6);
    ck_assert(msgBuffer->buffers->data[7] == 0x4A);
    ck_assert(msgBuffer->buffers->data[8] == 0x8D);
    ck_assert(msgBuffer->buffers->data[9] == 0x28);
    ck_assert(msgBuffer->buffers->data[10] == 0xB4);
    ck_assert(msgBuffer->buffers->data[11] == 0x04);
    ck_assert(msgBuffer->buffers->data[12] == 0xDC);
    ck_assert(msgBuffer->buffers->data[13] == 0x7D);
    ck_assert(msgBuffer->buffers->data[14] == 0xAF);
    ck_assert(msgBuffer->buffers->data[15] == 0x63);


    //// Degraded write
    status = SOPC_Guid_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Guid_Write(&guid, NULL);
    ck_assert(status != STATUS_OK);
    msgBufferFull->buffers->position = 17; // Set buffer almost full (15 byte left)
    status = SOPC_Guid_Write(&guid, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_Guid_Read(&guid2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(memcmp(&guid, &guid2, sizeof(SOPC_Guid)) == 0);

    //// Degraded read
    status = SOPC_Guid_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_Guid_Read(&guid, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_Guid_Read(&guid, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    SOPC_Guid_Clear(&guid);
    SOPC_Guid_Clear(&guid2);

    /////////////////////////////////////////
    // Test NodeId nominal and degraded cases
    //// Nominal write
    MsgBuffer_Reset(msgBuffer);
    SOPC_NodeId nodeId;
    SOPC_NodeId_Initialize(&nodeId);
    SOPC_NodeId nodeId2;
    SOPC_NodeId_Initialize(&nodeId2);

    // Two bytes node id
    nodeId.IdentifierType = IdentifierType_Numeric;
    nodeId.Data.Numeric = 114;
    status = SOPC_NodeId_Write(&nodeId, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[0] == 0x00);
    ck_assert(msgBuffer->buffers->data[1] == 0x72);

    // Four bytes node id
    nodeId.IdentifierType = IdentifierType_Numeric;
    nodeId.Namespace = 5;
    nodeId.Data.Numeric = 1025;
    status = SOPC_NodeId_Write(&nodeId, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[2] == 0x01);
    ck_assert(msgBuffer->buffers->data[3] == 0x05);
    ck_assert(msgBuffer->buffers->data[4] == 0x01);
    ck_assert(msgBuffer->buffers->data[5] == 0x04);

    // Numeric node id
    nodeId.IdentifierType = IdentifierType_Numeric;
    nodeId.Namespace = 5;
    nodeId.Data.Numeric = 0x1FFFF;
    status = SOPC_NodeId_Write(&nodeId, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuffer->buffers->data[6] == 0x02);
    ck_assert(msgBuffer->buffers->data[7] == 0x05);
    ck_assert(msgBuffer->buffers->data[8] == 0x00);
    ck_assert(msgBuffer->buffers->data[9] == 0xFF);
    ck_assert(msgBuffer->buffers->data[10] == 0xFF);
    ck_assert(msgBuffer->buffers->data[11] == 0x01);
    ck_assert(msgBuffer->buffers->data[12] == 0x00);

    // TODO: write all other types possibles !

    //// Degraded write
    status = SOPC_NodeId_Write(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_NodeId_Write(&nodeId, NULL);
    ck_assert(status != STATUS_OK);
    msgBufferFull->buffers->position = 26; // Set buffer almost full (6 byte left)
    status = SOPC_NodeId_Write(&nodeId, msgBufferFull);
    ck_assert(status != STATUS_OK);

    //// Nominal read
    ////// Two bytes NodeId
    Buffer_SetPosition(msgBuffer->buffers, 0); // Reset position for reading
    status = SOPC_NodeId_Read(&nodeId2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(nodeId2.Namespace == 0);
    ck_assert(nodeId2.Data.Numeric == 114);

    ////// Four bytes NodeId
    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(nodeId2.Namespace == 5);
    ck_assert(nodeId2.Data.Numeric == 1025);

    ////// Numeric NodeId
    SOPC_NodeId_Clear(&nodeId2);
    status = SOPC_NodeId_Read(&nodeId2, msgBuffer);
    ck_assert(status == STATUS_OK);
    ck_assert(nodeId2.Namespace == 5);
    ck_assert(nodeId2.Data.Numeric == 0x1FFFF);

    // TODO: read all other types possibles !

    //// Degraded read
    status = SOPC_NodeId_Read(NULL, msgBuffer);
    ck_assert(status != STATUS_OK);
    status = SOPC_NodeId_Read(&nodeId, NULL);
    ck_assert(status != STATUS_OK);
    status = SOPC_NodeId_Read(&nodeId, msgBuffer); // Nothing to read anymore
    ck_assert(status != STATUS_OK);

    SOPC_NodeId_Clear(&nodeId);
    SOPC_NodeId_Clear(&nodeId2);


    MsgBuffer_Delete(&msgBuffer);
    MsgBuffer_Delete(&msgBufferFull);
}
END_TEST

Suite *tests_make_suite_core_tools(void)
{
    Suite *s;
    TCase *tc_msgbuffer, *tc_encoder;

    s = suite_create("Core Tools");
    tc_msgbuffer = tcase_create("UA Message Buffer");
    tcase_add_test(tc_msgbuffer, test_ua_msg_buffer_create_set_type);
    tcase_add_test(tc_msgbuffer, test_ua_msg_buffer_reset);
    tcase_add_test(tc_msgbuffer, test_ua_msg_buffer_copy);
    tcase_add_test(tc_msgbuffer, test_ua_msg_buffers_create);
    tcase_add_test(tc_msgbuffer, test_ua_msg_buffers_chunk_mgr);
    tcase_add_test(tc_msgbuffer, test_ua_msg_buffers_copy);
    suite_add_tcase(s, tc_msgbuffer);
    tc_encoder = tcase_create("UA Encoder");
    tcase_add_test(tc_encoder, test_ua_encoder_endianess_mgt);
    tcase_add_test(tc_encoder, test_ua_encoder_basic_types);
    tcase_add_test(tc_encoder, test_ua_encoder_other_types);
    suite_add_tcase(s, tc_encoder);

    return s;
}
