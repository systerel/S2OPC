/******************************************************************************

 File Name            : service_read.h

 Date                 : 28/09/2017 17:28:28

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_read_h
#define _service_read_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_read_request.h"
#include "msg_read_response_bs.h"
#include "service_read_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_read__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define service_read__alloc_read_response msg_read_response_bs__alloc_read_response
#define service_read__check_ReadRequest msg_read_request__check_ReadRequest

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_read__fill_read_response_1(
   const constants__t_msg_i service_read__p_resp_msg,
   const t_bool service_read__p_isvalid,
   const constants__t_NodeId_i service_read__p_nid,
   const constants__t_AttributeId_i service_read__p_aid,
   const constants__t_ReadValue_i service_read__p_rvi);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_read__fill_read_response(
   const constants__t_msg_i service_read__req_msg,
   const constants__t_msg_i service_read__resp_msg);

#endif
