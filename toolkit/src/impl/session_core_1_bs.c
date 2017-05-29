/*
 *  Copyright (C) 2017 Systerel and others.
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

/*------------------------
   Exported Declarations
  ------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sopc_builtintypes.h"
#include "sopc_encodeable.h"

#include "session_core_1_bs.h"

#include "internal_channel_endpoint.h"

typedef struct session {
  constants__t_channel_i session_core_1_bs__channel;
  constants__t_sessionState session_core_1_bs__state;
  constants__t_session_token_i session_core_1_bs__session_token;
  constants__t_user_i session_core_1_bs__user;
  // Only one request handle managed by session in this version
  constants__t_request_handle_i session_core_1_bs__req_handle;
  t_bool session_core_1_bs__req_handle_used;
} session;

static session unique_session;
static t_bool unique_session_init = FALSE;

static int token_cpt = 0;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void session_core_1_bs__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void session_core_1_bs__get_session_from_req_handle(
   const constants__t_request_handle_i session_core_1_bs__req_handle,
   constants__t_session_i * const session_core_1_bs__session) {
  if((!FALSE) == unique_session_init &&
     unique_session.session_core_1_bs__req_handle == session_core_1_bs__req_handle){
    *session_core_1_bs__session = &unique_session;
  }else{
    *session_core_1_bs__session = constants__c_session_indet;
  }
}

void session_core_1_bs__get_session_from_token(
   const constants__t_session_token_i session_core_1_bs__session_token,
   constants__t_session_i * const session_core_1_bs__session) {
  int32_t comparison = 0;
  if((!FALSE) == unique_session_init){
    SOPC_StatusCode status = SOPC_NodeId_Compare((SOPC_NodeId*)session_core_1_bs__session_token, 
                                                 (SOPC_NodeId*)unique_session.session_core_1_bs__session_token,
                                                 &comparison);
    if(STATUS_OK == status && comparison == 0){
      *session_core_1_bs__session = &unique_session;
    }
  }
}

void session_core_1_bs__get_token_from_session(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_session_token_i * const session_core_1_bs__session_token) {
  if(&unique_session == session_core_1_bs__session){
    *session_core_1_bs__session_token = unique_session.session_core_1_bs__session_token;    
  }else{
    *session_core_1_bs__session_token = constants__c_session_token_indet;    
  }
}

void session_core_1_bs__get_fresh_session_token(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_session_token_i * const session_core_1_bs__token) {
  if(token_cpt + 1 > 0 &&
     &unique_session == session_core_1_bs__session &&
     unique_session.session_core_1_bs__session_token == constants__c_session_token_indet){
    SOPC_NodeId* token = malloc(sizeof(SOPC_NodeId));
    if(NULL != token){
      SOPC_NodeId_Initialize(token);
      token_cpt++;
      token->IdentifierType = IdentifierType_Numeric;
      token->Data.Numeric = token_cpt;
      *session_core_1_bs__token = token;
      unique_session.session_core_1_bs__session_token = token;
    }else{
      *session_core_1_bs__token = constants__c_session_token_indet;
    }
  }else{
    *session_core_1_bs__token = constants__c_session_token_indet;    
  } 
}

void session_core_1_bs__is_fresh_session_token(
   const constants__t_session_token_i session_core_1_bs__session_token,
   t_bool * const session_core_1_bs__ret) {
  if((!FALSE) == unique_session_init){
    if(session_core_1_bs__session_token != constants__c_session_token_indet &&
       session_core_1_bs__session_token != unique_session.session_core_1_bs__session_token){
      *session_core_1_bs__ret = (!FALSE);
    }else{
      *session_core_1_bs__ret = FALSE;
    }
  }else{
    *session_core_1_bs__ret = (!FALSE);
  }
}

void session_core_1_bs__is_valid_session_token(
   const constants__t_session_token_i session_core_1_bs__token,
   t_bool * const session_core_1_bs__ret) {
  int32_t comparison = 0;
  if(session_core_1_bs__token != constants__c_session_token_indet){
    SOPC_StatusCode status = SOPC_NodeId_Compare((SOPC_NodeId*)session_core_1_bs__token, 
                                                 (SOPC_NodeId*)unique_session.session_core_1_bs__session_token,
                                                 &comparison);
    if(STATUS_OK == status && comparison == 0){
      *session_core_1_bs__ret = (!FALSE);
    }else{
      *session_core_1_bs__ret = FALSE;
    }
  }else{
    *session_core_1_bs__ret = FALSE;
  }
}

void session_core_1_bs__has_session_token(
   const constants__t_session_i session_core_1_bs__session,
   t_bool * const session_core_1_bs__ret) {
  if(session_core_1_bs__session == &unique_session &&
     constants__c_session_token_indet != unique_session.session_core_1_bs__session_token){
    *session_core_1_bs__ret = (!FALSE);
  }else{
    *session_core_1_bs__ret = FALSE;
  }
}

void session_core_1_bs__set_session_token(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_session_token_i session_core_1_bs__token) {
  if(&unique_session == session_core_1_bs__session){
    if(session_core_1_bs__token != unique_session.session_core_1_bs__session_token){
      SOPC_NodeId* token = malloc(sizeof(SOPC_NodeId));
      if(NULL != token){
        SOPC_NodeId_Initialize(token);
        if(STATUS_OK == SOPC_NodeId_Copy(token, (SOPC_NodeId*) session_core_1_bs__token)){        
          unique_session.session_core_1_bs__session_token = token;
        }else{
          printf("session_core_1_bs__set_session_token\n");
          exit(1);         
        }
      }else{
        printf("session_core_1_bs__set_session_token\n");
        exit(1);
      }
    }else{
      // guarantee by B code
      printf("session_core_1_bs__set_session_token\n");
      exit(1);
    }
  }else{
    printf("session_core_1_bs__set_session_token\n");
    exit(1);
  }
}

void session_core_1_bs__delete_session_token(
   const constants__t_session_token_i session_core_1_bs__session_token) {
  if(constants__c_session_token_indet != session_core_1_bs__session_token){
    SOPC_NodeId* token = session_core_1_bs__session_token ;
    SOPC_NodeId_Clear(token);
    free(token);
  }
}

void session_core_1_bs__delete_session(
   const constants__t_session_i session_core_1_bs__session) {
  (void) session_core_1_bs__session;
  printf("session_core_1_bs__delete_session\n");
  exit(1);   
}

void session_core_1_bs__create_new_session(
   const constants__t_channel_i session_core_1_bs__channel,
   const constants__t_sessionState session_core_1_bs__state,
   constants__t_session_i * const session_core_1_bs__session) {
  if(FALSE == unique_session_init){
    unique_session.session_core_1_bs__channel = session_core_1_bs__channel;
    unique_session.session_core_1_bs__state = session_core_1_bs__state;
    unique_session.session_core_1_bs__session_token = constants__c_session_token_indet;
    unique_session.session_core_1_bs__user = constants__c_user_indet;
    unique_session.session_core_1_bs__req_handle = constants__c_request_handle_indet;
    unique_session.session_core_1_bs__req_handle_used = FALSE;
    unique_session_init = (!FALSE);
    *session_core_1_bs__session = &unique_session;
  }else{
    *session_core_1_bs__session = constants__c_session_indet;
  }
}
  
void session_core_1_bs__cli_add_pending_request(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_request_handle_i session_core_1_bs__req_handle,
   t_bool * const session_core_1_bs__ret) {
  if(session_core_1_bs__session == &unique_session){
    if(FALSE == unique_session.session_core_1_bs__req_handle_used){
      unique_session.session_core_1_bs__req_handle = session_core_1_bs__req_handle;
      unique_session.session_core_1_bs__req_handle_used = (!FALSE);
      *session_core_1_bs__ret = (!FALSE);
    }else{
      *session_core_1_bs__ret = FALSE;
    }
  }else{
    *session_core_1_bs__ret = FALSE;
  }
}

void session_core_1_bs__cli_remove_pending_request(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_request_handle_i session_core_1_bs__req_handle,
   t_bool * const session_core_1_bs__ret) {
  if(session_core_1_bs__session == &unique_session){
    if((!FALSE) == unique_session.session_core_1_bs__req_handle_used &&
       unique_session.session_core_1_bs__req_handle == session_core_1_bs__req_handle){
      unique_session.session_core_1_bs__req_handle_used = FALSE;
      *session_core_1_bs__ret = (!FALSE);
    }else{
      *session_core_1_bs__ret = FALSE;
    }
  }else{
    *session_core_1_bs__ret = FALSE;
  }
}

void session_core_1_bs__is_valid_session(
   const constants__t_session_i session_core_1_bs__session,
   t_bool * const session_core_1_bs__ret) {
  *session_core_1_bs__ret = session_core_1_bs__session == &unique_session;
}

void session_core_1_bs__get_session_state(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_sessionState * const session_core_1_bs__state) {
  if(session_core_1_bs__session == &unique_session){
    *session_core_1_bs__state = unique_session.session_core_1_bs__state;
  }else{
    *session_core_1_bs__state = constants__e_session_closed;
  }
}

void session_core_1_bs__set_session_state(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_sessionState session_core_1_bs__state) {
  if(&unique_session == session_core_1_bs__session){
    unique_session.session_core_1_bs__state = session_core_1_bs__state;
  }
}

void session_core_1_bs__set_session_state_closed(
   const constants__t_session_i session_core_1_bs__session) {
  if(session_core_1_bs__session == &unique_session){
    unique_session.session_core_1_bs__state = constants__e_session_closed;
    unique_session.session_core_1_bs__user = constants__c_user_indet;
    unique_session.session_core_1_bs__req_handle = constants__c_request_handle_indet;
    unique_session.session_core_1_bs__req_handle_used = FALSE;
    if(constants__c_session_token_indet != unique_session.session_core_1_bs__session_token){
      session_core_1_bs__delete_session_token(unique_session.session_core_1_bs__session_token);
      unique_session.session_core_1_bs__session_token = constants__c_session_token_indet;
    }
    unique_session.session_core_1_bs__channel = constants__c_channel_indet;
  }
}

void session_core_1_bs__set_session_channel(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__channel) {
  // TODO: add constraint that session has no channel in B model ?
  //  => for now the fact it is not is used for setting channel in both cases of activate session (change of user / channel)
  if(session_core_1_bs__session == &unique_session){
    unique_session.session_core_1_bs__channel = session_core_1_bs__channel;
  }else{
    printf("session_core_1_bs__set_session_channel\n");
    exit(1);   
  }
}

void session_core_1_bs__get_session_channel(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_channel_i * const session_core_1_bs__channel) {
  if(session_core_1_bs__session == &unique_session){
    *session_core_1_bs__channel = unique_session.session_core_1_bs__channel;
  }else{
    // session : s_session guarantee
    printf("session_core_1_bs__get_session_channel\n");
    exit(1);
  }
}

void session_core_1_bs__set_session_orphaned(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_channel_i session_core_1_bs__lost_channel) {
  (void) session_core_1_bs__session;
  (void) session_core_1_bs__lost_channel;
  printf("session_core_1_bs__set_session_orphaned\n");
  exit(1);   
   ;
}

void session_core_1_bs__is_valid_user(
   const constants__t_user_i session_core_1_bs__user,
   t_bool * const session_core_1_bs__ret) {
  assert(session_core_1_bs__user == 1);
  *session_core_1_bs__ret = (!FALSE);
}

void session_core_1_bs__set_session_user(
   const constants__t_session_i session_core_1_bs__session,
   const constants__t_user_i session_core_1_bs__user) {
  if(session_core_1_bs__session == &unique_session){
    unique_session.session_core_1_bs__user = session_core_1_bs__user;
  }
}

void session_core_1_bs__get_session_user(
   const constants__t_session_i session_core_1_bs__session,
   constants__t_user_i * const session_core_1_bs__user) {
  if(session_core_1_bs__session == &unique_session){
    *session_core_1_bs__user = unique_session.session_core_1_bs__user;
  }
}

