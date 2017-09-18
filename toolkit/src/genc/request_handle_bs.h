/******************************************************************************

 File Name            : request_handle_bs.h

 Date                 : 15/09/2017 14:19:12

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _request_handle_bs_h
#define _request_handle_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void request_handle_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void request_handle_bs__client_fresh_req_handle(
   const constants__t_msg_type_i request_handle_bs__resp_typ,
   constants__t_request_handle_i * const request_handle_bs__request_handle);
extern void request_handle_bs__client_remove_req_handle(
   const constants__t_request_handle_i request_handle_bs__req_handle);
extern void request_handle_bs__client_validate_response_request_handle(
   const constants__t_request_handle_i request_handle_bs__req_handle,
   const constants__t_msg_type_i request_handle_bs__resp_typ,
   t_bool * const request_handle_bs__ret);
extern void request_handle_bs__is_valid_req_handle(
   const constants__t_request_handle_i request_handle_bs__req_handle,
   t_bool * const request_handle_bs__ret);

#endif
