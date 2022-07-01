/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "sopc_mqtt_transport_layer.h"
#include "sopc_common_constants.h"
#include "sopc_macros.h"
#ifndef USE_MQTT_PAHO
#define USE_MQTT_PAHO 0
#endif
#if USE_MQTT_PAHO == 1
#include "MQTTAsync.h"
#endif

/***** Types definition internally used by sopc_mqtt_transport_layer ******/

/*=== Definition of a message queue used by sopc mqtt manager scheduler ===*/

typedef enum E_MQTT_ID_FSM /* Destination of the decision table for an event */
{
    E_MQTT_ID_FSM_MANAGER,          /* Main decision table, used by the mqtt manager */
    E_MQTT_ID_FSM_TRANSPORT_CONTEXT /* Decision table used by a transport context linked to a mqtt connection */
} eIdFsm;

/* Callback used by an event to free an allocated data field. This callback is used by the mgr scheduler after event
 * treatment */

typedef void EventClearFct(void* ptr);

/* Event definition used by mqtt manager */

typedef struct T_MQTT_EVENT
{
    eIdFsm idFsm;                                /* Used to identify decision table to use */
    MqttTransportAsyncHandle idTransportContext; /* Transport context identifier */
    uint16_t event;                              /* Event identifier. Can be transport context or manager event */
    uint16_t size;                               /* Size of raw data pointed by eventd data */
    void* pEventData;                            /* Data linked to the event */
    EventClearFct* pClearCallback;               /* Callback used by manager scheduler to free pEventData */
} tMqttEvent;

/* Message queue used by manager event scheduler */

typedef struct T_MQTT_EVENT_CHANNEL
{
    Mutex lock;                                     /* Thread safe mechanism */
    Condition cond;                                 /* Signal of nbEvents > 0 */
    volatile uint16_t idxRdEvent;                   /* Read index */
    volatile uint16_t idxWrEvent;                   /* Write index */
    volatile uint16_t nbEvents;                     /* Nb of pendings events */
    tMqttEvent bufferEvents[MQTT_MAX_BUFFER_EVENT]; /*Circular event buffer*/
} tMqttEventChannel;

/*=== Workspace definition of a MQTT TRANSPORT CONTEXT ===*/

/* This structure is used as context by mqtt paho library callbacks */

typedef struct T_MQTT_TRANSPORT_CONTEXT_INTERNAL_IDENTIFICATION
{
    MqttManagerHandle* pMqttMgr;          /*MQTT Manager called by user to invoke this transport context*/
    MqttTransportAsyncHandle transportId; /*Identifier of the transport context. */
} tMqttTransportContextIdentification;

/* This enum defines transport context status, used by its final states machine */

typedef enum E_MQTT_TRANSPORT_CONTEXT_STATUS
{
    E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED,   /* Transport context not initialized */
    E_MQTT_TRANSPORT_CONTEXT_STATUS_RESERVED,          /* Handle of the transport context is reserved. */
    E_MQTT_TRANSPORT_CONTEXT_STATUS_DISCONNECTING,     /* Disconnection of transport context on going */
    E_MQTT_TRANSPORT_CONTEXT_STATUS_CONNECTING,        /* Transport context is connecting */
    E_MQTT_TRANSPORT_CONTEXT_STATUS_WAITING_FOR_RETRY, /* A connection has failed. Waiting 15 s before retry */
    E_MQTT_TRANSPORT_CONTEXT_STATUS_SUBSCRIBING, /* Connection established. Transport context MQTT subscribe on going */
    E_MQTT_TRANSPORT_CONTEXT_STATUS_READY,       /* Transport context is ready. Message can be sent. */
    E_MQTT_TRANSPORT_CONTEXT_STATUS_SENDING, /* Sending on going. Wait for broker ACK (immediately returned by lib if
                                                QOS = 0) */
    E_MQTT_TRANSPORT_CONTEXT_STATUS_MAX      /* Not a status. Used as dim array of decision table */
} eMqttTransportContextStatus;

/* This enum defines transport context events, used by its final states machine */

typedef enum E_MQTT_TRANSPORT_CONTEXT_EVENT
{
    E_MQTT_TRANSPORT_CONTEXT_EVENT_CONNECT,                            /* Connexion request*/
    E_MQTT_TRANSPORT_CONTEXT_EVENT_DISCONNECT,                         /* Disconnect request */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_SEND,                               /* Send message request */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_DISCONNECTED_FROM_BROKER,           /* Disconnected event */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_CONNECTED_TO_BROKER,                /* Connected event */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_BROKER_CONNECTION_ERR,              /* Connexion request failed */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_BROKER_CONNECTION_LOST,             /* Connexion with broker lost */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_SENT_TO_BROKER_SUCCESSFULL, /* Message sent ok */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_SENT_TO_BROKER_FAILED,      /* Message sent ko */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_SUBSCRIBED_TO_BROKER_SUCCESSFULL,   /* MQTT subscribe request ok*/
    E_MQTT_TRANSPORT_CONTEXT_EVENT_SUBSCRIBED_TO_BROKER_FAILED,        /* MQTT subscribe request ko*/
    E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_RECEIVED_FROM_BROKER,       /* MQTT message received */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_TICK,                               /* Tick event, each 100ms. */
    E_MQTT_TRANSPORT_CONTEXT_EVENT_MAX /* Not an event. Used as dim array of decision table */
} eMqttTransportContextEvent;

/* This structure defines the configuration, submitted by get handle request sent to a mqtt manager */

typedef struct T_MQTT_TRANSPORT_CONTEXT_CONNEXION_CONFIGURATION
{
    char uri[MQTT_LIB_MAX_SIZE_URI];              /* Uri of the broker */
    char topicname[MQTT_LIB_MAX_SIZE_TOPIC_NAME]; /* Mqtt topic name or OPCUA PUB SUB queuename*/

} tMqttTransportContextConnexionConfig;

/* This structure defines the callbacks configuration, submitted by get handle request sent to a mqtt manager */
/* The following callbacks are associated to a transport context, and invoked by mqtt manager scheduler*/

typedef struct T_MQTT_TRANSPORT_CONTEXT_CALLBACKS_CONFIG
{
    FctGetHandleResponse* pCbGetHandleSuccess; /* On get handle request success */
    FctGetHandleResponse* pCbGetHandleFailure; /* On get handle request failure */
    FctClientStatus* pCbClientReady;           /* On transport context ready (connection established and subscribed) */
    FctClientStatus* pCbClientNotReady;        /* On transport context not ready, after a conection lost by example. */
    FctMessageReceived* pCbMessageReceived;    /* On message reception on topic name set by hard coded configuration */
    FctReleaseHandleResponse* pCbReleaseHandleResponse;
} tMqttTransportContextCallbacksConfig;

/* Transport context internal workspace. One by connexion defined by MQTT_MAX_CLIENT. Statically allocated by set to NOT
 * INITIALIZED status */

typedef struct T_MQTT_TRANSPORT_CONTEXT
{
    volatile eMqttTransportContextStatus status; /* Current status of mqtt transport context */
    volatile uint32_t cptStatusTimeout;          /* Timeout of current status. */
    volatile uint16_t cptRetry;                  /* Retry counter used by retry connect after fail */
    void* clientHandle;                          /* Paho library broker connection instance */

    tMqttTransportContextIdentification identification;   /* Transport context : 1 manager, 1 transport async handle */
    tMqttTransportContextConnexionConfig connexionConfig; /* Connection configuration of this instance (topic, uri)*/
    tMqttTransportContextCallbacksConfig callbacksConfig; /* Callbacks linked to this instance */
    tMqttEventChannel pendingsMessagesToSend; /* Pendings messages to send. In case of overload sending operations
                                                 (before broker previous ACK) messages are queued*/

    void* pUserContext; /* User context, setted by get handle request. This context is free to be used via asynchrone
                           API. For synchrone, reserved for synchrone context.*/
} tMqttTransportContext;

/*=== Workspace definition of the MQTT MANAGER ===*/

/* This enum defines the status used by the mqtt manager final states machine */

typedef enum E_MQTT_MANAGER_STATUS
{
    E_MQTT_MANAGER_STATUS_NOT_INITIALIZED, /* Mqtt manager not initialized */
    E_MQTT_MANAGER_STATUS_RUNNING,         /* Mqtt manager is running. Requests can be submitted. */
    E_MQTT_MANAGER_STATUS_STOPPING,        /* Mqtt manager is quitting. Internally closing all transport contexts */
    E_MQTT_MANAGER_STATUS_STOPPED,         /* Mqtt manager stopped. Scheduler thread can be joined.*/
    E_MQTT_MANAGER_STATUS_MAX              /* Not a status. Used as dim array of decision table */
} eMqttManagerStatus;

/* This enum defines the events used by mqtt manager final states machine */

typedef enum E_MQTT_MANAGER_EVENT
{
    E_MQTT_MANAGER_EVENT_CMD_NEW_TRANSPORT_CONTEXT, /* Command creates new transport context.*/
    E_MQTT_MANAGER_EVENT_CMD_DEL_TRANSPORT_CONTEXT, /* Command destroy transport context. */
    E_MQTT_MANAGER_EVENT_CMD_SEND_MSG,              /* Send a message */
    E_MQTT_MANAGER_EVENT_CMD_QUIT_MGR, /* Destroy all transport context and terminate manager events scheduler */
    E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_RECV_MSG,  /* Event of reception of message on topic set by
                                                         configuration. */
    E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_READY,     /* Event of transport context ready to send and receive message.
                                                         Connection established and subscribed. */
    E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_NOT_READY, /* Event of transport context not ready, after a conn. lost by
                                                         example. */
    E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_REMOVED,   /* Transport context destroyed. */
    E_MQTT_MANAGER_EVENT_TICK,                        /* Periodic event, each 100ms. */
    E_MQTT_MANAGER_EVENT_MAX                          /* Not an event. Used as dim array of decision table */
} eMqttManagerEvent;

/* This structure defines the workspace of the mqtt manager. It is also used as "MqttManagerHandle". */

struct T_MQTT_MANAGER_HANDLE
{
    volatile eMqttManagerStatus status;    /* Status of MQTT Manager*/
    volatile uint16_t nbTransportContexts; /* Nb of transport context (connections)*/
    volatile uint32_t cptStatusTimeout;    /* Status timeout */
    volatile uint64_t random;
    Thread libScheduler;                                                     /* Handle thread event scheduler */
    tMqttEventChannel schedulerChannel;                                      /* Event queue */
    tMqttTransportContext slotsTransportContext[MQTT_MAX_TRANSPORT_CONTEXT]; /* Transport contexts, static */
    /* Transport contexts or workspaces are static. This is to avoid a eventual not controlled behavior of
     * asynchrone MQTT external library, by a memory context bounded*/
};

/*=== Definition a transport context handle. This type is used by synchrone api ===*/

/* This structure defines the workspace of the synchrone context linked to a tMqttTransportContext. It is also used as
 * "MqttTransportHandle". */

struct T_MQTT_TRANSPORT_HANDLE
{
    /* Synchronization objects */

    Mutex lock;                      /* Thread safe */
    Condition signalHandleOperation; /* Signal used by sync api get handle request */
    Condition signalStatusChange;    /* Signal used by sync api send message */
    Condition signalNewMessage;      /* Signal used by sync api to signal new message received*/

    /* Circular buffer used to register received message. Used only if no received callback defined by get handle
     * request.*/

    uint16_t nbMessageReceived;
    uint16_t iWr;
    uint16_t iRd;
    SOPC_Buffer* fifo[MQTT_MAX_BUFFER_RCV_MSG];

    /* -- or -- */

    FctMessageSyncReceived* pCbMessageReceived; /* Callback of message reception */

    /* Context of this sync context : associated mqtt manager and associated asynch transport handle */

    MqttManagerHandle* pMgrCtx;
    MqttTransportAsyncHandle transportId;

    void* pUserContext;

    bool bReady; /* Status : ready to send and receive or not ready */
};

/* This structure defines a get handle request */

typedef struct T_MQTT_GET_HANDLE_REQUEST
{
    tMqttTransportContextConnexionConfig connectionConf; /* Connexion configuration */
    tMqttTransportContextCallbacksConfig callbacksConf;  /* Callbacks configuration*/
    void* pUserContext;                                  /* User context */
} tMqttGetHandleRequest;

/*=== States machine callbacks declarations used by MQTT MANAGER states machine ===*/

/* Function pointer definition used by states machine defined by an array of callbacks,
 * This array is defined as this : 1 column represents an EVENT, and 1 row represents a STATUS */

typedef eMqttManagerStatus (*pFsmSchCallback)(MqttManagerHandle* pMqttMgr,    /* MQTT Manager handle */
                                              eMqttManagerStatus status,      /* Current status */
                                              eMqttManagerEvent event,        /* Last event */
                                              MqttTransportAsyncHandle cltId, /* MQTT Transport async handle */
                                              void* bufferData,               /* Data associated to the last event */
                                              uint16_t dataSize);             /* Size of data */

/* Get handle request callback */

static eMqttManagerStatus cbNewHdl(MqttManagerHandle* pWks,
                                   eMqttManagerStatus status,
                                   eMqttManagerEvent event,
                                   MqttTransportAsyncHandle cltId,
                                   void* bufferData,
                                   uint16_t dataSize);
/* Release handle request callback */

static eMqttManagerStatus cbRelHdl(MqttManagerHandle* pWks,
                                   eMqttManagerStatus status,
                                   eMqttManagerEvent event,
                                   MqttTransportAsyncHandle cltId,
                                   void* bufferData,
                                   uint16_t dataSize);
/* Send message request callback */

static eMqttManagerStatus cbSnd(MqttManagerHandle* pWks,
                                eMqttManagerStatus status,
                                eMqttManagerEvent event,
                                MqttTransportAsyncHandle cltId,
                                void* bufferData,
                                uint16_t dataSize);
/* Receive message callback */

static eMqttManagerStatus cbRcv(MqttManagerHandle* pWks,
                                eMqttManagerStatus status,
                                eMqttManagerEvent event,
                                MqttTransportAsyncHandle cltId,
                                void* bufferData,
                                uint16_t dataSize);

/* Transport context ready callback */

static eMqttManagerStatus cbCltRdy(MqttManagerHandle* pWks,
                                   eMqttManagerStatus status,
                                   eMqttManagerEvent event,
                                   MqttTransportAsyncHandle cltId,
                                   void* bufferData,
                                   uint16_t dataSize);

/* Transport context not ready callback */

static eMqttManagerStatus cbCltNotRdy(MqttManagerHandle* pWks,
                                      eMqttManagerStatus status,
                                      eMqttManagerEvent event,
                                      MqttTransportAsyncHandle cltId,
                                      void* bufferData,
                                      uint16_t dataSize);

/* Transport context destroyed callback */

static eMqttManagerStatus cbCltRmv(MqttManagerHandle* pWks,
                                   eMqttManagerStatus status,
                                   eMqttManagerEvent event,
                                   MqttTransportAsyncHandle cltId,
                                   void* bufferData,
                                   uint16_t dataSize);
/* Quit MQTT manager request callback */

static eMqttManagerStatus cbQuit(MqttManagerHandle* pWks,
                                 eMqttManagerStatus status,
                                 eMqttManagerEvent event,
                                 MqttTransportAsyncHandle cltId,
                                 void* bufferData,
                                 uint16_t dataSize);

/* Periodic tick event callback */

static eMqttManagerStatus cbTmo(MqttManagerHandle* pWks,
                                eMqttManagerStatus status,
                                eMqttManagerEvent event,
                                MqttTransportAsyncHandle cltId,
                                void* bufferData,
                                uint16_t dataSize);

/*=== States machine used by MQTT MANAGER definition ===*/

static pFsmSchCallback tabFsmScheduler[E_MQTT_MANAGER_STATUS_MAX][E_MQTT_MANAGER_EVENT_MAX]=                                                                                //
{       /*                              NEW_HANDLE        DEL_HDL          SEND         QUIT          CLT_RECV       CLT_RDY      CLT_NRDY        CLT_RMV          TMO*/    //
        /*    NOT_INITIALIZED,    */    {NULL,            NULL,            NULL,        NULL,         NULL,          NULL,        NULL,           NULL,            NULL},   //
        /*    RUNNING,            */    {cbNewHdl,        cbRelHdl,        cbSnd,       cbQuit,       cbRcv,         cbCltRdy,    cbCltNotRdy,    cbCltRmv,        cbTmo},  //
        /*    STOPPING            */    {NULL,            NULL,            NULL,        NULL,         NULL,          cbCltRdy,    cbCltNotRdy,    cbCltRmv,        cbTmo},   //
        /*    STOPPED             */    {NULL,            NULL,            NULL,        NULL,         NULL,          NULL,        NULL,           NULL,            NULL}    //
};

/*=== States machine callbacks declarations used by MQTT TRANSPORT CONTEXT states machine ===*/

/* Function pointer definition used by states machine defined by an array of callbacks,
 * This array is defined as this : 1 column represents an EVENT, and 1 row represents a STATUS */

