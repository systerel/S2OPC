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
