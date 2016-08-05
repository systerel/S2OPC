/*
 * tcp_ua_connection.c
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <tcp_ua_connection.h>
#include <tcp_ua_low_level.h>
#include <ua_encoder.h>

StatusCode Initiate_Send_Buffer(TCP_UA_Connection* connection){
    StatusCode status = STATUS_NOK;
    if(connection->outputMsgBuffer == NULL){
        Buffer* buf = Create_Buffer(connection->sendBufferSize);
        if(buf != UA_NULL){
            connection->outputMsgBuffer = Create_Msg_Buffer(connection->socket,
                                                            buf,
                                                            connection->maxChunkCountSnd);
            if(connection->outputMsgBuffer != UA_NULL){
                status = STATUS_OK;
            }
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

StatusCode Initiate_Receive_Buffer(TCP_UA_Connection* connection){
    StatusCode status = STATUS_NOK;
    if(connection->inputMsgBuffer == UA_NULL){
        Buffer* buf = Create_Buffer(connection->receiveBufferSize);
        if(buf != UA_NULL){
            connection->inputMsgBuffer = Create_Msg_Buffer(connection->socket,
                                                           buf,
                                                           connection->maxChunkCountRcv);
            if(connection->inputMsgBuffer != UA_NULL){
                status = STATUS_OK;
            }
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

TCP_UA_Connection* Create_Connection(){
    TCP_UA_Connection* connection = UA_NULL;
    StatusCode status = STATUS_NOK;
    connection = (TCP_UA_Connection *) malloc(sizeof(TCP_UA_Connection));

    if(connection != UA_NULL){
        memset (connection, 0, sizeof(TCP_UA_Connection));
        connection->state = TCP_Connection_Disconnected;
        connection->protocolVersion = 0;
        // TODO: check constraints on connection properties (>8192 bytes ...)
        connection->sendBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
        connection->receiveBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
        connection->maxMessageSizeRcv = OPCUA_ENCODER_MAXMESSAGELENGTH;
        connection->maxMessageSizeSnd = OPCUA_ENCODER_MAXMESSAGELENGTH;
        connection->maxChunkCountRcv = 0;
        connection->maxChunkCountSnd = 0;
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

void Reset_Connection_State(TCP_UA_Connection* connection){
    connection->state = TCP_Connection_Disconnected;
    connection->sendBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
    connection->receiveBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
    connection->maxMessageSizeRcv = OPCUA_ENCODER_MAXMESSAGELENGTH;
    connection->maxMessageSizeSnd = OPCUA_ENCODER_MAXMESSAGELENGTH;
    connection->maxChunkCountRcv = 0;
    connection->maxChunkCountSnd = 0;
    Delete_Msg_Buffer(connection->inputMsgBuffer);
    Delete_Msg_Buffer(connection->outputMsgBuffer);
    Delete_Msg_Buffer(connection->sendingQueue);
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
    StatusCode status = STATUS_NOK;
    TCP_UA_Connection* connection = (TCP_UA_Connection*) callbackData;
    switch(socketEvent){
        case SOCKET_ACCEPT_EVENT:
            break;
        case SOCKET_CLOSE_EVENT:
            Reset_Connection_State(connection);
            break;
        case SOCKET_CONNECT_EVENT:
            // Manage connection
            assert(connection->socket == socket);
            status = Send_Hello_Msg(connection);
            if(status == STATUS_OK){
                status = Initiate_Receive_Buffer(connection);
            }else{
                // Close socket if status != OK ?
            }
            break;
        case SOCKET_EXCEPT_EVENT:
        case SOCKET_READ_EVENT:
            // Manage message reception
            status = Read_TCP_UA_Data(socket, connection->inputMsgBuffer);
            if(status == STATUS_OK){
                switch(connection->inputMsgBuffer->type){
                    case(TCP_UA_Message_Hello):
                        status = STATUS_INVALID_RCV_PARAMETER;
                        break;
                    case(TCP_UA_Message_Acknowledge):
                        status = Receive_Ack_Msg(connection);
                        if(status == STATUS_OK){
                            connection->callback(connection,
                                                 connection->callbackData,
                                                 ConnectionEvent_Connected,
                                                 UA_NULL,
                                                 status);
                        }else{
                            connection->callback(connection,
                                                 connection->callbackData,
                                                 ConnectionEvent_Error,
                                                 UA_NULL,
                                                 status);
                        }
                        break;
                    case(TCP_UA_Message_Error):
                        // Socket will close: => do something ?
                        break;
                    case(TCP_UA_Message_SecureMessage):
                        // call sc CB
                        break;
                    case(TCP_UA_Message_Unknown):
                    case(TCP_UA_Message_Invalid):
                    default:
                        status = STATUS_INVALID_STATE;
                        break;
                }
            }

            if(status == STATUS_OK_INCOMPLETE || status == STATUS_OK){
                // Wait for next event
                status = STATUS_OK;
            }else{
                // Erase content since incorrect reading
                // TODO: add trace with reason
                Reset_Msg_Buffer(connection->inputMsgBuffer);
            }
            break;
        case SOCKET_SHUTDOWN_EVENT:
            Reset_Connection_State(connection);
            break;
        case SOCKET_TIMEOUT_EVENT:
        case SOCKET_WRITE_EVENT:
        default:
            break;
    }
    return status;
}

StatusCode Check_TCPUA_address (char* uri){
    StatusCode status = STATUS_NOK;
    int idx = 0;
    bool isPort = 0;
    bool hasPort = 0;
    bool invalid = 0;
    if(uri != UA_NULL){

        if(strlen(uri) + 4  > 4096){
            // Encoded value shall be less than 4096 bytes
            status = STATUS_INVALID_PARAMETERS;
        }else if(strlen(uri) > 10 && memcmp(uri, (const char*) "opc.tcp://", 10) == 0){
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
            if(Check_TCPUA_address(uri) == STATUS_OK){
                connection->url = Create_String_From_CString(uri);
                connection->callback = callback;
                connection->callbackData = callbackData;

#if OPCUA_MULTITHREADED == OPCUA_CONFIG_NO
                status = Create_Client_Socket(UA_NULL,
                                              uri,
                                              On_Socket_Event_CB,
                                              (void*) connection,
                                              &(connection->socket));
#else

                if(status == STATUS_OK){
                    status = Create_Client_Socket(connection->socketManager,
                                                  uri,
                                                  On_Socket_Event_CB,
                                                  (void*) connection,
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

StatusCode Send_Hello_Msg(TCP_UA_Connection* connection){
    StatusCode status = STATUS_NOK;
    status = Initiate_Send_Buffer(connection);
    if(status == STATUS_OK){
        // encode message
        status = Encode_TCP_UA_Header(connection->outputMsgBuffer,
                                      TCP_UA_Message_Hello);
    }
    if(status == STATUS_OK){
        status = Write_UInt32(connection->outputMsgBuffer, connection->protocolVersion);
    }
    if(status == STATUS_OK){
        status = Write_UInt32(connection->outputMsgBuffer, connection->receiveBufferSize);
    }
    if(status == STATUS_OK){
        status = Write_UInt32(connection->outputMsgBuffer, connection->sendBufferSize);
    }
    if(status == STATUS_OK){
        status = Write_UInt32(connection->outputMsgBuffer, connection->maxMessageSizeRcv);
    }
    if(status == STATUS_OK){
        status = Write_UInt32(connection->outputMsgBuffer, connection->maxChunkCountRcv);
    }
    if(status == STATUS_OK){
        status = Write_UA_String(connection->outputMsgBuffer, connection->url);
    }
    if(status == STATUS_OK){
        status = Finalize_TCP_UA_Header(connection->outputMsgBuffer);
    }
    if(status == STATUS_OK){
        status = Flush_Msg_Buffer(connection->outputMsgBuffer);
    }
    // Check status and manage incorrect sending: close the connection or manage ?

    return status;
}

StatusCode Receive_Ack_Msg(TCP_UA_Connection* connection){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t tempValue = 0;
    uint32_t modifiedReceiveBuffer = 0;
    if(connection != UA_NULL
       && connection->inputMsgBuffer != UA_NULL){
        if(connection->inputMsgBuffer->msgSize == TCP_UA_ACK_MSG_LENGTH){
            // Read protocol version of server
            status = Read_UInt32(connection->inputMsgBuffer, &tempValue);
            if(status == STATUS_OK){
                // Check protocol version compatible
                if(connection->protocolVersion > tempValue){
                    //TODO: change protocol version or fail
                }
            }

            // ReceiveBufferSize
            if(status == STATUS_OK){
                // Read received buffer size of SERVER
                status = Read_UInt32(connection->inputMsgBuffer, &tempValue);
                if(status == STATUS_OK){
                    // Adapt send buffer size if needed
                    if(connection->sendBufferSize > tempValue){
                        if(tempValue >= TCP_UA_MIN_BUFFER_SIZE){ // see mantis #3447 for >= instead of >
                            connection->sendBufferSize = tempValue;
                            // Adapt send buffer size
                            Delete_Msg_Buffer(connection->outputMsgBuffer);
                            connection->outputMsgBuffer = UA_NULL;
                            Initiate_Send_Buffer(connection);
                        }else{
                            status = STATUS_INVALID_RCV_PARAMETER;
                        }
                    }
                }
            }

            // SendBufferSize
            if(status == STATUS_OK){
                // Read sending buffer size of SERVER
                status = Read_UInt32(connection->inputMsgBuffer, &tempValue);
                if(status == STATUS_OK){
                    // Check size and adapt receive buffer size if needed
                    if(connection->receiveBufferSize > tempValue){
                        if(tempValue >= TCP_UA_MIN_BUFFER_SIZE){ // see mantis #3447 for >= instead of >
                            connection->receiveBufferSize = tempValue;
                            modifiedReceiveBuffer = 1;
                        }else{
                            status = STATUS_INVALID_RCV_PARAMETER;
                        }
                    }else if(connection->receiveBufferSize < tempValue){
                        // Cannot be greater than requested by client
                        status = STATUS_INVALID_RCV_PARAMETER;
                    }
                }
            }


            //MaxMessageSize of SERVER
            if(status == STATUS_OK){
                status = Read_UInt32(connection->inputMsgBuffer, &tempValue);
                if(status == STATUS_OK){
                    if(connection->maxMessageSizeSnd > tempValue){
                        connection->maxMessageSizeSnd = tempValue;
                    }
                }
            }

            //MaxChunkCount of SERVER
            if(status == STATUS_OK){
                status = Read_UInt32(connection->inputMsgBuffer, &tempValue);
                if(status == STATUS_OK){
                    if(connection->maxChunkCountSnd > tempValue){
                        connection->maxChunkCountSnd = tempValue;
                    }
                }
            }

            // After end of decoding: modify receive buffer if needed
            if(modifiedReceiveBuffer != UA_FALSE && status == STATUS_OK){
                // Adapt receive buffer size
                Delete_Msg_Buffer(connection->inputMsgBuffer);
                connection->inputMsgBuffer = UA_NULL;
                Initiate_Receive_Buffer(connection);
            }
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }
    return status;
}