typedef eMqttTransportContextStatus (*pFsmCallback)(tMqttTransportContext* pMqttMgr,    /* Transport context */
                                                    eMqttTransportContextStatus status, /* Current status */
                                                    eMqttTransportContextEvent event,   /* Current event */
                                                    void* bufferData,                   /* Data linked to event */
                                                    uint16_t dataSize);                 /* Data size */

/* Connection request callback */

static eMqttTransportContextStatus cbConnReq(tMqttTransportContext* pCtx,
                                             eMqttTransportContextStatus status,
                                             eMqttTransportContextEvent event,
                                             void* bufferData,
                                             uint16_t dataSize);

/* Disconnection request callback */

static eMqttTransportContextStatus cbDiscReq(tMqttTransportContext* pCtx,
                                             eMqttTransportContextStatus status,
                                             eMqttTransportContextEvent event,
                                             void* bufferData,
                                             uint16_t dataSize);

/* Connection clear callback */

static eMqttTransportContextStatus cbConnClr(tMqttTransportContext* pCtx,
                                             eMqttTransportContextStatus status,
                                             eMqttTransportContextEvent event,
                                             void* bufferData,
                                             uint16_t dataSize);

/* Connection lost and retry callback */

static eMqttTransportContextStatus cbConnTry(tMqttTransportContext* pCtx,
                                             eMqttTransportContextStatus status,
                                             eMqttTransportContextEvent event,
                                             void* bufferData,
                                             uint16_t dataSize);

/* Subscribe request callback */

static eMqttTransportContextStatus cbSubReq(tMqttTransportContext* pCtx,
                                            eMqttTransportContextStatus status,
                                            eMqttTransportContextEvent event,
                                            void* bufferData,
                                            uint16_t dataSize);

/* Subscribe successful callback */

static eMqttTransportContextStatus cbSubRdy(tMqttTransportContext* pCtx,
                                            eMqttTransportContextStatus status,
                                            eMqttTransportContextEvent event,
                                            void* bufferData,
                                            uint16_t dataSize);

/* Message received callback */

static eMqttTransportContextStatus cbMsgRcv(tMqttTransportContext* pCtx,
                                            eMqttTransportContextStatus status,
                                            eMqttTransportContextEvent event,
                                            void* bufferData,
                                            uint16_t dataSize);

/* Overrun callback */

static eMqttTransportContextStatus cbPubBck(tMqttTransportContext* pCtx,
                                            eMqttTransportContextStatus status,
                                            eMqttTransportContextEvent event,
                                            void* bufferData,
                                            uint16_t dataSize);

/* Send request callback */

static eMqttTransportContextStatus cbPubReq(tMqttTransportContext* pCtx,
                                            eMqttTransportContextStatus status,
                                            eMqttTransportContextEvent event,
                                            void* bufferData,
                                            uint16_t dataSize);

/* Tick callback */

static eMqttTransportContextStatus cbTick(tMqttTransportContext* pCtx,
                                          eMqttTransportContextStatus status,
                                          eMqttTransportContextEvent event,
                                          void* bufferData,
                                          uint16_t dataSize);

/*=== States machine used by MQTT TRANSPORT definition ===*/

/***** State machine documentation *****/

static pFsmCallback tabFsmMqttClient[E_MQTT_TRANSPORT_CONTEXT_STATUS_MAX][E_MQTT_TRANSPORT_CONTEXT_EVENT_MAX]=
{
        /*  Not init <------| DISCONNECTED |-----Disconnecting <---------------------------------------------------------.                                                                  */ //
        /*      |                                        ^                                                               |                                                                  */ //
        /*      |<<< Manager set this                    '---- | DISCONNECT| ---------.---.------------------------------|---------.----------------------------------.  |SENT_OK |---.     */ //
        /*      |      status on get handle                                           |   |                              |         |  | MSG_RCV |                     |  |SEND    |---.     */ //
        /*      v                                                                     |   |                              |         |    ||                            |     |         |     */ //
        /*  Reserved -----| CONNECT |-----> Connecting                                |   |  .--------------| SUBERROR|--'         |    v|                            |     v         |     */ //
        /*                                      ^     '------- | CONNECTED | ------>  |  Subscribing  ------| SUBSUCCS|----------> Ready----- | SEND      | --------> Sending --------'     */ //
        /*                                      |     '------- | CONN_ERR  | ------> Wait for retry   ------| TIMEOUT |--.           ^------- | SENT_OK   | -------------' |^               */ //
        /*                                      |     '------- | CONN_LOST | --.                                         |           '------- | SENT_KO   | -------------' ||               */ //
        /*                                      '------------------------------'-----------------------------------------'                                                 ||               */ //
        /* */
        /*                                  CONNECT          DISCONNECT         SEND            DISCONNECTED        CONNECTED    CONN_ERR         CONN_LOST        SENT_SUCCESS     SENT_ERROR       SUB_SUCCESS      SUB_ERROR         MESS_RECEIVED */             //
        /*    NOT_INITIALIZED,          */ {NULL,            NULL,              NULL,           NULL,               NULL,        NULL,            NULL,            NULL,            NULL,            NULL,            NULL,             NULL,            cbTick},    //
        /*    RESERVED,                 */ {cbConnReq,       NULL,              NULL,           NULL,               NULL,        NULL,            NULL,            NULL,            NULL,            NULL,            NULL,             NULL,            cbTick},    //
        /*    DISCONNECTING             */ {NULL,            NULL,              NULL,           cbConnClr,          NULL,        NULL,            NULL,            NULL,            NULL,            NULL,            NULL,             NULL,            cbTick},    //
        /*    CONNECTING                */ {NULL,            cbDiscReq,         NULL,           NULL,               cbSubReq,    cbConnTry,       cbConnReq,       NULL,            NULL,            NULL,            NULL,             NULL,            cbTick},    //
        /*    WAITING_FOR_RETRY         */ {cbConnReq,       cbDiscReq,         NULL,           NULL,               NULL,        NULL,            NULL,            NULL,            NULL,            NULL,            NULL,             NULL,            cbTick},    //
        /*    SUBSCRIBING,              */ {NULL,            cbDiscReq,         NULL,           NULL,               NULL,        NULL,            cbConnReq,       NULL,            NULL,            cbSubRdy,        cbDiscReq,        NULL,            cbTick},    //
        /*    READY,                    */ {NULL,            cbDiscReq,         cbPubReq,       NULL,               NULL,        NULL,            cbConnReq,       NULL,            NULL,            NULL,            NULL,             cbMsgRcv,        cbTick},    //
        /*    SENDING,                  */ {NULL,            cbDiscReq,         cbPubBck,       NULL,               NULL,        NULL,            cbConnReq,       cbPubReq,        cbPubReq,        NULL,            NULL,             cbMsgRcv,        cbTick}     //
};

/*=== Declarations of static functions used by debug SOPC_CONSOLE_PRINTF ===*/

#if DEBUG_SCHEDULER == 1
static const char* get_event_string(eMqttTransportContextEvent event);
static const char* get_status_string(eMqttTransportContextStatus status);
static const char* get_mgr_event_string(eMqttManagerEvent event);
static const char* get_mgr_status_string(eMqttManagerStatus status);
#endif

/*=== Declarations of static functions used to initialize, write, read and clear an event queue, used by the scheduler
 * of the mqtt manager ===*/

static SOPC_ReturnStatus EVENT_CHANNEL_init(tMqttEventChannel* pChannel);
static SOPC_ReturnStatus EVENT_CHANNEL_deinit(tMqttEventChannel* pChannel);
static SOPC_ReturnStatus EVENT_CHANNEL_flush(tMqttEventChannel* pChannel);
static SOPC_ReturnStatus EVENT_CHANNEL_push(tMqttEventChannel* pChannel, tMqttEvent* pEvent);
static SOPC_ReturnStatus EVENT_CHANNEL_pop(tMqttEventChannel* pChannel, tMqttEvent* pEvent, uint32_t timeout);

/*=== Declarations of static functions used to send an event to a transport context state machine ===*/

SOPC_ReturnStatus CLT_EvtConnReq(MqttManagerHandle* pWks,
                                 MqttTransportAsyncHandle idx,
                                 tMqttTransportContextConnexionConfig* pConfig);

SOPC_ReturnStatus CLT_EvtSendMsgReq(MqttManagerHandle* pWks,
                                    MqttTransportAsyncHandle idx,
                                    uint8_t* bufferData,
                                    uint16_t size);

SOPC_ReturnStatus CLT_EvtDiscReq(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
SOPC_ReturnStatus CLT_EvtConnLost(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
SOPC_ReturnStatus CLT_EvtDisc(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
SOPC_ReturnStatus CLT_EvtConnError(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
SOPC_ReturnStatus CLT_EvtConnSuccess(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
SOPC_ReturnStatus CLT_EvtSubFailed(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
SOPC_ReturnStatus CLT_EvtSubSuccess(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
SOPC_ReturnStatus CLT_EvtMsgSent(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
SOPC_ReturnStatus CLT_EvtMsgSentError(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
SOPC_ReturnStatus CLT_EvtMsgRcv(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx, char* msgData, uint16_t msgLen);

/*=== Declarations of static functions used to send an event to the mqtt manager state machine ===*/

static SOPC_ReturnStatus MGR_EvtCltRemoved(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
static SOPC_ReturnStatus MGR_EvtCltRdy(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
static SOPC_ReturnStatus MGR_EvtCltNotReady(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
static SOPC_ReturnStatus MGR_EvtMsgRcv(MqttManagerHandle* pWks,
                                       MqttTransportAsyncHandle idx,
                                       uint8_t* bufferData,
                                       uint16_t size);
static SOPC_ReturnStatus MGR_EvtQuitScheduler(MqttManagerHandle* pWks);
static SOPC_ReturnStatus MGR_EvtReleaseHdl(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx);
static SOPC_ReturnStatus MGR_EvtSendMsg(MqttManagerHandle* pWks,
                                        MqttTransportAsyncHandle idx,
                                        uint8_t* bufferData,
                                        uint16_t size);

/*=== Declarations of static functions used to add or get pending messages to send to a transport context ===*/

static void TransportContext_AddPendingMsg(tMqttTransportContext* pClt, char* msgData, uint16_t msgLen);
static void TransportContext_FlushPendingMsg(tMqttTransportContext* pClt);
static void TransportContext_GetPendingMsg(tMqttTransportContext* pClt, char** msgData, uint16_t* msgLen);

/*=== Declarations of callbacks used by PAHO MQTT library ===*/

#if USE_MQTT_PAHO == 1
static void cb_lib_onConnect(void* context, MQTTAsync_successData* response);
static void cb_lib_onConnectFailure(void* context, MQTTAsync_failureData* response);
static void cb_lib_onConnLost(void* context, char* cause);
static int cb_lib_onMsgRcv(void* context, char* topicName, int topicLen, MQTTAsync_message* message);
static void cb_lib_onDisconnect(void* context, MQTTAsync_successData* response);
static void cb_lib_onSubscribe(void* context, MQTTAsync_successData* response);
static void cb_lib_onSubscribeFailure(void* context, MQTTAsync_failureData* response);
static void cb_lib_onSend(void* context, MQTTAsync_successData* response);
static void cb_lib_onSendFailure(void* context, MQTTAsync_failureData* response);
#endif

/*=== Declarations of wrapper of PAHO MQTT library ===*/

static int api_lib_create(tMqttTransportContext* pCtx);
static void api_lib_destroy(tMqttTransportContext* pCtx);
static int api_lib_connect(tMqttTransportContext* pCtx);
static int api_lib_disconnect(tMqttTransportContext* pCtx);
static int api_lib_subscribe(tMqttTransportContext* pCtx);
static int api_lib_send_msg(tMqttTransportContext* pCtx, uint8_t* data, uint16_t dataSize);

/*=== Declarations of callbacks of the thread used by Mqtt manager scheduler ===*/

static void* cbTask_MqttManagerScheduler(void* pArg);

/*=== States machine callbacks definitions used by MQTT MANAGER states machine ===*/

/* Get new transport context callback, invoked when NEW_TRANSPORT_CONTEXT event is received */
/* Output status : RUNNING */
static eMqttManagerStatus cbNewHdl(MqttManagerHandle* pWks,
                                   eMqttManagerStatus status,      /* Shall be RUNNING status */
                                   eMqttManagerEvent event,        /* Shall be NEW_TRANSPORT_CONTEXT event */
                                   MqttTransportAsyncHandle cltId, /* Not used */
                                   void* bufferData,               /* Shall be tMqttGetHandleRequest */
                                   uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(cltId);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbNewHdl entry : status = %s - event = %s\n", get_mgr_status_string(status),
                            get_mgr_event_string(event));
#endif
        tMqttGetHandleRequest* pGetHandleRequest = (tMqttGetHandleRequest*) bufferData;

        /* Retrieve new handle request data */
        if (NULL != pGetHandleRequest)
        {
            /* Search an empty slot. A slot is empty if transport context status is set to NOT_INITIALIZED*/
            bool bFound = false;
            uint16_t idxClient = 0;
            for (uint16_t i = 0; i < MQTT_MAX_TRANSPORT_CONTEXT && false == bFound; i++)
            {
                if (E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED == pWks->slotsTransportContext[i].status)
                {
                    /* Mark as reserved, copy callbacks configuration and user context*/
                    pWks->slotsTransportContext[i].status = E_MQTT_TRANSPORT_CONTEXT_STATUS_RESERVED;

                    memcpy(&pWks->slotsTransportContext[i].callbacksConfig, &pGetHandleRequest->callbacksConf,
                           sizeof(tMqttTransportContextCallbacksConfig));
                    pWks->slotsTransportContext[i].pUserContext = pGetHandleRequest->pUserContext;

                    /* Store transport id and flag found set*/
                    idxClient = i;
                    bFound = true;

                    /* This treatment is normally useless, because already init at manager creation*/
                    pWks->slotsTransportContext[i].identification.pMqttMgr = pWks;
                    pWks->slotsTransportContext[i].identification.transportId = i;
                }
            }

            /* Slot found */
            if (true == bFound)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF("\n cbNewHdl entry : slot found = %d\n", idxClient);
#endif
                //***************CALLBACK GET HANDLE SUCCESS ***********************

                if (NULL != pGetHandleRequest->callbacksConf.pCbGetHandleSuccess)
                {
                    (*pGetHandleRequest->callbacksConf.pCbGetHandleSuccess)(
                        idxClient,                        /* Transport context async handle */
                        pGetHandleRequest->pUserContext); /* User context */
                }

                /* Increment number of transport context*/
                pWks->nbTransportContexts++;

                /* Send to state machine of transport context the connection configuration*/
                CLT_EvtConnReq(pWks, idxClient, &pGetHandleRequest->connectionConf);
            }
            /* Slot not found, get handle failure is invoked with invalid transport async handle*/
            else
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF("\n cbNewHdl entry : no slot found\n");
#endif
                //***************CALLBACK GET HANDLE ERROR ***********************
                if (NULL != pGetHandleRequest->callbacksConf.pCbGetHandleFailure)
                {
                    (*pGetHandleRequest->callbacksConf.pCbGetHandleFailure)(MQTT_INVALID_TRANSPORT_ASYNC_HANDLE,
                                                                            pGetHandleRequest->pUserContext);
                }
            }
        }
        return pWks->status;
    }
    return E_MQTT_MANAGER_STATUS_NOT_INITIALIZED;
}

/* Release transport context handle, invoked when event DEL_TRANSPORT_CONTEXT is received */
/* Output status : RUNNING */
static eMqttManagerStatus cbRelHdl(MqttManagerHandle* pWks,
                                   eMqttManagerStatus status,      /* Shall be RUNNING status */
                                   eMqttManagerEvent event,        /* Shall be DEL_TRANSPORT_CONTEXT event */
                                   MqttTransportAsyncHandle cltId, /* Transport context async handle to destroy */
                                   void* bufferData,               /* Not used */
                                   uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbRelHdl entry : status = %s - event = %s\n", get_mgr_status_string(status),
                            get_mgr_event_string(event));
#endif
        /* Send disconnection request event to transport async handle specified */
        CLT_EvtDiscReq(pWks, cltId);
        return pWks->status;
    }
    return E_MQTT_MANAGER_STATUS_NOT_INITIALIZED;
}

/* Send a message, called when CMD_SEND_MSG is received */
/* Output status : RUNNING */
static eMqttManagerStatus cbSnd(
    MqttManagerHandle* pWks,
    eMqttManagerStatus status,      /* Shall be RUNNING status */
    eMqttManagerEvent event,        /* Shall be SEND event*/
    MqttTransportAsyncHandle cltId, /* Transport context async handle where send the message*/
    void* bufferData,               /* Message to send */
    uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    if (NULL != pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbSnd entry : status = %s - event = %s\n", get_mgr_status_string(status),
                            get_mgr_event_string(event));
#endif
        /* Send message to send to transport context async handle */
        CLT_EvtSendMsgReq(pWks, cltId, bufferData, dataSize);
        return pWks->status;
    }
    return E_MQTT_MANAGER_STATUS_NOT_INITIALIZED;
}

/* Received message callback invoked when TRANSPORT_CONTEXT_RCV_MSG event is received */
/* Output status : RUNNING */
static eMqttManagerStatus cbRcv(
    MqttManagerHandle* pWks,
    eMqttManagerStatus status,      /* Shall be RUNNING status */
    eMqttManagerEvent event,        /* Shall be TRANSPORT_CONTEXT_RCV_MSG event */
    MqttTransportAsyncHandle cltId, /* Transport context async handle concerned by reception */
    void* bufferData,               /* Message received */
    uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    if (NULL != pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbRcv entry : status = %s - event = %s\n", get_mgr_status_string(status),
                            get_mgr_event_string(event));
#endif
        /* Retrieve transport context workspace from transport context async handle */
        tMqttTransportContext* pCtx = &pWks->slotsTransportContext[cltId];

        //***************CALLBACK RECEPTION ***********************

        if (NULL != pCtx->callbacksConfig.pCbMessageReceived)
        {
            (*pCtx->callbacksConfig.pCbMessageReceived)(cltId, bufferData, dataSize, pCtx->pUserContext);
        }
        return pWks->status;
    }
    return E_MQTT_MANAGER_STATUS_NOT_INITIALIZED;
}

/* Transport context status changed to READY callback, invoked when TRANSPORT_CONTEXT_READY event is received*/
/* Output status : RUNNING */
static eMqttManagerStatus cbCltRdy(
    MqttManagerHandle* pWks,
    eMqttManagerStatus status,      /* Can be RUNNING or STOPPING*/
    eMqttManagerEvent event,        /* Shall be TRANSPORT_CONTEXT_READY */
    MqttTransportAsyncHandle cltId, /* Transport context async handle concerned by status changed event */
    void* bufferData,               /* Not used */
    uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbCltRdy entry : status = %s - event = %s\n", get_mgr_status_string(status),
                            get_mgr_event_string(event));
#endif
        /* Retrieve transport context workspace from transport context async handle */
        tMqttTransportContext* pCtx = &pWks->slotsTransportContext[cltId];

        //***************CALLBACK READY ***********************

        if (NULL != pCtx->callbacksConfig.pCbClientReady)
        {
            (*pCtx->callbacksConfig.pCbClientReady)(cltId, pCtx->pUserContext);
        }
        return pWks->status;
    }
    return E_MQTT_MANAGER_STATUS_NOT_INITIALIZED;
}

/* Transport context status changed to NO READY callback, invoked when TRANSPORT_CONTEXT_NOT_READY event is received*/
/* Output status : RUNNING */
static eMqttManagerStatus cbCltNotRdy(
    MqttManagerHandle* pWks,
    eMqttManagerStatus status,      /* Can be RUNNING or STOPPING */
    eMqttManagerEvent event,        /* Shall be TRANSPORT_CONTEXT_NOT_READY event */
    MqttTransportAsyncHandle cltId, /* Transport context async handle concerned status changed event */
    void* bufferData,               /* Not used */
    uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbCltNotRdy entry : status = %s - event = %s\n", get_mgr_status_string(status),
                            get_mgr_event_string(event));
#endif
        /* Retrieve transport context workspace from transport context async handle */
        tMqttTransportContext* pCtx = &pWks->slotsTransportContext[cltId];

        //***************CALLBACK NOT READY ***********************

        if (NULL != pCtx->callbacksConfig.pCbClientNotReady)
        {
            (*pCtx->callbacksConfig.pCbClientNotReady)(cltId, pCtx->pUserContext);
        }
        return pWks->status;
    }
    return E_MQTT_MANAGER_STATUS_NOT_INITIALIZED;
}

