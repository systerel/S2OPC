/******************************************************************************

 File Name            : msg_read_request.h

 Date                 : 13/07/2017 16:54:05

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_read_request_h
#define _msg_read_request_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_read_request_bs.h"
#include "msg_read_request_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"
#include "message_in_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_bool msg_read_request__init_ok;
extern t_entier4 msg_read_request__nb_ReadValue;
extern constants__t_AttributeId_i msg_read_request__tab_req_AttributeId[constants__t_ReadValue_i_max+1];
extern constants__t_NodeId_i msg_read_request__tab_req_NodeId[constants__t_ReadValue_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_request__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_request__read_ReadRequest(
   const constants__t_msg_i msg_read_request__msg,
   t_bool * const msg_read_request__rr);
extern void msg_read_request__read_nb_ReadValue(
   t_entier4 * const msg_read_request__rr);
extern void msg_read_request__readall_ReadValue_Node_AttributeId(
   const constants__t_ReadValue_i msg_read_request__rvi,
   t_bool * const msg_read_request__isvalid,
   constants__t_Node_i * const msg_read_request__node,
   constants__t_AttributeId_i * const msg_read_request__aid);

#endif
