/******************************************************************************

 File Name            : session_header_init.c

 Date                 : 25/07/2017 17:25:14

 C Translator Version : tradc Java V1.0 (14/03/2012)

******************************************************************************/

/*------------------------
   Exported Declarations
  ------------------------*/
#include "session_header_init.h"

/*---------------------
   List of Components
  ---------------------*/
#include "address_space.h"
#include "address_space_bs.h"
#include "address_space_it.h"
#include "channel_mgr_bs.h"
#include "constants.h"
#include "constants_bs.h"
#include "io_dispatch_mgr.h"
#include "message_in_bs.h"
#include "message_out_bs.h"
#include "msg_read_request.h"
#include "msg_read_request_bs.h"
#include "msg_read_response_bs.h"
#include "request_handle_bs.h"
#include "response_write_bs.h"
#include "service_read.h"
#include "service_read_it.h"
#include "service_response_cli_cb_bs.h"
#include "service_write_decode_bs.h"
#include "session_async_bs.h"
#include "session_core.h"
#include "session_core_1_bs.h"
#include "session_core_channel_lost_it_bs.h"
#include "session_core_orphaned_it_bs.h"
#include "session_header.h"
#include "session_mgr.h"

/*------------------------
   INITIALISATION Clause
  ------------------------*/
void INITIALISATION(void) {
   constants_bs__INITIALISATION();
   constants__INITIALISATION();
   request_handle_bs__INITIALISATION();
   session_core_1_bs__INITIALISATION();
   session_core_orphaned_it_bs__INITIALISATION();
   session_core_channel_lost_it_bs__INITIALISATION();
   message_in_bs__INITIALISATION();
   message_out_bs__INITIALISATION();
   channel_mgr_bs__INITIALISATION();
   session_core__INITIALISATION();
   session_mgr__INITIALISATION();
   session_async_bs__INITIALISATION();
   msg_read_request_bs__INITIALISATION();
   address_space_bs__INITIALISATION();
   response_write_bs__INITIALISATION();
   address_space_it__INITIALISATION();
   service_write_decode_bs__INITIALISATION();
   address_space__INITIALISATION();
   msg_read_request__INITIALISATION();
   msg_read_response_bs__INITIALISATION();
   service_read_it__INITIALISATION();
   service_read__INITIALISATION();
   service_response_cli_cb_bs__INITIALISATION();
   io_dispatch_mgr__INITIALISATION();
   session_header__INITIALISATION();
}

