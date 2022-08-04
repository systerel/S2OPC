/*
 * Licensed to Systerel under one or more contributor license
 * agreements. See the NOTICE file distributed with this work
 * for additional information regarding copyright ownership.
 * Systerel licenses this file to you under the Apache
 * License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

/******************************************************************************

 File Name            : call_method_mgr.c

 Date                 : 04/08/2022 14:53:04

 C Translator Version : tradc Java V1.2 (06/02/2022)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "call_method_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void call_method_mgr__INITIALISATION(void) {
}

/*--------------------
   OPERATIONS Clause
  --------------------*/
void call_method_mgr__treat_method_call_request(
   const constants__t_session_i call_method_mgr__p_session,
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_msg_i call_method_mgr__p_resp_msg,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode_service) {
   {
      constants__t_endpoint_config_idx_i call_method_mgr__l_endpoint_config_idx;
      t_bool call_method_mgr__l_continue;
      t_entier4 call_method_mgr__l_nb;
      constants_statuscodes_bs__t_StatusCode_i call_method_mgr__l_status_op;
      constants__t_CallMethod_i call_method_mgr__l_callMethod;
      
      session_mgr__session_get_endpoint_config(call_method_mgr__p_session,
         &call_method_mgr__l_endpoint_config_idx);
      if (call_method_mgr__l_endpoint_config_idx != constants__c_endpoint_config_idx_indet) {
         msg_call_method_bs__read_call_method_request(call_method_mgr__p_req_msg,
            call_method_mgr__StatusCode_service,
            &call_method_mgr__l_nb);
         if (*call_method_mgr__StatusCode_service == constants_statuscodes_bs__e_sc_ok) {
            call_method_it__init_iter_callMethods(call_method_mgr__p_req_msg,
               &call_method_mgr__l_continue);
            if (call_method_mgr__l_continue == false) {
               *call_method_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_nothing_to_do;
            }
            else if (call_method_mgr__l_nb > constants__k_n_genericOperationPerReq_max) {
               *call_method_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_too_many_ops;
            }
            else {
               msg_call_method_bs__alloc_CallMethod_Result(call_method_mgr__p_resp_msg,
                  call_method_mgr__l_nb,
                  call_method_mgr__StatusCode_service);
               call_method_mgr__l_continue = (*call_method_mgr__StatusCode_service == constants_statuscodes_bs__e_sc_ok);
               while (call_method_mgr__l_continue == true) {
                  call_method_it__continue_iter_callMethod(&call_method_mgr__l_continue,
                     &call_method_mgr__l_callMethod);
                  call_method_mgr__treat_one_method_call(call_method_mgr__p_session,
                     call_method_mgr__p_req_msg,
                     call_method_mgr__p_resp_msg,
                     call_method_mgr__l_callMethod,
                     call_method_mgr__l_endpoint_config_idx,
                     &call_method_mgr__l_status_op);
                  if (call_method_mgr__l_status_op == constants_statuscodes_bs__e_sc_bad_out_of_memory) {
                     call_method_mgr__l_continue = false;
                     *call_method_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_out_of_memory;
                  }
               }
            }
         }
      }
      else {
         *call_method_mgr__StatusCode_service = constants_statuscodes_bs__e_sc_bad_session_id_invalid;
      }
   }
}

