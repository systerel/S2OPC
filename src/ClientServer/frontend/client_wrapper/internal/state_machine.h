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

/** \file
 *
 * \brief The state machine of the subscribing client.
 *
 * The machine handles:
 * - Toolkit configuration,
 * - SecureChannel and Session creation,
 * - Creation of the Subscription,
 * - Creation of the MonitoredItems,
 * - A number of PublishRequest.
 *
 * The machine starts in the init state. It shall be configured with a call to SOPC_StaMac_ConfigureMachine(),
 * which configures the Toolkit. TheSOPC_StaMac_EventDispatcher() shall be called on the state machine from the callback
 * given to ::SOPC_Toolkit_Initialize.
 *
 * The machine API is thread safe.
 *
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

/* Machine static parameters, MonitoredItem parameters */
#define MONIT_TIMESTAMPS_TO_RETURN OpcUa_TimestampsToReturn_Both
#define MONIT_QSIZE 10

#include <stdbool.h>

/* The following includes are required to fetch the SOPC_LibSub_DataChangeCbk type */
#include "sopc_builtintypes.h"
#include "sopc_crypto_profiles.h"
#include "sopc_log_manager.h"
#include "sopc_types.h"
#include "sopc_user_app_itf.h"
#define SKIP_S2OPC_DEFINITIONS
#include "libs2opc_client.h"
#include "libs2opc_client_cmds.h"

/* Machine states */
typedef enum
{
    stError, /* stError is both error and closed state */
    stInit,
    stActivating,
    stActivated,
    stCreatingSubscr,
    stCreatingMonIt,
    stDeletingMonIt,
    stDeletingSubscr,
    stClosing
} SOPC_StaMac_State;

/* Request types */
typedef enum
{
    SOPC_REQUEST_TYPE_UNKNOWN = 0,  /* Unknown Request */
    SOPC_REQUEST_TYPE_USER,         /* User Crafted Request */
    SOPC_REQUEST_TYPE_PUBLISH,      /* PublishRequest */
    SOPC_REQUEST_TYPE_SUBSCRIPTION, /* Other internally managed Subscription related request */
    SOPC_REQUEST_TYPE_GET_ENDPOINTS /* GetEndpointsRequest */
} SOPC_StaMac_RequestType;

/* Request scopes */
typedef enum
{
    SOPC_REQUEST_SCOPE_STATE_MACHINE, /** The request is part of the inner working of the state machine */
    SOPC_REQUEST_SCOPE_APPLICATION,   /** The request is issued by the applicative layer and the response
                                       *  will be forwarded to the generic event callback */
    SOPC_REQUEST_SCOPE_DISCOVERY      /** The request is a discovery request and shall not be
                                       *  processed by a StaMac */
} SOPC_StaMac_RequestScope;

/* Machine content is private to the implementation */
typedef struct SOPC_StaMac_Machine SOPC_StaMac_Machine;

typedef struct
{
    SOPC_StaMac_Machine* pSM;              /* The state machine that issued the request */
    uintptr_t appCtx;                      /* Application context, chosen outside of the state machine */
    SOPC_StaMac_RequestScope requestScope; /* Whether the request is started by the state machine or the applicative */
    SOPC_StaMac_RequestType requestType;   /* the type of request */
} SOPC_StaMac_ReqCtx;

/* Machine lifecycle */

/**
 * \brief Creates a new state machine, initialized in state stInit.
 *
 * \param iscConfig               The configuration identifier to use with this machine
 * \param reverseConfigIdx        If the reverse connection mechanism shall be used,
 *                                it shall be set with index of the reverse endpoint configuration returned by
 *                                ::SOPC_ClientCommon_CreateReverseEndpoint.
 *                                Otherwise it shall be set to 0.
 * \param iCliId                  The client id of the machine, it shall be unique.
 * \param szPolicyId              Zero-terminated user identity policy id, see SOPC_LibSub_ConnectionCfg
 * \param szUsername              Zero-terminated username, see SOPC_LibSub_ConnectionCfg
 * \param szPassword              Zero-terminated password, see SOPC_LibSub_ConnectionCfg
 * \param pUserCertX509           X509 serialized certificate for X509IdentiyToken (DER format)
 * \param pUserKey                Serialized private key for X509IdentiyToken (PEM format)
 * \param cbkLibSubDataChanged    The callback to trigger when a PublishResponse is received
 * \param fPublishInterval        Subscription publish interval, in milliseconds
 * \param iCntMaxKeepAlive        The number of times an empty PublishResponse is not sent
 * \param iCntLifetime            The number of times a PublishResponse cannot be sent
 *                                before killing the subscription
 * \param iTokenTarget            Number of subscription tokens the server should always have
 * \param iTimeoutMs              Timeout for the synchroneous calls
 * \param cbkGenericEvent         Callback for generic responses to a call to SOPC_LibSub_AsyncSendRequestOnSession()
 * \param userContext             Caller defined user context that could be retrieved or set using accessors
 * \param ppSM                    The returned machine, when successful
 *
 * \return SOPC_STATUS_OK when \p ppSM points to a pointer to a valid machine.
 *         In other cases, (*ppSM) is not modified.
 */
