/******************************************************************************

 File Name            : service_read.h

 Date                 : 31/05/2017 17:51:43

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
#include "msg_read_response.h"

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

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_read__treat_read_request(
   const constants__t_msg_i service_read__req_msg,
   const constants__t_msg_i service_read__resp_msg);

#endif