void call_method_mgr__treat_one_method_call(
   const constants__t_session_i call_method_mgr__p_session,
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_msg_i call_method_mgr__p_res_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   const constants__t_endpoint_config_idx_i call_method_mgr__p_endpoint_config_idx,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode) {
   {
      constants__t_RawStatusCode call_method_mgr__l_rawStatusCode;
      
      call_method_mgr__check_method_call_inputs(call_method_mgr__p_session,
         call_method_mgr__p_req_msg,
         call_method_mgr__p_callMethod,
         call_method_mgr__p_res_msg,
         call_method_mgr__StatusCode);
      constants_statuscodes_bs__getall_conv_StatusCode_To_RawStatusCode(*call_method_mgr__StatusCode,
         &call_method_mgr__l_rawStatusCode);
      if (*call_method_mgr__StatusCode == constants_statuscodes_bs__e_sc_ok) {
         call_method_bs__exec_callMethod(call_method_mgr__p_req_msg,
            call_method_mgr__p_callMethod,
            call_method_mgr__p_endpoint_config_idx,
            &call_method_mgr__l_rawStatusCode);
         constants_statuscodes_bs__getall_conv_RawStatusCode_To_StatusCode(call_method_mgr__l_rawStatusCode,
            call_method_mgr__StatusCode);
         if (*call_method_mgr__StatusCode == constants_statuscodes_bs__e_sc_ok) {
            call_method_mgr__copy_exec_result(call_method_mgr__p_res_msg,
               call_method_mgr__p_callMethod,
               call_method_mgr__StatusCode);
         }
         call_method_bs__free_exec_result();
      }
      msg_call_method_bs__write_CallMethod_Res_Status(call_method_mgr__p_res_msg,
         call_method_mgr__p_callMethod,
         call_method_mgr__l_rawStatusCode);
   }
}

void call_method_mgr__check_method_call_inputs(
   const constants__t_session_i call_method_mgr__p_session,
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   const constants__t_msg_i call_method_mgr__p_res_msg,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode) {
   {
      constants__t_Node_i call_method_mgr__l_object;
      constants__t_NodeId_i call_method_mgr__l_objectid;
      constants__t_Node_i call_method_mgr__l_method;
      constants__t_NodeId_i call_method_mgr__l_methodid;
      constants__t_NodeClass_i call_method_mgr__l_nodeClass;
      constants__t_user_i call_method_mgr__l_user;
      t_bool call_method_mgr__l_valid_executable;
      t_bool call_method_mgr__l_valid_user_executable;
      t_bool call_method_mgr__l_object_has_method;
      
      msg_call_method_bs__read_CallMethod_Objectid(call_method_mgr__p_req_msg,
         call_method_mgr__p_callMethod,
         &call_method_mgr__l_objectid);
      address_space_itf__check_nodeId_isValid(call_method_mgr__l_objectid,
         call_method_mgr__StatusCode,
         &call_method_mgr__l_object);
      if (*call_method_mgr__StatusCode == constants_statuscodes_bs__e_sc_ok) {
         msg_call_method_bs__read_CallMethod_MethodId(call_method_mgr__p_req_msg,
            call_method_mgr__p_callMethod,
            &call_method_mgr__l_methodid);
         address_space_itf__check_nodeId_isValid(call_method_mgr__l_methodid,
            call_method_mgr__StatusCode,
            &call_method_mgr__l_method);
         if (*call_method_mgr__StatusCode == constants_statuscodes_bs__e_sc_ok) {
            address_space_itf__get_NodeClass(call_method_mgr__l_method,
               &call_method_mgr__l_nodeClass);
            if (call_method_mgr__l_nodeClass == constants__e_ncl_Method) {
               session_mgr__get_session_user_server(call_method_mgr__p_session,
                  &call_method_mgr__l_user);
               address_space_itf__get_Executable(call_method_mgr__l_method,
                  &call_method_mgr__l_valid_executable);
               address_space_itf__get_user_authorization(constants__e_operation_type_executable,
                  call_method_mgr__l_methodid,
                  constants__e_aid_UserExecutable,
                  call_method_mgr__l_user,
                  &call_method_mgr__l_valid_user_executable);
               if ((call_method_mgr__l_valid_executable == true) &&
                  (call_method_mgr__l_valid_user_executable == true)) {
                  address_space_itf__check_object_has_method(call_method_mgr__l_objectid,
                     call_method_mgr__l_methodid,
                     &call_method_mgr__l_object_has_method);
                  if (call_method_mgr__l_object_has_method == true) {
                     call_method_mgr__check_method_call_arguments(call_method_mgr__p_req_msg,
                        call_method_mgr__p_callMethod,
                        call_method_mgr__l_method,
                        call_method_mgr__p_res_msg,
                        call_method_mgr__StatusCode);
                  }
                  else {
                     *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_bad_method_invalid;
                  }
               }
               else {
                  *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_bad_user_access_denied;
               }
            }
            else {
               *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_bad_method_invalid;
            }
         }
         else {
            *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_bad_method_invalid;
         }
      }
   }
}

