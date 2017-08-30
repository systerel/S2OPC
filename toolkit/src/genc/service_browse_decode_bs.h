/******************************************************************************

 File Name            : service_browse_decode_bs.h

 Date                 : 30/08/2017 19:04:07

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_browse_decode_bs_h
#define _service_browse_decode_bs_h

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
extern void service_browse_decode_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_browse_decode_bs__decode_browse_request(
   const constants__t_byte_buffer_i service_browse_decode_bs__req_payload,
   constants__t_StatusCode_i * const service_browse_decode_bs__StatusCode_service);
extern void service_browse_decode_bs__free_browse_request(void);
extern void service_browse_decode_bs__get_nb_BrowseTargetMax(
   t_entier4 * const service_browse_decode_bs__p_nb_BrowseTargetMax);
extern void service_browse_decode_bs__get_nb_BrowseValue(
   t_entier4 * const service_browse_decode_bs__nb_req);
extern void service_browse_decode_bs__getall_BrowseValue(
   const constants__t_BrowseValue_i service_browse_decode_bs__p_bvi,
   t_bool * const service_browse_decode_bs__p_isvalid,
   constants__t_NodeId_i * const service_browse_decode_bs__p_NodeId,
   constants__t_BrowseDirection_i * const service_browse_decode_bs__p_dir,
   t_bool * const service_browse_decode_bs__p_isreftype,
   constants__t_NodeId_i * const service_browse_decode_bs__p_reftype,
   t_bool * const service_browse_decode_bs__p_inc_subtype);

#endif