/* Transport context destroyed event callback, invoked when TRANSPORT_CONTEXT_REMOVED event is received */
/* Output status : RUNNING or STOPPED */
static eMqttManagerStatus cbCltRmv(MqttManagerHandle* pWks,
                                   eMqttManagerStatus status,      /* Can be RUNNING or STOPPING */
                                   eMqttManagerEvent event,        /* Shall be TRANSPORT_CONTEXT_REMOVED */
                                   MqttTransportAsyncHandle cltId, /* Transport context concerned by REMOVED event */
                                   void* bufferData,               /* Not used */
                                   uint16_t dataSize)
{
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbCltRmv entry : status = %s - event = %s\n", get_mgr_status_string(status),
                            get_mgr_event_string(event));
#endif
        /* Decrement nb transport context. If this counter is set to 0 and mqtt manager
         * status is STOPPING, scheduler thread going to stop. */
        if (pWks->nbTransportContexts > 0)
        {
            pWks->nbTransportContexts--;
        }

#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbCltRmv current nb client %d\n", pWks->nbTransportContexts);
#endif
        /* No more transport context initialized and status is stopping => go to stopped */
        if (0 == pWks->nbTransportContexts && E_MQTT_MANAGER_STATUS_STOPPING == status)
        {
            pWks->status = E_MQTT_MANAGER_STATUS_STOPPED;
        }

        /* Retrieve transport context from transport context async handle */
        tMqttTransportContext* pCtx = &pWks->slotsTransportContext[cltId];

        //***************CALLBACK REMOVE***********************

        if (NULL != pCtx->callbacksConfig.pCbReleaseHandleResponse)
        {
            (*pCtx->callbacksConfig.pCbReleaseHandleResponse)(cltId, pCtx->pUserContext);
        }

        /* RAZ callbacks configuration and user context pointer */
        memset(&pCtx->callbacksConfig, 0, sizeof(tMqttTransportContextCallbacksConfig));
        pCtx->pUserContext = NULL;

        return pWks->status;
    }
    return E_MQTT_MANAGER_STATUS_NOT_INITIALIZED;
}

/* Quit callback invoked on QUIT_MGR event reception */
/* Output status : STOPPING or STOPPED */
static eMqttManagerStatus cbQuit(MqttManagerHandle* pWks,
                                 eMqttManagerStatus status,      /* Shall be RUNNING status */
                                 eMqttManagerEvent event,        /* Shall be QUIT_MGR event */
                                 MqttTransportAsyncHandle cltId, /* Not used */
                                 void* bufferData,               /* Not used */
                                 uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(cltId);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbQuit entry : status = %s - event = %s\n", get_mgr_status_string(status),
                            get_mgr_event_string(event));
#endif
        if (pWks->nbTransportContexts > 0)
        {
            for (uint16_t i = 0; i < MQTT_MAX_TRANSPORT_CONTEXT; i++)
            {
                pWks->status = E_MQTT_MANAGER_STATUS_STOPPING;
                CLT_EvtDiscReq(pWks, i);
            }
        }
        else
        {
            pWks->status = E_MQTT_MANAGER_STATUS_STOPPED;
        }

        return pWks->status;
    }
    return E_MQTT_MANAGER_STATUS_NOT_INITIALIZED;
}

/* Periodically invoked with EVENT_TICK event.*/
/* Output status : unchanged or NOT_INITIALIZED */
static eMqttManagerStatus cbTmo(MqttManagerHandle* pWks,
                                eMqttManagerStatus status,      /* All status */
                                eMqttManagerEvent event,        /* Shall be TICK */
                                MqttTransportAsyncHandle cltId, /* Not used */
                                void* bufferData,               /* Not used */
                                uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(cltId);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cbTmo entry : timeout value = %d - current status = %s\n", pWks->cptStatusTimeout,
                            get_mgr_status_string(status));
#endif
        pWks->cptStatusTimeout++;
        switch (status)
        {
        case E_MQTT_MANAGER_STATUS_NOT_INITIALIZED:
            break;
        case E_MQTT_MANAGER_STATUS_RUNNING:
            break;
        case E_MQTT_MANAGER_STATUS_STOPPING:
            /* On stopping timeout, go to NOT INITIALIZED.*/
            if (MQTT_STATUS_TIMEOUT_NB_PERIODS == pWks->cptStatusTimeout)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF("\n cbTmo MQTT_STATUS_TIMEOUT --> status to STOPPED\n");
#endif
                pWks->status = E_MQTT_MANAGER_STATUS_STOPPED;
            }
            break;
        default:
            break;
        }
        return pWks->status;
    }
    return E_MQTT_MANAGER_STATUS_NOT_INITIALIZED;
}

/*=== States machine callbacks definition used by MQTT TRANSPORT CONTEXT states machine ===*/

/* This function is used to avoid random function no CI compliant... */

static uint64_t MGR_GetRandom(MqttManagerHandle* pWks)
{
    pWks->random = (pWks->random * 16807 + 0) % 0x7FFFFFFF;
    return pWks->random;
}

/* This callback is invoked on CONN_LOST or CONNECT_CMD event.
 * It create a MQTT PAHO Lib client if necessary and initiates an asynchrone connection operation to the broker */
/* Output status shall be CONNECTING. */
static eMqttTransportContextStatus cbConnReq(
    tMqttTransportContext* pCtx,        /* Transport context workspace */
    eMqttTransportContextStatus status, /* Can be RESERVED, WAIT_FOR_RETRY, SUBSCRIBING, SENDING, READY*/
    eMqttTransportContextEvent event,   /* Can be CONNECT_CMD or CONN_LOST*/
    void* bufferData,  /* Shall be tMqttTransportContextConnexionConfig in the case of RESERVED_STATUS, else not used*/
    uint16_t dataSize) /* Shall be sizeof(tMqttTransportContextConnexionConfig) in the case of RESERVED_STATUS */
{
    SOPC_UNUSED_ARG(event);
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_conn_req entry for client %d\n", pCtx->identification.transportId);
#endif

        /* Set status to CONNECTING */
        pCtx->status = E_MQTT_TRANSPORT_CONTEXT_STATUS_CONNECTING;

        /* In the case of a new try after a failed connection, do not notify "not ready" status */
        if (E_MQTT_TRANSPORT_CONTEXT_STATUS_WAITING_FOR_RETRY != status)
        {
            MGR_EvtCltNotReady(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
        }

        int MQTTAsyncResult = 0;

        /* If the transport context status is RESERVED (result of a get new handle), create a MQTTAsync paho client */
        if (E_MQTT_TRANSPORT_CONTEXT_STATUS_RESERVED == status && NULL != bufferData &&
            sizeof(tMqttTransportContextConnexionConfig) == dataSize)
        {
            /* Retrieve from event data the connexion configuration (uri, topic name which will be used by messages sent
             * and message received). */
            memcpy(&pCtx->connexionConfig, (tMqttTransportContextConnexionConfig*) bufferData,
                   sizeof(tMqttTransportContextConnexionConfig));

            MQTTAsyncResult = api_lib_create(pCtx);

#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
            SOPC_CONSOLE_PRINTF("\n *******************>>>>>>>> %08lX with %s\n", (uint64_t) pCtx->clientHandle,
                                pCtx->connexionConfig.uri);
#endif
        }

        /* Check MQTT Lib client existence. If NULL, self generates a transport context connection error event */

        if (0 != MQTTAsyncResult)
        {
            CLT_EvtConnError(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
        }
        else
        {
            MQTTAsyncResult = api_lib_connect(pCtx);

            if (0 != MQTTAsyncResult)
            {
                /* Effective connection with previous settings, in case of PAHO API error, self generates a connection
                 * error event */
                CLT_EvtConnError(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
            }
        }

        return pCtx->status;
    }

    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/* Connection clear callback is invoked to destroy MQTT PAHO client and send a TRANSPORT_CONTEXT_REMOVED event to MQTT
 * MANAGER It is invoked by event DISCONNECTED when transport context status is DISCONNECTING.*/
/* Output status shall be NOT_INITIALIZED. */
static eMqttTransportContextStatus cbConnClr(tMqttTransportContext* pCtx,
                                             eMqttTransportContextStatus status, /* Shall be DISCONNECTING status */
                                             eMqttTransportContextEvent event,   /* Shall be DISCONNECTED event */
                                             void* bufferData,                   /* Not used */
                                             uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_conn_clr entry for client %d\n", pCtx->identification.transportId);
#endif
        /* Set transport context status to not initialized, raz try counter, connection configuration and flush pending
         * messages to send.*/

        pCtx->status = E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
        pCtx->cptRetry = 0;
        memset(&pCtx->connexionConfig, 0, sizeof(tMqttTransportContextConnexionConfig));
        TransportContext_FlushPendingMsg(pCtx);

        /* Destroy if necessary PAHO LIB client */

#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_conn_clr destroy mqtt lib for client %d\n", pCtx->identification.transportId);
        SOPC_CONSOLE_PRINTF("\n ********************************** %08lX\n", (uint64_t) pCtx->clientHandle);
#endif

        api_lib_destroy(pCtx);

#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_conn_clr destroy mqtt lib well performed for client %d\n",
                            pCtx->identification.transportId);
#endif

        /* Send an event TRANSPORT_CONTEXT_REMOVED to mqtt manager states machine */

        MGR_EvtCltRemoved(pCtx->identification.pMqttMgr, pCtx->identification.transportId);

#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_conn_clr exit for client %d\n", pCtx->identification.transportId);
#endif

        return pCtx->status;
    }
    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/* Connection retry callback is called after a failed connection, and it is invoked on CONNECT_ERR event during
 * CONNECTING status. If current try counter is < 3, transport context status is set to WAITING_FOR_RETRY.
 * Else, set to NOT_INITIALIZED and PAHO lib client is destroyed.*/
/* Output status can be NOT_INITIALIZED or WAITING_FOR_RETRY */
static eMqttTransportContextStatus cbConnTry(tMqttTransportContext* pCtx,
                                             eMqttTransportContextStatus status, /* Shall be CONNECTING */
                                             eMqttTransportContextEvent event,   /* Shall be CONN_ERR */
                                             void* bufferData,                   /* Not used */
                                             uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_retry entry for client %d\n", pCtx->identification.transportId);
#endif
        /* Check status of try counter. If > 3, set to not initialized. Else, go to waiting for retry. */

        if (pCtx->cptRetry >= MQTT_CONNECTION_RETRY_ON_ERROR)
        {
            pCtx->status = E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
        }
        else
        {
            pCtx->status = E_MQTT_TRANSPORT_CONTEXT_STATUS_WAITING_FOR_RETRY;
            pCtx->cptRetry++;
        }

        /* If not initialized, destroy MQTT Paho client, try counter reset, flush pending messages to send
         * then send TRANSPORT_CONTEXT_REMOVED event to Mqtt manager */

        if (E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED == pCtx->status)
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
            SOPC_CONSOLE_PRINTF("\n **********************************\n");
#endif
            api_lib_destroy(pCtx);

            pCtx->cptRetry = 0;

            TransportContext_FlushPendingMsg(pCtx);

            MGR_EvtCltRemoved(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
        }

        return pCtx->status;
    }
    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/* Disconnect request callback is invoked on DISCONNECT or SUBSCRIBE_ERROR event.
 * It call MQTT PAHO lib to initiates disconnection operation and switch status to DISCONNECTING. */
/* Output status shall be DISCONNECTING. */
static eMqttTransportContextStatus cbDiscReq(
    tMqttTransportContext* pCtx,
    eMqttTransportContextStatus status, /* Can be any status except DISCONNECTING, RESERVED and NOT_INITIALIZED */
    eMqttTransportContextEvent event,   /* Shall be DISCONNECT or SUBSCRIBE_ERROR event */
    void* bufferData,                   /* Not used */
    uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_disc_req entry for client %d\n", pCtx->identification.transportId);
#endif
        pCtx->status = E_MQTT_TRANSPORT_CONTEXT_STATUS_DISCONNECTING;

        /* Notify mqtt manager that transport context is not ready */

        MGR_EvtCltNotReady(pCtx->identification.pMqttMgr, pCtx->identification.transportId);

        /* Initiate disconnect operation */

        int MQTTAsyncResult = api_lib_disconnect(pCtx);

        /* In case of API error, self send a DISCONNECTED event.*/
        if (0 != MQTTAsyncResult)
        {
            CLT_EvtDisc(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
        }

        return pCtx->status;
    }

    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/* Callback of subscribe operation success. It is invoked on SUBSCRIBE_SUCCESS event.
 * It sends an TRANSPORT_CONTEXT_READY event to mqtt manager. */
/* Output status shall be READY. */
static eMqttTransportContextStatus cbSubRdy(tMqttTransportContext* pCtx,
                                            eMqttTransportContextStatus status, /* Shall be SUBSCRIBING status.*/
                                            eMqttTransportContextEvent event,   /* Shall be SUBSCRIBE_SUCCESS event.*/
                                            void* bufferData,                   /* Not used. */
                                            uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_sub_rdy entry for client %d\n", pCtx->identification.transportId);
#endif
        /* Set transport context status to ready state.*/
        pCtx->status = E_MQTT_TRANSPORT_CONTEXT_STATUS_READY;

        /* Send TRANSPORT_CONTEXT_READY event to mqtt manager. */
        MGR_EvtCltRdy(pCtx->identification.pMqttMgr, pCtx->identification.transportId);

        return pCtx->status;
    }
    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/* Message reception callback, invoked on MSG_RCV event when status is READY or SENDING.*/
/* Output status is unchanged. */
static eMqttTransportContextStatus cbMsgRcv(tMqttTransportContext* pCtx,
                                            eMqttTransportContextStatus status, /* Can be READY or SENDING. */
                                            eMqttTransportContextEvent event,   /* Shall be MSG_RCV. */
                                            void* bufferData,                   /* Message received. */
                                            uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_recv entry for client %d\n", pCtx->identification.transportId);
#endif
        /* Status not changed. Send event to mqtt manager with data received.*/

        MGR_EvtMsgRcv(pCtx->identification.pMqttMgr, pCtx->identification.transportId, bufferData, dataSize);

        return pCtx->status;
    }

    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/* Subscribe request callback, invoked on CONNECTED event when status is CONNECTING.
 * It calls MQTT Lib PAHO to initiate a subscribe operation. */
/* Output status shall be SUBSCRIBING. */
static eMqttTransportContextStatus cbSubReq(tMqttTransportContext* pCtx,
                                            eMqttTransportContextStatus status, /* Shall be CONNECTING status. */
                                            eMqttTransportContextEvent event,   /* Shall be CONNECTED event. */
                                            void* bufferData,                   /* Not used */
                                            uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_sub_req entry for client %d\n", pCtx->identification.transportId);
#endif
        /* Transport context status is set to subscribing.*/

        pCtx->status = E_MQTT_TRANSPORT_CONTEXT_STATUS_SUBSCRIBING;

        /* When quitting CONNECTING status, set retry counter to 0. */

        pCtx->cptRetry = 0;

        /* Set PAHO callbacks and context. */

        int MQTTAsyncResult = api_lib_subscribe(pCtx);

        /* Initiate subscribe operation */

        if (0 != MQTTAsyncResult)
        {
            CLT_EvtSubFailed(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
        }

        return pCtx->status;
    }

#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
    SOPC_CONSOLE_PRINTF("\n cb_sub_req no context\n");
#endif

    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/* Send message request callback. It is invoked on SEND, SENT_SUCCESS and SENT_ERROR events.*/
/* It call MQTT Lib Paho to initiate a publish operation if a message to send exists. */
/* Output status can be READY or SENDING. */
static eMqttTransportContextStatus cbPubReq(
    tMqttTransportContext* pCtx,
    eMqttTransportContextStatus status, /* Can be READY or SENDING.*/
    eMqttTransportContextEvent event,   /* Can be SEND, SENT_SUCCESS or SENT_ERROR.*/
    void* bufferData,                   /* Shall be not NULL in the case of SEND event. Else not used.*/
    uint16_t dataSize)
{
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_pub_req entry for client %d - event =  %s - status = %s\n",
                            pCtx->identification.transportId, get_event_string(event), get_status_string(status));
#endif
        uint8_t* pendingMsg = NULL;
        uint16_t sizePendingMsg = 0;

        /* In case of SENT error, flush all pending messages. */

        if (E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_SENT_TO_BROKER_FAILED == event)
        {
            TransportContext_FlushPendingMsg(pCtx);
        }

        /* In case of SEND COMMAND, and only if bufferData and dataSize are valid, add message to pending messages to
         * send. */

        if ((E_MQTT_TRANSPORT_CONTEXT_EVENT_SEND == event) && (NULL != bufferData) && (dataSize > 0))
        {
            TransportContext_AddPendingMsg(pCtx, (char*) bufferData, dataSize);
        }

        /* Get the oldest message. This message SHALL BE FREE at the end of this callback. */

        TransportContext_GetPendingMsg(pCtx, (char**) &pendingMsg, &sizePendingMsg);

        /* If there is a message to send, go to SENDING status and raz timeout*/

        if (NULL == pendingMsg)
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
            SOPC_CONSOLE_PRINTF("\n --- No pending messages -> go to ready state ---\n");
#endif
            pCtx->status = E_MQTT_TRANSPORT_CONTEXT_STATUS_READY;

            return pCtx->status;
        }
        else
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
            SOPC_CONSOLE_PRINTF("\n --- Pending messages -> go to sending state ---\n");
#endif
            pCtx->status = E_MQTT_TRANSPORT_CONTEXT_STATUS_SENDING;

            /* In the case of SENT_SUCCESS after overloaded server and message accumulated into pending messages queue
             * status stay on SENDING. So, in this particular case, RAZ status timeout counter.*/

            if (E_MQTT_TRANSPORT_CONTEXT_STATUS_SENDING == status)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF(
                    "\n --- Pending messages -> go to sending state and RAZ TMO because already in SENDING status--- "
                    "\n");
#endif
                pCtx->cptStatusTimeout = 0;
            }

            /* Initiate publish message operation (callbacks, context...) */
            int MQTTAsyncResult = api_lib_send_msg(pCtx, pendingMsg, sizePendingMsg);

            if (0 != MQTTAsyncResult)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF("\n --- Free payload because not treated by mqtt lib ---\n");
#endif

                CLT_EvtMsgSentError(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
            }

            /* Free payload.*/

            SOPC_Free(pendingMsg);
            pendingMsg = NULL;
            return pCtx->status;
        }
    }
    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/* Overrun callback. This callback is called on SEND event when status is SENDING.
 * It push to a fifo the message to send. This fifo will be pop on SENT_SUCCESS event.
 * This fifo will be flushed on SENT_ERROR.*/
