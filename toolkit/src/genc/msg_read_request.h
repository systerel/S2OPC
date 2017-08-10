/******************************************************************************

 File Name            : msg_read_request.h

 Date                 : 10/08/2017 16:06:10

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

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"
#include "message_in_bs.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern t_entier4 msg_read_request__nb_ReadValue;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_request__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_request__check_ReadRequest(
   const constants__t_msg_i msg_read_request__p_msg,
   t_bool * const msg_read_request__p_read_ok,
   t_entier4 * const msg_read_request__p_nb_ReadValue);
extern void msg_read_request__get_nb_ReadValue(
   t_entier4 * const msg_read_request__p_nb_ReadValue);
extern void msg_read_request__getall_ReadValue_NodeId_AttributeId(
   const constants__t_msg_i msg_read_request__p_msg,
   const constants__t_ReadValue_i msg_read_request__p_rvi,
   t_bool * const msg_read_request__p_isvalid,
   constants__t_NodeId_i * const msg_read_request__p_nid,
   constants__t_AttributeId_i * const msg_read_request__p_aid);

#endif
