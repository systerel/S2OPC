/*
 * secure_channel_low_level.c
 *
 *  Created on: Jul 27, 2016
 *      Author: vincent
 */
#include <stdlib.h>
#include <secure_channel_low_level.h>
#include <tcp_ua_connection.h>

SecureChannel_Connection* Create_Secure_Connection (){
    SecureChannel_Connection* sConnection = NULL;
    TCP_UA_Connection* connection = Create_Connection();

    if(connection != NULL){
        sConnection = (SecureChannel_Connection *) malloc(sizeof(SecureChannel_Connection));

        if(sConnection != 0){
            memset (sConnection, 0, sizeof(SecureChannel_Connection));
            sConnection->state = SC_Connection_Error;

        }else{
            // Delete connection
        }
    }
    return sConnection;
}

void Delete_Secure_Connection (SecureChannel_Connection* scConnection){
    if(scConnection != NULL){
        if(scConnection->transportConnection != NULL){
            Delete_Connection(scConnection->transportConnection);
        }
        if(scConnection->currentNonce != NULL){
            Delete_Private_Key(scConnection->currentNonce);
        }
        // clear string secu policy
        // clear crypto provider ? Who provide it ?
        // clear crypto provider PREC ? Who provide it ?
        free(scConnection);
    }
}
