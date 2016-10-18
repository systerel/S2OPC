/*
 * Entry point for tools tests. Tests use libcheck.
 *
 * If you want to debug the exe, you should define env var CK_FORK=no
 * http://check.sourceforge.net/doc/check_html/check_4.html#No-Fork-Mode
 *
 *  Created on: Oct 7, 2016
 *      Author: VMO
 */


#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <ua_msg_buffer.h>
#include <ua_encoder.h>
#include "check_stack.h"


START_TEST(test_ua_msg_buffer_create_set_type)
{
    StatusCode status = 0;
    int flushData = 3;
    UA_NamespaceTable table;
    UA_EncodeableType* encTypes;

    // Test creation / set type
    //// Test nominal case
    Buffer* buf = Buffer_Create(10);
    UA_MsgBuffer* msgBuf = MsgBuffer_Create(buf, 1, &flushData, &table, &encTypes);
    ck_assert(msgBuf != NULL);
    ck_assert(msgBuf->nbBuffers == 1);
    ck_assert(msgBuf->buffers == buf);
    ck_assert(msgBuf->type == TCP_UA_Message_Unknown);
    ck_assert(msgBuf->secureType == UA_SecureMessage);
    ck_assert(msgBuf->currentChunkSize == 0);
    ck_assert(msgBuf->nbChunks == 1);
    ck_assert(msgBuf->maxChunks == 1);
    ck_assert(msgBuf->sequenceNumberPosition == 0);
    ck_assert(msgBuf->isFinal == UA_Msg_Chunk_Unknown);
    ck_assert(msgBuf->receivedReqId == 0);
    ck_assert(msgBuf->flushData == &flushData);
    ck_assert(msgBuf->nsTable.namespaceArray == table.namespaceArray);
    ck_assert(msgBuf->encTypesTable == &encTypes);

    status = MsgBuffer_SetSecureMsgType(msgBuf, UA_OpenSecureChannel);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuf->secureType == UA_OpenSecureChannel);

    msgBuf->type = TCP_UA_Message_SecureMessage;
    status = MsgBuffer_SetSecureMsgType(msgBuf, UA_CloseSecureChannel);
    ck_assert(status == STATUS_OK);
    ck_assert(msgBuf->secureType == UA_CloseSecureChannel);

    //// Test set type degraded cases
    status = MsgBuffer_SetSecureMsgType(NULL, UA_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    msgBuf->type = TCP_UA_Message_Hello; // Not a secure type
    status = MsgBuffer_SetSecureMsgType(msgBuf, UA_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    msgBuf->type = TCP_UA_Message_Acknowledge; // Not a secure type
    status = MsgBuffer_SetSecureMsgType(msgBuf, UA_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    msgBuf->type = TCP_UA_Message_Error; // Not a secure type
    status = MsgBuffer_SetSecureMsgType(msgBuf, UA_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    msgBuf->type = TCP_UA_Message_Invalid; // Not a secure type
    status = MsgBuffer_SetSecureMsgType(msgBuf, UA_CloseSecureChannel);
    ck_assert(status != STATUS_OK);

    MsgBuffer_Delete(&msgBuf);

    //// Test buffer creation degraded cases
    msgBuf = MsgBuffer_Create(NULL, 1, &flushData, &table, &encTypes);
    ck_assert(msgBuf == NULL);


}
END_TEST

START_TEST(test_ua_msg_buffer_reset)
{
    StatusCode status = 0;
    uint8_t flushData = 3;
    UA_NamespaceTable table;
    UA_EncodeableType* encTypes;

    // Test reset
    //// Test nominal case
    Buffer* buf = Buffer_Create(10);
    UA_MsgBuffer* msgBuf = MsgBuffer_Create(buf, 1, &flushData, &table, &encTypes);
    ////// Modify properties
    Buffer_Write(buf, &flushData, 1);
    msgBuf->type = TCP_UA_Message_SecureMessage;
    MsgBuffer_SetSecureMsgType(msgBuf, UA_CloseSecureChannel);
    msgBuf->currentChunkSize = 9;
    msgBuf->nbChunks = 2;
    msgBuf->maxChunks = 3;
    msgBuf->sequenceNumberPosition = 2;
    msgBuf->isFinal = UA_Msg_Chunk_Final;
    msgBuf->receivedReqId = 1;
    msgBuf->flushData = NULL;
    msgBuf->nsTable.namespaceArray = NULL;
    msgBuf->encTypesTable = NULL;
    ////// Check modified properties
    ck_assert(msgBuf->buffers->data[0] == flushData);
    ck_assert(msgBuf->buffers->length == 1);
    ck_assert(msgBuf->secureType == UA_CloseSecureChannel);

    ////// Reset msg buffer
    MsgBuffer_Reset(msgBuf);
    ////// Check reset properties
    ck_assert(msgBuf->buffers->data[0] == 0);
    ck_assert(msgBuf->buffers->length == 0);
    ck_assert(msgBuf->type == TCP_UA_Message_Unknown);
    ck_assert(msgBuf->secureType == UA_SecureMessage);
    ck_assert(msgBuf->currentChunkSize == 0);
    ck_assert(msgBuf->nbChunks == 1);
    ck_assert(msgBuf->sequenceNumberPosition == 0);
    ck_assert(msgBuf->isFinal == UA_Msg_Chunk_Unknown);
    ck_assert(msgBuf->receivedReqId == 0);

    /////// Check properties not reset
    ck_assert(msgBuf->maxChunks == 3);
    ck_assert(msgBuf->flushData == NULL);
    ck_assert(msgBuf->nsTable.namespaceArray == NULL);
    ck_assert(msgBuf->encTypesTable == NULL);
    MsgBuffer_Delete(&msgBuf);

    ////// Modify properties
    buf = Buffer_Create(10);
    msgBuf = MsgBuffer_Create(buf, 2, &flushData, &table, &encTypes);
    Buffer_Write(buf, &flushData, 1);
    Buffer_Write(buf, &flushData, 1);
    msgBuf->type = TCP_UA_Message_SecureMessage;
    MsgBuffer_SetSecureMsgType(msgBuf, UA_CloseSecureChannel);
    msgBuf->currentChunkSize = 9;
    msgBuf->sequenceNumberPosition = 2;
    msgBuf->isFinal = UA_Msg_Chunk_Final;
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
    ck_assert(msgBuf->secureType == UA_CloseSecureChannel);
    ck_assert(msgBuf->sequenceNumberPosition == 2);
    ck_assert(msgBuf->isFinal == UA_Msg_Chunk_Final);
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
    StatusCode status = 0;
    uint8_t flushData = 3;
    UA_NamespaceTable table;
    UA_EncodeableType* encTypes;

    // Test reset
    //// Test nominal case
    Buffer* buf = Buffer_Create(10);
    UA_MsgBuffer* msgBuf = MsgBuffer_Create(buf, 1, &flushData, &table, &encTypes);
    ////// Modify properties
    Buffer_Write(buf, &flushData, 1);
    msgBuf->type = TCP_UA_Message_SecureMessage;
    MsgBuffer_SetSecureMsgType(msgBuf, UA_CloseSecureChannel);
    msgBuf->currentChunkSize = 9;
    msgBuf->nbChunks = 2;
    msgBuf->maxChunks = 3;
    msgBuf->sequenceNumberPosition = 2;
    msgBuf->isFinal = UA_Msg_Chunk_Final;
    msgBuf->receivedReqId = 1;
    ////// Check modified properties
    ck_assert(msgBuf->buffers->data[0] == flushData);
    ck_assert(msgBuf->buffers->length == 1);
    ck_assert(msgBuf->secureType == UA_CloseSecureChannel);

    Buffer* bufDest = Buffer_Create(2);
    UA_MsgBuffer* msgBufDest = MsgBuffer_Create(bufDest, 2, NULL, NULL, NULL);
    ////// Reset msg buffer
    status = MsgBuffer_CopyBuffer(msgBufDest, msgBuf);
    ck_assert(status == STATUS_OK);
    ////// Check copied properties
    ck_assert(msgBufDest->buffers != msgBuf->buffers);
    ck_assert(msgBufDest->buffers->data[0] == flushData);
    ck_assert(msgBufDest->buffers->length == 1);
    ck_assert(msgBufDest->type == TCP_UA_Message_SecureMessage);
    ck_assert(msgBufDest->secureType == UA_CloseSecureChannel);
    ck_assert(msgBufDest->currentChunkSize == 9);
    ck_assert(msgBufDest->nbChunks == 2);
    ck_assert(msgBufDest->sequenceNumberPosition == 2);
    ck_assert(msgBufDest->isFinal == UA_Msg_Chunk_Final);
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
    UA_NamespaceTable table;
    UA_EncodeableType* encTypes;

    // Test creation
    //// Test nominal case
    UA_MsgBuffers* msgBuf = MsgBuffers_Create(3, 10, &table, &encTypes);
    ck_assert(msgBuf != NULL);
    ck_assert(msgBuf->nbBuffers == 3);
    ck_assert(msgBuf->buffers[0].data != NULL);
    ck_assert(msgBuf->buffers[1].data != NULL);
    ck_assert(msgBuf->buffers[2].data != NULL);
    ck_assert(msgBuf->type == TCP_UA_Message_Unknown);
    ck_assert(msgBuf->secureType == UA_SecureMessage);
    ck_assert(msgBuf->currentChunkSize == 0);
    ck_assert(msgBuf->nbChunks == 0);
    ck_assert(msgBuf->maxChunks == 3);
    ck_assert(msgBuf->sequenceNumberPosition == 0);
    ck_assert(msgBuf->isFinal == UA_Msg_Chunk_Unknown);
    ck_assert(msgBuf->receivedReqId == 0);
    ck_assert(msgBuf->nsTable.namespaceArray == table.namespaceArray);
    ck_assert(msgBuf->encTypesTable == &encTypes);
    MsgBuffers_Delete(&msgBuf);

    //// Test buffer creation degraded cases
    msgBuf = MsgBuffers_Create(0, 10, &table, &encTypes);
    ck_assert(msgBuf == NULL);
    msgBuf = MsgBuffers_Create(3, 0, &table, &encTypes);
    ck_assert(msgBuf == NULL);
    msgBuf = MsgBuffers_Create(3, 10, NULL, &encTypes);
    ck_assert(msgBuf == NULL);
    msgBuf = MsgBuffers_Create(3, 10, &table, NULL);
    ck_assert(msgBuf == NULL);

}
END_TEST

START_TEST(test_ua_msg_buffers_chunk_mgr)
{
    UA_NamespaceTable table;
    UA_EncodeableType* encTypes;
    Buffer* buf = NULL;
    Buffer* buf2 = NULL;
    uint32_t bufIdx = 0;
    StatusCode status = STATUS_NOK;

    // Test chunks management
    //// Test nominal case
    UA_MsgBuffers* msgBuf = MsgBuffers_Create(3, 10, &table, &encTypes);

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
    msgBuf = MsgBuffers_Create(3, 10, &table, &encTypes);
    ////// Set current chunk first with only 0 chunk
    status = MsgBuffers_SetCurrentChunkFirst(msgBuf);
    ck_assert(status != STATUS_OK);
    MsgBuffers_Delete(&msgBuf);
}
END_TEST

START_TEST(test_ua_msg_buffers_copy)
{
    StatusCode status = 0;
    uint8_t flushData = 3;
    UA_NamespaceTable table;
    UA_EncodeableType* encTypes;

    // Test reset
    //// Test nominal case
    Buffer* buf = Buffer_Create(10);
    UA_MsgBuffer* msgBuf = MsgBuffer_Create(buf, 1, &flushData, &table, &encTypes);
    ////// Modify properties
    Buffer_Write(buf, &flushData, 1);
    msgBuf->type = TCP_UA_Message_SecureMessage;
    MsgBuffer_SetSecureMsgType(msgBuf, UA_CloseSecureChannel);
    msgBuf->currentChunkSize = 9;
    msgBuf->nbChunks = 2;
    msgBuf->maxChunks = 3;
    msgBuf->sequenceNumberPosition = 2;
    msgBuf->isFinal = UA_Msg_Chunk_Final;
    msgBuf->receivedReqId = 1;
    ////// Check modified properties
    ck_assert(msgBuf->buffers->data[0] == flushData);
    ck_assert(msgBuf->buffers->length == 1);
    ck_assert(msgBuf->secureType == UA_CloseSecureChannel);

    UA_MsgBuffer* msgBufsDest = MsgBuffers_Create(3, 5, &table, &encTypes);
    ////// Copy msg buffer
    status = MsgBuffers_CopyBuffer(msgBufsDest, 0,
                                   msgBuf, 1);
    ck_assert(status == STATUS_OK);
    ////// Check copied properties
    ck_assert(msgBufsDest->buffers[0].data != msgBuf->buffers->data);
    ck_assert(msgBufsDest->buffers[0].data[0] == flushData);
    ck_assert(msgBufsDest->buffers[0].length == 1);
    ck_assert(msgBufsDest->type == TCP_UA_Message_SecureMessage);
    ck_assert(msgBufsDest->secureType == UA_CloseSecureChannel);
    ck_assert(msgBufsDest->currentChunkSize == 9);
    ck_assert(msgBufsDest->nbChunks == 2);
    ck_assert(msgBufsDest->sequenceNumberPosition == 2);
    ck_assert(msgBufsDest->isFinal == UA_Msg_Chunk_Final);
    ck_assert(msgBufsDest->receivedReqId == 1);
    /////// Check not concerned fields were not copied
    ck_assert(msgBufsDest->maxChunks == 3);
    ck_assert(msgBufsDest->flushData == NULL);
    ck_assert(msgBufsDest->nsTable.namespaceArray == table.namespaceArray);
    ck_assert(msgBufsDest->encTypesTable == &encTypes);

    status = MsgBuffers_CopyBuffer(msgBufsDest, 1,
                                   msgBuf, 1);
    ck_assert(status == STATUS_OK);
    ////// Check copied properties
    ck_assert(msgBufsDest->buffers[1].data != msgBuf->buffers->data);
    ck_assert(msgBufsDest->buffers[1].data[0] == flushData);
    ck_assert(msgBufsDest->buffers[1].length == 1);
    ck_assert(msgBufsDest->type == TCP_UA_Message_SecureMessage);
    ck_assert(msgBufsDest->secureType == UA_CloseSecureChannel);
    ck_assert(msgBufsDest->currentChunkSize == 9);
    ck_assert(msgBufsDest->nbChunks == 2);
    ck_assert(msgBufsDest->sequenceNumberPosition == 2);
    ck_assert(msgBufsDest->isFinal == UA_Msg_Chunk_Final);
    ck_assert(msgBufsDest->receivedReqId == 1);
    /////// Check not concerned fields were not copied
    ck_assert(msgBufsDest->maxChunks == 3);
    ck_assert(msgBufsDest->flushData == NULL);
    ck_assert(msgBufsDest->nsTable.namespaceArray == table.namespaceArray);
    ck_assert(msgBufsDest->encTypesTable == &encTypes);


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
    EncodeDecode_Int16(&v16);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC);

    bytes = (uint8_t*) &vu16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    EncodeDecode_UInt16(&vu16);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC);

    bytes = (uint8_t*) &v32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    EncodeDecode_Int32(&v32);
    ck_assert(bytes[0] == 0xAB && bytes[1] == 0xBC &&
              bytes[2] == 0xCD && bytes[3] == 0xDE);

    bytes = (uint8_t*) &vu32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    EncodeDecode_UInt32(&vu32);
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
    EncodeDecode_Int64(&v64);
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
    EncodeDecode_UInt64(&vu64);
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
    EncodeDecode_Float(&vfloat);
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
    EncodeDecode_Double(&vdouble);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 &&
              bytes[2] == 0x00 && bytes[3] == 0x00 &&
              bytes[4] == 0x00 && bytes[5] == 0x00 &&
              bytes[6] == 0x1A && bytes[7] == 0xC0);


    // Test encoding with different endianess in machine and UA binary
    endianess = P_Endianess_BigEndian;
    bytes = (uint8_t*) &v16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    EncodeDecode_Int16(&v16);
    ck_assert(bytes[1] == 0xAB && bytes[0] == 0xBC);

    bytes = (uint8_t*) &vu16;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    EncodeDecode_UInt16(&vu16);
    ck_assert(bytes[1] == 0xAB && bytes[0] == 0xBC);

    bytes = (uint8_t*) &v32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    EncodeDecode_Int32(&v32);
    ck_assert(bytes[3] == 0xAB && bytes[2] == 0xBC &&
              bytes[1] == 0xCD && bytes[0] == 0xDE);

    bytes = (uint8_t*) &vu32;
    bytes[0] = 0xAB;
    bytes[1] = 0xBC;
    bytes[2] = 0xCD;
    bytes[3] = 0xDE;
    EncodeDecode_UInt32(&vu32);
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
    EncodeDecode_Int64(&v64);
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
    EncodeDecode_UInt64(&vu64);
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
    EncodeDecode_Float(&vfloat);
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
    EncodeDecode_Double(&vdouble);
    ck_assert(bytes[0] == 0x00 && bytes[1] == 0x00 &&
              bytes[2] == 0x00 && bytes[3] == 0x00 &&
              bytes[4] == 0x00 && bytes[5] == 0x00 &&
              bytes[6] == 0x1A && bytes[7] == 0xC0);

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
    suite_add_tcase(s, tc_encoder);

    return s;
}
