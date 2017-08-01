/******************************************************************************

 File Name            : msg_read_response_bs.h

 Date                 : 01/08/2017 11:32:40

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_read_response_bs_h
#define _msg_read_response_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_response_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_response_bs__alloc_read_response(
   const t_entier4 msg_read_response_bs__p_nb_resps,
   const constants__t_msg_i msg_read_response_bs__p_resp_msg,
   t_bool * const msg_read_response_bs__p_isvalid);
extern void msg_read_response_bs__set_read_response(
   const constants__t_msg_i msg_read_response_bs__resp_msg,
   const constants__t_ReadValue_i msg_read_response_bs__rvi,
   const constants__t_Variant_i msg_read_response_bs__val,
   const constants__t_StatusCode_i msg_read_response_bs__sc);

#endif