void call_method_mgr__check_method_call_arguments(
   const constants__t_msg_i call_method_mgr__p_req_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   const constants__t_Node_i call_method_mgr__p_method_node,
   const constants__t_msg_i call_method_mgr__p_res_msg,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode) {
   {
      t_entier4 call_method_mgr__l_nb_req_arg;
      t_bool call_method_mgr__l_input_arg_valid;
      t_entier4 call_method_mgr__l_nb_method_arg;
      t_bool call_method_mgr__l_continue;
      constants__t_Variant_i call_method_mgr__l_input_arg_variant;
      t_entier4 call_method_mgr__l_index;
      constants__t_Argument_i call_method_mgr__l_arg_desc;
      constants__t_Variant_i call_method_mgr__l_val;
      constants_statuscodes_bs__t_StatusCode_i call_method_mgr__l_arg_status;
      
      address_space_itf__get_InputArguments(call_method_mgr__p_method_node,
         &call_method_mgr__l_input_arg_variant);
      argument_pointer_bs__read_variant_nb_argument(call_method_mgr__l_input_arg_variant,
         call_method_mgr__p_method_node,
         &call_method_mgr__l_nb_method_arg,
         &call_method_mgr__l_input_arg_valid);
      msg_call_method_bs__read_CallMethod_Nb_InputArguments(call_method_mgr__p_req_msg,
         call_method_mgr__p_callMethod,
         &call_method_mgr__l_nb_req_arg);
      if ((call_method_mgr__l_input_arg_valid == true) &&
         (call_method_mgr__l_nb_req_arg < call_method_mgr__l_nb_method_arg)) {
         *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_bad_arguments_missing;
      }
      else if ((call_method_mgr__l_nb_req_arg > call_method_mgr__l_nb_method_arg) ||
         (call_method_mgr__l_input_arg_valid == false)) {
         *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_bad_too_many_arguments;
      }
      else {
         *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_ok;
         call_method_result_it__init_iter_callMethodResultIdx(call_method_mgr__l_nb_req_arg,
            &call_method_mgr__l_continue);
         if (call_method_mgr__l_continue == true) {
            msg_call_method_bs__alloc_CallMethod_Res_InputArgumentResult(call_method_mgr__p_res_msg,
               call_method_mgr__p_callMethod,
               call_method_mgr__l_nb_req_arg,
               call_method_mgr__StatusCode);
            call_method_mgr__l_continue = (*call_method_mgr__StatusCode == constants_statuscodes_bs__e_sc_ok);
            while (call_method_mgr__l_continue == true) {
               call_method_result_it__continue_iter_callMethodResultIdx(&call_method_mgr__l_continue,
                  &call_method_mgr__l_index);
               argument_pointer_bs__read_variant_argument(call_method_mgr__l_input_arg_variant,
                  call_method_mgr__l_index,
                  &call_method_mgr__l_arg_desc);
               msg_call_method_bs__read_CallMethod_InputArguments(call_method_mgr__p_req_msg,
                  call_method_mgr__p_callMethod,
                  call_method_mgr__l_index,
                  &call_method_mgr__l_val);
               call_method_mgr__check_method_call_one_argument_type(call_method_mgr__l_val,
                  call_method_mgr__l_arg_desc,
                  &call_method_mgr__l_arg_status);
               msg_call_method_bs__write_CallMethod_Res_InputArgumentResult(call_method_mgr__p_res_msg,
                  call_method_mgr__p_callMethod,
                  call_method_mgr__l_index,
                  call_method_mgr__l_arg_status);
               if (call_method_mgr__l_arg_status != constants_statuscodes_bs__e_sc_ok) {
                  *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_bad_invalid_argument;
               }
            }
            if (*call_method_mgr__StatusCode == constants_statuscodes_bs__e_sc_ok) {
               msg_call_method_bs__free_CallMethod_Res_InputArgument(call_method_mgr__p_res_msg,
                  call_method_mgr__p_callMethod);
            }
         }
      }
   }
}

