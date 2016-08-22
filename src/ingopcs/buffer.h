/*
 * buffer.h
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#ifndef INGOPCS_BUFFER_H_
#define INGOPCS_BUFFER_H_

#include <opcua_ingopcs_types.h>

typedef struct Buffer {
    uint32_t max_size; // maximum size
    uint32_t position; // read/write position
    uint32_t length; // data length
    UA_Byte* data;
} Buffer;

Buffer* Create_Buffer(uint32_t size);
StatusCode Init_Buffer(Buffer* buffer, uint32_t size);
Buffer* Set_Buffer(UA_Byte* data, uint32_t position, uint32_t length, uint32_t maxsize);
void Delete_Buffer(Buffer* buffer);
void Reset_Buffer(Buffer* buffer);
void Reset_Buffer_After_Position(Buffer*  buffer,
                                 uint32_t position);

StatusCode Set_Position_Buffer(Buffer* buffer, uint32_t position);
StatusCode Set_Data_Length_Buffer(Buffer* buffer, uint32_t length);

StatusCode Write_Buffer(Buffer* buffer, const UA_Byte* data_src, uint32_t count);
StatusCode Read_Buffer(UA_Byte* data_dest, Buffer* buffer, uint32_t count);

#endif /* INGOPCS_BUFFER_H_ */
