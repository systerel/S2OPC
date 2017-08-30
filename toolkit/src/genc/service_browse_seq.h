/******************************************************************************

 File Name            : service_browse_seq.h

 Date                 : 30/08/2017 15:32:02

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _service_browse_seq_h
#define _service_browse_seq_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "service_browse.h"
#include "service_browse_decode_bs.h"
#include "service_browse_seq_it.h"

/*--------------
   SEES Clause
  --------------*/
#include "address_space.h"
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void service_browse_seq__INITIALISATION(void);

/*--------------------------
   LOCAL_OPERATIONS Clause
  --------------------------*/
extern void service_browse_seq__fill_browse_response(
   const constants__t_BrowseValue_i service_browse_seq__p_bvi,
   const t_entier4 service_browse_seq__p_nb_bri,
   const constants__t_Node_i service_browse_seq__p_src_node,
   const constants__t_BrowseDirection_i service_browse_seq__p_dir,
   const t_bool service_browse_seq__p_isreftype,
   const constants__t_NodeId_i service_browse_seq__p_reftype,
   const t_bool service_browse_seq__p_inc_subtype);
extern void service_browse_seq__fill_browse_response_ref(
   const constants__t_BrowseValue_i service_browse_seq__p_bvi,
   const constants__t_BrowseResult_i service_browse_seq__p_bri,
   const constants__t_Reference_i service_browse_seq__p_ref,
   const constants__t_BrowseDirection_i service_browse_seq__p_dir,
   const t_bool service_browse_seq__p_isreftype,
   const constants__t_NodeId_i service_browse_seq__p_ref_type,
   const t_bool service_browse_seq__p_inc_subtype,
   t_bool * const service_browse_seq__p_continue_bri,
   constants__t_BrowseResult_i * const service_browse_seq__p_bri_new);
extern void service_browse_seq__treat_browse_request_BrowseValue_1(
   const constants__t_BrowseValue_i service_browse_seq__p_bvi,
   const t_entier4 service_browse_seq__p_nb_target_max);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void service_browse_seq__treat_browse_request_BrowseValues(
   constants__t_StatusCode_i * const service_browse_seq__StatusCode_service);

#endif