SOPC_ReturnStatus SOPC_StaMac_Create(uint32_t iscConfig,
                                     SOPC_ReverseEndpointConfigIdx reverseConfigIdx,
                                     uint32_t iCliId,
                                     const char* szPolicyId,
                                     const char* szUsername,
                                     const char* szPassword,
                                     const SOPC_SerializedCertificate* pUserCertX509,
                                     const SOPC_SerializedAsymmetricKey* pUserKey,
                                     SOPC_LibSub_DataChangeCbk* cbkLibSubDataChanged,
                                     double fPublishInterval,
                                     uint32_t iCntMaxKeepAlive,
                                     uint32_t iCntLifetime,
                                     uint32_t iTokenTarget,
                                     int64_t iTimeoutMs,
                                     SOPC_LibSub_EventCbk* cbkGenericEvent,
                                     uintptr_t userContext,
                                     SOPC_StaMac_Machine** ppSM);

/*
 * \brief Changes the callback for data change notifications on subscription
 */
SOPC_ReturnStatus SOPC_StaMac_ConfigureDataChangeCallback(SOPC_StaMac_Machine* pSM,
                                                          SOPC_ClientHelper_DataChangeCbk* pCbkClientHelper);

/**
 * \brief Deletes and deallocate the machine.
 */
void SOPC_StaMac_Delete(SOPC_StaMac_Machine** ppSM);

/**
 * \brief Creates a session asynchronously.
 *
 * The state machine will also create a subscription. See SOPC_StaMac_HasSubscription().
 *
 * See SOPC_ToolkitClient_AsyncActivateSession().
 * You shall call SOPC_StaMac_StopSession() to close the connection gracefully.
 */
SOPC_ReturnStatus SOPC_StaMac_StartSession(SOPC_StaMac_Machine* pSM);

/**
 * \brief Closes the session.
 *
 * If not SOPC_StaMac_IsConnected(), the machine is put in state stError.
 */
SOPC_ReturnStatus SOPC_StaMac_StopSession(SOPC_StaMac_Machine* pSM);

/**
 * \brief Sends a request, wraps SOPC_ToolkitClient_AsyncSendRequestOnSession().
 *
 * The machine must be activated.
 *
 * \warning Every client request should be sent with this wrapper, so that the machine
 *          can recognize the response from the server.
 *
 * \param pSM           The state machine used to send request
 * \param requestStruct The structure of the request, see SOPC_ToolkitClient_AsyncSendRequestOnSession()
 * \param appCtx        An ID that will be given back through the call to the event handler.
 *                      The value 0 indicates "no ID".
 * \param requestScope  scope of the request (state machine or application)
 * \param requestType   type of the request
 */
SOPC_ReturnStatus SOPC_StaMac_SendRequest(SOPC_StaMac_Machine* pSM,
                                          void* requestStruct,
                                          uintptr_t appCtx,
                                          SOPC_StaMac_RequestScope requestScope,
                                          SOPC_StaMac_RequestType requestType);

/*
 * \brief Create subscription associated to the given state machine
 */
SOPC_ReturnStatus SOPC_StaMac_CreateSubscription(SOPC_StaMac_Machine* pSM);

/*
 * \brief Create subscription associated to the given state machine using the subscription request (new API)
 */
SOPC_ReturnStatus SOPC_StaMac_NewCreateSubscription(SOPC_StaMac_Machine* pSM,
                                                    OpcUa_CreateSubscriptionRequest* req,
                                                    uintptr_t userAppContext);

/**
 * \brief return the context provided in ::SOPC_StaMac_NewCreateSubscription
 */
uintptr_t SOPC_StaMac_GetSubscriptionCtx(SOPC_StaMac_Machine* pSM);

/*
 * \brief Retrieve the subscription parameters revised by the server (for non-NULL params only)
 */
SOPC_ReturnStatus SOPC_StaMac_GetSubscriptionRevisedParams(SOPC_StaMac_Machine* pSM,
                                                           double* revisedPublishingInterval,
                                                           uint32_t* revisedLifetimeCount,
                                                           uint32_t* revisedMaxKeepAliveCount);

/*
 * \brief Set the number of tokens for the subscription associated to the given state machine
 */