/* Output status : unchanged. */
static eMqttTransportContextStatus cbPubBck(tMqttTransportContext* pCtx,
                                            eMqttTransportContextStatus status, /* Shall be SENDING. */
                                            eMqttTransportContextEvent event,   /* Shall be SEND. */
                                            void* bufferData,                   /* Shall be not NULL. */
                                            uint16_t dataSize)                  /* Shall be > 0. */
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_pub_bck entry for client - %d\n", pCtx->identification.transportId);
#endif
        if (NULL != bufferData && dataSize > 0)
        {
            TransportContext_AddPendingMsg(pCtx, (char*) bufferData, dataSize);
        }
        return pCtx->status;
    }
    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/* Periodic callback. Used to increment period counter and generate an error in case of too long transient status
 * (SUBSCRIBING, SENDING...)*/
/* Counter is RAZ out of callback, by the scheduler, if status before callback != status after callback execution */
/* Some callback can RAZ this counter (by ex. cbPubReq in the case of SENT_SUCCESS event during SENDING status). */
/* Output status : unchanged. */
static eMqttTransportContextStatus cbTick(tMqttTransportContext* pCtx,
                                          eMqttTransportContextStatus status, /* All status are concerned. */
                                          eMqttTransportContextEvent event,   /* Shall be TICK_EVENT. */
                                          void* bufferData,                   /* Not used. */
                                          uint16_t dataSize)
{
    SOPC_UNUSED_ARG(status);
    SOPC_UNUSED_ARG(event);
    SOPC_UNUSED_ARG(bufferData);
    SOPC_UNUSED_ARG(dataSize);

    if (pCtx != NULL)
    {
        /* Increment counter. */
        pCtx->cptStatusTimeout++;
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_CALLBACKS == 1 && DEBUG_TMO_TICK == 1
        SOPC_CONSOLE_PRINTF("\n tick = %d - client %d\n ", pCtx->cptStatusTimeout, pCtx->identification.transportId);
#endif
        switch (pCtx->status)
        {
        case E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED:
        {
            /* Nothing */
        }
        break;
        case E_MQTT_TRANSPORT_CONTEXT_STATUS_RESERVED:
        {
            /* Nothing */
        }
        break;
        case E_MQTT_TRANSPORT_CONTEXT_STATUS_DISCONNECTING:
        {
            /* In case of DISCONNECTING status very long timeout, send DISCONNECTED event to concerned transport
             * context. */
            if (MQTT_STATUS_TIMEOUT_DISCONNECTING_NB_PERIODS == pCtx->cptStatusTimeout)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF("\n TIMEOUT for client %d - DISCONNECTING STATUS\n",
                                    pCtx->identification.transportId);
#endif
                CLT_EvtDisc(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
            }
        }
        break;
        case E_MQTT_TRANSPORT_CONTEXT_STATUS_CONNECTING:
        {
            /* In the case of CONNECTING status very long timeout, send CONN_ERR event to concerned transport context.
             */
            if (MQTT_STATUS_TIMEOUT_CONNECTING_NB_PERIODS == pCtx->cptStatusTimeout)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF("\n TIMEOUT for client %d - CONNECTING STATUS\n", pCtx->identification.transportId);
#endif

                CLT_EvtConnError(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
            }
        }
        break;
        case E_MQTT_TRANSPORT_CONTEXT_STATUS_WAITING_FOR_RETRY:
        {
            /* In the case of WAITING_FOR_RETRY timeout, send CONNECT event to concerned transport context. */
            if (MQTT_RETRY_TIMEOUT_NB_PERIODS == pCtx->cptStatusTimeout)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF("\n TIMEOUT for client %d - WAITING_FOR_RETRY STATUS\n",
                                    pCtx->identification.transportId);
#endif
                CLT_EvtConnReq(pCtx->identification.pMqttMgr, pCtx->identification.transportId, NULL);
            }
        }
        break;
        case E_MQTT_TRANSPORT_CONTEXT_STATUS_SUBSCRIBING:
        {
            /* In the case of SUBSCRIBING very long timeout, send SUBSCRIBE_ERR event to concerned transport context. */
            if (MQTT_STATUS_TIMEOUT_SUBSCRIBING_NB_PERIODS == pCtx->cptStatusTimeout)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF("\n TIMEOUT for client %d - SUBSCRIBING STATUS\n",
                                    pCtx->identification.transportId);
#endif
                CLT_EvtSubFailed(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
            }
        }
        break;
        case E_MQTT_TRANSPORT_CONTEXT_STATUS_READY:
        {
            /* Nothing */
        }
        break;
        case E_MQTT_TRANSPORT_CONTEXT_STATUS_SENDING:
        {
            /* In the case of SENDING very long timeout, send a SENT_ERROR event to concerned transport context. */
            if (MQTT_STATUS_TIMEOUT_SENDING_NB_PERIODS == pCtx->cptStatusTimeout)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_CALLBACKS == 1
                SOPC_CONSOLE_PRINTF("\n TIMEOUT for client %d - SENDING STATUS\n", pCtx->identification.transportId);
#endif

                CLT_EvtMsgSentError(pCtx->identification.pMqttMgr, pCtx->identification.transportId);
            }
        }
        break;
        default:
        {
            /* Nothing */
        }
        break;
        }

        return pCtx->status;
    }
    return E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
}

/*=== Definition of static functions used to initialize, write, read and clear an event queue, used by the scheduler of
 * the mqtt manager ===*/

/* Clear scheduler event queue.*/
/* Returns : SOPC_STATUS_OK if well Deinit. */
static SOPC_ReturnStatus EVENT_CHANNEL_deinit(tMqttEventChannel* pChannel)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL != pChannel)
    {
        EVENT_CHANNEL_flush(pChannel);
        Mutex_Clear(&pChannel->lock);
        Condition_Clear(&pChannel->cond);
        memset(pChannel, 0, sizeof(tMqttEventChannel));
    }
    else
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }
    return result;
}

/* Initialize scheduler event queue */
/* Returns : SOPC_STATUS_OK if well init. */
static SOPC_ReturnStatus EVENT_CHANNEL_init(tMqttEventChannel* pChannel) /* Event queue handle */
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL != pChannel)
    {
        memset(pChannel, 0, sizeof(tMqttEventChannel));

        result = Mutex_Initialization(&pChannel->lock);
        if (SOPC_STATUS_OK != result)
        {
            EVENT_CHANNEL_deinit(pChannel);
            return result;
        }

        result = Condition_Init(&pChannel->cond);
        if (SOPC_STATUS_OK != result)
        {
            EVENT_CHANNEL_deinit(pChannel);
            return result;
        }
    }
    return result;
}

/* Add event to scheduler event queue*/
/* Returns : SOPC_STATUS_OK if well pushed. */
static SOPC_ReturnStatus EVENT_CHANNEL_push(tMqttEventChannel* pChannel, /* Event queue handle */
                                            tMqttEvent* pEvent)          /* Event */
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL != pChannel)
    {
        /* Critical section */
        Mutex_Lock(&pChannel->lock);
        if (pChannel->nbEvents < MQTT_MAX_BUFFER_EVENT)
        {
            /* Copy event info (value, free callback...) */
            memcpy(&pChannel->bufferEvents[pChannel->idxWrEvent], pEvent, sizeof(tMqttEvent));

            /* Next event to write */
            pChannel->idxWrEvent++;

            /* Circular buffer management */
            if (MQTT_MAX_BUFFER_EVENT == pChannel->idxWrEvent)
            {
                pChannel->idxWrEvent = 0;
            }

            /* Count an event */
            pChannel->nbEvents++;
            if (1 == pChannel->nbEvents)
            {
                /* One event, signal fifo not empty */
                Condition_SignalAll(&pChannel->cond);
            }
        }
        else
        {
            /* Fifo full, free current event */
            if (NULL != pEvent->pEventData)
            {
                if (NULL != pEvent->pClearCallback)
                {
                    (*pEvent->pClearCallback)(pEvent->pEventData);
                    pEvent->pEventData = NULL;
                }
            }
            result = SOPC_STATUS_OUT_OF_MEMORY;
        }
        /* End of critical section */
        Mutex_Unlock(&pChannel->lock);
    }
    else
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }

    return result;
}

/* Flush events of scheduler event queue. All events are free if free callback is associated. */
/* Returns : SOPC_STATUS_OK if well flushed. */
static SOPC_ReturnStatus EVENT_CHANNEL_flush(tMqttEventChannel* pChannel) /* Event channel handle */
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL != pChannel)
    {
        /* Critical section */
        Mutex_Lock(&pChannel->lock);

        /* Loop on all remainings events to free */
        while (pChannel->nbEvents > 0)
        {
            /* If data exists and clear callback exist, call clear callback on data. */
            if (NULL != pChannel->bufferEvents[pChannel->idxRdEvent].pClearCallback)
            {
                if (NULL != pChannel->bufferEvents[pChannel->idxRdEvent].pEventData)
                {
                    (*pChannel->bufferEvents[pChannel->idxRdEvent].pClearCallback)(
                        pChannel->bufferEvents[pChannel->idxRdEvent].pEventData);
                }
            }

            /* Raz event */
            memset(&pChannel->bufferEvents[pChannel->idxRdEvent], 0, sizeof(tMqttEvent));

            /* Next event to read. */
            pChannel->idxRdEvent++;

            /* Circular buffer management */
            if (MQTT_MAX_BUFFER_EVENT == pChannel->idxRdEvent)
            {
                pChannel->idxRdEvent = 0;
            }

            /*  Count read event */
            pChannel->nbEvents--;
        }

        /* End of critical section */
        Mutex_Unlock(&pChannel->lock);
    }
    else
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }

    return result;
}

/* Read event from scheduler event queue */
/* Returns : SOPC_STATUS_OK if event returned. */
static SOPC_ReturnStatus EVENT_CHANNEL_pop(tMqttEventChannel* pChannel, /* Event channel handle */
                                           tMqttEvent* pEvent,          /* Read event*/
                                           uint32_t timeout)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL != pChannel && NULL != pEvent)
    {
        /* Critical section */
        Mutex_Lock(&pChannel->lock);

        /* If timeout is used and nbEvents = 0, wait signal. */
        if (pChannel->nbEvents == 0 && timeout > 0)
        {
            result = Mutex_UnlockAndTimedWaitCond(&pChannel->cond, &pChannel->lock, timeout);
        }

        /* If signal received and if nbEvents > 0, so copy out event. */
        if (SOPC_STATUS_OK == result)
        {
            if (pChannel->nbEvents > 0)
            {
                /* Read out event */
                memcpy(pEvent, &pChannel->bufferEvents[pChannel->idxRdEvent], sizeof(tMqttEvent));
                /* Clean read event into circular buffer */
                memset(&pChannel->bufferEvents[pChannel->idxRdEvent], 0, sizeof(tMqttEvent));
                /* Next event to pop */
                pChannel->idxRdEvent++;
                /* Circular buffer management */
                if (MQTT_MAX_BUFFER_EVENT == pChannel->idxRdEvent)
                {
                    pChannel->idxRdEvent = 0;
                }
                /* Count read event */
                pChannel->nbEvents--;
            }
            else
            {
                result = SOPC_STATUS_NOK;
            }
        }
        /* End of critical section */
        Mutex_Unlock(&pChannel->lock);
    }
    else
    {
        result = SOPC_STATUS_INVALID_PARAMETERS;
    }

    return result;
}

/*=== Definitions of static functions used to send an event to the mqtt manager state machine ===*/

/* Generate a CMD_QUIT_MGR event and push it to mqtt manager scheduler event queue*/

static SOPC_ReturnStatus MGR_EvtQuitScheduler(MqttManagerHandle* pWks)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;

    event.event = E_MQTT_MANAGER_EVENT_CMD_QUIT_MGR;
    event.idFsm = E_MQTT_ID_FSM_MANAGER;
    event.idTransportContext = 0;
    event.pEventData = NULL;
    event.size = 0;
    event.pClearCallback = NULL;

    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n TRS_EvtQuitScheduler critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a TRANSPORT_CONTEXT_REMOVED event and push it to mqtt manager scheduler event queue*/

