/*
 * buffer.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <stdlib.h>
#include <string.h>
#include <buffer.h>

Buffer* Create_Buffer(uint32_t size){
	Buffer* buf = NULL;
	if(size > 0){
		buf = (Buffer*) malloc(sizeof(Buffer));
		if(buf != NULL){
			buf->size = size;
			buf->data = (UA_Byte*) malloc(sizeof(UA_Byte)*size);
			memset(buf->data, 0, sizeof(UA_Byte)*size);
		}
	}
	return buf;
}

Buffer* Set_Buffer(UA_Byte* data, uint32_t size){
	Buffer* buf = NULL;
	if(data != NULL && size > 0){
		buf = (Buffer*) malloc(sizeof(Buffer));
		if(buf != NULL){
			buf->size = size;
			buf->data = data;
		}
	}
	return buf;
}

void Delete_Buffer(Buffer* buffer){
	if(buffer != NULL){
		if(buffer->data != NULL){
			free(buffer->data);
		}
		free(buffer);
	}
}
