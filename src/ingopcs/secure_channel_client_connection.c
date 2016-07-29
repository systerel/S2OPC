/*
 * secure_channel_client_connection.c
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */
#include <stdlib.h>
#include <secure_channel_client_connection.h>

SC_Channel_Client_Connection* Create_Client_Channel (Namespace*      namespac,
                                                     EncodeableType* encodeableTypes){
    SC_Channel_Client_Connection* scClientConnection = NULL;
    SecureChannel_Connection* sConnection = Create_Secure_Connection();

    if(sConnection != NULL){
        scClientConnection = (SC_Channel_Client_Connection *) malloc (sizeof(SC_Channel_Client_Connection));

        if(scClientConnection != NULL){
            memset (scClientConnection, 0, sizeof(SC_Channel_Client_Connection));
            scClientConnection->instance = sConnection;
            scClientConnection->namespaces = namespac;
            scClientConnection->encodeableTypes = encodeableTypes;
        }
    }else{
        Delete_Secure_Connection(sConnection);
    }
    return scClientConnection;
}

void Delete_Client_Channel(SC_Channel_Client_Connection* scConnection){
	if(scConnection != NULL){
		if(scConnection->serverCertificate != NULL){
			Delete_Byte_String(scConnection->serverCertificate);
		}
		if(scConnection->clientCertificate != NULL){
			Delete_Byte_String(scConnection->clientCertificate);
		}
		if(scConnection->pendingRequests != NULL){
			free(scConnection->pendingRequests);
		}
		if(scConnection->securityPolicy != NULL){
			Delete_String(scConnection->securityPolicy);
		}
		if(scConnection->instance != NULL){
			Delete_Secure_Connection(scConnection->instance);
		}
		Delete_Timer(&scConnection->watchdogTimer);
	}
}
