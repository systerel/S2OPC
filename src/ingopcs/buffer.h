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
    uint32_t max_size;
    uint32_t size;
    uint32_t position;
    UA_Byte* data;
} Buffer;

Buffer* Create_Buffer(uint32_t size);
Buffer* Set_Buffer(UA_Byte* data, uint32_t size);
void Delete_Buffer(Buffer* buffer);


#endif /* INGOPCS_BUFFER_H_ */