SOPC_ReturnStatus SOPC_StaMac_SetSubscriptionNbTokens(SOPC_StaMac_Machine* pSM, uint32_t nbTokens);

/**
 * \brief Delete subscription associated to the given state machine
 */
SOPC_ReturnStatus SOPC_StaMac_DeleteSubscription(SOPC_StaMac_Machine* pSM);

/**
 * \brief Context structure to be provided when using ::SOPC_StaMac_CreateMonitoredItem or
 * ::SOPC_StaMac_NewCreateMonitoredItems
 */
typedef struct SOPC_CreateMonitoredItems_Ctx
{
    OpcUa_CreateMonitoredItemsResponse* Results; /* It shall be allocated if response expected
                                                    or might be NULL if not necessary for application. */
    uintptr_t outCtxId; /* Contains unique identifier filled by ::SOPC_StaMac_CreateMonitoredItem or
                           ::SOPC_StaMac_NewCreateMonitoredItems*/
    OpcUa_CreateMonitoredItemsRequest*
        req; /* Contains the request sent to the server (internal use only, freed during treatment)*/
} SOPC_CreateMonitoredItems_Ctx;

/**
 * \brief Creates a MonitoredItem asynchronously.
 *
 * The \p pAppCtx shall be used to test the effective creation of the MonitoredItem with
 * ::SOPC_StaMac_PopMonItByAppCtx.
 *
 * \param pSM        The state machine with a subscription used to create monitored items
 * \param lszNodeId  An array of describing the NodeIds to add.
                     It should be at least \p nElems long.
 * \param liAttrId   An array of attributes id. The subscription is created for the attribute lAttrId[i]
 *                   for the node id lszNodeId[i]. It should be at least \p nElems long.
 * \param nElems     The number of elements in previous arrays.
 * \param pAppCtx    The create monitored item application context filled during the call and
 *                   that shall be used to call ::SOPC_StaMac_PopMonItByAppCtx
 * \param lCliHndl   An array of client handles to be filled.
 *
 * \warning The szNodeId must be \0-terminated.
 */
SOPC_ReturnStatus SOPC_StaMac_CreateMonitoredItem(SOPC_StaMac_Machine* pSM,
                                                  char const* const* lszNodeId,
                                                  const uint32_t* liAttrId,
                                                  int32_t nElems,
                                                  SOPC_CreateMonitoredItems_Ctx* pAppCtx,
                                                  uint32_t* lCliHndl);

/**
 * \brief Type of callback called on Subscription Notification
 *
 * \param subscriptionAppCtx    The user application context provided in ::SOPC_StaMac_NewCreateSubscription call
 * \param status                OPC UA status code, \p notification is only valid when ::SOPC_IsGoodStatus
 * \param notificationType      Type of notification received (::OpcUa_DataChangeNotification_EncodeableType or
 *                              ::OpcUa_EventNotificationList_EncodeableType)
 * \param nbNotifElts           Number of elements in \p notification received and in \p monitoredItemCtxArray
 * \param notification          Notification of the type indicated by \p notificationType,
 *                              either pointer to a ::OpcUa_DataChangeNotification or
 *                              ::OpcUa_EventNotificationList.
 *                              Content is freed after callback return, thus any content to record shall be copied.
 * \param monitoredItemCtxArray Array of context for monitored items for which notification were received in
 *                              \p notification.
 *                              Notification element and monitored item context have the same index in the array.
 *
 */
typedef void SOPC_StaMacNotification_Fct(uintptr_t subscriptionAppCtx,
                                         SOPC_StatusCode status,
                                         SOPC_EncodeableType* notificationType,
                                         uint32_t nbNotifElts,
                                         const void* notification,
                                         uintptr_t* monitoredItemCtxArray);
/*
 * \brief Changes the callback for data change notifications on subscription
 */
SOPC_ReturnStatus SOPC_StaMac_NewConfigureNotificationCallback(SOPC_StaMac_Machine* pSM,
                                                               SOPC_StaMacNotification_Fct* pNotificationCb);

/**
 * \brief Creates new monitored items for the state machine subscription using the request (new API)
 *
 * The \p pAppCtx shall be used to test the effective creation of the MonitoredItem with
 * ::SOPC_StaMac_PopMonItByAppCtx.
 *
 * \param pSM              The state machine with a subscription used to create monitored items
 * \param req              The create monitored items request
 * \param userAppCtxArray  An array of user context defined with the same indexes as the monitored items in \p req.
 *                         The context will be provided in the callback defined by
 *                         when a notification occurs on a monitored item.
 * \param pAppCtx          The create monitored item application context is stored in the pointed structure and
 *                         could be used to call ::SOPC_StaMac_PopMonItByAppCtx
 */
