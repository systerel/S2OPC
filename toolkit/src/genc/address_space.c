/******************************************************************************

 File Name            : address_space.c

 Date                 : 31/05/2017 17:51:41

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "address_space.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void address_space__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void address_space__read_NodeClass_Attribute(
   const constants__t_Node_i address_space__node,
   const constants__t_AttributeId_i address_space__aid,
   constants__t_NodeClass_i * const address_space__ncl,
   constants__t_Variant_i * const address_space__val) {
   address_space_bs__read_NodeClass(address_space__node,
      address_space__ncl);
   address_space_bs__read_AddressSpace_Attribute_value(address_space__node,
      address_space__aid,
      address_space__val);
}