static SOPC_ReturnStatus MGR_EvtCltRemoved(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_REMOVED;
    event.idFsm = E_MQTT_ID_FSM_MANAGER;
    event.pEventData = NULL;
    event.idTransportContext = idx;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n TRS_EvtCltRemoved critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a TRANSPORT_CONTEXT_READY event and push it to mqtt manager scheduler event queue*/

static SOPC_ReturnStatus MGR_EvtCltRdy(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_READY;
    event.idFsm = E_MQTT_ID_FSM_MANAGER;
    event.pEventData = NULL;
    event.idTransportContext = idx;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n TRS_EvtCltRdy critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a TRANSPORT_CONTEXT_NOT_READY event and push it to mqtt manager scheduler event queue */

static SOPC_ReturnStatus MGR_EvtCltNotReady(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_NOT_READY;
    event.idFsm = E_MQTT_ID_FSM_MANAGER;
    event.pEventData = NULL;
    event.idTransportContext = idx;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n TRS_EvtCltNotReady critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}
/* Generate a TRANSPORT_CONTEXT_RECV_MSG event and push it to mqtt manager scheduler event queue */

static SOPC_ReturnStatus MGR_EvtMsgRcv(MqttManagerHandle* pWks,
                                       MqttTransportAsyncHandle idx,
                                       uint8_t* bufferData,
                                       uint16_t size)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_RECV_MSG;
    event.idFsm = E_MQTT_ID_FSM_MANAGER;
    event.idTransportContext = idx;
    event.pEventData = SOPC_Calloc(1, size);
    if (NULL != event.pEventData)
    {
        memcpy(event.pEventData, bufferData, size);
        event.size = size;
        event.pClearCallback = &SOPC_Free;
        if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
            SOPC_CONSOLE_PRINTF("\n TRS_EvtMsgRcv critical error\n");
#endif
            EVENT_CHANNEL_flush(&pWks->schedulerChannel);
            result = SOPC_STATUS_NOK;
        }
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n TRS_EvtMsgRcv critical error\n");
#endif
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a CMD_SEND_MSG event and push it to mqtt manager scheduler event queue */

static SOPC_ReturnStatus MGR_EvtSendMsg(MqttManagerHandle* pWks,
                                        MqttTransportAsyncHandle idx,
                                        uint8_t* bufferData,
                                        uint16_t size)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_MANAGER_EVENT_CMD_SEND_MSG;
    event.idFsm = E_MQTT_ID_FSM_MANAGER;
    event.idTransportContext = idx;
    event.pEventData = SOPC_Calloc(1, size);
    if (NULL != event.pEventData)
    {
        memcpy(event.pEventData, bufferData, size);
        event.size = size;
        event.pClearCallback = &SOPC_Free;

        if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
            SOPC_CONSOLE_PRINTF("\n TRS_EvtSendMsg critical error\n");
#endif
            EVENT_CHANNEL_flush(&pWks->schedulerChannel);
            result = SOPC_STATUS_NOK;
        }
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n TRS_EvtSendMsg critical error\n");
#endif
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a CMD_DEL_TRANSPORT_CONTEXT event and push it to mqtt manager scheduler event queue */

static SOPC_ReturnStatus MGR_EvtReleaseHdl(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_MANAGER_EVENT_CMD_DEL_TRANSPORT_CONTEXT;
    event.idFsm = E_MQTT_ID_FSM_MANAGER;
    event.pEventData = NULL;
    event.idTransportContext = idx;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n TRS_EvtReleaseHdl critical error\n");
#endif
        result = SOPC_STATUS_NOK;
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
    }
    return result;
}

/*=== Definitions of functions used to send an event to a transport context state machine ===*/

/* Generate a CONNECT event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtConnReq(MqttManagerHandle* pWks,
                                 MqttTransportAsyncHandle idx,
                                 tMqttTransportContextConnexionConfig* pConfig)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_CONNECT;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    if (pConfig != NULL)
    {
        event.pEventData = SOPC_Calloc(1, sizeof(tMqttTransportContextConnexionConfig));
        if (NULL != event.pEventData)
        {
            memcpy(event.pEventData, pConfig, sizeof(tMqttTransportContextConnexionConfig));
            event.idTransportContext = idx;
            event.size = sizeof(tMqttTransportContextConnexionConfig);
            event.pClearCallback = &SOPC_Free;
            if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
                SOPC_CONSOLE_PRINTF("\n CLT_EvtConnReq critical error\n");
#endif
                EVENT_CHANNEL_flush(&pWks->schedulerChannel);
                result = SOPC_STATUS_NOK;
            }
        }
        else
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
            SOPC_CONSOLE_PRINTF("\n CLT_EvtConnReq critical error\n");
#endif
            result = SOPC_STATUS_NOK;
        }
    }
    else
    {
        event.pEventData = NULL;
        event.idTransportContext = idx;
        event.size = 0;
        event.pClearCallback = NULL;
        if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
            SOPC_CONSOLE_PRINTF("\n CLT_EvtConnReq critical error\n");
#endif
            EVENT_CHANNEL_flush(&pWks->schedulerChannel);
            result = SOPC_STATUS_NOK;
        }
    }
    return result;
}

/* Generate a SEND event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtSendMsgReq(MqttManagerHandle* pWks,
                                    MqttTransportAsyncHandle idx,
                                    uint8_t* bufferData,
                                    uint16_t size)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_SEND;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;

    event.idTransportContext = idx;
    event.pEventData = SOPC_Calloc(1, size);
    if (NULL != event.pEventData)
    {
        memcpy(event.pEventData, bufferData, size);
        event.size = size;
        event.pClearCallback = &SOPC_Free;
        if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
            SOPC_CONSOLE_PRINTF("\n CLT_EvtSendMsgReq critical error\n");
#endif
            EVENT_CHANNEL_flush(&pWks->schedulerChannel);
            result = SOPC_STATUS_NOK;
        }
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtSendMsgReq critical error\n");
#endif
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a DISCONNECT event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtDiscReq(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_DISCONNECT;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.pEventData = NULL;
    event.idTransportContext = idx;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtDiscReq critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a CONN_LOST event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtConnLost(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_BROKER_CONNECTION_LOST;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.idTransportContext = idx;
    event.pEventData = NULL;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtConnLost critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a DISCONNECTED event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtDisc(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_DISCONNECTED_FROM_BROKER;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.idTransportContext = idx;
    event.pEventData = NULL;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtDisc critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a CONN_ERROR event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtConnError(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_BROKER_CONNECTION_ERR;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.idTransportContext = idx;
    event.pEventData = NULL;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtConnError critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a CONNECTED event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtConnSuccess(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_CONNECTED_TO_BROKER;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.idTransportContext = idx;
    event.pEventData = NULL;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtConnSuccess critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a SUBSCRIBE_FAILED event and push it to mqtt manager scheduler event queue for a specific transport context
 */

SOPC_ReturnStatus CLT_EvtSubFailed(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_SUBSCRIBED_TO_BROKER_FAILED;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.idTransportContext = idx;
    event.pEventData = NULL;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtSubFailed critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a SUBSCRIBE_SUCCESS event and push it to mqtt manager scheduler event queue for a specific transport context
 */

SOPC_ReturnStatus CLT_EvtSubSuccess(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_SUBSCRIBED_TO_BROKER_SUCCESSFULL;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.idTransportContext = idx;
    event.pEventData = NULL;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtSubSuccess critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a SENT_SUCCESS event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtMsgSent(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_SENT_TO_BROKER_SUCCESSFULL;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.idTransportContext = idx;
    event.pEventData = NULL;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtMsgSent critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a SENT_ERROR event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtMsgSentError(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_SENT_TO_BROKER_FAILED;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.idTransportContext = idx;
    event.pEventData = NULL;
    event.size = 0;
    event.pClearCallback = NULL;
    if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtMsgSentError critical error\n");
#endif
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/* Generate a MSG_RCV event and push it to mqtt manager scheduler event queue for a specific transport context */

SOPC_ReturnStatus CLT_EvtMsgRcv(MqttManagerHandle* pWks, MqttTransportAsyncHandle idx, char* msgData, uint16_t msgLen)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    tMqttEvent event;
    event.event = E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_RECEIVED_FROM_BROKER;
    event.idFsm = E_MQTT_ID_FSM_TRANSPORT_CONTEXT;
    event.idTransportContext = idx;
    event.pEventData = SOPC_Calloc(1, msgLen);
    if (NULL != event.pEventData)
    {
        memcpy(event.pEventData, msgData, msgLen);
        event.size = msgLen;
        event.pClearCallback = &SOPC_Free;
        if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pWks->schedulerChannel, &event))
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
            SOPC_CONSOLE_PRINTF("\n CLT_EvtMsgRcv critical error\n");
#endif
            EVENT_CHANNEL_flush(&pWks->schedulerChannel);
            result = SOPC_STATUS_NOK;
        }
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
        SOPC_CONSOLE_PRINTF("\n CLT_EvtMsgRcv critical error\n");
#endif
        result = SOPC_STATUS_NOK;
    }
    return result;
}

/*=== Definition of static functions used to add or get pending messages to send to a transport context ===*/

/* TransportContext_AddPendingMsg is used to add a message "message to send" list on SEND event */

static void TransportContext_AddPendingMsg(tMqttTransportContext* pClt, char* msgData, uint16_t msgLen)
{
    tMqttEvent event;
    event.event = 0;
    event.idFsm = 0;
    event.idTransportContext = 0;
    event.pEventData = SOPC_Calloc(1, msgLen);
    if (NULL != event.pEventData)
    {
        memcpy(event.pEventData, msgData, msgLen);
        event.size = msgLen;
        event.pClearCallback = &SOPC_Free;
        if (SOPC_STATUS_OK != EVENT_CHANNEL_push(&pClt->pendingsMessagesToSend, &event))
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_CHANNEL_UNAVAILABLE_ERROR == 1
            SOPC_CONSOLE_PRINTF("\n Overflow pendings messages\n");
#endif
            EVENT_CHANNEL_flush(&pClt->pendingsMessagesToSend);
        }
    }
}

/* TransportContext_FlushPendingMsg is used to ignore all messages backup during SENDING status  */

static void TransportContext_FlushPendingMsg(tMqttTransportContext* pClt)
{
    EVENT_CHANNEL_flush(&pClt->pendingsMessagesToSend);
}

/* TransportContext_GetPendingMsg is used to get a message to send on SEND or SENT event */

static void TransportContext_GetPendingMsg(tMqttTransportContext* pClt, char** msgData, uint16_t* msgLen)
{
    tMqttEvent event;
    if (SOPC_STATUS_OK == EVENT_CHANNEL_pop(&pClt->pendingsMessagesToSend, &event, 0))
    {
#if DEBUG_SCHEDULER == 1
        SOPC_CONSOLE_PRINTF("\n --- TransportContext_GetPendingMsg SOME pending msg ---\n");
#endif
        *msgData = event.pEventData;
        *msgLen = event.size;
    }
    else
    {
#if DEBUG_SCHEDULER == 1
        SOPC_CONSOLE_PRINTF("\n --- TransportContext_GetPendingMsg NO pending msg ---\n");
#endif
        *msgData = NULL;
        *msgLen = 0;
    }
}

/*=== Definition of callbacks used by PAHO MQTT library ===*/

#if USE_MQTT_PAHO == 1
static void cb_lib_onConnLost(void* context, char* cause)
{
    SOPC_UNUSED_ARG(cause);
    tMqttTransportContextIdentification* pClientContext = ((tMqttTransportContextIdentification*) context);
    if (NULL != pClientContext)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onConnLost\n");
#endif
        CLT_EvtConnLost(pClientContext->pMqttMgr, pClientContext->transportId);
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onConnLost - no context\n");
#endif
    }
}

static int cb_lib_onMsgRcv(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
    SOPC_UNUSED_ARG(topicLen);
    char* payloadptr = NULL;

    tMqttTransportContextIdentification* pClientContext = ((tMqttTransportContextIdentification*) context);

    if (NULL != pClientContext)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onMsgRcv\n");
#endif
        payloadptr = message->payload;
        CLT_EvtMsgRcv(pClientContext->pMqttMgr, pClientContext->transportId, payloadptr,
                      (uint16_t) message->payloadlen);
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onMsgRcv - no context\n");
#endif
    }
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}
static void cb_lib_onDisconnect(void* context, MQTTAsync_successData* response)
{
    SOPC_UNUSED_ARG(response);
    tMqttTransportContextIdentification* pClientContext = ((tMqttTransportContextIdentification*) context);
    if (NULL != pClientContext)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onDisconnect\n");
#endif
        CLT_EvtDisc(pClientContext->pMqttMgr, pClientContext->transportId);
    }
    else
    {
        SOPC_CONSOLE_PRINTF("\n cb_lib_onDisconnect - no context\n");
    }
}

static void cb_lib_onSubscribe(void* context, MQTTAsync_successData* response)
{
    SOPC_UNUSED_ARG(response);
    tMqttTransportContextIdentification* pClientContext = ((tMqttTransportContextIdentification*) context);
    if (NULL != pClientContext)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onSubscribe\n");
#endif
        CLT_EvtSubSuccess(pClientContext->pMqttMgr, pClientContext->transportId);
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onSubscribe - no context\n");
#endif
    }
}
static void cb_lib_onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    SOPC_UNUSED_ARG(response);
    tMqttTransportContextIdentification* pClientContext = ((tMqttTransportContextIdentification*) context);
    if (NULL != pClientContext)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onSubscribeFailure\n");
#endif
        CLT_EvtSubFailed(pClientContext->pMqttMgr, pClientContext->transportId);
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onSubscribeFailure - no context\n");
#endif
    }
}

static void cb_lib_onSend(void* context, MQTTAsync_successData* response)
{
    SOPC_UNUSED_ARG(response);

    tMqttTransportContextIdentification* pClientContext = ((tMqttTransportContextIdentification*) context);
    if (NULL != pClientContext)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onSend - clt = %dt\n", pClientContext->transportId);
#endif
        CLT_EvtMsgSent(pClientContext->pMqttMgr, pClientContext->transportId);
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onSend - no context\n");
#endif
    }
}

static void cb_lib_onSendFailure(void* context, MQTTAsync_failureData* response)
{
    SOPC_UNUSED_ARG(response);
    tMqttTransportContextIdentification* pClientContext = ((tMqttTransportContextIdentification*) context);
    if (NULL != pClientContext)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onSendFailure\n");
#endif
        CLT_EvtMsgSentError(pClientContext->pMqttMgr, pClientContext->transportId);
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onSendFailure - no context\n");
#endif
    }
}

static void cb_lib_onConnectFailure(void* context, MQTTAsync_failureData* response)
{
    SOPC_UNUSED_ARG(response);
    tMqttTransportContextIdentification* pClientContext = ((tMqttTransportContextIdentification*) context);
    if (NULL != pClientContext)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onConnectFailure\n");
#endif
        CLT_EvtConnError(pClientContext->pMqttMgr, pClientContext->transportId);
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onConnectFailure - no context\n");
#endif
    }
}
static void cb_lib_onConnect(void* context, MQTTAsync_successData* response)
{
    SOPC_UNUSED_ARG(response);
    tMqttTransportContextIdentification* pClientContext = ((tMqttTransportContextIdentification*) context);
    if (NULL != pClientContext)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onConnect\n");
#endif
        CLT_EvtConnSuccess(pClientContext->pMqttMgr, pClientContext->transportId);
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LIB_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n cb_lib_onConnect - no context\n");
#endif
    }
}
#endif

/*=== Definition of wrappers of PAHO MQTT library ===*/

static int api_lib_create(tMqttTransportContext* pCtx)
{
    if (NULL == pCtx)
    {
        return -1;
    }

    /* Generate a client string id from MqttTransportAsyncHandle*/
    uint64_t random = MGR_GetRandom(pCtx->identification.pMqttMgr);

    char clientIdStrFromIdx[24] = {0};
    snprintf(clientIdStrFromIdx, 23, "%lu", (long unsigned int) random);

#if USE_MQTT_PAHO == 1

    int MQTTAsyncResult = MQTTAsync_create(&pCtx->clientHandle, pCtx->connexionConfig.uri, clientIdStrFromIdx,
                                           MQTTCLIENT_PERSISTENCE_NONE, NULL);

    if (MQTTASYNC_SUCCESS != MQTTAsyncResult)
    {
        return -1;
    }
    return 0;
#else
    return -1;
#endif
}

static void api_lib_destroy(tMqttTransportContext* pCtx)
{
    if (NULL == pCtx)
    {
        return;
    }

    if (NULL == pCtx->clientHandle)
    {
        return;
    }
#if USE_MQTT_PAHO == 1
    MQTTAsync_destroy(&pCtx->clientHandle);
#endif
    pCtx->clientHandle = NULL;
}

