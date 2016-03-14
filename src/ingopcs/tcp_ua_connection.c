/*
 * tcp_ua_connection.c
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#include <stdlib.h>
#include <tcp_ua_connection.h>

TCP_UA_Connection* Create_Connection(){
    TCP_UA_Connection* connection = NULL;

    connection = (TCP_UA_Connection *) malloc(sizeof(TCP_UA_Connection));

    if(connection != NULL){
        memset (connection, 0, sizeof(TCP_UA_Connection));
        connection->protocolVersion = 0;
        connection->state = TCP_Connection_Error;
    }

    return connection;
}

void Delete_Connection(TCP_UA_Connection* connection){
    if(connection != NULL){
        if(connection->url != NULL){
            free(connection->url);
            // Socket and socket manager close ?
        }
        free(connection);
    }
}
