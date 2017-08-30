/******************************************************************************

 File Name            : msg_browse_response_bs.h

 Date                 : 30/08/2017 15:32:05

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _msg_browse_response_bs_h
#define _msg_browse_response_bs_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"
#include "message_out_bs.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void msg_browse_response_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void msg_browse_response_bs__malloc_browse_result(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const t_entier4 msg_browse_response_bs__p_nb_bri,
   t_bool * const msg_browse_response_bs__p_isallocated);
extern void msg_browse_response_bs__reset_ResponseBrowse_ContinuationPoint(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi);
extern void msg_browse_response_bs__reset_ResponseBrowse_Res_BrowseName(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri);
extern void msg_browse_response_bs__reset_ResponseBrowse_Res_DisplayName(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri);
extern void msg_browse_response_bs__reset_ResponseBrowse_Res_NodeClass(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri);
extern void msg_browse_response_bs__set_ResponseBrowse_BrowseStatus(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const t_bool msg_browse_response_bs__p_bool);
extern void msg_browse_response_bs__set_ResponseBrowse_ContinuationPoint(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_Reference_i msg_browse_response_bs__p_ref);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_BrowseName(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_QualifiedName_i msg_browse_response_bs__p_BrowseName);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_DisplayName(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_LocalizedText_i msg_browse_response_bs__p_DisplayName);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_Forwards(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const t_bool msg_browse_response_bs__p_bool);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_NodeClass(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_NodeClass_i msg_browse_response_bs__p_NodeClass);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_NodeId(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_ExpandedNodeId_i msg_browse_response_bs__p_ExpandedNodeId);
extern void msg_browse_response_bs__set_ResponseBrowse_Res_ReferenceTypeId(
   const constants__t_BrowseValue_i msg_browse_response_bs__p_bvi,
   const constants__t_BrowseResult_i msg_browse_response_bs__p_bri,
   const constants__t_NodeId_i msg_browse_response_bs__p_NodeId);

#endif