static int api_lib_connect(tMqttTransportContext* pCtx)
{
    if (pCtx == NULL)
    {
        return -1;
    }

    if (NULL == pCtx->clientHandle)
    {
        return -1;
    }
#if USE_MQTT_PAHO == 1
    /* Set library connection and reception callbacks, set lost connection detection interval, connection
     * timeout and context */
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;

    int MQTTAsyncResult = 0;

    MQTTAsyncResult =
        MQTTAsync_setCallbacks(pCtx->clientHandle, &pCtx->identification, cb_lib_onConnLost, cb_lib_onMsgRcv, NULL);

    if (MQTTASYNC_SUCCESS != MQTTAsyncResult)
    {
        return -1;
    }
    else
    {
        conn_opts.keepAliveInterval = MQTT_LIB_KEEPALIVE;
        conn_opts.connectTimeout = MQTT_LIB_CONNECTION_TIMEOUT;
        conn_opts.cleansession = 1;
        conn_opts.onSuccess = cb_lib_onConnect;
        conn_opts.onFailure = cb_lib_onConnectFailure;
        conn_opts.context = &pCtx->identification; /* Context shall be identification information : manager
                                                                 handle and transport context async handle */
        MQTTAsyncResult = MQTTAsync_connect(pCtx->clientHandle, &conn_opts);

        if (MQTTASYNC_SUCCESS != MQTTAsyncResult)
        {
            return -1;
        }
    }
    return 0;
#else
    return -1;
#endif
}

static int api_lib_subscribe(tMqttTransportContext* pCtx)
{
    if (NULL == pCtx)
    {
        return -1;
    }

    if (NULL == pCtx->clientHandle)
    {
        return -1;
    }
#if USE_MQTT_PAHO == 1
    MQTTAsync_responseOptions sub_opts = MQTTAsync_responseOptions_initializer;
    sub_opts.onSuccess = cb_lib_onSubscribe;
    sub_opts.onFailure = cb_lib_onSubscribeFailure;
    sub_opts.context = &pCtx->identification;
    int MQTTAsyncResult =
        MQTTAsync_subscribe(pCtx->clientHandle, pCtx->connexionConfig.topicname, MQTT_LIB_QOS, &sub_opts);

    /* Initiate subscribe operation */

    if (MQTTASYNC_SUCCESS != MQTTAsyncResult)
    {
        return -1;
    }
    return 0;
#else
    return -1;
#endif
}

static int api_lib_send_msg(tMqttTransportContext* pCtx, uint8_t* data, uint16_t dataSize)
{
    if (NULL == pCtx)
    {
        return -1;
    }

    if (NULL == pCtx->clientHandle)
    {
        return -1;
    }
#if USE_MQTT_PAHO == 1
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;

    opts.onSuccess = cb_lib_onSend;
    opts.onFailure = cb_lib_onSendFailure;
    opts.context = &pCtx->identification; /* Context shall be transport context identification*/

    pubmsg.payload = data;
    pubmsg.payloadlen = dataSize;
    pubmsg.qos = MQTT_LIB_QOS;

    int MQTTAsyncResult = 0;

    MQTTAsyncResult = MQTTAsync_sendMessage(pCtx->clientHandle, pCtx->connexionConfig.topicname, &pubmsg, &opts);

    if (MQTTASYNC_SUCCESS != MQTTAsyncResult)
    {
        return -1;
    }

    return 0;
#else
    SOPC_UNUSED_ARG(data);
    SOPC_UNUSED_ARG(dataSize);
    return -1;
#endif
}

static int api_lib_disconnect(tMqttTransportContext* pCtx)
{
    if (NULL == pCtx)
    {
        return -1;
    }

    if (NULL == pCtx->clientHandle)
    {
        return -1;
    }
#if USE_MQTT_PAHO == 1
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer5;
    disc_opts.onSuccess = cb_lib_onDisconnect; /* Disconnect callback */
    disc_opts.context = &pCtx->identification; /* Context shall be transport context identification */
    disc_opts.timeout = 0;
    int MQTTAsyncResult = MQTTAsync_disconnect(pCtx->clientHandle, &disc_opts);

    /* In case of API error, self send a DISCONNECTED event.*/
    if (MQTTASYNC_SUCCESS != MQTTAsyncResult)
    {
        return -1;
    }
    return 0;
#else
    return -1;
#endif
}

/*=== Definition of debug functions to display user friendly status / events ===*/

#if DEBUG_SCHEDULER == 1
static const char* get_mgr_status_string(eMqttManagerStatus status)
{
    switch (status)
    {
    case E_MQTT_MANAGER_STATUS_NOT_INITIALIZED:
        return "E_MQTT_TRANSPORT_STATUS_NOT_INITIALIZED";
    case E_MQTT_MANAGER_STATUS_RUNNING:
        return "E_MQTT_TRANSPORT_STATUS_RUNNING";
    case E_MQTT_MANAGER_STATUS_STOPPING:
        return "E_MQTT_TRANSPORT_STATUS_STOPPING";
    case E_MQTT_MANAGER_STATUS_STOPPED:
        return "E_MQTT_TRANSPORT_STATUS_STOPPED";
    default:
        return "XXX";
        break;
    }
}
static const char* get_mgr_event_string(eMqttManagerEvent event)
{
    switch (event)
    {
    case E_MQTT_MANAGER_EVENT_CMD_NEW_TRANSPORT_CONTEXT:
        return "E_MQTT_TRANSPORT_EVENT_CMD_NEW_HANDLE";
    case E_MQTT_MANAGER_EVENT_CMD_DEL_TRANSPORT_CONTEXT:
        return "E_MQTT_TRANSPORT_EVENT_CMD_DEL_HANDLE";
    case E_MQTT_MANAGER_EVENT_CMD_SEND_MSG:
        return "E_MQTT_TRANSPORT_EVENT_CMD_SEND_MSG";
    case E_MQTT_MANAGER_EVENT_CMD_QUIT_MGR:
        return "E_MQTT_TRANSPORT_EVENT_CMD_QUIT";
    case E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_RECV_MSG:
        return "E_MQTT_TRANSPORT_EVENT_RECV_MSG";
    case E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_READY:
        return "E_MQTT_TRANSPORT_EVENT_RECV_MSG";
    case E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_NOT_READY:
        return "E_MQTT_TRANSPORT_EVENT_CLIENT_NOT_READY";
    case E_MQTT_MANAGER_EVENT_TRANSPORT_CONTEXT_REMOVED:
        return "E_MQTT_TRANSPORT_EVENT_CLIENT_REMOVED";
    case E_MQTT_MANAGER_EVENT_TICK:
        return "E_MQTT_TRANSPORT_EVENT_TICK";
    default:
        return "XXX";
        break;
    }
}
static const char* get_status_string(eMqttTransportContextStatus status)
{
    switch (status)
    {
    case E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED:
        return "NOT_INITIALIZED";
    case E_MQTT_TRANSPORT_CONTEXT_STATUS_RESERVED:
        return "RESERVED";
    case E_MQTT_TRANSPORT_CONTEXT_STATUS_DISCONNECTING:
        return "DISCONNECTING";
    case E_MQTT_TRANSPORT_CONTEXT_STATUS_CONNECTING:
        return "CONNECTING";
    case E_MQTT_TRANSPORT_CONTEXT_STATUS_WAITING_FOR_RETRY:
        return "WAITING_FOR_RETRY";
    case E_MQTT_TRANSPORT_CONTEXT_STATUS_SUBSCRIBING:
        return "SUBSCRIBING";
    case E_MQTT_TRANSPORT_CONTEXT_STATUS_READY:
        return "READY";
    case E_MQTT_TRANSPORT_CONTEXT_STATUS_SENDING:
        return "SENDING";
    default:
        return "XXX";
        break;
    }
}
static const char* get_event_string(eMqttTransportContextEvent event)
{
    switch (event)
    {
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_CONNECT:
        return "CONNECT";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_DISCONNECT:
        return "DISCONNECT";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_SEND:
        return "SEND";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_DISCONNECTED_FROM_BROKER:
        return "DISCONNECTED";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_CONNECTED_TO_BROKER:
        return "CONNECTED";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_BROKER_CONNECTION_ERR:
        return "CONN ERR";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_BROKER_CONNECTION_LOST:
        return "CONN LOST";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_SENT_TO_BROKER_SUCCESSFULL:
        return "SENT SUCCESS";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_SENT_TO_BROKER_FAILED:
        return "SENT ERR";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_SUBSCRIBED_TO_BROKER_SUCCESSFULL:
        return "SUB SUCCESS";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_SUBSCRIBED_TO_BROKER_FAILED:
        return "SUB ERR";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_MESSAGE_RECEIVED_FROM_BROKER:
        return "RCV";
    case E_MQTT_TRANSPORT_CONTEXT_EVENT_TICK:
        return "TICK";
    default:
        return "XXX";
        break;
    }
}
#endif

/*=== Definition of callbacks of the thread used by Mqtt manager scheduler ===*/

static void* cbTask_MqttManagerScheduler(void* pArg)
{
    MqttManagerHandle* pWks = (MqttManagerHandle*) pArg;           /* Mqtt manager handle (or workspace...) */
    SOPC_TimeReference lastTick = SOPC_TimeReference_GetCurrent(); /* Used to generate a tick event */
    const uint32_t period = MQTT_SCHEDULER_PERIOD_MS;              /* 100ms */
    uint32_t timeToWait =
        period; /* Blocking time of sheduler event in case of no event. Reajusted to respect periodic event.*/
    SOPC_TimeReference current = 0; /* Used to generate a tick event */
    bool sendTick = false; /* Boolean used to indicates that a tick event shall be submit to all states machines */

    tMqttEvent currentEvent; /* Last event received */

    memset(&currentEvent, 0, sizeof(tMqttEvent));

    pWks->status = E_MQTT_MANAGER_STATUS_RUNNING;

    /* While not stopped, loop*/

    while (E_MQTT_MANAGER_STATUS_STOPPED != pWks->status && E_MQTT_MANAGER_STATUS_NOT_INITIALIZED != pWks->status)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_LOOP_ALIVE == 1
        SOPC_CONSOLE_PRINTF("\n================================++ IM ALIVE ++===================================\n");
#endif

        /* Get current tick used to calculate timeToWait and eval timeout */

        lastTick = SOPC_TimeReference_GetCurrent();

        /* Wait for an event during max of scheduler period*/

        if (SOPC_STATUS_OK == EVENT_CHANNEL_pop(&pWks->schedulerChannel, &currentEvent, timeToWait))
        {
#if DEBUG_SCHEDULER == 1
            SOPC_CONSOLE_PRINTF("\n Event received : id=%d ev=%d idx=%d\n", currentEvent.idFsm, currentEvent.event,
                                currentEvent.idTransportContext);
#endif
            /* Choose of state machine, MQTT manager or MQTT Transport Async Context*/

            switch (currentEvent.idFsm)
            {
            case E_MQTT_ID_FSM_MANAGER:
            {
                /* Systematically check if status and event are in max range of the table which represents the state
                 * machine */
                if (currentEvent.event < E_MQTT_MANAGER_EVENT_MAX && pWks->status < E_MQTT_MANAGER_STATUS_MAX)

                {
#if DEBUG_SCHEDULER == 1
                    SOPC_CONSOLE_PRINTF("\nSCH pre-status = %d\n", pWks->status);
#endif
                    /* Save current status, used to check status change after event treatment and raz tick counter */

                    volatile eMqttManagerStatus previous_status = pWks->status;

                    /* Systematically check if a callback exist for current status and current event before invoke it */

                    if (NULL != tabFsmScheduler[pWks->status][currentEvent.event])
                    {
                        pWks->status = tabFsmScheduler[pWks->status][currentEvent.event](
                            pWks, pWks->status, currentEvent.event, currentEvent.idTransportContext,
                            currentEvent.pEventData, currentEvent.size);
                    }

                    /* RAZ status timeout if status changes. */

                    if (pWks->status != previous_status)
                    {
                        pWks->cptStatusTimeout = 0;
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_TICK == 1
                        SOPC_CONSOLE_PRINTF("\n RAZ SCHEDULER TMO CPT\n");
#endif
                    }

#if DEBUG_SCHEDULER == 1
                    SOPC_CONSOLE_PRINTF("\nSCH post-status = %d\n", pWks->status);
#endif
                }
            }
            break;
            case E_MQTT_ID_FSM_TRANSPORT_CONTEXT:
            {
#if DEBUG_SCHEDULER == 1
                SOPC_CONSOLE_PRINTF("\nEVENT = %s\n", get_event_string(currentEvent.event));
#endif
                /* Systematically check if status, event and async handle are in max range of the table which represents
                 * the state machine */

                if (currentEvent.idTransportContext < MQTT_MAX_TRANSPORT_CONTEXT &&
                    currentEvent.event < E_MQTT_TRANSPORT_CONTEXT_EVENT_MAX &&
                    pWks->slotsTransportContext[currentEvent.idTransportContext].status <
                        E_MQTT_TRANSPORT_CONTEXT_STATUS_MAX)
                {
#if DEBUG_SCHEDULER == 1
                    SOPC_CONSOLE_PRINTF(
                        "\nCLT %d pre-status = %s\n", currentEvent.idTransportContext,
                        get_status_string(pWks->slotsTransportContext[currentEvent.idTransportContext].status));
#endif
                    /* Save current status, used to check status change after event treatment and raz tick counter */

                    volatile eMqttTransportContextStatus previous_status =
                        pWks->slotsTransportContext[currentEvent.idTransportContext].status;

                    /* Systematically check if a callback exist for current status and current event before invoke it */

                    if (NULL != tabFsmMqttClient[pWks->slotsTransportContext[currentEvent.idTransportContext].status]
                                                [currentEvent.event])
                    {
                        pWks->slotsTransportContext[currentEvent.idTransportContext].status =
                            tabFsmMqttClient[pWks->slotsTransportContext[currentEvent.idTransportContext].status]
                                            [currentEvent.event](
                                                &pWks->slotsTransportContext[currentEvent.idTransportContext],

                                                pWks->slotsTransportContext[currentEvent.idTransportContext].status,
                                                currentEvent.event, currentEvent.pEventData, currentEvent.size);
                    }

                    /* RAZ status timeout if status changes. */

                    if (pWks->slotsTransportContext[currentEvent.idTransportContext].status != previous_status)
                    {
                        pWks->slotsTransportContext[currentEvent.idTransportContext].cptStatusTimeout = 0;
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_TICK == 1
                        SOPC_CONSOLE_PRINTF("\n RAZ CLT %d TMO CPT\n", currentEvent.idTransportContext);
#endif
                    }
#if DEBUG_SCHEDULER == 1
                    SOPC_CONSOLE_PRINTF(
                        "\nCLT %d post-status = %s\n", currentEvent.idTransportContext,
                        get_status_string(pWks->slotsTransportContext[currentEvent.idTransportContext].status));
#endif
                }
            }
            break;
            default:
                /* Unknown state machine */
                break;
            }

            /* Execute clear callback associated to the last event if exists. */

            if (NULL != currentEvent.pClearCallback)
            {
#if DEBUG_SCHEDULER == 1 && DEBUG_LOOP_ALIVE == 1
                SOPC_CONSOLE_PRINTF(
                    "\n================================++ CLEAR CALLBACK ++===================================\n");
#endif
                if (NULL != currentEvent.pEventData)
                {
                    (*currentEvent.pClearCallback)(currentEvent.pEventData);
                }
            }

            /* RAZ last event */

            memset(&currentEvent, 0, sizeof(currentEvent));

            /* Reajust time to wait on the event queue */

            current = SOPC_TimeReference_GetCurrent();
            if (current < ((SOPC_TimeReference) timeToWait + lastTick))
            {
                timeToWait = (uint32_t)(current - lastTick);
            }
            else
            {
                /* Job take too time, RAZ period */

                sendTick = true;
                timeToWait = period;
            }
        }
        else
        {
            /* Timeout occured, elapsed tick period. Reset period to initial value of 100 ms. */

            sendTick = true;
            timeToWait = period;
        }

        if (sendTick == true) /* True on each elapsed period */
        {
            sendTick = false;

            /* Systematically check if status is in max range of the table which represents the state machine */

            if (pWks->status < E_MQTT_MANAGER_STATUS_MAX)
            {
                /* Save current status, used to check status change after event treatment and raz tick counter */

                volatile eMqttManagerStatus previous_status = pWks->status;

                /* Systematically check if a callback exist for current status and current event before invoke it */

                if (NULL != tabFsmScheduler[pWks->status][E_MQTT_MANAGER_EVENT_TICK])
                {
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_TICK == 1
                    SOPC_CONSOLE_PRINTF("\nSCH pre-status = %s - tick = %d\n", get_mgr_status_string(pWks->status),
                                        pWks->cptStatusTimeout);
#endif
                    pWks->status = tabFsmScheduler[pWks->status][E_MQTT_MANAGER_EVENT_TICK](
                        pWks, pWks->status, E_MQTT_MANAGER_EVENT_TICK, 0, NULL, 0);

#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_TICK == 1
                    SOPC_CONSOLE_PRINTF("\nSCH post-status = %s - tick = %d\n", get_mgr_status_string(pWks->status),
                                        pWks->cptStatusTimeout);
#endif
                }

                /* Raz tick counter if status changed */

                if (pWks->status != previous_status)
                {
                    pWks->cptStatusTimeout = 0;

#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_TICK == 1
                    SOPC_CONSOLE_PRINTF("\n RAZ SCHEDULER TMO CPT\n");
#endif
                }
            }

            /* Execute tick on each mqtt transport async context slot.*/
            for (uint16_t i = 0; i < MQTT_MAX_TRANSPORT_CONTEXT; i++)
            {
                /* Systematically check if status is in max range of the table which represents the state machine */
                if (pWks->slotsTransportContext[i].status < E_MQTT_TRANSPORT_CONTEXT_STATUS_MAX)

                {
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_TICK == 1
                    SOPC_CONSOLE_PRINTF("\nCLT %d pre-status = %s - tick = %d\n", i,
                                        get_status_string(pWks->slotsTransportContext[i].status),
                                        pWks->slotsTransportContext[i].cptStatusTimeout);
#endif

                    /* Save current status, used to check status change after event treatment and raz tick counter */
                    volatile eMqttTransportContextStatus previous_status = pWks->slotsTransportContext[i].status;

                    /* Systematically check if a callback exist for current status and current event before invoke it */
                    if (NULL !=
                        tabFsmMqttClient[pWks->slotsTransportContext[i].status][E_MQTT_TRANSPORT_CONTEXT_EVENT_TICK])
                    {
                        pWks->slotsTransportContext[i].status =
                            tabFsmMqttClient[pWks->slotsTransportContext[i].status]
                                            [E_MQTT_TRANSPORT_CONTEXT_EVENT_TICK](
                                                &pWks->slotsTransportContext[i], pWks->slotsTransportContext[i].status,
                                                E_MQTT_TRANSPORT_CONTEXT_EVENT_TICK, NULL, 0);
                    }

                    /* Raz tick counter if status changed */
                    if (pWks->slotsTransportContext[i].status != previous_status)
                    {
                        pWks->slotsTransportContext[i].cptStatusTimeout = 0;
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_TICK == 1
                        SOPC_CONSOLE_PRINTF("\n RAZ CLT %d TMO CPT\n", i);
#endif
                    }
#if DEBUG_SCHEDULER == 1 && DEBUG_TMO_TICK == 1
                    SOPC_CONSOLE_PRINTF("\nCLT %d post-status = %s - tick = %d\n", i,
                                        get_status_string(pWks->slotsTransportContext[i].status),
                                        pWks->slotsTransportContext[i].cptStatusTimeout);
#endif
                }
            }
        }
    }

    return NULL;
}

