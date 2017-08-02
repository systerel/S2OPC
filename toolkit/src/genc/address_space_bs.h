/******************************************************************************

 File Name            : address_space_bs.h

 Date                 : 03/08/2017 13:22:24

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _address_space_bs_h
#define _address_space_bs_h

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
extern void address_space_bs__INITIALISATION(void);

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space_bs__UNINITIALISATION(void);
extern void address_space_bs__get_NodeClass(
   const constants__t_Node_i address_space_bs__node,
   constants__t_NodeClass_i * const address_space_bs__ncl);
extern void address_space_bs__get_Value_StatusCode(
   const constants__t_Node_i address_space_bs__node,
   constants__t_StatusCode_i * const address_space_bs__sc);
extern void address_space_bs__read_AddressSpace_Attribute_value(
   const constants__t_Node_i address_space_bs__node,
   const constants__t_AttributeId_i address_space_bs__aid,
   constants__t_Variant_i * const address_space_bs__variant);
extern void address_space_bs__read_AddressSpace_free_value(
   const constants__t_Variant_i address_space_bs__val);
extern void address_space_bs__readall_AddressSpace_Node(
   const constants__t_NodeId_i address_space_bs__nid,
   t_bool * const address_space_bs__nid_valid,
   constants__t_Node_i * const address_space_bs__node);
extern void address_space_bs__set_Value(
   const constants__t_Node_i address_space_bs__node,
   const constants__t_Variant_i address_space_bs__value);

#endif
