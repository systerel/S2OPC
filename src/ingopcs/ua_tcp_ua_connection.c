/*
 * tcp_ua_connection.c
 *
 *  Created on: Jul 22, 2016
 *      Author: vincent
 */

#include "ua_tcp_ua_connection.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <ua_encoder.h>
#include "ua_tcp_ua_low_level.h"

StatusCode InitSendBuffer(TCP_UA_Connection* connection){
    StatusCode status = STATUS_NOK;
    if(connection->outputMsgBuffer == NULL){
        Buffer* buf = Buffer_Create(connection->sendBufferSize);
        if(buf != UA_NULL){
            connection->outputMsgBuffer = MsgBuffer_Create(buf,
                                                           connection->maxChunkCountSnd,
                                                           connection->socket,
                                                           UA_NULL, // no need for namespaces and types for decoding TCP UA headers
                                                           UA_NULL);
            if(connection->outputMsgBuffer != UA_NULL){
                status = STATUS_OK;
            }
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

StatusCode InitReceiveBuffer(TCP_UA_Connection* connection){
    StatusCode status = STATUS_NOK;
    if(connection->inputMsgBuffer == UA_NULL){
        Buffer* buf = Buffer_Create(connection->receiveBufferSize);
        if(buf != UA_NULL){
            connection->inputMsgBuffer = MsgBuffer_Create(buf,
                                                          connection->maxChunkCountRcv,
                                                          connection->socket,
                                                          UA_NULL, // no need for namespaces and types for decoding TCP UA headers
                                                          UA_NULL);
            if(connection->inputMsgBuffer != UA_NULL){
                status = STATUS_OK;
            }
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

TCP_UA_Connection* TCP_UA_Connection_Create(uint32_t scProtocolVersion){
    TCP_UA_Connection* connection = UA_NULL;
    StatusCode status = STATUS_NOK;

    if(tcpProtocolVersion == scProtocolVersion){
        connection = (TCP_UA_Connection *) malloc(sizeof(TCP_UA_Connection));
    }

    if(connection != UA_NULL){
        memset (connection, 0, sizeof(TCP_UA_Connection));
        connection->state = TCP_Connection_Disconnected;
        connection->protocolVersion = tcpProtocolVersion;
        // TODO: check constraints on connection properties (>8192 bytes ...)
        connection->sendBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
        connection->receiveBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
        connection->maxMessageSizeRcv = OPCUA_ENCODER_MAXMESSAGELENGTH;
        connection->maxMessageSizeSnd = OPCUA_ENCODER_MAXMESSAGELENGTH;
        connection->maxChunkCountRcv = 0;
        connection->maxChunkCountSnd = 0;
#if OPCUA_MULTITHREADED == OPCUA_CONFIG_NO
       status = SocketManager_Create(UA_NULL,
                                     1);
#else
       status = SocketManager_Create(&(connection->socketManager),
                                     1);
#endif //OPCUA_MULTITHREADED

        if(status != STATUS_OK){
            free(connection);
            connection = UA_NULL;
        }
    }

    return connection;
}

void ResetConnectionState(TCP_UA_Connection* connection){
    connection->state = TCP_Connection_Disconnected;
    connection->sendBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
    connection->receiveBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
    connection->maxMessageSizeRcv = OPCUA_ENCODER_MAXMESSAGELENGTH;
    connection->maxMessageSizeSnd = OPCUA_ENCODER_MAXMESSAGELENGTH;
    connection->maxChunkCountRcv = 0;
    connection->maxChunkCountSnd = 0;
    MsgBuffer_Delete(&connection->inputMsgBuffer);
    MsgBuffer_Delete(&connection->outputMsgBuffer);
    MsgBuffer_Delete(&connection->sendingQueue);
}

void TCP_UA_Connection_Delete(TCP_UA_Connection* connection){
    if(connection != UA_NULL){
        if(connection->url != UA_NULL){
            free(connection->url);
        }
        SocketManager_Delete(&connection->socketManager);
        Socket_Close(connection->socket);
        MsgBuffer_Delete(&connection->inputMsgBuffer);
        MsgBuffer_Delete(&connection->outputMsgBuffer);
        MsgBuffer_Delete(&connection->sendingQueue);
        free(connection);
    }
}

StatusCode SendHelloMsg(TCP_UA_Connection* connection){
    StatusCode status = STATUS_NOK;
    status = InitSendBuffer(connection);
    if(status == STATUS_OK){
        // encode message
        status = TCP_UA_EncodeHeader(connection->outputMsgBuffer,
                                     TCP_UA_Message_Hello);
    }
    if(status == STATUS_OK){
        status = UInt32_Write(connection->outputMsgBuffer,
                              &connection->protocolVersion);
    }
    if(status == STATUS_OK){
        status = UInt32_Write(connection->outputMsgBuffer,
                              &connection->receiveBufferSize);
    }
    if(status == STATUS_OK){
        status = UInt32_Write(connection->outputMsgBuffer,
                              &connection->sendBufferSize);
    }
    if(status == STATUS_OK){
        status = UInt32_Write(connection->outputMsgBuffer,
                              &connection->maxMessageSizeRcv);
    }
    if(status == STATUS_OK){
        status = UInt32_Write(connection->outputMsgBuffer,
                              &connection->maxChunkCountRcv);
    }
    if(status == STATUS_OK){
        status = String_Write(connection->outputMsgBuffer,
                                 connection->url);
    }
    if(status == STATUS_OK){
        status = TCP_UA_FinalizeHeader(connection->outputMsgBuffer);
    }
    if(status == STATUS_OK){
        status = TCP_UA_FlushMsgBuffer(connection->outputMsgBuffer);
    }
    // Check status and manage incorrect sending: close the connection or manage ?

    return status;
}

StatusCode ReceiveAckMsg(TCP_UA_Connection* connection){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t tempValue = 0;
    uint32_t modifiedReceiveBuffer = 0;
    if(connection != UA_NULL
       && connection->inputMsgBuffer != UA_NULL){
        if(connection->inputMsgBuffer->msgSize == TCP_UA_ACK_MSG_LENGTH){
            // Read protocol version of server
            status = UInt32_Read(connection->inputMsgBuffer, &tempValue);
            if(status == STATUS_OK){
                // Check protocol version compatible
                if(connection->protocolVersion > tempValue){
                    //TODO: change protocol version or fail
                }
            }

            // ReceiveBufferSize
            if(status == STATUS_OK){
                // Read received buffer size of SERVER
                status = UInt32_Read(connection->inputMsgBuffer, &tempValue);
                if(status == STATUS_OK){
                    // Adapt send buffer size if needed
                    if(connection->sendBufferSize > tempValue){
                        if(tempValue >= TCP_UA_MIN_BUFFER_SIZE){ // see mantis #3447 for >= instead of >
                            connection->sendBufferSize = tempValue;
                            // Adapt send buffer size
                            MsgBuffer_Delete(&connection->outputMsgBuffer);
                            connection->outputMsgBuffer = UA_NULL;
                            InitSendBuffer(connection);
                        }else{
                            status = STATUS_INVALID_RCV_PARAMETER;
                        }
                    }
                }
            }

            // SendBufferSize
            if(status == STATUS_OK){
                // Read sending buffer size of SERVER
                status = UInt32_Read(connection->inputMsgBuffer, &tempValue);
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
                status = UInt32_Read(connection->inputMsgBuffer, &tempValue);
                if(status == STATUS_OK){
                    if(connection->maxMessageSizeSnd > tempValue){
                        connection->maxMessageSizeSnd = tempValue;
                    }
                }
            }

            //MaxChunkCount of SERVER
            if(status == STATUS_OK){
                status = UInt32_Read(connection->inputMsgBuffer, &tempValue);
                if(status == STATUS_OK){
                    if(connection->maxChunkCountSnd > tempValue){
                        connection->maxChunkCountSnd = tempValue;
                    }
                }
            }

            // After end of decoding: modify receive buffer if needed
            if(modifiedReceiveBuffer != UA_FALSE && status == STATUS_OK){
                // Adapt receive buffer size
                MsgBuffer_Delete(&connection->inputMsgBuffer);
                connection->inputMsgBuffer = UA_NULL;
                InitReceiveBuffer(connection);
            }
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }
    return status;
}

StatusCode ReceiveErrorMsg(TCP_UA_Connection* connection){
    StatusCode status = STATUS_INVALID_PARAMETERS;
    StatusCode tmpStatus = STATUS_NOK;
    uint32_t error = 0;
    UA_String* reason = UA_NULL;
    if(connection != UA_NULL && connection->inputMsgBuffer != UA_NULL)
    {
        if(connection->inputMsgBuffer->msgSize >= TCP_UA_ERR_MIN_MSG_LENGTH)
        {
            // Read error cpde
            status = UInt32_Read(connection->inputMsgBuffer, &error);
            if(status == STATUS_OK){
                status = error;
                tmpStatus = String_Read(connection->inputMsgBuffer, reason);
                if(tmpStatus == STATUS_OK){
                    //TODO: log error reason !!!
                }
            }
        }

    }
    return status;
}

StatusCode OnSocketEvent_CB (Socket        socket,
                             uint32_t      socketEvent,
                             void*         callbackData,
                             uint16_t      usPortNumber,
                             unsigned char bIsSSL){
    (void) usPortNumber;
    (void) bIsSSL;
    StatusCode status = STATUS_NOK;
    TCP_UA_Connection* connection = (TCP_UA_Connection*) callbackData;
    switch(socketEvent){
        case SOCKET_ACCEPT_EVENT:
            status = STATUS_INVALID_STATE;
            if(connection->callback != UA_NULL){
                connection->callback(connection,
                                     connection->callbackData,
                                     ConnectionEvent_Error,
                                     UA_NULL,
                                     status);
            }
            break;
        case SOCKET_CLOSE_EVENT:
            status = STATUS_OK;
            if(connection->callback != UA_NULL){
                connection->callback(connection,
                                     connection->callbackData,
                                     ConnectionEvent_Disconnected,
                                     UA_NULL,
                                     status);
            }
            break;
        case SOCKET_CONNECT_EVENT:
            // Manage connection
            assert(connection->socket == socket);
            status = SendHelloMsg(connection);
            if(status == STATUS_OK){
                status = InitReceiveBuffer(connection);
            }else{
                if(connection->callback != UA_NULL){
                    connection->callback(connection,
                                         connection->callbackData,
                                         ConnectionEvent_Error,
                                         UA_NULL,
                                         status);
                }
            }
            break;
        case SOCKET_EXCEPT_EVENT:
            status = STATUS_INVALID_STATE;
            Socket_Close(connection->socket);
            break;
        case SOCKET_READ_EVENT:
            // Manage message reception
            status = TCP_UA_ReadData(socket, connection->inputMsgBuffer);
            if(status == STATUS_OK){
                switch(connection->inputMsgBuffer->type){
                    case(TCP_UA_Message_Hello):
                        status = STATUS_INVALID_RCV_PARAMETER;
                        break;
                    case(TCP_UA_Message_Acknowledge):
                        status = ReceiveAckMsg(connection);
                        if(status == STATUS_OK){
                            if(connection->callback != UA_NULL){
                                connection->callback(connection,
                                                     connection->callbackData,
                                                     ConnectionEvent_Connected,
                                                     UA_NULL,
                                                     status);
                            }
                        }else{
                            if(connection->callback != UA_NULL){
                                connection->callback(connection,
                                                     connection->callbackData,
                                                     ConnectionEvent_Error,
                                                     UA_NULL,
                                                     status);
                            }
                        }
                        break;
                    case(TCP_UA_Message_Error):
                        status = ReceiveErrorMsg(connection);
                        if(connection->callback != UA_NULL){
                            connection->callback(connection,
                                                 connection->callbackData,
                                                 ConnectionEvent_Disconnected,
                                                 UA_NULL,
                                                 status);
                        }
                        break;
                    case(TCP_UA_Message_SecureMessage):
                        if(connection->callback != UA_NULL){
                            connection->callback(connection,
                                                 connection->callbackData,
                                                 ConnectionEvent_Message,
                                                 connection->inputMsgBuffer,
                                                 status);
                        }
                        break;
                    case(TCP_UA_Message_Unknown):
                    case(TCP_UA_Message_Invalid):
                    default:
                        status = STATUS_INVALID_STATE;
                        break;
                }

                switch(status){
                    case(STATUS_OK):
                            break;
                    case(STATUS_OK_INCOMPLETE):
                            // Wait for next event
                            status = STATUS_OK;
                            break;
                    default:
                        // Erase content since incorrect reading
                        // TODO: add trace with reason Invalid header ? => more precise erorrs
                        MsgBuffer_Reset(connection->inputMsgBuffer);
                }
            }

            break;
        case SOCKET_SHUTDOWN_EVENT:
            ResetConnectionState(connection);
            break;
        case SOCKET_TIMEOUT_EVENT:
        case SOCKET_WRITE_EVENT:
        default:
            break;
    }
    return status;
}

StatusCode CheckURI (char* uri){
    StatusCode status = STATUS_NOK;
    size_t idx = 0;
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

StatusCode TCP_UA_Connection_Connect (TCP_UA_Connection*          connection,
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
            if(CheckURI(uri) == STATUS_OK){
                connection->url = String_CreateFromCString(uri);
                connection->callback = callback;
                connection->callbackData = callbackData;

#if OPCUA_MULTITHREADED == OPCUA_CONFIG_NO
                status = SocketManager_CreateClientSocket(UA_NULL,
                                              uri,
                                              OnSocketEvent_CB,
                                              (void*) connection,
                                              &(connection->socket));
#else

                if(status == STATUS_OK){
                    status = SocketManager_CreateClientSocket(connection->socketManager,
                                                  uri,
                                                  OnSocketEvent_CB,
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

void TCP_UA_Connection_Disconnect(TCP_UA_Connection* connection){
    Socket_Close(connection->socket);
    connection->socket = UA_NULL;
    String_Clear(connection->url);
    connection->url = UA_NULL;
    connection->callback = UA_NULL;
    connection->callbackData = UA_NULL;
    ResetConnectionState(connection);
}

uint32_t TCP_UA_Connection_GetReceiveProtocolVersion(TCP_UA_Connection* connection,
                                                     uint32_t*          protocolVersion){
    *protocolVersion = connection->receivedProtocolVersion;
    return 1;
}
