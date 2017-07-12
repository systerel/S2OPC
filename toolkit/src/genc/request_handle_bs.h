/******************************************************************************

 File Name            : request_handle_bs.h

 Date                 : 13/07/2017 16:54:09

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
extern void request_handle_bs__fresh_req_handle(
   constants__t_request_handle_i * const request_handle_bs__request_handle);
extern void request_handle_bs__is_valid_req_handle(
   const constants__t_request_handle_i request_handle_bs__req_handle,
   t_bool * const request_handle_bs__ret);
extern void request_handle_bs__remove_req_handle(
   const constants__t_request_handle_i request_handle_bs__req_handle);

#endif
