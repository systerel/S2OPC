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

SOPC_StatusCode OnSocketListenEvent_CB (SOPC_Socket* socket,
                                        uint32_t     socketEvent,
                                        void*        callbackData)
{
    SOPC_StatusCode status = STATUS_NOK;
    TCP_UA_Listener* listener = (TCP_UA_Listener*) callbackData;
    TCP_UA_Connection* newConnection = NULL;
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
            if(STATUS_OK != status){
                TCP_UA_Connection_Delete(newConnection);
            }
            break;
        case SOCKET_CONNECT_EVENT:
        case SOCKET_CLOSE_EVENT:
        case SOCKET_EXCEPT_EVENT:
        case SOCKET_WRITE_EVENT:
        case SOCKET_READ_EVENT:
            status = STATUS_INVALID_PARAMETERS;
            SOPC_Socket_Close(listener->socket);
            break;
        default:
            break;
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

#if OPCUA_MULTITHREADED == FALSE
                status = SOPC_SocketManager_CreateServerSocket(SOPC_SocketManager_GetGlobal(),
                                                               uri,
                                                               1, // listen all interfaces
                                                               OnSocketListenEvent_CB,
                                                               (void*) listener,
                                                               &(listener->socket));
#else
                assert(FALSE);
#endif //OPCUA_MULTITHREADED

                listener->callback(listener->callbackData,
                                   TCP_ListenerEvent_Opened,
                                   status,
                                   NULL);
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
        listener->callback = NULL;
        listener->callbackData = NULL;
    }
    return;
}
