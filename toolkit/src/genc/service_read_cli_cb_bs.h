/******************************************************************************

 File Name            : service_read_cli_cb_bs.h

 Date                 : 13/07/2017 16:54:09

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_read_cli_cb_bs_h
#define _service_read_cli_cb_bs_h

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
extern void service_read_cli_cb_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_read_cli_cb_bs__cli_service_read_response(
   const constants__t_msg_i service_read_cli_cb_bs__resp_msg,
   const constants__t_StatusCode_i service_read_cli_cb_bs__status);

#endif
