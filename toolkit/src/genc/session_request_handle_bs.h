/******************************************************************************

 File Name            : session_request_handle_bs.h

 Date                 : 10/08/2017 16:06:15

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _session_request_handle_bs_h
#define _session_request_handle_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "request_handle_bs.h"
#include "session_core.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void session_request_handle_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void session_request_handle_bs__client_add_session_request_handle(
   const constants__t_session_i session_request_handle_bs__session,
   const constants__t_request_handle_i session_request_handle_bs__req_handle);
extern void session_request_handle_bs__client_get_session_and_remove_request_handle(
   const constants__t_request_handle_i session_request_handle_bs__req_handle,
   constants__t_session_i * const session_request_handle_bs__session);
extern void session_request_handle_bs__client_remove_all_request_handles(
   const constants__t_session_i session_request_handle_bs__session);

#endif
