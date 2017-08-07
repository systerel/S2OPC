/******************************************************************************

 File Name            : msg_read_request_bs.h

 Date                 : 07/08/2017 16:37:09

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_read_request_bs_h
#define _msg_read_request_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_request_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_request_bs__getall_req_ReadValue_AttributeId(
   const constants__t_msg_i msg_read_request_bs__msg,
   const constants__t_ReadValue_i msg_read_request_bs__rvi,
   t_bool * const msg_read_request_bs__isvalid,
   constants__t_AttributeId_i * const msg_read_request_bs__aid);
extern void msg_read_request_bs__getall_req_ReadValue_NodeId(
   const constants__t_msg_i msg_read_request_bs__msg,
   const constants__t_ReadValue_i msg_read_request_bs__rvi,
   t_bool * const msg_read_request_bs__isvalid,
   constants__t_NodeId_i * const msg_read_request_bs__nid);
extern void msg_read_request_bs__read_req_nb_ReadValue(
   const constants__t_msg_i msg_read_request_bs__msg,
   t_entier4 * const msg_read_request_bs__nb);

#endif
