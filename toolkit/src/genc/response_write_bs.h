/******************************************************************************

 File Name            : response_write_bs.h

 Date                 : 28/09/2017 17:30:56

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _response_write_bs_h
#define _response_write_bs_h

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
extern void response_write_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void response_write_bs__alloc_write_request_responses_malloc(
   const t_entier4 response_write_bs__nb_req,
   t_bool * const response_write_bs__ResponseWrite_allocated);
extern void response_write_bs__getall_ResponseWrite_StatusCode(
   const constants__t_WriteValue_i response_write_bs__wvi,
   t_bool * const response_write_bs__isvalid,
   constants__t_StatusCode_i * const response_write_bs__sc);
extern void response_write_bs__reset_ResponseWrite(void);
extern void response_write_bs__set_ResponseWrite_StatusCode(
   const constants__t_WriteValue_i response_write_bs__wvi,
   const constants__t_StatusCode_i response_write_bs__sc);
extern void response_write_bs__write_WriteResponse_msg_out(
   const constants__t_msg_i response_write_bs__msg_out);

#endif
