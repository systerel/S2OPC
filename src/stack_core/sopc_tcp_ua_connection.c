/*
 *  Copyright (C) 2016 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sopc_tcp_ua_connection.h"

#include <stdlib.h>
#include <stdbool.h>

#include <assert.h>
#include <string.h>

#include "sopc_encoder.h"
#include "sopc_tcp_ua_low_level.h"

SOPC_StatusCode InitSendBuffer(TCP_UA_Connection* connection){
    SOPC_StatusCode status = STATUS_NOK;
    if(connection->outputMsgBuffer == NULL){
        Buffer* buf = Buffer_Create(connection->sendBufferSize);
        if(buf != NULL){
            connection->outputMsgBuffer = MsgBuffer_Create(buf,
                                                           connection->maxChunkCountSnd,
                                                           connection->socket,
                                                           NULL, // no need for namespaces and types for decoding TCP UA headers
                                                           NULL);
            if(connection->outputMsgBuffer != NULL){
                status = STATUS_OK;
            }
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

SOPC_StatusCode InitReceiveBuffer(TCP_UA_Connection* connection){
    SOPC_StatusCode status = STATUS_NOK;
    if(connection->inputMsgBuffer == NULL){
        Buffer* buf = Buffer_Create(connection->receiveBufferSize);
        if(buf != NULL){
            connection->inputMsgBuffer = MsgBuffer_Create(buf,
                                                          connection->maxChunkCountRcv,
                                                          connection->socket,
                                                          NULL, // no need for namespaces and types for decoding TCP UA headers
                                                          NULL);
            if(connection->inputMsgBuffer != NULL){
                status = STATUS_OK;
            }
        }
    }else{
        status = STATUS_INVALID_STATE;
    }
    return status;
}

TCP_UA_Connection* TCP_UA_Connection_Create(uint32_t scProtocolVersion){
    TCP_UA_Connection* connection = NULL;

    if(tcpProtocolVersion == scProtocolVersion){
        connection = (TCP_UA_Connection *) malloc(sizeof(TCP_UA_Connection));
    }

    if(connection != NULL){
        memset (connection, 0, sizeof(TCP_UA_Connection));
        SOPC_String_Initialize(&connection->url);
        connection->state = TCP_Connection_Disconnected;
        connection->protocolVersion = tcpProtocolVersion;
        // TODO: check constraints on connection properties (>8192 bytes ...)
        connection->sendBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
        connection->receiveBufferSize = OPCUA_TCPCONNECTION_DEFAULTCHUNKSIZE;
        connection->maxMessageSizeRcv = OPCUA_ENCODER_MAXMESSAGELENGTH;
        connection->maxMessageSizeSnd = OPCUA_ENCODER_MAXMESSAGELENGTH;
        connection->maxChunkCountRcv = 0;
        connection->maxChunkCountSnd = 0;
#if OPCUA_MULTITHREADED == FALSE
        // Will use the global socket manager
        connection->socketManager = NULL;
#else
        connection->socketManager = SOPC_SocketManager_Create(1);
#endif //OPCUA_MULTITHREADED

        if(connection->socketManager != NULL){
            free(connection);
            connection = NULL;
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
    if(connection != NULL){
        SOPC_String_Clear(&connection->url);
        SOPC_SocketManager_Delete(&connection->socketManager);
        SOPC_Socket_Close(connection->socket);
        MsgBuffer_Delete(&connection->inputMsgBuffer);
        MsgBuffer_Delete(&connection->outputMsgBuffer);
        MsgBuffer_Delete(&connection->sendingQueue);
        free(connection);
    }
}

SOPC_StatusCode SendHelloMsg(TCP_UA_Connection* connection){
    SOPC_StatusCode status = STATUS_NOK;
    status = InitSendBuffer(connection);
    if(status == STATUS_OK){
        // encode message
        status = TCP_UA_EncodeHeader(connection->outputMsgBuffer,
                                     TCP_UA_Message_Hello);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt32_Write(&connection->protocolVersion,
                              connection->outputMsgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt32_Write(&connection->receiveBufferSize,
                              connection->outputMsgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt32_Write(&connection->sendBufferSize,
                              connection->outputMsgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt32_Write(&connection->maxMessageSizeRcv,
                              connection->outputMsgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_UInt32_Write(&connection->maxChunkCountRcv,
                              connection->outputMsgBuffer);
    }
    if(status == STATUS_OK){
        status = SOPC_String_Write(&connection->url,
                              connection->outputMsgBuffer);
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

SOPC_StatusCode ReceiveAckMsg(TCP_UA_Connection* connection){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    uint32_t tempValue = 0;
    uint32_t modifiedReceiveBuffer = 0;
    if(connection != NULL
       && connection->inputMsgBuffer != NULL){
        if(connection->inputMsgBuffer->currentChunkSize == TCP_UA_ACK_MSG_LENGTH){
            // Read protocol version of server
            status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
            if(status == STATUS_OK){
                // Check protocol version compatible
                if(connection->protocolVersion > tempValue){
                    //TODO: change protocol version or fail
                }
            }

            // ReceiveBufferSize
            if(status == STATUS_OK){
                // Read received buffer size of SERVER
                status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
                if(status == STATUS_OK){
                    // Adapt send buffer size if needed
                    if(connection->sendBufferSize > tempValue){
                        if(tempValue >= TCP_UA_MIN_BUFFER_SIZE){ // see mantis #3447 for >= instead of >
                            connection->sendBufferSize = tempValue;
                            // Adapt send buffer size
                            MsgBuffer_Delete(&connection->outputMsgBuffer);
                            connection->outputMsgBuffer = NULL;
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
                status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
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
                status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
                if(status == STATUS_OK){
                    if(connection->maxMessageSizeSnd > tempValue){
                        connection->maxMessageSizeSnd = tempValue;
                    }
                }
            }

            //MaxChunkCount of SERVER
            if(status == STATUS_OK){
                status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
                if(status == STATUS_OK){
                    if(connection->maxChunkCountSnd > tempValue){
                        connection->maxChunkCountSnd = tempValue;
                    }
                }
            }

            // After end of decoding: modify receive buffer if needed
            if(modifiedReceiveBuffer != FALSE && status == STATUS_OK){
                // Adapt receive buffer size
                MsgBuffer_Delete(&connection->inputMsgBuffer);
                connection->inputMsgBuffer = NULL;
                InitReceiveBuffer(connection);
            }
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }
    return status;
}

SOPC_StatusCode ReceiveErrorMsg(TCP_UA_Connection* connection){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_StatusCode tmpStatus = STATUS_NOK;
    uint32_t error = 0;
    SOPC_String* reason = NULL;
    if(connection != NULL && connection->inputMsgBuffer != NULL)
    {
        if(connection->inputMsgBuffer->currentChunkSize >= TCP_UA_ERR_MIN_MSG_LENGTH)
        {
            // Read error cpde
            status = SOPC_UInt32_Read(&error, connection->inputMsgBuffer);
            if(status == STATUS_OK){
                status = error;
                tmpStatus = SOPC_String_Read(reason, connection->inputMsgBuffer);
                if(tmpStatus == STATUS_OK){
                    //TODO: log error reason !!!
                }
            }
        }

    }
    return status;
}

SOPC_StatusCode OnSocketEvent_CB (SOPC_Socket*    socket,
                             uint32_t      socketEvent,
                             void*         callbackData,
                             uint16_t      usPortNumber,
                             unsigned char bIsSSL){
    (void) usPortNumber;
    (void) bIsSSL;
    SOPC_StatusCode status = STATUS_NOK;
    TCP_UA_Connection* connection = (TCP_UA_Connection*) callbackData;
    switch(socketEvent){
        case SOCKET_ACCEPT_EVENT:
            status = STATUS_INVALID_STATE;
            if(connection->callback != NULL){
                connection->callback(connection,
                                     connection->callbackData,
                                     ConnectionEvent_Error,
                                     NULL,
                                     status);
            }
            break;
        case SOCKET_CLOSE_EVENT:
            status = STATUS_OK;
            if(connection->callback != NULL){
                connection->callback(connection,
                                     connection->callbackData,
                                     ConnectionEvent_Disconnected,
                                     NULL,
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
                if(connection->callback != NULL){
                    connection->callback(connection,
                                         connection->callbackData,
                                         ConnectionEvent_Error,
                                         NULL,
                                         status);
                }
            }
            break;
        case SOCKET_EXCEPT_EVENT:
            status = STATUS_INVALID_STATE;
            SOPC_Socket_Close(connection->socket);
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
                            if(connection->callback != NULL){
                                connection->callback(connection,
                                                     connection->callbackData,
                                                     ConnectionEvent_Connected,
                                                     NULL,
                                                     status);
                            }
                        }else{
                            if(connection->callback != NULL){
                                connection->callback(connection,
                                                     connection->callbackData,
                                                     ConnectionEvent_Error,
                                                     NULL,
                                                     status);
                            }
                        }
                        break;
                    case(TCP_UA_Message_Error):
                        status = ReceiveErrorMsg(connection);
                        if(connection->callback != NULL){
                            connection->callback(connection,
                                                 connection->callbackData,
                                                 ConnectionEvent_Disconnected,
                                                 NULL,
                                                 status);
                        }
                        break;
                    case(TCP_UA_Message_SecureMessage):
                        if(connection->callback != NULL){
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
        //case SOCKET_WRITE_EVENT:
        default:
            break;
    }
    return status;
}

SOPC_StatusCode CheckURI (const char* uri){
    SOPC_StatusCode status = STATUS_NOK;
    size_t idx = 0;
    uint8_t isPort = FALSE;
    uint8_t hasPort = FALSE;
    uint8_t hasName = FALSE;
    uint8_t invalid = FALSE;

    if(uri != NULL){

        if(strlen(uri) + 4  > 4096){
            // Encoded value shall be less than 4096 bytes
            status = STATUS_INVALID_PARAMETERS;
        }else if(strlen(uri) > 10 && memcmp(uri, (const char*) "opc.tcp://", 10) == 0){
            // search for a ':' defining port for given IP
            // search for a '/' defining endpoint name for given IP => at least 1 char after it (len - 1)
            for(idx = 10; idx < strlen(uri) - 1; idx++){
                if(isPort != FALSE){
                    if(uri[idx] >= '0' && uri[idx] <= '9'){
                        // port definition
                        hasPort = 1;
                    }else if(uri[idx] == '/'){
                        // Name of the endpoint after port, invalid otherwise
                        if(hasPort == 0){
                            invalid = 1;
                        }else{
                            hasName = 1;
                        }
                    }else{
                        if(hasPort == FALSE || hasName == FALSE){
                            // unexpected character since we do not expect a endpoint name
                            invalid = 1;
                        }
                    }
                }else{
                    if(uri[idx] == ':'){
                        isPort = 1;
                    }
                }
            }
            if(hasPort != FALSE && invalid == FALSE){
                status = STATUS_OK;
            }
        }
    }
    return status;
}

SOPC_StatusCode TCP_UA_Connection_Connect (TCP_UA_Connection*          connection,
                                           const char*                 uri,
                                           TCP_UA_Connection_Event_CB* callback,
                                           void*                       callbackData){
    SOPC_StatusCode status = STATUS_NOK;
    if(connection != NULL &&
       uri != NULL &&
       callback != NULL){
        if(connection->url.Length <= 0 &&
           connection->callback == NULL &&
           connection->callbackData == NULL &&
           connection->state == TCP_Connection_Disconnected)
        {
            if(CheckURI(uri) == STATUS_OK){
                status = SOPC_String_InitializeFromCString(&connection->url, uri);
            }

            if(status == STATUS_OK){
                connection->callback = callback;
                connection->callbackData = callbackData;

#if OPCUA_MULTITHREADED == FALSE
                status = SOPC_SocketManager_CreateClientSocket(SOPC_SocketManager_GetGlobal(),
                                                             uri,
                                                             OnSocketEvent_CB,
                                                             (void*) connection,
                                                             &(connection->socket));
#else

                if(status == STATUS_OK){
                    status = SOPC_SocketManager_CreateClientSocket(connection->socketManager,
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
    SOPC_Socket_Close(connection->socket);
    SOPC_String_Clear(&connection->url);
    connection->callback = NULL;
    connection->callbackData = NULL;
    ResetConnectionState(connection);
}

uint32_t TCP_UA_Connection_GetReceiveProtocolVersion(TCP_UA_Connection* connection,
                                                     uint32_t*          protocolVersion){
    *protocolVersion = connection->receivedProtocolVersion;
    return 1;
}
