/*
 * tcp_ua_connection.c
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#include <stdlib.h>
#include <stdbool.h>
#include <tcp_ua_connection.h>

TCP_UA_Connection* Create_Connection(){
    TCP_UA_Connection* connection = UA_NULL;
    StatusCode status = STATUS_NOK;
    connection = (TCP_UA_Connection *) malloc(sizeof(TCP_UA_Connection));

    if(connection != UA_NULL){
        memset (connection, 0, sizeof(TCP_UA_Connection));
        connection->state = TCP_Connection_Disconnected;
#if OPCUA_MULTITHREADED == OPCUA_CONFIG_NO
       status = Create_Socket_Manager(UA_NULL,
                                      1);
#else
       status = Create_Socket_Manager(&(connection->socketManager),
                                      1);
#endif //OPCUA_MULTITHREADED

        if(status != STATUS_OK){
            free(connection);
            connection = UA_NULL;
        }
    }

    return connection;
}

void Delete_Connection(TCP_UA_Connection* connection){
    if(connection != UA_NULL){
        if(connection->url != UA_NULL){
            free(connection->url);
        }
        Delete_Socket_Manager(&connection->socketManager);
        Close_Socket(connection->socket);
        Delete_Msg_Buffer(connection->inputMsgBuffer);
        Delete_Msg_Buffer(connection->outputMsgBuffer);
        Delete_Msg_Buffer(connection->sendingQueue);
        free(connection);
    }
}

StatusCode On_Socket_Event_CB (Socket        socket,
                               uint32_t      socketEvent,
                               void*         callbackData,
                               uint16_t      usPortNumber,
                               unsigned char bIsSSL){
    return STATUS_OK;
}

StatusCode Check_TCPUA_address (char* uri){
    StatusCode status = STATUS_NOK;
    int idx = 0;
    bool isPort = 0;
    bool hasPort = 0;
    bool invalid = 0;
    if(uri != UA_NULL){
        if(strlen(uri) > 10 && memcmp(uri, "opc.tcp://", 10) == 0){
            // search for a ':' defining port for given IP
            // search for a '/' defining endpoint name for given IP => at least 1 char after it (len - 1)
            for(idx = 10; idx < strlen(uri) - 1; idx++){
                if(isPort){
                    if(uri[idx] >= '0' && uri[idx] <= '9'){
                        // port definition
                        hasPort = 1;
                    }else if(uri[idx] == '/'){
                        // end of port definition + at least one character remaining
                        if(hasPort != 0 && invalid == 0){
                            status = STATUS_OK;
                        }
                    }else{
                        // unexpected character
                        invalid = 1;
                    }
                }else{
                    if(uri[idx] == ':'){
                        isPort = 1;
                    }
                }
            }
        }
    }
    return status;
}

StatusCode Connect_Transport (TCP_UA_Connection*          connection,
                              char*                       uri,
                              TCP_UA_Connection_Event_CB* callback,
                              void*                       callbackData){
    StatusCode status = STATUS_NOK;
    if(connection != UA_NULL &&
       uri != UA_NULL &&
       callback != UA_NULL){
        if(connection->url == UA_NULL &&
           connection->callback == UA_NULL &&
           connection->callbackData == UA_NULL &&
           connection->state == TCP_Connection_Disconnected)
        {
            // replace by correct uri check
            if(Check_TCPUA_address(uri) == STATUS_OK){
                connection->url = Create_String_From_CString(uri);
                connection->callback = callback;
                connection->callbackData = callbackData;

#if OPCUA_MULTITHREADED == OPCUA_CONFIG_NO
                status = Create_Client_Socket(UA_NULL,
                                              uri,
                                              On_Socket_Event_CB,
                                              UA_NULL,
                                              &(connection->socket));
#else

                if(status == STATUS_OK){
                    status = Create_Client_Socket(connection->socketManager,
                                                  uri,
                                                  On_Socket_Event_CB,
                                                  UA_NULL,
                                                  &(connection->socket));
                }
#endif //OPCUA_MULTITHREADED

            }else{
                status = STATUS_INVALID_PARAMETERS;
            }
        }else{
            status = STATUS_INVALID_STATE;
        }
    }else{
        status = STATUS_INVALID_PARAMETERS;
    }
    return status;
}
