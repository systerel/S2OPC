/******************************************************************************

 File Name            : address_space.h

 Date                 : 31/05/2017 17:51:41

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

#ifndef _address_space_h
#define _address_space_h

/*--------------------------
   Added by the Translator
  --------------------------*/
#include "b2c.h"

/*-----------------
   IMPORTS Clause
  -----------------*/
#include "address_space_bs.h"

/*--------------
   SEES Clause
  --------------*/
#include "constants.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
extern void address_space__INITIALISATION(void);

/*-------------------------------
   PROMOTES and EXTENDS Clauses
  -------------------------------*/
#define address_space__read_AddressSpace_free_value address_space_bs__read_AddressSpace_free_value
#define address_space__read_Value_StatusCode address_space_bs__read_Value_StatusCode
#define address_space__readall_AddressSpace_Node address_space_bs__readall_AddressSpace_Node

/*--------------------
   OPERATIONS Clause
  --------------------*/
extern void address_space__read_NodeClass_Attribute(
   const constants__t_Node_i address_space__node,
   const constants__t_AttributeId_i address_space__aid,
   constants__t_NodeClass_i * const address_space__ncl,
   constants__t_Variant_i * const address_space__val);

#endif
