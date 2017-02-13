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

#include "sopc_tcp_ua_listener.h"

#include <stdlib.h>
#include <string.h>

#include "sopc_tcp_ua_low_level.h"

typedef struct SOPC_Action_ListenerEventParameter {
    SOPC_StatusCode           prevStatus;
    TCP_UA_ListenerEvent_CB*  callback;
    struct SC_ServerEndpoint* callbackData;
    TCP_ListenerEvent         event;
    TCP_UA_Connection*        newTcpConnection;
} SOPC_Action_ListenerEventParameter;


void TCP_UA_Listener_EndSocketCreation_CB(void*           cbData,
                                          SOPC_StatusCode creationStatus,
                                          SOPC_Socket*    newSocket)
{
    TCP_UA_Listener* listener = (TCP_UA_Listener*) cbData;
    if(NULL != listener){
        if(STATUS_OK == creationStatus){
            listener->socket = newSocket;
            listener->callback(listener->callbackData,
                               TCP_ListenerEvent_Opened,
                               creationStatus,
                               NULL);
        }
    }
}

SOPC_Action_ListenerEventParameter* SOPC_CreateActionParameter_ListenerEvent(SOPC_StatusCode    prevStatus,
                                                                             TCP_UA_Listener*   listener,
                                                                             TCP_ListenerEvent  event,
                                                                             TCP_UA_Connection* newTcpConnection)
{
    SOPC_Action_ListenerEventParameter* param = NULL;
    if(listener != NULL){
        param = malloc(sizeof(SOPC_Action_ListenerEventParameter));
        if(param != NULL){
            param->prevStatus = prevStatus;
            param->callback = listener->callback;
            param->callbackData = listener->callbackData;
            param->event = event;
            param->newTcpConnection = newTcpConnection;
        }
    }
    return param;
}

SOPC_StatusCode OnSocketListenEvent_CB (SOPC_Socket* socket,
                                        uint32_t     socketEvent,
                                        void*        callbackData)
{
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    TCP_UA_Listener* listener = (TCP_UA_Listener*) callbackData;
    TCP_UA_Connection* newConnection = NULL;
    if(NULL != listener){
        switch(socketEvent){
            case SOCKET_ACCEPT_EVENT:
                newConnection = TCP_UA_Connection_Create(listener->protocolVersion,
                                                         1); // Server side connection == TRUE
                if(NULL != newConnection){
                    // Configure socket in new connection
                    status = TCP_UA_Connection_AcceptedSetSocket(newConnection,
                                                                 &listener->url,
                                                                 socket);
                    if(STATUS_OK == status && NULL != listener->callback){
                        status = listener->callback(listener->callbackData,
                                                    TCP_ListenerEvent_Connect,
                                                    status,
                                                    newConnection);
                    }
                }
                break;
            case SOCKET_CONNECT_EVENT:
            case SOCKET_CLOSE_EVENT:
            case SOCKET_EXCEPT_EVENT:
            case SOCKET_WRITE_EVENT:
            case SOCKET_READ_EVENT:
                if(NULL != listener->socket){
                    SOPC_Socket_Close(listener->socket);
                }
                if(NULL != listener->callback){
                   listener->callback(listener->callbackData,
                                      TCP_ListenerEvent_Closed,
                                      STATUS_OK,
                                      NULL);
                }
                break;
            default:
                break;
        }
    }
    return status;
}

TCP_UA_Listener* TCP_UA_Listener_Create(uint32_t scProtocolVersion){
    TCP_UA_Listener* result = malloc(sizeof(TCP_UA_Listener));
    if(NULL != result){
        memset(result, 0, sizeof(TCP_UA_Listener));
        result->protocolVersion = scProtocolVersion;
        result->state = TCP_Listener_Closed;
    }
    return result;
}

void TCP_UA_Listener_Delete(TCP_UA_Listener* listener){
    if(NULL != listener){
        free(listener);
    }
}

SOPC_StatusCode TCP_UA_Listener_Open(TCP_UA_Listener*             listener,
                                     const char*                  uri,
                                     TCP_UA_ListenerEvent_CB*     callback,
                                     struct SC_ServerEndpoint*    callbackData){
    SOPC_StatusCode status = STATUS_INVALID_PARAMETERS;
    if(listener != NULL &&
       uri != NULL &&
       callback != NULL){
        if(listener->url.Length <= 0 &&
           listener->callback == NULL &&
           listener->callbackData == NULL &&
           listener->state == TCP_Listener_Closed)
        {
            if(SOPC_Check_TCP_UA_URI(uri) == STATUS_OK){
                status = SOPC_String_InitializeFromCString(&listener->url, uri);
            }

            if(status == STATUS_OK){
                listener->callback = callback;
                listener->callbackData = callbackData;

                status = SOPC_CreateAction_SocketCreateServer(uri,
                                                              OnSocketListenEvent_CB,
                                                              (void*) listener,
                                                              TCP_UA_Listener_EndSocketCreation_CB,
                                                              (void*) listener);
            }
        }else{
            status = STATUS_INVALID_STATE;
        }
    }
    return status;
}

void TCP_UA_Listener_Close(TCP_UA_Listener* listener){
    if(NULL != listener){
        if(NULL != listener->socket){
            SOPC_Socket_Close(listener->socket);
        }
        if(NULL != listener->callback){
           listener->callback(listener->callbackData,
                              TCP_ListenerEvent_Closed,
                              STATUS_OK,
                              NULL);
        }
        SOPC_String_Clear(&listener->url);
        listener->callback = NULL;
        listener->callbackData = NULL;
    }
    return;
}
