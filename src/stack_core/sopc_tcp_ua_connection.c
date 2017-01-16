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

TCP_UA_Connection* TCP_UA_Connection_Create(uint32_t scProtocolVersion,
                                            uint8_t  serverSideConnection)
{
    TCP_UA_Connection* connection = NULL;

    if(tcpProtocolVersion == scProtocolVersion){
        connection = (TCP_UA_Connection *) malloc(sizeof(TCP_UA_Connection));
    }

    if(connection != NULL){
        memset (connection, 0, sizeof(TCP_UA_Connection));
        SOPC_String_Initialize(&connection->url);
        connection->serverSideConnection = serverSideConnection;
        connection->state = TCP_UA_Connection_Disconnected;
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
        // No multithread implemented
        assert(FALSE);
#endif //OPCUA_MULTITHREADED

    }

    return connection;
}

void ResetConnectionState(TCP_UA_Connection* connection){
    connection->state = TCP_UA_Connection_Disconnected;
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
        if(connection->state != TCP_UA_Connection_Disconnected){
            TCP_UA_Connection_Disconnect(connection);
        }
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
    SOPC_StatusCode status = STATUS_OK;
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

// Receive hello message from Client and revise connection properties
SOPC_StatusCode ReceiveHelloMsg(TCP_UA_Connection* connection){
    // As indicated in part. 6, table 35
    const uint32_t TCP_UA_HEL_MSG_MAX_LENGTH = TCP_UA_HEL_MSG_LENGTH + TCP_UA_MAX_URL_LENGTH;
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    SOPC_String url;
    SOPC_String_Initialize(&url);
    uint32_t tempValue = 0;
    uint8_t modifiedReceiveBuffer = 0;
    if(connection != NULL
       && connection->inputMsgBuffer != NULL){
        if(connection->inputMsgBuffer->currentChunkSize > TCP_UA_HEL_MSG_LENGTH &&
           connection->inputMsgBuffer->currentChunkSize <= TCP_UA_HEL_MSG_MAX_LENGTH){
            // Read protocol version of server
            status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
            if(STATUS_OK == status){
                // Check protocol version compatible
                if(connection->protocolVersion < tempValue){
                    //TODO: change protocol version or fail
                }
            }

            // ReceiveBufferSize
            if(STATUS_OK == status){
                // Read received buffer size of CLIENT
                status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
                if(STATUS_OK == status){
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
            if(STATUS_OK == status){
                // Read sending buffer size of CLIENT
                status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
                if(STATUS_OK == status){
                    // Check size and adapt receive buffer size if needed
                    if(connection->receiveBufferSize > tempValue){
                        if(tempValue >= TCP_UA_MIN_BUFFER_SIZE){ // see mantis #3447 for >= instead of >
                            connection->receiveBufferSize = tempValue;
                            modifiedReceiveBuffer = 1;
                        }else{
                            status = STATUS_INVALID_RCV_PARAMETER;
                        }
                    }
                    // In other case do not change size since it should be configured with max size by default
                }
            }


            //MaxMessageSize of CLIENT
            if(STATUS_OK == status){
                status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
                if(STATUS_OK == status){
                    if(connection->maxMessageSizeSnd > tempValue){
                        connection->maxMessageSizeSnd = tempValue;
                    }
                }
            }

            //MaxChunkCount of CLIENT
            if(STATUS_OK == status){
                status = SOPC_UInt32_Read(&tempValue, connection->inputMsgBuffer);
                if(STATUS_OK == status){
                    if(connection->maxChunkCountSnd > tempValue){
                        connection->maxChunkCountSnd = tempValue;
                    }
                }
            }

            // EndpointURL
            if(STATUS_OK == status){
                status = SOPC_String_Read(&url, connection->inputMsgBuffer);
                if(STATUS_OK == status){
                    if(url.Length > TCP_UA_MAX_URL_LENGTH)
                    {
                        status = STATUS_INVALID_RCV_PARAMETER; // TcpEndpointUrlInvalid
                    }else{
                        // TODO: to be checked on secure channel connection that it corresponds
                        // to server certificate declared URL(s)
                        SOPC_String_Clear(&connection->url);
                        status = SOPC_String_Copy(&connection->url, &url);
                    }
                    SOPC_String_Clear(&url);
                }
            }

            // After end of decoding: modify receive buffer if needed
            if(modifiedReceiveBuffer != FALSE && status == STATUS_OK){
                // Adapt receive buffer size
                MsgBuffer_Delete(&connection->inputMsgBuffer);
                connection->inputMsgBuffer = NULL;
                InitReceiveBuffer(connection);
            }
        }else if(connection->inputMsgBuffer->currentChunkSize > TCP_UA_HEL_MSG_MAX_LENGTH){
            status = STATUS_INVALID_RCV_PARAMETER; // TcpEndpointUrlInvalid to return (part6 table 35)
        }else{
            status = STATUS_INVALID_RCV_PARAMETER;
        }
    }
    return status;
}

SOPC_StatusCode SendAckMsg(TCP_UA_Connection* connection){
    SOPC_StatusCode status = STATUS_OK;
    if(status == STATUS_OK){
        // encode message
        status = TCP_UA_EncodeHeader(connection->outputMsgBuffer,
                                     TCP_UA_Message_Acknowledge);
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
        status = TCP_UA_FinalizeHeader(connection->outputMsgBuffer);
    }

    if(status == STATUS_OK){
        status = TCP_UA_FlushMsgBuffer(connection->outputMsgBuffer);
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

SOPC_StatusCode OnSocketEvent_CB (SOPC_Socket* socket,
                                  uint32_t     socketEvent,
                                  void*        callbackData){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    TCP_UA_Connection* connection = (TCP_UA_Connection*) callbackData;
    TCP_UA_Connection_Event_CB* cb = NULL;
    void* cbData = NULL;
    if(socket != NULL && connection != NULL){
        switch(socketEvent){
            case SOCKET_ACCEPT_EVENT:
                status = STATUS_INVALID_STATE;
                if(connection->callback != NULL){
                    connection->callback(connection->callbackData,
                                         ConnectionEvent_Error,
                                         NULL,
                                         status);
                }
                TCP_UA_Connection_Disconnect(connection);
                break;
            case SOCKET_CLOSE_EVENT:
            case SOCKET_EXCEPT_EVENT:
                status = STATUS_OK;
                if(connection->state != TCP_UA_Connection_Disconnected){
                    cb = connection->callback;
                    cbData = connection->callbackData;
                    TCP_UA_Connection_Disconnect(connection);
                    if(cb != NULL){
                        cb(cbData,
                           ConnectionEvent_Disconnected,
                           NULL,
                           status);
                    }
                }
                break;
            case SOCKET_CONNECT_EVENT:
                // Manage connection
                assert(connection->socket == socket);
                status = SendHelloMsg(connection);
                if(status != STATUS_OK){
                    if(connection->callback != NULL){
                        connection->callback(connection->callbackData,
                                             ConnectionEvent_Error,
                                             NULL,
                                             status);
                    }
                    TCP_UA_Connection_Disconnect(connection);
                }
                break;
            case SOCKET_READ_EVENT:
                // Manage message reception
                status = TCP_UA_ReadData(socket, connection->inputMsgBuffer);
                if(status == STATUS_OK){
                    switch(connection->inputMsgBuffer->type){
                        case(TCP_UA_Message_Hello):
                            if(connection->serverSideConnection == FALSE){
                                status = STATUS_INVALID_RCV_PARAMETER;
                            }else{
                                status = ReceiveHelloMsg(connection);
                                if(STATUS_OK == status){
                                    status = SendAckMsg(connection);
                                }

                                if(STATUS_OK == status){
                                    connection->state = TCP_UA_Connection_Connected;
                                    if(connection->callback != NULL){
                                        connection->callback(connection->callbackData,
                                                             ConnectionEvent_Connected,
                                                             NULL,
                                                             status);
                                    }
                                }else{
                                    if(connection->callback != NULL){
                                        connection->callback(connection->callbackData,
                                                             ConnectionEvent_Error,
                                                             NULL,
                                                             status);
                                    }
                                    TCP_UA_Connection_Disconnect(connection);
                                }
                            }
                            break;
                        case(TCP_UA_Message_Acknowledge):
                            if(connection->serverSideConnection == FALSE){
                                status = ReceiveAckMsg(connection);
                                if(status == STATUS_OK){
                                    connection->state = TCP_UA_Connection_Connected;
                                    if(connection->callback != NULL){
                                        connection->callback(connection->callbackData,
                                                             ConnectionEvent_Connected,
                                                             NULL,
                                                             status);
                                    }
                                }else{
                                    if(connection->callback != NULL){
                                        connection->callback(connection->callbackData,
                                                             ConnectionEvent_Error,
                                                             NULL,
                                                             status);
                                    }
                                    TCP_UA_Connection_Disconnect(connection);
                                }
                            }else{
                                status = STATUS_INVALID_RCV_PARAMETER;
                            }
                            break;
                        case(TCP_UA_Message_Error):
                            status = ReceiveErrorMsg(connection);
                            if(connection->state != TCP_UA_Connection_Disconnected){
                                cb = connection->callback;
                                cbData = connection->callbackData;
                                TCP_UA_Connection_Disconnect(connection);
                                if(cb != NULL){
                                    cb(cbData,
                                       ConnectionEvent_Disconnected,
                                       NULL,
                                       status);
                                }
                            }
                            break;
                        case(TCP_UA_Message_SecureMessage):
                            if(connection->callback != NULL){
                                connection->callback(connection->callbackData,
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
                            // TODO: add trace with reason Invalid header ? => more precise errors
                            MsgBuffer_Reset(connection->inputMsgBuffer);
                    }
                }else if(OpcUa_BadDisconnect == status){
                    OnSocketEvent_CB(socket, SOCKET_CLOSE_EVENT, callbackData);
                }

                break;
            //case SOCKET_WRITE_EVENT:
            default:
                break;
        }
    }
    return status;
}

SOPC_StatusCode TCP_UA_Connection_Connect (TCP_UA_Connection*          connection,
                                           const char*                 uri,
                                           TCP_UA_Connection_Event_CB* callback,
                                           void*                       callbackData){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(connection != NULL &&
       uri != NULL &&
       callback != NULL)
    {
        if(connection->url.Length <= 0 &&
           connection->callback == NULL &&
           connection->callbackData == NULL &&
           connection->state == TCP_UA_Connection_Disconnected)
        {
            if(SOPC_Check_TCP_UA_URI(uri) == STATUS_OK){
                status = SOPC_String_InitializeFromCString(&connection->url, uri);
            }

            if(status == STATUS_OK){
                connection->state = TCP_UA_Connection_Connecting;
                connection->callback = callback;
                connection->callbackData = callbackData;
                // Initialize send & receive buffers necessary for sending & receiving first messages
                // Note: those buffers are resized after reception of the ACK message

#if OPCUA_MULTITHREADED == FALSE
                status = SOPC_SocketManager_CreateClientSocket(SOPC_SocketManager_GetGlobal(),
                                                               uri,
                                                               OnSocketEvent_CB,
                                                               (void*) connection,
                                                               &(connection->socket));
#else
                assert(FALSE);
#endif //OPCUA_MULTITHREADED
            }
            if(STATUS_OK == status){
                status = InitSendBuffer(connection);
            }
            if(STATUS_OK == status){
                status = InitReceiveBuffer(connection);
            }
        }else{
            status = STATUS_INVALID_STATE;
        }
    }
    return status;
}

SOPC_StatusCode TCP_UA_Connection_AcceptedSetSocket(TCP_UA_Connection* connection,
                                                    SOPC_String*       sURI,
                                                    SOPC_Socket*       connectionSocket)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(connection != NULL && sURI != NULL && connectionSocket != NULL){
        if(connection->url.Length <= 0 &&
           connection->callback == NULL &&
           connection->callbackData == NULL &&
           connection->state == TCP_UA_Connection_Disconnected)
        {
            connection->state = TCP_UA_Connection_Connecting;
            connection->socket = connectionSocket;

            // Attach the listener URI already verified before char -> String conversion
            status = SOPC_String_AttachFrom(&connection->url, sURI);

            if(STATUS_OK == status){
                status = SOPC_SocketManager_ConfigureAcceptedSocket(connectionSocket,
                                                                    OnSocketEvent_CB,
                                                                    (void*) connection);
            }

            if(STATUS_OK == status){
                status = InitSendBuffer(connection);
            }
            if(STATUS_OK == status){
                status = InitReceiveBuffer(connection);
            }
        }else{
            status = STATUS_INVALID_STATE;
        }
    }

    return status;
}

SOPC_StatusCode TCP_UA_Connection_AcceptedSetCallback(TCP_UA_Connection*          connection,
                                                      TCP_UA_Connection_Event_CB* callback,
                                                      void*                       callbackData){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(connection != NULL){
        if(connection->url.Length > 0 &&
           connection->callback == NULL &&
           connection->callbackData == NULL &&
           connection->state == TCP_UA_Connection_Connecting)
        {
            status = STATUS_OK;
            connection->callback = callback;
            connection->callbackData = callbackData;
        }else{
            status = STATUS_INVALID_STATE;
        }
    }
    return status;
}

void TCP_UA_Connection_Disconnect(TCP_UA_Connection* connection){
    if(connection != NULL){
        SOPC_Socket_Close(connection->socket);
        SOPC_String_Clear(&connection->url);
        connection->callback = NULL;
        connection->callbackData = NULL;
        ResetConnectionState(connection);
    }
}

uint32_t TCP_UA_Connection_GetReceiveProtocolVersion(TCP_UA_Connection* connection,
                                                     uint32_t*          protocolVersion){
    *protocolVersion = connection->receivedProtocolVersion;
    return 1;
}
