/******************************************************************************

 File Name            : service_write_decode_bs.h

 Date                 : 30/08/2017 19:04:07

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_write_decode_bs_h
#define _service_write_decode_bs_h

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
extern void service_write_decode_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_write_decode_bs__decode_write_request(
   const constants__t_msg_i service_write_decode_bs__write_msg,
   constants__t_StatusCode_i * const service_write_decode_bs__StatusCode_service);
extern void service_write_decode_bs__free_write_request(void);
extern void service_write_decode_bs__get_nb_WriteValue(
   t_entier4 * const service_write_decode_bs__nb_req);
extern void service_write_decode_bs__getall_WriteValue(
   const constants__t_WriteValue_i service_write_decode_bs__wvi,
   t_bool * const service_write_decode_bs__isvalid,
   constants__t_NodeId_i * const service_write_decode_bs__nid,
   constants__t_AttributeId_i * const service_write_decode_bs__aid,
   constants__t_Variant_i * const service_write_decode_bs__value);

#endif
