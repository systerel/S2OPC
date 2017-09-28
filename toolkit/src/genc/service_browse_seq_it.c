/******************************************************************************

 File Name            : service_browse_seq_it.c

 Date                 : 28/09/2017 17:52:23

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "service_browse_seq_it.h"

/*----------------------------
   CONCRETE_VARIABLES Clause
  ----------------------------*/
t_entier4 service_browse_seq_it__RefIndex;
t_entier4 service_browse_seq_it__RefIndexEnd;
t_entier4 service_browse_seq_it__bri_i;
t_entier4 service_browse_seq_it__bvi_i;
t_entier4 service_browse_seq_it__nb_bri;
t_entier4 service_browse_seq_it__nb_bvi;

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void service_browse_seq_it__INITIALISATION(void) {
   service_browse_seq_it__nb_bvi = 0;
   service_browse_seq_it__bvi_i = 0;
   service_browse_seq_it__nb_bri = 0;
   service_browse_seq_it__bri_i = 0;
   service_browse_seq_it__RefIndex = 0;
   service_browse_seq_it__RefIndexEnd = 0;
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void service_browse_seq_it__init_iter_browse_request(
   const t_entier4 service_browse_seq_it__p_nb_req,
   t_bool * const service_browse_seq_it__p_continue) {
   service_browse_seq_it__nb_bvi = service_browse_seq_it__p_nb_req;
   service_browse_seq_it__bvi_i = 1;
   *service_browse_seq_it__p_continue = (1 <= service_browse_seq_it__p_nb_req);
}

void service_browse_seq_it__continue_iter_browse_request(
   t_bool * const service_browse_seq_it__p_continue,
   constants__t_BrowseValue_i * const service_browse_seq_it__p_bvi) {
   constants__get_cast_t_BrowseValue(service_browse_seq_it__bvi_i,
      service_browse_seq_it__p_bvi);
   service_browse_seq_it__bvi_i = service_browse_seq_it__bvi_i +
      1;
   *service_browse_seq_it__p_continue = (service_browse_seq_it__bvi_i <= service_browse_seq_it__nb_bvi);
}

void service_browse_seq_it__init_iter_browse_result(
   const t_entier4 service_browse_seq_it__p_nb_bri,
   t_bool * const service_browse_seq_it__p_continue) {
   service_browse_seq_it__nb_bri = service_browse_seq_it__p_nb_bri;
   service_browse_seq_it__bri_i = 1;
   *service_browse_seq_it__p_continue = (1 <= service_browse_seq_it__p_nb_bri);
}

void service_browse_seq_it__continue_iter_browse_result(
   t_bool * const service_browse_seq_it__p_continue,
   constants__t_BrowseResult_i * const service_browse_seq_it__p_bri) {
   constants__get_cast_t_BrowseResult(service_browse_seq_it__bri_i,
      service_browse_seq_it__p_bri);
   *service_browse_seq_it__p_continue = (service_browse_seq_it__bri_i <= service_browse_seq_it__nb_bri);
   service_browse_seq_it__bri_i = service_browse_seq_it__bri_i +
      1;
}

void service_browse_seq_it__init_iter_reference(
   const constants__t_Node_i service_browse_seq_it__p_node,
   t_bool * const service_browse_seq_it__p_continue) {
   address_space__get_Node_RefIndexBegin(service_browse_seq_it__p_node,
      &service_browse_seq_it__RefIndex);
   address_space__get_Node_RefIndexEnd(service_browse_seq_it__p_node,
      &service_browse_seq_it__RefIndexEnd);
   *service_browse_seq_it__p_continue = (service_browse_seq_it__RefIndexEnd >= service_browse_seq_it__RefIndex);
}

void service_browse_seq_it__continue_iter_reference(
   t_bool * const service_browse_seq_it__p_continue,
   constants__t_Reference_i * const service_browse_seq_it__p_ref) {
   address_space__get_RefIndex_Reference(service_browse_seq_it__RefIndex,
      service_browse_seq_it__p_ref);
   service_browse_seq_it__RefIndex = service_browse_seq_it__RefIndex +
      1;
   *service_browse_seq_it__p_continue = (service_browse_seq_it__RefIndex <= service_browse_seq_it__RefIndexEnd);
}