/*=== Definition of public API ===*/

/*** MQTT Manager API ***/

/* SOPC_MQTT_MGR_Initialize : used by SOPC_MQTT_MGR_Create */

static SOPC_ReturnStatus SOPC_MQTT_MGR_Initialize(MqttManagerHandle* pWks)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    uint16_t index = 0;
    if (NULL == pWks)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    memset(pWks, 0, sizeof(MqttManagerHandle));

    /* Random initialization */

    pWks->random = SOPC_TimeReference_GetCurrent();

    /* Initialization of scheduler event fifo used by mqtt manager*/
    result = EVENT_CHANNEL_init(&pWks->schedulerChannel);
    if (SOPC_STATUS_OK != result)
    {
        memset(pWks, 0, sizeof(MqttManagerHandle));
        return SOPC_STATUS_NOK;
    }

    /* Initialization of static transport context.*/
    for (index = 0; index < MQTT_MAX_TRANSPORT_CONTEXT && result == SOPC_STATUS_OK; index++)
    {
        pWks->slotsTransportContext[index].status = E_MQTT_TRANSPORT_CONTEXT_STATUS_NOT_INITIALIZED;
        pWks->slotsTransportContext[index].identification.transportId = index;
        pWks->slotsTransportContext[index].identification.pMqttMgr = pWks;
        pWks->slotsTransportContext[index].cptRetry = 0;
        pWks->slotsTransportContext[index].cptStatusTimeout = 0;
        pWks->slotsTransportContext[index].clientHandle = NULL;
        memset(&pWks->slotsTransportContext[index].connexionConfig, 0, sizeof(tMqttTransportContextConnexionConfig));
        memset(&pWks->slotsTransportContext[index].callbacksConfig, 0, sizeof(tMqttTransportContextCallbacksConfig));
        pWks->slotsTransportContext[index].pUserContext = NULL;
        result = EVENT_CHANNEL_init(&pWks->slotsTransportContext[index].pendingsMessagesToSend);

#if DEBUG_SCHEDULER == 1 && DEBUG_API == 1
        SOPC_CONSOLE_PRINTF("\nInitialize clt %d\n", index);
#endif
    }

    if (SOPC_STATUS_OK != result)
    {
        for (uint16_t i = 0; i < index; i++)
        {
            EVENT_CHANNEL_deinit(&pWks->slotsTransportContext[i].pendingsMessagesToSend);
        }
        EVENT_CHANNEL_deinit(&pWks->schedulerChannel);
        memset(pWks, 0, sizeof(MqttManagerHandle));
        return SOPC_STATUS_NOK;
    }

    pWks->status = E_MQTT_MANAGER_STATUS_RUNNING;
    pWks->libScheduler = (Thread) NULL;
    result = SOPC_Thread_Create(&pWks->libScheduler, cbTask_MqttManagerScheduler, pWks, "MQTT_MGR");

    if (SOPC_STATUS_OK != result)
    {
        pWks->libScheduler = (Thread) NULL;
        for (uint16_t i = 0; i < MQTT_MAX_TRANSPORT_CONTEXT; i++)
        {
            EVENT_CHANNEL_deinit(&pWks->slotsTransportContext[i].pendingsMessagesToSend);
        }
        EVENT_CHANNEL_deinit(&pWks->schedulerChannel);
        memset(pWks, 0, sizeof(MqttManagerHandle));
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

/* SOPC_MQTT_MGR_Clear : used by SOPC_MQTT_MGR_Destroy */

static SOPC_ReturnStatus SOPC_MQTT_MGR_Clear(MqttManagerHandle* pWks)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == pWks)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_API == 1
        SOPC_CONSOLE_PRINTF("\nError context NULL...\n");
#endif
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (pWks->libScheduler != (Thread) NULL)
    {
        /* Scheduler go to STOPPING */
        result = MGR_EvtQuitScheduler(pWks);

        /* If can't send event, scheduler never go to STOPPING */
        if (result != SOPC_STATUS_OK)
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_API == 1
            SOPC_CONSOLE_PRINTF("\n Critical error on clear, event channel compromised\n");
#endif
            pWks->status = E_MQTT_MANAGER_STATUS_STOPPED;
        }
        result = SOPC_Thread_Join(pWks->libScheduler);
        pWks->libScheduler = (Thread) NULL;
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_API == 1
        SOPC_CONSOLE_PRINTF("\nOups, lib scheduler not started...\n");
#endif
    }

    if (result == SOPC_STATUS_OK)
    {
        EVENT_CHANNEL_deinit(&pWks->schedulerChannel);
        for (uint16_t i = 0; i < MQTT_MAX_TRANSPORT_CONTEXT; i++)
        {
            EVENT_CHANNEL_deinit(&pWks->slotsTransportContext[i].pendingsMessagesToSend);

#if DEBUG_SCHEDULER == 1 && DEBUG_API == 1
            SOPC_CONSOLE_PRINTF("\n **********************************\n");
#endif
            api_lib_destroy(&pWks->slotsTransportContext[i]);

            memset(&pWks->slotsTransportContext[i], 0, sizeof(tMqttTransportContext));
#if DEBUG_SCHEDULER == 1 && DEBUG_API == 1
            SOPC_CONSOLE_PRINTF("\n DeInitialization clients %d\n", i);
#endif
        }

        memset(pWks, 0, sizeof(MqttManagerHandle));
    }

    return SOPC_STATUS_OK;
}

/* SOPC_MQTT_MGR_InitializeGetNewHandleRequest : used by SOPC_MQTT_TRANSPORT_ASYNC_GetHandle
 * Initialize a tMqttGetHandleRequest with callbacks, user context and broker uri. */

static SOPC_ReturnStatus SOPC_MQTT_MGR_InitializeGetNewHandleRequest(tMqttGetHandleRequest* pGetHandleRequest,
                                                                     const char* sUri,
                                                                     const char* sTopicName,
                                                                     FctGetHandleResponse* pCbGetHandleSuccess,
                                                                     FctGetHandleResponse pCbGetHandleFailure,
                                                                     FctClientStatus* pCbClientReady,
                                                                     FctClientStatus* pCbClientNotReady,
                                                                     FctMessageReceived* pCbMessageReceived,
                                                                     FctReleaseHandleResponse* pCbReleaseHandleResponse,
                                                                     void* pUserContext)
{
    if (NULL == pGetHandleRequest || NULL == sUri || NULL == sTopicName || NULL == pCbGetHandleSuccess ||
        NULL == pCbGetHandleFailure || NULL == pCbClientReady || NULL == pCbClientNotReady ||
        NULL == pCbMessageReceived || NULL == pCbReleaseHandleResponse)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

#if DEBUG_SCHEDULER == 1 && DEBUG_API == 1
    SOPC_CONSOLE_PRINTF("\nInitialize handle request with connectionConf\n");
#endif
    pGetHandleRequest->connectionConf.uri[sizeof(pGetHandleRequest->connectionConf.uri) - 1] = 0;
    pGetHandleRequest->connectionConf.topicname[sizeof(pGetHandleRequest->connectionConf.topicname) - 1] = 0;

    snprintf(pGetHandleRequest->connectionConf.uri, sizeof(pGetHandleRequest->connectionConf.uri) - 1, "%s", sUri);

    snprintf(pGetHandleRequest->connectionConf.topicname, sizeof(pGetHandleRequest->connectionConf.topicname) - 1, "%s",
             sTopicName);

    pGetHandleRequest->callbacksConf.pCbGetHandleSuccess = pCbGetHandleSuccess;
    pGetHandleRequest->callbacksConf.pCbGetHandleFailure = pCbGetHandleFailure;
    pGetHandleRequest->callbacksConf.pCbClientReady = pCbClientReady;
    pGetHandleRequest->callbacksConf.pCbClientNotReady = pCbClientNotReady;
    pGetHandleRequest->callbacksConf.pCbMessageReceived = pCbMessageReceived;
    pGetHandleRequest->callbacksConf.pCbReleaseHandleResponse = pCbReleaseHandleResponse;
    pGetHandleRequest->pUserContext = pUserContext;

    return result;
}

/* SOPC_MQTT_MGR_Create : Create a MQTT manager, which is used to manage several MQTT Transport Context*/

SOPC_ReturnStatus SOPC_MQTT_MGR_Create(MqttManagerHandle** ppWks) /* Return a MQTT Manager Handle != NULL.*/
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    if (NULL == ppWks)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    *ppWks = SOPC_Malloc(sizeof(MqttManagerHandle));

    if (NULL == *ppWks)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    result = SOPC_MQTT_MGR_Initialize(*ppWks);
    if (result != SOPC_STATUS_OK)
    {
        SOPC_MQTT_MGR_Destroy(ppWks);
        return SOPC_STATUS_NOK;
    }

    return SOPC_STATUS_OK;
}

/* SOPC_MQTT_MGR_Destroy : Destroy a MQTT manager*/

SOPC_ReturnStatus SOPC_MQTT_MGR_Destroy(MqttManagerHandle** ppWks) /* Return a MQTT Manager Handle set to NULL.*/
{
    if (NULL == ppWks || NULL == *ppWks)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    SOPC_MQTT_MGR_Clear(*ppWks);

    SOPC_Free(*ppWks);
    *ppWks = NULL;

    return SOPC_STATUS_OK;
}

/*** MQTT Transport Asynchrone API ***/

SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_ASYNC_GetHandle(MqttManagerHandle* pWks,
                                                      void* pUserContext,
                                                      const char* uri,
                                                      const char* topicName,
                                                      FctGetHandleResponse* pCbGetHandleSuccess,
                                                      FctGetHandleResponse* pCbGetHandleFailure,
                                                      FctClientStatus* pCbClientReady,
                                                      FctClientStatus* pCbClientNotReady,
                                                      FctMessageReceived* pCbMessageReceived,
                                                      FctReleaseHandleResponse* pCbReleaseHandle)
{
    if (NULL == pWks || NULL == uri || NULL == topicName || NULL == pCbGetHandleSuccess || NULL == pCbClientReady ||
        NULL == pCbMessageReceived || NULL == pCbReleaseHandle || NULL == pCbClientNotReady ||
        NULL == pCbGetHandleFailure)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (E_MQTT_MANAGER_STATUS_RUNNING != pWks->status)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

#if DEBUG_SCHEDULER == 1
    SOPC_CONSOLE_PRINTF("\nAsync get handle entry\n");
#endif

    tMqttEvent event;

    event.event = E_MQTT_MANAGER_EVENT_CMD_NEW_TRANSPORT_CONTEXT;
    event.idFsm = E_MQTT_ID_FSM_MANAGER;
    event.idTransportContext = 0;
    event.pEventData = SOPC_Calloc(1, sizeof(tMqttGetHandleRequest));

    if (NULL == event.pEventData)
    {
        return SOPC_STATUS_OUT_OF_MEMORY;
    }

    /* Get handle request data initialization */
    SOPC_MQTT_MGR_InitializeGetNewHandleRequest((tMqttGetHandleRequest*) event.pEventData, uri, topicName,
                                                pCbGetHandleSuccess, pCbGetHandleFailure, pCbClientReady,
                                                pCbClientNotReady, pCbMessageReceived, pCbReleaseHandle, pUserContext);

    event.size = sizeof(tMqttGetHandleRequest);
    event.pClearCallback = &SOPC_Free;

    result = EVENT_CHANNEL_push(&pWks->schedulerChannel, &event);

    if (SOPC_STATUS_OK != result)
    {
        EVENT_CHANNEL_flush(&pWks->schedulerChannel);
        result = SOPC_STATUS_NOK;
    }

    return result;
}

/* SOPC_MQTT_TRANSPORT_ASYNC_ReleaseHandle : Release transport async handle */

SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_ASYNC_ReleaseHandle(
    MqttManagerHandle* pWks,      /* MQTT Manager Handle */
    MqttTransportAsyncHandle idx) /* MQTT Transport Async Handle */
{
    if (NULL == pWks)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (E_MQTT_MANAGER_STATUS_RUNNING != pWks->status)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    result = MGR_EvtReleaseHdl(pWks, idx);

    return result;
}

SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_ASYNC_SendMessage(MqttManagerHandle* pWks,      /* MQTT Manager Handle */
                                                        MqttTransportAsyncHandle idx, /* MQTT Transport Async Handle */
                                                        uint8_t* bufferData,          /* Data to send */
                                                        uint16_t size) /* Size of data to send in bytes */
{
    if (NULL == pWks)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    if (E_MQTT_MANAGER_STATUS_RUNNING != pWks->status)
    {
        return SOPC_STATUS_INVALID_STATE;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    result = MGR_EvtSendMsg(pWks, idx, bufferData, size);

    return result;
}

/*** MQTT Transport Synchrone API ***/

/*=== Internal asynchrone callbacks declarations. Those callbacks are used by synchrone api, which is implemented on
 * asynchrone API.===*/

/* Called by Mqtt manager on get handle positive response */

static void SYNCH_getHandleCb(MqttTransportAsyncHandle idx, /*Transport context async handle*/
                              void* pCtx); /* User context (used by sync context or Transport context sync handle */

/* Called by Mqtt manager on get handle negative response */

static void SYNCH_getHandleFailedCb(
    MqttTransportAsyncHandle idx, /*Transport context async handle*/
    void* pCtx);                  /* User context (used by sync context or Transport context sync handle */

/* Called by Mqtt manager on transport context async handle ready */

static void SYNCH_clientReadyCb(MqttTransportAsyncHandle idx, /*Transport context async handle*/
                                void* pCtx); /* User context (used by sync context or Transport context sync handle */

/* Called by Mqtt manager on transport context async handle not ready */

static void SYNCH_clientNotReady(MqttTransportAsyncHandle idx, /*Transport context async handle*/
                                 void* pCtx); /* User context (used by sync context or Transport context sync handle */

/* Called by Mqtt manager on transport context async handle message reception */

static void SYNCH_msgReceived(MqttTransportAsyncHandle idx, /*Transport context async handle*/
                              uint8_t* bufferData,          /* Data received */
                              uint16_t dataSize,            /* Data size */
                              void* pCtx); /* User context (used by sync context or Transport context sync handle */

/*=== Internal asynchrone callbacks definition. Those callbacks are used by synchrone api, which is built on
 * asynchrone.===*/

static void SYNCH_getHandleCb(MqttTransportAsyncHandle idx, void* pCtx)
{
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_getHandleCb - idx = %d - pCtx = %08lX\n", idx, (uint64_t) pCtx);
#endif
        MqttTransportHandle* ctx = (MqttTransportHandle*) pCtx;
        Mutex_Lock(&ctx->lock);
        if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE == ctx->transportId)
        {
            ctx->transportId = idx;
            ctx->bReady = false;
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
            SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_getHandleCb - idx = %d - pCtx = %08lX ==> HANDLE SUCCESS\n",
                                idx, (uint64_t) pCtx);
#endif
            Condition_SignalAll(&ctx->signalHandleOperation);
        }
        Mutex_Unlock(&ctx->lock);
    }
}

