/*
 * buffer.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_BUFFER_H_
#define INGOPCS_BUFFER_H_

#include <ua_base_types.h>

typedef struct {
    uint32_t max_size; // maximum size
    uint32_t position; // read/write position
    uint32_t length; // data length
    uint8_t* data;
} Buffer;

Buffer* Buffer_Create(uint32_t size);
StatusCode Buffer_Init(Buffer* buffer, uint32_t size);
Buffer* Buffer_Set_Data(uint8_t* data, uint32_t position, uint32_t length, uint32_t maxsize);
void Buffer_Delete(Buffer* buffer);
void Buffer_Reset(Buffer* buffer);
void Buffer_ResetAfterPosition(Buffer*  buffer,
                                 uint32_t position);

StatusCode Buffer_SetPosition(Buffer* buffer, uint32_t position);
StatusCode Buffer_SetDataLength(Buffer* buffer, uint32_t length);

StatusCode Buffer_Write(Buffer* buffer, const uint8_t* data_src, uint32_t count);
StatusCode Buffer_Read(uint8_t* data_dest, Buffer* buffer, uint32_t count);

StatusCode Buffer_Copy(Buffer* dest, Buffer* src);

StatusCode Buffer_CopyWithLength(Buffer* dest, Buffer* src, uint32_t limitedLength);

#endif /* INGOPCS_BUFFER_H_ */