SOPC_ReturnStatus SOPC_StaMac_NewCreateMonitoredItems(SOPC_StaMac_Machine* pSM,
                                                      OpcUa_CreateMonitoredItemsRequest* req,
                                                      const uintptr_t* userAppCtxArray,
                                                      SOPC_CreateMonitoredItems_Ctx* pAppCtx);

/**
 * \brief Context structure to be provided when using ::SOPC_StaMac_NewDeleteMonitoredItems
 */
typedef struct SOPC_DeleteMonitoredItems_Ctx
{
    OpcUa_DeleteMonitoredItemsResponse* Results; /* It shall be allocated if response expected
                                                    or might be NULL if not necessary for application. */
    uintptr_t outCtxId; /* Contains unique identifier filled by ::SOPC_StaMac_NewDeleteMonitoredItems */
    OpcUa_DeleteMonitoredItemsRequest*
        req; /* Contains the request sent to the server (internal use only, freed during treatment)*/
} SOPC_DeleteMonitoredItems_Ctx;

SOPC_ReturnStatus SOPC_StaMac_NewDeleteMonitoredItems(SOPC_StaMac_Machine* pSM,
                                                      OpcUa_DeleteMonitoredItemsRequest* req,
                                                      SOPC_DeleteMonitoredItems_Ctx* outAppCtx);

/**
 * \brief Returns a bool whether the machine is configured and ready for a new SecureChannel.
 *
 * Note: for now, it is the stInit state.
 */
bool SOPC_StaMac_IsConnectable(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is in a connected state or not.
 *
 * Connected states are: stActivating, stActivated, stCreating*, stClosing.
 */
bool SOPC_StaMac_IsConnected(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns a bool whether the machine is in stError or not.
 */
bool SOPC_StaMac_IsError(SOPC_StaMac_Machine* pSM);

/**
 * \brief Put the state machine in error state (without closing). This avoids additional notifications.
 */
void SOPC_StaMac_SetError(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns a bool whether the machine has an active subscription or not.
 */
bool SOPC_StaMac_HasSubscription(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns subscription Id whether the machine has an active subscription or not (id = 0).
 */
uint32_t SOPC_StaMac_HasSubscriptionId(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns whether the machine has created the MonitoredItems with the given \p appCtx or not.
 *        When it returns true, it is considered acknowledged and the application context is removed from state machine.
 */
bool SOPC_StaMac_PopMonItByAppCtx(SOPC_StaMac_Machine* pSM, SOPC_CreateMonitoredItems_Ctx* pAppCtx);

/**
 * \brief Returns whether the machine has deleted the MonitoredItems with the given \p appCtx or not.
 *        When it returns true, it is considered acknowledged and the application context is removed from state machine.
 */
bool SOPC_StaMac_PopDeleteMonItByAppCtx(SOPC_StaMac_Machine* pSM, SOPC_DeleteMonitoredItems_Ctx* pAppCtx);

/** \brief Returns the timeout of the machine, used for the synchronous calls. */
int64_t SOPC_StaMac_GetTimeout(SOPC_StaMac_Machine* pSM);

/**
 * \brief Returns the user context provided to SOPC_StaMac_Create or modified through SOPC_StaMac_SetUserContext
 */
uintptr_t SOPC_StaMac_GetUserContext(SOPC_StaMac_Machine* pSM);

/**
 * \brief Overwrite the user content in the given state machine
 */
void SOPC_StaMac_SetUserContext(SOPC_StaMac_Machine* pSM, uintptr_t userContext);

/**
 * \brief Handles the events from the Toolkit and changes the state machine state.
 *
 * This function can be called even if the message is not destined to this particular machine.
 * If the event is a response from a request issued with the SOPC_REQUEST_SCOPE_APPLICATION,
 * the generic event callback of the machine is called.
 *
 * \param pSM       The state machine for which event should be handled
 * \param pAppCtx   A pointer to an uintptr_t which will contain the appCtx given to
 *                  SOPC_StaMac_SendRequest(). NULL is valid. 0 indicates "appID unavailable".
 * \param event     The event to be handled in the state machine
 * \param arg       The first parameter (id or status) associated with the event
 * \param pParam    The second parameter associated with the event
 * \param appCtx    The appCtx given by the Toolkit.
 *
 * \warning This function shall be called upon each event raised by the Toolkit.
 *
 * \return True if the event is targeted to the machine (including all request scopes).
 *         False otherwise, or when pSM is NULL.
 */
bool SOPC_StaMac_EventDispatcher(SOPC_StaMac_Machine* pSM,
                                 uintptr_t* pAppCtx,
                                 SOPC_App_Com_Event event,
                                 uint32_t arg,
                                 void* pParam,
                                 uintptr_t appCtx);

#endif /* STATE_MACHINE_H_ */
