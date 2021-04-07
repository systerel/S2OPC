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

#ifndef SOPC_PUBLISH_REQUEST_QUEUE_IMPL_H_
#define SOPC_PUBLISH_REQUEST_QUEUE_IMPL_H_

#include "constants.h"

typedef struct SOPC_InternalPublishRequestQueueElement
{
    constants__t_session_i session;
    constants__t_timeref_i req_exp_time;
    constants__t_server_request_handle_i req_handle;
    constants__t_request_context_i req_ctx;
    constants__t_msg_i resp_msg;

} SOPC_InternalPublishRequestQueueElement;

#endif /* SOPC_PUBLISH_REQUEST_QUEUE_IMPL_H_ */
