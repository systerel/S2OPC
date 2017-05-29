/******************************************************************************

 File Name            : msg_read_response.h

 Date                 : 31/05/2017 17:51:42

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_read_response_h
#define _msg_read_response_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "msg_read_response_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_out_bs.h"
#include "msg_read_request.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
extern constants__t_StatusCode_i msg_read_response__StatusCode;
extern constants__t_msg_i msg_read_response__msg;
extern t_entier4 msg_read_response__nb_resps_to_go;
extern constants__t_StatusCode_i msg_read_response__tab_StatusCode[constants__t_ReadValue_i_max+1];
extern constants__t_Variant_i msg_read_response__tab_Value[constants__t_ReadValue_i_max+1];

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_read_response__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_read_response__continue_iterwrite_DataValue(
   const constants__t_ReadValue_i msg_read_response__rvi,
   const constants__t_Variant_i msg_read_response__val,
   const constants__t_StatusCode_i msg_read_response__sc,
   t_bool * const msg_read_response__continue);
extern void msg_read_response__init_iterwrite_DataValue(
   const t_entier4 msg_read_response__a_nb_resps,
   const constants__t_msg_i msg_read_response__resp_msg,
   t_bool * const msg_read_response__continue);
extern void msg_read_response__write_response_error(
   const constants__t_StatusCode_i msg_read_response__sc,
   const constants__t_msg_i msg_read_response__resp_msg);

#endif
