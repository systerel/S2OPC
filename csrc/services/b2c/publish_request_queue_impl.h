/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
