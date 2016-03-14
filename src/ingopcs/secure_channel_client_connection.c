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
        // deallocate sconnection
    }
    return scClientConnection;
}

//TODO: delete => delete fields when needed !!!
