/*
 * opcua_ingopcs_type.c
 *
 *  Created on: 29 juil. 2016
 *      Author: Vincent
 */

#include <stdlib.h>
#include <string.h>
#include <opcua_ingopcs_types.h>

UA_Byte_String* Create_Byte_String(){
	UA_Byte_String* bstring = NULL;
	bstring = (UA_Byte_String*) malloc(sizeof(UA_Byte_String));
	if(bstring != NULL){
		bstring->length = -1;
		bstring->characters = NULL;
	}
	return bstring;
}

UA_Byte_String* Create_Byte_String_Copy(UA_Byte_String* src){
	UA_Byte_String* dest = NULL;
	if(src != NULL){
		dest = Create_Byte_String();
		if(dest != NULL){
			if(src->length > 0){
				dest->length = src->length;
				dest->characters = (UA_Byte*) malloc (sizeof(UA_Byte)*dest->length);
				// No need of secure copy, both have same size here
				memcpy(dest->characters, src->characters, dest->length);
			} // else only -1 value is a correct value as created
		}
	}
	return dest;
}

void Delete_Byte_String(UA_Byte_String* bstring){
	if(bstring != NULL){
		if(bstring->characters != NULL){
			free(bstring->characters);
		}
		free(bstring);
	}
}

UA_String* Create_String(){
	return (UA_String*) Create_Byte_String();
}
UA_String* Create_String_Copy(UA_String* src){
	return (UA_String*) Create_Byte_String_Copy((UA_Byte_String*) src);
}
void Delete_String(UA_String* bstring){
	Delete_Byte_String((UA_Byte_String*) bstring);
}