static void SYNCH_getHandleFailedCb(MqttTransportAsyncHandle idx, void* pCtx)
{
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_getHandleFailedCb - idx = %d - pCtx = %08lX\n", idx,
                            (uint64_t) pCtx);
#endif
        MqttTransportHandle* ctx = (MqttTransportHandle*) pCtx;
        Mutex_Lock(&ctx->lock);
        if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE != ctx->transportId && ctx->transportId == idx)
        {
            ctx->transportId = MQTT_INVALID_TRANSPORT_ASYNC_HANDLE;
            ctx->bReady = false;

#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
            SOPC_CONSOLE_PRINTF(
                "\n SYNCHRO API : SYNCH_getHandleFailedCb - idx = %d - pCtx = %08lX ==> HANDLE FAILURE\n", idx,
                (uint64_t) pCtx);
#endif
            Condition_SignalAll(&ctx->signalHandleOperation);
        }
        Mutex_Unlock(&ctx->lock);
    }
}

static void SYNCH_clientReadyCb(MqttTransportAsyncHandle idx, void* pCtx)
{
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_clientReadyCb - idx = %d - pCtx = %08lX\n", idx, (uint64_t) pCtx);
#endif
        MqttTransportHandle* ctx = (MqttTransportHandle*) pCtx;
        Mutex_Lock(&ctx->lock);
        if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE != ctx->transportId && ctx->transportId == idx)
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
            SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_clientReadyCb - idx = %d - pCtx = %08lX ==> READY\n", idx,
                                (uint64_t) pCtx);
#endif
            ctx->bReady = true;
            Condition_SignalAll(&ctx->signalStatusChange);
        }
        Mutex_Unlock(&ctx->lock);
    }
}

static void SYNCH_clientNotReady(MqttTransportAsyncHandle idx, void* pCtx)
{
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_clientNotReady - idx = %d - pCtx = %08lX\n", idx, (uint64_t) pCtx);
#endif
        MqttTransportHandle* ctx = (MqttTransportHandle*) pCtx;
        Mutex_Lock(&ctx->lock);
        if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE != ctx->transportId && ctx->transportId == idx)
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
            SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_clientNotReady - idx = %d - pCtx = %08lX ==> NOT READY\n", idx,
                                (uint64_t) pCtx);
#endif
            ctx->bReady = false;
            Condition_SignalAll(&ctx->signalStatusChange);
        }
        Mutex_Unlock(&ctx->lock);
    }
}

static void SYNCH_handleReleased(MqttTransportAsyncHandle idx, void* pCtx)
{
    if (NULL != pCtx)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_handleReleased - idx = %d - pCtx = %08lX\n", idx, (uint64_t) pCtx);
#endif
        MqttTransportHandle* ctx = (MqttTransportHandle*) pCtx;
        Mutex_Lock(&ctx->lock);

        if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE != ctx->transportId && ctx->transportId == idx)
        {
            ctx->transportId = MQTT_INVALID_TRANSPORT_ASYNC_HANDLE;
            ctx->bReady = false;
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
            SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_handleReleased - idx = %d - pCtx = %08lX ==> HANDLE RELEASED\n",
                                idx, (uint64_t) pCtx);
#endif
            Condition_SignalAll(&ctx->signalHandleOperation);
        }

        Mutex_Unlock(&ctx->lock);
    }
}

/* SYNCH_msgReceived : Callback of reception of a message */

static void SYNCH_msgReceived(MqttTransportAsyncHandle idx, uint8_t* bufferData, uint16_t dataSize, void* pCtx)
{
    if (NULL != pCtx)
    {
        MqttTransportHandle* ctx = (MqttTransportHandle*) pCtx;
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_msgReceived - idx = %d - pCtx = %08lX\n", idx, (uint64_t) pCtx);
#endif
        Mutex_Lock(&ctx->lock);

#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SYNCH_msgReceived - idx = %d - pCtx = %08lX == > CALL CB\n", idx,
                            (uint64_t) pCtx);
#endif

        if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE != ctx->transportId && ctx->transportId == idx)
        {
            if (NULL != ctx->pCbMessageReceived)
            {
                (*ctx->pCbMessageReceived)(ctx, bufferData, dataSize, ctx->pUserContext);
            }
            else
            {
                SOPC_Buffer* pBuffer = NULL;
                if (ctx->nbMessageReceived >= MQTT_MAX_BUFFER_RCV_MSG)
                {
                    while (ctx->nbMessageReceived > 0)
                    {
                        ctx->nbMessageReceived--;
                        pBuffer = ctx->fifo[ctx->iRd++];
                        if (MQTT_MAX_BUFFER_RCV_MSG == ctx->iRd)
                        {
                            ctx->iRd = 0;
                        }
                        if (NULL != pBuffer)
                        {
                            SOPC_Buffer_Delete(pBuffer);
                            pBuffer = NULL;
                        }
                    }
                    ctx->nbMessageReceived = 0;
                    ctx->iRd = 0;
                    ctx->iWr = 0;
                    memset(ctx->fifo, 0, MQTT_MAX_BUFFER_RCV_MSG * sizeof(SOPC_Buffer*));
                }

                pBuffer = SOPC_Buffer_Create(dataSize);
                if (NULL != pBuffer)
                {
                    SOPC_Buffer_SetPosition(pBuffer, 0);
                    SOPC_Buffer_Write(pBuffer, bufferData, dataSize);

                    ctx->fifo[ctx->iWr++] = pBuffer;
                    if (MQTT_MAX_BUFFER_RCV_MSG == ctx->iWr)
                    {
                        ctx->iWr = 0;
                    }
                    ctx->nbMessageReceived++;

#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_CALLBACKS == 1
                    SOPC_CONSOLE_PRINTF(
                        "\n SYNCHRO API : SYNCH_msgReceived - idx = %d - pCtx = %08lX == > MSG PUSHED\n", idx,
                        (uint64_t) pCtx);
#endif

                    if (1 == ctx->nbMessageReceived)
                    {
                        Condition_SignalAll(&ctx->signalNewMessage);
                    }
                }
            }
        }

        Mutex_Unlock(&ctx->lock);
    }
}

/*=== Definition of synchrone API ===*/

SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_SYNCH_ReleaseHandle(
    MqttTransportHandle** ppCtx) /* Transport synchrone handle to release.*/
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

    if (NULL == ppCtx || NULL == *ppCtx || NULL == (*ppCtx)->pMgrCtx)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    MqttTransportHandle* pCtx = *ppCtx;
    MqttManagerHandle* pWks = pCtx->pMgrCtx;

#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
    SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReleaseHandle [%08lX] : Entry\n", (uint64_t) pCtx);
#endif

    Mutex_Lock(&pCtx->lock);

    if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE != pCtx->transportId)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReleaseHandle [%08lX] : Send request\n", (uint64_t) pCtx);
#endif
        result = SOPC_MQTT_TRANSPORT_ASYNC_ReleaseHandle(pWks, pCtx->transportId);

        if (SOPC_STATUS_OK == result)
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
            SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReleaseHandle [%08lX] : Wait result\n", (uint64_t) pCtx);
#endif
            Mutex_UnlockAndWaitCond(&pCtx->signalHandleOperation, &pCtx->lock);
        }
        else
        {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
            SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReleaseHandle [%08lX] : Send request FAILED\n",
                                (uint64_t) pCtx);
#endif
        }

        pCtx->transportId = MQTT_INVALID_TRANSPORT_ASYNC_HANDLE;
    }
    Mutex_Unlock(&pCtx->lock);

#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
    SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReleaseHandle [%08lX] : Exit\n", (uint64_t) pCtx);
#endif

    /* Flush messages not read */

    SOPC_Buffer* pBuffer = NULL;
    while (pCtx->nbMessageReceived > 0)
    {
        pCtx->nbMessageReceived--;
        pBuffer = pCtx->fifo[pCtx->iRd++];
        if (MQTT_MAX_BUFFER_RCV_MSG == pCtx->iRd)
        {
            pCtx->iRd = 0;
        }
        if (pBuffer != NULL)
        {
            SOPC_Buffer_Delete(pBuffer);
            pBuffer = NULL;
        }
    }
    pCtx->nbMessageReceived = 0;
    pCtx->iRd = 0;
    pCtx->iWr = 0;
    memset(pCtx->fifo, 0, MQTT_MAX_BUFFER_RCV_MSG * sizeof(SOPC_Buffer*));

    // Clear synchronisation objects

    Mutex_Clear(&pCtx->lock);
    Condition_Clear(&pCtx->signalHandleOperation);
    Condition_Clear(&pCtx->signalStatusChange);
    Condition_Clear(&pCtx->signalNewMessage);
    SOPC_Free(pCtx);
    *ppCtx = NULL;

    return result;
}

SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_SYNCH_ReadMessage(MqttTransportHandle* pCtx,
                                                        SOPC_Buffer** ppBuffer,
                                                        uint32_t timeoutMs)
{
    SOPC_Buffer* pBuffer = NULL;
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
    SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReadMessage [%08lX] : Entry\n", (uint64_t) pCtx);
#endif

    if (NULL == pCtx || NULL == pCtx->pMgrCtx || NULL == ppBuffer)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    Mutex_Lock(&pCtx->lock);

    if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE == pCtx->transportId)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReadMessage [%08lX] : Invalid state\n", (uint64_t) pCtx);
#endif
        Mutex_Unlock(&pCtx->lock);
        return SOPC_STATUS_INVALID_STATE;
    }

    if (0 == pCtx->nbMessageReceived && timeoutMs > 0)
    {
        Mutex_UnlockAndTimedWaitCond(&pCtx->signalNewMessage, &pCtx->lock, timeoutMs);
    }

    if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE == pCtx->transportId)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReadMessage [%08lX] : Invalid state\n", (uint64_t) pCtx);
#endif
        Mutex_Unlock(&pCtx->lock);
        return SOPC_STATUS_INVALID_STATE;
    }

    if (pCtx->nbMessageReceived > 0)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReadMessage [%08lX] : Message read OK\n", (uint64_t) pCtx);
#endif
        pBuffer = pCtx->fifo[pCtx->iRd];
        pCtx->fifo[pCtx->iRd] = NULL;
        pCtx->iRd++;
        pCtx->nbMessageReceived--;
        if (pCtx->iRd >= MQTT_MAX_BUFFER_RCV_MSG)
        {
            pCtx->iRd = 0;
        }
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_ReadMessage [%08lX] : Message read TMO\n", (uint64_t) pCtx);
#endif
    }

    Mutex_Unlock(&pCtx->lock);
    *ppBuffer = pBuffer;
    return SOPC_STATUS_OK;
}

SOPC_ReturnStatus SOPC_MQTT_TRANSPORT_SYNCH_SendMessage(MqttTransportHandle* pCtx,
                                                        uint8_t* bufferData,
                                                        uint16_t dataSize,
                                                        uint32_t timeoutMs)
{
    SOPC_ReturnStatus result = SOPC_STATUS_OK;

#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
    SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_SendMessage [%08lX] : Entry\n", (uint64_t) pCtx);
#endif

    if (NULL == pCtx || NULL == pCtx->pMgrCtx || NULL == bufferData || 0 == dataSize)
    {
        return SOPC_STATUS_INVALID_PARAMETERS;
    }

    MqttManagerHandle* pWks = pCtx->pMgrCtx;

    Mutex_Lock(&pCtx->lock);

    if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE == pCtx->transportId)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_SendMessage [%08lX] : Invalid state\n", (uint64_t) pCtx);
#endif
        Mutex_Unlock(&pCtx->lock);
        return SOPC_STATUS_INVALID_STATE;
    }

    if (false == pCtx->bReady && timeoutMs > 0)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_SendMessage [%08lX] : Not ready, wait for %d ms\n",
                            (uint64_t) pCtx, timeoutMs);
#endif
        Mutex_UnlockAndTimedWaitCond(&pCtx->signalStatusChange, &pCtx->lock, timeoutMs);
    }

    if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE == pCtx->transportId)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_SendMessage [%08lX] : Invalid state\n", (uint64_t) pCtx);
#endif
        Mutex_Unlock(&pCtx->lock);
        return SOPC_STATUS_INVALID_STATE;
    }

    if (true == pCtx->bReady)
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_SendMessage [%08lX] : Ready state, send message\n",
                            (uint64_t) pCtx);
#endif
        result = SOPC_MQTT_TRANSPORT_ASYNC_SendMessage(pWks, pCtx->transportId, bufferData, dataSize);
    }
    else
    {
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_SendMessage [%08lX] : Not ready state, message not sent\n",
                            (uint64_t) pCtx);
#endif
        result = SOPC_STATUS_NOK;
    }

#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
    SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_SendMessage [%08lX] : Exit\n", (uint64_t) pCtx);
#endif

    Mutex_Unlock(&pCtx->lock);

    return result;
}

MqttTransportHandle* SOPC_MQTT_TRANSPORT_SYNCH_GetHandle(MqttManagerHandle* pWks,
                                                         const char* uri,
                                                         const char* topicName,
                                                         FctMessageSyncReceived* pCbMessageReceived,
                                                         void* pUserContext)
{
    if (NULL == pWks || NULL == uri)
    {
        return NULL;
    }

    SOPC_ReturnStatus result = SOPC_STATUS_OK;
    MqttTransportHandle* pCtx = SOPC_Calloc(1, sizeof(MqttTransportHandle));

    if (NULL == pCtx)
    {
        return NULL;
    }

    pCtx->pMgrCtx = pWks;

    result = Mutex_Initialization(&pCtx->lock);

    if (SOPC_STATUS_OK != result)
    {
        SOPC_Free(pCtx);
        return NULL;
    }

    result = Condition_Init(&pCtx->signalHandleOperation);

    if (SOPC_STATUS_OK != result)
    {
        Mutex_Clear(&pCtx->lock);
        SOPC_Free(pCtx);
        pCtx = NULL;
        return NULL;
    }

    result = Condition_Init(&pCtx->signalStatusChange);

    if (SOPC_STATUS_OK != result)
    {
        Mutex_Clear(&pCtx->lock);
        Condition_Clear(&pCtx->signalHandleOperation);
        SOPC_Free(pCtx);
        pCtx = NULL;
        return NULL;
    }

    result = Condition_Init(&pCtx->signalNewMessage);

    if (SOPC_STATUS_OK != result)
    {
        Mutex_Clear(&pCtx->lock);
        Condition_Clear(&pCtx->signalHandleOperation);
        Condition_Clear(&pCtx->signalStatusChange);
        SOPC_Free(pCtx);
        pCtx = NULL;
        return NULL;
    }

    pCtx->transportId = MQTT_INVALID_TRANSPORT_ASYNC_HANDLE;
    pCtx->pCbMessageReceived = pCbMessageReceived;
    pCtx->pUserContext = pUserContext;
    memset(pCtx->fifo, 0, MQTT_MAX_BUFFER_RCV_MSG * sizeof(SOPC_Buffer*));
    pCtx->iRd = 0;
    pCtx->iWr = 0;
    pCtx->nbMessageReceived = 0;

    Mutex_Lock(&pCtx->lock);

    result = SOPC_MQTT_TRANSPORT_ASYNC_GetHandle(pWks, pCtx, uri, topicName, SYNCH_getHandleCb, SYNCH_getHandleFailedCb,
                                                 SYNCH_clientReadyCb, SYNCH_clientNotReady, SYNCH_msgReceived,
                                                 SYNCH_handleReleased);

    if (SOPC_STATUS_OK != result)
    {
        Mutex_Unlock(&pCtx->lock);
        Mutex_Clear(&pCtx->lock);
        Condition_Clear(&pCtx->signalHandleOperation);
        Condition_Clear(&pCtx->signalStatusChange);
        Condition_Clear(&pCtx->signalNewMessage);
        SOPC_Free(pCtx);
        pCtx = NULL;
        return NULL;
    }

#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
    SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_GetHandle [%08lX] : Ready to wait for result\n", (uint64_t) pCtx);
#endif

    Mutex_UnlockAndWaitCond(&pCtx->signalHandleOperation, &pCtx->lock);

    if (MQTT_INVALID_TRANSPORT_ASYNC_HANDLE == pCtx->transportId)
    {
        Mutex_Unlock(&pCtx->lock);
        Mutex_Clear(&pCtx->lock);
        Condition_Clear(&pCtx->signalHandleOperation);
        Condition_Clear(&pCtx->signalStatusChange);
        Condition_Clear(&pCtx->signalNewMessage);
        SOPC_Free(pCtx);
        pCtx = NULL;
#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
        SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_GetHandle [%08lX] : Failure\n", (uint64_t) pCtx);
#endif
        return pCtx;
    }

#if DEBUG_SCHEDULER == 1 && DEBUG_SYNCHRO_API == 1
    SOPC_CONSOLE_PRINTF("\n SYNCHRO API : SOPC_MQTT_GetHandle [%08lX] : Success\n", (uint64_t) pCtx);
#endif

    Mutex_Unlock(&pCtx->lock);

    return pCtx;
}