void call_method_mgr__check_method_call_one_argument_type(
   const constants__t_Variant_i call_method_mgr__p_value,
   const constants__t_Argument_i call_method_mgr__p_arg,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode) {
   {
      constants__t_NodeId_i call_method_mgr__l_value_type;
      t_entier4 call_method_mgr__l_value_valueRank;
      constants__t_NodeId_i call_method_mgr__l_arg_type;
      t_entier4 call_method_mgr__l_arg_valueRank;
      t_bool call_method_mgr__l_bool;
      t_bool call_method_mgr__l_compat_with_conv;
      
      address_space_itf__get_conv_Variant_Type(call_method_mgr__p_value,
         &call_method_mgr__l_value_type);
      address_space_itf__get_conv_Variant_ValueRank(call_method_mgr__p_value,
         &call_method_mgr__l_value_valueRank);
      argument_pointer_bs__read_argument_type(call_method_mgr__p_arg,
         &call_method_mgr__l_arg_type);
      argument_pointer_bs__read_argument_valueRank(call_method_mgr__p_arg,
         &call_method_mgr__l_arg_valueRank);
      address_space_itf__read_variable_compat_type(call_method_mgr__l_value_type,
         call_method_mgr__l_value_valueRank,
         call_method_mgr__l_arg_type,
         call_method_mgr__l_arg_valueRank,
         &call_method_mgr__l_bool,
         &call_method_mgr__l_compat_with_conv);
      if (call_method_mgr__l_bool == false) {
         *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_bad_type_mismatch;
      }
      else {
         *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_ok;
      }
   }
}

void call_method_mgr__copy_exec_result(
   const constants__t_msg_i call_method_mgr__p_res_msg,
   const constants__t_CallMethod_i call_method_mgr__p_callMethod,
   constants_statuscodes_bs__t_StatusCode_i * const call_method_mgr__StatusCode) {
   {
      t_bool call_method_mgr__l_continue;
      t_entier4 call_method_mgr__l_nb;
      constants__t_Variant_i call_method_mgr__l_value;
      t_entier4 call_method_mgr__l_index;
      
      call_method_bs__read_nb_exec_result(&call_method_mgr__l_nb);
      call_method_result_it__init_iter_callMethodResultIdx(call_method_mgr__l_nb,
         &call_method_mgr__l_continue);
      if (call_method_mgr__l_continue == false) {
         *call_method_mgr__StatusCode = constants_statuscodes_bs__e_sc_ok;
      }
      else {
         msg_call_method_bs__alloc_CallMethod_Res_OutputArgument(call_method_mgr__p_res_msg,
            call_method_mgr__p_callMethod,
            call_method_mgr__l_nb,
            call_method_mgr__StatusCode);
         call_method_mgr__l_continue = (*call_method_mgr__StatusCode == constants_statuscodes_bs__e_sc_ok);
         while (call_method_mgr__l_continue == true) {
            call_method_result_it__continue_iter_callMethodResultIdx(&call_method_mgr__l_continue,
               &call_method_mgr__l_index);
            call_method_bs__read_exec_result(call_method_mgr__l_index,
               &call_method_mgr__l_value);
            msg_call_method_bs__write_CallMethod_Res_OutputArgument(call_method_mgr__p_res_msg,
               call_method_mgr__p_callMethod,
               call_method_mgr__l_index,
               call_method_mgr__l_value);
            call_method_mgr__l_continue = (call_method_mgr__l_continue == true);
         }
      }
   }
}

