/*
 * secure_channel_client_connection.c
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#include <wrappers.h>

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
            scClientConnection->state = SC_Connection_Disconnected;
            scClientConnection->securityMode = Msg_Security_Mode_Invalid;
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

StatusCode On_Transport_Event_CB(void*            connection,
                                 void*            callbackData,
                                 ConnectionEvent  event,
                                 UA_Msg_Buffer*   msgBuffer,
                                 StatusCode       status){
    return STATUS_OK;
}

StatusCode Connect_Client_Channel(SC_Channel_Client_Connection* connection,
                                  char*                         uri,
                                  void*                         pkiConfig,
                                  UA_Byte_String*               clientCertificate,
                                  UA_Byte_String*               clientKey,
                                  UA_Byte_String*               serverCertificate,
                                  Msg_Security_Mode             securityMode,
                                  char*                         securityPolicy,
                                  uint32_t                      requestedLifetime,
                                  SC_Connection_Event_CB*       callback,
                                  void*                         callbackData
                                  )
{
    StatusCode status = STATUS_NOK;

    if(uri != NULL &&
       pkiConfig != NULL &&
       clientCertificate != NULL &&
       clientKey != NULL &&
       serverCertificate != NULL &&
       securityMode != Msg_Security_Mode_Invalid &&
       securityPolicy != NULL &&
       requestedLifetime > 0)
    {
        if(connection->clientCertificate == NULL &&
           connection->clientKey == NULL &&
           connection->serverCertificate == NULL &&
           connection->securityMode == Msg_Security_Mode_Invalid &&
           connection->securityPolicy == NULL &&
           connection->callback == NULL &&
           connection->callbackData == NULL)
        {
            // Create PKI provider
            connection->clientCertificate = Create_Byte_String_Copy(clientCertificate);
            connection->clientKey = Create_Private_Key(clientKey);
            connection->serverCertificate = Create_Byte_String_Copy(serverCertificate);
            connection->securityMode = securityMode;
            connection->securityPolicy = Create_String_From_CString(securityPolicy);
            connection->requestedLifetime = requestedLifetime;
            connection->callback = callback;
            connection->callbackData = callbackData;

            if(connection->clientCertificate == NULL ||
               connection->clientKey == NULL ||
               connection->serverCertificate == NULL ||
               connection->securityMode == Msg_Security_Mode_Invalid ||
               connection->securityPolicy == NULL)
            {
                status = STATUS_NOK;
            }else{
                connection->state = SC_Connection_Connecting_Transport;
                status = Connect_Transport(connection->instance->transportConnection,
                                           uri,
                                           On_Transport_Event_CB,
                                           NULL);

                if(status != STATUS_OK){
                    connection->state = SC_Connection_Disconnected;
                }
            }

        }else{
            status = STATUS_INVALID_STATE;
        }
    }else{
        status = STATUS_INVALID_PARAMETERS;
    }
    return status;
}
