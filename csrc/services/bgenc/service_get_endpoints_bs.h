/******************************************************************************

 File Name            : service_get_endpoints_bs.h

 Date                 : 13/10/2017 09:44:55

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_get_endpoints_bs_h
#define _service_get_endpoints_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_in_bs.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_get_endpoints_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_get_endpoints_bs__treat_get_endpoints_request(
   const constants__t_msg_i service_get_endpoints_bs__req_msg,
   const constants__t_msg_i service_get_endpoints_bs__resp_msg,
   const constants__t_endpoint_config_idx_i service_get_endpoints_bs__endpoint_config_idx,
   constants__t_StatusCode_i * const service_get_endpoints_bs__ret);

#endif
