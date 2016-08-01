/*
 * secure_channel_low_level.c
 *
 *  Created on: Jul 27, 2016
 *      Author: vincent
 */

#include <wrappers.h>

#include <stdlib.h>
#include <secure_channel_low_level.h>
#include <tcp_ua_connection.h>

SecureChannel_Connection* Create_Secure_Connection (){
    SecureChannel_Connection* sConnection = UA_NULL;
    TCP_UA_Connection* connection = Create_Connection();

    if(connection != UA_NULL){
        sConnection = (SecureChannel_Connection *) malloc(sizeof(SecureChannel_Connection));

        if(sConnection != 0){
            memset (sConnection, 0, sizeof(SecureChannel_Connection));
            sConnection->state = SC_Connection_Error;
            sConnection->transportConnection = connection;

        }else{
            Delete_Connection(connection);
        }
    }
    return sConnection;
}

void Delete_Secure_Connection (SecureChannel_Connection* scConnection){
    if(scConnection != UA_NULL){
        if(scConnection->transportConnection != UA_NULL){
            Delete_Connection(scConnection->transportConnection);
        }
        if(scConnection->currentNonce != UA_NULL){
            Delete_Private_Key(scConnection->currentNonce);
        }
        Delete_String(scConnection->currentSecuPolicy);
        Delete_Crypto_Provider(scConnection->currentCryptoProvider);
        Delete_Crypto_Provider(scConnection->precCryptoProvider);
        free(scConnection);
    }
}
